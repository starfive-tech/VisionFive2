#!/bin/sh
#creat by samin@starfive
##########################JPU Dec Test#####################################
dec_input_path='stream'
dec_output_path='output/dec'
dec_exec='./jpg_dec_test'
dec_log_path='log/dec'
BITSTREAM_8M="8388608" #8M
if [ -d $dec_output_path ];then
	rm -rf $dec_output_path
fi
if [ -d $dec_log_path ];then
	rm -rf $dec_log_path
fi
mkdir -p $dec_output_path
mkdir -p $dec_log_path
dec_cmd_base="--rotation=0 --mirror=0 --stream-endian=0 --frame-endian=0 --subsample=none --ordering=none --scaleH=0 --scaleV=0 --pixelj=0"
for dec_file in $dec_input_path/*; do
	dec_file_base=$(basename $dec_file)
	dec_file_base=${dec_file_base%.*}
	dec_output_file="$dec_output_path/$dec_file_base.yuv"
	echo [input]:$dec_file
	echo [output]:$dec_output_file

	echo "$dec_exec $dec_cmd_base --bs-size=$BITSTREAM_8M --output=$dec_output_file --input=$dec_file" | tee  ${dec_log_path}/${dec_file_base}.log
	$dec_exec $dec_cmd_base --bs-size=$BITSTREAM_8M --output=$dec_output_file --input=$dec_file 2>&1| tee -a ${dec_log_path}/${dec_file_base}.log
done

