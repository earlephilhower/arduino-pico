#pragma once

#include <LwipIntfDev.h>
#include <utility/CYW43shim.h>

class CYW43lwIP: public LwipIntfDev<CYW43> {
public:
    CYW43lwIP(int8_t cs = SS);

    void end();
    bool connected();

    uint8_t* macAddress(bool apMode, uint8_t *mac);

    uint8_t* BSSID(uint8_t *bssid);
    int32_t RSSI();
    int channel();
    uint8_t encryptionType();

    int8_t scanNetworks(bool async = false);
    int8_t scanComplete();
    void scanDelete();

    const char* SSID(uint8_t networkItem);
    uint8_t encryptionType(uint8_t networkItem);
    uint8_t* BSSID(uint8_t networkItem, uint8_t *bssid);
    uint8_t channel(uint8_t networkItem);
    int32_t RSSI(uint8_t networkItem);

    uint8_t status();

    void noLowPowerMode();

private:

    // WiFi Scan callback
    std::map<uint64_t, cyw43_ev_scan_result_t> _scan;
    static int _scanCB(void *env, const cyw43_ev_scan_result_t *result);

};
