/**
 * @file basic_port_serial.c
 * @brief basic_port.h for a bare RP2350 or RP2040 board with a serial terminal.
 *
 * Text goes to and from a terminal (ANSI/VT100) on the board's USB port (a
 * USB serial device) through stdio. There
 * is no screen of its own: graphics, pictures, fonts, the mouse, MUSIC, the
 * editor and SHELL give "Feature not available" (basic.c checks that before
 * it gets here, built with BASIC_NO_SCREEN). BEEP, SOUND and PLAY are square
 * waves on SPEAKER_PIN for a piezo or small speaker. Files are on the flash
 * disk (flash_disk.c).
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */

#include "basic_port.h"
#include "basic.h"

#include <malloc.h>
#include <unistd.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#include "pico/stdio/driver.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "psram_heap.h"
#include "spr.h"
#include "stack_switch.h"
#include "config.h"
#include "term_editor.h"
#include "usb_console.h"
#include "board_watchdog.h"
#include "hardware/watchdog.h"
#if PICO_BASIC_WIFI
#include "wifi.h"
#endif
#if PICO_BASIC_SD
#include "sd_card.h"
#endif

/// BEEP, SOUND and PLAY's pin: a piezo or speaker (through a resistor) to
/// ground. CONFIG.TXT's SPEAKER, else the build's (-1: none).
#define SPEAKER_PIN (config.speaker)

/// What the screen-only features say if asked anyway.
#define NO_SCREEN "not available on a serial terminal"

/// A terminal was connected when last looked (main.c has just shown the
/// banner if one was).
static bool was_connected = true;

// ---- The console's backlog: what is printed while no terminal is connected ----------
// (an AUTORUN.BAS on a board on its own, say) is kept, the newest of it, and
// shown when one connects, after the banner.

#ifndef CONSOLE_BACKLOG
#if PICO_RP2040
#define CONSOLE_BACKLOG 4096  ///< bytes kept (SRAM is short on the RP2040)
#else
#define CONSOLE_BACKLOG 8192  ///< bytes kept
#endif
#endif

static char backlog[CONSOLE_BACKLOG];  ///< a ring
static uint32_t backlog_start, backlog_len;  ///< where the oldest byte is, and how many
static bool backlog_lost;  ///< older output was overwritten

/// stdio output (after its CR LF translation) while no terminal is there.
/// @param buf the bytes
/// @param len how many
static void backlog_out(const char *buf, int len) {
    if (stdio_usb_connected())
        return;  // the USB driver has it
    for (int i = 0; i < len; ++i) {
        backlog[(backlog_start + backlog_len) % CONSOLE_BACKLOG] = buf[i];
        if (backlog_len < CONSOLE_BACKLOG) {
            ++backlog_len;
        } else {
            backlog_start = (backlog_start + 1) % CONSOLE_BACKLOG;
            backlog_lost = true;
        }
    }
}

/// The backlog as a stdio driver of its own, next to the USB one.
static stdio_driver_t backlog_driver = {
    .out_chars = backlog_out,
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = PICO_STDIO_DEFAULT_CRLF,
#endif
};

void console_backlog_start(void) {
    stdio_set_driver_enabled(&backlog_driver, true);
}

/// Sends the backlog to the terminal that has just connected, and empties it.
static void backlog_replay(void) {
    if (!backlog_len)
        return;
    printf("--- printed while no terminal was connected%s ---\n",
           backlog_lost ? " (the oldest is lost)" : "");
    stdio_flush();
    uint32_t first = CONSOLE_BACKLOG - backlog_start;  // up to the end of the ring
    if (first > backlog_len)
        first = backlog_len;
    stdio_usb.out_chars(backlog + backlog_start, (int)first);  // translated already
    if (backlog_len > first)
        stdio_usb.out_chars(backlog, (int)(backlog_len - first));
    backlog_start = backlog_len = 0;
    backlog_lost = false;
    printf("---\n");
}

int port_key(uint32_t timeout_us) {
    // A running program asks every statement: look at the USB at most every
    // 200 us (a key waits that long at worst).
    static uint32_t last_poll;
    uint32_t now = time_us_32();
    if (!timeout_us && now - last_poll < 200)
        return -1;
    last_poll = now;
    int c = getchar_timeout_us(timeout_us);
    if (c == PICO_ERROR_TIMEOUT)
        c = -1;
    // A terminal that connects after start-up (or again, after closing)
    // missed the banner: show it, and a prompt, before the key that came
    // with it is echoed. A key arriving counts as connected too: a terminal
    // can send before the USB line state says it is there.
    bool connected = c >= 0 || stdio_usb_connected();
    if (connected && !was_connected) {
        if (c < 0)
            sleep_ms(50);  // some terminals miss what comes right after they connect
        printf("\n");
        board_banner();
        fflush(stdout);
        basic_print_free();
        backlog_replay();
        printf(basic_program_running() ? "A program is running (Ctrl-C stops it)\n" : "Ok\n");
    }
    was_connected = connected;
    return c;
}

/// The IBM PC's pictures for control codes 1-31, as Unicode.
static const uint16_t cp437_low[32] = {
    0x0020, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022, 0x25D8, 0x25CB, 0x25D9,
    0x2642, 0x2640, 0x266A, 0x266B, 0x263C, 0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7,
    0x25AC, 0x21A8, 0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
};

/// The IBM PC's characters 127-255, as Unicode.
static const uint16_t cp437_high[129] = {
    0x2302,
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA, 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4, 0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
};

int port_glyph_utf8(int c, char *out) {
    // The terminal has no PC character set: send the character's picture as
    // UTF-8 (most terminals are), so CHR$(1) is a smiley and CHR$(219) a block.
    unsigned u = c >= 0 && c < 32 ? cp437_low[c] : c >= 127 && c < 256 ? cp437_high[c - 127] : '?';
    if (u < 0x80) {
        out[0] = (char)u;
        return 1;
    }
    if (u < 0x800) {
        out[0] = (char)(0xC0 | u >> 6);
        out[1] = (char)(0x80 | (u & 0x3F));
        return 2;
    }
    out[0] = (char)(0xE0 | u >> 12);
    out[1] = (char)(0x80 | (u >> 6 & 0x3F));
    out[2] = (char)(0x80 | (u & 0x3F));
    return 3;
}

