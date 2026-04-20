/*
 * hal_gpio.h
 *
 * Public GPIO HAL API.
 *
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A GPIO can be left unassigned (NONE), configured as input or configured as
 * output.
 */
typedef enum {
    HAL_GPIO_FN_MIN = -1,
    HAL_GPIO_FN_NOCHANGE = 0,
    HAL_GPIO_FN_NONE,
    HAL_GPIO_FN_INPUT,
    HAL_GPIO_FN_OUTPUT,
    HAL_GPIO_FN_MAX
} hal_gpio_function_t;

/* The mode describes how the pin is biased or driven. */
typedef enum {
    HAL_GPIO_MODE_MIN = -1,
    HAL_GPIO_MODE_NOCHANGE = 0,
    HAL_GPIO_MODE_PULL_UP,
    HAL_GPIO_MODE_PULL_DOWN,
    HAL_GPIO_MODE_PUSHPULL,
    HAL_GPIO_MODE_MAX
} hal_gpio_mode_t;

/* codes for error handling */
enum {
    HAL_GPIO_OK              = 0,
    HAL_GPIO_ERR_INVAL       = -1,  /* invalid argument */
    HAL_GPIO_ERR_NOT_INIT    = -2,  /* hal_gpio_init() has not completed */
    HAL_GPIO_ERR_UNSUPPORTED = -3,  /* requested operation does not fit bank */
    HAL_GPIO_ERR_BOUNDS      = -4,  /* bank or pin is out of range */
    HAL_GPIO_ERR_STATE       = -5,  /* bank/pin is in the wrong state */
};


int hal_gpio_init(void);
int hal_gpio_deinit(void);
int hal_gpio_pin_config(uint8_t bankid, uint8_t pin,
                        hal_gpio_function_t function,
                        hal_gpio_mode_t mode);

int hal_gpio_read(uint8_t bankid, uint8_t pin, bool *value);
int hal_gpio_write(uint8_t bankid, uint8_t pin, bool value);
int hal_gpio_get_function(uint8_t bankid, uint8_t pin, hal_gpio_function_t *function);
int hal_gpio_get_mode(uint8_t bankid, uint8_t pin, hal_gpio_mode_t *mode);

const char *hal_gpio_function_name(hal_gpio_function_t fn);
const char *hal_gpio_mode_name(hal_gpio_mode_t mode);

unsigned int hal_gpio_bankcount(void);
unsigned int hal_gpio_bank_pincount(uint8_t bankid);
unsigned int hal_gpio_pincount(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */
