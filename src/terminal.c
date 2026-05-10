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

#include "terminal.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_gpio.h"
#include "hal_gpio_layout.h"
#include "microrl.h"
#include "ulog.h"
#include "tusb.h"
#include "util.h"


/* microrl context */
static microrl_t s_rl;
/* flag whether CDC is ready */
static bool s_cdc_ready = false;

/* for tab-completion */
#if MICRORL_CFG_USE_COMPLETE

static char s_cmd_list[]   = "list";
static char s_cmd_read[]   = "read";
static char s_cmd_write[]  = "write";
static char s_cmd_config[] = "config";

static char s_empty[] = "";

static const char *const s_command_choices[] = {
    s_cmd_list, s_cmd_read, s_cmd_write, s_cmd_config, "help", "?", NULL
};
static const char *const s_pinval_choices[] = {
    "0", "1", NULL
};
static const char *s_bank_pin_choices[HAL_GPIO_BANKS_MAX * HAL_GPIO_PINS_MAX + 1];
static const char *s_function_choices[HAL_GPIO_FN_MAX + 1];
static const char *s_mode_choices[HAL_GPIO_MODE_MAX + 1];
static char *const s_no_completion[] = {
    s_empty, NULL
};

static const char *s_complete_matches[64];
#endif


/* write string to cdc */
static int _cdc_out(char *buf, size_t bufsize) {
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
    return (int)len;
}

/* printf to terminal */
static int _printf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    int n;

    if (fmt == NULL) {
        return -1;
    }

    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0) {
        return n;
    }

    return _cdc_out(buf, sizeof(buf));
}

/* logging output handler that will output to the terminal */
static void _log_output_handler(ulog_event *ev, void *arg) {
    (void) arg;

    static char buffer[256];
    int result = ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    if (result == 0) {
        _cdc_out(buffer, sizeof(buffer));
        _cdc_out("\r\n", 2);
    }
}

static void cmd_usage(void)
{
    _printf("\r\n"
                   "Commands:\r\n"
                   "  list\r\n"
                   "  read <bank>:<pin>\r\n"
                   "  write <bank>:<pin> <0|1>\r\n"
                   "  config <bank>:<pin> <function> [mode]\r\n"
                   "    function: none|input|output\r\n"
                   "    mode: pull_up|pull_down|pushpull\r\n");
}

static void cmd_list(void)
{
    _printf("\r\nGPIOs:\r\n");

    size_t bankcount = hal_gpio_bankcount();
    for (uint8_t bankid = 0; bankid < bankcount; ++bankid) {

        unsigned int pincount = hal_gpio_bank_pincount(bankid);
        const char *name = hal_gpio_bank_name(bankid);
        _printf("  Bank %u (%s), %u pins\r\n",
                       bankid,
                       name != NULL ? name : "(unnamed)",
                       pincount);

        for (unsigned int pin = 0; pin < pincount; ++pin) {
            hal_gpio_function_t fn = HAL_GPIO_FN_NOCHANGE;
            hal_gpio_mode_t mode = HAL_GPIO_MODE_NOCHANGE;
            bool value = false;
            int rc_val = hal_gpio_read(bankid, (uint8_t) pin, &value);
            int rc_fn = hal_gpio_get_function(bankid, (uint8_t) pin, &fn);
            int rc_md = hal_gpio_get_mode(bankid, (uint8_t) pin, &mode);

            _printf("    %u:%u  value=%s  function=%s  mode=%s",
                           (unsigned) bankid,
                           pin,
                           (rc_val == HAL_GPIO_OK) ? (value ? "1" : "0") : "?",
                           (rc_fn == HAL_GPIO_OK) ? hal_gpio_function_name(fn) : "?",
                           (rc_md == HAL_GPIO_OK) ? hal_gpio_mode_name(mode) : "?");

            if (rc_val != HAL_GPIO_OK || rc_fn != HAL_GPIO_OK || rc_md != HAL_GPIO_OK) {
                _printf("  [");
                if (rc_val != HAL_GPIO_OK) {
                    _printf("read=%d", rc_val);
                }
                if (rc_fn != HAL_GPIO_OK) {
                    _printf("%sgetfn=%d", (rc_val != HAL_GPIO_OK) ? " " : "", rc_fn);
                }
                if (rc_md != HAL_GPIO_OK) {
                    _printf("%sgetmode=%d", (rc_val != HAL_GPIO_OK || rc_fn != HAL_GPIO_OK) ? " " : "", rc_md);
                }
                _printf("]");
            }

            _printf("\r\n");
        }
    }
}