void port_put_glyph(int c) {
    // putchar, not fwrite: the SDK's putchar goes straight to the console,
    // while fwrite would wait in the C library's buffer behind later text.
    char buf[3];
    for (int i = 0, n = port_glyph_utf8(c, buf); i < n; ++i)
        putchar((unsigned char)buf[i]);
}

// ---- Sound: a square wave on SPEAKER_PIN ------------------------------------------
// PLAY's notes queue up and play in the background, one alarm per note.

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"

/// PLAY's queue.
#define NOTES 64
static struct {
    uint16_t freq, ms;  ///< the note (freq 0: a rest)
} notes[NOTES];
/// The queue's ends; a note is playing while note_alarm is set.
///@{
static volatile int note_head, note_tail;
static volatile alarm_id_t note_alarm;
///@}

/// Starts or stops the tone (0 Hz: silence).
/// @param freq_hz the frequency
static void tone(unsigned freq_hz) {
    if (SPEAKER_PIN < 0)
        return;  // no speaker
    uint slice = pwm_gpio_to_slice_num(SPEAKER_PIN);
    if (!freq_hz || freq_hz < 20 || freq_hz > 20000) {
        pwm_set_gpio_level(SPEAKER_PIN, 0);
        return;
    }
    float clk = (float)clock_get_hz(clk_sys);
    float div = clk / ((float)freq_hz * 65536.0f);
    if (div < 1.0f)
        div = 1.0f;
    uint32_t top = (uint32_t)(clk / (div * (float)freq_hz)) - 1;
    if (top > 65535)
        top = 65535;
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, top);
    pwm_set_gpio_level(SPEAKER_PIN, (top + 1) / 2);
    gpio_set_function(SPEAKER_PIN, GPIO_FUNC_PWM);
    pwm_set_enabled(slice, true);
}

/// The alarm at the end of a note: the next one, or silence.
/// @param id the alarm
/// @param data unused
/// @return 0 (one-shot; the next note sets its own)
static int64_t note_done(alarm_id_t id, void *data) {
    (void)id;
    (void)data;
    if (note_tail == note_head) {
        tone(0);
        note_alarm = 0;
        return 0;
    }
    int t = note_tail;
    tone(notes[t].freq);
    note_tail = (t + 1) % NOTES;
    return (int64_t)(notes[t].ms ? notes[t].ms : 1) * 1000;  // the alarm comes back after it
}

bool port_play(unsigned freq_hz, unsigned ms) {
    int next = (note_head + 1) % NOTES;
    if (next == note_tail)
        return false;
    notes[note_head].freq = freq_hz;
    notes[note_head].ms = ms > 65535 ? 65535 : ms;
    note_head = next;
    if (!note_alarm) {
        uint32_t irq = save_and_disable_interrupts();
        if (!note_alarm)
            note_alarm = add_alarm_in_us(1, note_done, NULL, true);
        restore_interrupts(irq);
    }
    return true;
}

int port_play_pending(void) {
    return (note_head - note_tail + NOTES) % NOTES + (note_alarm ? 1 : 0);
}

void port_play_stop(void) {
    if (note_alarm)
        cancel_alarm(note_alarm);
    note_alarm = 0;
    note_head = note_tail = 0;
    tone(0);
}

void port_beep(unsigned freq_hz, unsigned ms) {
    port_play_stop();
    tone(freq_hz);
    sleep_ms(ms);
    tone(0);
}

// ---- What a screen would do -----------------------------------------------------

void port_get_cell(int row, int col, uint8_t *ch, uint8_t *attr) {
    (void)row;
    (void)col;
    *ch = ' ';  // the terminal can't be read back
    *attr = 7;
}

void port_set_cell(int row, int col, uint8_t ch, uint8_t attr) {
    (void)row;
    (void)col;
    (void)ch;
    (void)attr;
}

bool port_shell(const char *command) {
    (void)command;
    return false;
}

void port_edit(const char *path) {
    term_edit(path, true);
}

void port_edit_file(const char *path) {
    term_edit(path, false);
}

const char *port_set_pages(int active, int visual) {
    return active || visual ? NO_SCREEN : NULL;
}

void port_reset_pages(void) {
}

const char *port_copy_page(int from, int to) {
    (void)from;
    (void)to;
    return NO_SCREEN;
}

void port_fill_rect(int x, int y, int w, int h, uint8_t rgb332) {
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)rgb332;
}

uint8_t port_get_pixel(int x, int y) {
    (void)x;
    (void)y;
    return 0;
}

const char *port_image(const char *path, int x, int y, int w, int h, int sx, int sy, bool fit,
                       int cx0, int cy0, int cx1, int cy1) {
    (void)path, (void)x, (void)y, (void)w, (void)h, (void)sx, (void)sy, (void)fit;
    (void)cx0, (void)cy0, (void)cx1, (void)cy1;
    return NO_SCREEN;
}

const char *port_save_image(const char *path) {
    (void)path;
    return NO_SCREEN;
}

bool image_load(const char *path, int want_w, int want_h, image_t *img, const char **err) {
    (void)path, (void)want_w, (void)want_h;
    memset(img, 0, sizeof *img);
    *err = NO_SCREEN;
    return false;
}

void image_free(image_t *img) {
    (void)img;
}

bool spr_load(const char *path, spr_info_t *info, spr_put_fn put, void *ctx, const char **err) {
    (void)path, (void)info, (void)put, (void)ctx;
    *err = NO_SCREEN;
    return false;
}

bool port_mouse(int *x, int *y, int *buttons) {
    *x = *y = *buttons = 0;
    return false;  // no mouse: MOUSEX, MOUSEY and MOUSEB are 0
}

int port_mouse_wheel(void) {
    return 0;
}

void port_mouse_pointer(bool on) {
    (void)on;
}

bool port_key_down(int scan) {
    (void)scan;
    return false;  // a terminal sends keys, not whether they're held
}

const char *port_music_play(const char *path, bool loop) {
    (void)path;
    (void)loop;
    return NO_SCREEN;
}

void port_music_stop(void) {
}

void port_music_pause(bool paused) {
    (void)paused;
}

void port_music_volume(int percent) {
    (void)percent;
}

void port_music_pos(int order, int row) {
    (void)order;
    (void)row;
}

