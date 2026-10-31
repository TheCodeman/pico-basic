/**
 * @file config.h
 * @brief FLASH:/CONFIG.TXT: the board's settings, read at power-up.
 *
 * KEY = value lines (any case); # ' or ; start a comment. Without the file
 * the build's defaults hold (the Cytron Maker Pi Pico's pins). Changes take
 * effect at the next power-up or reset.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdbool.h>

#define CONFIG_PATH "FLASH:/CONFIG.TXT"  ///< where the settings are
#define CONFIG_PULLUPS 4  ///< extra pins SD_PULLUPS can hold up

/// The settings in force.
typedef struct {
    bool sd;          ///< the SD card is used
    int sd_spi;       ///< its SPI controller, 0 or 1 (from the pins)
    int sd_sck, sd_mosi, sd_miso, sd_cs;  ///< its pins
    int sd_mode;      ///< SPI mode 0-3
    int sd_pullups[CONFIG_PULLUPS];  ///< pins held up while it is used (DAT1, DAT2)
    int sd_npullups;  ///< how many
    int speaker;      ///< BEEP, SOUND and PLAY's pin
    int neopixel;     ///< NEOPIXEL OPEN's default pin
    bool from_file;   ///< CONFIG.TXT was read
    bool sd_bad;      ///< the SD card is off because its pins were wrong
} config_t;

extern config_t config;  ///< the settings in force

/// Sets the build's defaults, then reads CONFIG.TXT if it is there. Problems
/// are kept to be shown with the banner (config_warnings).
void config_load(void);

/// Prints the problems CONFIG.TXT had, if any (one line each).
void config_warnings(void);

/// Prints the settings in force (the CONFIG command).
void config_print(void);
