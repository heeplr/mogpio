/*
 * SPDX-License-Identifier: MIT License
 *
 * Copyright (c) 2026 Daniel Hiepler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of the moGPIO firmware.
 */

/*
 * Layout for the GPIO HAL to use the platforms internal GPIO.
 *
 * only one single layout can be compiled
 */

#include <stddef.h>

#include "hal_gpio.h"
#include "hal_gpio_layout.h"

#ifdef PLATFORM_PICO
#include "driver/rpi_pico.h"
#elif PLATFORM_ESP32
#include "driver/esp32.h"
#endif


/* raspberry pico internal GPIOs  */
static hal_gpio_pico_ctx_t s_pico_ctx = {
#ifdef DEBUG_OUT
    .first_gpio = 2,    // GPIO0 and GPIO1 used for UART0
#else
    .first_gpio = 0,    // use all pins
#endif
    .pin_count = HAL_PICO_PINS,
};

/* GPIO hardware driver layout */
static const hal_gpio_driver_t s_drivers[] = {
    {
        .ops = &hal_gpio_pico_ops,
        .ctx = &s_pico_ctx,
        .pin_count = HAL_PICO_PINS,
    },
};

/*
 * bank/pin layout - this can be set arbitrary. Pins are mapped to driver pins
 * in sequential order
 */
static const hal_gpio_bank_t s_banks[] = {
    {
        .bank_id = 0,
        .name = "pico-gpio",
        .pin_count = HAL_PICO_PINS,
    },
};

/* this layout */
const hal_gpio_layout_t g_hal_gpio_layout = {
    .drivers = s_drivers,
    .banks = s_banks,
    .driver_count = sizeof(s_drivers) / sizeof(s_drivers[0]),
    .bank_count = sizeof(s_banks) / sizeof(s_banks[0]),
};
