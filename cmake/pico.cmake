
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# TinyUSB
set(PICO_TINYUSB_PATH "${CMAKE_SOURCE_DIR}/tinyusb")
set(FAMILY rp2040)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
set(TOP ${PICO_TINYUSB_PATH})

# Initialize Pico SDK
pico_sdk_init()

# Add the firmware executable
add_executable(mogpio)

target_sources(mogpio PUBLIC
    src/mogpio.c
    src/usb_descriptors.c
    src/usbio.c
    src/msc_fs.c
    src/terminal.c
    src/hal_gpio.c
    src/util.c
    microrl-remaster/src/microrl/microrl.c
)

if(LAYOUT STREQUAL "intern")
    set(LAYOUT_DEFINE LAYOUT_INTERN)
    target_sources(mogpio PUBLIC
        src/driver/rpi_pico.c
        src/layouts/intern.c
    )

elseif(LAYOUT STREQUAL "intern-sipo-piso")
    set(LAYOUT_DEFINE LAYOUT_INTERN_SIPO_PISO)
    target_sources(mogpio PUBLIC
        src/driver/rpi_pico.c
        src/driver/sipo.c
        src/driver/piso.c
        src/layouts/intern-sipo-piso.c
    )

elseif(LAYOUT STREQUAL "sipo-piso")
    set(LAYOUT_DEFINE LAYOUT_SIPO_PISO)
    target_sources(mogpio PUBLIC
        src/driver/sipo.c
        src/driver/piso.c
        src/layouts/sipo-piso.c
    )

else()
    message(FATAL_ERROR "platform ${PLATFORM} doesn't support layout ${LAYOUT}")

endif()

# provide LAYOUT_* macro
target_compile_definitions(mogpio PRIVATE ${LAYOUT_DEFINE})


# provide PLATFORM_PICO macro
target_compile_definitions(mogpio PRIVATE PLATFORM_PICO)

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
    # enable stdout via UART
    pico_enable_stdio_uart(mogpio 1)
else()
    # disable stdio via UART
    pico_enable_stdio_uart(mogpio 0)
endif()

# no USB CDC for stdout
pico_enable_stdio_usb(mogpio 0)
