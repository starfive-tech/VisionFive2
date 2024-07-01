#!/bin/bash
##################################################################
##                                                              ##
##      SPDX-License-Identifier: GPL-2.0-or-later               ##
##                                                              ##
##      Copyright (C) 2018-2022   Starfive Technology           ##
##                                                              ##
##  Author: Andy Hu <andy.hu@starfivetech.com>                  ##
##  Date:   2022-07-27                                          ##
##  Description: This script used to generate img file          ##
##    which could be burned to tf card through dd or            ##
##    rpi-imager or balenaEtcher tool.                          ##
##    Run it after the usdk initramfs and rootfs had been built ##
##                                                              ##
##################################################################
COLOR_NORMAL="\033[0m"
COLOR_GREEN="\033[1;32m"
COLOR_YELLOW="\033[1;33m"
COLOR_RED="\033[1;31m"
COLOR_GREY="\033[1;30m"
HWBOARD=$1

TOPDIR=`dirname $0`
BUILD_DIR=$TOPDIR/work
INPUT_DIR=$TOPDIR
OUTPUT_DIR=$TOPDIR/work
if [ $HWBOARD == "visionfive2" ]; then
	GENIMAGE_CFG=$TOPDIR/conf/genimage-vf2.cfg
elif [ $HWBOARD == "vf2-amp" ]; then
	GENIMAGE_CFG=$TOPDIR/conf/genimage-vf2-amp.cfg
else
	GENIMAGE_CFG=$TOPDIR/conf/genimage.cfg
fi
GENIMAGE=$TOPDIR/work/buildroot_initramfs/host/bin/genimage

if [ ! -f $GENIMAGE ]; then
    printf $COLOR_RED
    echo "Error: $GENIMAGE not found. need building the usdk first"
    printf $COLOR_NORMAL
    exit 1
fi

GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"

# Pass an empty rootpath. genimage makes a full copy of the given rootpath to
# ${GENIMAGE_TMP}/root so passing TARGET_DIR would be a waste of time and disk
# space. We don't rely on genimage to build the rootfs image, just to insert a
# pre-built one in the disk image.

trap 'rm -rf "${ROOTPATH_TMP}"' EXIT
ROOTPATH_TMP="$(mktemp -d)"

rm -rf "${GENIMAGE_TMP}"

$GENIMAGE \
	--rootpath "${ROOTPATH_TMP}"     \
	--tmppath "${GENIMAGE_TMP}"    \
	--inputpath "${INPUT_DIR}"  \
	--outputpath "${OUTPUT_DIR}" \
	--config "${GENIMAGE_CFG}"
