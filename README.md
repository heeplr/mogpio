

moGPIO provides straight forward plug & play GPIO via USB



# Motivation

A lot of USB-to-GPIO adapters exist but I found none that is really plug & play,
let alone standard [libgpiod API](https://libgpiod.readthedocs.io) compatible.


# Overview

access GPIOs using:
  * standard libgpiod API (usbio)
  * interactive terminal (virtual serial port)
  * reading/writing files on emulated mass storage device


# Install

Copy
* mogpio-intern-pico.uf2 to your pico.
* mogpio-intern-pico2.uf2 to your pico2

(s. [latest release](https://github.com/heeplr/mogpio/releases))



# Usage

There are three ways moGPIO exposes GPIOs:



## 1. USBIO Protocol

moGPIO will show up as /dev/gpiochipX[^1] once you plug it in, ready to use.
It's compatible to all available applications that build upon libgpiod.
(e.g. https://docs.kernel.org/driver-api/driver/drivers-on-gpio.html).

moGPIO uses the [usbio](https://github.com/intel/usbio-drivers) driver that
should come with any linux kernel >=6.18.x

[^1]: As the time of writing, the usbio protocol supports 5 * 32 GPIO max per device. Any more pins can be controlled only with terminal or mass storage mode. Also limiting to 32 banks is a memory footprint compromise and can be increased in the future. (Please [open an issue](https://github.com/heeplr/mogpio/issues) if you need more).


1. plug into USB port
2. moGPIO will register as new `/dev/gpiochipX` using the `usbio` driver.
3. Use commandline[^2] tools `gpiodetect`, `gpioinfo`, `gpioget`, `gpioset`
   or the libgpiod API bindings from the language of your choice

[^2]: `gpiomon` command and libgpiod events will fail as interrupts are not
       supported in the usbio driver (yet).




## 2. CDC terminal interface

moGPIO provides a serial device you can connect to access a terminal.
Available commands:

* `?` or `help` print usage information
* `list` to list configured GPIOs and their state
* `read <bank>:<pin>` to read a GPIO input pin value
* `write <bank>:<pin> <0|1>` to set a GPIO output pin value
* `config <bank>:<pin> <function> [<mode>]` to configure a pin

The terminal uses [microrl-remaster](https://github.com/dimmykar/microrl-remaster) and provides

* line editing
* command history
* tab completion



## 3. Mass Storage Class Device

moGPIO provides a small FAT16 partition with `PINS.TXT` and `CONFIG.TXT` files.

The files contents represent the state at time of read. Writes will affect the
state on actual write.
(Some OS' cache agressively and won't write until eject/unmount or explicit flush.)


### PINS.TXT

Format:

`<bank>:<pin>=<value>`

Example:
```
0:0=0
0:1=0
0:2=0
0:3=0
0:4=0
0:5=0
0:6=0
0:7=0
```


### CONFIG.TXT

Format:

`<bank>:<pin>=<function>,<mode>`

* functions
 * `IN` (input pin)
 * `OUT` (putput pin)

* modes
 * `UP` (pull up resistor)
 * `DOWN` (pull down resistor)
 * `PUSHPULL` (push-pull mode)
 * `DEFAULT` (don't change default)


Example:
```
0:0=IN,DOWN
0:1=IN,DOWN
0:2=IN,DOWN
0:3=IN,DOWN
0:4=IN,DOWN
0:5=IN,DOWN
0:6=IN,DOWN
0:7=IN,DOWN
0:8=OUT,PUSHPULL
```


# Development

It should be easily possible to port moGPIO to other platforms or add different
GPIO hardware layouts.

Currently there are 3 layouts:

* intern (internal GPIOs of platform)
* sipo-piso (bit-bang serial shift registers)
* intern-sipo-piso (combination of the above)


A new layout/driver can be implemented easily:

1. create `dirver/yourdriver.c` to access the GPIOs and implement the
   following functions and exports them using a `hal_gpio_driver_ops_t` structure:


```C
static int init(void *vctx)
{
    // init hardware ...
    return HAL_GPIO_OK;
}

static int deinit(void *vctx)
{
    // deinit hardware ...
    return HAL_GPIO_OK;
}

static int set_function(void *vctx, size_t pin, hal_gpio_function_t function)
{
    // set GPIO pin function (input/output/...)
    return HAL_GPIO_OK;
}

static int set_mode(void *vctx, size_t pin, hal_gpio_mode_t mode)
{
    // set GPIO pin mode (pull-up/down, ...)
    return HAL_GPIO_OK;
}

static int get_function(void *vctx, size_t pin, hal_gpio_function_t *function)
{
    // get current function of pin
    return HAL_GPIO_OK;
}

static int get_mode(void *vctx, size_t pin, hal_gpio_mode_t *mode)
{
    // get current mode of pin
    return HAL_GPIO_OK;
}

static int read(void *vctx, size_t pin, bool *value)
{
    // read value from (input) pin
    return HAL_GPIO_OK;
}

static int write(void *vctx, size_t pin, bool value)
{
    // write value to output pin
    return HAL_GPIO_ERR_UNSUPPORTED;
}


const hal_gpio_driver_ops_t hal_gpio_yourdriver_ops = {
    .init = init,
    .deinit = deinit,
    .read = read,
    .write = write,
    .set_function = set_function,
    .set_mode = set_mode,
    .get_function = get_function,
    .get_mode = get_mode
};
```

2. create a new `layouts/yourlayout.c` or add your driver to an existing one:

```C

static hal_gpio_yourdriver_ctx_t s_yourdriver_ctx = {
    // holds stuff for this instance of your driver
}

/* GPIO hardware driver layout */
static const hal_gpio_driver_t s_drivers[] = {
    {
        .ops = &hal_gpio_yourdriver_ops,  // yourdriver.c driver functions
        .ctx = &s_yourdriver_ctx,         // driver context
        .pin_count = 16,                  // pin 0-15
    },
    ...
};

/*
 * bank/pin layout - this can be set arbitrary. Pins are mapped to driver pins
 * in sequential order
 */
static const hal_gpio_bank_t s_banks[] = {
    {
        .bank_id = 0,
        .name = "yourdriver-chain",
        .pin_count = 16,
    },
};

/* this layout */
const hal_gpio_layout_t g_hal_gpio_layout = {
    .drivers = s_drivers,
    .banks = s_banks,
    .driver_count = sizeof(s_drivers) / sizeof(s_drivers[0]),
    .bank_count = sizeof(s_banks) / sizeof(s_banks[0]),
};
```

3. add your driver to `target_sources()` in all supported `cmake/*.cmake` platform files

4. add your layout as `elseif(LAYOUT STREQUAL "yourlayout") to compile necessary drivers




# TODO
* runtime loglevel config + terminal debugging
* ESP32 port
* support interrupts
* onboard minimal documentation
* optimization
* proper pin bit-mapping (usbio pinmask, first/last gpio etc.)
* tests
* configure HAL dynamically + non-volatile config?
