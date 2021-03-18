module="jpu"
device="jpu"
mode="664"

# invoke insmod	with all arguments we got
# and use a pathname, as newer modutils	don't look in .	by default
/system/bin/insmod /system/lib/modules/$module.ko $*	|| exit	1
echo "module $module inserted"

#remove old nod
rm -f /dev/${device}

#read the major asigned at loading time
major=`cat /proc/devices | busybox awk "\\$2==\"$module\" {print \\$1}"`
echo "$module major = $major"

# Remove stale nodes and replace them, then give gid and perms
# Usually the script is	shorter, it's simple that has several devices in it.

busybox mknod /dev/${device} c $major 0
echo "node $device created"

chmod $mode  /dev/${device}
echo "set node access to $mode"
