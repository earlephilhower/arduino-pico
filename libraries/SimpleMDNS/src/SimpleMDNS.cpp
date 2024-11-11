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
    if (!_running) {
        return;
    }
    struct netif *n = netif_list;
    while (n) {
        mdns_resp_add_service(n, _hostname, "_arduino", DNSSD_PROTO_TCP, port, _arduinoGetTxt, (void *)passwd);
        n = n->next;
    }
}

void SimpleMDNS::addService(const char *service, const char *proto, unsigned int port) {
    if (!_running) {
        return;
    }
    char s[128];
    snprintf(s, sizeof(s), "_%s", service);
    s[sizeof(s) - 1] = 0;
    struct netif *n = netif_list;
    while (n) {
        mdns_resp_add_service(n, _hostname, s, !strcasecmp("tcp", proto) ? DNSSD_PROTO_TCP : DNSSD_PROTO_UDP, port, _nullGetTxt, nullptr);
        n = n->next;
    }
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

void SimpleMDNS::_nullGetTxt(struct mdns_service *service, void *txt_userdata) {
    /* nop */
}

const char *SimpleMDNS::_hostname = nullptr;

SimpleMDNS MDNS;
