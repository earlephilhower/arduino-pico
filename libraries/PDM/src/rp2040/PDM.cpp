

#include "Arduino.h"
#include "PDM.h"
#include "OpenPDMFilter.h"

extern "C" {
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
}
#include "hardware/sync.h"
#include "pdm.pio.h"
static PIOProgram _pdmPgm(&pdm_pio_program);

// raw buffers contain PDM data
#define RAW_BUFFER_SIZE 512 // should be a multiple of (decimation / 8)
uint8_t rawBuffer0[RAW_BUFFER_SIZE];
uint8_t rawBuffer1[RAW_BUFFER_SIZE];
uint8_t* rawBuffer[2] = {rawBuffer0, rawBuffer1};
volatile int rawBufferIndex = 0;

int decimation = 64;

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


PDMClass::PDMClass(int dinPin, int clkPin, int pwrPin) :
    _dinPin(dinPin),
    _clkPin(clkPin),
    _pwrPin(pwrPin),
    _onReceive(NULL),
    _gain(-1),
    _channels(-1),
    _samplerate(-1),
    _init(-1),
    _dmaChannel(0),
    _pio(nullptr),
    _smIdx(-1),
    _pgmOffset(-1) {
}

PDMClass::~PDMClass() {
}

int PDMClass::begin(int channels, int sampleRate) {
    //_channels = channels; // only one channel available

    // clear the final buffers
    _doubleBuffer.reset();
    finalBuffer = (int16_t*)_doubleBuffer.data();
    int finalBufferLength = _doubleBuffer.availableForWrite() / sizeof(int16_t);
    _doubleBuffer.swap(0);

    int rawBufferLength = RAW_BUFFER_SIZE / (decimation / 8);
    // Saturate number of samples. Remaining bytes are dropped.
    if (rawBufferLength > finalBufferLength) {
        rawBufferLength = finalBufferLength;
    }

    /* Initialize Open PDM library */
    filter.Fs = sampleRate;
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
        return -1;
    }
    pdm_pio_program_init(_pio, _smIdx, _pgmOffset, _clkPin, _dinPin, clkDiv);

    // Wait for microphone
    delay(100);

    // Configure DMA for transferring PIO rx buffer to raw buffers
    _dmaChannel = dma_claim_unused_channel(false);
    dma_channel_config c = dma_channel_get_default_config(_dmaChannel);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(_pio, _smIdx, false));
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);

    // Clear DMA interrupts
    dma_hw->ints0 = 1u << _dmaChannel;
    // Enable DMA interrupts
    dma_channel_set_irq0_enabled(_dmaChannel, true);
    // Share but allocate a high priority to the interrupt
    irq_add_shared_handler(DMA_IRQ_0, dmaHandler, 0);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_configure(_dmaChannel, &c,
                          rawBuffer[rawBufferIndex],        // Destinatinon pointer
                          &_pio->rxf[_smIdx],      // Source pointer
                          RAW_BUFFER_SIZE, // Number of transfers
                          true                // Start immediately
                         );

    _init = 1;

    return 1;
}

void PDMClass::end() {
    dma_channel_abort(_dmaChannel);
    pinMode(_clkPin, INPUT);
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
    static int cutSamples = 100;

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << _dmaChannel;
    // Restart dma pointing to the other buffer
    int shadowIndex = rawBufferIndex ^ 1;
    dma_channel_set_write_addr(_dmaChannel, rawBuffer[shadowIndex], true);

    if (_doubleBuffer.available()) {
        // buffer overflow, stop
        return end();
    }

    // fill final buffer with PCM samples
    Open_PDM_Filter_64(rawBuffer[rawBufferIndex], finalBuffer, 1, &filter);

    if (cutSamples) {
        memset(finalBuffer, 0, cutSamples);
        cutSamples = 0;
    }

    // swap final buffer and raw buffers' indexes
    finalBuffer = (int16_t*)_doubleBuffer.data();
    _doubleBuffer.swap(filter.nSamples * sizeof(int16_t));
    rawBufferIndex = shadowIndex;

    if (_onReceive) {
        _onReceive();
    }
}
#ifdef PIN_PDM_DIN
PDMClass PDM(PIN_PDM_DIN, PIN_PDM_CLK, -1);
#endif // PIN_PDM_DIN


