/*
    Copyright (c) 2024 functionpointer

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

#ifndef NCM_ETHERNET_H
#define NCM_ETHERNET_H

#include <tusb-ncm.h>
#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include <LwipEthernet.h>
#include <LwipIntfDev.h>

#ifdef __FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "freertos/freertos-lwip.h"
#else
#include "pico/util/queue.h"
#include <pico/async_context_threadsafe_background.h>
#endif

#ifndef NCMETHERNET_XMIT_QUEUE_LENGTH
#define NCMETHERNET_XMIT_QUEUE_LENGTH 12
#endif

extern "C" {
    typedef struct _ncmethernet_packet_t {
        const uint8_t *src;
        uint16_t size;
    } ncmethernet_packet_t;
}


class NCMEthernet;

extern "C" {
    extern NCMEthernet *_ncm_ethernet_instance;
}

class NCMEthernet {
public:
    // constructor and methods as required by LwipIntfDev

    NCMEthernet(int8_t cs, arduino::SPIClass &spi, int8_t intrpin);

    bool begin(const uint8_t *address, netif *netif);
    void end();

    uint16_t sendFrame(struct pbuf *pbuf);

    uint16_t readFrameSize();

    uint16_t readFrameData(uint8_t *buffer, uint16_t bufsize);

    uint16_t readFrame(uint8_t* buffer, uint16_t bufsize);

    void discardFrame(uint16_t ign);

    bool isLinked();

    constexpr bool isLinkDetectable() const {
        return true;
    }

    constexpr bool needsSPI() const {
        return false;
    }

    void usbInterfaceCB(int itf, uint8_t *dst, int len);

    volatile bool _holding_both_mutexes = false;
    bool _tud_recv_cb_called = false;
    ncmethernet_packet_t *_recv_pkg = nullptr;

#ifdef __FREERTOS
    SemaphoreHandle_t _recv_semaphore;
    QueueHandle_t _xmit_queue;

    TaskHandle_t _ncmTask;
#else
    queue_t _xmit_queue;

    async_when_pending_worker_t _recv_irq_worker;
#endif
    static void _set_recv_pending();
    void _try_process_xmit_queue();
protected:
    netif *_netif;
    uint8_t _id;
    uint8_t _epIn;
    uint8_t _epNotif;
    uint8_t _epOut;
    uint8_t _strID;
    uint8_t _strMac;
    char macAddrStr[6 * 2 + 2] = {0};

    static void _usb_interface_cb(int itf, uint8_t *dst, int len, void *param) {
        ((NCMEthernet *)param)->usbInterfaceCB(itf, dst, len);
    }

    static constexpr bool interruptIsPossible() {
        return false;
    }

    static constexpr PinStatus interruptMode() {
        return HIGH;
    }

};
#endif  // NCM_ETHERNET_H
