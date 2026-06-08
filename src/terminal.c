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

#include <stddef.h>
#include <string.h>

#include "console.h"
#include "terminal.h"
#include "ulog.h"
#include "hal_gpio.h"
#include "hal_gpio_layout.h"
#include "microrl.h"
#include "util.h"


/* microrl context */
static microrl_t s_rl;

/* command dispatcher */
typedef int (*terminal_cmd_exec_t)(int argc, const char * const *argv);

/* one terminal command */
typedef struct {
    const char *name;
    terminal_cmd_exec_t exec;
    /* tab-completion for max. 5 arguments*/
    const char *const *choices[5];
} terminal_cmd_t;

/* buffers to hold auto-generated autocompletion choices */
static const char *s_bank_pin_choices[HAL_GPIO_BANKS_MAX * HAL_GPIO_PINS_MAX + 1];
static const char *s_function_choices[HAL_GPIO_FN_MAX + 1];
static const char *s_mode_choices[HAL_GPIO_MODE_MAX + 1];
static const char *s_boolean_choices[] = { "0", "1", NULL };

/* empty match result */
static char *const s_no_completion[] = { "", NULL };
static const char *s_complete_matches[64];



/** helpers */
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

    return console_write(buf, sizeof(buf));
}


/** command functions */
static void cmd_usage(void)
{
    ulog_level loglevel;

    _printf("\r\nloglevel:\r\n");
    ulog_topic_level_get("usbio", &loglevel);
    _printf(" usbio:\t\t%s\r\n", ulog_level_to_string(loglevel));
    ulog_topic_level_get("msc", &loglevel);
    _printf(" msc:\t\t%s\r\n", ulog_level_to_string(loglevel));
    ulog_topic_level_get("terminal", &loglevel);
    _printf(" terminal:\t%s\r\n", ulog_level_to_string(loglevel));
    ulog_topic_level_get("driver", &loglevel);
    _printf(" driver:\t%s\r\n", ulog_level_to_string(loglevel));
    ulog_topic_level_get("hal", &loglevel);
    _printf(" hal:\t\t%s\r\n", ulog_level_to_string(loglevel));

    _printf("\r\n"
       "Commands:\r\n"
       "  list\r\n"
       "  read <bank>:<pin>\r\n"
       "  write <bank>:<pin> <0|1>\r\n"
       "  config <bank>:<pin> <function> [mode]\r\n");
    _printf(
       "    function: none|input|output\r\n"
       "    mode: pull_up|pull_down|pushpull\r\n"
       "  log_level <all|usbio|msc|terminal|driver|hal|all> <trace|debug|info|warn|error|fatal>\r\n");
}

static int cmd_list(int argc, const char * const *argv)
{
    MICRORL_UNUSED(argc);
    MICRORL_UNUSED(argv);

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
    return 0;
}

static int cmd_read(int argc, const char * const *argv)
{
    size_t bank = 0;
    size_t pin = 0;

    if (argc == 2 && parse_bank_pin(argv[1], &bank, &pin)) {
        bool value = false;
        int rc = hal_gpio_read((uint8_t) bank, (uint8_t) pin, &value);

        if (rc != HAL_GPIO_OK) {
            ulog_topic_error("terminal", "failed to read bank/pin %d:%d (result: %d)", bank, pin, rc);
            return 1;
        }

        _printf("%u\r\n", value ? 1u : 0u);
        return 0;
    }

    cmd_usage();
    return 1;
}

static int cmd_write(int argc, const char * const *argv)
{
    size_t bank = 0;
    size_t pin = 0;
    bool value = false;

    if (argc == 3 &&
        parse_bank_pin(argv[1], &bank, &pin) &&
        parse_boolean(argv[2], &value)) {
        int rc = hal_gpio_write(bank, pin, value);

        if (rc != HAL_GPIO_OK) {
            ulog_topic_error("terminal", "failed to write bank/pin %u:%u (result: %d)", bank, pin, rc);
            return 1;
        }
        return 0;
    }

    cmd_usage();
    return 1;

}

