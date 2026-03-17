#!/bin/sh
# test for switchmode scripts
while :
do
    cc_tools switchmode 2
    sleep 5
    cc_tools switchmode 0
    sleep 5
    cc_tools warnrec
    sleep 5
done
