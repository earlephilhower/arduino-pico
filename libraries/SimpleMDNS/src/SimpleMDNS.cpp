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

#include <Arduino.h>
#include "SimpleMDNS.h"
#include <LwipEthernet.h>
#include <lwip/apps/mdns.h>

bool SimpleMDNS::begin(const char *hostname, unsigned int ttl) {
    if (_running) {
        return false;
    }
    mdns_resp_init();
    struct netif *n = netif_list;
    while (n) {
        mdns_resp_add_netif(n, hostname);
        n = n->next;
    }
    __setStateChangeCallback(_statusCB);
    _hostname = strdup(hostname);
    _running = true;

    return true;
}

void SimpleMDNS::enableArduino(unsigned int port, bool passwd) {
    if (!_running || _arduinoAdded) {
        return;
    }
    struct netif *n = netif_list;
    while (n) {
        mdns_resp_add_service(n, _hostname, "_arduino", DNSSD_PROTO_TCP, port, _arduinoGetTxt, (void *)passwd);
        n = n->next;
    }
    _arduinoAdded = true;
}

hMDNSService SimpleMDNS::addService(const char *service, const char *proto, unsigned int port) {
    if (!_running) {
        return nullptr;
    }
    if (_svcMap.find(service) != _svcMap.end()) {
        // Duplicates = error
        return nullptr;
    }
    char s[128];
    snprintf(s, sizeof(s), "_%s", service);
    s[sizeof(s) - 1] = 0;
    SimpleMDNSService *svc = new SimpleMDNSService();
    _svcMap.insert({strdup(service), svc});
    struct netif *n = netif_list;
    while (n) {
        mdns_resp_add_service(n, _hostname, s, !strcasecmp("tcp", proto) ? DNSSD_PROTO_TCP : DNSSD_PROTO_UDP, port, SimpleMDNSService::callback, (void *)svc);
        n = n->next;
    }
    return (hMDNSService*) service;
}

void SimpleMDNS::update() {
    /* No-op */
}

void SimpleMDNS::end() {
    /* No-op */
}

void SimpleMDNS::_statusCB(struct netif *netif) {
    mdns_resp_netif_settings_changed(netif);
}

void SimpleMDNS::_addServiceTxt(struct mdns_service *service, const char *str) {
    mdns_resp_add_service_txtitem(service, str, strlen(str));
}

void SimpleMDNS::_arduinoGetTxt(struct mdns_service *service, void *txt_userdata) {
    _addServiceTxt(service, "tcp_check=no");
    _addServiceTxt(service, "ssh_upload=no");
    _addServiceTxt(service, "board=" ARDUINO_VARIANT);
    _addServiceTxt(service, (bool)txt_userdata ? "auth_upload=yes" : "auth_upload=no");
}


SimpleMDNSService::SimpleMDNSService() {
}

void SimpleMDNSService::callback(struct mdns_service *service, void *txt_userdata) {
    SimpleMDNSService *obj = (SimpleMDNSService *)txt_userdata;
    for (auto s : obj->txt) {
        mdns_resp_add_service_txtitem(service, s, strlen(s));
    }
}

hMDNSTxt SimpleMDNSService::add(const char *key, const char *value) {
    char s[128];
    snprintf(s, sizeof(s), "%s=%s", key, value);
    s[sizeof(s) - 1] = 0;
    char *copy = strdup(s);
    txt.push_back(copy);
    return (void *)copy; // Do not use...
};

// Add a (static) MDNS TXT item ('key' = 'value') to the service
hMDNSTxt SimpleMDNS::addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, const char* p_pcValue) {
    const char *s = (const char *)p_hService;
    auto o = _svcMap.find(s);
    if (o != _svcMap.end()) {
        return o->second->add(p_pcKey, p_pcValue);
    }
    return nullptr;
}

hMDNSTxt SimpleMDNS::addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, uint32_t p_u32Value) {
    char s[16];
    sprintf(s, "%lu", p_u32Value);
    return addServiceTxt(p_hService, p_pcKey, s);
}

hMDNSTxt SimpleMDNS::addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, uint16_t p_u16Value) {
    return addServiceTxt(p_hService, p_pcKey, (uint32_t)p_u16Value);
}

hMDNSTxt SimpleMDNS::addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, uint8_t p_u8Value) {
    return addServiceTxt(p_hService, p_pcKey, (uint32_t)p_u8Value);
}

hMDNSTxt SimpleMDNS::addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, int32_t p_i32Value) {
    char s[16];
    sprintf(s, "%ld", p_i32Value);
    return addServiceTxt(p_hService, p_pcKey, s);
}

hMDNSTxt SimpleMDNS::addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, int16_t p_i16Value) {
    return addServiceTxt(p_hService, p_pcKey, (int32_t)p_i16Value);
}

hMDNSTxt SimpleMDNS::addServiceTxt(const hMDNSService p_hService, const char* p_pcKey, int8_t p_i8Value) {
    return addServiceTxt(p_hService, p_pcKey, (int32_t)p_i8Value);
}


const char *SimpleMDNS::_hostname = nullptr;

SimpleMDNS MDNS;
