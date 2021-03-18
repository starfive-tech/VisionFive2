#!/bin/sh
cmd=$1
PWD=`pwd`
FFMPEG=${PWD}/ffmpeg
FFMPEG_SRC=${FFMPEG}/src
FFMPEG_INC=${FFMPEG}/include
FFMPEG_LIB=${FFMPEG}/lib
FFMPEG_BIN=${FFMPEG}/bin
FFMPEG_SHARE=${FFMPEG}/share
FFMPEG_VERSION="2.1.8"
CPU_CORE=`cat /proc/cpuinfo |grep "processor"|wc -l`

if [ $1 == "clean" ];then   #clean
	rm -rf $FFMPEG_INC $FFMPEG_LIB $FFMPEG_BIN $FFMPEG_SHARE $FFMPEG_SRC/ffmpeg-${FFMPEG_VERSION}
else     					#build
	cd $FFMPEG_SRC
	tar -xvf ffmpeg-${FFMPEG_VERSION}.tar.gz
	cd ffmpeg-${FFMPEG_VERSION}
	./config_ffmpeg_riscv.sh
	make -j${CPU_CORE} && make install
fi
