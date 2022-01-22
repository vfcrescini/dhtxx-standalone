// Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

// 2021-09-26 dhtxx.h

// Standalone DHT11/DHT22 sensor driver for Raspberry PI
// Data from:
//  https://cdn.datasheetspdf.com/pdf-down/D/H/T/DHT11-Aosong.pdf
//  https://cdn.datasheetspdf.com/pdf-down/D/H/T/DHT22-Aosong.pdf


#define DHTXX_DHT11_HUM_MIN  20.000
#define DHTXX_DHT11_HUM_MAX  90.000
#define DHTXX_DHT11_TMP_MIN   0.000
#define DHTXX_DHT11_TMP_MAX  50.000

#define DHTXX_DHT22_HUM_MIN   0.000
#define DHTXX_DHT22_HUM_MAX 100.000
#define DHTXX_DHT22_TMP_MIN -40.000
#define DHTXX_DHT22_TMP_MAX  80.000

#define DHTXX_ERRSTR_MAX    128


typedef enum
{
  DHTXX_TYPE_DHT11,
  DHTXX_TYPE_DHT22
}  dhtxx_type_t;


// get humidity and temperature reading from sensor of the given type

int dhtxx_get(int pin, dhtxx_type_t stype, float *ahum, float *atmp, char errstr[DHTXX_ERRSTR_MAX]);
