/**
 * @file flash_disk.c
 * @brief Implementation of flash_disk.h: FatFs's disk functions on the flash.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "flash_disk.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ff.h"
#include "diskio.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#ifndef FLASH_DISK_OFFSET
#define FLASH_DISK_OFFSET (1024 * 1024)  ///< where the disk starts in the flash
#endif

/// FatFs's sector.
#define SECTOR      512
/// The flash's erase block: 8 sectors.
#define BLOCK       FLASH_SECTOR_SIZE
/// Sectors on the disk.
#define DISK_SECTORS ((PICO_FLASH_SIZE_BYTES - FLASH_DISK_OFFSET) / SECTOR)

/// The disk's first byte, read through the XIP window.
static const uint8_t *const disk_xip = (const uint8_t *)(XIP_BASE + FLASH_DISK_OFFSET);

/// The erase block being written: its contents, which block (-1: none), and
/// whether it differs from the flash.
///@{
static uint8_t cache[BLOCK];
static int32_t cache_block = -1;
static bool cache_dirty;
///@}

static FATFS fs;  ///< the mounted file system
static bool mounted;  ///< it is

/// Writes the cached block back to the flash, if it changed.
/// @return false if the flash didn't take it
static bool cache_flush(void) {
    if (cache_block < 0 || !cache_dirty)
        return true;
    uint32_t off = FLASH_DISK_OFFSET + (uint32_t)cache_block * BLOCK;
    if (memcmp(disk_xip + (uint32_t)cache_block * BLOCK, cache, BLOCK)) {
        // Code runs from the flash, so nothing may run while it is busy:
        // interrupt handlers are in the flash too.
        uint32_t irq = save_and_disable_interrupts();
        flash_range_erase(off, BLOCK);
        flash_range_program(off, cache, BLOCK);
        restore_interrupts(irq);
    }
    cache_dirty = false;
    return !memcmp(disk_xip + (uint32_t)cache_block * BLOCK, cache, BLOCK);
}

DSTATUS disk_status(BYTE pdrv) {
    return pdrv ? STA_NOINIT : 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
    return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv || sector + count > DISK_SECTORS)
        return RES_PARERR;
    for (; count--; ++sector, buff += SECTOR) {
        int32_t block = sector / (BLOCK / SECTOR);
        if (block == cache_block)
            memcpy(buff, cache + sector % (BLOCK / SECTOR) * SECTOR, SECTOR);
        else
            memcpy(buff, disk_xip + sector * SECTOR, SECTOR);
    }
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv || sector + count > DISK_SECTORS)
        return RES_PARERR;
    for (; count--; ++sector, buff += SECTOR) {
        int32_t block = sector / (BLOCK / SECTOR);
        if (block != cache_block) {
            if (!cache_flush())
                return RES_ERROR;
            memcpy(cache, disk_xip + (uint32_t)block * BLOCK, BLOCK);
            cache_block = block;
        }
        uint8_t *dst = cache + sector % (BLOCK / SECTOR) * SECTOR;
        if (memcmp(dst, buff, SECTOR)) {
            memcpy(dst, buff, SECTOR);
            cache_dirty = true;
        }
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    if (pdrv)
        return RES_PARERR;
    switch (cmd) {
    case CTRL_SYNC: return cache_flush() ? RES_OK : RES_ERROR;
    case GET_SECTOR_COUNT: *(LBA_t *)buff = DISK_SECTORS; return RES_OK;
    case GET_SECTOR_SIZE: *(WORD *)buff = SECTOR; return RES_OK;
    case GET_BLOCK_SIZE: *(DWORD *)buff = BLOCK / SECTOR; return RES_OK;
    default: return RES_PARERR;
    }
}

DWORD get_fattime(void) {
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    if (tm.tm_year < 80)  // the clock hasn't been set: 1 Jan 2026
        return (DWORD)(2026 - 1980) << 25 | 1 << 21 | 1 << 16;
    return (DWORD)(tm.tm_year - 80) << 25 | (DWORD)(tm.tm_mon + 1) << 21 | (DWORD)tm.tm_mday << 16 |
           (DWORD)tm.tm_hour << 11 | (DWORD)tm.tm_min << 5 | (DWORD)tm.tm_sec >> 1;
}

const char *flash_disk_mount(void) {
    extern char __flash_binary_end;
    if ((uintptr_t)&__flash_binary_end > XIP_BASE + FLASH_DISK_OFFSET)
        return "the program has grown into the disk (raise FLASH_DISK_OFFSET)";
    FRESULT r = f_mount(&fs, "", 1);
    if (r == FR_NO_FILESYSTEM) {
        // The first start-up: FAT with 4 KB clusters, one to an erase block.
        void *work = malloc(BLOCK);
        if (!work)
            return "not enough memory to format the disk";
        MKFS_PARM opt = {.fmt = FM_FAT | FM_SFD, .au_size = BLOCK};
        r = f_mkfs("", &opt, work, BLOCK);
        free(work);
        if (r != FR_OK)
            return "couldn't format the disk";
        r = f_mount(&fs, "", 1);
        if (r == FR_OK)
            f_setlabel("PICOBASIC");
    }
    if (r != FR_OK)
        return "couldn't mount the disk";
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
