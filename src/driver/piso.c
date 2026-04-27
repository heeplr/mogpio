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
 * Driver for PISO (Parallel-In, Serial-Out) shift-register chains.
 *
 * This driver is input-only by design. It rejects attempts to configure any
 * pin as an output.
 */

#include <stddef.h>
#include <string.h>

#include "hal_gpio.h"
#include "hal_gpio_layout.h"
#include "piso.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"



static int _validate_pin(const hal_gpio_piso_ctx_t *ctx, uint8_t pin)
{
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= ctx->pin_count || pin >= HAL_PISO_MAX_PINS) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    return HAL_GPIO_OK;
}

static inline uint8_t _mapped_pin(const hal_gpio_piso_ctx_t *ctx, uint8_t pin)
{
    /*
     * The reversal flag is useful when the bit order of the chain is opposite
     * to the order the application wants to see.
     */
    return ctx->reverse_order ? (uint8_t)(ctx->pin_count - 1u - pin) : pin;
}

static void piso_refresh_cache(hal_gpio_piso_ctx_t *ctx)
{
    /*
     * 74HC165 sequence:
     *   1) Drive /PL (load pin) low then high to capture parallel inputs.
     *   2) Clock the chain and sample the serial output bit once per pin.
     *
     */
    gpio_put(ctx->load_pin, true);
    for (uint8_t i = 0; i < ctx->pin_count; ++i) {
        gpio_put(ctx->clock_pin, false);
        const uint8_t dst = _mapped_pin(ctx, i);
        ctx->cached_bits[dst] = gpio_get(ctx->data_pin);
        gpio_put(ctx->clock_pin, true);
    }
    gpio_put(ctx->load_pin, false);
}

static int piso_init(void *vctx)
{
    hal_gpio_piso_ctx_t *ctx = (hal_gpio_piso_ctx_t *)vctx;
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    gpio_init(ctx->data_pin);
    gpio_init(ctx->clock_pin);
    gpio_init(ctx->load_pin);

    /* read the chain on the data pin, while the other two pins are outputs. */
    gpio_set_dir(ctx->data_pin, false);
    gpio_set_dir(ctx->clock_pin, true);
    gpio_set_dir(ctx->load_pin, true);

    gpio_set_pulls(ctx->data_pin, false, false);
    gpio_put(ctx->clock_pin, false);
    gpio_put(ctx->load_pin, true);

    memset(ctx->cached_bits, 0, sizeof(ctx->cached_bits));

    return HAL_GPIO_OK;
}

static int piso_deinit(void *vctx)
{
    hal_gpio_piso_ctx_t *ctx = (hal_gpio_piso_ctx_t *)vctx;
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    gpio_deinit(ctx->data_pin);
    gpio_deinit(ctx->clock_pin);
    gpio_deinit(ctx->load_pin);

    return HAL_GPIO_OK;
}

static int piso_pin_config(void *vctx, size_t pin,
                           hal_gpio_function_t function,
                           hal_gpio_mode_t mode)
{
    hal_gpio_piso_ctx_t *ctx = (hal_gpio_piso_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    switch (function) {
        case HAL_GPIO_FN_OUTPUT:
            return HAL_GPIO_ERR_UNSUPPORTED;

        case HAL_GPIO_FN_NONE:
        case HAL_GPIO_FN_NOCHANGE:
            break;

        default:
            return HAL_GPIO_ERR_INVAL;
    }

    /*
     * A PISO bank cannot generate a pull-up/pull-down on the external inputs.
     * The mode is therefore only accepted as PUSHPULL to keep the API honest.
     */
    switch (mode) {
        case HAL_GPIO_MODE_PUSHPULL:
        case HAL_GPIO_MODE_NOCHANGE:
            return HAL_GPIO_OK;

        default:
            break;
    }

    return HAL_GPIO_ERR_UNSUPPORTED;
}

static int piso_get_function(void *vctx, size_t pin, hal_gpio_function_t *function)
{
    hal_gpio_piso_ctx_t *ctx = (hal_gpio_piso_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }
    /* a PISO will always be an INPUT pin */
    *function = HAL_GPIO_FN_INPUT;
    return HAL_GPIO_OK;
}

static int piso_get_mode(void *vctx, size_t pin, hal_gpio_mode_t *mode)
{
    hal_gpio_piso_ctx_t *ctx = (hal_gpio_piso_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }
    /* a PISO will always be in PUSHPULL mode */
    *mode = HAL_GPIO_MODE_PUSHPULL;
    return HAL_GPIO_OK;
}

static int piso_read(void *vctx, size_t pin, bool *value)
{
    hal_gpio_piso_ctx_t *ctx = (hal_gpio_piso_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* Always refresh before a read so the caller gets the current chain state. */
    piso_refresh_cache(ctx);
    *value = ctx->cached_bits[pin] ? 1 : 0;
    return HAL_GPIO_OK;
}

static int piso_write(void *vctx, size_t pin, bool value)
{
    (void)vctx;
    (void)pin;
    (void)value;
    return HAL_GPIO_ERR_UNSUPPORTED;
}

const hal_gpio_driver_ops_t hal_gpio_piso_ops = {
    .init = piso_init,
    .deinit = piso_deinit,
    .pin_config = piso_pin_config,
    .read = piso_read,
    .write = piso_write,
    .get_function = piso_get_function,
    .get_mode = piso_get_mode
};
