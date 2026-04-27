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

#ifndef _PISO_H
#define _PISO_H

#define HAL_PISO_MAX_PINS     32u


typedef struct {
    size_t data_pin;     // Serial data from the PISO chain into the Pico.
    size_t clock_pin;    // Shift clock.
    size_t load_pin;     // Parallel load / latch control.
    size_t pin_count;    // Total number of input bits exposed.
    bool reverse_order;   // Flip bit order if the chain is wired backwards.

    bool cached_bits[HAL_PISO_MAX_PINS];
    hal_gpio_function_t function[HAL_PISO_MAX_PINS];
    bool configured[HAL_PISO_MAX_PINS];
} hal_gpio_piso_ctx_t;


extern const hal_gpio_driver_ops_t hal_gpio_piso_ops;

#endif /* _PISO_H */
