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
 * Driver for SIPO (Serial-In, Parallel-Out) shift-register chains.
 *
 * This driver is output-only by design. It rejects attempts to configure any
 * pin as an input.
 */

#include <string.h>
#include <stddef.h>

#include "hal_gpio.h"
#include "hal_gpio_flavor.h"
#include "sipo.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"




static int _validate_pin(const hal_gpio_sipo_ctx_t *ctx, uint8_t pin)
{
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= ctx->pin_count || pin >= HAL_SIPO_MAX_PINS) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    return HAL_GPIO_OK;
}

static inline uint8_t _mapped_pin(const hal_gpio_sipo_ctx_t *ctx, uint8_t pin)
{
    return ctx->reverse_order ? (uint8_t)(ctx->pin_count - 1u - pin) : pin;
}

static void sipo_flush_shadow(hal_gpio_sipo_ctx_t *ctx)
{
    /*
     * 74HC595 sequence:
     *   1) Put the next bit on DATA.
     *   2) Pulse CLOCK so it shifts into the chain.
     *   3) Pulse LATCH once all bits are shifted so outputs update together.
     *
     * This shadow-buffer approach is convenient because the application writes
     * a single logical pin at a time, while the driver still updates the whole
     * chain in one clean transfer.
     */
    gpio_put(ctx->latch_pin, false);
    for (uint8_t i = 0; i < ctx->pin_count; ++i) {
        gpio_put(ctx->clock_pin, false);
        const uint8_t src = _mapped_pin(ctx, i);
        gpio_put(ctx->data_pin, ctx->shadow_bits[src]);
        gpio_put(ctx->clock_pin, true);
    }
    gpio_put(ctx->latch_pin, true);

}


static int sipo_init(void *vctx)
{
    hal_gpio_sipo_ctx_t *ctx = (hal_gpio_sipo_ctx_t *)vctx;
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    gpio_init(ctx->data_pin);
    gpio_init(ctx->clock_pin);
    gpio_init(ctx->latch_pin);

    gpio_set_dir(ctx->data_pin, true);
    gpio_set_dir(ctx->clock_pin, true);
    gpio_set_dir(ctx->latch_pin, true);

    gpio_put(ctx->data_pin, false);
    gpio_put(ctx->clock_pin, false);
    gpio_put(ctx->latch_pin, false);

    memset(ctx->shadow_bits, 0, sizeof(ctx->shadow_bits));
    memset(ctx->function, 0, sizeof(ctx->function));
    memset(ctx->configured, 0, sizeof(ctx->configured));

    /* Drive the physical outputs to a known low state immediately. */
    sipo_flush_shadow(ctx);
    return HAL_GPIO_OK;
}

static int sipo_deinit(void *vctx)
{
    hal_gpio_sipo_ctx_t *ctx = (hal_gpio_sipo_ctx_t *)vctx;
    if (ctx == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    gpio_deinit(ctx->data_pin);
    gpio_deinit(ctx->clock_pin);
    gpio_deinit(ctx->latch_pin);

    return HAL_GPIO_OK;
}

static int sipo_pin_config(void *vctx, uint8_t pin,
                           hal_gpio_function_t function,
                           hal_gpio_mode_t mode)
{
    hal_gpio_sipo_ctx_t *ctx = (hal_gpio_sipo_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    switch (function) {

        case HAL_GPIO_FN_INPUT:
            return HAL_GPIO_ERR_UNSUPPORTED;

        case HAL_GPIO_FN_NONE:
            /* Releasing the pin also clears its output level in the shadow buffer. */
            ctx->function[pin] = HAL_GPIO_FN_NONE;
            ctx->configured[pin] = false;
            ctx->shadow_bits[pin] = false;
            sipo_flush_shadow(ctx);
            break;

        case HAL_GPIO_FN_NOCHANGE:
            break;

        default:
            return HAL_GPIO_ERR_INVAL;
    }


    if (mode != HAL_GPIO_MODE_PUSHPULL) {
        return HAL_GPIO_ERR_UNSUPPORTED;
    }

    ctx->function[pin] = HAL_GPIO_FN_OUTPUT;
    ctx->configured[pin] = true;
    return HAL_GPIO_OK;
}

static int sipo_get_function(void *vctx, uint8_t pin, hal_gpio_function_t *function)
{
    hal_gpio_sipo_ctx_t *ctx = (hal_gpio_sipo_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }
    /* a SIPO will always be an OUTPUT pin */
    *function = HAL_GPIO_FN_OUTPUT;
    return HAL_GPIO_OK;
}

static int sipo_get_mode(void *vctx, uint8_t pin, hal_gpio_mode_t *mode)
{
    hal_gpio_sipo_ctx_t *ctx = (hal_gpio_sipo_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* a SIPO will always be in PUSHPULL mode */
    *mode = HAL_GPIO_MODE_PUSHPULL;
    return HAL_GPIO_OK;
}

static int sipo_read(void *vctx, uint8_t pin, bool *value)
{
    hal_gpio_sipo_ctx_t *ctx = (hal_gpio_sipo_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    if (!ctx->configured[pin] || ctx->function[pin] != HAL_GPIO_FN_OUTPUT) {
        return HAL_GPIO_ERR_STATE;
    }

    /*
     * Reading a SIPO pin returns the shadow value that was last written.
     * That is usually the most useful behavior for output-only hardware.
     */
    *value = ctx->shadow_bits[pin];

    return HAL_GPIO_OK;
}

static int sipo_write(void *vctx, uint8_t pin, bool value)
{
    hal_gpio_sipo_ctx_t *ctx = (hal_gpio_sipo_ctx_t *)vctx;
    int rc = _validate_pin(ctx, pin);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    if (!ctx->configured[pin] || ctx->function[pin] != HAL_GPIO_FN_OUTPUT) {
        return HAL_GPIO_ERR_STATE;
    }

    ctx->shadow_bits[pin] = value;
    sipo_flush_shadow(ctx);
    return HAL_GPIO_OK;
}

const hal_gpio_driver_ops_t hal_gpio_sipo_ops = {
    .init = sipo_init,
    .deinit = sipo_deinit,
    .pin_config = sipo_pin_config,
    .read = sipo_read,
    .write = sipo_write,
    .get_function = sipo_get_function,
    .get_mode = sipo_get_mode
};
