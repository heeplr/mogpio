
#ifndef _RPI_PICO
#define _RPI_PICO

#ifdef DEBUG_OUT
#define HAL_PICO_MAX_PINS 28u
#else
#define HAL_PICO_MAX_PINS 30u
#endif

typedef struct {
    uint8_t first_gpio;   /* First GPIO used by this bank. */
    uint8_t pin_count;    /* Number of pins exposed */

    /*
     * Bookkeeping lets the driver reject illegal operations cleanly.
     * The HAL core does not interpret these arrays; they are purely driver
     * implementation details.
     */
    hal_gpio_function_t function[HAL_PICO_MAX_PINS];
    hal_gpio_mode_t mode[HAL_PICO_MAX_PINS];
    bool configured[HAL_PICO_MAX_PINS];
} hal_gpio_pico_ctx_t;


extern const hal_gpio_driver_ops_t hal_gpio_pico_ops;

#endif /* _RPI_PICO */
