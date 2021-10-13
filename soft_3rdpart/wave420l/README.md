# WAVE420L编译与使用

## 主目录结构说明

doc				  	文档，包括ip和api说明文档

code					源码，包含驱动和用户态示例程序

firmware			固件

README.md 	 本文件

### 【wave420l代码结构】

wave420l源码由两部分组成: 

​	code/vdi/linux/driver: 编译生成vpu.ko，为wave420l的模块文件，主要功能是空间分配及映射、中断注册及平台无关功能实现。

​	code/下除vdi/linux/driver以外的目录: 编译生成sample app。主要功能是初始化vpu，设置vpu工作参数，启动vpu编码工作。

## 代码编译

编译程序需要进入wave420l/code目录。

code目录主要文件说明：

set_env.sh						 配置环境变量文件

Wave420Driver.mak		编译linux模块驱动makefile文件

Wave420Enc.mak		编译单路编码示例程序makefile

### 驱动模块编译

$ source set_env.sh 

$ make -f  Wave420Driver.mak  

编译成功后在vdi/linux/driver目录下会生成vpu.ko，在运行wave420l sample app前，需要先通过专用加载脚本加载该模块。

### sample app程序编译

$ source set_env.sh 	

$ make -f  Wave420Enc.mak  

编译成功后在code目录下会生成w4_enc_test可执行程序。

## wave420L运行

### 1.vpu.ko加载

编译生成的vpu.ko不能直接通过insmod命令加载，需要使用code\vdi\linux\driver\load.sh脚本进行加载。

### 2.sample app运行

运行run.sh，开始编码。具体命令如下：

***./w4_enc_test --secondary-axi=0 --stream-endian=16 --frame-endian=16 --enable-lineBufInt -n25  --kbps=30 --nv21=0 --picWidth=256 --picHeight=128 --output=set1_001A.cfg.bin --cfgFileName=./cfg/set1_001A.cfg --ref_stream_path=./yuv/foreman_256x128.yuv***

典型的放置目录结构如下：

![image-20211013174222901](C:\Users\jiack\AppData\Roaming\Typora\typora-user-images\image-20211013174222901.png)

monet.bin/monet.h：与wave420l/firmware目录下内容对应

w4_enc_test：sample app测试程序

vpu.ko：vpu驱动程序

load.sh：vpu.ko加载脚本

其它：与code目录下相同名字的内容对应

