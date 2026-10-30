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
#if PICO_BASIC_SD
#include "sd_card.h"
#endif
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
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

#ifdef DISK_TRACE
#include "hardware/watchdog.h"
/// Kept through a reset: a run is under way, where the disk code got to
/// (flash_disk.c), and where a fault happened.
uint32_t __uninitialized_ram(trace_magic);
uint32_t __uninitialized_ram(fault_pc);
uint32_t __uninitialized_ram(fault_lr);
extern uint32_t trace_mark;

/// HardFault: keeps the faulting PC and LR from the stacked frame, then lets
/// the watchdog reset the board.
/// @param frame the stacked registers (r0-r3, r12, lr, pc, xpsr)
void __attribute__((used)) hardfault_c(uint32_t *frame) {
    fault_pc = frame[6];
    fault_lr = frame[5];
    watchdog_reboot(0, 0, 10);
    for (;;)
        ;
}

/// The HardFault vector: the frame is on the main stack (nothing here uses
/// the process stack).
void __attribute__((naked)) isr_hardfault(void) {
    __asm volatile(
        "mrs r0, msp\n"
        "ldr r1, =hardfault_c\n"
        "bx r1\n");
}
#endif

/// How long start-up waits for a terminal before going on without one (the
/// banner is shown again when one connects later: basic_port_serial.c).
#define CONNECT_WAIT_MS 3000

/// Free space on the flash disk, or why it isn't there.
static const char *disk_error;
#if PICO_BASIC_SD
/// Why the SD card isn't mounted ("none": no card), or NULL.
static const char *sd_error;
#endif

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
#if PICO_BASIC_SD
    uint32_t sd_total, sd_free;
    sd_card_space(&sd_total, &sd_free);
    if (sd_total)
        printf("SD card (SD:): %lu MB, %lu MB free\n", (unsigned long)(sd_total >> 10),
               (unsigned long)(sd_free >> 10));
    else
        printf("SD card (SD:): %s\n", sd_error ? sd_error : "none");
#endif
}

/// Runs BASIC (on its own stack); SYSTEM comes back here.
/// @param arg unused
static void run_basic(void *arg) {
    (void)arg;
    basic_main(NULL);
}

int main(void) {
    // The disk first: formatting it (the first start-up) writes the flash
    // with interrupts off for a while, which the USB can't be left waiting
    // through while the PC is setting it up.
    psram_setup(clock_get_hz(clk_sys));
#ifdef DISK_TRACE
    // A flash test before USB starts: erase and program the last 4 KB.
    static uint8_t pat[4096];
    static uint8_t got_erase[4], got_prog_nc[4], got_prog_c[4];
    static int bad_nc, bad_c;
    {
        const uint32_t test = PICO_FLASH_SIZE_BYTES - 4096;
        for (int i = 0; i < 4096; ++i)
            pat[i] = (uint8_t)(i * 7 + 1);
        uint32_t irq = save_and_disable_interrupts();
        flash_range_erase(test, 4096);
        restore_interrupts(irq);
        const volatile uint8_t *nc = (const volatile uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + test);
        const volatile uint8_t *ca = (const volatile uint8_t *)(XIP_BASE + test);
        for (int i = 0; i < 4; ++i)
            got_erase[i] = nc[i];
        irq = save_and_disable_interrupts();
        flash_range_program(test, pat, 4096);
        restore_interrupts(irq);
        for (int i = 0; i < 4; ++i) {
            got_prog_nc[i] = nc[i];
            got_prog_c[i] = ca[i];
        }
        for (int i = 0; i < 4096; ++i) {
            bad_nc += nc[i] != pat[i];
            bad_c += ca[i] != pat[i];
        }
    }
    stdio_init_all();
    while (!stdio_usb_connected())
        sleep_ms(10);
    sleep_ms(500);
    printf("pre-USB flash test: erased reads %02x %02x %02x %02x; programmed reads %02x %02x %02x %02x"
           " uncached, %02x %02x %02x %02x cached (want 01 08 0f 16); %d / %d bytes wrong\n",
           got_erase[0], got_erase[1], got_erase[2], got_erase[3], got_prog_nc[0], got_prog_nc[1],
           got_prog_nc[2], got_prog_nc[3], got_prog_c[0], got_prog_c[1], got_prog_c[2], got_prog_c[3],
           bad_nc, bad_c);
    if (trace_magic == 0x7ace) {
        // The last run faulted or hung (the watchdog reset it): say where.
        printf("previous run: last mark %08lx, fault pc %08lx lr %08lx, reset by %s\n",
               (unsigned long)trace_mark, (unsigned long)fault_pc, (unsigned long)fault_lr,
               watchdog_caused_reboot() ? "the watchdog" : "something else");
        trace_magic = 0;
        disk_error = "not mounted (trace build, after a fault)";
    } else {
        trace_magic = 0x7ace;
        fault_pc = fault_lr = 0;
        trace_mark = 0;
        watchdog_enable(8000, true);
        printf("trace build: mounting\n");
        disk_error = flash_disk_mount();
        hw_clear_bits(&watchdog_hw->ctrl, WATCHDOG_CTRL_ENABLE_BITS);
        trace_magic = 0;
        printf("mount done\n");
    }
#else
    disk_error = flash_disk_mount();
    stdio_init_all();
#endif
#if PICO_BASIC_SD
    sd_error = sd_card_mount();
#endif
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
