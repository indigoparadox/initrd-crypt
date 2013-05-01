
# Make release (stripped/no error messages) by default. Can be overridden
# (nominally with "debug") in order to produce a more forthcoming init.
RELEASE=release
IMGDIR = ./image
DESTDIR := $(shell pwd)/build
HOSTSDIR := $(shell pwd)/examples
# TODO: Dynamically determine hostname?
HOSTNAME := test
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
	IMGBINDYNAMIC := sbin/dropbear lib/ld-linux.so.2 lib/libc.so.6 lib/libcrypt.so.1 lib/libnss_files-$(LDVER).so lib/libnss_files.so.2 lib/libutil.so.1 lib/libz.so.1
endif
# TODO: Add optional support for fbsplash.
# sbin/fbcondecor_helper s sbin/splash_util s

image: init
	if [ ! -d $(DESTDIR) ]; then mkdir -p $(DESTDIR); fi
	$(eval TMP = $(shell mktemp -d))
	rsync -avz --exclude ".keep" $(IMGDIR)/ $(TMP)/initrd/
	# TODO: Make sure static files are static and dynamic files aren't.
	$(foreach var,$(IMGBINSTATIC),cp -L /$(var) $(TMP)/initrd/$(var);)
	$(foreach var,$(IMGBINDYNAMIC),cp -L /$(var) $(TMP)/initrd/$(var);)
	cp src/init/init $(TMP)/initrd/init
	if [ ! -f $(HOSTSDIR)/$(HOSTNAME)_rsa ]; then \
		dropbearkey -f $(HOSTSDIR)/$(HOSTNAME)_rsa -t rsa -s 4096; fi
	cp $(HOSTSDIR)/$(HOSTNAME)_rsa \
		$(TMP)/initrd/etc/dropbear/dropbear_rsa_host_key
	if [ -f $(DESTDIR)/initrd.gz ]; then rm $(DESTDIR)/initrd.gz; fi
	cd $(TMP)/initrd && find . | cpio -ov --format=newc > $(DESTDIR)/initrd
	gzip $(DESTDIR)/initrd
	rm -rf $(TMP)

init:
	cd src/init && make clean && make RELEASE=$(RELEASE) \
		HOSTSDIR=$(HOSTSDIR) HOSTNAME=$(HOSTNAME)

.PHONY: init image

