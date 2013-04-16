
CFLAGS = -Wall -O3
LDFLAGS = -static -lcryptsetup -ldevmapper -ludev -luuid -lgcrypt -lgpg-error -lrt

all: init

init: host.o init.o mount.o crysco.o scrambles.o genstrings.o
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

