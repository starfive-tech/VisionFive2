#!/bin/sh
cmd=$1
PWD=`pwd`
PLATFORM=riscvlinux
FFMPEG_OUT=${PWD}/ffmpeg
FFMPEG_SRC=${PWD}/../ffmpeg
CONFIG_SCRIPT=config_ffmpeg.sh
CONFIG_SCRIPT_PATH=${FFMPEG_SRC}/${CONFIG_SCRIPT}
FFMPEG_INC=${FFMPEG_OUT}/include
FFMPEG_LIB=${FFMPEG_OUT}/lib
FFMPEG_BIN=${FFMPEG_OUT}/bin
FFMPEG_SHARE=${FFMPEG_OUT}/share
FFMPEG_VERSION="4.2.2"
CPU_CORE=`cat /proc/cpuinfo |grep "processor"|wc -l`

if [ $1 == "clean" ];then   #clean
	if [ -f ${FFMPEG_SRC}/ffmpeg-${FFMPEG_VERSION} ];then
		cd ${FFMPEG_SRC}/ffmpeg-${FFMPEG_VERSION}
		make distclean
	fi
	rm -rf $FFMPEG_INC $FFMPEG_LIB $FFMPEG_BIN $FFMPEG_SHARE $FFMPEG_SRC/ffmpeg-${FFMPEG_VERSION}
	exit
else     					#build
	cd $FFMPEG_SRC
	tar -xvf ffmpeg-${FFMPEG_VERSION}.tar.gz
	cd ffmpeg-${FFMPEG_VERSION}
	cp $CONFIG_SCRIPT_PATH .
	./$CONFIG_SCRIPT $FFMPEG_OUT
	make -j${CPU_CORE} && make install
fi

if [ ! -d ${FFMPEG_LIB}/${PLATFORM} ];then
	mkdir ${FFMPEG_LIB}/${PLATFORM}
fi
mv $FFMPEG_LIB/*.a ${FFMPEG_LIB}/${PLATFORM}
