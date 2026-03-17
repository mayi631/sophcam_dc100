
#!/bin/sh
# test for start/stop rec scripts
cc_tools stoprec
while :
do
    cc_tools startrec
    sleep 2
    rd=$(($RANDOM%10))
    sleep $rd
    echo "slepp $rd sec"
    cc_tools stoprec
    sleep 2
done