static int cmd_config(int argc, const char * const *argv)
{
    size_t bank = 0;
    size_t pin = 0;
    hal_gpio_function_t fn;
    hal_gpio_mode_t mode = HAL_GPIO_MODE_NOCHANGE;

    if (argc < 3 || argc > 4) {
        ulog_topic_error("terminal", "invalid amount of arguments: %d", argc);
        cmd_usage();
        return 1;
    }
    if(!parse_bank_pin(argv[1], &bank, &pin)) {
        ulog_topic_error("terminal", "invalid bank/pin: %s", argv[1]);
        cmd_usage();
        return 1;
    }
    if(!parse_function(argv[2], &fn)) {
        ulog_topic_error("terminal", "invalid function: %s", argv[2]);
        cmd_usage();
        return 1;
    }
    if (!parse_mode(argv[3], &mode)) {
        ulog_topic_error("terminal", "invalid mode: %s", argv[3]);
        cmd_usage();
        return 1;
    }

    int rc = hal_gpio_set_function(bank, pin, fn);
    if (rc != HAL_GPIO_OK) {
        ulog_topic_error("terminal", "failed to configure bank/pin %u:%u (result: %d)", (unsigned)bank, (unsigned)pin, rc);
        return 1;
    }

    rc = hal_gpio_set_mode(bank, pin, mode);
    if (rc != HAL_GPIO_OK) {
        ulog_topic_error("terminal", "failed to set mode of bank/pin %u:%u (result: %d)", (unsigned)bank, (unsigned)pin, rc);
        return 1;
    }
    return 0;
}

static int cmd_log_level(int argc, const char * const *argv)
{
    ulog_level level = ULOG_LEVEL_TOTAL;

    if (argc != 3) {
        ulog_topic_error("terminal", "invalid amount of arguments: %d", argc);
        cmd_usage();
        return 1;
    }

    if (token_eq(argv[2], "trace"))
        level = ULOG_LEVEL_TRACE;
    else if (token_eq(argv[2], "debug"))
        level = ULOG_LEVEL_DEBUG;
    else if (token_eq(argv[2], "info"))
        level = ULOG_LEVEL_INFO;
    else if (token_eq(argv[2], "warn"))
        level = ULOG_LEVEL_WARN;
    else if (token_eq(argv[2], "error"))
        level = ULOG_LEVEL_ERROR;
    else if (token_eq(argv[2], "fatal"))
        level = ULOG_LEVEL_FATAL;

    const char *topic = argv[1];

    if (level != ULOG_LEVEL_TOTAL) {
        /* set level of topic */
        if (token_eq(topic, "all")) {
            ulog_topic_level_set("usbio", level);
            ulog_topic_level_set("msc", level);
            ulog_topic_level_set("terminal", level);
            ulog_topic_level_set("driver", level);
            ulog_topic_level_set("hal", level);
            return 0;
        }
        else {
            ulog_status rc = ulog_topic_level_set(argv[1], level);
            if(rc == ULOG_STATUS_OK) {
                return 0;
            }
            ulog_topic_error("terminal", "invalid topic: %s", argv[1]);
        }
    }

    ulog_topic_error("terminal", "invalid loglevel: %s", argv[2]);
    cmd_usage();
    return 1;
}

/* list of commands */
static const terminal_cmd_t s_commands[] = {
    {
        .name = "list",
        .exec = cmd_list,
    },
    {
        .name = "read",
        .exec = cmd_read,
        .choices = {
            s_bank_pin_choices,
        },
    },
    {
        .name = "write",
        .exec = cmd_write,
        .choices = {
            s_bank_pin_choices,
            s_boolean_choices
        },
    },
    {
        .name = "config",
        .exec = cmd_config,
        .choices = {
            s_bank_pin_choices,
            s_function_choices,
            s_mode_choices,
        },
    },
    {
        .name = "log_level",
        .exec = cmd_log_level,
        .choices = {
            (const char *const[]) { "all", "usbio", "msc", "terminal", "driver", NULL },
            (const char *const[]) { "trace", "debug", "info", "warn", "error", "fatal", NULL },
        },
    },
};

