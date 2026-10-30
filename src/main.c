/**
 * @file main.c
 * @brief Pico BASIC on its own: a GW-BASIC style BASIC on an RP2350, used from
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
#include "pico/stdlib.h"
#include "psram_heap.h"
#include "stack_switch.h"

#ifndef PICO_BASIC_VERSION
#define PICO_BASIC_VERSION "0.1"
#endif

/// BASIC's stack: its expression evaluator recurses, and core 0's own stack is
/// only 4 KB.
#define BASIC_STACK (32 * 1024)
static uint8_t basic_stack[BASIC_STACK] __attribute__((aligned(8)));

/// Runs BASIC (on its own stack); SYSTEM comes back here.
/// @param arg unused
static void run_basic(void *arg) {
    (void)arg;
    basic_main(NULL);
}

int main(void) {
    stdio_init_all();
    size_t psram = psram_setup(clock_get_hz(clk_sys));
    // Give a terminal that was just plugged in a moment to listen.
    sleep_ms(500);
    printf("\x1b[0m\x1b[2J\x1b[H");
    printf("Pico BASIC %s for the RP2350 (%s)\n", PICO_BASIC_VERSION, PICO_BASIC_BOARD);
    if (psram)
        printf("PSRAM: %u MB\n", (unsigned)(psram >> 20));
    const char *err = flash_disk_mount();
    if (err) {
        printf("Flash disk: %s; files won't work\n", err);
    } else {
        uint32_t total, free_bytes;
        flash_disk_space(&total, &free_bytes);
        printf("Flash disk: %u KB, %u KB free\n", (unsigned)(total >> 10), (unsigned)(free_bytes >> 10));
    }
    for (;;) {
        call_on_stack(run_basic, NULL, basic_stack, sizeof basic_stack);
        printf("\n");
    }
}
