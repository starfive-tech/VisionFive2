#!/bin/sh
if [ $# != 1 ];then
		echo "cmd: xxx.jpg"
		exit
fi
dec_file=$1
dec_exec='./jpg_dec_test'
BITSTREAM_8M="8388608" #8M
dec_cmd_base="--rotation=0 --mirror=0 --stream-endian=0 --frame-endian=0 --subsample=none --ordering=none --scaleH=0 --scaleV=0 --pixelj=0"
dec_file_base=${dec_file%.*}
dec_output_file="$dec_file_base.yuv"
BITSTREAM_SIZE=$BITSTREAM_8M
#echo "$dec_exec $dec_cmd_base --bs-size=$BITSTREAM_SIZE --output=$dec_output_file --input=$dec_file"
$dec_exec $dec_cmd_base --bs-size=$BITSTREAM_SIZE --output=$dec_output_file --input=$dec_file
