#
# Copyright (C) 2021 Linzhi Ltd.
#
# This work is licensed under the terms of the MIT License.
# A copy of the license can be found in the file COPYING.txt
#

.PHONY:		all clean spotless

CFLAGS = -Wall -Wextra -Wshadow -Wmissing-prototypes -Wmissing-declarations
OBJS = fand.o regmap.o mio.o ttc.o pwm.o pclk.o rpm.o
LDLIBS = -lmosquitto

all:		fand

fand:		$(OBJS)
		$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
		rm -f $(OBJS)

spotless:	clean
		rm -f fand
