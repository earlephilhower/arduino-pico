/*
    WiFi <-> LWIP for ESPHost library in RP2040 Core

    Copyright (c) 2024 Juraj Andrassy

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

#include "ESPHost.h"
#include <CEspControl.h>
#include <LwipEthernet.h>

ESPHost::ESPHost(int8_t cs, arduino::SPIClass &spi, int8_t intrpin) {
    (void) cs;
    (void) spi;
    (void) intrpin;
    // CS and SPI are configured in ESPHost library
}

bool ESPHost::begin(const uint8_t *address, netif *netif) {
    (void) address;
    (void) netif;

    // WiFi is initialized in ESPHostLwIP

    return true;
}

void ESPHost::end() {
    // WiFi is ended in ESPHostLwIP
}

uint16_t ESPHost::sendFrame(const uint8_t *data, uint16_t datalen) {
    ethernet_arch_lwip_gpio_mask();
    int res = CEspControl::getInstance().sendBuffer(apMode ? ESP_AP_IF : ESP_STA_IF, 0, (uint8_t*) data, datalen);
    CEspControl::getInstance().communicateWithEsp();
    ethernet_arch_lwip_gpio_unmask();
    return (res == ESP_CONTROL_OK) ? datalen : 0;
}

uint16_t ESPHost::readFrameData(uint8_t *buffer, uint16_t bufsize) {
    uint8_t ifn = 0;
    uint16_t res;
    if (apMode) {
        res = CEspControl::getInstance().getSoftApRx(ifn, buffer, bufsize);
    } else {
        res = CEspControl::getInstance().getStationRx(ifn, buffer, bufsize);
    }
    return res;
}

uint16_t ESPHost::readFrameSize() {
    CEspControl::getInstance().communicateWithEsp();
    uint16_t res;
    if (apMode) {
        res = CEspControl::getInstance().peekSoftApRxPayloadLen();
    } else {
        res = CEspControl::getInstance().peekStationRxPayloadLen();
    }
    return res;
}
