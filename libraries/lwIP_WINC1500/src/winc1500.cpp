/*
    WiFi <-> LWIP for ATWINC1500 in RP2040 Core

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

#include "winc1500.h"

#include <LwipEthernet.h>

extern "C" {
#include <driver/include/m2m_wifi.h>
}

WINC1500::WINC1500(int8_t cs, arduino::SPIClass &spi, int8_t intrpin) {
    (void) cs;
    (void) spi;
    (void) intrpin;
    // CS and SPI are configured with -D in nm_bsp_arduino.h for nm_bus_wrapper_arduino.cpp
}

bool WINC1500::begin(const uint8_t *address, netif *netif) {
    (void) address;
    (void) netif;

    // WiFi is initialized in WINC1500LwIP

    return true;
}

void WINC1500::end() {
    // WiFi is ended in WINC1500LwIP
}

uint16_t WINC1500::sendFrame(const uint8_t *data, uint16_t datalen) {
    ethernet_arch_lwip_gpio_mask();
    int res = m2m_wifi_send_ethernet_pkt((uint8_t *) data, datalen);
    ethernet_arch_lwip_gpio_unmask();
    return (res == M2M_SUCCESS) ? datalen : 0;
}

uint16_t WINC1500::readFrameSize() {

    // readFrameSize is invoked from polled handlePackets
    // we use it only to periodically invoke m2m_wifi_handle_events
    m2m_wifi_handle_events(NULL);

    // m2m_wifi_handle_events executes for every received input packet
    // function winc_netif_rx_callback (in lwIP_WINC1500.cpp)
    // which directly puts the packet into netif->input.
    // so here we just return 0 to end handlePackets.
    return 0;
}

extern "C" {
void winc1500debug(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buff[256];
  vsnprintf(buff, sizeof(buff), fmt, args);
  va_end(args);
  Serial.print(buff);
}
}
