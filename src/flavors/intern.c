/*
 * plain.c
 *
 * Board description for the GPIO HAL to use the platforms' internal GPIO
 *
 */

#include <stddef.h>
#include <stdint.h>

#include "hal_gpio.h"
#include "hal_gpio_flavor.h"

#ifdef PLATFORM_PICO
#include "gpio/rpi_pico.h"
#elif PLATFORM_ESP32
#include "gpio/esp32.h"
#endif


enum {
    HAL_BANK_INTERN   = 0,
    HAL_BANK_COUNT    = 1,
};


/*
 * The driver contexts are mutable because the drivers store runtime state such
 * as configuration flags and cached/shadowed pin values.
 */
static hal_gpio_pico_ctx_t s_pico_ctx = {
#ifdef DEBUG_OUT
    .first_gpio = 2,    // GPIO0 and GPIO1 used for UART0
#else
    .first_gpio = 0,    // use all pins
#endif
    .pin_count = HAL_PICO_MAX_PINS,
};


static const hal_gpio_bank_t s_banks[] = {
    {
        .bank_id = HAL_BANK_INTERN,
        .name = "pico-gpio",
        .pin_count = HAL_PICO_MAX_PINS,
        .ops = &hal_gpio_pico_ops,
        .ctx = &s_pico_ctx,
    }
};

const hal_gpio_flavor_t g_hal_gpio_flavor = {
    .banks = s_banks,
    .bank_count = sizeof(s_banks) / sizeof(s_banks[0]),
};