int port_music_state(int what) {
    (void)what;
    return 0;
}

// ---- Time ------------------------------------------------------------------------

void port_sleep_ms(unsigned ms) {
    sleep_ms(ms);
}

double port_seconds(void) {
    return to_us_since_boot(get_absolute_time()) / 1e6;
}

// ---- Memory: big blocks in PSRAM (if there is any), small ones in SRAM ------------
// The PSRAM allocator walks its block list on every call, so the many small,
// short-lived blocks (strings, temporaries, program lines) stay in SRAM, which
// is faster too; arrays and other big blocks go to PSRAM. When SRAM runs low,
// small blocks go to PSRAM as well. With no PSRAM it is all SRAM.

/// Allocations from this size up go to PSRAM.
#define BIG_BLOCK  512
#define SRAM_KEEP  (16 * 1024)  ///< left for the system and the flash disk

/// The SRAM heap's free bytes (above it and freed inside).
/// @return them
static size_t sram_free(void) {
    extern char __StackLimit;  // top of the heap, from the SDK's linker script
    struct mallinfo mi = mallinfo();
    return (size_t)(&__StackLimit - (char *)sbrk(0)) + mi.fordblks;
}

/// Whether memory is in PSRAM.
/// @param p the memory
/// @return it is
static bool is_psram(const void *p) {
    return (uintptr_t)p >= 0x11000000u && (uintptr_t)p < 0x12000000u;
}

/// True if SRAM has room for n more bytes. Cheap in the usual case: the
/// untouched space above the heap (mallinfo walks the whole heap, so it's only
/// asked when that space is short).
/// @param n the bytes
/// @return there is room
static bool sram_room(size_t n) {
    extern char __StackLimit;
    if ((size_t)(&__StackLimit - (char *)sbrk(0)) >= n + SRAM_KEEP)
        return true;
    return sram_free() >= n + SRAM_KEEP;
}

void *port_alloc(size_t n) {
    if (!n)
        n = 1;
    if (n < BIG_BLOCK && sram_room(n))  // the usual case
        return malloc(n);
    void *p = psram_malloc(n);  // NULL without PSRAM
    if (p)
        return p;
    // PSRAM full or missing: SRAM, while there's still a safe margin.
    return sram_free() >= n + SRAM_KEEP / 2 ? malloc(n) : NULL;
}

void *port_calloc(size_t count, size_t size) {
    if (size && count > SIZE_MAX / size)
        return NULL;
    void *p = port_alloc(count * size);
    if (p)
        memset(p, 0, count * size);
    return p;
}

void port_release(void *p) {
    if (is_psram(p))
        psram_free(p);
    else
        free(p);
}

void *port_realloc(void *p, size_t n) {
    if (!p)
        return port_alloc(n);
    if (!n) {
        port_release(p);
        return NULL;
    }
    size_t old = is_psram(p) ? psram_block_size(p) : malloc_usable_size(p);
    if (is_psram(p)) {
        void *q = psram_realloc(p, n);
        if (q)
            return q;
    } else if ((n < BIG_BLOCK || !psram_present()) && sram_room(n)) {
        return realloc(p, n);
    }
    // Move it (SRAM to PSRAM as it grows, or PSRAM full: try SRAM).
    void *q = port_alloc(n);
    if (!q)
        return NULL;
    memcpy(q, p, old < n ? old : n);
    port_release(p);
    return q;
}

size_t port_free_mem(void) {
    size_t s = sram_free();
    return (s > SRAM_KEEP ? s - SRAM_KEEP : 0) + psram_free_total();
}

/// BASIC's expression evaluator recurses; it runs on a stack of its own (see
/// main.c) and checks it has this much left, for interrupts and the deepest
/// leaf calls.
#define STACK_MARGIN 2048

bool port_stack_low(void) {
    char here;
    return &here < stack_bottom() + STACK_MARGIN;
}

// ---- GPIO -------------------------------------------------------------------
// The RP2040 (Pico) and RP2350A (Pico 2) have GPIO 0-29 with ADC on GP26-29; the RP2350B (Pico
// Plus 2) has GPIO 0-47 with ADC on GP40-47.

#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

/// GPIO pins.
#define PIN_COUNT NUM_BANK0_GPIOS
#if defined(PICO_DEFAULT_LED_PIN) || !PICO_BASIC_WIFI
/// The LED's pin.
#define LED_PIN   PICO_DEFAULT_LED_PIN
/// Pins BASIC knows: the GPIOs.
#define PINS      PIN_COUNT
#else
/// A W board's LED is on the radio chip: BASIC calls it the pin after the
/// last GPIO (PIN LED, 1 works; PWM just turns it on or off).
#define LED_PIN   PIN_COUNT
#define LED_ON_RADIO 1
#define PINS      (PIN_COUNT + 1)
#endif
/// The first ADC pin.
#define ADC_FIRST ADC_BASE_PIN
/// The ADC pins (the last channel is the temperature sensor).
#define ADC_PINS  (NUM_ADC_CHANNELS - 1)

/// What a pin is being used for.
enum { UNUSED, INPUT, OUTPUT, PULLUP, PULLDOWN, PWM, ADC };
static uint8_t pin_use[PINS];           ///< what BASIC has set each pin to
/// Each PWM slice's frequency.
static float slice_freq[NUM_PWM_SLICES];

/// "1000" or "8.5": the SDK's printf has no %g.
/// @param f the frequency
/// @param buf the text
/// @param size room in buf
/// @return buf
static const char *freq_text(double f, char *buf, size_t size) {
    long whole = (long)(f + 0.05);
    int tenth = (int)((f + 0.05 - whole) * 10);
    if (tenth)
        snprintf(buf, size, "%ld.%d", whole, tenth);
    else
        snprintf(buf, size, "%ld", whole);
    return buf;
}

int port_pin_count(void) {
    return PINS;
}

int port_led_pin(void) {
    return LED_PIN;
}

/// COM1's TX and RX pins while it is open (-1: closed).
///@{
static int serial_tx = -1, serial_rx = -1;
///@}
static int i2c_pins[2] = {-1, -1};          ///< SDA, SCL while I2C is open
static int spi_pins[3] = {-1, -1, -1};      ///< SCK, MOSI, MISO while SPI is open
/// Its flow control pins (CTS, RTS) while that is on.
///@{
static int serial_cts = -1, serial_rts = -1;
///@}

