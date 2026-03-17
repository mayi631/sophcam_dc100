#!/bin/sh
echo 453 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio453/direction

while [ 1 ];
do
echo 1 > /sys/class/gpio/gpio453/value
sleep 3
echo 0 > /sys/class/gpio/gpio453/value
sleep 3
done