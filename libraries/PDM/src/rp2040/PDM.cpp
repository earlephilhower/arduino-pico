

#include "Arduino.h"
#include "PDM.h"
#include "OpenPDMFilter.h"

extern "C" {
#include <hardware/pio.h>
#include <hardware/dma.h>
#include <hardware/clocks.h>
}
#include <hardware/sync.h>
#include "pdm.pio.h"
static PIOProgram _pdmPgm(&pdm_pio_program);

// raw buffers contain PDM data
#define RAW_BUFFER_SIZE 512 // should be a multiple of (decimation / 8)
uint8_t rawBuffer0[RAW_BUFFER_SIZE];
uint8_t rawBuffer1[RAW_BUFFER_SIZE];
uint8_t* rawBuffer[2] = {rawBuffer0, rawBuffer1};
volatile int rawBufferIndex = 0;

int decimation = 128;

// Two DMA channels chained into each other (A fills rawBuffer0, then B
// fills rawBuffer1, then A again...), so the hardware never stops
// reading the PIO. With a single channel restarted from the interrupt,
// any interrupt delay longer than the PIO FIFO (~10 us of PDM bits)
// dropped bits and made a click; now the interrupt may be up to one
// whole block late. _dmaChannel is A, dmaChannelB is B.
static int dmaChannelB = -1;
static volatile int nextRaw = 0;   // which buffer completes next

// final buffer is the one to be filled with PCM data
int16_t* volatile finalBuffer;

// OpenPDM filter used to convert PDM into PCM
#define FILTER_GAIN     16
TPDMFilter_InitStruct filter;

extern "C" {
    __attribute__((__used__)) void dmaHandler(void) {
        PDM.IrqHandler(true);
    }
}


PDMClass::PDMClass(int dinPin, int clkPin) :
    _dinPin(dinPin),
    _clkPin(clkPin),
    _onReceive(NULL),
    _gain(-1),
    _channels(-1),
    _samplerate(-1),
    _init(-1),
    _cutSamples(100),
    _dmaChannel(0),
    _pio(nullptr),
    _smIdx(-1),
    _pgmOffset(-1) {
}

PDMClass::~PDMClass() {
}

void PDMClass::setCLK(int clkPin) {
    _clkPin = clkPin;
}

void PDMClass::setDIN(int dinPin) {
    _dinPin = dinPin;
}

int PDMClass::begin(int channels, int sampleRate) {

    if (_init == 1) {
        //ERROR: please call end first
        return 0;
    }

    if (_dinPin == -1 || _clkPin == -1) {
        //ERROR: please call setDIN/setCLK first
        return 0;
    }

    //_channels = channels; // only one channel available

    // clear the final buffers
    _doubleBuffer.reset();
    finalBuffer = (int16_t*)_doubleBuffer.data();
    int finalBufferLength = _doubleBuffer.availableForWrite() / sizeof(int16_t);
    _doubleBuffer.swap(0);

    // The mic accepts an input clock from 1.2 to 3.25 Mhz
    // Setup the decimation factor accordingly.
    // PDM clock = sampleRate * decimation (the PIO program takes
    // two instructions per bit, which clkDiv below accounts for).
    decimation = 128;
    if ((sampleRate * decimation) > 3250000) {
        decimation = 64;
    }

    // Sanity check, abort if still over 3.25Mhz
    if ((sampleRate * decimation) > 3250000) {
        //ERROR:  Sample rate too high, the mic would glitch
        return -1;
    }

    int rawBufferLength = RAW_BUFFER_SIZE / (decimation / 8);
    // Saturate number of samples. Remaining bytes are dropped.
    if (rawBufferLength > finalBufferLength) {
        rawBufferLength = finalBufferLength;
    }

    /* Initialize Open PDM library */
    filter.Fs = sampleRate;
    filter.MaxVolume = 1;
    filter.nSamples = rawBufferLength;
    filter.LP_HZ = sampleRate / 2;
    filter.HP_HZ = 10;
    filter.In_MicChannels = 1;
    filter.Out_MicChannels = 1;
    filter.Decimation = decimation;
    if (_gain == -1) {
        _gain = FILTER_GAIN;
    }
    filter.filterGain = _gain;
    Open_PDM_Filter_Init(&filter);

    // Configure PIO state machine
    float clkDiv = (float)clock_get_hz(clk_sys) / sampleRate / decimation / 2;

    if (!_pdmPgm.prepare(&_pio, &_smIdx, &_pgmOffset)) {
        // ERROR, no free slots
        return 0;
    }
    pdm_pio_program_init(_pio, _smIdx, _pgmOffset, _clkPin, _dinPin, clkDiv);

    // Wait for microphone
    delay(100);

    // Configure two chained DMA channels for transferring the PIO rx
    // FIFO into the raw buffers, without ever stopping
    // (claimed into ints first: _dmaChannel is unsigned, so a failed
    // claim, -1, could not be detected on it)
    const int chA = dma_claim_unused_channel(false);
    const int chB = dma_claim_unused_channel(false);

    if (chA < 0 || chB < 0) {
        if (chA >= 0) {
            dma_channel_unclaim(chA);
        }
        if (chB >= 0) {
            dma_channel_unclaim(chB);
        }
        pio_sm_unclaim(_pio, _smIdx);
        return 0;
    }
    _dmaChannel = chA;
    dmaChannelB = chB;

    dma_channel_config ca = dma_channel_get_default_config(_dmaChannel);
    channel_config_set_read_increment(&ca, false);
    channel_config_set_write_increment(&ca, true);
    channel_config_set_dreq(&ca, pio_get_dreq(_pio, _smIdx, false));
    channel_config_set_transfer_data_size(&ca, DMA_SIZE_8);

    dma_channel_config cb = ca;
    channel_config_set_chain_to(&ca, dmaChannelB);
    channel_config_set_chain_to(&cb, _dmaChannel);

    // Clear DMA interrupts
    dma_hw->ints0 = (1u << _dmaChannel) | (1u << dmaChannelB);
    // Enable DMA interrupts
    dma_channel_set_irq0_enabled(_dmaChannel, true);
    dma_channel_set_irq0_enabled(dmaChannelB, true);
    // Share but allocate a high priority to the interrupt
    irq_add_shared_handler(DMA_IRQ_0, dmaHandler, 0);
    irq_set_enabled(DMA_IRQ_0, true);

    nextRaw = 0;
    rawBufferIndex = 0;

    // B waits to be triggered by A's chain
    dma_channel_configure(dmaChannelB, &cb,
                          rawBuffer[1],            // Destination pointer
                          &_pio->rxf[_smIdx],      // Source pointer
                          RAW_BUFFER_SIZE,         // Number of transfers
                          false                    // Wait for A
                         );
    dma_channel_configure(_dmaChannel, &ca,
                          rawBuffer[0],            // Destination pointer
                          &_pio->rxf[_smIdx],      // Source pointer
                          RAW_BUFFER_SIZE,         // Number of transfers
                          true                     // Start immediately
                         );

    _cutSamples = 100;

    _init = 1;

    return 1;
}

