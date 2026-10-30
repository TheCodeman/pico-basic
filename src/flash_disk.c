/**
 * @file flash_disk.c
 * @brief Implementation of flash_disk.h: FatFs's disk functions on the flash.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "flash_disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ff.h"
#include "diskio.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/flash.h"
#include "pico/stdlib.h"

#ifndef FLASH_DISK_OFFSET
#define FLASH_DISK_OFFSET (1024 * 1024)  ///< where the disk starts in the flash
#endif

#ifdef DISK_TRACE
#include "hardware/watchdog.h"
#define TRACE(...) (printf(__VA_ARGS__), stdio_flush(), sleep_ms(100))
/// Where the disk code got to, kept through a reset (main.c prints it).
uint32_t __uninitialized_ram(trace_mark);
#define MARK(n) (trace_mark = (n), watchdog_update())
#else
#define TRACE(...) ((void)0)
#define MARK(n) ((void)0)
#endif

/// FatFs's sector.
#define SECTOR      512
/// The flash's erase block: 8 sectors.
#define BLOCK       FLASH_SECTOR_SIZE
/// Sectors on the disk.
#define DISK_SECTORS ((PICO_FLASH_SIZE_BYTES - FLASH_DISK_OFFSET) / SECTOR)

/// The disk's first byte, read through the XIP window that bypasses the
/// cache: what was just written is read back from the chip itself, never
/// from a stale cache line.
static const volatile uint8_t *const disk_xip =
    (const volatile uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + FLASH_DISK_OFFSET);

/// Copies n bytes from the disk's flash (bytewise: the window is volatile).
/// @param dst where to
/// @param off the offset in the disk
/// @param n how many
static void flash_copy(uint8_t *dst, uint32_t off, uint32_t n) {
    MARK(0x40000000u | off);
    for (uint32_t i = 0; i < n; ++i) {
        dst[i] = disk_xip[off + i];
        MARK(0x41000000u | (off + i));
    }
    MARK(0x42000000u | off);
}

/// Whether the disk's flash at off holds the n bytes at src.
/// @param off the offset in the disk
/// @param src the bytes
/// @param n how many
/// @return it does
static bool flash_same(uint32_t off, const uint8_t *src, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
        if (disk_xip[off + i] != src[i])
            return false;
    return true;
}

/// The erase block being written: its contents, which block (-1: none), and
/// whether it differs from the flash.
///@{
static uint8_t cache[BLOCK];
static int32_t cache_block = -1;
static bool cache_dirty;
///@}

/// What went wrong in the flash disk itself, for flash_disk_mount's message.
static char disk_error[64];

static FATFS fs;  ///< the mounted file system
static bool mounted;  ///< it is


/// Erases and programs the cached block (run by flash_safe_execute, with
/// interrupts off and the other core, if running, out of the flash).
/// @param arg the block's offset in the flash
static void erase_and_program(void *arg) {
    uint32_t off = (uint32_t)(uintptr_t)arg;
    flash_range_erase(off, BLOCK);
    flash_range_program(off, cache, BLOCK);
}

/// The flash chip's status registers 1 and 2 (commands 05 and 35).
static uint8_t status1, status2;

/// Reads the status registers (run by flash_safe_execute).
/// @param arg unused
static void read_status(void *arg) {
    (void)arg;
    uint8_t tx[2] = {0x05, 0}, rx[2];
    flash_do_cmd(tx, rx, 2);
    status1 = rx[1];
    tx[0] = 0x35;
    flash_do_cmd(tx, rx, 2);
    status2 = rx[1];
}

/// Clears the block-protect bits (status register 1's BP0-2, TB, SEC and
/// register 2's CMP), keeping the rest (QE above all: the quad reads the
/// program runs by need it). Run by flash_safe_execute.
/// @param arg unused
static void unprotect(void *arg) {
    (void)arg;
    uint8_t we[1] = {0x06}, rx[3];
    flash_do_cmd(we, rx, 1);
    uint8_t ws[3] = {0x01, 0x00, (uint8_t)(status2 & ~0x40)};
    flash_do_cmd(ws, rx, 3);
    uint8_t rs[2] = {0x05, 0};
    do
        flash_do_cmd(rs, rx, 2);
    while (rx[1] & 1);  // busy writing the status
}

/// Writes the cached block back to the flash, if it changed.
/// @return false if the flash didn't take it
static bool cache_flush(void) {
    MARK(0x50000000u | (uint32_t)(cache_block & 0xffff));
    if (cache_block < 0 || !cache_dirty)
        return true;
    uint32_t off = FLASH_DISK_OFFSET + (uint32_t)cache_block * BLOCK;
    TRACE("flush block %ld (flash offset %lx)\n", (long)cache_block, (unsigned long)off);
    if (!flash_same((uint32_t)cache_block * BLOCK, cache, BLOCK)) {
        TRACE("  erase+program...\n");
#ifdef DISK_TRACE
        uint32_t irq = save_and_disable_interrupts();
        flash_range_erase(off, BLOCK);
        restore_interrupts(irq);
        TRACE("  erased: reads %02x %02x\n", disk_xip[(uint32_t)cache_block * BLOCK],
              disk_xip[(uint32_t)cache_block * BLOCK + 1]);
#endif
        // Code runs from the flash, so nothing may run from it while it is
        // busy: flash_safe_execute keeps interrupts off meanwhile.
        int r = flash_safe_execute(erase_and_program, (void *)(uintptr_t)off, 1000);
        if (r != PICO_OK) {
            snprintf(disk_error, sizeof disk_error, "block %ld: flash_safe_execute gave %d",
                     (long)cache_block, r);
            return false;
        }
        TRACE("  done; reads %02x %02x (cached %02x)\n", disk_xip[(uint32_t)cache_block * BLOCK],
              disk_xip[(uint32_t)cache_block * BLOCK + 1],
              *(const volatile uint8_t *)(XIP_BASE + off));
    }
    cache_dirty = false;
    for (int i = 0; i < BLOCK; ++i) {
        uint8_t got = disk_xip[(uint32_t)cache_block * BLOCK + i];
        if (got != cache[i]) {
            snprintf(disk_error, sizeof disk_error, "block %ld byte %d reads %02x, not %02x",
                     (long)cache_block, i, got, cache[i]);
            return false;
        }
    }
    return true;
}

DSTATUS disk_status(BYTE pdrv) {
    return pdrv ? STA_NOINIT : 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
    return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    MARK(0x20000000u | (uint32_t)sector << 8 | count);
    if (pdrv || sector + count > DISK_SECTORS)
        return RES_PARERR;
    for (; count--; ++sector, buff += SECTOR) {
        int32_t block = sector / (BLOCK / SECTOR);
        if (block == cache_block)
            memcpy(buff, cache + sector % (BLOCK / SECTOR) * SECTOR, SECTOR);
        else
            flash_copy(buff, sector * SECTOR, SECTOR);
    }
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    MARK(0x10000000u | (uint32_t)sector << 8 | count);
    if (pdrv || sector + count > DISK_SECTORS) {
        snprintf(disk_error, sizeof disk_error, "write of %u at %lu, past %u", (unsigned)count,
                 (unsigned long)sector, (unsigned)DISK_SECTORS);
        return RES_PARERR;
    }
    for (; count--; ++sector, buff += SECTOR) {
        int32_t block = sector / (BLOCK / SECTOR);
        if (block != cache_block) {
            if (!cache_flush())
                return RES_ERROR;
            flash_copy(cache, (uint32_t)block * BLOCK, BLOCK);
            cache_block = block;
        }
        uint8_t *dst = cache + sector % (BLOCK / SECTOR) * SECTOR;
        MARK(0x43000000u | (uint32_t)sector);
        if (memcmp(dst, buff, SECTOR)) {
            memcpy(dst, buff, SECTOR);
            cache_dirty = true;
        }
        MARK(0x44000000u | (uint32_t)sector);
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    MARK(0x30000000u | cmd);
    if (pdrv)
        return RES_PARERR;
    switch (cmd) {
    case CTRL_SYNC: return cache_flush() ? RES_OK : RES_ERROR;
    case GET_SECTOR_COUNT: *(LBA_t *)buff = DISK_SECTORS; return RES_OK;
    case GET_SECTOR_SIZE: *(WORD *)buff = SECTOR; return RES_OK;
    case GET_BLOCK_SIZE: *(DWORD *)buff = BLOCK / SECTOR; return RES_OK;
    default:
        snprintf(disk_error, sizeof disk_error, "ioctl %u", cmd);
        return RES_PARERR;
    }
}

DWORD get_fattime(void) {
    MARK(0x80000000u);
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    if (tm.tm_year < 80)  // the clock hasn't been set: 1 Jan 2026
        return (DWORD)(2026 - 1980) << 25 | 1 << 21 | 1 << 16;
    return (DWORD)(tm.tm_year - 80) << 25 | (DWORD)(tm.tm_mon + 1) << 21 | (DWORD)tm.tm_mday << 16 |
           (DWORD)tm.tm_hour << 11 | (DWORD)tm.tm_min << 5 | (DWORD)tm.tm_sec >> 1;
}

#ifdef DISK_TRACE
/// The flash chip's JEDEC ID (maker, type, size), read with command 9F.
static uint8_t jedec[4];

/// Reads the JEDEC ID (run by flash_safe_execute).
/// @param arg unused
static void read_jedec(void *arg) {
    (void)arg;
    uint8_t tx[4] = {0x9f, 0, 0, 0};
    flash_do_cmd(tx, jedec, 4);
}
#endif

const char *flash_disk_mount(void) {
#ifdef DISK_TRACE
    int jr = flash_safe_execute(read_jedec, NULL, 1000);
    TRACE("flash JEDEC ID (rc %d): %02x %02x %02x = %lu KB; disk %u sectors from offset %x\n", jr,
          jedec[1], jedec[2], jedec[3], jedec[3] ? (1ul << jedec[3]) >> 10 : 0ul,
          (unsigned)DISK_SECTORS, (unsigned)FLASH_DISK_OFFSET);
#endif
    // Some boards' flash comes with parts of it write-protected (a status
    // register setting that survives power-off): erasing and programming
    // there do nothing. Clear that.
    flash_safe_execute(read_status, NULL, 1000);
    TRACE("status registers %02x %02x\n", status1, status2);
    if ((status1 & 0x7c) || (status2 & 0x40)) {
        flash_safe_execute(unprotect, NULL, 1000);
        flash_safe_execute(read_status, NULL, 1000);
        TRACE("after clearing protection: %02x %02x\n", status1, status2);
    }
    extern char __flash_binary_end;
    if ((uintptr_t)&__flash_binary_end > XIP_BASE + FLASH_DISK_OFFSET)
        return "the program has grown into the disk (raise FLASH_DISK_OFFSET)";
    TRACE("mount: first bytes %02x %02x %02x\n", disk_xip[0], disk_xip[1], disk_xip[2]);
#ifdef DISK_TRACE
    {
        static uint8_t t[BLOCK];
        uint32_t t0 = time_us_32();
        for (int i = 0; i < 16; ++i)
            t[i] = disk_xip[i];
        uint32_t t1 = time_us_32();
        memcpy(t, (const void *)(XIP_BASE + FLASH_DISK_OFFSET), BLOCK);
        uint32_t t2 = time_us_32();
        for (int i = 0; i < 16; ++i)
            t[i] = ((const volatile uint8_t *)(XIP_NOCACHE_BASE + FLASH_DISK_OFFSET))[i];
        uint32_t t3 = time_us_32();
        TRACE("timing: 16 nocache-noalloc %lu us, 4096 cached %lu us, 16 nocache %lu us\n",
              (unsigned long)(t1 - t0), (unsigned long)(t2 - t1), (unsigned long)(t3 - t2));
        // Erase and program the flash's last 4 KB (outside the disk) and read it back.
        const uint32_t test = PICO_FLASH_SIZE_BYTES - BLOCK;
        for (int i = 0; i < BLOCK; ++i)
            t[i] = (uint8_t)(i * 7 + 1);
        MARK(0x90000000u);
        uint32_t irq = save_and_disable_interrupts();
        flash_range_erase(test, BLOCK);
        restore_interrupts(irq);
        MARK(0x91000000u);
        const volatile uint8_t *nc = (const volatile uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + test);
        const volatile uint8_t *ca = (const volatile uint8_t *)(XIP_BASE + test);
        TRACE("test erase: nocache %02x %02x, cached %02x %02x\n", nc[0], nc[1], ca[0], ca[1]);
        irq = save_and_disable_interrupts();
        flash_range_program(test, t, BLOCK);
        restore_interrupts(irq);
        MARK(0x92000000u);
        TRACE("test program: nocache %02x %02x, cached %02x %02x (want 01 08)\n", nc[0], nc[1], ca[0], ca[1]);
        int bad = 0;
        for (int i = 0; i < BLOCK; ++i) {
            if (nc[i] != t[i])
                ++bad;
            if ((i & 255) == 255)
                MARK(0x93000000u | i);
        }
        TRACE("test whole block uncached: %d bad\n", bad);
    }
#endif
    FRESULT r = f_mount(&fs, "", 1);
    TRACE("f_mount %d\n", r);
    if (r == FR_NO_FILESYSTEM) {
        // The first start-up: FAT with 4 KB clusters, one to an erase block.
        MARK(0x61000000u);
        void *work = malloc(BLOCK);
        MARK(0x62000000u | ((uintptr_t)work & 0xffffff));
        if (!work)
            return "not enough memory to format the disk";
        MKFS_PARM opt = {.fmt = FM_FAT | FM_SFD, .au_size = BLOCK};
#ifdef DISK_TRACE
        printf("f_mkfs (work at %p)...\n", work);
        MARK(0x63000000u);
        stdio_flush();
        MARK(0x64000000u);
        busy_wait_ms(100);
        MARK(0x65000000u);
#endif
        MARK(0x60000000u);
        r = f_mkfs("", &opt, work, BLOCK);
        MARK(0x70000000u);
        TRACE("f_mkfs %d\n", r);
        free(work);
        if (r != FR_OK) {
            static char msg[100];
            snprintf(msg, sizeof msg, "couldn't format the disk (FatFs error %d%s%s)", r,
                     disk_error[0] ? ": " : "", disk_error);
            return msg;
        }
        r = f_mount(&fs, "", 1);
        if (r == FR_OK)
            f_setlabel("PICOBASIC");
    }
    if (r != FR_OK) {
        static char msg[100];
        snprintf(msg, sizeof msg, "couldn't mount the disk (FatFs error %d%s%s)", r,
                 disk_error[0] ? ": " : "", disk_error);
        return msg;
    }
    mounted = true;
    return NULL;
}

void flash_disk_space(uint32_t *total, uint32_t *free_bytes) {
    *total = *free_bytes = 0;
    if (!mounted)
        return;
    FATFS *f;
    DWORD clusters;
    if (f_getfree("", &clusters, &f) != FR_OK)
        return;
    *total = (f->n_fatent - 2) * f->csize * SECTOR;
    *free_bytes = clusters * f->csize * SECTOR;
}
