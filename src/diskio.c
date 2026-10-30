/**
 * @file diskio.c
 * @brief FatFs's disk functions: drive 0 is the flash disk, drive 1 the SD card.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "ff.h"
#include "diskio.h"
#include "flash_disk.h"
#if PICO_BASIC_SD
#include "sd_card.h"
#endif

DSTATUS disk_status(BYTE pdrv) {
#if PICO_BASIC_SD
    if (pdrv == 1)
        return sd_status();
#endif
    return pdrv == 0 ? flash_disk_status(0) : STA_NOINIT;
}

DSTATUS disk_initialize(BYTE pdrv) {
#if PICO_BASIC_SD
    if (pdrv == 1)
        return sd_initialize();
#endif
    return pdrv == 0 ? flash_disk_initialize(0) : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
#if PICO_BASIC_SD
    if (pdrv == 1)
        return sd_read(buff, sector, count);
#endif
    return pdrv == 0 ? flash_disk_read(0, buff, sector, count) : RES_PARERR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
#if PICO_BASIC_SD
    if (pdrv == 1)
        return sd_write(buff, sector, count);
#endif
    return pdrv == 0 ? flash_disk_write(0, buff, sector, count) : RES_PARERR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
#if PICO_BASIC_SD
    if (pdrv == 1)
        return sd_ioctl(cmd, buff);
#endif
    return pdrv == 0 ? flash_disk_ioctl(0, cmd, buff) : RES_PARERR;
}
