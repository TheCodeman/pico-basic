cmake_minimum_required(VERSION 3.13)

# A plain Pico 2 (RP2350A, 4 MB flash) by default; for the Pimoroni Pico Plus 2
# (RP2350B, 16 MB flash, 8 MB PSRAM) configure with
#   -DPICO_BOARD=pimoroni_pico_plus2_rp2350
# and for the original Pico (RP2040, 2 MB flash) with -DPICO_BOARD=pico.
# Wi-Fi (and the clock from NTP): -DPICO_BOARD=pico2_w or pico_w, or a Pico
# Plus 2 with Pimoroni's RM2 breakout with -DPICO_BASIC_WIFI=ON.
set(PICO_BOARD pico2 CACHE STRING "Board type")
if(PICO_BOARD MATCHES "_w$|_w_")
    set(WIFI_DEFAULT ON)
else()
    set(WIFI_DEFAULT OFF)
endif()
option(PICO_BASIC_WIFI "Wi-Fi and NTP (a W board, or a Pico Plus 2 with an RM2 breakout)" ${WIFI_DEFAULT})
if(PICO_BASIC_WIFI AND PICO_BOARD STREQUAL "pimoroni_pico_plus2_rp2350")
    set(PICO_CYW43_SUPPORTED 1)  # the RM2 breakout: pins set below
endif()

include(pico_sdk_import.cmake)

project(pico_basic C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(pico_basic
    src/main.c
    src/basic.c
    src/basic_port_serial.c
    src/config.c
    src/flash_disk.c
    src/diskio.c
    src/sd_card.c
    src/stack_switch.c
    src/term_editor.c
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
    hardware_pio
    hardware_pwm
    hardware_spi
)

# NEOPIXEL's WS2812B driver runs on a PIO state machine.
pico_generate_pio_header(pico_basic ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

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
set(SOUND_PIN 18 CACHE STRING "GPIO for the speaker")

target_compile_definitions(pico_basic PRIVATE
    BASIC_NO_SCREEN=1
    # EDIT: the full-screen editor on the terminal (term_editor.c).
    PICO_BASIC_EDITOR=1
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
# the Cytron Maker Pi Pico's slot, SPI1 with SCK GP10, MOSI GP11 (card CMD /
# SDI), MISO GP12 (card DAT0 / SDO), CS GP15, and DAT1 GP13, DAT2 GP14 held
# up (unused in SPI mode). BASIC's own SPI OPEN then defaults to SPI0 on
# GP4-7 (MISO GP4, CS GP5, SCK GP6, MOSI GP7), and COM1 to TX GP8, RX GP9,
# clear of the card and the board's buzzer (GP18), buttons (GP20-22) and
# RGB LED (GP28).
option(PICO_BASIC_SD "An SD card over SPI as drive 1: (SD:)" ON)
set(SD_SPI 1 CACHE STRING "SD card: SPI controller (0 or 1)")
set(SD_MISO 12 CACHE STRING "SD card: MISO pin (card SDO / DO)")
set(SD_CS 15 CACHE STRING "SD card: CS pin")
set(SD_SCK 10 CACHE STRING "SD card: SCK pin (card CLK)")
set(SD_MOSI 11 CACHE STRING "SD card: MOSI pin (card SDI / DI)")
set(SD_DAT1 13 CACHE STRING "SD card: DAT1 pin, held up (-1: not wired)")
set(SD_DAT2 14 CACHE STRING "SD card: DAT2 pin, held up (-1: not wired)")
if(PICO_BASIC_SD)
    if(SD_SPI EQUAL 1)
        set(SPI_SCK_DEFAULT 6)  # SPI OPEN on SPI0, GP4-7
        set(COM1_TX_DEFAULT 8)  # COM1 off GP4-5
    else()
        set(SPI_SCK_DEFAULT 10)  # SPI OPEN on SPI1
        set(COM1_TX_DEFAULT 4)
    endif()
    target_compile_definitions(pico_basic PRIVATE PICO_BASIC_SD=1 SD_SPI=${SD_SPI} SD_MISO=${SD_MISO}
        SD_CS=${SD_CS} SD_SCK=${SD_SCK} SD_MOSI=${SD_MOSI} SD_DAT1=${SD_DAT1} SD_DAT2=${SD_DAT2}
        SPI_SCK_DEFAULT=${SPI_SCK_DEFAULT} COM1_TX_DEFAULT=${COM1_TX_DEFAULT})
else()
    target_compile_definitions(pico_basic PRIVATE PICO_BASIC_SD=0)
endif()

# AUTORUN.BAS runs at power-up; held low then, this pin skips it (the Maker
# Pi Pico's GP20 button; it gets a pull-up, so nothing connected never skips).
set(AUTORUN_SKIP_PIN 20 CACHE STRING "Pin held low at power-up to skip AUTORUN.BAS (-1: none)")
target_compile_definitions(pico_basic PRIVATE AUTORUN_SKIP_PIN=${AUTORUN_SKIP_PIN})

# -DDISK_TRACE=ON: a build that starts USB first, waits for a terminal, and
# prints each step of mounting (and formatting) the flash disk.
option(DISK_TRACE "Trace the flash disk's mount and format on the console" OFF)
if(DISK_TRACE)
    target_compile_definitions(pico_basic PRIVATE DISK_TRACE=1)
endif()

if(PICO_BASIC_WIFI)
    # The CYW43 driver and lwIP run in a low-priority interrupt; lwIP's SNTP
    # client sets the clock (src/wifi.c, src/lwipopts.h).
    target_sources(pico_basic PRIVATE src/wifi.c)
    target_link_libraries(pico_basic pico_cyw43_arch_lwip_threadsafe_background pico_lwip_sntp)
    target_compile_definitions(pico_basic PRIVATE PICO_BASIC_WIFI=1 CYW43_HOST_NAME="pico-basic")
    if(PICO_BOARD STREQUAL "pimoroni_pico_plus2_rp2350")
        # As pico-os: the RM2 breakout on the SP/CE connector, power GP32,
        # CS GP33, clock GP34, data GP35; the bus clock retried slower if need be.
        target_compile_definitions(pico_basic PRIVATE CYW43_PIO_CLOCK_DIV_DYNAMIC=1
            CYW43_DEFAULT_PIN_WL_REG_ON=32 CYW43_DEFAULT_PIN_WL_CS=33 CYW43_DEFAULT_PIN_WL_CLOCK=34
            CYW43_DEFAULT_PIN_WL_DATA_OUT=35 CYW43_DEFAULT_PIN_WL_DATA_IN=35
            CYW43_DEFAULT_PIN_WL_HOST_WAKE=35 CYW43_WL_GPIO_COUNT=3 CYW43_WL_GPIO_LED_PIN=0)
    endif()
else()
    target_compile_definitions(pico_basic PRIVATE PICO_BASIC_WIFI=0)
endif()

# The console: the board's own USB port, as a USB serial (CDC) device
# (/dev/ttyACM* on Linux, a COM port on Windows). The UART is left free.
# picotool can still reboot it into BOOTSEL through the USB reset interface.
pico_enable_stdio_uart(pico_basic 0)
pico_enable_stdio_usb(pico_basic 1)

pico_add_extra_outputs(pico_basic)
