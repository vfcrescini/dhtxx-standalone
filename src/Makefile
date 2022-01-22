# Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

# 2021-10-10 Makefile


.PHONY = all clean distclean

CFLAGS = -Wall -O2


all: dhtxx libdhtxx.so modpython.so


clean:
	$(RM) *.o


distclean: clean
	$(RM) -r dhtxx libdhtxx.* modpython.so


dhtxx: main.o libdhtxx.so
	$(CC) $(CFLAGS) -o $(@) $(<) -L. -ldhtxx
	strip $(@)


libdhtxx.so: libdhtxx.so.0
	ln -fs $(<) $(@)


modpython.so: modpython.o libdhtxx.so
	$(CC) -Wl,-soname=dhtxx.so -shared $(CFLAGS) -o $(@) $(^) -L. -ldhtxx
	strip $(@)


libdhtxx.so.0: dhtxx.o gpio.o
	$(CC) -Wl,-soname=$(@) -shared $(CFLAGS) -o $(@) $(^)
	strip $(@)


libdhtxx.a: dhtxx.o gpio.o
	$(AR) rv $(@) $(^)


main.o: main.c dhtxx.h gpio.h
	$(CC) $(CFLAGS) -c -o $(@) $(<)


modpython.o: modpython.c dhtxx.h
	$(CC) $(CFLAGS) $(shell python3-config --includes) -fPIC -c -o $(@) $(<)


%.o: %.c %.h
	$(CC) $(CFLAGS) -fPIC -c -o $(@) $(<)
