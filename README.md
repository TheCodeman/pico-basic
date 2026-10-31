# Pico BASIC (standalone)

A GW-BASIC style BASIC for the RP2350 and RP2040 on their own: no HDMI, no keyboard or
USB drive. You use it from a terminal program on the PC, over the board's own
USB port, and it keeps its files in the Pico's own flash.

The manual is [docs/BASIC_MANUAL.md](docs/BASIC_MANUAL.md), with an A-Z
reference of every command, function and operator in
[docs/BASIC_REFERENCE.md](docs/BASIC_REFERENCE.md); both are in the
[PDF](docs/BASIC_MANUAL.pdf) and [HTML](docs/BASIC_MANUAL.html). Remake them
with `python3 tools/make_reference.py` (which also checks every keyword in
`src/basic.c` has an entry) and `python3 tools/make_manual.py`.

The interpreter (`src/basic.c`) is copied from pico-os (`src/basic/`). It is
built with `BASIC_NO_SCREEN`, which makes the features that need a screen of
their own give `Feature not available`. `src/basic_port_serial.c` is this
board's side of `basic_port.h`.

## Boards

- **Raspberry Pi Pico 2** (RP2350A, 4 MB flash): the default. It has a 3 MB
  flash disk and BASIC gets about 450 KB of SRAM.
- **Pimoroni Pico Plus 2** (RP2350B, 16 MB flash, 8 MB PSRAM): it has a 15 MB
  flash disk. The PSRAM is found at start-up, and big blocks such as arrays
  go there.
- **Raspberry Pi Pico** (RP2040, 2 MB flash): a 1 MB flash disk and about
  190 KB for BASIC. No floating-point hardware, so maths is slower.

## Building

With the Pico SDK 2.3.1:

```bash
export PICO_SDK_PATH=~/.pico-sdk/sdk/2.3.1
cmake -B build -G Ninja
cmake --build build
```

To build for the Pico Plus 2, configure with
`-DPICO_BOARD=pimoroni_pico_plus2_rp2350`, and for the original Pico with
`-DPICO_BOARD=pico` (e.g. `cmake -B build-pico -G Ninja -DPICO_BOARD=pico`). Either build writes
`build/pico_basic.uf2` (in that build folder). To load it, hold BOOTSEL
while plugging the board into USB and copy the file over; once BASIC runs,
`picotool load -f` can reboot it and load a new one through the same port. A
Debug Probe on the SWD pins works too (OpenOCD: see the manual).

Options: `-DSOUND_PIN=18` sets the speaker pin. `-DFLASH_DISK_OFFSET=0x100000`
sets where the disk starts in the flash; the program must fit below it.

## Connecting

The console is the board's USB port, as a USB serial (CDC) device: plug it
into the PC with a data cable and it appears as `/dev/ttyACM*` on Linux
(`usb-Raspberry_Pi_Pico_...` in `/dev/serial/by-id/`), a COM port on
Windows. Open it with any terminal (minicom, picocom, PuTTY, Tera Term), ANSI
or VT100 with UTF-8, at least 80 x 30; the baud rate doesn't matter. For
example: `picocom /dev/ttyACM1`. BASIC waits 3 seconds at start-up for a
terminal, and shows its banner again to one that connects later.

| Pico pin | To |
|---|---|
| USB | the PC: power and the console |
| GP18 (pin 24) | optional: a piezo or small speaker (through about 100 Ω) to GND, for BEEP, SOUND and PLAY |

Use 3.3 V logic levels only. The speaker pin (GP18) is reserved, and so is GP47
(the PSRAM) on the Pico Plus 2: `PIN`, `PWM`, `I2C` and `SPI` refuse them.
Every other pin is free, GP0 and GP1 included. COM1 uses UART1, with TX on
GP8 (the default), GP4 or GP20 and RX on GP9, GP5 or GP21.

## What works

