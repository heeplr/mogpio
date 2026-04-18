
#ifndef _PISO_H
#define _PISO_H

#define HAL_PISO_MAX_PINS     32u


typedef struct {
    uint8_t data_pin;     // Serial data from the PISO chain into the Pico.
    uint8_t clock_pin;    // Shift clock.
    uint8_t load_pin;     // Parallel load / latch control.
    uint8_t pin_count;    // Total number of input bits exposed.
    bool reverse_order;   // Flip bit order if the chain is wired backwards.

    bool cached_bits[HAL_PISO_MAX_PINS];
    hal_gpio_function_t function[HAL_PISO_MAX_PINS];
    bool configured[HAL_PISO_MAX_PINS];
} hal_gpio_piso_ctx_t;


extern const hal_gpio_driver_ops_t hal_gpio_piso_ops;

#endif /* _PISO_H */
