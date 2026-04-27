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

#ifndef _UTIL_H
#define _UTIL_H


const char *skip_ws(const char *s, const char *end);
void rtrim_ws(char *s);
bool token_eq(const char *s, const char *tok);
size_t append_fmt(char *dst, size_t cap, size_t used, const char *fmt, ...);

bool parse_u32(const char *s, uint32_t *out);
bool parse_u16(const char *s, uint16_t *out);
bool parse_u8(const char *s, uint8_t *out);
bool parse_function(const char *s, hal_gpio_function_t *fn);
bool parse_mode(const char *s, hal_gpio_mode_t *mode);
bool parse_bank_pin(const char *s, size_t *bank, size_t *pin);
bool parse_value01(const char *s, bool *value);


#endif /* _UTIL_H */
