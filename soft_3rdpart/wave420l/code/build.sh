#!/bin/bash
. ./set_env.sh
CP="cp -rdvp"
OUTPUT=venc_driver
DRI_OPT=$OUTPUT/driver
DRI_SRC=./vdi/linux/driver/

ENC_BIN=w4_enc_test
STEAM=./stream
YUV=./yuv
CFG=./cfg
FIRMWARE=../firmware/monet.bin

if [ ! -d $DRI_OPT ];then
	mkdir -p $DRI_OPT
fi

make -f  Wave420Driver.mak
$CP $DRI_SRC/venc.ko $DRI_SRC/load.sh  $DRI_SRC/unload.sh $DRI_OPT
make -f  Wave420Driver.mak clean

make -f  Wave420Enc.mak
$CP $ENC_BIN $OUTPUT
make -f  Wave420Enc.mak clean

$CP $FIRMWARE $OUTPUT
$CP $STEAM $OUTPUT
$CP $YUV $OUTPUT
$CP $CFG $OUTPUT
$CP script/* $OUTPUT
mkdir -p $OUTPUT/output
