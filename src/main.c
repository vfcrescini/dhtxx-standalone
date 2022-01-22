// Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

// 2021-09-26 main.c

// Standalone DHT11/DHT22 sensor driver for Raspberry PI


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "gpio.h"
#include "dhtxx.h"


int main(int argc, char *argv[])
{
  int pin = 0;
  int curr_opt = 0;
  char stype = DHTXX_TYPE_DHT22;
  float ohum = 0.0;
  float otmp = 0.0;
  char errstr[DHTXX_ERRSTR_MAX];

  opterr = 0;
  optind = 0;

  while((curr_opt = getopt(argc, argv, "hd")) != -1)
  {
    switch(curr_opt)
    {
      case 'h':
        fprintf(stderr, "Usage: dhtxx [-h|-d] <pin>\n");
        return 0;
        break;
      case 'd':
        stype = DHTXX_TYPE_DHT11;
        break;
      default:
        fprintf(stderr, "Invalid option: -%c\n", curr_opt);
        return 1;
    }
  }

  if (argc <= optind)
  {
    fprintf(stderr, "Need to specify GPIO pin\n");
    return 2;
  }

  pin = atoi(argv[optind]);

  if (pin <= 0 || pin > 40)
  {
    fprintf(stderr, "Invalid GPIO pin: %d\n", pin);
    return 3;
  }

  memset(errstr, 0x00, DHTXX_ERRSTR_MAX);

  if (dhtxx_get(pin, stype, &ohum, &otmp, errstr))
  {
    fprintf(stderr, "Failed to get data: %s\n", errstr);
    return 4;
  }

  // dump result

  fprintf(stdout, "%8.3f %8.3f\n", ohum, otmp);

  return 0;
}
