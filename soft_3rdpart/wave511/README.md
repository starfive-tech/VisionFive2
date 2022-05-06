## ﻿工程目录介绍

1. code                             驱动、测试源码和各种脚本等
2. doc                              设计文档相关
3. firmware                         IP内部固件二进制程序
4. media                            测试用多媒体视频源
5. ffmpeg                           ffmpeg工程
6. README.md                       本文件

## 工程编译

1. 运行 .  build_env.sh             编译环境设置
2. make -f WaveDecDriver.mak        驱动编译
3. make -f WaveDecode.mak           单路解码测试程序编译
4. make -f WaveDecodeMult.mak       多路解码测试程序编译
5. make -f WaveDecode-ffmpeg.mak    多媒体文件解码测试程序编译
6. build_for_vdec.sh 	                           本工程一键编译脚本
7. build_ffmpeg.sh                                  编译ffmpeg脚本

> 注意： 运行make -f WaveDecode-ffmpeg.mak 之前，需要先执行./build_ffmpeg.sh,编译得到ffmpeg相关的库文件和头文件。强烈建议使用build_for_vdec.sh一键编译脚本编译全部文件。

## 编译输出文件

1. vpu.ko                           内核解码驱动程序
2. dec_test                         单路解码测试程序
3. multi_instance_dec_test          多路解码测试程序
4. ffmpeg_dec_test                  带ffmpeg解析功能的解码测试程序

## 运行调试流程如下

1. 编译内核
2. 设置tftp 服务器
3. Freedom 中加载uboot，并且启动
4. 在Uboot控制台中利用网络加载内核并启动，样例如下：
   tftpboot 0xa0000000 192.168.70.220:image.fit
   bootm start 0xa0000000
   bootm loados 0xa0000000
   go 0x80700000 0x86000000
5. 输入内核用户/密码（root/sifive），配置网络参数，样例如下：
   ifconfig eth0 down
   ifconfig eth0 192.168.70.250 netmask 255.255.255.0
   route add default gw 192.168.70.1
   ifconfig eth0 up
5. 使用scp等工具，把调试用相关驱动、脚本和测试源下载到linux系统中
   例如：scp x:\yyy\zzz root@192.168.70.250:~，可能的其他文件包括：
     Vpu.ko                         驱动程序
     load.sh                        驱动加载脚本
     unload.sh                      驱动卸载脚本
     chagall.bin                    firmware程序
     dec_test                       单路测试程序
     multi_instance_dec_test        多路测试程序
     ffmpeg_dec_test                带容器解析功能的测试程序
     bd_10b_01.cfg_0.264            码流文件
     xxxx.mp4                       多媒体文件
6. 运行 load.sh 加载驱动
7. 运行 dec_test/multi_instance_dec_test/ffmpeg_dec_test 解码测试
8. 取回yuv视频帧，并验证，样例如下：
   scp root@192.168.70.250:~\zzz.yuv x:\yyy\

## 测试程序命令行样例

1. 单路解码
./dec_test --output ./out.dat --input tv0.265 --codec 12

2. 2路解码
./multi_instance_dec_test --codec 0,0 --input bd_8b_01.cfg_0.264-0,bd_8b_01.cfg_0.264-1 --output bd_8b_01.cfg_0.yuv-0,bd_8b_01.cfg_0.yuv-1 --instance-num 2 --enable-wtl 1,1

3. 4路解码
./multi_instance_dec_test --codec 0,0,0,0 --input bd_8b_01.cfg_0.264-0,bd_8b_01.cfg_0.264-1,bd_8b_01.cfg_0.264-2,bd_8b_01.cfg_0.264-3 --output bd_8b_01.cfg_0.yuv-0,bd_8b_01.cfg_0.yuv-1,bd_8b_01.cfg_0.yuv-2,bd_8b_01.cfg_0.yuv-3 --instance-num 4 --enable-wtl 1,1,1,1

4.支持ffmpeg parser/demuxer
./ffmpeg_dec_test  --codec 12 --output hevc_fhd_inter_8b_11.cfg_0.yuv --input media/hevc_fhd_inter_8b_11.cfg_0.mkv

## 备注

1. VDEC默认使用cma方式从通用的CMA空间分配mem。如果想给VDEC指定特定的预留空间，可以在dts文件中做预留，同时，在文件code/vdi/linux/driver/vdec.c中，打开VPU_SUPPORT_RESERVED_VIDEO_MEMORY宏。预留方式简单示例：



	```dts
	reserved_memory: reserved-memory {
		#address-cells = <2>;
		#size-cells = <2>;
		ranges;
	    vpu_reserved: framebuffer@d8000000 {
	        reg = <0x0 0xd8000000 0x0 0x20000000>;
	    };
	};
	vpu_dec:vpu_dec@118F0000 {
	    compatible = "starfive,vdec";
	    reg = <0 0x118F0000 0 0x10000>;
	    memory-region = <&vpu_reserved>;
	    interrupt-parent = <&plic>;
	    interrupts = <23>;
	    clocks = <&vpuclk>;
	    clock-names = "vcodec";
	    status = "okay";
	};
