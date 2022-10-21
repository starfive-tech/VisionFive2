#!/bin/bash
source set_env.sh
CP="cp -rdvp"
OUTPUT=vdec_driver
DRI_OPT=$OUTPUT/driver
DRI_SRC=./vdi/linux/driver/

DEC_BIN=dec_test
DEC_MULT_BIN=multi_instance_dec_test
DEC_FFMPEG_BIN=ffmpeg_dec_test
STEAM=./stream
FIRMWARE=../firmware/chagall.bin
MEDIA=../media

if [ ! -d $DRI_OPT ];then
		mkdir -p $DRI_OPT
fi
make -f WaveDecode-ffmpeg.mak
$CP $DEC_FFMPEG_BIN $OUTPUT
make -f WaveDecode-ffmpeg.mak clean

make -f WaveDecDriver.mak
$CP $DRI_SRC/vdec.ko $DRI_OPT
make -f WaveDecDriver.mak clean

make -f WaveDecode.mak
$CP $DEC_BIN $OUTPUT
make -f WaveDecode.mak clean

make -f WaveDecodeMult.mak
$CP $DEC_MULT_BIN $OUTPUT
make -f WaveDecodeMult.mak clean


$CP $FIRMWARE $OUTPUT
$CP $STEAM $OUTPUT
$CP $MEDIA $OUTPUT
$CP script/* $OUTPUT
mkdir -p $OUTPUT/output
