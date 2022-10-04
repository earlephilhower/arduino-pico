/*
    LwipIntfDev.h

    Arduino network template class for generic device

    Original Copyright (c) 2020 esp8266 Arduino All rights reserved.
    This file is part of the esp8266 Arduino core environment.

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

// TODO:
// unchain pbufs

#include <LWIPMutex.h>

#include <netif/ethernet.h>
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/etharp.h>
#include <lwip/ethip6.h>
#include <lwip/dhcp.h>
#include <lwip/dhcp6.h>
#include <lwip/dns.h>
#include <lwip/raw.h>
#include <lwip/icmp.h>
#include <lwip/timeouts.h>
#include <lwip/inet_chksum.h>
#include <lwip/apps/sntp.h>

//#include <user_interface.h>  // wifi_get_macaddr()

#include "SPI.h"
//#include "Schedule.h"
#include "LwipIntf.h"
#include "wl_definitions.h"

#ifndef DEFAULT_MTU
#define DEFAULT_MTU 1500
#endif

extern "C" void cyw43_hal_generate_laa_mac(__unused int idx, uint8_t buf[6]);

template<class RawDev>
class LwipIntfDev: public LwipIntf, public RawDev {
public:
    LwipIntfDev(int8_t cs = SS, SPIClass& spi = SPI, int8_t intr = -1) :
        RawDev(cs, spi, intr), _mtu(DEFAULT_MTU), _intrPin(intr), _started(false), _default(false) {
        memset(&_netif, 0, sizeof(_netif));
    }

    bool config(const IPAddress& local_ip, const IPAddress& arg1, const IPAddress& arg2,
                const IPAddress& arg3 = IPADDR_NONE, const IPAddress& dns2 = IPADDR_NONE);

    // default mac-address is inferred from esp8266's STA interface
    bool begin(const uint8_t* macAddress = nullptr, const uint16_t mtu = DEFAULT_MTU);

    void end();

    netif* getNetIf() {
        return &_netif;
    }

    IPAddress localIP() const {
        return IPAddress(ip4_addr_get_u32(ip_2_ip4(&_netif.ip_addr)));
    }
    IPAddress subnetMask() const {
        return IPAddress(ip4_addr_get_u32(ip_2_ip4(&_netif.netmask)));
    }
    IPAddress gatewayIP() const {
        return IPAddress(ip4_addr_get_u32(ip_2_ip4(&_netif.gw)));
    }

    // 1. Currently when no default is set, esp8266-Arduino uses the first
    //    DHCP client interface receiving a valid address and gateway to
    //    become the new lwIP default interface.
    // 2. Otherwise - when using static addresses - lwIP for every packets by
    //    defaults selects automatically the best suited output interface
    //    matching the destination address.  If several interfaces match,
    //    the first one is picked.  On esp8266/Arduno: WiFi interfaces are
    //    checked first.
    // 3. Or, use `::setDefault(true)` to force using this interface's gateway
    //    as default router.
    void setDefault(bool deflt = true);

    // true if interface has a valid IPv4 address
    bool connected() {
        return !!ip4_addr_get_u32(ip_2_ip4(&_netif.ip_addr));
    }

    bool routable() {
        return !ip_addr_isany(&_netif.gw);
    }

    // ICMP echo, returns TTL
    int ping(IPAddress host, uint8_t ttl, uint32_t timeout = 5000);

    int hostByName(const char* aHostname, IPAddress& aResult, int timeout);

    // ESP8266WiFi API compatibility

    wl_status_t status();

protected:
    err_t netif_init();
    void  check_route();
    void  netif_status_callback();

    static err_t netif_init_s(netif* netif);
    static err_t linkoutput_s(netif* netif, struct pbuf* p);
    static void  netif_status_callback_s(netif* netif);

    // called on a regular basis or on interrupt
    err_t handlePackets();

    // members

    netif _netif;

    uint16_t _mtu;
    int8_t   _intrPin;
    uint8_t  _macAddress[6];
    bool     _started;
    bool     _default;

    // ICMP Ping
    int _ping_seq_num = 1;
    const int _ping_id = 0xfade;
    volatile int _ping_ttl;
    static u8_t _pingCB(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr);

    // DNS lookup callback
    bool _dns_lookup_pending = false;
    typedef struct {
        IPAddress *ip;
        LwipIntfDev<RawDev> *wifi;
    } _dns_cb_t;
    static void _dns_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
};


template<class RawDev>
void LwipIntfDev<RawDev>::_dns_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    (void) name;
    _dns_cb_t *cb = (_dns_cb_t *)callback_arg;
    if (!cb->wifi->_dns_lookup_pending) {
        return;
    }
    if (ipaddr) {
        *(cb->ip) = IPAddress(ipaddr);
    }
    cb->wifi->_dns_lookup_pending = false; // resume hostByName
}

template<class RawDev>
int LwipIntfDev<RawDev>::hostByName(const char* aHostname, IPAddress& aResult, int timeout_ms) {
    ip_addr_t addr;
    aResult = static_cast<uint32_t>(0xffffffff);

    if (aResult.fromString(aHostname)) {
        // Host name is a IP address use it!
        return 1;
    }

    LWIPMutex m;
    _dns_cb_t cb = { &aResult, this };
#if LWIP_IPV4 && LWIP_IPV6
    err_t err = dns_gethostbyname_addrtype(aHostname, &addr, &_dns_found_callback, &cb, LWIP_DNS_ADDRTYPE_DEFAULT);
#else
    err_t err = dns_gethostbyname(aHostname, &addr, &_dns_found_callback, &cb);
#endif
    if (err == ERR_OK) {
        aResult = IPAddress(&addr);
    } else if (err == ERR_INPROGRESS) {
        _dns_lookup_pending = true;
        uint32_t now = millis();
        while ((millis() - now < (uint32_t)timeout_ms) && _dns_lookup_pending) {
            sys_check_timeouts();
            delay(10);
        }
        _dns_lookup_pending = false;
        if (aResult.isSet()) {
            err = ERR_OK;
        }
    }

    if (err == ERR_OK) {
        return 1;
    }

    return 0;
}

template<class RawDev>
u8_t LwipIntfDev<RawDev>::_pingCB(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr) {
    (void) addr;
    LwipIntfDev<RawDev> *w = (LwipIntfDev<RawDev> *)arg;
    struct icmp_echo_hdr *iecho;
    if (pbuf_header(p, -20) == 0) {
        iecho = (struct icmp_echo_hdr *)p->payload;
        if ((iecho->id == w->_ping_id) && (iecho->seqno == htons(w->_ping_seq_num))) {
            w->_ping_ttl = pcb->ttl;
            pbuf_free(p);
            return 1; // We've processed it
        }
    }
    return 0; // Wasn't ours
}

template<class RawDev>
int LwipIntfDev<RawDev>::ping(IPAddress host, uint8_t ttl, uint32_t _timeout) {
    const int PING_DATA_SIZE = 32;
    struct pbuf *p;
    int ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

    auto ping_pcb = raw_new(IP_PROTO_ICMP);
    ping_pcb->ttl = ttl;

    LWIPMutex m;

    raw_recv(ping_pcb, _pingCB, this);
    raw_bind(ping_pcb, IP_ADDR_ANY);

    p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
    if (!p) {
        return 0;
    }
    if ((p->len == p->tot_len) && (p->next == nullptr)) {
        struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)p->payload;
        ICMPH_TYPE_SET(iecho, ICMP_ECHO);
        ICMPH_CODE_SET(iecho, 0);
        iecho->chksum = 0;
        iecho->id     = _ping_id;
        iecho->seqno  = htons(++_ping_seq_num);
        /* fill the additional data buffer with some data */
        for (size_t i = 0; i < PING_DATA_SIZE; i++) {
            ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)(i + 'A');
        }
        iecho->chksum = inet_chksum(iecho, ping_size);
        _ping_ttl = -1;
        raw_sendto(ping_pcb, p, host);
        uint32_t now = millis();
        while ((millis() - now < _timeout) && (_ping_ttl < 0)) {
            sys_check_timeouts();
            delay(10);
        }
        pbuf_free(p);
        raw_remove(ping_pcb);
        return _ping_ttl;
    } else {
        pbuf_free(p);
        raw_remove(ping_pcb);
        return -1;
    }
}

