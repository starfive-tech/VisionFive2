#!/bin/sh
module="vdec"
device="vdec"
para_off="/sys/module/cpufreq/parameters/off"

# invoke rmmod with all	arguments we got
/sbin/rmmod $module $* || exit 1

# Remove stale nodes
rm -f /dev/${device}
if [ -f $para_off ];then
	chmod 444 $para_off
fi

