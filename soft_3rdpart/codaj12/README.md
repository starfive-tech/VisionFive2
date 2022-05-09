# JPU(CODA12) driver/test  for linux

本工程编译依赖于freelight-u-sdk的buildroot编译工具链。

> 注：JPU_PATH为本工程的顶层目录

#### 一键编译脚本

```shell
$cd $JPU_PATH && ./build_ffmpeg.sh   	#编译ffmpeg
$cd $JPU_PATH && ./build_for_riscv.sh	#编译jpu driver && jpu 测试程序
```

在freelight-u-sdk的buildroot编译工具链编译生成后，进入JPU_PATH,运行build_for_riscv.sh一键编译脚本。

 该脚本会将编译出的驱动（jpu.ko）、用户态程序(jpg_enc_test 、jpg_dec_test、multi_instance_test)、脚本文件（script）、测试/配置源文件（cfg、steam、yuv）拷贝到initramfs的文件系统中，在freedom-u-sdk顶层目录make clean后重新make freedom-u-sdk工程，可以将所有文件打包到image.fit中，系统启动后可以直接进入到对应目录，运行测试程序。或者用户也可以待vic linux系统启动后，将编译好的相关文件，通过网络（比如scp tftp等）手动将需要的文件传输到initramfs文件系统中。

#### 一键运行脚本

linux系统启动后

```shell
$cd root/jpu/driver && ./load.sh  #安装模块驱动
$cd ../ && ./run_all.sh 		  #运行所有测试项
```

### 手动编译安装说明

#### ffmpeg编译

```shell
$cd $JPU_PATH/ffmpeg/src  && tar -xvf ffmpeg-*.tar.gz   #进入ffmpeg源码目录，解压源码
$cd ffmpeg-xxx && ./config_ffmpeg_riscv.sh     			#运行ffmpeg配置
$make -jN && make install 								#默认安装到 $JPU_PATH/ffmpeg/
```

#### 内核驱动模块编译

```shell
 $cd  $JPU_PATH/jdi/linux/driver
 $source build_env_setup_riscv.sh
 $make
```

将生成jpu.ko模块驱动文件，使用load.sh安装模块驱动文件。

#### 编译用户态编解码测试程序

```shell
 $cd $JPU_PATH
 $make clean && make
```

将生成jpg_enc_test 、jpg_dec_test可执行程序，分别用于编码、解码测试。

#### 编译用户态多路测试程序：

```shell
$cd $JPU_PATH
$make clean && make MAKECMDGOALS=multi
```

将生成multi_instance_test，最多支持4路同时编、解码操作。

#### 驱动模块安装

将$JPU_PATH/jdi/linux/driver/目录下编译出的jpu.ko文件和load.sh一起放进文件系统中，执行load.sh,安装模块驱动。

#### 编解码测试

将script目录下的.sh文件拷贝到$JPU_PATH目录：

```shell
$./run_enc.sh   #编码测试，编码后的文件存在output/enc中
$./run_dec.sh   #解码测试，解码后的文件存在output/dec中
$./run_multi.sh #多路编解码测试,编解码后的文件存在output/multi中
```

#### 验证

*.yuv：解码后的文件使用yuvplayer.exe查看，需要根据文件名称手动指定分辨率、yuv格式等。注意，该工具不支持12bit的yuv文件查看。

*.jpg：使用NewView.exe程序打开查看
