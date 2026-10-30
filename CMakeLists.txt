cmake_minimum_required(VERSION 3.13)

# A plain Pico 2 (RP2350A, 4 MB flash) by default; for the Pimoroni Pico Plus 2
# (RP2350B, 16 MB flash, 8 MB PSRAM) configure with
#   -DPICO_BOARD=pimoroni_pico_plus2_rp2350
# and for the original Pico (RP2040, 2 MB flash) with -DPICO_BOARD=pico.
set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(pico_basic C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(pico_basic
    src/main.c
    src/basic.c
    src/basic_port_serial.c
    src/flash_disk.c
    src/diskio.c
    src/sd_card.c
    src/stack_switch.c
    src/xfer.c
    src/fatfs/ff.c
    src/fatfs/ffunicode.c
)

target_include_directories(pico_basic PRIVATE src src/fatfs)

target_link_libraries(pico_basic
    pico_stdlib
    hardware_adc
    hardware_flash
    pico_flash
    hardware_i2c
    hardware_irq
    hardware_pwm
    hardware_spi
)

# The RP2350 boards can have PSRAM; the RP2040 has none (psram_none.c says so).
if(PICO_PLATFORM STREQUAL "rp2040")
    target_sources(pico_basic PRIVATE src/psram_none.c)
else()
    target_sources(pico_basic PRIVATE src/psram_heap.c)
    target_link_libraries(pico_basic hardware_psram hardware_xip_cache)
endif()

# Where the flash disk starts: the program below it, the disk to the end of
# the flash (3 MB on a Pico 2, 15 MB on a Pico Plus 2).
set(FLASH_DISK_OFFSET 0x100000 CACHE STRING "Flash disk start, bytes from the start of the flash (a multiple of 4096)")
# BEEP, SOUND and PLAY: a piezo or small speaker (through a resistor) on this pin.
set(SOUND_PIN 15 CACHE STRING "GPIO for the speaker")

target_compile_definitions(pico_basic PRIVATE
    BASIC_NO_SCREEN=1
    FLASH_DISK_OFFSET=${FLASH_DISK_OFFSET}
    SOUND_PIN=${SOUND_PIN}
    PICO_BASIC_BOARD="${PICO_BOARD}"
    # Core 0's boot stack: the whole 4 KB SCRATCH_Y bank (the SDK default is
    # 2 KB). BASIC then runs on a 32 KB stack of its own (main.c).
    PICO_STACK_SIZE=0x1000
    # malloc returns NULL when the heap is full (BASIC says "Out of memory")
    # instead of stopping the board with a panic.
    PICO_MALLOC_PANIC=0
)

# PSRAM: boards that have a chip select for it check the chip's ID at boot
# instead of assuming it is fitted; others have none.
if(PICO_BOARD STREQUAL "pimoroni_pico_plus2_rp2350")
    target_compile_definitions(pico_basic PRIVATE PICO_AUTO_DETECT_PSRAM_SIZE=1)
endif()

# A microSD card over SPI as a second drive ("1:" or "SD:"). Default pins:
# SPI0 with MISO GP16, CS GP17, SCK GP18, MOSI GP19 (the usual Pico SD
# wiring; SPI1, BASIC's own SPI OPEN default, stays free).
option(PICO_BASIC_SD "An SD card over SPI as drive 1: (SD:)" ON)
set(SD_SPI 0 CACHE STRING "SD card: SPI controller (0 or 1)")
set(SD_MISO 16 CACHE STRING "SD card: MISO pin (card DO)")
set(SD_CS 17 CACHE STRING "SD card: CS pin")
set(SD_SCK 18 CACHE STRING "SD card: SCK pin (card CLK)")
set(SD_MOSI 19 CACHE STRING "SD card: MOSI pin (card DI)")
if(PICO_BASIC_SD)
    target_compile_definitions(pico_basic PRIVATE PICO_BASIC_SD=1 SD_SPI=${SD_SPI} SD_MISO=${SD_MISO}
        SD_CS=${SD_CS} SD_SCK=${SD_SCK} SD_MOSI=${SD_MOSI})
else()
    target_compile_definitions(pico_basic PRIVATE PICO_BASIC_SD=0)
endif()

# -DDISK_TRACE=ON: a build that starts USB first, waits for a terminal, and
# prints each step of mounting (and formatting) the flash disk.
option(DISK_TRACE "Trace the flash disk's mount and format on the console" OFF)
if(DISK_TRACE)
    target_compile_definitions(pico_basic PRIVATE DISK_TRACE=1)
endif()

# The console: the board's own USB port, as a USB serial (CDC) device
# (/dev/ttyACM* on Linux, a COM port on Windows). The UART is left free.
# picotool can still reboot it into BOOTSEL through the USB reset interface.
pico_enable_stdio_uart(pico_basic 0)
pico_enable_stdio_usb(pico_basic 1)

pico_add_extra_outputs(pico_basic)
