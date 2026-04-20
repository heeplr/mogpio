#include "terminal.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_gpio.h"
#include "hal_gpio_flavor.h"
#include "microrl.h"
#include "tusb.h"


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

static char s_fn_none[]   = "none";
static char s_fn_input[]  = "input";
static char s_fn_output[] = "output";

static char s_mode_pull_up[]   = "pull_up";
static char s_mode_pull_down[] = "pull_down";
static char s_mode_pushpull[]  = "pushpull";

static char s_empty[] = "";

static char *const s_command_choices[] = {
    s_cmd_list, s_cmd_read, s_cmd_write, s_cmd_config, NULL
};

static char *const s_function_choices[] = {
    s_fn_none, s_fn_input, s_fn_output, NULL
};

static char *const s_mode_choices[] = {
    s_mode_pull_up, s_mode_pull_down, s_mode_pushpull, NULL
};

static char *const s_no_completion[] = {
    s_empty, NULL
};

static char *s_complete_matches[8];
#endif

/* return printable name of HAL_GPIO_FN_* */
static const char *function_name(hal_gpio_function_t fn)
{
    switch (fn) {
    case HAL_GPIO_FN_NONE:    return "none";
    case HAL_GPIO_FN_INPUT:   return "input";
    case HAL_GPIO_FN_OUTPUT:  return "output";
    case HAL_GPIO_FN_NOCHANGE:return "nochange";
    default:                  return "unknown";
    }
}

/* return printable name of HAL_GPIO_MODE_* */
static const char *mode_name(hal_gpio_mode_t mode)
{
    switch (mode) {
    case HAL_GPIO_MODE_PULL_UP:    return "pull_up";
    case HAL_GPIO_MODE_PULL_DOWN:  return "pull_down";
    case HAL_GPIO_MODE_PUSHPULL:   return "pushpull";
    case HAL_GPIO_MODE_NOCHANGE:   return "nochange";
    default:                       return "unknown";
    }
}

static bool parse_u8(const char *s, uint8_t *out)
{
    char *end = NULL;
    long v;

    if (s == NULL || out == NULL || *s == '\0') {
        return false;
    }

    v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || v < 0 || v > 255) {
        return false;
    }

    *out = (uint8_t)v;
    return true;
}

static bool parse_bank_pin(const char *s, uint8_t *bank, uint8_t *pin)
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

static bool parse_value01(const char *s, bool *value)
{
    if (s == NULL || value == NULL) {
        return false;
    }

    if (strcmp(s, "0") == 0 || strcmp(s, "low") == 0 || strcmp(s, "off") == 0) {
        *value = false;
        return true;
    }

    if (strcmp(s, "1") == 0 || strcmp(s, "high") == 0 || strcmp(s, "on") == 0) {
        *value = true;
        return true;
    }

    return false;
}

static bool parse_function(const char *s, hal_gpio_function_t *fn)
{
    long v;
    char *end = NULL;

    if (s == NULL || fn == NULL) {
        return false;
    }

    if (strcmp(s, "none") == 0) {
        *fn = HAL_GPIO_FN_NONE;
        return true;
    }

    if (strcmp(s, "input") == 0) {
        *fn = HAL_GPIO_FN_INPUT;
        return true;
    }

    if (strcmp(s, "output") == 0) {
        *fn = HAL_GPIO_FN_OUTPUT;
        return true;
    }

    v = strtol(s, &end, 10);
    if (end != s && *end == '\0' && v >= 0 && v <= 2) {
        *fn = (hal_gpio_function_t)v;
        return true;
    }

    return false;
}

