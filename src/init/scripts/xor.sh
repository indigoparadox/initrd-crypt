#!/bin/bash

KEY_LEN=128

while [ "$1" ]; do
   case "$1" in
      "-k"|"--key")
         ACTION=generate
         ;;

      "-f"|"--file")
         shift
         SEED_FILE=$1
         ;;

      *)
         XOR_STRING=$1
         ;;
   esac
   shift
done

if [ -z "`which xxd`" ] || [ -z "$SEED_FILE" ]; then
   echo "Missing parameters."
   exit 1
fi

if [ "generate" = "$ACTION" ]; then
   KEY=()
   echo -n "" > $SEED_FILE
   for ITER in `seq 1 2 $KEY_LEN`; do
      echo -n "$(( ($RANDOM % 254) + 1 )) " >> $SEED_FILE
      echo -n "$(( ($RANDOM % 254) + 1 )), "
   done
else
   KEY=( $(cat $SEED_FILE) )
   XOR_STRING_BYTES=(
      `echo -n "$XOR_STRING" | xxd -p | sed 's/\(..\)/0x\1 /g'`
   )
   for ((i=0 ; $i < ${#XOR_STRING_BYTES[*]} ; i++)); do
      printf "0x%x, " $((${XOR_STRING_BYTES[$i]} ^ ${KEY[$i]}));
   done
fi

