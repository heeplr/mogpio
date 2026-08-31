cmake_minimum_required(VERSION 3.13)

# extract version and commit hash from git
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0 --always
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

# set platform
# (build plain RP2040 by default)
set(PLATFORM "pico" CACHE STRING "platform to compile on (pico, esp32)")
# set board layout (plain by default)
set(LAYOUT "intern" CACHE STRING "gpio hardware layout")
# turn on uart log output
set(UART_TERMINAL off CACHE BOOL "use first uart as serial terminal")


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
    ${CMAKE_CURRENT_LIST_DIR}/microlog/include
)

# pass version info to compile
if(DEFINED GIT_VERSION)
    target_compile_definitions(mogpio PRIVATE GIT_VERSION="${GIT_VERSION}")
    message(STATUS "Found version from tag: ${GIT_VERSION}")
else()
    message(WARN "No version string found!")
endif()
if(DEFINED GIT_COMMIT)
    target_compile_definitions(mogpio PRIVATE GIT_COMMIT="${GIT_COMMIT}")
    message(STATUS "Found git commit: ${GIT_COMMIT}")
endif()

# config header for microlog
target_compile_definitions(mogpio PRIVATE ULOG_BUILD_CONFIG_HEADER_ENABLED=1)

# log to uart aswell?
if(UART_TERMINAL)
    target_compile_definitions(mogpio PRIVATE UART_TERMINAL)
endif()

# Additional compile options
target_compile_options(mogpio PRIVATE -Wall -Wextra)

message(STATUS "=============================================================================")
message(STATUS "building for platform: ${PLATFORM} layout: ${LAYOUT} uart terminal: ${UART_TERMINAL}")