static bool parse_mode(const char *s, hal_gpio_mode_t *mode)
{
    long v;
    char *end = NULL;

    if (s == NULL || mode == NULL) {
        return false;
    }

    if (strcmp(s, "pull_up") == 0) {
        *mode = HAL_GPIO_MODE_PULL_UP;
        return true;
    }

    if (strcmp(s, "pull_down") == 0) {
        *mode = HAL_GPIO_MODE_PULL_DOWN;
        return true;
    }

    if (strcmp(s, "pushpull") == 0) {
        *mode = HAL_GPIO_MODE_PUSHPULL;
        return true;
    }

    v = strtol(s, &end, 10);
    if (end != s && *end == '\0' && v >= 0 && v <= 2) {
        *mode = (hal_gpio_mode_t)v;
        return true;
    }

    return false;
}

static void print_usage(void)
{
    terminal_write("\r\n"
                   "Commands:\r\n"
                   "  list\r\n"
                   "  read <bank>:<pin>\r\n"
                   "  write <bank>:<pin> <0|1>\r\n"
                   "  config <bank>:<pin> <function> [mode]\r\n"
                   "    function: none|input|output\r\n"
                   "    mode: pull_up|pull_down|pushpull\r\n");
}

static void print_list(void)
{
    size_t bank_index;

    terminal_write("\r\nGPIOs:\r\n");

    for (bank_index = 0; bank_index < g_hal_gpio_flavor.bank_count; ++bank_index) {
        const hal_gpio_bank_t *bank = &g_hal_gpio_flavor.banks[bank_index];
        unsigned int pin;

        terminal_write("  Bank %u (%s), %u pins\r\n",
                       (unsigned)bank->bank_id,
                       bank->name != NULL ? bank->name : "(unnamed)",
                       (unsigned)bank->pin_count);

        for (pin = 0; pin < bank->pin_count; ++pin) {
            bool value = false;
            hal_gpio_function_t fn = HAL_GPIO_FN_NOCHANGE;
            hal_gpio_mode_t mode = HAL_GPIO_MODE_NOCHANGE;
            int rc_val = hal_gpio_read(bank->bank_id, (uint8_t)pin, &value);
            int rc_fn = hal_gpio_get_function(bank->bank_id, (uint8_t)pin, &fn);
            int rc_md = hal_gpio_get_mode(bank->bank_id, (uint8_t)pin, &mode);

            terminal_write("    %u:%u  value=%s  function=%s  mode=%s",
                           (unsigned)bank->bank_id,
                           pin,
                           (rc_val == HAL_GPIO_OK) ? (value ? "1" : "0") : "?",
                           (rc_fn == HAL_GPIO_OK) ? function_name(fn) : "?",
                           (rc_md == HAL_GPIO_OK) ? mode_name(mode) : "?");

            if (rc_val != HAL_GPIO_OK || rc_fn != HAL_GPIO_OK || rc_md != HAL_GPIO_OK) {
                terminal_write("  [");
                if (rc_val != HAL_GPIO_OK) {
                    terminal_write("read=%d", rc_val);
                }
                if (rc_fn != HAL_GPIO_OK) {
                    terminal_write("%sgetfn=%d", (rc_val != HAL_GPIO_OK) ? " " : "", rc_fn);
                }
                if (rc_md != HAL_GPIO_OK) {
                    terminal_write("%sgetmode=%d", (rc_val != HAL_GPIO_OK || rc_fn != HAL_GPIO_OK) ? " " : "", rc_md);
                }
                terminal_write("]");
            }

            terminal_write("\r\n");
        }
    }
}

static void print_read(uint8_t bank, uint8_t pin)
{
    bool value = false;
    int rc = hal_gpio_read(bank, pin, &value);

    if (rc != HAL_GPIO_OK) {
        terminal_write("ERR read %u:%u -> %d\r\n", (unsigned)bank, (unsigned)pin, rc);
        return;
    }

    terminal_write("%u\r\n", value ? 1u : 0u);
}

static void print_write(uint8_t bank, uint8_t pin, bool value)
{
    int rc = hal_gpio_write(bank, pin, value);

    if (rc != HAL_GPIO_OK) {
        terminal_write("ERR write %u:%u -> %d\r\n", (unsigned)bank, (unsigned)pin, rc);
        return;
    }

    terminal_write("OK\r\n");
}

