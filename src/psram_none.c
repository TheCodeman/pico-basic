/**
 * @file psram_none.c
 * @brief psram_heap.h for chips without PSRAM (the RP2040): there is none, so
 * every allocation fails and BASIC keeps everything in SRAM.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "psram_heap.h"

#include <stdlib.h>
#include <string.h>

size_t psram_setup(uint32_t sys_hz) {
    (void)sys_hz;
    return 0;
}

void *psram_test(void *start, size_t len) {
    (void)start;
    (void)len;
    return NULL;
}

void *psram_malloc(size_t size) {
    (void)size;
    return NULL;
}

void psram_free(void *p) {
    (void)p;
}

void *psram_buffer_alloc(size_t size) {
    return size <= 64 * 1024 ? malloc(size) : NULL;
}

void psram_buffer_free(void *p) {
    free(p);
}

void *psram_realloc(void *p, size_t size) {
    (void)p;
    (void)size;
    return NULL;
}

size_t psram_block_size(const void *p) {
    (void)p;
    return 0;
}

void psram_get_stats(psram_stats_t *stats) {
    memset(stats, 0, sizeof *stats);
}

bool psram_present(void) {
    return false;
}

size_t psram_free_total(void) {
    return 0;
}

void psram_scrub(void *start, size_t len, unsigned period_s) {
    (void)start;
    (void)len;
    (void)period_s;
}

uint32_t psram_scrub_passes(void) {
    return 0;
}
