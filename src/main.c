/**
 * @file main.c
 * @brief Pico BASIC on its own: a GW-BASIC style BASIC on an RP2350 or RP2040, used from
 * a serial terminal, with its files in the Pico's flash.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#include "basic.h"
#include "flash_disk.h"
#include "hardware/clocks.h"
#if !PICO_RP2040
#include "hardware/psram.h"
#endif
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "psram_heap.h"
#include "stack_switch.h"
#include "usb_console.h"

#ifndef PICO_BASIC_VERSION
#define PICO_BASIC_VERSION "0.1"
#endif

/// BASIC's stack: its expression evaluator recurses, and core 0's own stack is
/// only 4 KB.
#define BASIC_STACK (32 * 1024)
static uint8_t basic_stack[BASIC_STACK] __attribute__((aligned(8)));

/// How long start-up waits for a terminal before going on without one (the
/// banner is shown again when one connects later: basic_port_serial.c).
#define CONNECT_WAIT_MS 3000

/// Free space on the flash disk, or why it isn't there.
static const char *disk_error;

void board_banner(void) {
#if PICO_RP2040
    printf("Pico BASIC %s for the RP2040 (%s)\n", PICO_BASIC_VERSION, PICO_BASIC_BOARD);
#else
    printf("Pico BASIC %s for the RP2350 (%s)\n", PICO_BASIC_VERSION, PICO_BASIC_BOARD);
    if (psram_present())
        printf("PSRAM: %u MB\n", (unsigned)(psram_get_size() >> 20));
#endif
    if (disk_error) {
        printf("Flash disk: %s; files won't work\n", disk_error);
    } else {
        uint32_t total, free_bytes;
        flash_disk_space(&total, &free_bytes);
        printf("Flash disk: %u KB, %u KB free\n", (unsigned)(total >> 10), (unsigned)(free_bytes >> 10));
    }
}

/// Runs BASIC (on its own stack); SYSTEM comes back here.
/// @param arg unused
static void run_basic(void *arg) {
    (void)arg;
    basic_main(NULL);
}

int main(void) {
    stdio_init_all();
    psram_setup(clock_get_hz(clk_sys));
    disk_error = flash_disk_mount();
    // A terminal already open on the USB port connects within a second or
    // so of the board appearing; without one, BASIC starts anyway.
    for (int ms = 0; ms < CONNECT_WAIT_MS && !stdio_usb_connected(); ms += 10)
        sleep_ms(10);
    sleep_ms(50);  // some terminals miss what comes right after they connect
    printf("\x1b[0m\x1b[2J\x1b[H");
    board_banner();
    for (;;) {
        call_on_stack(run_basic, NULL, basic_stack, sizeof basic_stack);
        printf("\n");
    }
}
