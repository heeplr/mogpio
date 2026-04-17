
# TinyUSB
set(FAMILY espressif)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
set(TOP ${PICO_TINYUSB_PATH})


idf_component_register(
    SRCS "src/mogpio.c"
         "src/usb_descriptors.c"
         "src/msc_fs.c"
         "src/usbio.c"
         "src/hal_gpio.c"
    INCLUDE_DIRS "."
)

if(FLAVOR STREQUAL "intern")
    idf_component_register(
        SRCS "src/gpio/esp32.c"
             "src/boards/intern.c"
    )
    set(FLAVOR_DEFINE FLAVOR_INTERN)

elseif(FLAVOR STREQUAL "intern-sipo-piso")
    idf_component_register(
        SRCS "src/gpio/esp32.c"
             "src/gpio/sipo.c"
             "src/gpio/piso.c"
             "src/boards/intern-sipo-piso.c"
    )
    set(FLAVOR_DEFINE FLAVOR_INTERN_SIPO_PISO)

elseif(FLAVOR STREQUAL "sipo-piso")
    idf_component_register(
        SRCS "src/gpio/sipo.c"
             "src/gpio/piso.c"
             "src/boards/sipo-piso.c"
    )
    set(FLAVOR_DEFINE FLAVOR_SIPO_PISO)

else()
    message(FATAL_ERROR "platform ${PLATFORM} doesn't support flavor ${FLAVOR}")

endif()

# provide BOA
target_compile_definitions(mogpio PRIVATE ${FLAVOR_DEFINE})
# provide PLATFORM_ESP32 macro
target_compile_definitions(${COMPONENT_LIB} PRIVATE PLATFORM_ESP32)