void PDMClass::end() {

    if (_init != 1) {
        return;
    }

    dma_channel_set_irq0_enabled(_dmaChannel, false);
    dma_channel_set_irq0_enabled(dmaChannelB, false);

    // Break the chain (each channel chains to itself = no chaining),
    // then stop both
    hw_write_masked(&dma_hw->ch[_dmaChannel].al1_ctrl, _dmaChannel << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB,
                    DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS);
    hw_write_masked(&dma_hw->ch[dmaChannelB].al1_ctrl, dmaChannelB << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB,
                    DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS);
    dma_channel_abort(_dmaChannel);
    dma_channel_abort(dmaChannelB);
    dma_hw->ints0 = (1u << _dmaChannel) | (1u << dmaChannelB);
    dma_channel_unclaim(_dmaChannel);
    dma_channel_unclaim(dmaChannelB);
    dmaChannelB = -1;
    irq_remove_handler(DMA_IRQ_0, dmaHandler);
    pio_sm_unclaim(_pio, _smIdx);
    pinMode(_clkPin, INPUT);
    rawBufferIndex = 0;
    _pgmOffset = -1;

    _init = 0;
}

int PDMClass::available() {
    //NVIC_DisableIRQ(DMA_IRQ_0n);
    //uint32_t interrupts = save_and_disable_interrupts();
    irq_set_enabled(DMA_IRQ_0, false);
    size_t avail = _doubleBuffer.available();
    irq_set_enabled(DMA_IRQ_0, true);
    //restore_interrupts(interrupts);
    //NVIC_EnableIRQ(DMA_IRQ_0n);
    return avail;
}

int PDMClass::read(void* buffer, size_t size) {
    irq_set_enabled(DMA_IRQ_0, false);
    int read = _doubleBuffer.read(buffer, size);
    irq_set_enabled(DMA_IRQ_0, true);
    return read;
}

void PDMClass::onReceive(void(*function)(void)) {
    _onReceive = function;
}

void PDMClass::setGain(int gain) {
    _gain = gain;
    if (_init == 1) {
        filter.filterGain = _gain;
        Open_PDM_Filter_Init(&filter);
    }
}

void PDMClass::setBufferSize(int bufferSize) {
    _doubleBuffer.setSize(bufferSize);
}

void PDMClass::IrqHandler(bool halftranfer) {

    (void) halftranfer;

    // The DMA_IRQ_0 handler is shared: only act on our channels, in the
    // order they complete (both if this interrupt came very late)
    while (true) {

        const int ch = nextRaw ? dmaChannelB : _dmaChannel;
        const uint32_t bit = 1u << ch;

        if (!(dma_hw->ints0 & bit)) {
            break;
        }

        // Clear the interrupt request and re-arm this channel for its
        // next turn (the other one is already filling its buffer)
        dma_hw->ints0 = bit;
        dma_channel_set_write_addr(ch, rawBuffer[nextRaw], false);
        rawBufferIndex = nextRaw;
        nextRaw ^= 1;

        if (!_doubleBuffer.available()) {
            // fill final buffer with PCM samples
            if (filter.Decimation == 128) {
                Open_PDM_Filter_128(rawBuffer[rawBufferIndex], finalBuffer, 1, &filter);
            } else {
                Open_PDM_Filter_64(rawBuffer[rawBufferIndex], finalBuffer, 1, &filter);
            }

            if (_cutSamples) {
                memset(finalBuffer, 0, _cutSamples);
                _cutSamples = 0;
            }

            // swap final buffer
            finalBuffer = (int16_t*)_doubleBuffer.data();
            _doubleBuffer.swap(filter.nSamples * sizeof(int16_t));
        }

        if (_onReceive) {
            _onReceive();
        }
    }
}
#ifndef PIN_PDM_DIN
#define PIN_PDM_DIN -1
#endif // PIN_PDM_DIN
//
#ifndef PIN_PDM_CLK
#define PIN_PDM_CLK -1
#endif // PIN_PDM_CLK

PDMClass PDM(PIN_PDM_DIN, PIN_PDM_CLK);


