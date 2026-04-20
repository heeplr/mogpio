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

#include "logger.h"
#include "hal_gpio.h"
#include "msc_fs.h"
#include "terminal.h"

void tud_mount_cb(void) {}

void tud_umount_cb(void) {}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    INFO("suspending... remote wakeup: %d", remote_wakeup_en);
}

void tud_resume_cb(void) {
    INFO("resumed...");
}

int main(void) {
    board_init();

#ifdef HAVE_LOGGING
    stdio_init_all();
#endif

    /* initialize GPIO HAL */
    hal_gpio_init();
    /* initialize mass storage interface */
    msc_fs_init();
    /* initialize serial terminal interface */
    terminal_init();
    /* TinyUSB init */
    tusb_init();

    INFO("moGPIO initialized");

    while (1) {
        tud_task();
        terminal_task();
    }
}
