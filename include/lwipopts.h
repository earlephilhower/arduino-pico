#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Critical section protection
extern void noInterrupts();
extern void interrupts();
#define SYS_ARCH_DECL_PROTECT int
#define SYS_ARCH_PROTECT(lev) {(void) lev; noInterrupts();}
#define SYS_ARCH_UNPROTECT(lev) {(void) lev; interrupts();}

#ifndef DEBUG_RP2040_PORT
extern void panic(const char *fmt, ...);
#define LWIP_PLATFORM_ASSERT(x) panic("lwip")
#endif

extern unsigned long __lwip_rand(void);
#define LWIP_RAND() __lwip_rand()

#ifndef __LWIP_MEMMULT
#define __LWIP_MEMMULT 1
#endif

// Common settings used in most of the pico_w examples
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html for details)

#define NO_SYS                        1
#define LWIP_SOCKET                   0
#define MEM_LIBC_MALLOC               0

#define MEM_ALIGNMENT                 4
#define MEM_SIZE                      (__LWIP_MEMMULT * 16384)
#define MEMP_NUM_TCP_SEG              (32)
#define MEMP_NUM_ARP_QUEUE            (10)
#define PBUF_POOL_SIZE                (__LWIP_MEMMULT > 1 ? 32 : 24)
#define LWIP_ARP                      7
#define LWIP_ETHERNET                 1
#define LWIP_ICMP                     1
#define LWIP_RAW                      1
#define TCP_WND                       (8 * TCP_MSS)
#define TCP_MSS                       1460
#define TCP_SND_BUF                   (8 * TCP_MSS)
#define TCP_SND_QUEUELEN              ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define TCP_LISTEN_BACKLOG            1
#define TCP_DEFAULT_LISTEN_BACKLOG    (2 * __LWIP_MEMMULT)
#define LWIP_NETIF_STATUS_CALLBACK    1
#define LWIP_NETIF_LINK_CALLBACK      1
#define LWIP_NETIF_HOSTNAME           1
#define LWIP_NUM_NETIF_CLIENT_DATA    5
#define LWIP_NETCONN                  0
#define LWIP_CHKSUM_ALGORITHM         0
#define LWIP_DHCP                     1
#define LWIP_IPV4                     1
#define LWIP_TCP                      1
#define LWIP_UDP                      1
#define LWIP_DNS                      1
#define LWIP_DNS_SUPPORT_MDNS_QUERIES 1
#define LWIP_TCP_KEEPALIVE            1
#define LWIP_NETIF_TX_SINGLE_PBUF     1
#define DHCP_DOES_ARP_CHECK           0
#define LWIP_DHCP_DOES_ACD_CHECK      0
#define LWIP_IGMP                     1
#define LWIP_MDNS_RESPONDER           1
#define MDNS_MAX_SERVICES             4

// See #1285
#define MEMP_NUM_UDP_PCB              (__LWIP_MEMMULT * 7)
#define MEMP_NUM_TCP_PCB              (__LWIP_MEMMULT * 5)

#if LWIP_IPV6
#define LWIP_IPV6_DHCP6               1
#define LWIP_IPV6_MLD                 1
#endif

// NTP
extern void __setSystemTime(unsigned long long sec, unsigned long us);
#define SNTP_SET_SYSTEM_TIME_US(sec, us)  __setSystemTime(sec, us)
#define SNTP_MAX_SERVERS                  2
//#define SNTP_SERVER_ADDRESS               "pool.ntp.org"
#define SNTP_SERVER_DNS                   1

#ifndef LWIP_DEBUG
#define LWIP_STATS                    0
#define LWIP_STATS_DISPLAY            0
#define MEM_STATS                     0
#define SYS_STATS                     0
#define MEMP_STATS                    0
#define LINK_STATS                    0
#else
#define LWIP_STATS                    1
#define LWIP_STATS_DISPLAY            1
#define MEM_STATS                     1
#define SYS_STATS                     1
#define MEMP_STATS                    1
#define LINK_STATS                    1
#define ETHARP_DEBUG                  LWIP_DBG_ON
#define NETIF_DEBUG                   LWIP_DBG_ON
#define PBUF_DEBUG                    LWIP_DBG_ON
#define API_LIB_DEBUG                 LWIP_DBG_ON
#define API_MSG_DEBUG                 LWIP_DBG_ON
#define SOCKETS_DEBUG                 LWIP_DBG_ON
#define ICMP_DEBUG                    LWIP_DBG_ON
#define INET_DEBUG                    LWIP_DBG_ON
#define IP_DEBUG                      LWIP_DBG_ON
#define IP_REASS_DEBUG                LWIP_DBG_ON
#define RAW_DEBUG                     LWIP_DBG_ON
#define MEM_DEBUG                     LWIP_DBG_ON
#define MEMP_DEBUG                    LWIP_DBG_ON
#define SYS_DEBUG                     LWIP_DBG_ON
#define TCP_DEBUG                     LWIP_DBG_ON
#define TCP_INPUT_DEBUG               LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG              LWIP_DBG_ON
#define TCP_RTO_DEBUG                 LWIP_DBG_ON
#define TCP_CWND_DEBUG                LWIP_DBG_ON
#define TCP_WND_DEBUG                 LWIP_DBG_ON
#define TCP_FR_DEBUG                  LWIP_DBG_ON
#define TCP_QLEN_DEBUG                LWIP_DBG_ON
#define TCP_RST_DEBUG                 LWIP_DBG_ON
#define UDP_DEBUG                     LWIP_DBG_ON
#define TCPIP_DEBUG                   LWIP_DBG_ON
#define PPP_DEBUG                     LWIP_DBG_ON
#define SLIP_DEBUG                    LWIP_DBG_ON
#define DHCP_DEBUG                    LWIP_DBG_ON
#endif


#ifdef __cplusplus
}
#endif // __cplusplus
