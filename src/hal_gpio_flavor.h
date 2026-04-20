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
 * hal_gpio_flavor.h
 *
 * Compile-time board layout
 */

#ifndef HAL_GPIO_BOARD_H
#define HAL_GPIO_BOARD_H


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Callbacks to be implemented by drivers
 */
typedef struct hal_gpio_driver_ops {
    int (*init)(void *ctx);
    int (*deinit)(void *ctx);
    int (*pin_config)(void *ctx, uint8_t pin,
                      hal_gpio_function_t function,
                      hal_gpio_mode_t mode);
    int (*read)(void *ctx, uint8_t pin, bool *value);
    int (*write)(void *ctx, uint8_t pin, bool value);
    int (*get_function)(void *ctx, uint8_t pin, hal_gpio_function_t *function);
    int (*get_mode)(void *ctx, uint8_t pin, hal_gpio_mode_t *mode);
} hal_gpio_driver_ops_t;


/* A bank is a single logical block of GPIOs. */
typedef struct {
    uint8_t bank_id;
    const char *name;
    uint8_t pin_count;
    const hal_gpio_driver_ops_t *ops;
    void *ctx;
} hal_gpio_bank_t;

/*
 * The complete board description.
 *
 * This is the single handoff point from board layout into the HAL.
 */
typedef struct {
    const hal_gpio_bank_t *banks;
    size_t bank_count;
} hal_gpio_flavor_t;


/*
 * The active board description.
 *
 * The HAL core reads this symbol and dispatches every API call through it.
 */
extern const hal_gpio_flavor_t g_hal_gpio_flavor;


#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_BOARD_H */
