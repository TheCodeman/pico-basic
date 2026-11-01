/**
 * @file wifi.h
 * @brief Wi-Fi (the CYW43 on a Pico 2 W, a Pico W, or an RM2 breakout) and
 * the clock from NTP, set up from FLASH:/CONFIG.TXT. Wi-Fi builds only
 * (PICO_BASIC_WIFI).
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdbool.h>

/// Starts the radio and, with WIFI_SSID set (and WIFI not OFF), connects in
/// the background; the clock is set by NTP once it's up, and every hour.
void wifi_start(void);

/// The banner's line: "Wi-Fi: connected to HOME (192.168.1.20)" and so on.
void wifi_banner(void);

/// WIFI: the connection, its address and signal, and the clock.
void wifi_status(void);

/// WIFI ON (connect to CONFIG.TXT's network, again if need be) or WIFI OFF.
/// @param on connect
void wifi_switch(bool on);

/// WIFI SCAN: lists the networks around.
void wifi_scan(void);

/// NTP: asks for the time now and prints it (waits up to 5 seconds).
void wifi_ntp(void);

/// WIFI(n) for BASIC (see port_wifi_value).
/// @param n which
/// @return it
int wifi_value(int n);

/// WIFI$(n) for BASIC (see port_wifi_text).
/// @param n which
/// @return it
const char *wifi_text(int n);

/// Sets the board's LED (on the radio chip, on a W board).
/// @param on lit
void wifi_led(bool on);
