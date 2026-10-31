/**
 * @file config.c
 * @brief Implementation of config.h.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ff.h"
#include "hardware/gpio.h"

// The build's defaults (CMakeLists.txt): the Maker Pi Pico's pins.
#ifndef PICO_BASIC_SD
#define PICO_BASIC_SD 1
#endif
#ifndef SD_SCK
#define SD_SCK 10
#endif
#ifndef SD_MOSI
#define SD_MOSI 11
#endif
#ifndef SD_MISO
#define SD_MISO 12
#endif
#ifndef SD_CS
#define SD_CS 15
#endif
#ifndef SD_DAT1
#define SD_DAT1 13
#endif
#ifndef SD_DAT2
#define SD_DAT2 14
#endif
#ifndef SOUND_PIN
#define SOUND_PIN 18
#endif
#ifndef NEOPIXEL_PIN
#define NEOPIXEL_PIN 28
#endif

config_t config;

#define WARNINGS 6  ///< problems kept to show
static char warnings[WARNINGS][96];
static int nwarnings;

/// Keeps a problem to show with the banner.
/// @param fmt printf style
static void warn(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void warn(const char *fmt, ...) {
    if (nwarnings == WARNINGS)
        return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(warnings[nwarnings++], sizeof warnings[0], fmt, ap);
    va_end(ap);
}

/// A pin number value; -1 for "-1" or "none"; -2 if it isn't one.
/// @param v the value
/// @return the pin
static int pin_value(const char *v) {
    if (!strcasecmp(v, "none") || !strcmp(v, "-1"))
        return -1;
    if (toupper((unsigned char)v[0]) == 'G' && toupper((unsigned char)v[1]) == 'P')
        v += 2;
    char *end;
    long n = strtol(v, &end, 10);
    if (end == v || *end || n < 0 || n >= NUM_BANK0_GPIOS)
        return -2;
    return (int)n;
}

/// Checks the SD card's pins: SCK, MOSI and MISO of one SPI controller
/// (SCK GP2/6/10/14..., MOSI 3/7/11/15..., MISO 0/4/8/12..., SPI0 or SPI1 by
/// (pin / 8) mod 2), CS any other pin. Turns the card off if they aren't.
static void check_sd_pins(void) {
    const char *why = NULL;
    int spi = (config.sd_sck / 8) & 1;
    if (config.sd_sck % 4 != 2)
        why = "SD_SCK must be an SPI clock pin (GP2, 6, 10, 14, 18, 22...)";
    else if (config.sd_mosi % 4 != 3 || ((config.sd_mosi / 8) & 1) != spi)
        why = "SD_MOSI must be SD_SCK's SPI's TX pin (GP3, 7, 11, 15, 19...)";
    else if (config.sd_miso % 4 != 0 || ((config.sd_miso / 8) & 1) != spi)
        why = "SD_MISO must be SD_SCK's SPI's RX pin (GP0, 4, 8, 12, 16...)";
    else if (config.sd_cs < 0 || config.sd_cs == config.sd_sck || config.sd_cs == config.sd_mosi ||
             config.sd_cs == config.sd_miso)
        why = "SD_CS must be a pin of its own";
    if (why) {
        warn("CONFIG.TXT: %s; SD card off", why);
        config.sd = false;
        config.sd_bad = true;
    }
    config.sd_spi = spi;
}

/// Sets one setting from a KEY = value line.
/// @param line the line's number (for messages)
/// @param key the key
/// @param v its value
static void set(int line, const char *key, const char *v) {
    int *pin = !strcasecmp(key, "SD_SCK")    ? &config.sd_sck
               : !strcasecmp(key, "SD_MOSI") ? &config.sd_mosi
               : !strcasecmp(key, "SD_MISO") ? &config.sd_miso
               : !strcasecmp(key, "SD_CS")   ? &config.sd_cs
               : !strcasecmp(key, "SPEAKER") ? &config.speaker
               : !strcasecmp(key, "NEOPIXEL") ? &config.neopixel
                                             : NULL;
    if (pin) {
        int p = pin_value(v);
        if (p == -2 || (p == -1 && pin != &config.speaker))
            warn("CONFIG.TXT line %d: %s = %s isn't a pin", line, key, v);
        else
            *pin = p;
    } else if (!strcasecmp(key, "SD")) {
        if (!strcasecmp(v, "ON") || !strcasecmp(v, "YES") || !strcmp(v, "1"))
            config.sd = true;
        else if (!strcasecmp(v, "OFF") || !strcasecmp(v, "NO") || !strcmp(v, "0"))
            config.sd = false;
        else
            warn("CONFIG.TXT line %d: SD = %s: ON or OFF", line, v);
    } else if (!strcasecmp(key, "SD_MODE")) {
        if (strlen(v) == 1 && v[0] >= '0' && v[0] <= '3')
            config.sd_mode = v[0] - '0';
        else
            warn("CONFIG.TXT line %d: SD_MODE = %s: 0 to 3", line, v);
    } else if (!strcasecmp(key, "SD_PULLUPS")) {
        config.sd_npullups = 0;
        char buf[64];
        snprintf(buf, sizeof buf, "%s", v);
        for (char *t = strtok(buf, ", "); t; t = strtok(NULL, ", ")) {
            int p = pin_value(t);
            if (p == -1)
                continue;  // "none"
            if (p < 0 || config.sd_npullups == CONFIG_PULLUPS)
                warn("CONFIG.TXT line %d: SD_PULLUPS: %s isn't a pin (or too many)", line, t);
            else
                config.sd_pullups[config.sd_npullups++] = p;
        }
    } else {
        warn("CONFIG.TXT line %d: %s isn't a setting (ignored)", line, key);
    }
}

void config_load(void) {
    config = (config_t){
        .sd = PICO_BASIC_SD, .sd_sck = SD_SCK, .sd_mosi = SD_MOSI, .sd_miso = SD_MISO,
        .sd_cs = SD_CS, .sd_mode = 3, .speaker = SOUND_PIN, .neopixel = NEOPIXEL_PIN,
    };
    if (SD_DAT1 >= 0)
        config.sd_pullups[config.sd_npullups++] = SD_DAT1;
    if (SD_DAT2 >= 0)
        config.sd_pullups[config.sd_npullups++] = SD_DAT2;
    nwarnings = 0;
    FIL f;
    if (f_open(&f, CONFIG_PATH, FA_READ) == FR_OK) {
        config.from_file = true;
        char line[128];
        for (int n = 1; f_gets(line, sizeof line, &f); ++n) {
            char *c = strpbrk(line, "#';\r\n");  // the comment, or the end
            if (c)
                *c = 0;
            char *eq = strchr(line, '=');
            char *key = line;
            while (isspace((unsigned char)*key))
                ++key;
            if (!*key)
                continue;  // blank or a comment
            if (!eq) {
                warn("CONFIG.TXT line %d: KEY = value, please", n);
                continue;
            }
            *eq = 0;
            char *v = eq + 1, *e = eq;
            while (e > key && isspace((unsigned char)e[-1]))
                *--e = 0;
            while (isspace((unsigned char)*v))
                ++v;
            for (e = v + strlen(v); e > v && isspace((unsigned char)e[-1]);)
                *--e = 0;
            set(n, key, v);
        }
        f_close(&f);
    }
    if (config.sd)
        check_sd_pins();
    if (config.speaker >= 0 && config.sd &&
        (config.speaker == config.sd_sck || config.speaker == config.sd_mosi ||
         config.speaker == config.sd_miso || config.speaker == config.sd_cs)) {
        warn("CONFIG.TXT: SPEAKER is one of the SD card's pins; no speaker");
        config.speaker = -1;
    }
}

void config_warnings(void) {
    for (int i = 0; i < nwarnings; ++i)
        printf("%s\n", warnings[i]);
}

void config_print(void) {
    printf("Settings (%s):\n", config.from_file ? CONFIG_PATH : "the build's; no " CONFIG_PATH);
    if (config.sd) {
        printf("  SD = ON: SPI%d, SCK GP%d, MOSI GP%d, MISO GP%d, CS GP%d, mode %d", config.sd_spi,
               config.sd_sck, config.sd_mosi, config.sd_miso, config.sd_cs, config.sd_mode);
        for (int i = 0; i < config.sd_npullups; ++i)
            printf("%s GP%d", i ? "," : "; pull-ups", config.sd_pullups[i]);
        printf("\n");
    } else {
        printf("  SD = OFF\n");
    }
    if (config.speaker >= 0)
        printf("  SPEAKER = GP%d\n", config.speaker);
    else
        printf("  SPEAKER = none\n");
    printf("  NEOPIXEL = GP%d (NEOPIXEL OPEN's default)\n", config.neopixel);
    config_warnings();
}