/// The NEOPIXEL strip's pin while one is open (-1: none).
static int pixel_pin = -1;

const char *port_pin_reserved(int pin) {
#if PICO_BASIC_WIFI
    if (pin == CYW43_DEFAULT_PIN_WL_REG_ON || pin == CYW43_DEFAULT_PIN_WL_CS ||
        pin == CYW43_DEFAULT_PIN_WL_CLOCK || pin == CYW43_DEFAULT_PIN_WL_DATA_OUT)
        return "the Wi-Fi chip";
#endif
    if (pin == serial_tx || pin == serial_rx || pin == serial_cts || pin == serial_rts)
        return "the COM1 serial port (CLOSE it first)";
    if (pin >= 0 && (pin == i2c_pins[0] || pin == i2c_pins[1]))
        return "I2C (I2C CLOSE first)";
    if (pin >= 0 && (pin == spi_pins[0] || pin == spi_pins[1] || pin == spi_pins[2]))
        return "SPI (SPI CLOSE first)";
    if (pin >= 0 && pin == SPEAKER_PIN)
        return "the speaker (BEEP, SOUND, PLAY)";
#ifdef PICO_PSRAM_CS_PIN
    if (pin == PICO_PSRAM_CS_PIN)
        return "PSRAM";
#endif
#if PICO_BASIC_SD
    if (sd_card_in_use() && sd_card_pin(pin))
        return "the SD card";
#endif
    if (pin >= 0 && pin == pixel_pin)
        return "NEOPIXEL (NEOPIXEL CLOSE first)";
    return NULL;
}

void port_pin_mode(int pin, int mode) {
#ifdef LED_ON_RADIO
    if (pin == LED_PIN) {
        pin_use[pin] = mode == PORT_PIN_OUTPUT ? OUTPUT : INPUT;
        return;
    }
#endif
    if (pin_use[pin] == PWM)
        port_pwm(pin, 0, 0);
    gpio_init(pin);
    gpio_set_dir(pin, mode == PORT_PIN_OUTPUT);
    gpio_set_pulls(pin, mode == PORT_PIN_PULLUP, mode == PORT_PIN_PULLDOWN);
    pin_use[pin] = mode == PORT_PIN_OUTPUT   ? OUTPUT
                   : mode == PORT_PIN_PULLUP ? PULLUP
                   : mode == PORT_PIN_PULLDOWN ? PULLDOWN
                                               : INPUT;
}

void port_pin_write(int pin, int level) {
    if (pin_use[pin] != OUTPUT)
        port_pin_mode(pin, PORT_PIN_OUTPUT);
#ifdef LED_ON_RADIO
    if (pin == LED_PIN) {
        wifi_led(level != 0);
        return;
    }
#endif
    gpio_put(pin, level != 0);
}

int port_pin_read(int pin) {
#ifdef LED_ON_RADIO
    if (pin == LED_PIN)
        return 0;  // (it's an output)
#endif
    if (pin_use[pin] == UNUSED || pin_use[pin] == PWM || pin_use[pin] == ADC)
        port_pin_mode(pin, PORT_PIN_INPUT);
    return gpio_get(pin);
}

void port_pwm(int pin, double freq, double duty) {
#ifdef LED_ON_RADIO
    if (pin == LED_PIN) {
        port_pin_write(pin, freq > 0 && duty >= 50);
        return;
    }
#endif
    uint slice = pwm_gpio_to_slice_num(pin);
    uint partner = pin ^ 1;
    if (freq <= 0) {
        if (pin_use[pin] == PWM) {
            pwm_set_gpio_level(pin, 0);
            if (partner >= PIN_COUNT || pin_use[partner] != PWM)
                pwm_set_enabled(slice, false);
        }
        gpio_init(pin);  // back to a plain input
        pin_use[pin] = INPUT;
        return;
    }
    // Pick the smallest divider that lets the counter's top fit 16 bits,
    // for the finest duty steps.
    float clk = (float)clock_get_hz(clk_sys);
    float div = clk / ((float)freq * 65536.0f);
    if (div < 1.0f)
        div = 1.0f;
    if (div > 255.9f)
        div = 255.9f;
    uint32_t top = (uint32_t)(clk / (div * (float)freq)) - 1;
    if (top > 65535)
        top = 65535;
    if (partner < PIN_COUNT && pin_use[partner] == PWM && slice_freq[slice] != (float)freq) {
        char text[16];
        printf("Warning: GP%u shares a PWM slice with GP%d; both now run at %s Hz\n", partner,
               pin, freq_text(freq, text, sizeof text));
    }
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, top);
    if (duty < 0)
        duty = 0;
    if (duty > 100)
        duty = 100;
    pwm_set_gpio_level(pin, (uint32_t)((top + 1) * duty / 100.0 + 0.5));
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_set_enabled(slice, true);
    slice_freq[slice] = (float)freq;
    pin_use[pin] = PWM;
}

/// Starts the ADC, once.
static void adc_start(void) {
    static bool started;
    if (!started) {
        adc_init();
        started = true;
    }
}

int port_adc(int pin) {
    if (pin < ADC_FIRST || pin >= ADC_FIRST + ADC_PINS)
        return -1;
    adc_start();
    if (pin_use[pin] != ADC) {
        if (pin_use[pin] == PWM)
            port_pwm(pin, 0, 0);
        adc_gpio_init(pin);
        pin_use[pin] = ADC;
    }
    adc_select_input(pin - ADC_FIRST);
    return adc_read();
}

double port_temperature(void) {
    adc_start();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(ADC_TEMPERATURE_CHANNEL_NUM);
    uint32_t sum = 0;
    for (int i = 0; i < 16; ++i)  // the sensor is noisy
        sum += adc_read();
    double volts = sum / 16.0 * 3.3 / 4096;
    return 27 - (volts - 0.706) / 0.001721;  // the RP2040 and RP2350 datasheets' formula
}