static void cmd_read(uint8_t bank, uint8_t pin)
{
    bool value = false;
    int rc = hal_gpio_read(bank, pin, &value);

    if (rc != HAL_GPIO_OK) {
        _printf("ERR read %u:%u -> %d\r\n", (unsigned)bank, (unsigned)pin, rc);
        return;
    }

    _printf("%u\r\n", value ? 1u : 0u);
}

static void cmd_write(uint8_t bank, uint8_t pin, bool value)
{
    int rc = hal_gpio_write(bank, pin, value);

    if (rc != HAL_GPIO_OK) {
        _printf("ERR write %u:%u -> %d\r\n", (unsigned)bank, (unsigned)pin, rc);
        return;
    }

    _printf("OK\r\n");
}

static void cmd_config(uint8_t bank, uint8_t pin, hal_gpio_function_t fn, hal_gpio_mode_t mode)
{
    int rc = hal_gpio_set_function(bank, pin, fn);
    if (rc != HAL_GPIO_OK) {
        _printf("ERR function %u:%u -> %d\r\n", (unsigned)bank, (unsigned)pin, rc);
        return;
    }

    rc = hal_gpio_set_mode(bank, pin, mode);
    if (rc != HAL_GPIO_OK) {
        _printf("ERR mode %u:%u -> %d\r\n", (unsigned)bank, (unsigned)pin, rc);
        return;
    }
    _printf("OK\r\n");
}

/* called when return is pressed in the terminal to process a command */
static int terminal_execute(struct microrl *mrl, int argc, const char * const *argv)
{
    size_t bank = 0;
    size_t pin = 0;

    MICRORL_UNUSED(mrl);

    if (argc <= 0 || argv == NULL || argv[0] == NULL || argv[0][0] == '\0') {
        return 0;
    }

    if (token_eq(argv[0], "list")) {
        cmd_list();
        return 0;
    }

    if (token_eq(argv[0], "read")) {
        if (argc != 2 || !parse_bank_pin(argv[1], &bank, &pin)) {
            cmd_usage();
            return 1;
        }

        cmd_read(bank, pin);
        return 0;
    }

    if (token_eq(argv[0], "write")) {
        bool value = false;

        if (argc != 3 || !parse_bank_pin(argv[1], &bank, &pin) || !parse_value01(argv[2], &value)) {
            cmd_usage();
            return 1;
        }

        cmd_write(bank, pin, value);
        return 0;
    }

    if (token_eq(argv[0], "config")) {
        hal_gpio_function_t fn;
        hal_gpio_mode_t mode = HAL_GPIO_MODE_NOCHANGE;

        if (argc < 3 || argc > 4 || !parse_bank_pin(argv[1], &bank, &pin) || !parse_function(argv[2], &fn)) {
            cmd_usage();
            return 1;
        }

        if (argc == 4 && !parse_mode(argv[3], &mode)) {
            cmd_usage();
            return 1;
        }

        cmd_config(bank, pin, fn, mode);
        return 0;
    }

    cmd_usage();
    return 1;
}

/* wrapper to register _printf() with microrl */
static int terminal_out(struct microrl *mrl, const char *str)
{
    MICRORL_UNUSED(mrl);
    return _printf(str);
}


#if MICRORL_CFG_USE_COMPLETE
/* return all matches from a list of choices for an incomplete prefix string */
static char **terminal_complete_prefix(const char * const* choices, const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    size_t i;
    size_t n = 0;

    for (i = 0; choices[i] != NULL && n < (MICRORL_ARRAYSIZE(s_complete_matches) - 1u); ++i) {
        if (strncasecmp(choices[i], prefix, prefix_len) == 0) {
            s_complete_matches[n++] = choices[i];
        }
    }

    if (n == 0) {
        s_complete_matches[n++] = s_empty;
    }

    s_complete_matches[n] = NULL;
    return (char **) s_complete_matches;
}

