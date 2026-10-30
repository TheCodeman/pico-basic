/**
 * @file sd_card.c
 * @brief Implementation of sd_card.h: SD cards in SPI mode.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "sd_card.h"

#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#ifndef SD_SPI
#define SD_SPI  1   ///< the SPI controller: 0 or 1
#endif
#ifndef SD_MISO
#define SD_MISO 12  ///< card SDO / DO (data out)
#endif
#ifndef SD_CS
#define SD_CS   15  ///< card CS
#endif
#ifndef SD_SCK
#define SD_SCK  10  ///< card CLK
#endif
#ifndef SD_MOSI
#define SD_MOSI 11  ///< card SDI / DI (data in)
#endif
#ifndef SD_DAT1
#define SD_DAT1 13  ///< card DAT1: unused in SPI mode, held up (-1: not wired)
#endif
#ifndef SD_DAT2
#define SD_DAT2 14  ///< card DAT2: likewise
#endif
#ifndef SD_SPI_KHZ
#define SD_SPI_KHZ 25000  ///< the clock once the card is set up
#endif

/// The controller.
#define SPI (SD_SPI ? spi1 : spi0)

/// The card's state.
static enum { NONE, READY } state;
/// SDHC/SDXC: sectors are addressed by number (SDSC: by byte).
static bool block_addr;
/// The pins are set up for the card.
static bool pins_on;

static FATFS fs;  ///< the mounted card

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
    gpio_put(SD_CS, 0);
    xfer(0xff);
}

/// Deselects it (CS high, then a byte so it lets go of DO).
static void deselect(void) {
    gpio_put(SD_CS, 1);
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
    if (token != 0xfe)
        return false;
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
        spi_set_format(SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
        gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);
        gpio_set_function(SD_MISO, GPIO_FUNC_SPI);
        gpio_pull_up(SD_MISO);  // DO floats while the card is deselected
        gpio_init(SD_CS);
        gpio_put(SD_CS, 1);
        gpio_set_dir(SD_CS, GPIO_OUT);
        // The card's other data lines aren't used in SPI mode, but left
        // floating some cards won't answer: hold them up.
        if (SD_DAT1 >= 0) {
            gpio_init(SD_DAT1);
            gpio_pull_up(SD_DAT1);
        }
        if (SD_DAT2 >= 0) {
            gpio_init(SD_DAT2);
            gpio_pull_up(SD_DAT2);
        }
    } else {
        spi_deinit(SPI);
        gpio_init(SD_SCK);
        gpio_init(SD_MOSI);
        gpio_init(SD_MISO);
        gpio_init(SD_CS);
        gpio_set_pulls(SD_MISO, false, false);
        if (SD_DAT1 >= 0)
            gpio_set_pulls(SD_DAT1, false, false);
        if (SD_DAT2 >= 0)
            gpio_set_pulls(SD_DAT2, false, false);
    }
    pins_on = on;
}

/// Sets the card up: SPI mode, its version and addressing, then full speed.
/// @return a card is ready
static bool card_init(void) {
    state = NONE;
    pins(true);
    spi_set_baudrate(SPI, 400 * 1000);
    gpio_put(SD_CS, 1);
    for (int i = 0; i < 10; ++i)  // 80 clocks with CS high: into SPI mode
        xfer(0xff);
    uint8_t r = 0xff;
    for (int i = 0; i < 5 && r != 0x01; ++i)
        r = command(0, 0);  // GO_IDLE_STATE
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
    spi_set_baudrate(SPI, SD_SPI_KHZ * 1000);
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
    return card_init() ? 0 : STA_NOINIT;
}

DRESULT sd_read(BYTE *buff, LBA_t sector, UINT count) {
    if (state != READY)
        return RES_NOTRDY;
    uint32_t addr = block_addr ? sector : sector * 512;
    bool ok;
    if (count == 1) {
        ok = command(17, addr) == 0 && read_data(buff, 512);
    } else {
        ok = command(18, addr) == 0;
        for (UINT i = 0; ok && i < count; ++i)
            ok = read_data(buff + i * 512, 512);
        command(12, 0);  // STOP_TRANSMISSION
    }
    deselect();
    return ok ? RES_OK : RES_ERROR;
}

DRESULT sd_write(const BYTE *buff, LBA_t sector, UINT count) {
    if (state != READY)
        return RES_NOTRDY;
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
    return ok ? RES_OK : RES_ERROR;
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
    return pin == SD_SCK || pin == SD_MOSI || pin == SD_MISO || pin == SD_CS || pin == SD_DAT1 ||
           pin == SD_DAT2;
}

int sd_card_spi(void) {
    return SD_SPI;
}

const char *sd_card_mount(void) {
    FRESULT r = f_mount(&fs, "1:", 1);
    if (r == FR_OK)
        return NULL;
    // Mounted later, when the drive is first used (a card put in then).
    f_mount(&fs, "1:", 0);
    return r == FR_NOT_READY ? "none" : r == FR_NO_FILESYSTEM ? "no FAT or exFAT file system on it"
                                                              : "can't be read";
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
