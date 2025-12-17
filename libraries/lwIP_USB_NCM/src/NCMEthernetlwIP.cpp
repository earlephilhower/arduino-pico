#include "NCMEthernetlwIP.h"
#include <LwipEthernet.h>
#include <tusb.h>
#include <pico/async_context_threadsafe_background.h>
#include <Arduino.h>

NCMEthernetlwIP::NCMEthernetlwIP() {
}

bool NCMEthernetlwIP::begin(const uint8_t *macAddress, const uint16_t mtu) {
    // super call
    return LwipIntfDev<NCMEthernet>::begin(macAddress, mtu);
}

void NCMEthernetlwIP::packetReceivedIRQWorker(NCMEthernet *instance) {
    NCMEthernetlwIP *d = static_cast<NCMEthernetlwIP*>(instance);
    d->_irq(instance);
}
