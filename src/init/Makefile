
LIBS = -static -lcryptsetup -ldevmapper -ludev -luuid -lgcrypt -lgpg-error -lrt -lblkid
ifeq ($(RELEASE),release)
	# Release
	# Things like CONSOLE and NET can be defined in individual host configs.
	CFLAGS := -Wall -O3
else
	# Debug
	CFLAGS := -DDEBUG -DERRORS -Wall -O3 -g
endif
OBJS = config.o init.o mount.o crysco.o console.o network.o util.o
BIN = init
ALLDEPS := init
HOSTSDIR := $(shell pwd)/../../config
HOSTNAME := test
ifeq ($(RELEASE),release)
	ALLDEPS += strip
endif

all: $(ALLDEPS)

init: $(OBJS)
	# What a useless error message. This is such a crude way to block it out. Is
	# there a better way? None of the -W flags seem to cover it.
	$(CC) -o $(BIN) $^ $(LIBS) 2>&1 | grep -v "warning: Using .* in statically linked applications" | grep -v "libudev.a" | cat

strip:
	strip $(BIN)

config.o: config.c
	cat config_base.template.m4 \
		config.template.m4 \
		$(HOSTSDIR)/$(HOSTNAME).h.m4 | m4 > config_base.h
	cat config_extern.template.m4 \
		config.template.m4 \
		$(HOSTSDIR)/$(HOSTNAME).h.m4 | m4 > config_extern.h
	$(CC) $(CFLAGS) -o $@ -c config.c
	rm srand

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	find -name "config_*.h" -exec rm {} \;
	find -name "init" -type f -exec rm {} \;

