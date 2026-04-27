cmake_minimum_required(VERSION 3.13)


# set platform
# (build plain RP2040 by default)
set(PLATFORM "pico" CACHE STRING "platform to compile on (pico, esp32)")
# set board layout (plain by default)
set(LAYOUT "intern" CACHE STRING "gpio hardware layout")

# esp idf
if(PLATFORM STREQUAL "esp32")
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  project(mogpio)
  include(cmake/esp32.cmake)
# pico-sdk
elseif(PLATFORM STREQUAL "pico" OR PLATFORM STREQUAL "pico2")
  include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
  project(mogpio C CXX ASM)
  include(cmake/pico.cmake)
# unknown
else()
  message(FATAL_ERROR "unknown platform: ${PLATFORM}")
endif()

# Include directories for
#  * tinyUSB config
#  * microrl-remaster
target_include_directories(mogpio PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/microrl-remaster/src/include/microrl
)

# debug compile ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(mogpio PRIVATE DEBUG_OUT)
endif()


# Additional compile options
target_compile_options(mogpio PRIVATE -Wall -Wextra)

message(STATUS "building for platform: ${PLATFORM} layout: ${LAYOUT}")
