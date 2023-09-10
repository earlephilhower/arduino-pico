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


#include <Arduino.h>
#include <lwIP_CYW43.h>
//#include <W5100lwIP.h>
#include <W5500lwIP.h>
//#include <ENC28J60lwIP.h>
#include <functional>


// One of them is to be declared in the main sketch
// Wiznet5500lwIP eth(CSPIN);
// Wiznet5100lwIP eth(CSPIN);
// ENC28J60lwIP eth(CSPIN);

void ethernet_arch_lwip_begin() __attribute__((weak));
void ethernet_arch_lwip_end() __attribute__((weak));
bool ethernet_arch_lwip_try() __attribute__((weak));

// Internal Ethernet helper functions
void __addEthernetInterface(std::function<void(void)> _packetHandler);
void __startEthernetContext();