template<class RawDev>
bool LwipIntfDev<RawDev>::config(const IPAddress& localIP, const IPAddress& gateway,
                                 const IPAddress& netmask, const IPAddress& dns1,
                                 const IPAddress& dns2) {
    if (_started) {
        DEBUGV("LwipIntfDev: use config() then begin()\n");
        return false;
    }

    IPAddress realGateway, realNetmask, realDns1;
    if (!ipAddressReorder(localIP, gateway, netmask, dns1, realGateway, realNetmask, realDns1)) {
        return false;
    }
    ip4_addr_set_u32(ip_2_ip4(&_netif.ip_addr), localIP.v4());
    ip4_addr_set_u32(ip_2_ip4(&_netif.gw), realGateway.v4());
    ip4_addr_set_u32(ip_2_ip4(&_netif.netmask), realNetmask.v4());

    if (realDns1.isSet()) {
        // Set DNS1-Server
        dns_setserver(0, realDns1);
    }

    if (dns2.isSet()) {
        // Set DNS2-Server
        dns_setserver(1, dns2);
    }
    return true;
}
extern char wifi_station_hostname[];
template<class RawDev>
boolean LwipIntfDev<RawDev>::begin(const uint8_t* macAddress, const uint16_t mtu) {
    if (mtu) {
        _mtu = mtu;
    }

    if (macAddress) {
        memcpy(_macAddress, macAddress, 6);
    } else {
        _netif.num = 2;
        for (auto n = netif_list; n; n = n->next)
            if (n->num >= _netif.num) {
                _netif.num = n->num + 1;
            }

#if 1
        // forge a new mac-address from the esp's wifi sta one
        // I understand this is cheating with an official mac-address
        cyw43_hal_generate_laa_mac(0, _macAddress);
#else
        // https://serverfault.com/questions/40712/what-range-of-mac-addresses-can-i-safely-use-for-my-virtual-machines
        memset(_macAddress, 0, 6);
        _macAddress[0] = 0xEE;
#endif
        _macAddress[3] += _netif.num;  // alter base mac address
        _macAddress[0] &= 0xfe;        // set as locally administered, unicast, per
        _macAddress[0] |= 0x02;  // https://en.wikipedia.org/wiki/MAC_address#Universal_vs._local
    }

    // setup lwIP netif

    _netif.hwaddr_len = sizeof _macAddress;
    memcpy(_netif.hwaddr, _macAddress, sizeof _macAddress);

    // due to netif_add() api: ...
    ip_addr_t ip_addr, netmask, gw;
    ip_addr_copy(ip_addr, _netif.ip_addr);
    ip_addr_copy(netmask, _netif.netmask);
    ip_addr_copy(gw, _netif.gw);

    _netif.hostname = wifi_station_hostname;

    if (!netif_add(&_netif, ip_2_ip4(&ip_addr), ip_2_ip4(&netmask), ip_2_ip4(&gw), this,
                   netif_init_s, ethernet_input)) {
        return false;
    }

    if (!RawDev::begin(_macAddress, &_netif)) {
        return false;
    }

    if (localIP().v4() == 0) {
        // IP not set, starting DHCP
        _netif.flags |= NETIF_FLAG_UP;
        switch (dhcp_start(&_netif)) {
        case ERR_OK:
            break;

        case ERR_IF:
            return false;

        default:
            netif_remove(&_netif);
            return false;
        }
    } else {
        // IP is set, static config
        netif_set_link_up(&_netif);
        netif_set_up(&_netif);
    }

#if LWIP_IPV6
    netif_create_ip6_linklocal_address(&_netif, true);
#endif
#if LWIP_IPV6_DHCP6_STATELESS
    err_t __res = dhcp6_enable_stateless(&_netif);
    DEBUGV("LwipIntfDev: Enabled DHCP6 stateless: %d\n", __res);
#endif

    _started = true;

    if (_intrPin >= 0) {
        if (RawDev::interruptIsPossible()) {
            // attachInterrupt(_intrPin, [&]() { this->handlePackets(); }, FALLING);
        } else {
            ::printf((PGM_P)F(
                         "lwIP_Intf: Interrupt not implemented yet, enabling transparent polling\r\n"));
            _intrPin = -1;
        }
    }

#if 0
    if (_intrPin < 0
            && !schedule_recurrent_function_us(
    [&]() {
    this->handlePackets();
        return true;
    },
    100)) {
        netif_remove(&_netif);
        return false;
    }
#endif
    return true;
}