void port_pins_list(void) {
    static const char *const names[] = {"", "in", "out", "in up", "in down", "PWM", "ADC"};
    int rows = (PIN_COUNT + 3) / 4;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < 4; ++col) {
            int pin = col * rows + row;
            if (pin >= PIN_COUNT) {
                printf("\n");
                break;
            }
            const char *why = port_pin_reserved(pin);
            char what[24];
            if (pin == i2c_pins[0] || pin == i2c_pins[1])
                snprintf(what, sizeof what, "I2C%d %s", (pin / 2) & 1, pin == i2c_pins[0] ? "SDA" : "SCL");
            else if (pin == spi_pins[0] || pin == spi_pins[1] || pin == spi_pins[2])
                snprintf(what, sizeof what, "SPI%d %s", (pin / 8) & 1,
                         pin == spi_pins[0] ? "SCK" : pin == spi_pins[1] ? "MOSI" : "MISO");
            else if (pin == serial_tx || pin == serial_rx || pin == serial_cts || pin == serial_rts)
                snprintf(what, sizeof what, "COM1 %s", pin == serial_tx ? "TX" : pin == serial_rx ? "RX"
                                                       : pin == serial_cts ? "CTS" : "RTS");
            else if (pin == pixel_pin)
                snprintf(what, sizeof what, "NeoPixel x%d", port_pixels_count());
            else if (why)
                snprintf(what, sizeof what, "%s", pin == SPEAKER_PIN ? "speaker"
#if PICO_BASIC_SD
                                                  : sd_card_pin(pin) ? "SD card"
#endif
                                                  : "PSRAM");
            else if (pin_use[pin] == UNUSED)
                snprintf(what, sizeof what, "free%s", pin == LED_PIN ? " (LED)" : "");
            else if (pin_use[pin] == PWM) {
                char text[16];
                snprintf(what, sizeof what, "PWM %sHz",
                         freq_text(slice_freq[pwm_gpio_to_slice_num(pin)], text, sizeof text));
            }
            else if (pin_use[pin] == ADC)
                snprintf(what, sizeof what, "ADC");
            else
                snprintf(what, sizeof what, "%s %d", names[pin_use[pin]], gpio_get(pin));
            printf("GP%-2d %-14s%s", pin, what, col < 3 ? " " : "\n");
        }
    }
}

void port_i2c_close(void);
void port_spi_close(void);
void port_pixels_close(void);
int port_pixels_count(void);

void port_pins_reset(void) {
    port_i2c_close();
    port_spi_close();
    port_pixels_close();
    for (int pin = 0; pin < PIN_COUNT; ++pin) {
        if (pin_use[pin] == UNUSED)
            continue;
        if (pin_use[pin] == PWM)
            port_pwm(pin, 0, 0);
        gpio_init(pin);
        gpio_set_pulls(pin, false, false);
        pin_use[pin] = UNUSED;
    }
}

// ---- Serial port: COM1 is UART1 -----------------------------------------------
// Received bytes go into a ring buffer from the UART's interrupt, so none are
// lost while BASIC is busy.

#include "hardware/irq.h"
#include "hardware/uart.h"

/// The receive ring's size.
#define RX_SIZE 4096
static uint8_t *rx_buf;  ///< RX_SIZE, while COM1 is open (SRAM is short)
/// The ring's ends: the interrupt writes head, BASIC reads tail.
///@{
static volatile uint16_t rx_head, rx_tail;
///@}

/// UART1's interrupt: received bytes into the ring.
static void serial_irq(void) {
    while (uart_is_readable(uart1)) {
        uint16_t next = (rx_head + 1) % RX_SIZE;
        if (next == rx_tail) {
            // Full. With flow control, leave the rest in the UART so its
            // FIFO fills and RTS tells the sender to wait; reading resumes
            // when there's room. Without, the byte is lost either way.
            if (serial_rts >= 0) {
                uart_set_irq_enables(uart1, false, false);
                return;
            }
            uart_getc(uart1);
            continue;
        }
        rx_buf[rx_head] = uart_getc(uart1);
        rx_head = next;
    }
}

/// Whether a pin can be UART1's TX (or RX).
/// @param pin the pin
/// @param tx TX (else RX)
/// @return it can
static bool uart1_pin(int pin, bool tx) {
    // UART1's TX/RX pins below GP30, as on every RP2 chip (24 and 25 aren't
    // on the Pico 2's header: VBUS sense and the LED).
    static const int8_t txs[] = {4, 8, 20, 24}, rxs[] = {5, 9, 21, 25};
    for (int i = 0; i < 4; ++i)
        if (pin == (tx ? txs[i] : rxs[i]))
            return true;
    return false;
}

const char *port_serial_open(unsigned baud, int data_bits, char parity, int stop_bits, int tx,
                             int rx) {
    if (!uart1_pin(tx, true) || !uart1_pin(rx, false))
        return "COM1 can use TX on GP4, 8, 20 or 24 and RX on GP5, 9, 21 or 25";
    const char *why = port_pin_reserved(tx);
    if (!why)
        why = port_pin_reserved(rx);
    if (why)
        return why;
    if (pin_use[tx] != UNUSED || pin_use[rx] != UNUSED)
        return "a pin is in use by PINMODE/PIN/PWM";
    if (!rx_buf && !(rx_buf = malloc(RX_SIZE)))
        return "not enough memory for COM1's buffer";
    uart_init(uart1, baud);
    uart_set_format(uart1, data_bits, stop_bits,
                    parity == 'E' ? UART_PARITY_EVEN : parity == 'O' ? UART_PARITY_ODD
                                                     : UART_PARITY_NONE);
    uart_set_hw_flow(uart1, false, false);
    uart_set_fifo_enabled(uart1, true);
    gpio_set_function(tx, GPIO_FUNC_UART);
    gpio_set_function(rx, GPIO_FUNC_UART);
    gpio_pull_up(rx);  // an unconnected input reads as idle, not as noise
    rx_head = rx_tail = 0;
    irq_set_exclusive_handler(UART1_IRQ, serial_irq);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(uart1, true, false);
    serial_tx = tx;
    serial_rx = rx;
    return NULL;
}

