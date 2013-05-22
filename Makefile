
# Make release (stripped/no error messages) by default. Can be overridden
# (nominally with "debug") in order to produce a more forthcoming init.
RELEASE=release
IMGDIR = ./image
DESTDIR := $(shell pwd)/build
HOSTSDIR := $(shell pwd)/config
HOSTNAME := $(shell hostname)
# TODO Dynamically determine LD version.
#LDVER := $(shell ls /lib/ld-*.so | awk 'BEGIN {FS="-"} {print $2}' | cut -c -4)
LDVER := 2.15
ifeq ($(shell uname -m),x86_64)
	# 64-bit X86 Binaries
	IMGBINSTATIC := bin/busybox sbin/cryptsetup sbin/lvm.static sbin/mdadm 
	IMGBINDYNAMIC := usr/sbin/dropbear lib/ld-linux.so.2 lib/libc.so.6 lib/libcrypt.so.1 lib/libnss_files-$(LDVER).so lib/libnss_files.so.2 lib/libutil.so.1 lib/libz.so.1 lib/ld-$(LDVER).so lib/ld-linux-x86-64.so.2
else
	# 32-bit X86 Binaries
	IMGBINSTATIC := bin/busybox sbin/cryptsetup sbin/lvm.static sbin/mdadm
	IMGBINDYNAMIC := usr/sbin/dropbear lib/ld-linux.so.2 lib/libc.so.6 lib/libcrypt.so.1 lib/libnss_files-$(LDVER).so lib/libnss_files.so.2 lib/libutil.so.1 lib/libz.so.1
endif
# TODO: Add optional support for fbsplash.
# sbin/fbcondecor_helper s sbin/splash_util s
TORBIN := lib/librt.so.1 lib/libdl.so.2 lib/libpthread.so.0 lib/libm.so.6
DHCPBIN := lib/librt.so.1 lib/libpthread.so.0 sbin/dhcpcd

image: init
	@if [ ! -d $(DESTDIR) ]; then mkdir -p $(DESTDIR); fi
	@$(eval TMP = $(shell mktemp -d))
	@rsync -avz --exclude ".keep" $(IMGDIR)/ $(TMP)/initrd/
	# TODO: Make sure static files are static and dynamic files aren't.
	@$(foreach var,$(IMGBINSTATIC),cp -vL /$(var) $(TMP)/initrd/$(var);)
	@$(foreach var,$(IMGBINDYNAMIC),cp -vL /$(var) $(TMP)/initrd/$(var);)
	@cp src/init/init $(TMP)/initrd/init
	@if [ -n '`grep "^#define DHCP 1$$" src/init/config_extern.h`' ]; then \
		$(foreach var,$(DHCPBIN),cp -vL /$(var) $(TMP)/initrd/$(var);) \
	fi
	@if [ -n '`grep "^#define TOR 1$$" src/init/config_extern.h`' ]; then \
		$(foreach var,$(TORBIN),cp -vL /$(var) $(TMP)/initrd/$(var);) \
	fi
	# TODO: Use modular overridable lib paths.
	@if [ -n '`grep "^#define TOR 1$$" src/init/config_extern.h`' ] && \
		 [ ! -x 'tor/src/or/tor' ]; \
	then \
		cd tor; \
		make clean; \
		./configure \
			--disable-gcc-hardening \
			--enable-static-libevent \
			--enable-static-openssl \
			--enable-static-zlib \
			--disable-asciidoc \
			--with-libevent-dir=/usr/lib \
			--with-openssl-dir=/usr/lib \
			--with-zlib-dir=/usr/lib; \
		make; \
	fi
	@cp -v tor/src/or/tor $(TMP)/initrd/bin/tor
	@# Create/copy TOR config.
	@if [ -n '`grep "^#define TOR 1$$" src/init/config_extern.h`' ] && \
		 [ ! -f $(HOSTSDIR)/$(HOSTNAME)_tor_private ]; \
	then \
		openssl genrsa -out $(HOSTSDIR)/$(HOSTNAME)_tor_private 1024; \
		hex2base32 `openssl pkey -in $(HOSTSDIR)/$(HOSTNAME)_tor_private \
			-pubout -outform der | tail -c +23 | sha1sum | head -c 20` > \
			$(HOSTSDIR)/$(HOSTNAME)_tor_hostname; \
		echo `cat $(HOSTSDIR)/$(HOSTNAME)_tor_hostname`.onion > \
			$(HOSTSDIR)/$(HOSTNAME)_tor_hostname; \
	fi
	@$(eval SSHPORT = $(shell grep "^#define SSH_PORT" src/init/config_extern.h | awk '{print $$3}' | tr -d '"'))
	@if [ -n '`grep "^#define TOR 1$$" src/init/config_extern.h`' ]; then \
		cp -v $(HOSTSDIR)/$(HOSTNAME)_tor_private \
			$(TMP)/initrd/var/lib/tor/hidden_service/private_key; \
		cp -v $(HOSTSDIR)/$(HOSTNAME)_tor_hostname \
			$(TMP)/initrd/var/lib/tor/hidden_service/hostname; \
		sed -i 's/XPORTX/$(SSHPORT)/g' $(TMP)/initrd/etc/torrc; \
	fi
	@# Copy network files if the config calls for them.
	@$(eval NETDNS = $(shell grep "^#define NET_DNS" src/init/config_extern.h | awk '{print $$3}' | tr -d '"'))
	@if [ -n '`grep "^#define NET 1$$" src/init/config_extern.h`' ]; then \
		echo Copying network configuration...; \
		if [ ! -f $(HOSTSDIR)/$(HOSTNAME)_rsa ]; then \
			dropbearkey -f $(HOSTSDIR)/$(HOSTNAME)_rsa -t rsa -s 4096; \
		fi; \
		cp -v $(HOSTSDIR)/$(HOSTNAME)_rsa \
			$(TMP)/initrd/etc/dropbear/dropbear_rsa_host_key; \
		cp -v $(HOSTSDIR)/authorized_keys $(TMP)/initrd/root/.ssh; \
		echo "nameserver $(NETDNS)" > $(TMP)/initrd/etc/resolv.conf; \
	fi
	@# Remove old initrd.gz and place new one.
	@if [ -f $(DESTDIR)/initrd.gz ]; then rm $(DESTDIR)/initrd.gz; fi
	@echo Building init image...
	@cd $(TMP)/initrd && find . | cpio -ov --format=newc > $(DESTDIR)/initrd
	@gzip $(DESTDIR)/initrd
	@rm -rf $(TMP)

init:
	@if [ ! -f $(HOSTSDIR)/$(HOSTNAME).h.m4 ]; then \
		echo Missing host configuration for $(HOSTNAME). Aborting.; \
		exit 1; \
	fi
	@cd src/init && make clean && make RELEASE=$(RELEASE) \
		HOSTSDIR=$(HOSTSDIR) HOSTNAME=$(HOSTNAME)

.PHONY: init image

