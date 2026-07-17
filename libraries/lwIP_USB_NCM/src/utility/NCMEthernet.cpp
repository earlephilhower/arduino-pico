/*
    Copyright (c) 2026 functionpointer

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

#include "NCMEthernet.h"
#include <LwipEthernet.h>
#include <tusb.h>
#include "USB.h"
#include <NCMEthernetlwIP.h>

#define USBD_NCM_EPSIZE 64

NCMEthernet::NCMEthernet(int8_t cs, arduino::SPIClass &spi, int8_t intrpin) {
    (void) cs;
    (void) spi;
    (void) intrpin;
}

bool NCMEthernet::begin(const uint8_t* mac_address, netif *net) {
    (void) net;
    memcpy(tud_network_mac_address, mac_address, 6);

#ifdef __FREERTOS
    this->_xmit_queue = xQueueCreate(NCMETHERNET_XMIT_QUEUE_LENGTH, sizeof(struct pbuf*));
    if (this->_xmit_queue == NULL) {
        panic("Unable to allocate NCMEthernet xmit queue");
    }
    // creating _ncmTask is done in NCMEthernetlwIP
    this->_recv_semaphore = xSemaphoreCreateCounting(10, 0);
#else
    queue_init(&this->_xmit_queue, sizeof(struct pbuf*), NCMETHERNET_XMIT_QUEUE_LENGTH);

    this->_recv_irq_worker.user_data = this;
    // this->_recv_irq_worker.do_work will be set by NCMEthernetlwIP
    // can't do that here because it has to call functions in LwipIntfDev
    async_context_add_when_pending_worker(__getEthernetContext(), &this->_recv_irq_worker);
#endif

    if (_ncm_ethernet_instance != nullptr) {
        panic("multiple NCM interfaces not supported");
    }
    _ncm_ethernet_instance = this;

    USB.disconnect();

    _epIn = USB.registerEndpointIn();
    _epOut = USB.registerEndpointOut();
    _epNotif = USB.registerEndpointIn();
    _strID = USB.registerString("Pico NCM");

    // mac address we give to the PC
    // must be different than our own
    uint8_t len = 0;
    for (unsigned i = 0; i < 6; i++) {
        uint8_t mac_byte = tud_network_mac_address[i];
        if (i == 5) { // invert last byte
            mac_byte ^= 0xFF;
        }
        macAddrStr[len++] = "0123456789ABCDEF"[(mac_byte >> 4) & 0xf];
        macAddrStr[len++] = "0123456789ABCDEF"[(mac_byte >> 0) & 0xf];
    }
    _strMac = USB.registerString(macAddrStr);

    _id = USB.registerInterface(2, _usb_interface_cb, (void *)this, TUD_CDC_NCM_DESC_LEN, 3, 0);

    USB.connect();

    return true;
}

void NCMEthernet::end() {
    USB.disconnect();
    USB.unregisterInterface(_id);
    USB.unregisterEndpointIn(_epIn);
    USB.unregisterEndpointIn(_epNotif);
    USB.unregisterEndpointOut(_epOut);
    USB.connect();
}

bool NCMEthernet::isLinked() {
    return tud_mounted();
}

void NCMEthernet::usbInterfaceCB(int itf, uint8_t *dst, int len) {
    uint8_t desc[TUD_CDC_NCM_DESC_LEN] = {
        // Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size.
        TUD_CDC_NCM_DESCRIPTOR((uint8_t)itf, _strID, _strMac, _epNotif, USBD_NCM_EPSIZE, _epOut, _epIn, CFG_TUD_NET_ENDPOINT_SIZE, CFG_TUD_NET_MTU)
    };
    memcpy(dst, desc, len);
}

uint16_t NCMEthernet::readFrame(uint8_t* buffer, uint16_t bufsize) {
    uint16_t data_len = this->readFrameSize();

    if (data_len == 0) {
        return 0;
    }

    if (data_len > bufsize) {
        // Packet is bigger than buffer - drop the packet
        discardFrame(data_len);
        return 0;
    }

    return readFrameData(buffer, data_len);
}

uint16_t NCMEthernet::readFrameSize() {
    if (!this->_holding_both_mutexes || this->_recv_pkg == nullptr) {
        return 0;
    }
    return this->_recv_pkg->size;
}

uint16_t NCMEthernet::readFrameData(uint8_t* buffer, uint16_t framesize) {
    if (this->_recv_pkg == nullptr) {
        return 0;
    }
    auto size = min(this->_recv_pkg->size, framesize);
    memcpy(buffer, (const void*)this->_recv_pkg->src, size);
    this->_recv_pkg = nullptr;
    return size;
}

void NCMEthernet::discardFrame(uint16_t ign) {
    (void) ign;
    this->_recv_pkg = nullptr;
}

uint16_t NCMEthernet::sendFrame(struct pbuf *p) {
    // in case of baremetal we are probably in IRQ context
    // we should be holding lwIP mutex
    // maybe also USB mutex if we were called by NCMEthernetlwIP::_call_irq (i.e. p is an answer packet)

    // in case of freertos we are in lwIP task
    // possibly also holding USB mutex if called by NCMEthernetlwIP::_call_irq
#ifdef __FREERTOS
    if (xQueueSend(this->_xmit_queue, &p, 2) != pdPASS) {
#else
    if (!queue_try_add(&_ncm_ethernet_instance->_xmit_queue, &p)) {
#endif
        // queue full, drop packet
        this->_try_process_xmit_queue();
        return 0;
    }
    // tell lwIP we are still using it
    pbuf_ref(p);


    uint16_t ret = p->tot_len;

    // we may be holding USB mutex, if so we can transmit right away
    this->_try_process_xmit_queue();

    return ret;
}

void NCMEthernet::_try_process_xmit_queue() {
    bool has_usb_mutex = false;
    if (!this->_holding_both_mutexes) {
#ifdef __FREERTOS
        NCMEthernet::_set_recv_pending();
        return;
#else
        if (mutex_try_enter(&USB.mutex, NULL)) {
            has_usb_mutex = true;
        } else {
            NCMEthernet::_set_recv_pending();
            return;
        }
#endif
    }

    struct pbuf *p = nullptr;
    while (true) {
        if (!tud_ready()) {
            break;
        }
#ifdef __FREERTOS
        if (xQueuePeek(this->_xmit_queue, &p, 0) != pdPASS) {
            break;
        }
        if (p == nullptr) {
            panic("null in queue?");
        }
#else
        if (!queue_try_peek(&this->_xmit_queue, &p)) {
            break;
        }
#endif
        if (tud_network_can_xmit(p->tot_len)) {
            tud_network_xmit(p, 0);
#ifdef __FREERTOS
            struct pbuf *removed = nullptr;
            if (xQueueReceive(this->_xmit_queue, &removed, 0) != pdPASS) {
#else
            if (!queue_try_remove(&this->_xmit_queue, nullptr)) {
#endif
                panic("couldn't remove packet from queue after transmitting");
            }
#ifdef __FREERTOS
            if (removed != p) {
                panic("different packet removed");
            }
            if (p == nullptr) {
                panic("packet is null?");
            }
#endif
            pbuf_free(p);
        }
        tud_task();
    }

#ifdef __FREERTOS
    if (uxQueueMessagesWaiting(this->_xmit_queue) > 0) {
#else
    if (!queue_is_empty(&this->_xmit_queue)) {
#endif
        NCMEthernet::_set_recv_pending();
    }

    if (has_usb_mutex) {
        mutex_exit(&USB.mutex);
    }
}

void NCMEthernet::_set_recv_pending() {
#ifdef __FREERTOS
    xSemaphoreGive(_ncm_ethernet_instance->_recv_semaphore);
#else
    async_context_set_work_pending(__getEthernetContext(), &_ncm_ethernet_instance->_recv_irq_worker);
#endif
}

extern "C" {
    // data transfer between tinyUSB callbacks and NCMEthernet class
    NCMEthernet *_ncm_ethernet_instance = nullptr;

    /*
        Interface to tinyUSB.
    */
    uint8_t tud_network_mac_address[6] = {0};

    void tud_network_init_cb(void) {
    }

    bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
        if (_ncm_ethernet_instance == nullptr) {
            return false;
        }
        ncmethernet_packet_t p;
        p.src = src;
        p.size = size;
