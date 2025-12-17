#pragma once

#include <LwipIntfDev.h>
#include <utility/NCMEthernet.h>
#include <LwipEthernet.h>
#include <WiFi.h>

class NCMEthernetlwIP: public LwipIntfDev<NCMEthernet> {
public:
    NCMEthernetlwIP();

    bool begin(const uint8_t* macAddress = nullptr, const uint16_t mtu = DEFAULT_MTU);
    void packetReceivedIRQWorker(NCMEthernet *instance) override;

};
