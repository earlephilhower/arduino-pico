// Abstract class for audio output devices to allow easy swapping between output devices

#pragma once

#include <Print.h>

class AudioOutputBase : public Print {
public:
    virtual ~AudioOutputBase() { }
    virtual bool setBuffers(size_t buffers, size_t bufferWords, int32_t silenceSample = 0) = 0;
    virtual bool setBitsPerSample(int bps) = 0;
    virtual bool setFrequency(int freq) = 0;
    virtual bool setStereo(bool stereo = true) = 0;
    virtual bool begin() = 0;
    virtual bool end() = 0;
    virtual bool getUnderflow() = 0;
    virtual void onTransmit(void(*)(void *), void *) = 0;
    // From Print
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
    virtual int availableForWrite() = 0;
};