/* process tab-press for string completion */
static char **terminal_complete(struct microrl *mrl, int argc, const char * const *argv)
{
    MICRORL_UNUSED(mrl);

    /* first command after prompt ? */
    if (argc <= 1) {
        return terminal_complete_prefix(s_command_choices, argv[0] != NULL ? argv[0] : "");
    }

    /* argument to read command */
    if (token_eq(argv[0], s_cmd_read)) {
        /* bank:id */
        if (argc == 2) {
            return terminal_complete_prefix(s_bank_pin_choices, argv[1] != NULL ? argv[1] : "");
        }
    }

    /* argument to write command */
    if (token_eq(argv[0], s_cmd_write)) {
        /* bank:id */
        if (argc == 2) {
            return terminal_complete_prefix(s_bank_pin_choices, argv[1] != NULL ? argv[1] : "");
        }
        /* value */
        if (argc == 3) {
            return terminal_complete_prefix(s_pinval_choices, argv[2] != NULL ? argv[2] : "");
        }
    }

    /* argument to config command */
    if (token_eq(argv[0], s_cmd_config)) {
        /* bank:id */
        if (argc == 2) {
            return terminal_complete_prefix(s_bank_pin_choices, argv[1] != NULL ? argv[1] : "");
        }

        /* pin function */
        if (argc == 3) {
            return terminal_complete_prefix(s_function_choices, argv[2] != NULL ? argv[2] : "");
        }

        /* pin mode */
        if (argc == 4) {
            return terminal_complete_prefix(s_mode_choices, argv[3] != NULL ? argv[3] : "");
        }
    }

    /* no match */
    return (char **) s_no_completion;
}
#endif

/* regularly read & process input from terminal (if any) */
void terminal_task(void)
{
    if (tud_cdc_connected()) {
        uint8_t buf[64];
        uint32_t n;

        if (!s_cdc_ready) {
            s_cdc_ready = true;
            (void)microrl_clear_terminal(&s_rl);
        }

        while ((n = tud_cdc_read(buf, sizeof(buf))) > 0) {
            (void)microrl_processing_input(&s_rl, buf, n);
        }
    } else {
        s_cdc_ready = false;
    }
}

/* initialize microrl */
void terminal_init(void)
{
    // register terminal logging output handler
    ulog_output_add(_log_output_handler, NULL, ULOG_LEVEL_TRACE);

    microrl_init(&s_rl, terminal_out, terminal_execute);

#if MICRORL_CFG_USE_COMPLETE
    /* initialize bank pin choices */
    static char bankpins[512];
    static size_t space_left = sizeof(bankpins);
    static char *s = bankpins;
    static size_t completions = 0;
    /* walk all banks */
    for(uint8_t b = 0; b < hal_gpio_bankcount(); b++) {
        /* walk all pins */
        for(uint8_t p = 0; p < hal_gpio_bank_pincount(b); p++) {
            if(space_left > 0) {
                int written = snprintf(s, space_left, "%d:%d", b, p);
                space_left -= ((size_t) written + 1);
                /* store this completion */
                s_bank_pin_choices[completions] = s;
                /* next completion */
                completions++;
                /* next buffer += written_string + \0 */
                s += written+1;
            }
        }
    }
    /* initialize function string choices */
    hal_gpio_function_t f;
    for(f = 0; f < HAL_GPIO_FN_MAX; f++) {
        s_function_choices[f] = hal_gpio_function_name(f);
    }
    s_function_choices[f] = NULL;
    /* initialize mode string choices */
    hal_gpio_mode_t m;
    for(m = 0; m < HAL_GPIO_MODE_MAX; m++) {
        s_mode_choices[m] = hal_gpio_mode_name(m);
    }
    s_mode_choices[m] = NULL;

    microrl_set_complete_callback(&s_rl, terminal_complete);
#endif
}
