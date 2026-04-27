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
 * Layout for the GPIO HAL to use
 *
 * - the platform's internal GPIOs (Bank 0)
 * - PISO shift registers (Bank 1)
 * - SIPO shift registers (Bank 2)
 */

#include <stddef.h>

#include "hal_gpio.h"
#include "hal_gpio_layout.h"

#ifdef PLATFORM_PICO
#include "driver/rpi_pico.h"
#elif PLATFORM_ESP32
#include "driver/esp32.h"
#endif

#include "driver/piso.h"
#include "driver/sipo.h"


/* raspberry pico internal GPIOs  */
static hal_gpio_pico_ctx_t s_pico_ctx = {
#ifdef DEBUG_OUT
    .first_gpio = 2, /* GPIO0 and GPIO1 used for UART0 */
#else
    .first_gpio = 0,
#endif
    .pin_count = HAL_PICO_PINS,
};

/* PISO chain with 2x8 outputs on GPIO6/7/8 */
static hal_gpio_piso_ctx_t s_piso_ctx = {
    .data_pin = 2,
    .clock_pin = 3,
    .load_pin = 4,
    .pin_count = HAL_PISO_MAX_PINS,
    .reverse_order = false,
};

/* SIPO chain 2x8 inputs on GPIO3/4/5 */
static hal_gpio_sipo_ctx_t s_sipo_ctx = {
    .data_pin = 6,
    .clock_pin = 7,
    .latch_pin = 8,
    .pin_count = HAL_SIPO_MAX_PINS,
    .reverse_order = false,
};

/* GPIO hardware driver layout */
static const hal_gpio_driver_t s_drivers[] = {
    {
        .ops = &hal_gpio_pico_ops,
        .ctx = &s_pico_ctx,
        .pin_count = HAL_PICO_PINS,
    },
    {
        .ops = &hal_gpio_piso_ops,
        .ctx = &s_piso_ctx,
        .pin_count = 16,
    },
    {
        .ops = &hal_gpio_sipo_ops,
        .ctx = &s_sipo_ctx,
        .pin_count = 16,
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
    {
        .bank_id = 1,
        .name = "piso-input-chain",
        .pin_count = 16,
    },
    {
        .bank_id = 2,
        .name = "sipo-output-chain",
        .pin_count = 16,
    },
};

/* this layout */
const hal_gpio_layout_t g_hal_gpio_layout = {
    .drivers = s_drivers,
    .banks = s_banks,
    .driver_count = sizeof(s_drivers) / sizeof(s_drivers[0]),
    .bank_count = sizeof(s_banks) / sizeof(s_banks[0]),
};
