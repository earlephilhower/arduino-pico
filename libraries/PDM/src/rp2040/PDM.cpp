

#include "Arduino.h"
#include "PDM.h"
#include "OpenPDMFilter.h"

#include "AudioBufferManager.h"

extern "C" {
#include <hardware/pio.h>
#include <hardware/dma.h>
#include <hardware/clocks.h>
}
#include <hardware/sync.h>
#include "pdm.pio.h"
static PIOProgram _pdmPgm(&pdm_pio_program);

// Raw PDM data is read from the PIO by an AudioBufferManager, so the DMA
// keeps running between blocks: an interrupt that comes late no longer
// drops PDM bits (a click), and one that is missed entirely (e.g. during
// a flash erase) only loses audio, never writes outside the buffers.
#define RAW_BUFFER_SIZE 512 // bytes, should be a multiple of (decimation / 8)
#define RAW_BUFFER_WORDS (RAW_BUFFER_SIZE / 4)
#define RAW_BUFFERS 4       // 2 being filled by the DMA + 2 spare
static uint32_t rawBuffer[RAW_BUFFER_WORDS]; // block being filtered

int decimation = 128;

// final buffer is the one to be filled with PCM data
int16_t* volatile finalBuffer;

// OpenPDM filter used to convert PDM into PCM
#define FILTER_GAIN     16
TPDMFilter_InitStruct filter;

// Called by the AudioBufferManager (DMA interrupt) when a block is ready
static void __not_in_flash_func(pdmBlockReady)(void *) {
    PDM.IrqHandler(true);
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
    _arb(nullptr),
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

    _cutSamples = 100;

    // The PIO pushes one byte of PDM bits at a time
    _arb = new AudioBufferManager(RAW_BUFFERS, RAW_BUFFER_WORDS, 0, INPUT, DMA_SIZE_8);
    _arb->setCallback(pdmBlockReady, nullptr);

    if (!_arb->begin(pio_get_dreq(_pio, _smIdx, false), (volatile void *)&_pio->rxf[_smIdx])) {
        // ERROR, no free DMA channels
        delete _arb;
        _arb = nullptr;
        pio_sm_unclaim(_pio, _smIdx);
        return 0;
    }

    _init = 1;

    return 1;
}

void PDMClass::end() {

    if (_init != 1) {
        return;
    }

    // Stops and releases the DMA channels
    delete _arb;
    _arb = nullptr;

    pio_sm_unclaim(_pio, _smIdx);
    pinMode(_clkPin, INPUT);
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

    // Filter every block the DMA has finished (more than one if this
    // interrupt came late)
    while (_arb && _arb->available() >= RAW_BUFFER_WORDS) {

        _arb->read(rawBuffer, RAW_BUFFER_WORDS, false);
        uint8_t *raw = reinterpret_cast<uint8_t *>(rawBuffer);

        if (!_doubleBuffer.available()) {
            // fill final buffer with PCM samples
            if (filter.Decimation == 128) {
                Open_PDM_Filter_128(raw, finalBuffer, 1, &filter);
            } else {
                Open_PDM_Filter_64(raw, finalBuffer, 1, &filter);
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


