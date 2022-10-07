#!/bin/bash
jpu_path=`pwd`
#output="../../work/buildroot_initramfs_sysroot"
output="./jpu_driver"
jpu_driver=${jpu_path}/jdi/linux/driver
jpu_env=${jpu_driver}/build_env_setup_riscv.sh
obj_jpu=$output
obj_jpu_driver=${obj_jpu}/driver

#set env
#source $jpu_env

#mkdir
if [ ! -d $obj_jpu ];then
	mkdir -p $obj_jpu
fi
if [ ! -d $obj_jpu_driver ];then
	mkdir -p $obj_jpu_driver
fi

#cp test files
cp -rdvp ${jpu_path}/script/* ${obj_jpu}/
cp -rdvp ${jpu_path}/cfg $obj_jpu
cp -rdvp ${jpu_path}/yuv $obj_jpu
cp -rdvp ${jpu_path}/stream $obj_jpu

#make linux driver
cd $jpu_driver
source build_env_setup_riscv.sh
make clean
make

#make multi_instance
cd $jpu_path
make clean
make MAKECMDGOALS=multi USE_FFMPEG=no
cp ${jpu_path}/multi_instance_test $obj_jpu

#make decoder/encoder
make clean
make USE_FFMPEG=no
cp ${jpu_path}/jpg_enc_test ${jpu_path}/jpg_dec_test $obj_jpu

#cp driver
cp ${jpu_driver}/jpu.ko $obj_jpu_driver
cp -rdvp jpu_driver ../../work/buildroot_initramfs_sysroot/root/


