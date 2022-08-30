#!/bin/bash


PARENT_FILE_PATH=$(cd "$(dirname "$0")/.."; pwd)
FILE_PATH=$(cd "$(dirname "$0")"; pwd)


sudo insmod ./install/km/Mantis.ko
sudo chmod 666 /dev/sivmantis0
ls -alt /dev/sivmantis0