// RTS/CTS flow control on the two pins after TX (TX+2 CTS in, TX+3 RTS out:
// GP6/7 for GP4/5, and so on). NULL if done, else why not.
const char *port_serial_flow(bool on) {
    if (serial_tx < 0)
        return "COM1 isn't open";
    if (on && serial_cts < 0) {
        int cts = serial_tx + 2, rts = serial_tx + 3;
        const char *why = port_pin_reserved(cts);
        if (!why)
            why = port_pin_reserved(rts);
        if (why)
            return why;
        if (pin_use[cts] != UNUSED || pin_use[rts] != UNUSED)
            return "a flow control pin is in use by PINMODE/PIN/PWM";
        gpio_set_function(cts, GPIO_FUNC_UART);
        gpio_set_function(rts, GPIO_FUNC_UART);
        gpio_pull_down(cts);  // nothing connected: never clear to send
        serial_cts = cts;
        serial_rts = rts;
        uart_set_hw_flow(uart1, true, true);
    } else if (!on && serial_cts >= 0) {
        uart_set_hw_flow(uart1, false, false);
        gpio_init(serial_cts);
        gpio_init(serial_rts);
        gpio_set_pulls(serial_cts, false, false);
        serial_cts = serial_rts = -1;
        uart_set_irq_enables(uart1, true, false);
    }
    return NULL;
}

// Holds TX low (a BREAK) for `ms` milliseconds.
void port_serial_break(unsigned ms) {
    if (serial_tx < 0)
        return;
    uart_tx_wait_blocking(uart1);
    uart_set_break(uart1, true);
    sleep_ms(ms);
    uart_set_break(uart1, false);
}

void port_serial_close(void) {
    if (serial_tx < 0)
        return;
    port_serial_flow(false);
    uart_set_irq_enables(uart1, false, false);
    irq_set_enabled(UART1_IRQ, false);
    irq_remove_handler(UART1_IRQ, serial_irq);
    uart_tx_wait_blocking(uart1);  // let the last bytes go out
    uart_deinit(uart1);
    gpio_init(serial_tx);
    gpio_init(serial_rx);
    gpio_set_pulls(serial_rx, false, false);
    serial_tx = serial_rx = -1;
    free(rx_buf);
    rx_buf = NULL;
    rx_head = rx_tail = 0;
}

void port_serial_write(const char *data, int n) {
    uart_write_blocking(uart1, (const uint8_t *)data, n);
}

int port_serial_getc(void) {
    if (rx_tail == rx_head)
        return -1;
    int c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % RX_SIZE;
    if (serial_rts >= 0)
        uart_set_irq_enables(uart1, true, false);  // in case it stopped when full
    return c;
}

int port_serial_waiting(void) {
    return (rx_head - rx_tail + RX_SIZE) % RX_SIZE;
}

int port_serial_room(void) {
    return RX_SIZE - 1 - port_serial_waiting();
}

// ---- I2C and SPI ------------------------------------------------------------------
// The chip's own controllers. Which one a pin belongs to follows from its
// number: I2C SDA on even pins and SCL on odd ones, I2C0 or I2C1 by (pin / 2)
// mod 2; SPI in groups of four (MISO, CS, SCK, MOSI), SPI0 or SPI1 by
// (pin / 8) mod 2.

#include "hardware/i2c.h"
#include "hardware/spi.h"

/// The I2C controller open, or NULL.
static i2c_inst_t *i2c_bus;
/// The SPI controller open, or NULL.
static spi_inst_t *spi_bus;

/// Why a pin can't be used, or NULL.
/// @param pin the pin
/// @return the reason, or NULL
static const char *pin_free(int pin) {
    if (pin < 0 || pin >= PIN_COUNT)
        return "no such pin";
    const char *why = port_pin_reserved(pin);
    if (why)
        return why;
    if (pin_use[pin] != UNUSED)
        return "a pin is in use by PINMODE/PIN/PWM";
    return NULL;
}

const char *port_i2c_open(int sda, int scl, int khz) {
    port_i2c_close();
    if (sda < 0 || scl < 0 || sda >= PIN_COUNT || scl >= PIN_COUNT || (sda & 1) || scl != sda + 1)
        return "I2C needs SDA on an even pin and SCL on the next one (e.g. GP2, GP3)";
    const char *why = pin_free(sda);
    if (!why)
        why = pin_free(scl);
    if (why)
        return why;
    if (khz < 10 || khz > 1000)
        return "the speed must be 10-1000 kHz";
    i2c_bus = (sda / 2) & 1 ? i2c1 : i2c0;
    i2c_init(i2c_bus, khz * 1000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);  // weak: devices should bring their own pull-ups
    gpio_pull_up(scl);
    i2c_pins[0] = sda;
    i2c_pins[1] = scl;
    return NULL;
}

void port_i2c_close(void) {
    if (!i2c_bus)
        return;
    i2c_deinit(i2c_bus);
    for (int i = 0; i < 2; ++i) {
        gpio_init(i2c_pins[i]);
        gpio_set_pulls(i2c_pins[i], false, false);
        i2c_pins[i] = -1;
    }
    i2c_bus = NULL;
}

// Bytes written, or -1 if no device answered (or it stopped answering), -2
// if I2C isn't open.
int port_i2c_write(int addr, const uint8_t *data, int n, bool nostop) {
    if (!i2c_bus)
        return -2;
    int r = i2c_write_timeout_us(i2c_bus, (uint8_t)addr, data, n, nostop, 2000 + n * 200);
    return r < 0 ? -1 : r;
}

int port_i2c_read(int addr, uint8_t *data, int n) {
    if (!i2c_bus)
        return -2;
    int r = i2c_read_timeout_us(i2c_bus, (uint8_t)addr, data, n, false, 2000 + n * 200);
    return r < 0 ? -1 : r;
}

