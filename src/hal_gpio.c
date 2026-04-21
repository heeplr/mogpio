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
 * hal_gpio.c
 *
 * Generic GPIO HAL dispatcher.
 *
 * This file knows nothing about the hardware details of any bank. It only
 * knows how to:
 *   - look up a bank in the hal_gpio_flavor description,
 *   - call the bank's driver through function pointers,
 *   - and enforce a minimal initialized/not-initialized state for the system.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hal_gpio.h"
#include "hal_gpio_flavor.h"


/* initialization state */
static bool s_hal_initialized = false;

/* lookup bank by id */
static const hal_gpio_bank_t *hal_find_bank(uint8_t bankid)
{
    for (size_t i = 0; i < g_hal_gpio_flavor.bank_count; ++i) {
        const hal_gpio_bank_t *bank = &g_hal_gpio_flavor.banks[i];
        if (bank->bank_id == bankid) {
            return bank;
        }
    }
    return NULL;
}

/* return total amount of banks accessible to HAL */
unsigned int hal_gpio_bankcount(void)
{
    return g_hal_gpio_flavor.bank_count;
}

/* return amount of GPIOs provided by one bank */
unsigned int hal_gpio_bank_pincount(uint8_t bankid)
{
    if(bankid > g_hal_gpio_flavor.bank_count)
        return 0;

    size_t bank = (size_t) bankid;
    return g_hal_gpio_flavor.banks[bank].pin_count;
}

/* return total GPIO pincount accessible to HAL */
unsigned int hal_gpio_pincount(void)
{
    unsigned int count = 0;
    for (size_t i = 0; i < g_hal_gpio_flavor.bank_count; ++i) {
        count += g_hal_gpio_flavor.banks[i].pin_count;
    }
    return count;
}


/* initialize HAL (powerup/resume) */
int hal_gpio_init(void)
{
    if (s_hal_initialized) {
        return HAL_GPIO_OK;
    }

    for (size_t i = 0; i < g_hal_gpio_flavor.bank_count; ++i) {
        const hal_gpio_bank_t *bank = &g_hal_gpio_flavor.banks[i];
        if (bank->ops == NULL || bank->ops->init == NULL) {
            return HAL_GPIO_ERR_INVAL;
        }

        int rc = bank->ops->init(bank->ctx);
        if(rc != HAL_GPIO_OK) {
            return rc;
        }
    }

    s_hal_initialized = true;
    return HAL_GPIO_OK;
}

/* deinitialize (shutdown/suspend) */
int hal_gpio_deinit(void)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_OK;
    }

    /* Deinitialize in reverse order */
    for (size_t i = g_hal_gpio_flavor.bank_count; i > 0; --i) {
        const hal_gpio_bank_t *bank = &g_hal_gpio_flavor.banks[i - 1u];
        if (bank->ops != NULL && bank->ops->deinit != NULL) {
            (void)bank->ops->deinit(bank->ctx);
        }
    }

    s_hal_initialized = false;
    return HAL_GPIO_OK;
}

/* configure pin */
int hal_gpio_pin_config(uint8_t bankid, uint8_t pin,
                        hal_gpio_function_t function,
                        hal_gpio_mode_t mode)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    const hal_gpio_bank_t *bank = hal_find_bank(bankid);
    if (bank == NULL) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    if (bank->ops == NULL || bank->ops->pin_config == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= bank->pin_count) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    return bank->ops->pin_config(bank->ctx, pin, function, mode);
}

/* read pin value */
int hal_gpio_read(uint8_t bankid, uint8_t pin, bool *value)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    const hal_gpio_bank_t *bank = hal_find_bank(bankid);
    if (bank == NULL) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    if (bank->ops == NULL || bank->ops->read == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= bank->pin_count) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    return bank->ops->read(bank->ctx, pin, value);
}

/* write pin value */
int hal_gpio_write(uint8_t bankid, uint8_t pin, bool value)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    const hal_gpio_bank_t *bank = hal_find_bank(bankid);
    if (bank == NULL) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    if (bank->ops == NULL || bank->ops->write == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= bank->pin_count) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    return bank->ops->write(bank->ctx, pin, value);
}

/* get current function of a pin */
int hal_gpio_get_function(uint8_t bankid, uint8_t pin, hal_gpio_function_t *function)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    const hal_gpio_bank_t *bank = hal_find_bank(bankid);
    if (bank == NULL) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    if (bank->ops == NULL || bank->ops->write == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= bank->pin_count) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    return bank->ops->get_function(bank->ctx, pin, function);
}

/* get current mode of a pin */
int hal_gpio_get_mode(uint8_t bankid, uint8_t pin, hal_gpio_mode_t *mode) {
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    const hal_gpio_bank_t *bank = hal_find_bank(bankid);
    if (bank == NULL) {
        return HAL_GPIO_ERR_BOUNDS;
    }
    if (bank->ops == NULL || bank->ops->write == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }
    if (pin >= bank->pin_count) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    return bank->ops->get_mode(bank->ctx, pin, mode);
}

/* return printable name of bank */
const char *hal_gpio_bank_name(uint8_t bankid) {
    const hal_gpio_bank_t *bank = hal_find_bank(bankid);
    if(!bank)
        return NULL;

    return bank->name;
}

/* return printable name of HAL_GPIO_FN_* */
const char *hal_gpio_function_name(hal_gpio_function_t fn)
{
    switch (fn) {
    case HAL_GPIO_FN_NONE:    return "NONE";
    case HAL_GPIO_FN_INPUT:   return "INPUT";
    case HAL_GPIO_FN_OUTPUT:  return "OUTPUT";
    case HAL_GPIO_FN_NOCHANGE:return "NOCHANGE";
    default:                  return "UNKNOWN";
    }
}

/* return printable name of HAL_GPIO_MODE_* */
const char *hal_gpio_mode_name(hal_gpio_mode_t mode)
{
    switch (mode) {
    case HAL_GPIO_MODE_PULL_UP:    return "PULLUP";
    case HAL_GPIO_MODE_PULL_DOWN:  return "PULLDOWN";
    case HAL_GPIO_MODE_PUSHPULL:   return "PUSHPULL";
    case HAL_GPIO_MODE_NOCHANGE:   return "NOCHANGE";
    default:                       return "UNKNOWN";
    }
}
