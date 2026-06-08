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

#ifdef PLATFORM_PICO
#include "pico/stdlib.h"
#include "bsp/board.h"
#endif


#include "hal_gpio.h"
#include "msc_fs.h"
#include "console.h"
#include "terminal.h"
#include "ulog.h"


void tud_mount_cb(void) {}

void tud_umount_cb(void) {}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    ulog_info("suspending... remote wakeup: %d", remote_wakeup_en);
}

void tud_resume_cb(void) {
    ulog_info("resumed...");
}

/* logging output handler that will output to the terminal */
static void ulog_output_handler(ulog_event *ev, void *arg) {
    (void) arg;

    static char buffer[256];
    int result = ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    if (result == 0) {
        console_write(buffer, sizeof(buffer));
        console_write("\r\n", 2);
    }
}


int main(void) {

#ifdef PLATFORM_PICO
    board_init();
#ifdef UART_TERMINAL
    stdio_init_all();
#endif /* UART_TERMINAL */
#endif /* PLATFORM_PICO */

    /* register terminal logging output handler */
    ulog_output_add(ulog_output_handler, NULL, ULOG_LEVEL_TRACE);
    ulog_output_level_set_all(ULOG_LEVEL_TRACE);
    /* add logging topics */
    ulog_topic_add("usbio", ULOG_OUTPUT_ALL, ULOG_LEVEL_WARN);
    ulog_topic_add("msc", ULOG_OUTPUT_ALL, ULOG_LEVEL_WARN);
    ulog_topic_add("terminal", ULOG_OUTPUT_ALL, ULOG_LEVEL_WARN);
    ulog_topic_add("driver", ULOG_OUTPUT_ALL, ULOG_LEVEL_WARN);
    ulog_topic_add("hal", ULOG_OUTPUT_ALL, ULOG_LEVEL_WARN);
    /* initialize GPIO HAL */
    hal_gpio_init();
    /* initialize mass storage interface */
    msc_fs_init();
    /* initialize serial terminal interface */
    terminal_init();
    /* TinyUSB init */
    tusb_init();

    ulog_info("moGPIO initialized");

    while (1) {
        tud_task();
        terminal_task();
    }
}
