#!/bin/bash

KEY_LEN=64

for ITER in `seq 1 2 $KEY_LEN`; do
   echo -n $(( $RANDOM % 255 ))
   if [ $(( $KEY_LEN - 2 )) -ge $ITER ]; then
      echo -n ,
   fi
done

