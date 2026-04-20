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

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hal_gpio.h"


/* return pointer to string starting with non whitespace character */
const char *skip_ws(const char *s, const char *end) {
    while (s < end && isspace((unsigned char)*s)) {
        ++s;
    }
    return s;
}

/* terminate string with trailing whitespace trimmed */
void rtrim_ws(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[--n] = '\0';
    }
}

/* parse integer */
bool parse_u32(const char *s, uint32_t *out) {
    if(!s) {
        return false;
    }

    size_t len = strlen(s);
    if (len == 0) {
        return false;
    }

    uint32_t value = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c < '0' || c > '9') {
            return false;
        }
        uint32_t digit = (uint32_t)(c - '0');
        if (value > (UINT32_MAX - digit) / 10u) {
            return false;
        }
        value = value * 10u + digit;
    }

    *out = value;
    return true;
}

bool parse_u8(const char *s, uint8_t *out)
{
    if(!s) {
        return false;
    }

    size_t len = strlen(s);
    if (len == 0) {
        return false;
    }

    uint8_t value = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c < '0' || c > '9') {
            return false;
        }
        uint32_t digit = (uint32_t)(c - '0');
        if (value > (UINT8_MAX - digit) / 10u) {
            return false;
        }
        value = value * 10u + digit;
    }

    *out = value;
    return true;
}

/* return true if tokens are equal, false if not */
bool token_eq(const char *s, const char *tok) {
    return strcasecmp(s, tok) == 0;
}

/* append string to buffer */
size_t append_fmt(char *dst, size_t cap, size_t used, const char *fmt, ...) {
    if (!fmt || used >= cap) {
        return SIZE_MAX;
    }

    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(dst + used, cap - used, fmt, ap);
    va_end(ap);

    if (n < 0 || (size_t)n >= cap - used) {
        return SIZE_MAX;
    }

    return used + (size_t)n;
}


/******************************************************************************/

/* parse pin function string */
bool parse_function(const char *s, hal_gpio_function_t *fn)
{
    if (!s || !fn) {
        return false;
    }

    /* try all function names */
    for(hal_gpio_function_t f = HAL_GPIO_FN_MIN; f < HAL_GPIO_FN_MAX; f++) {
        if(token_eq(s, hal_gpio_function_name(f))) {
            *fn = f;
            return true;
        }
    }

    uint8_t v;
    bool res = parse_u8(s, &v);
    if (res && v < HAL_GPIO_FN_MAX) {
        *fn = (hal_gpio_function_t) v;
        return true;
    }

    return false;
}

/* parse pin mode string */
bool parse_mode(const char *s, hal_gpio_mode_t *mode)
{
    if (!s || !mode ) {
        return false;
    }

    /* try all mode names */
    for(hal_gpio_mode_t m = HAL_GPIO_MODE_MIN; m < HAL_GPIO_MODE_MAX; m++) {
        if(token_eq(s, hal_gpio_mode_name(m))) {
            *mode = m;
            return true;
        }
    }

    /* try numeric mode id */
    uint8_t v;
    bool res = parse_u8(s, &v);
    if (res && v < HAL_GPIO_MODE_MAX) {
        *mode = (hal_gpio_mode_t) v;
        return true;
    }

    return false;
}

/* parse bank:pin tuple */
bool parse_bank_pin(const char *s, uint8_t *bank, uint8_t *pin)
{
    const char *colon;
    char bank_buf[8];
    char pin_buf[8];

    if (s == NULL || bank == NULL || pin == NULL) {
        return false;
    }

    colon = strchr(s, ':');
    if (colon == NULL) {
        return false;
    }

    if ((size_t)(colon - s) >= sizeof(bank_buf)) {
        return false;
    }

    if (strlen(colon + 1) >= sizeof(pin_buf)) {
        return false;
    }

    memcpy(bank_buf, s, (size_t)(colon - s));
    bank_buf[colon - s] = '\0';
    strcpy(pin_buf, colon + 1);

    return parse_u8(bank_buf, bank) && parse_u8(pin_buf, pin);
}

/* parse binary value */
bool parse_value01(const char *s, bool *value)
{
    if (!s || !value) {
        return false;
    }

    if (token_eq(s, "0") || token_eq(s, "low") || token_eq(s, "off")) {
        *value = false;
        return true;
    }

    if (token_eq(s, "1") || token_eq(s, "high") || token_eq(s, "on")) {
        *value = true;
        return true;
    }

    return false;
}
