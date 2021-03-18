module="jpu"
device="jpu"

# invoke rmmod with all	arguments we got
/system/bin/rmmod $module $* || exit 1

# Remove stale nodes
rm -f /dev/${device} 




 
