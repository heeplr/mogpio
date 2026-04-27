
# TinyUSB
set(FAMILY espressif)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
set(TOP ${PICO_TINYUSB_PATH})


idf_component_register(
    SRCS "src/mogpio.c"
         "src/usb_descriptors.c"
         "src/usbio.c"
         "src/msc_fs.c"
         "src/terminal.c"
         "src/hal_gpio.c"
         "src/util.c"
         "microrl-remaster/src/microrl/microrl.c"
    INCLUDE_DIRS "."
)

if(LAYOUT STREQUAL "intern")
    idf_component_register(
        SRCS "src/driver/esp32.c"
             "src/boards/intern.c"
    )
    set(LAYOUT_DEFINE LAYOUT_INTERN)

elseif(LAYOUT STREQUAL "intern-sipo-piso")
    idf_component_register(
        SRCS "src/driver/esp32.c"
             "src/driver/sipo.c"
             "src/driver/piso.c"
             "src/boards/intern-sipo-piso.c"
    )
    set(LAYOUT_DEFINE LAYOUT_INTERN_SIPO_PISO)

elseif(LAYOUT STREQUAL "sipo-piso")
    idf_component_register(
        SRCS "src/driver/sipo.c"
             "src/driver/piso.c"
             "src/boards/sipo-piso.c"
    )
    set(LAYOUT_DEFINE LAYOUT_SIPO_PISO)

else()
    message(FATAL_ERROR "platform ${PLATFORM} doesn't support layout ${LAYOUT}")

endif()

# provide BOA
target_compile_definitions(mogpio PRIVATE ${LAYOUT_DEFINE})
# provide PLATFORM_ESP32 macro
target_compile_definitions(${COMPONENT_LIB} PRIVATE PLATFORM_ESP32)


