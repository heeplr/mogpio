cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Project with C, CXX, and ASM support
project(mogpio C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# tiny usb settings
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
set(TOP ${PICO_TINYUSB_PATH})

# Initialize Pico SDK
pico_sdk_init()

# Add the firmware executable
add_executable(mogpio)

target_sources(mogpio PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/mogpio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/src/msc_fs.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usbio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/hal_gpio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/hal_gpio_board.c
    ${CMAKE_CURRENT_LIST_DIR}/src/gpio/rpi_pico.c
    ${CMAKE_CURRENT_LIST_DIR}/src/gpio/sipo.c
    ${CMAKE_CURRENT_LIST_DIR}/src/gpio/piso.c
)

# Include directories for tinyUSB config
target_include_directories(mogpio PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# Link libraries
target_link_libraries(mogpio
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

# Create UF2 output
pico_add_extra_outputs(mogpio)

# debug compile ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_OUT)
    # enable stdout via UART
    pico_enable_stdio_uart(mogpio 1)
else()
    # disable stdio via UART
    pico_enable_stdio_uart(mogpio 0)
endif()

# no USB CDC for stdout
pico_enable_stdio_usb(mogpio 0)

# Additional compile options
target_compile_options(mogpio PRIVATE -Wall -Wextra)

