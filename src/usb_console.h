/**
 * @file usb_console.h
 * @brief The console on the board's USB port: what main.c and the port share.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */
#pragma once

/// Prints the board's banner: the version, the PSRAM and the flash disk.
/// Shown at start-up, and again when a terminal connects later.
void board_banner(void);
