


"USB GPIOs with moGPIO is the USBee's knees!'"

Firmware to provide hassle free GPIOs via USB at OS level with zero programming

This uses [TinyUSB](https://tinyusb.org/)) to provide
1. a mass storage device
2. a usbio vendor class device

when plugged into a USB-Host (tested on Android, Windows, linux & OS X).

It provides two separate ways to interact with GPIOs:

## MCD
The mass storage class device emulates a FAT12 mass storage

# Build
```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
$ cp mogpio.uf2 /mnt/pico/
```

# debug build (log output to uart)
```shell
$ cmake -D CMAKE_BUILD_TYPE=DEBUG ..
```


# TODO
* proper logging
* optimization
* proper pin bit-mapping (usbio pinmask, first/last gpio etc.)
* tests
