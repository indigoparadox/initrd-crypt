
IMGDIR = ./image
DESTDIR := $(shell pwd)/build
LDVER = 2.15
# TODO: Differentiate between 32-bit and 64-bit ld libs.
IMGBINSTATIC := bin/busybox sbin/cryptsetup sbin/lvm.static sbin/mdadm 
IMGBINDYNAMIC := usr/sbin/dropbear lib/ld-linux.so.2 lib/libc.so.6 lib/libcrypt.so.1 lib/libnss_files-$(LDVER).so lib/libnss_files.so.2 lib/libutil.so.1 lib/libz.so.1 lib/ld-$(LDVER).so lib/ld-linux-x86-64.so.2

image: init
	$(eval TMP = $(shell mktemp -d))
	rsync -avz $(IMGDIR)/ $(TMP)/initrd/
	# TODO: Make sure static files are static and dynamic files aren't.
	$(foreach var,$(IMGBINSTATIC),cp -L /$(var) $(TMP)/initrd/$(var);)
	$(foreach var,$(IMGBINDYNAMIC),cp -L /$(var) $(TMP)/initrd/$(var);)
	cp src/init/init $(TMP)/initrd/init
	cd $(TMP)/initrd && find . | cpio -ov --format=newc > $(DESTDIR)/initrd
	gzip $(DESTDIR)/initrd
	rm -rf $(TMP)

init:
	# TODO: Make release/debug distinction here.
	cd src/init && make clean && make

.PHONY: init image