static void print_config(uint8_t bank, uint8_t pin, hal_gpio_function_t fn, hal_gpio_mode_t mode)
{
    int rc = hal_gpio_pin_config(bank, pin, fn, mode);

    if (rc != HAL_GPIO_OK) {
        terminal_write("ERR config %u:%u -> %d\r\n", (unsigned)bank, (unsigned)pin, rc);
        return;
    }

    terminal_write("OK\r\n");
}

/* called when return is pressed in the terminal to process a command */
static int terminal_execute(struct microrl *mrl, int argc, const char * const *argv)
{
    uint8_t bank = 0;
    uint8_t pin = 0;

    MICRORL_UNUSED(mrl);

    if (argc <= 0 || argv == NULL || argv[0] == NULL || argv[0][0] == '\0') {
        return 0;
    }

    if (strcmp(argv[0], "list") == 0) {
        print_list();
        return 0;
    }

    if (strcmp(argv[0], "read") == 0) {
        if (argc != 2 || !parse_bank_pin(argv[1], &bank, &pin)) {
            print_usage();
            return 1;
        }

        print_read(bank, pin);
        return 0;
    }

    if (strcmp(argv[0], "write") == 0) {
        bool value = false;

        if (argc != 3 || !parse_bank_pin(argv[1], &bank, &pin) || !parse_value01(argv[2], &value)) {
            print_usage();
            return 1;
        }

        print_write(bank, pin, value);
        return 0;
    }

    if (strcmp(argv[0], "config") == 0) {
        hal_gpio_function_t fn;
        hal_gpio_mode_t mode = HAL_GPIO_MODE_NOCHANGE;

        if (argc < 3 || argc > 4 || !parse_bank_pin(argv[1], &bank, &pin) || !parse_function(argv[2], &fn)) {
            print_usage();
            return 1;
        }

        if (argc == 4 && !parse_mode(argv[3], &mode)) {
            print_usage();
            return 1;
        }

        print_config(bank, pin, fn, mode);
        return 0;
    }

    print_usage();
    return 1;
}

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

/* printf to terminal */
int terminal_write(const char *fmt, ...)
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

    // Send only what fits in the local buffer, but don't silently overrun.
    size_t len = strnlen(buf, sizeof(buf));
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

/* wrapper to register terminal_write() with microrl */
static int terminal_out(struct microrl *mrl, const char *str)
{
    MICRORL_UNUSED(mrl);
    return terminal_write(str);
}


#if MICRORL_CFG_USE_COMPLETE
/* return all matches from a list of choices for an incomplete prefix string */
static char **terminal_complete_prefix(char *const *choices, const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    size_t i;
    size_t n = 0;

    for (i = 0; choices[i] != NULL && n < (MICRORL_ARRAYSIZE(s_complete_matches) - 1u); ++i) {
        if (strncmp(choices[i], prefix, prefix_len) == 0) {
            s_complete_matches[n++] = choices[i];
        }
    }

    if (n == 0) {
        s_complete_matches[n++] = s_empty;
    }

    s_complete_matches[n] = NULL;
    return s_complete_matches;
}

/* process tab-press for string completion */
static char **terminal_complete(struct microrl *mrl, int argc, const char * const *argv)
{
    MICRORL_UNUSED(mrl);

    /* first command after prompt ? */
    if (argc <= 1) {
        return terminal_complete_prefix(s_command_choices, argv[0] != NULL ? argv[0] : "");
    }

    /* argument to some command */
    if (strcmp(argv[0], s_cmd_config) == 0) {
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

/* initialize microrl */
void terminal_init(void)
{
    (void)microrl_init(&s_rl, terminal_out, terminal_execute);

#if MICRORL_CFG_USE_COMPLETE
    (void)microrl_set_complete_callback(&s_rl, terminal_complete);
#endif
}