The whole language: the program commands, the statements, the functions,
`PRINT USING`, error trapping, events and data files (`OPEN`, random access,
`FILES`, `KILL`, `NAME`, `MKDIR`, `CHDIR`). Also `COLOR`, `LOCATE` and `CLS`
as ANSI codes, and `INKEY$` with cursor and function keys. Then `BEEP`,
`SOUND` and `PLAY` on the speaker pin; `NEOPIXEL` for WS2812B LEDs (the
Maker Pi Pico's RGB LED on GP28, or strips on any pin); `PIN`, `PWM`, `SERVO`, `ADC` and
`TEMP()`; `I2C` and `SPI`; COM1; and `TIMER`, `DATE$` and `TIME$`. Line
editing has history, `EDIT n` puts a line on the input line, and `EDIT`
opens the program (or `EDIT "file"` any text file) in a full-screen editor
on the terminal (`src/term_editor.c`).

## AUTORUN.BAS

At power-up BASIC runs `AUTORUN.BAS` if there is one, from the SD card first,
else from the flash disk, after waiting up to 3 seconds for a terminal (so an
open one sees its output; a board on its own runs it anyway). Holding GP20 low at
power-up (the Maker Pi Pico's first button), or Esc / Ctrl-C within a second
from an open terminal, skips it; Ctrl-C stops it. `-DAUTORUN_SKIP_PIN=-1`
(or another pin) at build time changes the skip pin.

## The SD card

A microSD card over SPI is a second drive, `SD:` (or `1:`), next to the
flash disk (`FLASH:` or `0:`, the current drive at start-up): FAT16, FAT32
or exFAT, formatted on a PC. `SAVE "SD:GAME"`, `FILES "SD:"`,
`CHDIR "SD:"` (the card becomes the current drive), `XRECV "SD:FILE"`. A
card can be swapped while BASIC runs. The banner shows it, or
`SD card (SD:): none`.

| microSD module | Pico pin |
|---|---|
| CLK (SCK) | GP10 (pin 14) |
| SDI / DI (MOSI) | GP11 (pin 15) |
| SDO / DO (MISO) | GP12 (pin 16) |
| DAT1, DAT2 (if wired) | GP13, GP14 (held up) |
| CS (DAT3) | GP15 (pin 20) |
| VCC | 3V3(OUT) (pin 36), 3.3 V only |
| GND | GND (pin 18) |

That is SPI1, the Cytron Maker Pi Pico's microSD slot as it is wired (its
buzzer, GP18, is the speaker too). BASIC's own `SPI OPEN` then defaults to
SPI0 on GP4-7 (GP6 SCK, GP7 MOSI, GP4 MISO, GP5 free for CS) and COM1 to
GP8/GP9. Build options: `-DPICO_BASIC_SD=OFF`
leaves the card out; `-DSD_SPI=0 -DSD_MISO=16 -DSD_CS=17 -DSD_SCK=18
-DSD_MOSI=19` (for example) choose other pins (one SPI's MISO, SCK and MOSI
pins; CS any). While a card is in, its pins and that SPI are refused to
`PIN`, `PWM` and `SPI OPEN`.

## The flash disk

On the first start-up the disk is formatted as FAT (labelled PICOBASIC).
Writes go through a 4 KB cache, which is written to the flash when a file is
closed. So `CLOSE` your files before you switch off.

Getting files on and off:

- **Programs:** paste the listing into the terminal at the `Ok` prompt (the
  numbered lines go into the program), then `SAVE "NAME"`. To get one back,
  `LOAD` it, `LIST` it and copy the text from the terminal.
- **Any file**, by XMODEM: `XRECV "NAME"` receives one from the terminal,
  and `XSEND "NAME"` sends one. Start the terminal's XMODEM send or receive
  after typing the command:
  - minicom: Ctrl-A S to send, Ctrl-A R to receive (it uses lrzsz).
  - picocom: `--send-cmd "sx -vv" --receive-cmd "rx -vv"`, then Ctrl-A
    Ctrl-S or Ctrl-A Ctrl-R.
  - Tera Term: File > Transfer > XMODEM.
  - A shell, with the terminal closed: `sx FILE < /dev/ttyACM1 > /dev/ttyACM1`
    or `rx FILE < /dev/ttyACM1 > /dev/ttyACM1` (the board's port).

  XMODEM's ^Z padding is taken off files the board receives (the PC's `rx`
  keeps it unless run with `-a`). A failed receive deletes
  its partial file. Ctrl-X a few times cancels a transfer. The protocol code
  (`src/xfer.c`) is pico-os's.
