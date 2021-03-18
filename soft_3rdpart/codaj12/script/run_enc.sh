#!/bin/sh
#creat by samin@starfive
##########################JPU Enc Test#####################################
#enc_input_path='stream'
enc_cfg_path="cfg"
enc_exec="./jpg_enc_test"
enc_log_path="log/enc"
enc_output_path="output/enc"

if [ -d $enc_output_path ];then
	rm -rf $enc_output_path
fi
if [ -d $enc_log_path ];then
	rm -rf $enc_log_path
fi
mkdir -p $enc_output_path
mkdir -p $enc_log_path
enc_cmd_in_out="--cfg-dir=./cfg --yuv-dir=./yuv"
enc_cmd_mode="--stream-endian=0 --frame-endian=0 --pixelj=0 --slice-height=0 --enable-slice-intr=0"
enc_cmd_size="--bs-size=8388608 --rotation=0 --mirror=0"

for enc_file in $enc_cfg_path/*; do
	enc_file_cfg=$(basename $enc_file)
	res=`echo $enc_file_cfg | grep "out"`
	if [ "$res" != "" ];then
		res=`echo $enc_file_cfg | grep "12b"`
		if [ "$res" != "" ];then
			bit12="--12bit"
		else
			bit12=""
		fi
		res=`echo $enc_file_cfg | grep "tiled"`
		if [ "$res" != "" ];then
			tiledMode="--enable-tiledMode=1"
		else
			tiledMode="--enable-tiledMode=0"
		fi
		################## go ####################
		enc_file_base=${enc_file_cfg%.*}
		enc_output_file="$enc_output_path/$enc_file_base.jpg"
		echo [input]:$enc_file_cfg
		echo [output]:$enc_output_file
		echo "$enc_exec --output=$enc_output_file $enc_cmd_in_out $bit12 $enc_cmd_mode $tiledMode $enc_cmd_size --input=$enc_file_cfg" | tee ${enc_log_path}/${enc_file_base}.log
		$enc_exec --output=$enc_output_file $enc_cmd_in_out $bit12 $enc_cmd_mode $tiledMode $enc_cmd_size --input=$enc_file_cfg 2>&1 |tee -a ${enc_log_path}/${enc_file_base}.log
	else
		continue
	fi
done
echo "JPU Enc test OK."
