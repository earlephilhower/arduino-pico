#pragma once

#include <LwipIntfDev.h>
#include "nodriver.h"

class NoDriverLwIP : public LwipIntfDev<NoDriver> {
public:

    NoDriverLwIP() :
        LwipIntfDev<NoDriver>() {
    }

    bool initHW(bool apMode) {
        (void) apMode;
        return false;
    }

    void setSSID(const char *p) {
        (void) p;
    }

    void setBSSID(const uint8_t *bssid) {
        (void) bssid;
    }

    void setPassword(const char *p) {
        (void) p;
    }

    void setSTA() {
    }

    void setAP() {
    }

    void setTimeout(int timeout) {
        (void) timeout;
    }

    void end() {
    }

    bool connected() {
        return false;
    }

    uint8_t* macAddress(bool apMode, uint8_t *mac) {
        (void) apMode;
        return mac;
    }

    uint8_t* BSSID(uint8_t *bssid) {
        return bssid;
    }

    int32_t RSSI() {
        return 0;
    }

    int channel() {
        return -1;
    }

    uint8_t encryptionType() {
        return ENC_TYPE_AUTO;
    }

    int8_t scanNetworks(bool async = false) {
        (void) async;
        return 0;
    }

    int8_t scanComplete() {
        return 0;
    }

    void scanDelete() {
    }

    const char* SSID(uint8_t networkItem) {
        (void) networkItem;
        return nullptr;
    }

    uint8_t encryptionType(uint8_t networkItem) {
        (void) networkItem;
        return ENC_TYPE_AUTO;
    }

    uint8_t* BSSID(uint8_t networkItem, uint8_t *bssid) {
        (void) networkItem;
        return bssid;
    }

    uint8_t channel(uint8_t networkItem) {
        (void) networkItem;
        return 0;
    }

    int32_t RSSI(uint8_t networkItem) {
        (void) networkItem;
        return 0;
    }

    uint8_t status() {
        return WL_NO_MODULE;
    }

    void noLowPowerMode() {
    }

};