template<class RawDev>
void LwipIntfDev<RawDev>::end() {
    RawDev::end();
    netif_remove(&_netif);
    memset(&_netif, 0, sizeof(_netif));
}


template<class RawDev>
wl_status_t LwipIntfDev<RawDev>::status() {
    return _started ? (connected() ? WL_CONNECTED : WL_DISCONNECTED) : WL_NO_SHIELD;
}

template<class RawDev>
err_t LwipIntfDev<RawDev>::linkoutput_s(netif* netif, struct pbuf* pbuf) {
    LwipIntfDev* lid = (LwipIntfDev*)netif->state;

    if (pbuf->len != pbuf->tot_len || pbuf->next) {
        Serial.println("ERRTOT\r\n");
    }

    uint16_t len = lid->sendFrame((const uint8_t*)pbuf->payload, pbuf->len);

#if PHY_HAS_CAPTURE
    if (phy_capture) {
        phy_capture(lid->_netif.num, (const char*)pbuf->payload, pbuf->len, /*out*/ 1,
                    /*success*/ len == pbuf->len);
    }
#endif

    return len == pbuf->len ? ERR_OK : ERR_MEM;
}

template<class RawDev>
err_t LwipIntfDev<RawDev>::netif_init_s(struct netif* netif) {
    return ((LwipIntfDev*)netif->state)->netif_init();
}

