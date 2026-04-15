/*
 * hal_gpio_board.h
 *
 * Compile-time board layout
 */

#ifndef HAL_GPIO_BOARD_H
#define HAL_GPIO_BOARD_H

#include "hal_gpio.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Callbacks to be implemented by drivers
 */
typedef struct hal_gpio_driver_ops {
    int (*init)(void *ctx);
    int (*deinit)(void *ctx);
    int (*pin_config)(void *ctx, uint8_t pin,
                      hal_gpio_function_t function,
                      hal_gpio_mode_t mode);
    int (*read)(void *ctx, uint8_t pin, bool *value);
    int (*write)(void *ctx, uint8_t pin, bool value);
    int (*get_function)(void *ctx, uint8_t pin, hal_gpio_function_t *function);
    int (*get_mode)(void *ctx, uint8_t pin, hal_gpio_mode_t *mode);
} hal_gpio_driver_ops_t;


/* A bank is a single logical block of GPIOs. */
typedef struct {
    uint8_t bank_id;
    const char *name;
    uint8_t pin_count;
    const hal_gpio_driver_ops_t *ops;
    void *ctx;
} hal_gpio_bank_t;

/*
 * The complete board description.
 *
 * This is the single handoff point from board layout into the HAL.
 */
typedef struct {
    const hal_gpio_bank_t *banks;
    size_t bank_count;
} hal_gpio_board_t;


/*
 * The active board description.
 *
 * The HAL core reads this symbol and dispatches every API call through it.
 */
extern const hal_gpio_board_t g_hal_gpio_board;

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_BOARD_H */
