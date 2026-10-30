/**
 * @file sd_card.h
 * @brief A microSD card over SPI, as FatFs's second drive ("1:" or "SD:").
 *
 * SPI mode: SDSC (byte addresses), SDHC and SDXC (block addresses). The card
 * is set up when FatFs first uses it (400 kHz, then SD_SPI_KHZ), and checked
 * on every access, so a card pulled out is noticed and one put in is set up
 * afresh. Every wait has a time limit: a missing card never hangs.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "ff.h"
#include "diskio.h"

/// FatFs's disk functions for the card (diskio.c passes drive 1 here).
///@{
DSTATUS sd_status(void);
DSTATUS sd_initialize(void);
DRESULT sd_read(BYTE *buff, LBA_t sector, UINT count);
DRESULT sd_write(const BYTE *buff, LBA_t sector, UINT count);
DRESULT sd_ioctl(BYTE cmd, void *buff);
///@}

/// Whether a card is set up now (its pins and SPI are then BASIC's no more).
/// @return it is
bool sd_card_in_use(void);

/// Whether a pin is one of the card's (whether or not a card is in).
/// @param pin the pin
/// @return it is
bool sd_card_pin(int pin);

/// The card's SPI controller number (0 or 1).
/// @return it
int sd_card_spi(void);

/// Mounts the card if one is in. NULL if it is ready, else why not.
/// @return NULL, or why not
const char *sd_card_mount(void);

/// The card's size and free space in KB (0 if none is mounted).
/// @param total set to its size
/// @param free set to its free space
void sd_card_space(uint32_t *total, uint32_t *free);
