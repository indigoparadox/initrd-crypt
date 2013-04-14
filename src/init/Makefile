
#HOSTNAME = `hostname`
HOSTNAME = himmel

CFLAGS = -Wall -O3
LDFLAGS = -static

all: host init

host:
	# TODO: Have autoconf do this, or something.
	./host.sh $(HOSTNAME)

init: host.o init.o datatypes.o mount.o crysco.o bstrlib/bstrlib.o
	$(CC) -o $@ $^ $(LDFLAGS)

release: host init
	strip init

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	#rm host.h
	#rm host.c
	rm init

