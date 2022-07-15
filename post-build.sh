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
echo ""
echo "post build copy images to release folder!"
folder=~/release/`git describe`-`date -I`
echo "create folder:$folder"
printf ${COLOR_NORMAL}

mkdir -p $folder
cp work/image.fit $folder/
cp work/evb_fw_payload.img $folder/
cp work/u-boot-spl.bin.normal.out $folder/
cp work/initramfs.cpio.gz $folder/
cp work/linux/arch/riscv/boot/Image.gz $folder/
cp -rf work/linux/arch/riscv/boot/dts/starfive $folder/
tree $folder
echo ""
