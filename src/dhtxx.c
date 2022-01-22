// Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

// 2021-09-26 dhtxx.c

// Standalone DHT11/DHT22 sensor driver for Raspberry PI
// Data from:
//  https://cdn.datasheetspdf.com/pdf-down/D/H/T/DHT11-Aosong.pdf
//  https://cdn.datasheetspdf.com/pdf-down/D/H/T/DHT22-Aosong.pdf


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "gpio.h"
#include "dhtxx.h"


#define DHTXX_TICK_MAX 32000


static int dhtxx_set_priority_max();

static int dhtxx_set_priority_normal();

static int dhtxx_millisleep(uint32_t msec);

static int dhtxx_milliwait(uint32_t msec);


static int dhtxx_set_priority_max()
{
  struct sched_param sp;

  memset(&sp, 0x00, sizeof(sp));

  sp.sched_priority = sched_get_priority_max(SCHED_FIFO);

  if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
  {
    return 1;
  }

  return 0;
}


static int dhtxx_set_priority_normal()
{
  struct sched_param sp;

  memset(&sp, 0x00, sizeof(sp));

  sp.sched_priority = 0;

  if (sched_setscheduler(0, SCHED_OTHER, &sp) == -1)
  {
    return 1;
  }

  return 0;
}


static int dhtxx_millisleep(uint32_t msec)
{
  struct timespec st;

  memset(&st, 0x00, sizeof(st));

  // msec = 1 / 1000 secs
  // nsec  = 1 / 1000000000 secs = 1 / 1000000 usec

  st.tv_sec = msec / 1000;
  st.tv_nsec = (msec % 1000) * 1000000;

  while(1)
  {
    if (clock_nanosleep(CLOCK_MONOTONIC, 0, &st, &st) != 0 && errno != EINTR)
    {
      return 1;
    }

    break;
  }

  return 0;
}


static int dhtxx_milliwait(uint32_t msec)
{
  struct timeval stc;
  struct timeval std;
  struct timeval ste;

  memset(&stc, 0x00, sizeof(stc));
  memset(&std, 0x00, sizeof(std));
  memset(&ste, 0x00, sizeof(ste));

  // msec = 1 / 1000 secs
  // usec  = 1 / 1000000 secs = 1 / 1000 msec

  std.tv_sec = msec / 1000;
  std.tv_usec = (msec % 1000) * 1000;

  gettimeofday(&stc, NULL);

  timeradd(&stc, &std, &ste);

  while(1)
  {
    gettimeofday(&stc, NULL);

    if (timercmp(&stc, &ste, >))
    {
      break;
    }
  }

  return 0;
}


// get humidity and temperature reading from sensor of the given type

