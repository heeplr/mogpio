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
 * hal_gpio.h
 *
 * Public GPIO HAL API.
 *
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* absolute maxima */
#define HAL_GPIO_BANKS_MAX  32
#define HAL_GPIO_PINS_MAX   32

/*
 * A GPIO can be left unassigned (NONE), configured as input or configured as
 * output.
 */
typedef enum {
    HAL_GPIO_FN_MIN = -1,
    HAL_GPIO_FN_NOCHANGE = 0,
    HAL_GPIO_FN_NONE,
    HAL_GPIO_FN_INPUT,
    HAL_GPIO_FN_OUTPUT,
    HAL_GPIO_FN_MAX
} hal_gpio_function_t;

/* The mode describes how the pin is biased or driven. */
typedef enum {
    HAL_GPIO_MODE_MIN = -1,
    HAL_GPIO_MODE_NOCHANGE = 0,
    HAL_GPIO_MODE_PULL_UP,
    HAL_GPIO_MODE_PULL_DOWN,
    HAL_GPIO_MODE_PUSHPULL,
    HAL_GPIO_MODE_MAX
} hal_gpio_mode_t;

/* codes for error handling */
enum {
    HAL_GPIO_OK              = 0,
    HAL_GPIO_ERR_INVAL       = -1,  /* invalid argument */
    HAL_GPIO_ERR_NOT_INIT    = -2,  /* hal_gpio_init() has not completed */
    HAL_GPIO_ERR_UNSUPPORTED = -3,  /* requested operation does not fit bank */
    HAL_GPIO_ERR_BOUNDS      = -4,  /* bank or pin is out of range */
    HAL_GPIO_ERR_STATE       = -5,  /* bank/pin is in the wrong state */
};

/*
 * Callbacks implemented by a driver backend.
 */
typedef struct hal_gpio_driver_ops {
    int (*init)(void *ctx);
    int (*deinit)(void *ctx);
    int (*pin_config)(void *ctx, size_t pin,
                      hal_gpio_function_t function,
                      hal_gpio_mode_t mode);
    int (*read)(void *ctx, size_t pin, bool *value);
    int (*write)(void *ctx, size_t pin, bool value);
    int (*get_function)(void *ctx, size_t pin, hal_gpio_function_t *function);
    int (*get_mode)(void *ctx, size_t pin, hal_gpio_mode_t *mode);
} hal_gpio_driver_ops_t;

/*
 * One driver instance.
 *
 * The HAL keeps the driver callbacks and the runtime context together.
 */
typedef struct hal_gpio_driver {
    const hal_gpio_driver_ops_t *ops;
    void *ctx;
    size_t pin_count;
} hal_gpio_driver_t;

/*
 * A bank is a logical block of GPIOs.
 *
 * Bank descriptors no longer carry driver state.
 */
typedef struct {
    size_t bank_id;
    const char *name;
    size_t pin_count;
} hal_gpio_bank_t;



int hal_gpio_init(void);
int hal_gpio_deinit(void);
int hal_gpio_pin_config(size_t bankid, size_t pin,
                        hal_gpio_function_t function,
                        hal_gpio_mode_t mode);

int hal_gpio_read(size_t bankid, size_t pin, bool *value);
int hal_gpio_write(size_t bankid, size_t pin, bool value);
int hal_gpio_get_function(size_t bankid, size_t pin, hal_gpio_function_t *function);
int hal_gpio_get_mode(size_t bankid, size_t pin, hal_gpio_mode_t *mode);

const char *hal_gpio_function_name(hal_gpio_function_t fn);
const char *hal_gpio_mode_name(hal_gpio_mode_t mode);
const char *hal_gpio_bank_name(size_t bankid);

size_t hal_gpio_bankcount(void);
size_t hal_gpio_bank_pincount(size_t bankid);
size_t hal_gpio_pincount(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */
