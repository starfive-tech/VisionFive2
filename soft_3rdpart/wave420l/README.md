# WAVE420L编译与使用

## 主目录结构说明

doc				  	文档，包括ip和api说明文档

code					源码，包含驱动和用户态示例程序

firmware			固件

README.md 	 本文件

## wave420l代码结构

wave420l源码由两部分组成: 

​	code/vdi/linux/driver: 编译生成venc.ko，为wave420l的模块文件，主要功能是空间分配及映射、中断注册及平台无关功能实现。

​	code/下除vdi/linux/driver以外的目录: 编译生成sample app。主要功能是初始化venc，设置venc工作参数，启动venc编码工作。

## 代码编译

编译程序需要进入wave420l/code目录。

code目录主要文件说明：

set_env.sh						 配置环境变量文件

Wave420Driver.mak		编译linux模块驱动makefile文件

Wave420Enc.mak		编译单路编码示例程序makefile

build.sh							工程编译脚本

### 编译脚本

提供一键编译脚本，所有的输出文件存放在venc_driver目录中。其中驱动在venc_driver/driver目录中，其他运行时需要的应用程序、运行脚本、yuv源文件、固件等，存放在venc_driver中。

```bash
$ ./build.sh
```

## wave420L运行

### 1. venc.ko加载

编译生成的venc.ko通过专用的加载脚本进行加载(load.sh)/卸载(unload.sh)。

脚本存放在目录venc_driver/driver中。

```bash
# cd venc_driver/
# ls
cfg                  enc_352x288_8b.sh    stream
driver               enc_4096x2160_8b.sh  w4_enc_test
enc_1920x1080_8b.sh  monet.bin            yuv
enc_256x128.sh       output
enc_352x288_10b.sh   run.sh
# cd driver/
# ls
load.sh    unload.sh  venc.ko
# ./load.sh 
[ 1213.628546] SUCCESS alloc_chrdev_region
[ 1213.636767] [pmu]domain: 0x40 enable
```

### 2. sample app运行

运行所有的enc_*.sh，执行编码操作。

也可以执行./run.sh,运行所有测试脚本。

编码输出的H265流文件存放在output目录中。

```bash
$ cd  venc_driver/
$ ./enc_1920x1080_8b.sh 
[VDI] instance pool physaddr=0x4005000, virtaddr=0xffffffff98142000, base=0x4005000, size=4096
[VDI] map vdb_register core_idx=0, virtaddr=0x3f97f12000, size=65535
[VDI] allocate_common_memory, physaddr=0xffffffffa0100000, virtaddr=0xffffffff97d12000
[VDI] vdi_get_common_memory physaddr=0xffffffffa0100000, size=2097152, virtaddr=0xffffffff97d12000
[VDI] success to init driver 
FW PATH = monet.bin
[VDI] instance pool physaddr=0x4005000, virtaddr=0xffffffff97dfc000, base=0x4005000, size=4096
[VDI] map vdb_register core_idx=0, virtaddr=0x3f97dec000, size=65535
[VDI] allocate_common_memory, physaddr=0xffffffffa0100000, virtaddr=0xffffffff97bec000
[VDI] vdi_get_common_memory physaddr=0xffffffffa0100000, size=2097152, virtaddr=0xffffffff97bec000
[VDI] success to init driver 

VPU INIT Start!!!

get hw version 0 !!!
VPU coreNum : [0]
Firmware : CustomerCode: 0000 | version : 0.0.0 rev.196485
Hardware : 0009
API      : 5.5.34

[VDI] vdi_allocate_dma_memory, physaddr=0xa0300000, virtaddr=0x3f970ec000~0x3f97bec000, size=11534336
STREAM_BUF=0xa0300000 STREAM_BUF_SIZE=11534336(0xb00000)
[VDI] vdi_allocate_dma_memory, physaddr=0xa0020000, virtaddr=0x3f970cc000~0x3f970ec000, size=131072
* Enc InitialInfo =>
 instance #0, 
 minframeBuffercount: 2
 minSrcBufferCount: 1
 picWidth: 1920
 picHeight: 1080
 [VDI] vdi_allocate_dma_memory, physaddr=0xa0e00000, virtaddr=0x3f96dd4000~0x3f970cb600, size=3110400
[VDI] vdi_allocate_dma_memory, physaddr=0xa1100000, virtaddr=0x3f96adc000~0x3f96dd3600, size=3110400
[VDI] vdi_allocate_dma_memory, physaddr=0xa0040000, virtaddr=0x3f96abb000~0x3f96adc000, size=135168
[VDI] vdi_allocate_dma_memory, physaddr=0xa0080000, virtaddr=0x3f96a98000~0x3f96abb000, size=143360
[VDI] vdi_allocate_dma_memory, physaddr=0xa00c0000, virtaddr=0x3f96a86000~0x3f96a98000, size=73728
[VDI] vdi_allocate_dma_memory, physaddr=0xa1400000, virtaddr=0x3f96a45000~0x3f96a86000, size=266240
Allocated source framebuffers : 1, size : 3110400
[VDI] vdi_allocate_dma_memory, physaddr=0xa1500000, virtaddr=0x3f9674d000~0x3f96a44600, size=3110400
output bin file: ./output/inter_8b_11.cfg.265
Enc Start : Press any key to stop.
------------------------------------------------------------------------------
I     NO     T   RECON   RD_PTR    WR_PTR     BYTES  SRCIDX  USEDSRCIDX Cycle 
------------------------------------------------------------------------------
00     1     0     0    a0300000  a0319f70    19f70      0         0     5111808
00     2     1     1    a0300000  a030b867     b867      0         0     5013504
00     3     1     0    a0300000  a030bbb2     bbb2      0         0     5046272
00     4     1     1    a0300000  a030ba33     ba33      0         0     5046272
00     5     1     0    a0300000  a030ba09     ba09      0         0     5046272
00     6     1     1    a0300000  a030b63d     b63d      0         0     5046272
00     7     1     0    a0300000  a030ba7c     ba7c      0         0     5046272
00     8     1     1    a0300000  a030bb5d     bb5d      0         0     5046272
00     9     1     0    a0300000  a030b736     b736      0         0     5046272
00    10     1     1    a0300000  a030baa1     baa1      0         0     5046272
00    10     0    -1    a0300000  a0300000        0     -1        -2           0

inst 0 Enc End. Tot Frame 10
# ls
ErrorLog.txt         enc_352x288_10b.sh   run.sh
cfg                  enc_352x288_8b.sh    stream
driver               enc_4096x2160_8b.sh  w4_enc_test
enc_1920x1080_8b.sh  monet.bin            yuv
enc_256x128.sh       output
```

### 3. venc.ko卸载

```bash
$ cd  venc_driver/driver/
$ ./unload.sh
```

### 4. 文件导出

可以通过网络scp或者tftp将编码后的文件导出，使用视频播放工具(例如VLC)播放流视频。