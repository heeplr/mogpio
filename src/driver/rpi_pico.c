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
 * Backend for the Pico's built-in GPIO pins.
 *
 * This driver is deliberately straightforward: each HAL pin maps directly to
 * one physical Pico GPIO. The driver tracks the configured function for each
 * pin so it can reject invalid operations such as writing to an input.
 */

#include <string.h>

#include "logger.h"
#include "hal_gpio.h"
#include "hal_gpio_layout.h"
#include "rpi_pico.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"



static inline size_t gpio_number(const hal_gpio_pico_ctx_t *ctx, size_t pin)
{
    return (size_t)(ctx->first_gpio + pin);
}

static int pico_validate_pin(const hal_gpio_pico_ctx_t *ctx, size_t pin)
{
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= ctx->pin_count || pin >= HAL_PICO_PINS) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    return HAL_GPIO_OK;
}

static int pico_init(void *vctx)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    /*
     * On init we mark every HAL pin as deconfigured. The application must call
     * hal_gpio_pin_config() explicitly for each pin it wants to use.
     */
    memset(ctx->function, 0, sizeof(ctx->function));
    memset(ctx->mode, 0, sizeof(ctx->mode));
    memset(ctx->configured, 0, sizeof(ctx->configured));

    return HAL_GPIO_OK;
}

static int pico_deinit(void *vctx)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    /*
     * Deinitializing a Pico GPIO returns it to a neutral state. Doing this for
     * every exposed pin makes suspend/powerdown behavior predictable.
     */
    for (size_t pin = 0; pin < ctx->pin_count && pin < HAL_PICO_PINS; ++pin) {
        gpio_deinit(gpio_number(ctx, pin));
        ctx->configured[pin] = false;
        ctx->function[pin] = HAL_GPIO_FN_NONE;
    }

    return HAL_GPIO_OK;
}

static int pico_set_function(void *vctx, size_t pin, hal_gpio_function_t function)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    int rc = pico_validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    const size_t gpio = gpio_number(ctx, pin);

    INFO("pin %d, func: %d", pin, function);

    if (function == HAL_GPIO_FN_NONE) {
        /* NONE means “release the pin”. */
        gpio_deinit(gpio);
        return HAL_GPIO_OK;
    }

    if (function != HAL_GPIO_FN_INPUT && function != HAL_GPIO_FN_OUTPUT) {
        return HAL_GPIO_ERR_INVAL;
    }

    if(function != HAL_GPIO_FN_NOCHANGE)
        gpio_set_dir(gpio, function == HAL_GPIO_FN_OUTPUT);

    return HAL_GPIO_OK;
}

static int pico_set_mode(void *vctx, size_t pin, hal_gpio_mode_t mode)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    int rc = pico_validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    const size_t gpio = gpio_number(ctx, pin);

    INFO("pin %d, mode: %d", pin, mode);

    switch (mode) {
        case HAL_GPIO_MODE_PULL_UP:
            gpio_set_pulls(gpio, true, false);
            break;

        case HAL_GPIO_MODE_PULL_DOWN:
            gpio_set_pulls(gpio, false, true);
            break;

        case HAL_GPIO_MODE_PUSHPULL:
            gpio_set_pulls(gpio, false, false);
            break;

        case HAL_GPIO_MODE_NOCHANGE:
            break;

        default:
            return HAL_GPIO_ERR_INVAL;
    }

    return HAL_GPIO_OK;
}

static int pico_get_function(void *vctx, size_t pin, hal_gpio_function_t *function)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    int rc = pico_validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* get pico gpio pin */
    const size_t gpio = gpio_number(ctx, pin);

    switch(gpio_get_dir(gpio)) {

        case GPIO_OUT:
            *function = HAL_GPIO_FN_OUTPUT;
            break;

        case GPIO_IN:
            *function = HAL_GPIO_FN_INPUT;
            break;
    }

    return HAL_GPIO_OK;
}

static int pico_get_mode(void *vctx, size_t pin, hal_gpio_mode_t *mode)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    int rc = pico_validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    const size_t gpio = gpio_number(ctx, pin);

    bool pulled_up = gpio_is_pulled_up(gpio);
    bool pulled_down = gpio_is_pulled_down(gpio);

    if(pulled_up && !pulled_down) {
        *mode = HAL_GPIO_MODE_PULL_UP;
    }
    else if(!pulled_up && pulled_down) {
        *mode = HAL_GPIO_MODE_PULL_DOWN;
    }
    else if(!pulled_up && !pulled_down) {
        *mode = HAL_GPIO_MODE_PUSHPULL;
    }

    return HAL_GPIO_OK;
}

static int pico_read(void *vctx, size_t pin, bool *value)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    int rc = pico_validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    INFO("-> pin: %d configured: %d", pin, ctx->configured[pin]);
    if (!ctx->configured[pin] || ctx->function[pin] == HAL_GPIO_FN_NONE) {
        return HAL_GPIO_ERR_STATE;
    }

    /*
     * Reading an output is allowed here because it simply reports the pin's
     * actual logic level, which can be useful for debug.
     */
    *value = gpio_get(gpio_number(ctx, pin));
    return HAL_GPIO_OK;
}

static int pico_write(void *vctx, size_t pin, bool value)
{
    hal_gpio_pico_ctx_t *ctx = (hal_gpio_pico_ctx_t *)vctx;
    int rc = pico_validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    if (!ctx->configured[pin] || ctx->function[pin] != HAL_GPIO_FN_OUTPUT) {
        return HAL_GPIO_ERR_UNSUPPORTED;
    }

    gpio_put(gpio_number(ctx, pin), value);
    return HAL_GPIO_OK;
}

const hal_gpio_driver_ops_t hal_gpio_pico_ops = {
    .init = pico_init,
    .deinit = pico_deinit,
    .read = pico_read,
    .write = pico_write,
    .set_function = pico_set_function,
    .set_mode = pico_set_mode,
    .get_function = pico_get_function,
    .get_mode = pico_get_mode
};

