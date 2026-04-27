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

/* hal_gpio.c - Generic GPIO HAL dispatcher. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hal_gpio.h"
#include "hal_gpio_layout.h"

/* initialization state */
static bool s_hal_initialized = false;


/* amount of registered driver instances */
static size_t hal_gpio_drivercount(void)
{
    return g_hal_gpio_layout.driver_count;
}

/* amount of registered banks */
size_t hal_gpio_bankcount(void)
{
    return g_hal_gpio_layout.bank_count;
}

/* amount of registered pins on bank */
size_t hal_gpio_bank_pincount(size_t bankid)
{
    if (bankid >= hal_gpio_bankcount()) {
        return 0;
    }

    const hal_gpio_bank_t *bank = &g_hal_gpio_layout.banks[bankid];
    if (bank == NULL) {
        return 0;
    }
    return bank->pin_count;
}

/* amount of total registered gpio pins */
size_t hal_gpio_pincount(void)
{
    size_t count = 0;

    for (size_t d = 0; d < hal_gpio_drivercount(); ++d) {
        const hal_gpio_driver_t *driver = &g_hal_gpio_layout.drivers[d];
        if (driver == NULL) {
            continue;
        }
        count += driver->pin_count;
    }

    return count;
}

/* map bank:pin to hardware gpio */
static int hal_gpio_from_bank_pin(size_t bankid, size_t pin, size_t *gpio)
{
    if (gpio == NULL || g_hal_gpio_layout.banks == NULL || bankid >= hal_gpio_bankcount()) {
        return HAL_GPIO_ERR_INVAL;
    }

    size_t gpio_count = 0;
    for (size_t b = 0; b < bankid; ++b) {
        gpio_count += hal_gpio_bank_pincount(b);
    }

    *gpio = gpio_count + pin;
    return HAL_GPIO_OK;
}

/* get driver + pin offset for hardware gpio */
static int hal_gpio_driver_for_pin(size_t gpio, hal_gpio_driver_t **driver, size_t *pin) {

    size_t count = 0;
    for (size_t d=0; d < hal_gpio_drivercount(); d++) {
        const hal_gpio_driver_t *drv = &g_hal_gpio_layout.drivers[d];
        if (gpio < count + drv->pin_count) {
            *driver = (hal_gpio_driver_t *) drv;
            *pin = gpio - count;
            return HAL_GPIO_OK;
        }
        count += drv->pin_count;
    }

    return HAL_GPIO_ERR_BOUNDS;
}

/* initialize GPIO hardware */
int hal_gpio_init(void)
{
    if (s_hal_initialized) {
        return HAL_GPIO_OK;
    }

    for (size_t d = 0; d < hal_gpio_drivercount(); ++d) {
        const hal_gpio_driver_t *driver = &g_hal_gpio_layout.drivers[d];
        if (driver == NULL) {
            return HAL_GPIO_ERR_INVAL;
        }

        if (driver->ops->init != NULL) {
            int rc = driver->ops->init(driver->ctx);
            if (rc != HAL_GPIO_OK) {
                return rc;
            }
        }
    }

    s_hal_initialized = true;
    return HAL_GPIO_OK;
}

/* deinitialize GPIO hardware */
int hal_gpio_deinit(void)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_OK;
    }

        if (g_hal_gpio_layout.drivers == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    for (size_t d = hal_gpio_drivercount(); d > 0; --d) {
        const hal_gpio_driver_t *driver = &g_hal_gpio_layout.drivers[d - 1];
        if (driver->ops->deinit != NULL) {
            driver->ops->deinit(driver->ctx);
        }
    }

    s_hal_initialized = false;
    return HAL_GPIO_OK;
}

/* set config for GPIO pin */
int hal_gpio_pin_config(size_t bankid, size_t pin,
                        hal_gpio_function_t function,
                        hal_gpio_mode_t mode)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    if (bankid >= hal_gpio_bankcount()) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    if (pin >= hal_gpio_bank_pincount(bankid)) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    /* get hardware offset of this bank/pin */
    size_t gpio;
    int rc = hal_gpio_from_bank_pin(bankid, pin, &gpio);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* get driver + pin for hardware offset */
    hal_gpio_driver_t *driver;
    size_t hw_pin;
    rc = hal_gpio_driver_for_pin(gpio, &driver, &hw_pin);
    if(rc != HAL_GPIO_OK) {
        return rc;
    }

    if (driver == NULL || driver->ops->pin_config == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    return driver->ops->pin_config(driver->ctx, hw_pin, function, mode);
}

