
CFLAGS = -Wall -O3
LDFLAGS = -static -lcryptsetup -ldevmapper -ludev -luuid -lgcrypt -lgpg-error -lrt
OBJS = host.o init.o mount.o crysco.o scrambles.o genstrings.o

all: init

init: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

release: init
	strip init

genstrings.o:
	m4 genstrings.c.m4 > genstrings.c
	$(CC) $(CFLAGS) -o $@ -c genstrings.c

host.o:
	m4 /home/alexanderfb/Downloads/hosts/test.c.m4 > host.c
	$(CC) $(CFLAGS) -o $@ -c host.c

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	rm host.c
	rm genstrings.c
	rm init

