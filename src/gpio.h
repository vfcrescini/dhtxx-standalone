// Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

// 2021-09-26 gpio.h

// Very simply GPIO library for RPi
// Uses /proc/device-tree/soc/ranges to get base address; info from:
//   https://github.com/devicetree-org/devicetree-specification/releases/download/v0.3/devicetree-specification-v0.3.pdf


# include <stdint.h>


#define DHTXX_GPIO_DTREE_PATH  "/proc/device-tree/soc/ranges"
#define DHTXX_GPIO_BASE_OFFSET 0x200000
#define DHTXX_GPIO_LEN         4096
#define DHTXX_GPIO_ERRSTR_MAX  128


// create a memory map of the GPIOs, and give back a pointer to it
// returns zero on success, non-zero otherwise, with errstr set

int dhtxx_gpio_init(volatile uint32_t **gpio, char errstr[DHTXX_GPIO_ERRSTR_MAX]);


// set GPIO pin for input

int dhtxx_gpio_set_input(volatile uint32_t *gpio, int num);


// set GPIO pin for output

int dhtxx_gpio_set_output(volatile uint32_t *gpio, int num);


// sets value to the current state of the given input GPIO pin
// value is set to non-zero if pin is high, zero otherwise
// returns zero on success, non-zero otherwise

int dhtxx_gpio_read(volatile uint32_t *gpio, int num, uint8_t *value);


// asserts/deasserts given output GPIO pin
// assert, if value != 0, deassert if value == 0
// returns zero on success, non-zero otherwise

int dhtxx_gpio_write(volatile uint32_t *gpio, int num, uint8_t value);
