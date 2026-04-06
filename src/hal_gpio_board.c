/*
 * hal_gpio_board.c
 *
 * Board description for the GPIO HAL.
 *
 * This file is the single place where the HAL learns:
 *   - which bank exists,
 *   - which driver handles that bank,
 *   - how many pins the bank exposes,
 *   - and which hardware pins or shift-register lines the driver uses.
 *
 * The public API does not need to know any of this. The core HAL simply
 * iterates over the board description and calls the appropriate driver via
 * function pointers.
 *
 * This is the only file that knows which logical bank uses which driver and
 * which Pico pins / shift-register lines that bank is attached to.
 */

/* todo: make this conditional */
#include "gpio/rpi_pico.h"
#include "gpio/piso.h"
#include "gpio/sipo.h"

#include "hal_gpio_board.h"


enum {
    HAL_BANK_PICO     = 0,
    HAL_BANK_PISO     = 1,
    HAL_BANK_SIPO     = 2,
    HAL_BANK_COUNT    = 3,
};


/*
 * The driver contexts are mutable because the drivers store runtime state such
 * as configuration flags and cached/shadowed pin values.
 */
static hal_gpio_pico_ctx_t s_pico_ctx = {
#ifdef DEBUG_OUT
    .first_gpio = 2,    // GPIO0 and GPIO1 used for UART0
#else
    .first_gpio = 0,    // use all pins
#endif
    .pin_count = HAL_PICO_MAX_PINS,
};

//~ static hal_gpio_piso_ctx_t s_piso_ctx = {
    //~ .data_pin = 2,
    //~ .clock_pin = 3,
    //~ .load_pin = 4,
    //~ .pin_count = HAL_PISO_MAX_PINS,
    //~ .reverse_order = false,
//~ };

//~ static hal_gpio_sipo_ctx_t s_sipo_ctx = {
    //~ .data_pin = 6,
    //~ .clock_pin = 7,
    //~ .latch_pin = 8,
    //~ .pin_count = HAL_SIPO_MAX_PINS,
    //~ .reverse_order = false,
//~ };

static const hal_gpio_bank_desc_t s_banks[] = {
    {
        .bank_id = HAL_BANK_PICO,
        .name = "pico-gpio",
        .pin_count = HAL_PICO_MAX_PINS,
        .ops = &hal_gpio_pico_ops,
        .ctx = &s_pico_ctx,
    },
    //~ {
        //~ .bank_id = HAL_BANK_PISO,
        //~ .name = "piso-input-chain",
        //~ .pin_count = HAL_PISO_MAX_PINS,
        //~ .ops = &hal_gpio_piso_ops,
        //~ .ctx = &s_piso_ctx,
    //~ },
    //~ {
        //~ .bank_id = HAL_BANK_SIPO,
        //~ .name = "sipo-output-chain",
        //~ .pin_count = HAL_SIPO_MAX_PINS,
        //~ .ops = &hal_gpio_sipo_ops,
        //~ .ctx = &s_sipo_ctx,
    //~ },
};

const hal_gpio_board_desc_t g_hal_gpio_board = {
    .banks = s_banks,
    .bank_count = sizeof(s_banks) / sizeof(s_banks[0]),
};
