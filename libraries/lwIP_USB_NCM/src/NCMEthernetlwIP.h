#pragma once

#include <LwipIntfDev.h>
#include <utility/NCMEthernet.h>
#include <LwipEthernet.h>
#include <WiFi.h>
#include <pico/async_context.h>

class NCMEthernetlwIP: public LwipIntfDev<NCMEthernet> {
public:
    NCMEthernetlwIP();

    bool begin(const uint8_t* macAddress = nullptr, const uint16_t mtu = DEFAULT_MTU);

#ifdef __FREERTOS
    static void ncmTaskFunction(void *param);
    static void _call_irq(void *cbData);
#else
    static void _call_irq(__unused async_context_t *context, __unused async_when_pending_worker_t *worker);
#endif

    void _call_handlepackets();

};
