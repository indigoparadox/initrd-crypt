
CFLAGS = -Wall -O3
LDFLAGS = -static -lcryptsetup -ldevmapper -ludev -luuid -lgcrypt -lgpg-error -lrt
OBJS = config.o init.o mount.o crysco.o

all: init

init: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

release: init
	strip init

config.o:
	m4 /home/alexanderfb/Downloads/hosts/test.h.m4 > config.h
	$(CC) $(CFLAGS) -o $@ -c config.c

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	rm config.h
	rm init

