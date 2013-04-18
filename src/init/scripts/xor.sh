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
   echo -n "" > $SEED_FILE
   for ITER in `seq 1 2 $KEY_LEN`; do
      MY_RAND=`shuf -i 1-255 -n 1`
      echo -n "$MY_RAND " >> $SEED_FILE
      echo -n "$MY_RAND, "
   done
else
   KEY=( $(cat $SEED_FILE) )
   XOR_STRING_BYTES=()
   while [ -n "$XOR_STRING" ]; do
      CHAR=${XOR_STRING:0:1}
      XOR_STRING_BYTES+=(`printf "%d" \'$CHAR`)
      XOR_STRING=${XOR_STRING:1}
   done

   for ((i=0 ; $i < ${#XOR_STRING_BYTES[*]} ; i++)); do
      echo -n "$((${XOR_STRING_BYTES[$i]} ^ ${KEY[$i]})), "
   done
   printf "0"
fi

