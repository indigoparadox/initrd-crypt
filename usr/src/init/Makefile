
#HOSTNAME = `hostname`
HOSTNAME = himmel

CFLAGS = -Wall -O3

all: host init

host:
	./host.sh $(HOSTNAME)

init: host.o init.o datatypes.o mount.o crysco.o bstrlib/bstrlib.o
	$(CC) $(CFLAGS) -static -o $@ $^
	strip $@

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	rm host.h
	rm host.c
	rm init

