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

#include <stdint.h>
#include <Arduino.h>
#include "pico/util/queue.h"
#include <SPI.h>
#include <LwipEthernet.h>
#include <pico/async_context_threadsafe_background.h>
#include <pico/critical_section.h>

extern "C" {
	typedef struct _ncmethernet_packet_t {
    	const uint8_t *src;
    	uint16_t size;
	} ncmethernet_packet_t;
}

/**
* incoming packet flow:
* tinyUSB calls tud_network_recv_cb
* that stores the packet in _ncmethernet_pkg and sets _ncm_ethernet_recv_irq_worker pending
* _ncm_ethernet_recv_irq_worker, in different execution context, calls _recv_irq_work
* _recv_irq_work uses _ncm_ethernet_instance to call packetReceivedIRQWorker
* in NCMEthernetlwIP packetReceivedIRQWorker is overridden to call LwipIntfDev::_irq()
* LwipIntfDev::_irq() calls readFrameSize() and readFrameData() and _netif.input
*
* outgoing paket flow:
* LwipIntfDev calls sendFrame()
*/

class NCMEthernet {
public:
    // constructor and methods as required by LwipIntfDev

    NCMEthernet(int8_t cs, arduino::SPIClass &spi, int8_t intrpin);

    bool begin(const uint8_t *address, netif *netif);
    void end();

    uint16_t sendFrame(const uint8_t *data, uint16_t datalen);

    uint16_t readFrameSize();

    uint16_t readFrameData(uint8_t *buffer, uint16_t bufsize);

    uint16_t readFrame(uint8_t* buffer, uint16_t bufsize);

    void discardFrame(uint16_t ign) {
      (void) ign;
    }

    bool interruptIsPossible() {
      return false;
    }

    PinStatus interruptMode() {
      return HIGH;
    }

    constexpr bool needsSPI() const {
      return false;
    }

	void usbInterfaceCB(int itf, uint8_t *dst, int len);

	virtual void packetReceivedIRQWorker(NCMEthernet *instance) {};

	async_context_threadsafe_background_t _async_context;
	async_when_pending_worker_t _recv_irq_worker;

	critical_section_t _recv_critical_section;
	ncmethernet_packet_t _recv_pkg;
protected:
    netif *_netif;
    uint8_t _id;
    uint8_t _epIn;
    uint8_t _epNotif;
    uint8_t _epOut;
    uint8_t _strID;
    uint8_t _strMac;
	char macAddrStr[6*2+2] = {0};

    static void _usb_interface_cb(int itf, uint8_t *dst, int len, void *param) {
        ((NCMEthernet *)param)->usbInterfaceCB(itf, dst, len);
    }
	static void _recv_irq_work(async_context_t *context, async_when_pending_worker_t *worker) {
		NCMEthernet *d = static_cast<NCMEthernet*>(worker->user_data);
		d->packetReceivedIRQWorker(d);
	}
};

extern "C" {
	extern NCMEthernet *_ncm_ethernet_instance;
}

#endif  // NCM_ETHERNET_H
