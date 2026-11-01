/**
 * @file lwipopts.h
 * @brief lwIP settings for Wi-Fi builds (pico_cyw43_arch_lwip_threadsafe_background:
 * lwIP runs in a low-priority interrupt, no RTOS). Only what NTP needs: DHCP,
 * DNS and UDP.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdint.h>

#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define MEM_LIBC_MALLOC             0  ///< malloc isn't safe from the lwIP interrupt
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000  ///< lwIP's own heap
#define MEMP_NUM_ARP_QUEUE          10
#define MEMP_NUM_SYS_TIMEOUT        (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 2)
#define PBUF_POOL_SIZE              8     ///< incoming packets, 1.5 KB each
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    0
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0
#define LWIP_TCP                    0
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_DHCP                   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0
#define SNTP_SERVER_DNS             1
#define SNTP_STARTUP_DELAY          0
#define SNTP_UPDATE_DELAY           3600000  ///< NTP again every hour
void wifi_set_time(uint32_t sec, uint32_t us);
#define SNTP_SET_SYSTEM_TIME_US(sec, us) wifi_set_time(sec, us)
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0