/******************************************************************************/
/* get command by name */
static const terminal_cmd_t *_find_cmd(const char *name)
{
    if (name == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < MICRORL_ARRAYSIZE(s_commands); ++i) {
        if (token_eq(name, s_commands[i].name)) {
            return &s_commands[i];
        }
    }

    return NULL;
}

/* called when return is pressed in the terminal to process a command */
static int terminal_execute(struct microrl *mrl, int argc, const char * const *argv)
{
    MICRORL_UNUSED(mrl);

    const terminal_cmd_t *cmd;

    if (argc <= 0 || argv == NULL || argv[0] == NULL || argv[0][0] == '\0') {
        return 0;
    }

    cmd = _find_cmd(argv[0]);
    if (cmd != NULL) {
        return cmd->exec(argc, argv);
    }

    ulog_topic_warn("terminal", "command %s failed", argv[0]);
    cmd_usage();
    return 1;
}

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
        s_complete_matches[n++] = "";
    }

    s_complete_matches[n] = NULL;
    return (char **) s_complete_matches;
}

/* return all main command matches built from s_commands */
static char **terminal_complete_command_prefix(const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    size_t n = 0;

    for (size_t i = 0; i < MICRORL_ARRAYSIZE(s_commands) &&
                       n < (MICRORL_ARRAYSIZE(s_complete_matches) - 1u); ++i) {
        if (strncasecmp(s_commands[i].name, prefix, prefix_len) == 0) {
            s_complete_matches[n++] = s_commands[i].name;
        }
    }

    if (n == 0) {
        s_complete_matches[n++] = "";
    }

    s_complete_matches[n] = NULL;
    return (char **)s_complete_matches;
}

/* process tab-press for string completion */
static char **terminal_complete(struct microrl *mrl, int argc, const char * const *argv)
{
    const terminal_cmd_t *cmd;

    MICRORL_UNUSED(mrl);

    /* first command after prompt ? */
    if (argc <= 1) {
        return terminal_complete_command_prefix(argv[0] != NULL ? argv[0] : "");
    }

    cmd = _find_cmd(argv[0]);
    if (cmd == NULL) {
        return (char **) s_no_completion;
    }

    /* argv[1] == first argument, so completion index is argc - 1 */
    if (argc >= 2 && argc <= 5) {
        const char *const *choices = cmd->choices[argc - 2];
        if (choices != NULL) {
            return terminal_complete_prefix(choices, argv[argc - 1] != NULL ? argv[argc - 1] : "");
        }
    }

    return (char **)s_no_completion;
}

/* wrapper to register _printf() with microrl */
static int terminal_out(struct microrl *mrl, const char *str)
{
    MICRORL_UNUSED(mrl);
    return _printf(str);
}

/* regularly read & process input from terminal (if any) */
void terminal_task(void)
{
    /* read console input */
    char buf[64];
    uint32_t n;

    while ((n = console_read_available(buf, sizeof(buf))) > 0) {
        (void) microrl_processing_input(&s_rl, buf, n);
    }
}

/* initialize microrl */
void terminal_init(void)
{
    microrl_init(&s_rl, terminal_out, terminal_execute);

    /* initialize bank pin choices for tab completion */
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
    /* initialize function string choices for tab completion */
    hal_gpio_function_t f;
    for(f = 0; f < HAL_GPIO_FN_MAX; f++) {
        s_function_choices[f] = hal_gpio_function_name(f);
    }
    s_function_choices[f] = NULL;
    /* initialize mode string choices for tab completion */
    hal_gpio_mode_t m;
    for(m = 0; m < HAL_GPIO_MODE_MAX; m++) {
        s_mode_choices[m] = hal_gpio_mode_name(m);
    }
    s_mode_choices[m] = NULL;

    /* handler to return completion choices when tab is pressed */
    microrl_set_complete_callback(&s_rl, terminal_complete);
}