const char *port_spi_open(int sck, int mosi, int miso, int khz, int mode) {
    port_spi_close();
    if (sck < 0 || mosi < 0 || miso < 0 || sck >= PIN_COUNT || mosi >= PIN_COUNT || miso >= PIN_COUNT ||
        sck % 4 != 2 || mosi % 4 != 3 || miso % 4 != 0 || ((sck / 8) & 1) != ((mosi / 8) & 1) ||
        ((sck / 8) & 1) != ((miso / 8) & 1))
        return "SPI needs SCK, MOSI and MISO of one SPI (e.g. GP6, GP7, GP4)";
    const char *why = pin_free(sck);
    if (!why)
        why = pin_free(mosi);
    if (!why)
        why = pin_free(miso);
    if (why)
        return why;
    if (khz < 1 || khz > 50000 || mode < 0 || mode > 3)
        return "the speed must be 1-50000 kHz and the mode 0-3";
#if PICO_BASIC_SD
    if (sd_card_in_use() && ((sck / 8) & 1) == sd_card_spi())
        return sd_card_spi() ? "SPI1 is used by the SD card" : "SPI0 is used by the SD card";
#endif
    spi_bus = (sck / 8) & 1 ? spi1 : spi0;
    spi_init(spi_bus, khz * 1000);
    spi_set_format(spi_bus, 8, (mode & 2) ? SPI_CPOL_1 : SPI_CPOL_0, (mode & 1) ? SPI_CPHA_1 : SPI_CPHA_0,
                   SPI_MSB_FIRST);
    gpio_set_function(sck, GPIO_FUNC_SPI);
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(miso, GPIO_FUNC_SPI);
    spi_pins[0] = sck;
    spi_pins[1] = mosi;
    spi_pins[2] = miso;
    return NULL;
}

void port_spi_close(void) {
    if (!spi_bus)
        return;
    spi_deinit(spi_bus);
    for (int i = 0; i < 3; ++i) {
        gpio_init(spi_pins[i]);
        gpio_set_pulls(spi_pins[i], false, false);
        spi_pins[i] = -1;
    }
    spi_bus = NULL;
}

// Sends n bytes and keeps the n that came back (in may be NULL). False if
// SPI isn't open.
bool port_spi_transfer(const uint8_t *out, uint8_t *in, int n) {
    if (!spi_bus)
        return false;
    if (in)
        spi_write_read_blocking(spi_bus, out, in, n);
    else
        spi_write_blocking(spi_bus, out, n);
    return true;
}

// ---- XMODEM over the console: XSEND and XRECV ---------------------------------------
// xfer.c (from pico-os) does the protocol; this gives it the USB console
// (raw: stdio's printf would turn LF into CR LF) and files on the flash disk.

#include "ff.h"
#include "xfer.h"

/// A file being sent or received.
typedef struct {
    FIL f;         ///< the file
    bool writing;  ///< being received
} xfile_t;

/// XRECV has created its file (so a broken download is deleted, but not a
/// file of that name that was there before nothing arrived).
static bool xrecv_created;

/// A byte from the console, waiting up to timeout_ms.
/// @param ctx unused
/// @param timeout_ms how long to wait
/// @return the byte, or XFER_TIMEOUT
static int x_getc(void *ctx, unsigned timeout_ms) {
    (void)ctx;
    int c = getchar_timeout_us(timeout_ms * 1000u);  // input is never translated
    return c == PICO_ERROR_TIMEOUT ? XFER_TIMEOUT : c;
}

/// Sends bytes to the console.
/// @param ctx unused
/// @param data the bytes
/// @param len their count
static void x_write(void *ctx, const uint8_t *data, int len) {
    (void)ctx;
    stdio_usb.out_chars((const char *)data, len);  // the driver itself: no CR LF
}

/// Opens the file to send.
/// @param ctx unused
/// @param path its path
/// @param size set to its size
/// @param name set to its name (XMODEM doesn't send it)
/// @param name_size room in name
/// @return the file, or NULL
static void *x_open_read(void *ctx, const char *path, uint32_t *size, char *name, int name_size) {
    (void)ctx;
    xfile_t *x = malloc(sizeof *x);
    if (!x)
        return NULL;
    if (f_open(&x->f, path, FA_READ) != FR_OK) {
        free(x);
        return NULL;
    }
    x->writing = false;
    *size = f_size(&x->f);
    snprintf(name, name_size, "%s", path);
    return x;
}

/// Opens the file to receive into, replacing one of that name (as SAVE does).
/// @param ctx unused
/// @param name its name
/// @param size unused (XMODEM doesn't send it)
/// @param resume unused (ZMODEM only)
/// @param offset set to 0
/// @param saved_as set to the name
/// @param saved_as_size room in saved_as
/// @return the file, or NULL
static void *x_open_write(void *ctx, const char *name, uint32_t size, bool resume, uint32_t *offset,
                          char *saved_as, int saved_as_size) {
    (void)ctx, (void)size, (void)resume;
    xfile_t *x = malloc(sizeof *x);
    if (!x)
        return NULL;
    if (f_open(&x->f, name, FA_READ | FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
        free(x);
        return NULL;
    }
    x->writing = true;
    xrecv_created = true;
    *offset = 0;
    snprintf(saved_as, saved_as_size, "%s", name);
    return x;
}

/// Reads from the file being sent.
/// @param ctx unused
/// @param file the file
/// @param buf where
/// @param len how many
/// @return bytes read, or -1
static int x_read(void *ctx, void *file, uint8_t *buf, int len) {
    (void)ctx;
    UINT got;
    return f_read(&((xfile_t *)file)->f, buf, len, &got) == FR_OK ? (int)got : -1;
}

/// Seeks in the file being sent (ZMODEM only).
/// @param ctx unused
/// @param file the file
/// @param offset where
/// @return done
static bool x_seek(void *ctx, void *file, uint32_t offset) {
    (void)ctx;
    return f_lseek(&((xfile_t *)file)->f, offset) == FR_OK;
}

/// Writes received bytes to the file.
/// @param ctx unused
/// @param file the file
/// @param buf the bytes
/// @param len their count
/// @return written
static bool x_write_file(void *ctx, void *file, const uint8_t *buf, int len) {
    (void)ctx;
    UINT done;
    return f_write(&((xfile_t *)file)->f, buf, len, &done) == FR_OK && done == (UINT)len;
}

/// XMODEM pads the last block with ^Z (SUB): takes them off the end of a
/// received file, so it comes back the size it was sent.
/// @param f the file
static void strip_padding(FIL *f) {
    FSIZE_t size = f_size(f);
    uint8_t tail[128];
    UINT got;
    FSIZE_t start = size > sizeof tail ? size - sizeof tail : 0;
    if (f_lseek(f, start) != FR_OK || f_read(f, tail, (UINT)(size - start), &got) != FR_OK)
        return;
    while (got && tail[got - 1] == 0x1a)
        --got;
    if (start + got < size && f_lseek(f, start + got) == FR_OK)
        f_truncate(f);
}

/// Closes the file; a broken download is deleted.
/// @param ctx unused
/// @param file the file
/// @param ok it all arrived
static void x_close(void *ctx, void *file, bool ok) {
    (void)ctx;
    xfile_t *x = file;
    if (x->writing && ok)
        strip_padding(&x->f);
    f_close(&x->f);
    // FatFs keeps no name in a FIL: x_open_write's name is the one the
    // caller passed, so a broken download is deleted there (port_xrecv).
    free(x);
}

/// The transfers' callbacks.
static const xfer_io_t xmodem_io = {NULL,   x_getc, x_write,      x_open_read, x_open_write,
                                    x_read, x_seek, x_write_file, x_close,     NULL};

bool port_xsend(const char *path, char *msg, int size) {
    stdio_flush();
    xfer_result_t res;
    const char *paths[1] = {path};
    xfer_send(&xmodem_io, XFER_XMODEM_1K, paths, 1, &res);
    snprintf(msg, size, "%s", res.message);
    return res.ok;
}

bool port_xrecv(const char *path, char *msg, int size) {
    stdio_flush();
    xfer_result_t res;
    xrecv_created = false;
    xfer_receive(&xmodem_io, XFER_XMODEM, path, &res);
    if (!res.ok && xrecv_created)
        f_unlink(path);
    snprintf(msg, size, "%s", res.message);
    return res.ok;
}

// ---- NEOPIXEL: WS2812B LEDs on a PIO state machine ----------------------------------

#include "hardware/pio.h"
#include "ws2812.pio.h"

static PIO pixel_pio;  ///< the PIO block in use
static uint pixel_sm, pixel_offset;  ///< its state machine and where the program is
static uint32_t *pixels;  ///< the colours, 0xRRGGBB
static int pixel_count;  ///< the LEDs (0: none open)
static int pixel_bright = 255;  ///< 0-255, applied when shown
static bool pixel_rgb;  ///< red first (else green, as WS2812B want)

const char *port_pixels_open(int pin, int count, bool rgb) {
    port_pixels_close();
    if (pin_use[pin] != UNUSED)
        return "the pin is in use by PINMODE/PIN/PWM";
    uint32_t *buf = calloc(count, sizeof *buf);
    if (!buf)
        return "not enough memory for the LEDs";
    if (!pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pixel_pio, &pixel_sm,
                                                          &pixel_offset, pin, 1, true)) {
        free(buf);
        return "no free PIO state machine";
    }
    ws2812_program_init(pixel_pio, pixel_sm, pixel_offset, pin, 800000);
    pixels = buf;
    pixel_count = count;
    pixel_pin = pin;
    pixel_rgb = rgb;
    pixel_bright = 255;
    return NULL;
}