#ifdef __FREERTOS
        // we get called as part of tud_task() from somewhere.
        // Might be in freertosUSBTask(), might be in SerialUSB stuff, delay(), or something else.
        // We are holding the non-recursive FreeRTOS Semaphore __get_freertos_mutex_for_ptr(&USB.mutex).
        // Also we won't be in IRQ context.
#else
        // We may or may not be in IRQ context, as tud_task() is called by usbTaskIRQ but also plenty of libraries.
        // We should be holding &USB.mutex.
        // If we we called by NCMEthernetlwIP::_call_irq we are also holding the lwIP mutex.
        // We can't check if we do, because LWIP mutex is a recursive mutex and the owner is cpu core number.
        // This means in IRQ context acquiring LWIP mutex will always succeed.
        // So we sidestep this issue with "_holding_both_mutexes", which is only written by NCMEthernetlwIP::_call_irq,
        // and only while holding both mutexes. i.e. if _holding_both_mutexes==true we can safely call handlePackets.
#endif

        if (!_ncm_ethernet_instance->_holding_both_mutexes) {
            // Not in lwIP, can't call handlePackets safely, as it includes pbuf_alloc.
            // So we return false, which causes tinyUSB to keep the packet until we retry with tud_network_recv_renew.
            // We schedule _recv_irq_worker / ncmTask to run. It will get both mutexes and call tud_network_recv_renew.
            NCMEthernet::_set_recv_pending();
            return false;
        }

        // tell NCMEthernetlwIP::_call_irq that we did receive a packet, so there may be even more packets.
        _ncm_ethernet_instance->_tud_recv_cb_called = true;

        if (_ncm_ethernet_instance->_recv_pkg != nullptr) {
            panic("_recv_pkg should be null");
        }

        _ncm_ethernet_instance->_recv_pkg = &p;
        static_cast<NCMEthernetlwIP*>(_ncm_ethernet_instance)->_call_handlepackets();
        if (_ncm_ethernet_instance->_recv_pkg != nullptr) {
            // handlePackets didn't take the packet for some reason
            _ncm_ethernet_instance->_recv_pkg = nullptr;
            NCMEthernet::_set_recv_pending();
            return false;
        }
        _ncm_ethernet_instance->_recv_pkg = nullptr;
        return true;
    }

    uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
        // this is called by tud_network_xmit, which is called by NCMEthernet::sendFrame
        // we have both mutexes, so it is safe to call lwIP functions

        struct pbuf *p = (struct pbuf *) ref;
        (void) arg;

        return pbuf_copy_partial(p, dst, p->tot_len, 0);
    }
}
