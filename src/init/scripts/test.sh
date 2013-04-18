#!/bin/sh

VMTEST_DIR=../../build

qemu-system-x86_64 -m 256 -s -boot a \
   -hda $VMTEST_DIR/hda.img \
   -hdb $VMTEST_DIR/hdb.img \
   -kernel $VMTEST_DIR/vmlinuz \
   -initrd $VMTEST_DIR/build/initrd.gz

