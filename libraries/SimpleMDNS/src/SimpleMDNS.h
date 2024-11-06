/*
    SimpleMDNS for Rasperry Pi Pico
    Implements a basic MDNS responder (xxx.local)

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

class SimpleMDNS {

public:
    void begin(const char *hostname, unsigned int ttl = 60);
    void enableArduino(unsigned int port, bool passwd = false);
    void addService(const char *service, const char *proto, unsigned int port);

    // No-ops here
    void end();
    void update();

private:
    static void _statusCB(struct netif *netif);
    static void _addServiceTxt(struct mdns_service *service, const char *str);
    static void _arduinoGetTxt(struct mdns_service *service, void *txt_userdata);
    static void _nullGetTxt(struct mdns_service *service, void *txt_userdata);

    bool _running = false;
    static const char *_hostname;
};

extern SimpleMDNS MDNS;
