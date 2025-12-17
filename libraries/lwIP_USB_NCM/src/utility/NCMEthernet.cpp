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

#include "NCMEthernet.h"
#include <LwipEthernet.h>
#include <tusb.h>
#include "USB.h"

#define USBD_NCM_EPSIZE 64

NCMEthernet::NCMEthernet(int8_t cs, arduino::SPIClass &spi, int8_t intrpin) {
    (void) cs;
    (void) spi;
    (void) intrpin;
}

bool NCMEthernet::begin(const uint8_t* mac_address, netif *net) {
    (void) net;
    memcpy(tud_network_mac_address, mac_address, 6);

    if (!critical_section_is_initialized(&this->_recv_critical_section)) {
        critical_section_init(&this->_recv_critical_section);
        critical_section_enter_blocking(&this->_recv_critical_section);
        this->_recv_pkg.size = 0;
        this->_recv_pkg.src = nullptr;
        critical_section_exit(&this->_recv_critical_section);
    }

    async_context_threadsafe_background_config_t config = async_context_threadsafe_background_default_config();
    if (!async_context_threadsafe_background_init(&this->_async_context, &config)) {
        return false;
    }

    this->_recv_irq_worker.user_data = this;
    this->_recv_irq_worker.do_work = &NCMEthernet::_recv_irq_work;
    async_context_add_when_pending_worker(&this->_async_context.core, &this->_recv_irq_worker);

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

// Need to define here so we don't have to include tusb.h in global header (causes problemw w/BT redefining things)
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
    return this->_recv_pkg.size;
}

uint16_t NCMEthernet::readFrameData(uint8_t* buffer, uint16_t framesize) {
    critical_section_enter_blocking(&this->_recv_critical_section);

    size_t size = this->_recv_pkg.size;
    memcpy(buffer, (const void*)this->_recv_pkg.src, size);
    this->_recv_pkg.size = 0;

    critical_section_exit(&this->_recv_critical_section);
    tud_network_recv_renew();
    return size;
}

uint16_t NCMEthernet::sendFrame(const uint8_t* buf, uint16_t len) {
    // this is basically linkoutput_fn

    for (;;) {
        /* if TinyUSB isn't ready, we must signal back to lwip that there is nothing we can do */
        if (!tud_ready()) {
            return 0;
        }

        /* if the network driver can accept another packet, we make it happen */
        if (tud_network_can_xmit(len)) {
            tud_network_xmit((void*)const_cast<uint8_t*>(buf), len);
            return len;
        }

        /* transfer execution to TinyUSB in the hopes that it will finish transmitting the prior packet */
        tud_task();
    }
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
        if (_ncm_ethernet_instance == nullptr || _ncm_ethernet_instance->_recv_pkg.size > 0) {
            return false;
        }

        critical_section_enter_blocking(&_ncm_ethernet_instance->_recv_critical_section);
        _ncm_ethernet_instance->_recv_pkg.src = src;
        _ncm_ethernet_instance->_recv_pkg.size = size;
        critical_section_exit(&_ncm_ethernet_instance->_recv_critical_section);

        async_context_set_work_pending(&_ncm_ethernet_instance->_async_context.core, &_ncm_ethernet_instance->_recv_irq_worker);

        return true;
    }

    uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
        // this is called by tud_network_xmit, which is called by NCMEthernet::sendFrame,
        // which is called by LwipIntfDev<RawDev>::linkoutput_s
        // linkoutput_s gives us pbuf->payload and pbuf->len

        memcpy(dst, ref, arg);
        return arg;
    }
}