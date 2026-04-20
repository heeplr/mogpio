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
 * Board description for the GPIO HAL to use
 *
 * - PISO shift registers (Bank 0)
 * - SIPO shift registers (Bank 1)
 */

#include <stddef.h>
#include <stdint.h>

#include "hal_gpio.h"
#include "hal_gpio_flavor.h"

#include "gpio/piso.h"
#include "gpio/sipo.h"


enum {
    HAL_BANK_PISO     = 0,
    HAL_BANK_SIPO     = 1,
    HAL_BANK_COUNT    = 2,
};


static hal_gpio_piso_ctx_t s_piso_ctx = {
    .data_pin = 2,
    .clock_pin = 3,
    .load_pin = 4,
    .pin_count = HAL_PISO_MAX_PINS,
    .reverse_order = false,
};

static hal_gpio_sipo_ctx_t s_sipo_ctx = {
    .data_pin = 6,
    .clock_pin = 7,
    .latch_pin = 8,
    .pin_count = HAL_SIPO_MAX_PINS,
    .reverse_order = false,
};

static const hal_gpio_bank_t s_banks[] = {
    {
        .bank_id = HAL_BANK_PISO,
        .name = "piso-input-chain",
        .pin_count = HAL_PISO_MAX_PINS,
        .ops = &hal_gpio_piso_ops,
        .ctx = &s_piso_ctx,
    },
    {
        .bank_id = HAL_BANK_SIPO,
        .name = "sipo-output-chain",
        .pin_count = HAL_SIPO_MAX_PINS,
        .ops = &hal_gpio_sipo_ops,
        .ctx = &s_sipo_ctx,
    },
};

const hal_gpio_flavor_t g_hal_gpio_flavor = {
    .banks = s_banks,
    .bank_count = sizeof(s_banks) / sizeof(s_banks[0]),
};
