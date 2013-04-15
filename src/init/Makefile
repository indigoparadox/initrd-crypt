
CFLAGS = -g -Wall -O3
LDFLAGS = -static

all: init

init: host.o init.o datatypes.o mount.o crysco.o
	$(CC) -o $@ $^ $(LDFLAGS)

release: init
	strip init

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	rm host.h
	rm host.c
	rm init

