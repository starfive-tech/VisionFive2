#!/bin/sh
#creat by samin@starfive
##########################JPU MULT Test#####################################
mult_output_path="output/multi"
mult_exec="./multi_instance_test"
mult_log_path="log/mult"
BITSTREAM_5M="5242880" #5M
BITSTREAM_8M="8388608" #8M
BITSTREAM_10M="10485760" #10M
input_file="outI422_qcif_inI422_12b.cfg,stream/enc_0_176_144_444_ref.jpg,outI422_qcif_inI422.cfg,stream/enc_0_176_144_420_ref_12b.jpg"
output_file="${mult_output_path}/outI422_qcif_inI422_12b.jpg,${mult_output_path}/enc_0_176_144_444_ref.yuv,${mult_output_path}/outI422_qcif_inI422.jpg,${mult_output_path}/enc_0_176_144_420_ref_12b.yuv"

if [ -d $mult_output_path ];then
	rm -rf $mult_output_path
fi
if [ -d $mult_log_path ];then
	rm -rf $mult_log_path
fi
mkdir -p $mult_output_path
mkdir -p $mult_log_path

instance_cmd="-e 0,1,0,1 --instance-num=4"  # 0:encoder 1:decoder
cfg_cmd="--cfg-dir=./cfg --yuv-dir=./yuv"
base_cmd="--stream-endian=0 --frame-endian=0 --bs-size=$BITSTREAM_5M --pixelj=0 --slice-height=0 --enable-slice-intr=0"

###########    go    #########
echo "$mult_exec $instance_cmd $cfg_cmd $base_cmd --input=$input_file --output=$output_file" |tee ${mult_log_path}/mult_instance_4.log
$mult_exec $instance_cmd $cfg_cmd $base_cmd --input=$input_file --output=$output_file 2>&1 | tee -a ${mult_log_path}/mult_instance_4.log
echo "JPU MULTI test end."


