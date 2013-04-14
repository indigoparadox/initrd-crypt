#!/bin/sh

if [ -z "$1" ]; then
   MY_HOSTNAME="`hostname`"
else
   MY_HOSTNAME="$1"
fi

# Generate the "encryption" string.
KEY_LEN=128
WORDS="`dd if=/dev/urandom bs=$KEY_LEN count=1`"
while [ -n "$WORDS" ]; do
   CHAR=${WORDS:0:1}
   CHAR_INT=$((`printf "%d" \'$CHAR` % 255))
   KEY_ARRAY="$KEY_ARRAY $CHAR_INT,"
   WORDS=${WORDS:1}
done
KEY_ARRAY="${KEY_ARRAY:0:-1}"

cp hosts/$MY_HOSTNAME.h host.h
cp hosts/$MY_HOSTNAME.c host.c
sed -i "s/::SKEY::/$KEY_ARRAY /g" host.c

