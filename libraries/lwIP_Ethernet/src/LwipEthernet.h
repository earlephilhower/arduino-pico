/*
    LwipEthernet.h

    Arduino interface for lwIP generic callbacks and functions

    Original Copyright (c) 2020 esp8266 Arduino All rights reserved.

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

#pragma once

#include <Arduino.h>
#include <functional>

void ethernet_arch_lwip_begin() __attribute__((weak));
void ethernet_arch_lwip_end() __attribute__((weak));
void ethernet_arch_lwip_gpio_mask() __attribute__((weak));
void ethernet_arch_lwip_gpio_unmask() __attribute__((weak));

void __addEthernetGPIO(int pin);
void __removeEthernetGPIO(int pin);

// Internal Ethernet helper functions
void __startEthernetContext();

int __addEthernetPacketHandler(std::function<void(void)> _packetHandler);
void __removeEthernetPacketHandler(int id);

// Used by WiFiClient to get DNS lookup
int hostByName(const char *aHostname, IPAddress &aResult, int timeout_ms = 5000);

// Set the LWIP polling time (default 50ms).  Lower polling times == lower latency but higher CPU usage
void lwipPollingPeriod(int ms);

// Sets the global netif state change callback
void __setStateChangeCallback(std::function<void(struct netif *)> s);
