/*
    WiFi <-> LWIP driver for the CYG43 chip on the Raspberry Pico W

    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "lwip/netif.h"
extern "C" {
#include "cyw43.h"
#include "cyw43_stats.h"
}
class CYW43 {
public:
    /**
        Constructor that uses the default hardware SPI pins
        @param cs the Arduino Chip Select / Slave Select pin (default 10)
    */
    CYW43(int8_t cs, arduino::SPIClass& spi, int8_t intrpin);

    /**
        Initialise the Ethernet controller
        Must be called before sending or receiving Ethernet frames

        @param address the local MAC address for the Ethernet interface
        @return Returns true if setting up the Ethernet interface was successful
    */
    bool begin(const uint8_t* address, netif *netif);

    /**
        Shut down the Ethernet controlled
    */
    void end();

    /**
        Send an Ethernet frame
        @param data a pointer to the data to send
        @param datalen the length of the data in the packet
        @return the number of bytes transmitted
    */
    uint16_t sendFrame(const uint8_t* data, uint16_t datalen);

    /**
        Read an Ethernet frame
        @param buffer a pointer to a buffer to write the packet to
        @param bufsize the available space in the buffer
        @return the length of the received packet
               or 0 if no packet was received
    */
    uint16_t readFrame(uint8_t* buffer, uint16_t bufsize);

    bool interruptIsPossible() {
        return true;
    }

    void setSSID(const char *p) {
        _ssid = p;
    }
    void setPassword(const char *p) {
        _password = p;
    }

    void setSTA() {
        _ap = false;
    }

    void setAP() {
        _ap = true;
    }

    void setTimeout(int timeout) {
        _timeout = timeout;
    }

    // LWIP netif for the IRQ packet processing
    static netif   *_netif;
protected:
    int _timeout = 10000;
    bool     _ap = false;
    // The WiFi driver object
    cyw43_t *_self;
    int      _itf;
    const char *_ssid = nullptr;
    const char *_password = nullptr;
};
