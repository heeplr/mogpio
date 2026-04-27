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
 * - PISO shift registers (Bank 0)
 * - SIPO shift registers (Bank 1)
 *
 * only one single layout can be compiled
 */

#include <stddef.h>

#include "hal_gpio.h"
#include "hal_gpio_layout.h"
#include "driver/piso.h"
#include "driver/sipo.h"


/* PISO chain with 2x8 outputs on GPIO6/7/8 */
static hal_gpio_piso_ctx_t s_piso_ctx = {
    .data_pin = 6,
    .clock_pin = 7,
    .load_pin = 8,
    .pin_count = 16,
    .reverse_order = false,
    .mode = {
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
        HAL_GPIO_MODE_PULL_DOWN,
    }
};

/* SIPO chain 2x8 inputs on GPIO3/4/5 */
static hal_gpio_sipo_ctx_t s_sipo_ctx = {
    .data_pin = 3,
    .clock_pin = 4,
    .latch_pin = 5,
    .pin_count = 16,
    .reverse_order = false,
};

/* GPIO hardware driver layout */
static const hal_gpio_driver_t s_drivers[] = {
    {
        .ops = &hal_gpio_piso_ops,  // piso.c driver functions
        .ctx = &s_piso_ctx,         // driver context
        .pin_count = 16,            // pin 0-15
    },
    {
        .ops = &hal_gpio_sipo_ops,  // sipo.c driver functions
        .ctx = &s_sipo_ctx,         // driver context
        .pin_count = 16,            // pin 16-31
    },
};

/*
 * bank/pin layout - this can be set arbitrary. Pins are mapped to driver pins
 * in sequential order
 */
static const hal_gpio_bank_t s_banks[] = {
    {
        .bank_id = 0,
        .name = "piso-input-chain",
        .pin_count = 16,
    },
    {
        .bank_id = 1,
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
