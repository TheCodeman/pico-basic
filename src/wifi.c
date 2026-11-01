/**
 * @file wifi.c
 * @brief Implementation of wifi.h: the CYW43 driver with lwIP in the
 * background (pico_cyw43_arch_lwip_threadsafe_background), a worker that
 * keeps the link up, and lwIP's SNTP client for the clock.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "wifi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#include "hardware/clocks.h"
#include "lwip/apps/sntp.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "pico/cyw43_arch.h"
#include "pico/cyw43_driver.h"
#include "pico/stdlib.h"

#define CHECK_MS 1000    ///< how often the worker looks at the link
#define RETRY_MS 30000   ///< how often a lost network is tried again
#define SCAN_MAX 24      ///< the most networks WIFI SCAN lists

static bool driver;          ///< cyw43_arch_init done
static bool radio;           ///< the radio answered
static volatile bool want;   ///< keep connected to CONFIG.TXT's network
static bool sntp_on;         ///< SNTP started (the link was up)
static volatile int synced;  ///< times NTP has set the clock
static volatile time_t synced_at;  ///< when it last did (UTC)
static absolute_time_t retry_at;   ///< when to try connecting again
static async_at_time_worker_t worker;  ///< keeps the link up

void wifi_set_time(uint32_t sec, uint32_t us) {
    // From lwIP's interrupt: settimeofday only stores an offset.
    struct timeval tv = {.tv_sec = (time_t)sec, .tv_usec = (suseconds_t)us};
    settimeofday(&tv, NULL);
    synced_at = (time_t)sec;
    ++synced;
}

/// The station interface.
/// @return it
static struct netif *sta(void) {
    return &cyw43_state.netif[CYW43_ITF_STA];
}

/// The link's status.
/// @return CYW43_LINK_*
static int link_status(void) {
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
}

/// Why connecting failed.
/// @param s the link status
/// @return the reason
static const char *why(int s) {
    switch (s) {
    case CYW43_LINK_BADAUTH: return "wrong password";
    case CYW43_LINK_NONET: return "network not found";
    case CYW43_LINK_FAIL: return "failed";
    default: return "no answer";
    }
}

/// Starts connecting to CONFIG.TXT's network (in lwIP's context).
static void start_connect(void) {
    const char *pw = config.wifi_password;
    cyw43_arch_wifi_connect_async(config.wifi_ssid, pw[0] ? pw : NULL,
                                  pw[0] ? CYW43_AUTH_WPA2_AES_PSK : CYW43_AUTH_OPEN);
    retry_at = make_timeout_time_ms(RETRY_MS);
}

/// Starts or stops SNTP with the link (in lwIP's context).
/// @param up the link is up
static void sntp_follow(bool up) {
    if (up && !sntp_on) {
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, config.ntp_server);
        sntp_init();
        sntp_on = true;
    } else if (!up && sntp_on) {
        sntp_stop();
        sntp_on = false;
    }
}

/// Every second, in the background: SNTP follows the link, and a network
/// lost (or not found) is tried again every 30 seconds.
/// @param context the async context
/// @param w this worker
static void check_link(async_context_t *context, async_at_time_worker_t *w) {
    int s = link_status();
    sntp_follow(s == CYW43_LINK_UP);
    if (want && s != CYW43_LINK_UP && s != CYW43_LINK_JOIN && s != CYW43_LINK_NOIP &&
        s != CYW43_LINK_BADAUTH && time_reached(retry_at))
        start_connect();
    async_context_add_at_time_worker_in_ms(context, w, CHECK_MS);
}

/// The bus clock dividers to try (for 125 MHz; scaled to the system clock):
/// the SDK's first, then slower ones in case the wiring is marginal.
static const uint8_t clock_divs[] = {2, 3, 4, 6};

/// Starts the driver and powers the radio up, once.
/// @return it's ready
static bool start_radio(void) {
    if (radio)
        return true;
    if (!driver) {
        const char *c = config.wifi_country;
        if (cyw43_arch_init_with_country(CYW43_COUNTRY(c[0], c[1], 0)) != 0)
            return false;
        driver = true;
    }
    for (unsigned i = 0; i < sizeof clock_divs && !radio; ++i) {
#if CYW43_PIO_CLOCK_DIV_DYNAMIC
        unsigned mhz = (clock_get_hz(clk_sys) + 500000) / 1000000;
        cyw43_set_pio_clkdiv_int_frac8((clock_divs[i] * mhz + 124) / 125, 0);
#endif
        uint32_t pm;
        cyw43_arch_lwip_begin();
        radio = cyw43_wifi_get_pm(&cyw43_state, &pm) == 0;  // runs the power-up
        cyw43_arch_lwip_end();
    }
    if (!radio)
        return false;
    cyw43_arch_enable_sta_mode();
    worker.do_work = check_link;
    async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &worker, CHECK_MS);
    return true;
}

void wifi_start(void) {
    if (!config.wifi || !config.wifi_ssid[0])
        return;
    if (!start_radio())
        return;
    cyw43_arch_lwip_begin();
    want = true;
    start_connect();
    cyw43_arch_lwip_end();
}

/// The local time as text: "2026-10-31 18:30:05 EDT".
/// @param t the time
/// @param out the text
/// @param size room in out
static void local_text(time_t t, char *out, size_t size) {
    struct tm tm;
    localtime_r(&t, &tm);
    if (config.clock12) {  // CLOCK = 12: "2026-10-31 6:30:05 PM EDT"
        size_t n = strftime(out, size, "%Y-%m-%d ", &tm);
        char zone[16];
        strftime(zone, sizeof zone, "%Z", &tm);
        snprintf(out + n, size - n, "%d:%02d:%02d %s %s", tm.tm_hour % 12 ? tm.tm_hour % 12 : 12,
                 tm.tm_min, tm.tm_sec, tm.tm_hour < 12 ? "AM" : "PM", zone);
    } else {
        strftime(out, size, "%Y-%m-%d %H:%M:%S %Z", &tm);
    }
}

void wifi_banner(void) {
    if (!config.wifi_ssid[0])
        return;
    if (!config.wifi) {
        printf("Wi-Fi: off (WIFI = OFF in CONFIG.TXT)\n");
    } else if (!radio) {
        printf("Wi-Fi: the radio didn't start\n");
    } else {
        int s = link_status();
        if (s == CYW43_LINK_UP)
            printf("Wi-Fi: connected to %s (%s)\n", config.wifi_ssid,
                   ip4addr_ntoa(netif_ip4_addr(sta())));
        else if (s < 0)
            printf("Wi-Fi: can't connect to %s: %s\n", config.wifi_ssid, why(s));
        else
            printf("Wi-Fi: connecting to %s (WIFI shows how it's going)\n", config.wifi_ssid);
    }
}

void wifi_status(void) {
    if (!config.wifi_ssid[0] && !want) {
        printf("No network: set WIFI_SSID (and WIFI_PASSWORD) in FLASH:CONFIG.TXT\n");
    } else if (!radio) {
        printf(config.wifi ? "The radio didn't start\n" : "Off (WIFI ON connects)\n");
    } else {
        int s = link_status();
        if (s == CYW43_LINK_UP) {
            int32_t rssi = 0;
            cyw43_wifi_get_rssi(&cyw43_state, &rssi);
            printf("Connected to %s, signal %ld dBm\n", config.wifi_ssid, (long)rssi);
            printf("IP %s", ip4addr_ntoa(netif_ip4_addr(sta())));
            printf(", gateway %s", ip4addr_ntoa(netif_ip4_gw(sta())));
            printf(", DNS %s\n", ipaddr_ntoa(dns_getserver(0)));
        } else if (!want) {
            printf("Not connected (WIFI ON connects)\n");
        } else if (s >= 0) {
            printf("Connecting to %s...\n", config.wifi_ssid);
        } else {
            printf("Not connected: %s: %s%s\n", config.wifi_ssid, why(s),
                   s == CYW43_LINK_BADAUTH ? " (fix WIFI_PASSWORD, then WIFI ON)" : ", retrying");
        }
    }
    char t[40];
    local_text(time(NULL), t, sizeof t);
    if (synced) {
        char at[40];
        local_text(synced_at, at, sizeof at);
        printf("Clock: %s, from %s at %s\n", t, config.ntp_server, at);
    } else {
        printf("Clock: %s (not set by NTP yet)\n", t);
    }
}

void wifi_switch(bool on) {
    if (on) {
        if (!config.wifi_ssid[0]) {
            printf("No network: set WIFI_SSID (and WIFI_PASSWORD) in FLASH:CONFIG.TXT\n");
            return;
        }
        if (!start_radio()) {
            printf("The radio didn't start\n");
            return;
        }
        cyw43_arch_lwip_begin();
        want = true;
        start_connect();
        cyw43_arch_lwip_end();
        printf("Connecting to %s\n", config.wifi_ssid);
    } else if (radio) {
        cyw43_arch_lwip_begin();
        want = false;
        sntp_follow(false);
        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
        cyw43_arch_lwip_end();
    }
}

static struct {
    char ssid[33];
    int16_t rssi;     ///< its signal (dBm)
    uint8_t auth;     ///< its security
    uint8_t channel;  ///< its channel
} found[SCAN_MAX];    ///< the networks the scan found
static volatile int found_count;  ///< their count

/// The scan's callback: adds a network, or keeps its strongest signal.
/// @param env unused
/// @param r the result
/// @return 0
static int on_scan(void *env, const cyw43_ev_scan_result_t *r) {
    (void)env;
    if (!r->ssid_len)
        return 0;  // a hidden network
    char name[33];
    memcpy(name, r->ssid, r->ssid_len);
    name[r->ssid_len] = 0;
    int i = 0;
    while (i < found_count && strcmp(found[i].ssid, name))
        ++i;
    if (i == found_count) {
        if (found_count == SCAN_MAX)
            return 0;
        strcpy(found[i].ssid, name);
        found[i].rssi = -999;
        found_count = i + 1;
    }
    if (r->rssi > found[i].rssi) {
        found[i].rssi = r->rssi;
        found[i].auth = r->auth_mode;
        found[i].channel = r->channel;
    }
    return 0;
}

/// qsort's comparison: the strongest signal first.
/// @param a a network
/// @param b another
/// @return less, equal or more than 0
static int by_signal(const void *a, const void *b) {
    return ((const typeof(found[0]) *)b)->rssi - ((const typeof(found[0]) *)a)->rssi;
}

void wifi_scan(void) {
    if (!start_radio()) {
        printf("The radio didn't start\n");
        return;
    }
    found_count = 0;
    cyw43_wifi_scan_options_t opts = {0};
    cyw43_arch_lwip_begin();
    int e = cyw43_wifi_scan(&cyw43_state, &opts, NULL, on_scan);
    cyw43_arch_lwip_end();
    if (e) {
        printf("The scan failed (%d)\n", e);
        return;
    }
    absolute_time_t until = make_timeout_time_ms(10000);
    while (cyw43_wifi_scan_active(&cyw43_state) && !time_reached(until))
        sleep_ms(50);
    qsort(found, found_count, sizeof found[0], by_signal);
    printf("%-32s  Signal   Ch  Security\n", "Network");
    for (int i = 0; i < found_count; ++i) {
        uint8_t a = found[i].auth;
        printf("%-32s  %4d dBm %2u  %s\n", found[i].ssid, found[i].rssi, found[i].channel,
               a == 0 ? "open" : a & 4 ? "WPA2" : a & 2 ? "WPA" : "WEP");
    }
    if (!found_count)
        printf("No networks found\n");
}

void wifi_ntp(void) {
    if (!radio || link_status() != CYW43_LINK_UP) {
        printf("Wi-Fi isn't connected (WIFI says why)\n");
        return;
    }
    int before = synced;
    cyw43_arch_lwip_begin();
    sntp_follow(false);  // starting again asks at once
    sntp_follow(true);
    cyw43_arch_lwip_end();
    absolute_time_t until = make_timeout_time_ms(5000);
    while (synced == before && !time_reached(until))
        sleep_ms(20);
    char t[40];
    local_text(time(NULL), t, sizeof t);
    if (synced == before)
        printf("No answer from %s; the clock says %s\n", config.ntp_server, t);
    else
        printf("%s (from %s)\n", t, config.ntp_server);
}

void wifi_led(bool on) {
    if (start_radio() || driver)
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}
