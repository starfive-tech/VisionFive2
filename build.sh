#!/bin/bash
##################################################################
##                                                              ##
##      SPDX-License-Identifier: GPL-2.0-or-later               ##
##                                                              ##
##	Copyright (C) 2018-2022   Starfive Technology           ##  
##                                                              ##
##	Author:	jason zhou  <jason.zhou@starfivetech.com>       ##
##                                                              ##
##################################################################

COLOR_NORMAL="\033[0m"
COLOR_GREEN="\033[1;32m"
COLOR_YELLOW="\033[1;33m"
COLOR_RED="\033[1;31m"
COLOR_GREY="\033[1;30m"

printf ${COLOR_GREEN}
printf "JH7110 Build Script"
echo $0 $1

#default
cpu_numb_input="1"

if [ $1 ]
then
	#cmdline para input cpu processor number
	echo $1
	cpu_numb_input=$1
else
	#interactive input cpu number
	num=`cat /proc/cpuinfo | grep "processor" | wc -l`
	printf "please input the cpu number to perform concurrent build, choose 1 ~ $num:\n"

	read cpu_numb_input
	if (( $cpu_numb_input > $num ))
	then
		echo "input > $num  error, exit"
		exit 1
	fi

	if (( $cpu_numb_input  <  1 ))
	then
		echo "input < 1 error, exit"
		exit 1
	fi

	echo ""
	echo "make -j$cpu_numb_input"
fi

printf ${COLOR_NORMAL}

# clean all including rootfs
#rm -rf work/

# clean kernel built
make clean

tag=`git describe`
echo "make -j$cpu_numb_input > build.$tag.log"
make -j$cpu_numb_input > build.$tag.log

echo "./build_soft_3rdpart.sh >>  build.$tag.log"
./build_soft_3rdpart.sh >>  build.$tag.log

echo "rm initramfs, than rebuild"
rm -rf work/initramfs.cpio.gz

echo "make -j$cpu_numb_input >> build.$tag.log"
make -j$cpu_numb_input >> build.$tag.log


echo ""
echo "Rootfs & Kernel version string:"
strings work/image.fit | grep "JH7110_51"
echo ""
echo "u-boot version string:"
strings work/u-boot/u-boot.bin | grep "JH7110_51"
