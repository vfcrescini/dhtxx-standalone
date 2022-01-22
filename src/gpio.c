// Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

// 2021-09-26 gpio.c

// Very simply GPIO library for RPi
// Uses /proc/device-tree/soc/ranges to get base address; info from:
//   https://github.com/devicetree-org/devicetree-specification/releases/download/v0.3/devicetree-specification-v0.3.pdf


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

# include "gpio.h"


// get base address from DHTXX_GPIO_DTREE_PATH

static int dhtxx_gpio_base(uint32_t *base, char errstr[DHTXX_GPIO_ERRSTR_MAX]);


// get base address from DHTXX_GPIO_DTREE_PATH

static int dhtxx_gpio_base(uint32_t *base, char errstr[DHTXX_GPIO_ERRSTR_MAX])
{
  int fd = 0;
  int rv = 0;
  uint8_t buf[4];

  memset(buf, 0x00, 4);

  if (base == NULL)
  {
    if (errstr != NULL)
    {
      snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Invalid argument");
    }
    return 1;
  }

  if ((fd = open(DHTXX_GPIO_DTREE_PATH, O_RDONLY)) == -1)
  {
    if (errstr != NULL)
    {
      int e = errno;

      snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Failed to call open(%s): [%d]: %s", DHTXX_GPIO_DTREE_PATH, e, strerror(e));
    }

    return 2;
  }

  // first four bytes are for the child bus address, which we skip over

  if (lseek(fd, 0x04, SEEK_SET) == -1)
  {
    if (errstr != NULL)
    {
      int e = errno;

      snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Failed to call lseek(): [%d]: %s", e, strerror(e));
    }
    return 3;
  }

  // second four bytes are for the parent bus address, which is what we want

  rv = read(fd, &buf, 4);

  if (rv != 4)
  {
    if (rv == -1)
    {
      if (errstr != NULL)
      {
        int e = errno;

        snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Failed to call read(): [%d]: %s", e, strerror(e));
      }
      return 4;
    }
    if (errstr != NULL)
    {
      snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Failed to call read()");
    }
    return 5;
  }

  close(fd);

  // because some machines are little endian

  *base = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;

  return 0;

}


// create a memory map of the GPIOs, and give back a pointer to it
// returns zero on success, non-zero otherwise, with errstr set

int dhtxx_gpio_init(volatile uint32_t **gpio, char errstr[DHTXX_GPIO_ERRSTR_MAX])
{
  int fd;
  uint32_t base;
  volatile uint32_t *tmp = NULL;

  if (gpio == NULL)
  {
    if (errstr != NULL)
    {
      snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Invalid argument");
    }
    return 1;
  }

  if (dhtxx_gpio_base(&base, errstr))
  {
    return 2;
  }

  fd = open("/dev/mem", O_RDWR | O_SYNC);

  if (fd < 0)
  {
    if (errstr != NULL)
    {
      int e = errno;
      snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Failed call to open(/dev/mem): [%d] %s", e, strerror(e));
    }
    return 3;
  }

  tmp = (uint32_t *) mmap(NULL, DHTXX_GPIO_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base + DHTXX_GPIO_BASE_OFFSET);

  if (tmp == MAP_FAILED)
  {
    if (errstr != NULL)
    {
      int e = errno;
      snprintf(errstr, DHTXX_GPIO_ERRSTR_MAX, "Failed call to mmap(/dev/mem): [%d] %s", e, strerror(e));
    }

    close(fd);

    return 4;
  }

  close(fd);

  *gpio = tmp;

  return 0;
}


// set GPIO pin for input

int dhtxx_gpio_set_input(volatile uint32_t *gpio, int num)
{
  volatile uint32_t *tmp;

  if (gpio == NULL || num < 0)
  {
    return 1;
  }

  tmp = gpio + (num / 10);
  *tmp = *tmp & ~(7 << ((num % 10) * 3));

  return 0;
}


// set GPIO pin for output

int dhtxx_gpio_set_output(volatile uint32_t *gpio, int num)
{
  volatile uint32_t *tmp;

  if (gpio == NULL || num < 0)
  {
    return 1;
  }

  tmp = gpio + (num / 10);
  *tmp = *tmp & ~(7 << ((num % 10) * 3));
  *tmp = *tmp | (1 << ((num % 10) * 3));

  return 0;
}


// sets value to the current state of the given input GPIO pin
// value is set to non-zero if pin is high, zero otherwise
// returns zero on success, non-zero otherwise

int dhtxx_gpio_read(volatile uint32_t *gpio, int num, uint8_t *value)
{
  if (gpio == NULL || num < 0 || value == NULL)
  {
    return 1;
  }

  *value = (*(gpio + 13) & (1 << num)) != 0;

  return 0;
}


// asserts/deasserts given output GPIO pin
// assert if value != 0, deassert if value == 0
// returns zero on success, non-zero otherwise

int dhtxx_gpio_write(volatile uint32_t *gpio, int num, uint8_t value)
{
  if (gpio == NULL || num < 0)
  {
    return 1;
  }

  *(gpio + (value ? 7 : 10)) = 1 << num;

  return 0;
}
