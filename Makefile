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
