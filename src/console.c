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

/* console.c - glue for console stream in/outputs */


#include <stddef.h>
#include <string.h>

#ifdef LOG_UART
#include "pico/stdio.h"
#endif

#include "tusb.h"


#ifdef LOG_UART
static int _uart_write(const char *buf, size_t bufsize)
{
    if (buf == NULL || bufsize == 0) {
        return 0;
    }

    return write(buf, bufsize, stdout);
}

static int _uart_read_available(char *buf, size_t bufsize)
{
    size_t n = 0;
    int ch;

    if (buf == NULL || bufsize == 0) {
        return 0;
    }

    while (n < bufsize) {
        ch = stdio_getchar_timeout_us(0);
        if (ch == PICO_ERROR_TIMEOUT) {
            break;
        }
        buf[n++] = (char)ch;
    }

    return (int)n;
}
#endif


/* write string to CDC */
static int _cdc_out(const char *buf, size_t bufsize) {
    // Send only what fits in the local buffer, but don't silently overrun.
    size_t len = strnlen(buf, bufsize);
    if (!tud_cdc_connected() || len == 0) {
        return 0;
    }

    // TinyUSB CDC write can also be limited by FIFO space.
    size_t off = 0;
    while (off < len) {
        uint32_t space = tud_cdc_write_available();
        if (space == 0) {
            tud_task(); // or just return and retry later if you prefer non-blocking
            continue;
        }

        size_t chunk = len - off;
        if (chunk > space) {
            chunk = space;
        }

        tud_cdc_write(buf + off, (uint32_t)chunk);
        off += chunk;
    }

    tud_cdc_write_flush();
    return (int) len;
}

/* read available input from CDC */
static int _cdc_read_available(char *buf, size_t bufsize) {
    if (tud_cdc_connected()) {
        return tud_cdc_read(buf, bufsize);
    }
    return 0;
}

int console_write(const char *buf, size_t bufsize)
{
    if (buf == NULL || bufsize == 0) {
        return 0;
    }

    int n = _cdc_out(buf, bufsize);
#ifdef LOG_UART
    n = MAX(_uart_write(buf, bufsize), n);
#endif
    return n;
}

/* read available inputs with descending priority */
int console_read_available(char *buf, size_t bufsize) {
    int n;
#ifdef LOG_UART
    n = _uart_read_available(buf, busize);
    if(n > 0) {
        return n;
    }
#endif
    n = _cdc_read_available(buf, bufsize);
    return n;
}
