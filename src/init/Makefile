
LDFLAGS = -static -lcryptsetup -ldevmapper -ludev -luuid -lgcrypt -lgpg-error -lrt
CFLAGS := -Wall -O3 -g
OBJS = config.o init.o mount.o crysco.o
BIN = init

debug: $(OBJS)
	$(CC) -o $(BIN) $^ $(LDFLAGS)

release: $(OBJS)
	$(CC) -o $(BIN) $^ $(LDFLAGS)
	strip $(BIN)

config.o: config.c
	m4 /home/alexanderfb/Downloads/hosts/test.h.m4 > config.h
	$(CC) $(CFLAGS) -o $@ -c config.c

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	#rm config.h
	rm init

