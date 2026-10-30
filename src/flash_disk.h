/**
 * @file flash_disk.h
 * @brief A FAT disk in the end of the Pico's own flash, for FatFs.
 *
 * The program sits at the start of the flash; the disk takes from
 * FLASH_DISK_OFFSET to the end. Writes go through a 4 KB cache (the flash's
 * erase size), written back when FatFs syncs (f_sync, f_close).
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/// Mounts the disk, formatting it first if it has no FAT file system (the
/// first start-up). NULL if it is ready, else what went wrong.
/// @return NULL, or why not
const char *flash_disk_mount(void);

/// The disk's size and free space in bytes (0 if not mounted).
/// @param total set to its size
/// @param free set to its free space
void flash_disk_space(uint32_t *total, uint32_t *free);
