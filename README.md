# Pico BASIC (standalone)

A GW-BASIC style BASIC for the RP2350 on its own: no HDMI, no USB keyboard
or drive. You use it from a serial terminal, and it keeps its files in the
Pico's own flash.

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

## Building

With the Pico SDK 2.3.1:

```bash
export PICO_SDK_PATH=~/.pico-sdk/sdk/2.3.1
cmake -B build -G Ninja
cmake --build build
```

To build for the Pico Plus 2, configure with
`-DPICO_BOARD=pimoroni_pico_plus2_rp2350`. Either build writes
`build/pico_basic.uf2`. To load it, hold BOOTSEL while plugging the board
into USB and copy the file over (USB only powers the board; the console is
on the UART).

Options: `-DSOUND_PIN=15` sets the speaker pin. `-DFLASH_DISK_OFFSET=0x100000`
sets where the disk starts in the flash; the program must fit below it.

## Wiring

| Pico pin | To |
|---|---|
| GP0 (pin 1), UART0 TX | the terminal's RX (a USB-serial adapter or a Raspberry Pi Debug Probe) |
| GP1 (pin 2), UART0 RX | the terminal's TX |
| GND (pin 3) | the adapter's GND |
| GP15 (pin 20) | a piezo or small speaker (through about 100 Ω) to GND, for BEEP, SOUND and PLAY |

Use 3.3 V logic levels only. Set the terminal to 115200 baud, 8N1, ANSI or
VT100, with UTF-8 so that `CHR$(1)` and the other control-code pictures show.
Suitable terminals include minicom, picocom, PuTTY and Tera Term.
For example: `picocom -b 115200 /dev/ttyACM0`

GP0, GP1 and the speaker pin are reserved, and so is GP47 (the PSRAM) on the
Pico Plus 2: `PIN`, `PWM`, `I2C` and `SPI` refuse them. COM1 uses UART1, with
TX on GP4, GP8 or GP20 and RX on GP5, GP9 or GP21.

## What works

The whole language: the program commands, the statements, the functions,
`PRINT USING`, error trapping, events and data files (`OPEN`, random access,
`FILES`, `KILL`, `NAME`, `MKDIR`, `CHDIR`). Also `COLOR`, `LOCATE` and `CLS`
as ANSI codes, and `INKEY$` with cursor and function keys. Then `BEEP`,
`SOUND` and `PLAY` on the speaker pin; `PIN`, `PWM`, `SERVO`, `ADC` and
`TEMP()`; `I2C` and `SPI`; COM1; and `TIMER`, `DATE$` and `TIME$`. Line
editing has history, and `EDIT n` puts a line on the input line.

## What doesn't (Feature not available)

- `SCREEN` 1 and up, and every graphics statement.
- `SPRITE`, `FONT`, `GPRINT`, `IMAGE` and `SAVEIMAGE`.
- `MUSIC`, `MOUSE`, the full-screen `EDIT` and `SHELL`.

`MOUSEX`, `MOUSEY`, `MOUSEB`, `KEYDOWN()` and `MUSIC()` give 0. `SCREEN(r, c)`
and `PEEK` of the text screen give spaces, because a terminal can't be read
back.

## The flash disk

On the first start-up the disk is formatted as FAT (labelled PICOBASIC).
Writes go through a 4 KB cache, which is written to the flash when a file is
closed. So `CLOSE` your files before you switch off.

Getting files on and off:

- **Programs:** paste the listing into the terminal at the `Ok` prompt (the
  numbered lines go into the program), then `SAVE "NAME"`. To get one back,
  `LOAD` it, `LIST` it and copy the text from the terminal.
- **Anything else** (planned): XMODEM send and receive commands.
