
LDFLAGS = -static -lcryptsetup -ldevmapper -ludev -luuid -lgcrypt -lgpg-error -lrt
CFLAGS := -DERRORS -DCONSOLE -Wall -O3 -g
OBJS = config.o init.o mount.o crysco.o
BIN = init

debug: $(OBJS)
	$(CC) -o $(BIN) $^ $(LDFLAGS)

release: $(OBJS)
	$(CC) -o $(BIN) $^ $(LDFLAGS)
	strip $(BIN)

config.o: config.c
	cat config_base.template.m4 \
		config.template.m4 \
		/home/alexanderfb/Downloads/hosts/test.h.m4 | m4 > config_base.h
	cat config_extern.template.m4 \
		config.template.m4 \
		/home/alexanderfb/Downloads/hosts/test.h.m4 | m4 > config_extern.h
	$(CC) $(CFLAGS) -o $@ -c config.c
	rm srand

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	#rm config_base.h
	#rm config_extern.h
	#rm init