template<class RawDev>
void LwipIntfDev<RawDev>::netif_status_callback_s(struct netif* netif) {
    ((LwipIntfDev*)netif->state)->netif_status_callback();
}

template<class RawDev>
err_t LwipIntfDev<RawDev>::netif_init() {
    _netif.name[0]      = 'e';
    _netif.name[1]      = '0' + _netif.num;
    _netif.mtu          = _mtu;
    _netif.chksum_flags = NETIF_CHECKSUM_ENABLE_ALL;
    _netif.flags = NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP | NETIF_FLAG_BROADCAST | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6_MLD
    _netif.flags |= NETIF_FLAG_MLD6;
#endif

    // lwIP's doc: This function typically first resolves the hardware
    // address, then sends the packet.  For ethernet physical layer, this is
    // usually lwIP's etharp_output()
    _netif.output = etharp_output;

#if LWIP_IPV6
    _netif.output_ip6 = ethip6_output;
#endif

    // lwIP's doc: This function outputs the pbuf as-is on the link medium
    // (this must points to the raw ethernet driver, meaning: us)
    _netif.linkoutput = linkoutput_s;

    _netif.status_callback = netif_status_callback_s;

    return ERR_OK;
}

template<class RawDev>
void LwipIntfDev<RawDev>::netif_status_callback() {
    check_route();
    if (connected()) {
        sntp_stop();
        sntp_init();
    }
}

template<class RawDev>
void LwipIntfDev<RawDev>::check_route() {
    if (connected()) {
        if (_default || (netif_default == nullptr && routable())) {
            // on user request,
            // or if there is no current default interface, but our gateway is valid
            netif_set_default(&_netif);
        }
    } else if (netif_default == &_netif) {
        netif_set_default(nullptr);
    }
}

template<class RawDev>
err_t LwipIntfDev<RawDev>::handlePackets() {
    int pkt = 0;
    while (1) {
        if (++pkt == 10)
            // prevent starvation
        {
            return ERR_OK;
        }

        uint16_t tot_len = RawDev::readFrameSize();
        if (!tot_len) {
            return ERR_OK;
        }

        // from doc: use PBUF_RAM for TX, PBUF_POOL from RX
        // however:
        // PBUF_POOL can return chained pbuf (not in one piece)
        // and WiznetDriver does not have the proper API to deal with that
        // so in the meantime, we use PBUF_RAM instead which is currently
        // guarantying to deliver a continuous chunk of memory.
        // TODO: tweak the wiznet driver to allow copying partial chunk
        //       of received data and use PBUF_POOL.
        pbuf* pbuf = pbuf_alloc(PBUF_RAW, tot_len, PBUF_RAM);
        if (!pbuf || pbuf->len < tot_len) {
            if (pbuf) {
                pbuf_free(pbuf);
            }
            RawDev::discardFrame(tot_len);
            return ERR_BUF;
        }

        uint16_t len = RawDev::readFrameData((uint8_t*)pbuf->payload, tot_len);
        if (len != tot_len) {
            // tot_len is given by readFrameSize()
            // and is supposed to be honoured by readFrameData()
            // todo: ensure this test is unneeded, remove the print
            pbuf_free(pbuf);
            return ERR_BUF;
        }

        err_t err = _netif.input(pbuf, &_netif);

#if PHY_HAS_CAPTURE
        if (phy_capture) {
            phy_capture(_netif.num, (const char*)pbuf->payload, tot_len, /*out*/ 0,
                        /*success*/ err == ERR_OK);
        }
#endif

        if (err != ERR_OK) {
            pbuf_free(pbuf);
            return err;
        }
        // (else) allocated pbuf is now lwIP's responsibility
    }
}

template<class RawDev>
void LwipIntfDev<RawDev>::setDefault(bool deflt) {
    _default = deflt;
    check_route();
}
