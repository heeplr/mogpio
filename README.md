

moGPIO provides plug & play GPIO via USB

# Features

* currently supports up to 32 * 32 GPIO pins[^1] per device (more possible)
* access GPIOs using
  * standard libgpiod API (usbio)
  * reading/writing files on emulated mass storage device
  * interactive terminal via serial port
* terminal with autocompletion

[^1]: As the time of writing, the usbio protocol supports 5 * 32 GPIO max. Any more pins can be controlled only from the interactive terminal or via the mass storage mode. Also limiting to 32 banks is a memory footprint compromise and can be increased. (Please [open an issue](https://github.com/heeplr/mogpio/issues) if you need more).

# Install

Copy
* mogpio-intern-pico.uf2 to your pico.
* mogpio-intern-pico2.uf2 to your pico2

(s. [latest release](https://github.com/heeplr/mogpio/releases))



# Usage

There are three ways moGPIO exposes GPIOs:



## 1. USBIO Protocol

Just plug into your linux box and your moGPIO device will register as new
`/dev/gpiochipX` using the `usbio` driver.
You can use `gpiodetect`, `gpioinfo`, `gpioget`, `gpioset` commands like with any other /dev/gpiochipX.

Of yourse you can use the standard libgpiod API aswell.

NOTE: `gpiomon` command will fail, as interrupts are not supported in usbio, yet.



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
* ...



## 3. Mass Storage Class Device

moGPIO provides a small FAT16 partition with `PINS.TXT` and `CONFIG.TXT` files.

The files contents represent the state at time of read. Writes will affect the
state on actual write. (Some OS' cache agressively and won't write until
eject/unmount or explicit flush.)


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
GPIO hardware flavors.

Currently there are 3 flavors:

* intern (internal GPIOs of platform)
* sipo-piso (bit-bang serial shift registers)
* intern-sipo-piso (combination of the above)



# TODO
* support interrupts
* onboard minimal documentation
* optimization
* proper pin bit-mapping (usbio pinmask, first/last gpio etc.)
* tests
* configure HAL dynamically + non-volatile config?
