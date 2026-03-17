#!/bin/sh
# test for take photos scripts
while :
do
        cc_tools warnrec 
        rd=$(($RANDOM%10))
        sleep 10
        sleep $rd
        echo "slepp $rd sec"

done
~