int dhtxx_get(int pin, dhtxx_type_t stype, float *ahum, float *atmp, char errstr[DHTXX_ERRSTR_MAX])
{ 
  volatile uint32_t *gpio = NULL;
  uint32_t tick_average_low = 0;
  uint32_t tick_count_low[41];
  uint32_t tick_count_high[41];
  uint8_t data[5];
  float ohum = 0.0;
  float otmp = 0.0;

  memset(tick_count_low, 0x00, sizeof(tick_count_low));
  memset(tick_count_high, 0x00, sizeof(tick_count_high));
  memset(data, 0x00, sizeof(data));

  if (pin <= 0 || pin > 40 || (stype != DHTXX_TYPE_DHT11 && stype != DHTXX_TYPE_DHT22) || ahum == NULL || atmp == NULL)
  {
    if (errstr != NULL)
    {
      snprintf(errstr, DHTXX_ERRSTR_MAX, "Invalid arguments");
    }
    return 1;
  }

  if (dhtxx_gpio_init(&gpio, errstr))
  {
    return 2;
  }

  dhtxx_set_priority_max();

  // step 1: set pin to output, assert for 500 msec, then de-assert for 20 msec

  dhtxx_gpio_set_output(gpio, pin);

  dhtxx_gpio_write(gpio, pin, 1);

  dhtxx_millisleep(500);

  dhtxx_gpio_write(gpio, pin, 0);

  dhtxx_milliwait(20);

  // step 2: set pin input, then sleep

  dhtxx_gpio_set_input(gpio, pin);

  // XXX - there's gotta be a better way
  // declare as volatile so it doesn't get optimised out despite being no-op

  for(volatile uint8_t i = 0; i < 50; i++);

  // step 3: wait for sensor to de-assert pin

  for(uint32_t i = 0; 1; i++)
  {
    uint8_t value = 0;

    if (i >= DHTXX_TICK_MAX)
    {
      if (errstr != NULL)
      {
        snprintf(errstr, DHTXX_ERRSTR_MAX, "Timeout while waiting for sensor to de-assert pin at initalisation");
      }
      return 3;
    }

    dhtxx_gpio_read(gpio, pin, &value);

    if (value == 0)
    {
      break;
    }
  }

  // step 4: record pulses

  for(uint8_t i = 0; i < 41; i++)
  {
    uint8_t value = 0;

    // count ticks until sensor asserts pin

    while(1)
    {
      dhtxx_gpio_read(gpio, pin, &value);

      if (value != 0)
      {
        break;
      }

      tick_count_low[i] = tick_count_low[i] + 1;

      if (tick_count_low[i] >= DHTXX_TICK_MAX)
      {
	if (errstr != NULL)
        {
          snprintf(errstr, DHTXX_ERRSTR_MAX, "Timeout while waiting for sensor to assert pin at bit %d", i);
	}
        return 4;
      }
    }

   // count ticks until sensor de-asserts pin

    while(1)
    {
      dhtxx_gpio_read(gpio, pin, &value);

      if (value == 0)
      {
        break;
      }

      tick_count_high[i] = tick_count_high[i] + 1;

      if (tick_count_high[i] >= DHTXX_TICK_MAX)
      {
	if (errstr != NULL)
        {
          snprintf(errstr, DHTXX_ERRSTR_MAX, "Timeout while waiting for sensor to de-assert pin at bit %d", i);
	}
        return 5;
      }
    }
  }

  // done. now drop back to normal priority

  dhtxx_set_priority_normal();

  // according to specs, low pulses are 50 msecs long
  // high pulses that are short (26 to 26 msecs) represent 0
  // high pulses that are long (70 msecs) represent 1

  // calculate average low signal ticks

  for(uint8_t i = 0; i < 40; i++)
  {
    tick_average_low = tick_average_low + tick_count_low[i + 1];
  }

  tick_average_low = tick_average_low / 40;

  // use low average tick count to decide whether high tick count is a 0 or a 1
  // skip first bit
  // remaining 40 bits = 5 bytes

  for(uint8_t i = 0; i < 5; i++)
  {
    for(uint8_t j = 0; j < 8; j++)
    {
      uint8_t bit = tick_count_high[(i * 8) + j + 1] >= tick_average_low;

      data[i] = (data[i] << 1) | bit;
    }
  }

  // validate checksum
  // fifth byte should be equal to the lowest eight bits of the sum of the first four bytes

  if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xff))
  {
    if (errstr != NULL)
    {
      snprintf(errstr, DHTXX_ERRSTR_MAX, "Checksum error");
    }
    return 6;
  }

  // calculate

  switch(stype)
  {
    case DHTXX_TYPE_DHT11:
      ohum = data[0] + (data[1] * 0.1);
      otmp = data[2] + (data[3] * 0.1);
      break;
    case DHTXX_TYPE_DHT22:
    default:
      ohum = ((data[0] << 8) + data[1]) / 10.0;
      otmp = (((data[2] & 0x7F) << 8) + data[3]) * ((data[2] & 0x80) ? -0.1 : 0.1);
      break;
  }

  // validate range

  switch(stype)
  {
    case DHTXX_TYPE_DHT11:
      if (ohum < DHTXX_DHT11_HUM_MIN || ohum > DHTXX_DHT11_HUM_MAX)
      {
	if (errstr != NULL)
        {
          snprintf(errstr, DHTXX_ERRSTR_MAX, "Humidity out of range: %8.3f", ohum);
	}
        return 7;
      }

      if (otmp < DHTXX_DHT11_TMP_MIN || otmp > DHTXX_DHT11_TMP_MAX)
      {
       	if (errstr != NULL)
        {
          snprintf(errstr, DHTXX_ERRSTR_MAX, "Temperature out of range: %8.3f", otmp);
	}
        return 8;
      }

      break;
    case DHTXX_TYPE_DHT22:
    default:
      if (ohum < DHTXX_DHT22_HUM_MIN || ohum > DHTXX_DHT22_HUM_MAX)
      {
	if (errstr != NULL)
        {
          snprintf(errstr, DHTXX_ERRSTR_MAX, "Humidity out of range: %8.3f", ohum);
	}
        return 7;
      }

      if (otmp < DHTXX_DHT22_TMP_MIN || otmp > DHTXX_DHT22_TMP_MAX)
      {
       	if (errstr != NULL)
        {
          snprintf(errstr, DHTXX_ERRSTR_MAX, "Temperature out of range: %8.3f", otmp);
	}
        return 8;
      }

      break;
  }

  *ahum = ohum;
  *atmp = otmp;

  return 0;
}
