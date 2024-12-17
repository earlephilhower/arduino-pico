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
#include <string>
#include <map>
#include <vector>

typedef void* hMDNSTxt; // Unusable in SimpleMDNS, for signature compatibility only

class SimpleMDNSService {
public:
    SimpleMDNSService();
    static void callback(struct mdns_service *service, void *txt_userdata);
    hMDNSTxt add(const char *key, const char *val);
private:
    std::vector<const char *> txt;
};

// hMDNSService (opaque handle to access the service)
typedef const void* hMDNSService;

class SimpleMDNS {

public:
    bool begin(const char *hostname, unsigned int ttl = 60);
    void enableArduino(unsigned int port, bool passwd = false);

    hMDNSService addService(const char *service, const char *proto, unsigned int port);
    hMDNSService addService(const char *name, const char *service, const char *proto, unsigned int port) {
        (void) name; // Ignored
        return addService(service, proto, port);
    }

    // Add a (static) MDNS TXT item ('key' = 'value') to the service
    hMDNSTxt addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, const char* p_pcValue);
    hMDNSTxt addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, uint32_t p_u32Value);
    hMDNSTxt addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, uint16_t p_u16Value);
    hMDNSTxt addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, uint8_t p_u8Value);
    hMDNSTxt addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, int32_t p_i32Value);
    hMDNSTxt addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, int16_t p_i16Value);
    hMDNSTxt addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, int8_t p_i8Value);

    // No-ops here
    void end();
    void update();

private:
    static void _statusCB(struct netif *netif);
    static void _addServiceTxt(struct mdns_service *service, const char *str);
    static void _arduinoGetTxt(struct mdns_service *service, void *txt_userdata);

    bool _running = false;
    static const char *_hostname;
    std::map<std::string, SimpleMDNSService*> _svcMap;
    bool _arduinoAdded = false;
};

extern SimpleMDNS MDNS;

#define __SIMPLEMDNS_H 1
#ifdef __LEAMDNS_H
#error SimpleMDNS and LeaMDNS both included.  Only one allowed at a time.
#endif
