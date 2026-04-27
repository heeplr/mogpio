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
 * hal_gpio_layout.h
 *
 * Compile-time board layout.
 */
#ifndef HAL_GPIO_LAYOUT_H
#define HAL_GPIO_LAYOUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * The complete board/layout description.
 *
 * Drivers and banks are defined in matching order. The n-th driver serves the
 * n-th bank, and GPIO pin numbering is derived sequentially from that order.
 */
typedef struct {
    const hal_gpio_driver_t *drivers;
    const hal_gpio_bank_t *banks;
    size_t driver_count;
    size_t bank_count;
} hal_gpio_layout_t;

/*
 * The active board description.
 */
extern const hal_gpio_layout_t g_hal_gpio_layout;

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_LAYOUT_H */
