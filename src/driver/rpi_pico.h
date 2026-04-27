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

#ifndef _RPI_PICO
#define _RPI_PICO

#ifdef DEBUG_OUT
#define HAL_PICO_PINS 28u
#else
#define HAL_PICO_PINS 30u
#endif


/* driver instance configuration & runtime data */
typedef struct {
    size_t first_gpio;   /* First GPIO used by this bank. */
    size_t pin_count;    /* Number of pins exposed */

    /*
     * Bookkeeping lets the driver reject illegal operations cleanly.
     * The HAL core does not interpret these arrays; they are purely driver
     * implementation details.
     */
    hal_gpio_function_t function[HAL_PICO_PINS];
    hal_gpio_mode_t mode[HAL_PICO_PINS];
    bool configured[HAL_PICO_PINS];
} hal_gpio_pico_ctx_t;


extern const hal_gpio_driver_ops_t hal_gpio_pico_ops;

#endif /* _RPI_PICO */
