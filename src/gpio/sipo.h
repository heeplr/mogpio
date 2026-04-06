
#include "hal_gpio_board.h"


#define HAL_SIPO_MAX_PINS     16u

typedef struct {
    uint8_t data_pin;     // Serial data from to the SIPO chain.
    uint8_t clock_pin;    // Shift clock.
    uint8_t latch_pin;    // Storage-register latch.
    uint8_t pin_count;    // Total number of output bits exposed.
    bool reverse_order;   // Flip bit order if the chain is wired backwards.

    bool shadow_bits[HAL_SIPO_MAX_PINS];
    hal_gpio_function_t function[HAL_SIPO_MAX_PINS];
    bool configured[HAL_SIPO_MAX_PINS];
} hal_gpio_sipo_ctx_t;


extern const hal_gpio_driver_ops_t hal_gpio_sipo_ops;
