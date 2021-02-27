#ifndef __SERIALUSB_H__
#define __SERIALUSB_H__

#include "api/Stream.h"

class SerialUSB : public Stream
{
public:
    SerialUSB() { }
    void begin(int baud = 115200);
    void end();

    virtual int peek() override;
    virtual int read() override;
    virtual int available() override;
    virtual int availableForWrite() override;
    virtual void flush() override;
    virtual size_t write(uint8_t c) override;
    virtual size_t write(const uint8_t *p, size_t len) override;
    using Print::write;
    operator bool();
private:
    bool _running = false;
};

extern SerialUSB Serial;

#endif