void port_pixel_set(int n, int r, int g, int b) {
    if (n >= 0 && n < pixel_count)
        pixels[n] = (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
}

void port_pixels_show(void) {
    for (int i = 0; i < pixel_count; ++i) {
        uint32_t c = pixels[i];
        uint32_t r = (c >> 16 & 255) * pixel_bright / 255, g = (c >> 8 & 255) * pixel_bright / 255,
                 b = (c & 255) * pixel_bright / 255;
        uint32_t word = pixel_rgb ? r << 16 | g << 8 | b : g << 16 | r << 8 | b;
        pio_sm_put_blocking(pixel_pio, pixel_sm, word << 8);  // 24 bits, top first
    }
    // The LEDs take the colours after the line has been low for 50 us (the
    // last LED's bits are still going out when the FIFO empties: 30 us).
    while (!pio_sm_is_tx_fifo_empty(pixel_pio, pixel_sm))
        tight_loop_contents();
    busy_wait_us(100);
}

void port_pixels_bright(int level) {
    pixel_bright = level < 0 ? 0 : level > 255 ? 255 : level;
}

int port_pixels_count(void) {
    return pixel_count;
}

void port_pixels_close(void) {
    if (!pixel_count)
        return;
    pio_sm_set_enabled(pixel_pio, pixel_sm, false);
    pio_remove_program_and_unclaim_sm(&ws2812_program, pixel_pio, pixel_sm, pixel_offset);
    gpio_init(pixel_pin);
    free(pixels);
    pixels = NULL;
    pixel_count = 0;
    pixel_pin = -1;
}

int port_neopixel_pin(void) {
    return config.neopixel;
}

void port_config_show(void) {
    config_print();
}

int port_wifi_value(int n) {
#if PICO_BASIC_WIFI
    return wifi_value(n);
#else
    (void)n;
    return 0;
#endif
}

const char *port_wifi_text(int n) {
#if PICO_BASIC_WIFI
    return wifi_text(n);
#else
    (void)n;
    return "";
#endif
}

// ---- Watchdog ---------------------------------------------------------------------

static int watchdog_timeout;  ///< ms while armed, 0 off
static int watchdog_fired = -1;  ///< the last reset was the watchdog's (-1: not looked yet)

bool port_watchdog_reset(void) {
    if (watchdog_fired < 0)  // (main looks at start-up, before anything arms it)
        watchdog_fired = watchdog_enable_caused_reboot();
    return watchdog_fired;
}

void port_watchdog_set(int ms) {
    port_watchdog_reset();
    if (ms > 0) {
        watchdog_enable((uint32_t)ms, true);  // paused while a debugger halts the chip
        watchdog_timeout = ms;
    } else {
        watchdog_disable();
        watchdog_timeout = 0;
    }
}

void port_watchdog_feed(void) {
    if (watchdog_timeout)
        watchdog_update();
}

void board_watchdog_feed(void) {
    port_watchdog_feed();
}

int port_watchdog_ms(void) {
    return watchdog_timeout;
}

int port_watchdog_max(void) {
#if PICO_RP2040
    return 0xffffff / 2000;  // the RP2040's counter runs at 2 ticks a microsecond
#else
    return 0xffffff / 1000;
#endif
}

bool port_clock_12h(void) {
    return config.clock12;
}

bool port_wifi(int what) {
#if PICO_BASIC_WIFI
    switch (what) {
    case 0: wifi_status(); break;
    case 1: wifi_switch(true); break;
    case 2: wifi_switch(false); break;
    case 3: wifi_scan(); break;
    default: wifi_ntp(); break;
    }
    return true;
#else
    (void)what;
    return false;
#endif
}