/* read input value from gpIo */
int hal_gpio_read(size_t bankid, size_t pin, bool *value)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    if (bankid >= hal_gpio_bankcount()) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    if (pin >= hal_gpio_bank_pincount(bankid)) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    /* get hardware offset of this bank/pin */
    size_t gpio;
    int rc = hal_gpio_from_bank_pin(bankid, pin, &gpio);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* get driver + pin for hardware offset */
    hal_gpio_driver_t *driver;
    size_t hw_pin;
    rc = hal_gpio_driver_for_pin(gpio, &driver, &hw_pin);
    if(rc != HAL_GPIO_OK) {
        return rc;
    }

    if (driver == NULL || driver->ops->read == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    return driver->ops->read(driver->ctx, hw_pin, value);
}

/* set value of gpiO */
int hal_gpio_write(size_t bankid, size_t pin, bool value)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    if (bankid >= hal_gpio_bankcount()) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    if (pin >= hal_gpio_bank_pincount(bankid)) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    /* get hardware offset of this bank/pin */
    size_t gpio;
    int rc = hal_gpio_from_bank_pin(bankid, pin, &gpio);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* get driver + pin for hardware offset */
    hal_gpio_driver_t *driver;
    size_t hw_pin;
    rc = hal_gpio_driver_for_pin(gpio, &driver, &hw_pin);
    if(rc != HAL_GPIO_OK) {
        return rc;
    }

    if (driver == NULL || driver->ops->write == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    return driver->ops->write(driver->ctx, hw_pin, value);
}

/* get currently configured function of GPIO */
int hal_gpio_get_function(size_t bankid, size_t pin, hal_gpio_function_t *function)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    if (bankid >= hal_gpio_bankcount()) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    if (pin >= hal_gpio_bank_pincount(bankid)) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    /* get hardware offset of this bank/pin */
    size_t gpio;
    int rc = hal_gpio_from_bank_pin(bankid, pin, &gpio);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* get driver + pin for hardware offset */
    hal_gpio_driver_t *driver;
    size_t hw_pin;
    rc = hal_gpio_driver_for_pin(gpio, &driver, &hw_pin);
    if(rc != HAL_GPIO_OK) {
        return rc;
    }

    if (driver == NULL || driver->ops->get_function == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    return driver->ops->get_function(driver->ctx, hw_pin, function);
}

/* get currently configured mode of GPIO */
int hal_gpio_get_mode(size_t bankid, size_t pin, hal_gpio_mode_t *mode)
{
    if (!s_hal_initialized) {
        return HAL_GPIO_ERR_NOT_INIT;
    }

    if (bankid >= hal_gpio_bankcount()) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    if (pin >= hal_gpio_bank_pincount(bankid)) {
        return HAL_GPIO_ERR_BOUNDS;
    }

    /* get hardware offset of this bank/pin */
    size_t gpio;
    int rc = hal_gpio_from_bank_pin(bankid, pin, &gpio);
    if (rc != HAL_GPIO_OK) {
        return rc;
    }

    /* get driver + pin for hardware offset */
    hal_gpio_driver_t *driver;
    size_t hw_pin;
    rc = hal_gpio_driver_for_pin(gpio, &driver, &hw_pin);
    if(rc != HAL_GPIO_OK) {
        return rc;
    }

    if (driver == NULL || driver->ops->get_mode == NULL) {
        return HAL_GPIO_ERR_INVAL;
    }

    return driver->ops->get_mode(driver->ctx, hw_pin, mode);
}

/* get printable name of bank */
const char *hal_gpio_bank_name(size_t bankid)
{
    if (bankid >= hal_gpio_bankcount()) {
        return NULL;
    }

    const hal_gpio_bank_t *bank = &g_hal_gpio_layout.banks[bankid];
    if (bank == NULL) {
        return NULL;
    }

    return bank->name;
}

/* get printable name of GPIO function */
const char *hal_gpio_function_name(hal_gpio_function_t fn)
{
    switch (fn) {
    case HAL_GPIO_FN_NONE: return "NONE";
    case HAL_GPIO_FN_INPUT: return "INPUT";
    case HAL_GPIO_FN_OUTPUT: return "OUTPUT";
    case HAL_GPIO_FN_NOCHANGE: return "NOCHANGE";
    default: return "UNKNOWN";
    }
}

/* get printable name of GPIO mode */
const char *hal_gpio_mode_name(hal_gpio_mode_t mode)
{
    switch (mode) {
    case HAL_GPIO_MODE_PULL_UP: return "PULLUP";
    case HAL_GPIO_MODE_PULL_DOWN: return "PULLDOWN";
    case HAL_GPIO_MODE_PUSHPULL: return "PUSHPULL";
    case HAL_GPIO_MODE_NOCHANGE: return "NOCHANGE";
    default: return "UNKNOWN";
    }
}
