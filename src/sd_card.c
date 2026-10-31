/**
 * @file sd_card.c
 * @brief Implementation of sd_card.h: SD cards in SPI mode.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "sd_card.h"

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#ifndef SD_SPI_KHZ
#define SD_SPI_KHZ 25000  ///< the clock once the card is set up
#endif

/// The controller.
#define SPI (config.sd_spi ? spi1 : spi0)

/// The card's state.
static enum { NONE, READY } state;
/// SDHC/SDXC: sectors are addressed by number (SDSC: by byte).
static bool block_addr;
/// The pins are set up for the card.
static bool pins_on;
/// The clock in use once the card is set up; halved when a transfer fails.
static uint32_t speed_khz = SD_SPI_KHZ;

static FATFS fs;  ///< the mounted card
/// Why the last transfer failed, for sd_card_mount's message.
static char last_error[64];

// ---- SPI bytes ------------------------------------------------------------------

/// Sends one byte and returns the one that came back.
/// @param b the byte
/// @return the byte read
static uint8_t xfer(uint8_t b) {
    uint8_t r;
    spi_write_read_blocking(SPI, &b, &r, 1);
    return r;
}

/// Selects the card (CS low).
static void select(void) {
    gpio_put(config.sd_cs, 0);
    xfer(0xff);
}

/// Deselects it (CS high, then a byte so it lets go of DO).
static void deselect(void) {
    gpio_put(config.sd_cs, 1);
    xfer(0xff);
}

/// Waits until the card isn't busy (it sends FF).
/// @param ms how long at most
/// @return it is ready
static bool wait_ready(uint32_t ms) {
    absolute_time_t until = make_timeout_time_ms(ms);
    do {
        if (xfer(0xff) == 0xff)
            return true;
    } while (!time_reached(until));
    return false;
}

/// Sends a command and returns its R1 response (bit 7 set: none came).
/// @param cmd the command (ACMDs: with 0x80 added)
/// @param arg its argument
/// @return R1
static uint8_t command(uint8_t cmd, uint32_t arg) {
    if (cmd & 0x80) {  // ACMD: CMD55 first
        cmd &= 0x7f;
        uint8_t r = command(55, 0);
        if (r > 1)
            return r;
    }
    deselect();
    select();
    if (cmd != 0 && !wait_ready(500))
        return 0xff;
    uint8_t frame[6] = {(uint8_t)(0x40 | cmd), (uint8_t)(arg >> 24), (uint8_t)(arg >> 16),
                        (uint8_t)(arg >> 8), (uint8_t)arg,
                        cmd == 0 ? 0x95 : cmd == 8 ? 0x87 : 0x01};  // CRCs CMD0 and CMD8 need
    spi_write_blocking(SPI, frame, sizeof frame);
    if (cmd == 12)
        xfer(0xff);  // a stuff byte after STOP_TRANSMISSION
    uint8_t r;
    int n = 10;
    do
        r = xfer(0xff);
    while ((r & 0x80) && --n);
    return r;
}

/// Reads a data block of `len` bytes after its start token.
/// @param buf where
/// @param len how many
/// @return it came
static bool read_data(uint8_t *buf, unsigned len) {
    absolute_time_t until = make_timeout_time_ms(200);
    uint8_t token;
    do
        token = xfer(0xff);
    while (token == 0xff && !time_reached(until));
    if (token != 0xfe) {
        snprintf(last_error, sizeof last_error, "data token %02x at %lu kHz", token,
                 (unsigned long)speed_khz);
        return false;
    }
    spi_read_blocking(SPI, 0xff, buf, len);
    xfer(0xff);  // the CRC, not checked
    xfer(0xff);
    return true;
}

/// Sends a data block (token 0xFE or 0xFC), or just the stop token (0xFD).
/// @param buf the 512 bytes (NULL with the stop token)
/// @param token the token
/// @return the card took it
static bool write_data(const uint8_t *buf, uint8_t token) {
    if (!wait_ready(500))
        return false;
    xfer(token);
    if (token == 0xfd)
        return true;
    spi_write_blocking(SPI, buf, 512);
    xfer(0xff);  // a dummy CRC
    xfer(0xff);
    return (xfer(0xff) & 0x1f) == 0x05;  // data accepted
}

// ---- Setting the card up ---------------------------------------------------------

/// The card's pins: SPI, and CS as a plain output. Or back to inputs.
/// @param on set them up (else free them)
static void pins(bool on) {
    if (on == pins_on)
        return;
    if (on) {
        spi_init(SPI, 400 * 1000);
        // Mode 3 (clock idle high, sampled on the rising edge): cards take
        // modes 0 and 3, and on the Maker Pi Pico (an indicator LED on each
        // line) only mode 3 reads cleanly.
        spi_set_format(SPI, 8, config.sd_mode & 2 ? SPI_CPOL_1 : SPI_CPOL_0,
                       config.sd_mode & 1 ? SPI_CPHA_1 : SPI_CPHA_0, SPI_MSB_FIRST);
        gpio_set_function(config.sd_sck, GPIO_FUNC_SPI);
        gpio_set_function(config.sd_mosi, GPIO_FUNC_SPI);
        gpio_set_function(config.sd_miso, GPIO_FUNC_SPI);
        gpio_pull_up(config.sd_miso);  // DO floats while the card is deselected
        gpio_init(config.sd_cs);
        gpio_put(config.sd_cs, 1);
        gpio_set_dir(config.sd_cs, GPIO_OUT);
        // The card's other data lines aren't used in SPI mode, but left
        // floating some cards won't answer: hold them up.
        for (int i = 0; i < config.sd_npullups; ++i) {
            gpio_init(config.sd_pullups[i]);
            gpio_pull_up(config.sd_pullups[i]);
        }
    } else {
        spi_deinit(SPI);
        gpio_init(config.sd_sck);
        gpio_init(config.sd_mosi);
        gpio_init(config.sd_miso);
        gpio_init(config.sd_cs);
        gpio_set_pulls(config.sd_miso, false, false);
        for (int i = 0; i < config.sd_npullups; ++i)
            gpio_set_pulls(config.sd_pullups[i], false, false);
    }
    pins_on = on;
}

/// Sets the card up: SPI mode, its version and addressing, then full speed.
/// @return a card is ready
static bool card_init(void) {
    state = NONE;
    pins(true);
    spi_set_baudrate(SPI, 400 * 1000);
    gpio_put(config.sd_cs, 1);
    for (int i = 0; i < 10; ++i)  // 80 clocks with CS high: into SPI mode
        xfer(0xff);
    // GO_IDLE_STATE, until it answers: a card still busy with something it
    // was doing (a transfer cut short by a reset) takes a while to listen.
    uint8_t r = 0xff;
    absolute_time_t give_up = make_timeout_time_ms(1000), nobody = make_timeout_time_ms(100);
    bool answered = false;  // anything but FF (a card there, if busy)
    for (;;) {
        r = command(0, 0);
        answered |= r != 0xff;
        // Nothing at all for 100 ms (the data line just floats high): no card.
        if (r == 0x01 || time_reached(give_up) || (!answered && time_reached(nobody)))
            break;
        deselect();
        for (int i = 0; i < 10; ++i)  // more clocks with CS high
            xfer(0xff);
    }
    if (r != 0x01) {
        deselect();
        pins(false);  // no card: the pins are BASIC's again
        return false;
    }
    absolute_time_t until = make_timeout_time_ms(1000);
    bool ok = false;
    if (command(8, 0x1aa) == 0x01) {  // SEND_IF_COND: version 2 or later
        uint8_t ocr[4];
        spi_read_blocking(SPI, 0xff, ocr, 4);
        if (ocr[2] == 0x01 && ocr[3] == 0xaa) {
            while (!time_reached(until) && command(0x80 | 41, 1u << 30))  // ACMD41 with HCS
                ;
            if (!time_reached(until) && command(58, 0) == 0) {  // READ_OCR
                spi_read_blocking(SPI, 0xff, ocr, 4);
                block_addr = ocr[0] & 0x40;  // CCS
                ok = true;
            }
        }
    } else {  // version 1 (SDSC)
        uint8_t cmd = command(0x80 | 41, 0) <= 1 ? 0x80 | 41 : 1;
        while (!time_reached(until) && command(cmd, 0))
            ;
        ok = !time_reached(until) && command(16, 512) == 0;  // 512-byte blocks
        block_addr = false;
    }
    deselect();
    if (!ok) {
        pins(false);
        return false;
    }
    spi_set_baudrate(SPI, speed_khz * 1000);  // the fastest that has worked
    state = READY;
    return true;
}

// ---- FatFs's disk functions --------------------------------------------------------

DSTATUS sd_status(void) {
    if (state != READY)
        return STA_NOINIT;
    // Still there? SEND_STATUS: a card pulled out gives no answer.
    uint8_t r = command(13, 0);
    xfer(0xff);  // R2's second byte
    deselect();
    if (r & 0x80) {
        state = NONE;
        pins(false);
        return STA_NOINIT;
    }
    return 0;
}

DSTATUS sd_initialize(void) {
    if (!config.sd)
        return STA_NOINIT;  // SD = OFF in CONFIG.TXT
    return card_init() ? 0 : STA_NOINIT;
}

/// After a failed transfer: halve the clock (the lines may be too slow for
/// it: long wires, or the Maker Pi Pico's indicator LEDs) and set the card up
/// afresh, as garbled commands can leave it confused. False at 400 kHz, or
/// if the card is gone.
/// @return slowed down, so try again
static bool slow_down(void) {
    if (speed_khz <= 400)
        return false;
    speed_khz = speed_khz / 2 < 400 ? 400 : speed_khz / 2;
    return card_init();
}

/// Reads sectors once at the current speed.
/// @param buff where
/// @param sector the first
/// @param count how many
/// @return they came
static bool read_once(BYTE *buff, LBA_t sector, UINT count) {
    uint32_t addr = block_addr ? sector : sector * 512;
    bool ok;
    if (count == 1) {
        uint8_t r = command(17, addr);
        if (r)
            snprintf(last_error, sizeof last_error, "CMD17 gave %02x at %lu kHz", r,
                     (unsigned long)speed_khz);
        ok = r == 0 && read_data(buff, 512);
    } else {
        ok = command(18, addr) == 0;
        for (UINT i = 0; ok && i < count; ++i)
            ok = read_data(buff + i * 512, 512);
        command(12, 0);  // STOP_TRANSMISSION
    }
    deselect();
    return ok;
}

DRESULT sd_read(BYTE *buff, LBA_t sector, UINT count) {
    if (state != READY)
        return RES_NOTRDY;
    while (!read_once(buff, sector, count))
        if (!slow_down())
            return RES_ERROR;
    return RES_OK;
}

/// Writes sectors once at the current speed.
/// @param buff the data
/// @param sector the first
/// @param count how many
/// @return the card took them
static bool write_once(const BYTE *buff, LBA_t sector, UINT count) {
    uint32_t addr = block_addr ? sector : sector * 512;
    bool ok;
    if (count == 1) {
        ok = command(24, addr) == 0 && write_data(buff, 0xfe);
    } else {
        ok = command(25, addr) == 0;
        for (UINT i = 0; ok && i < count; ++i)
            ok = write_data(buff + i * 512, 0xfc);
        ok = write_data(NULL, 0xfd) && ok;
    }
    ok = wait_ready(500) && ok;  // the card programs the last block
    deselect();
    return ok;
}

DRESULT sd_write(const BYTE *buff, LBA_t sector, UINT count) {
    if (state != READY)
        return RES_NOTRDY;
    while (!write_once(buff, sector, count))
        if (!slow_down())
            return RES_ERROR;
    return RES_OK;
}

DRESULT sd_ioctl(BYTE cmd, void *buff) {
    if (state != READY)
        return RES_NOTRDY;
    DRESULT res = RES_ERROR;
    switch (cmd) {
    case CTRL_SYNC:
        select();
        res = wait_ready(500) ? RES_OK : RES_ERROR;
        break;
    case GET_SECTOR_COUNT: {
        uint8_t csd[16];
        if (command(9, 0) == 0 && read_data(csd, 16)) {  // SEND_CSD
            if ((csd[0] >> 6) == 1) {  // CSD version 2 (SDHC/SDXC): (C_SIZE + 1) x 512 KB
                uint32_t c_size = (uint32_t)(csd[7] & 0x3f) << 16 | csd[8] << 8 | csd[9];
                *(LBA_t *)buff = (c_size + 1) << 10;
            } else {  // version 1: (C_SIZE + 1) x 2^(C_SIZE_MULT + 2) x 2^READ_BL_LEN bytes
                uint32_t c_size = (uint32_t)(csd[6] & 3) << 10 | csd[7] << 2 | csd[8] >> 6;
                unsigned mult = ((csd[9] & 3) << 1 | csd[10] >> 7) + 2;
                unsigned bl_len = csd[5] & 15;
                *(LBA_t *)buff = (c_size + 1) << (mult + bl_len - 9);
            }
            res = RES_OK;
        }
        break;
    }
    case GET_SECTOR_SIZE:
        *(WORD *)buff = 512;
        res = RES_OK;
        break;
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = 1;  // not known: FatFs only uses it to format
        res = RES_OK;
        break;
    default:
        res = RES_PARERR;
    }
    deselect();
    return res;
}

// ---- For the rest of pico-basic ----------------------------------------------------

bool sd_card_in_use(void) {
    return pins_on;
}

bool sd_card_pin(int pin) {
    if (pin == config.sd_sck || pin == config.sd_mosi || pin == config.sd_miso || pin == config.sd_cs)
        return true;
    for (int i = 0; i < config.sd_npullups; ++i)
        if (pin == config.sd_pullups[i])
            return true;
    return false;
}

int sd_card_spi(void) {
    return config.sd_spi;
}

const char *sd_card_mount(void) {
    if (!config.sd)
        return config.sd_bad ? "off (its pins in CONFIG.TXT are wrong: see below)" : "off (SD = OFF in CONFIG.TXT)";
    FRESULT r = f_mount(&fs, "1:", 1);
    if (r == FR_OK)
        return NULL;
    // Mounted later, when the drive is first used (a card put in then).
    f_mount(&fs, "1:", 0);
    static char msg[100];
    snprintf(msg, sizeof msg, "can't be read (FatFs %d: %s)", r, last_error);
    return r == FR_NOT_READY ? "none" : r == FR_NO_FILESYSTEM ? "no FAT or exFAT file system on it"
                                                              : msg;
}

void sd_card_space(uint32_t *total, uint32_t *free_kb) {
    *total = *free_kb = 0;
    FATFS *f;
    DWORD clusters;
    if (state != READY || f_getfree("1:", &clusters, &f) != FR_OK)
        return;
    *total = (uint32_t)((uint64_t)(f->n_fatent - 2) * f->csize / 2);
    *free_kb = (uint32_t)((uint64_t)clusters * f->csize / 2);
}
