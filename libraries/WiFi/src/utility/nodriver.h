#pragma once

#include <Arduino.h>
#include <SPI.h>

class NoDriver {
public:

    NoDriver(int8_t cs, arduino::SPIClass& spi, int8_t intrpin) {
        (void) cs;
        (void) spi;
        (void) intrpin;
    }

    bool begin(const uint8_t* address, netif *netif) {
        (void) address;
        (void) netif;
        return false;
    }

    uint16_t sendFrame(const uint8_t* data, uint16_t datalen) {
        (void) data;
        (void) datalen;
        return 0;
    }

    uint16_t readFrame(uint8_t* buffer, uint16_t bufsize) {
        (void) buffer;
        (void) bufsize;
        return 0;
    }

    uint16_t readFrameSize() {
        return 0;
    }

    void discardFrame(uint16_t ign) {
        (void) ign;
    }

    uint16_t readFrameData(uint8_t *ign1, uint16_t ign2) {
        (void) ign1;
        (void) ign2;
        return 0;
    }

    bool interruptIsPossible() {
        return true;
    }

    PinStatus interruptMode() {
        return LOW;
    }

    constexpr bool needsSPI() const {
        return false;
    }

};
