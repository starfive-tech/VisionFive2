#!/bin/sh
dec_output_path=output
if [ ! -d $dec_output_path ];then
		mkdir -p $dec_output_path
fi
input_file=$1
file_base=$(basename $input_file)
dec_file_base=${dec_file_base%.*}
dec_output_file="$dec_output_path/$dec_file_base.yuv"

./ffmpeg_dec_test --output $dec_output_file --input $input_file
