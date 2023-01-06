# StarFiveTech Freedom Unleashed SDK

This builds a complete RISC-V cross-compile toolchain for the `StarFiveTech` `JH7110` SoC. It also builds U-boot SPL, U-boot and a flattened image tree (FIT) image with a Opensbi binary, linux kernel, device tree, ramdisk image and rootfs image for the `JH7110 EVB` board.

## Prerequisites

Recommend OS: Ubuntu 16.04/18.04/20.04 x86_64

Install required additional packages:

```
$ sudo apt update
$ sudo apt-get install build-essential g++ git autoconf automake autotools texinfo
bison xxd curl flex gawk gdisk gperf libgmp-dev libmpfr-dev libmpc-dev libz-dev libssl-
dev libncurses-dev libtool patchutils python screen texinfo unzip zlib1g-dev libyaml-
dev wget cpio bc dosfstools mtools device-tree-compiler libglib2.0-dev libpixman-1-dev kpartx
```

Additional packages for Git LFS support:

```
$ curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | sudo bash
$ sudo apt-get install git-lfs
```

## Fetch Code Instructions ##

Checkout this repository  (e.g.: branch `jh7110-devel`). Then checkout all of the linked submodules using:

	$ git clone git@192.168.110.45:sdk/freelight-u-sdk.git
	$ git checkout --track origin/jh7110-devel
	$ git submodule update --init --recursive

In case someone run `git clone git@gitlab.starfivetech.com:sdk/freelight-u-sdk.git`, recommend to add the below at the tail of the /etc/hosts to fix the network domain issue:

```
192.168.110.45 gitlab.starfivetech.com
```

This will take some time and require around 7GB of disk space. Some modules may fail because certain dependencies don't have the best git hosting. The only solution is to wait and try again later (or ask someone for a copy of that source repository).

For user who build the release tag version, the above command is enough. For developer, need to switch the 5 submodules `buildroot`, `u-boot`, `linux`, `opensbi`, `soft_3rdpart` to correct branch manually, or refer to `.gitmodule`

```
$ cd buildroot && git checkout jh7110-devel && cd ..
$ cd u-boot && git checkout jh7110-master && cd ..
$ cd linux && git checkout jh7110-5.15.y-devel && cd ..
$ cd opensbi && git checkout master && cd ..
$ cd soft_3rdpart && git checkout jh7110-devel && cd ..
```

## Quick Build Instructions

Below are the quick building for the initramfs image `image.fit` which could be translated to board through tftp and run on board. The completed toolchain, `u-boot-spl.bin.normal.out`, `evb_fw_payload.img`, `image.fit` will be generated under `work/` directory. The completed build tree will consume about 15G of disk space.

	$ make -j$(nproc)

Then the below target files will be generated, copy files to tftp server workspace path:

```
work/
├── evb_fw_payload.img
├── image.fit
├── initramfs.cpio.gz
├── u-boot-spl.bin.normal.out
├── linux/arch/riscv/boot
    ├── dts
    │   └── starfive
    │       ├── evb-overlay
    │       │   ├── jh7110-evb-overlay-can.dtbo
    │       │   ├── jh7110-evb-overlay-rgb2hdmi.dtbo
    │       │   ├── jh7110-evb-overlay-sdio.dtbo
    │       │   ├── jh7110-evb-overlay-spi.dtbo
    │       │   ├── jh7110-evb-overlay-uart4-emmc.dtbo
    │       │   └── jh7110-evb-overlay-uart5-pwm.dtbo
    │       ├── jh7110-evb-can-pdm-pwmdac.dtb
    │       ├── jh7110-evb.dtb
    │       ├── jh7110-evb-dvp-rgb2hdmi.dtb
    │       ├── jh7110-evb-i2s-ac108.dtb
    │       ├── jh7110-evb-pcie-i2s-sd.dtb
    │       ├── jh7110-evb-spi-uart2.dtb
    │       ├── jh7110-evb-uart1-rgb2hdmi.dtb
    │       ├── jh7110-evb-uart4-emmc-spdif.dtb
    │       ├── jh7110-evb-uart5-pwm-i2c-tdm.dtb
    │       ├── jh7110-evb-usbdevice.dtb
    │       ├── jh7110-fpga.dtb
    └── Image.gz
```

Additional command to config buildroot, uboot, linux, busybox:

```
$ make buildroot_initramfs-menuconfig   # buildroot initramfs menuconfig
$ make buildroot_rootfs-menuconfig      # buildroot rootfs menuconfig
$ make uboot-menuconfig                 # uboot menuconfig
$ make linux-menuconfig                 # Kernel menuconfig
$ make -C ./work/buildroot_initramfs/ O=./work/buildroot_initramfs busybox-menuconfig  # for initramfs busybox menuconfig
$ make -C ./work/buildroot_rootfs/ O=./work/buildroot_rootfs busybox-menuconfig        # for rootfs busybox menuconfig
```

Additional command to build single package or module:

```
$ make vmlinu    # build linux kernel
$ make uboot     # build u-boot
$ make -C ./work/buildroot_rootfs/ O=./work/buildroot_rootfs busybox-rebuild   # build busybox package
$ make -C ./work/buildroot_rootfs/ O=./work/buildroot_rootfs ffmpeg-rebuild    # build ffmpeg package
$ make vpudriver-build # build wave511/wave420l/codaj12 driver
```

## Running on JH7110 EVB Board via Network

After the JH7110 EVB Board is properly connected to the serial port cable, network cable and power cord, turn on the power from the wall power socket to power and you will see the startup information as follows:

```
U-Boot 2021.10 (Oct 10 2022 - 22:49:48 +0800)

CPU:   rv64imacu
Model: StarFive JH7110 EVB
DRAM:  4 GiB
MMC:   sdio0@16010000: 0
Loading Environment from SPIFlash... SF: Detected gd25lq128 with page size 256 Bytes, erase size 4 KiB, total 16 MiB
In:    serial@10000000
Out:   serial@10000000
Err:   serial@10000000
Model: StarFive JH7110 EVB
Net:   eth0: ethernet@16030000, eth1: ethernet@16040000
Hit any key to stop autoboot:  0 
StarFive #
```

Then press any key to stop and enter uboot terminal, there are two way to boot the board

#### 1. Running image.fit with the default dtb `jh7110-evb.dtb`

transfer image.fit through TFTP:

Step1: set enviroment parameter:

```
setenv bootfile vmlinuz; setenv fdt_addr_r 0x48000000; setenv fdt_high 0xffffffffffffffff; setenv fdtcontroladdr 0xffffffffffffffff; setenv initrd_high 0xffffffffffffffff; setenv kernel_addr_r 0x44000000; setenv fileaddr a0000000; setenv ipaddr 192.168.xxx.xxx; setenv serverip 192.168.xxx.xxx;
```

Step2: upload image file to ddr:

```
tftpboot ${fileaddr} ${serverip}:image.fit;
```

Step3: load and excute:

	bootm start ${fileaddr};bootm loados ${fileaddr};booti 0x40200000 0x46100000:${filesize} 0x46000000
	or:
	bootm ${fileaddr}#config-1
	or:
	bootm ${fileaddr}

When you see the `buildroot login:` message, then congratulations, the launch was successful

```
buildroot login:root
Password: starfive
```

#### 2. Running the other dtb with the Image.gz and initramfs.cpio.gz

If we want to loading the other dtb, e.g. `jh7110-evb-pcie-i2s-sd.dtb`, follow the below

Step1: set enviroment parameter:

```
setenv bootfile vmlinuz; setenv fdt_addr_r 0x48000000; setenv fdt_high 0xffffffffffffffff; setenv fdtcontroladdr 0xffffffffffffffff; setenv initrd_high 0xffffffffffffffff; setenv kernel_addr_r 0x44000000; setenv fileaddr a0000000; setenv ipaddr 192.168.xxx.xxx; setenv serverip 192.168.xxx.xxx;
setenv kernel_comp_addr_r 0xb0000000;setenv kernel_comp_size 0x10000000;
```

Step2: upload files to ddr:

```
tftpboot ${fdt_addr_r} jh7110-evb-pcie-i2s-sd.dtb;
tftpboot ${kernel_addr_r} Image.gz;
tftpboot ${ramdisk_addr_r} initramfs.cpio.gz;
```

Step3: load and excute:

```
booti ${kernel_addr_r} ${ramdisk_addr_r}:${filesize} ${fdt_addr_r}
```

When you see the `buildroot login:` message, then congratulations, the launch was successful

```
buildroot login:root
Password: starfive
```

## APPENDIX I: Generate Booting SD Card

If you don't already use a local tftp server, then you probably want to make the TF card target; the default size is 16 GBs. **NOTE THIS WILL DESTROY ALL EXISTING DATA** on the target TF card; The `GPT` Partition Table for the TF card is recommended. 

#### Generate SD Card Image File

We could generate a sdcard image file by the below command. The sdcard image file could be burned into sd card or tf card through `dd` command, or `rpi-imager` or `balenaEtcher` tool

```
$ make -j$(nproc)
$ make buildroot_rootfs -j$(nproc)
$ ./genimage.sh
```

The output file `work/sdcard.img`  will be generated.

#### Burn Image File to SD Card

The `sdcard.img` can be burn into a tf card. e.g. through `dd` command as below

```
$ sudo dd if=work/sdcard.img of=/dev/sdX bs=4096
$ sync
```

Then extend the tf card rootfs partition if needed. There are two ways to implement it. 

The first way could be done on Ubuntu host, need to install the below package:

```
$ sudo apt install cloud-guest-utils e2fsprogs 
```

Then insert the tf card to Ubuntu host, run the below, note `/dev/sdX` is the tf card device.

```
$ sudo growpart /dev/sdX 4  # extend partition 4
$ sudo e2fsck -f /dev/sdX4
$ sudo resize2fs /dev/sdX4  # extend filesystem
$ sudo fsck.ext4 /dev/sdX4
```

The second way is when your system doesn't have `growpart` command, e.g. the buildroot on board. The tf card can be insert into the Ubuntu host or  just directly on JH7110 board. you can use fdisk instead：

```bash
#!bin/sh
disk=/dev/mmcblk0
part=p4
sudo fdisk $disk << EOF
p
d
4
n



NO
w
EOF

sudo e2fsck -f ${disk}${part}
sudo resize2fs ${disk}${part}
sudo fsck.ext4 ${disk}${part}
```

If you need to add a new partition, such as a swap partition (here we do set the rest of disk space to swap partition,
but normally swap partition size should be the same as DDR size or double of DDR size),
you can use the following shell script afer the image running on board:

```bash
#!bin/sh
sgdisk -e /dev/mmcblk0
disk=/dev/mmcblk0
gdisk $disk << EOF
p
n
5


8200
p
c
5
hibernation
w
y
EOF

mkswap /dev/mmcblk0p5
swapoff -a
swapon /dev/mmcblk0p5
```

## APPENDIX II: Using DTB Overlay Dynamically
The system support loading dtb overlay dynamically when the board is running. Run below on board:

```
# mount -t configfs none /sys/kernel/config
# mkdir -p /sys/kernel/config/device-tree/overlays/dtoverlay
# cd <the dtoverlay.dtbo path>
# cat jh7110-evb-overlay-rgb2hdmi.dtbo > /sys/kernel/config/device-tree/overlays/dtoverlay/dtbo
```

Additional, you could remove the dtbo feature:

```
# rmdir /sys/kernel/config/device-tree/overlays/dtoverlay
```

## APPENDIX III: Updating SPL and U-Boot binaries Under U-boot

Prepare the tftp sever. e.g. `sudo apt install tftpd-hpa` for Ubuntu host.

1. Power on the evaluation board and wait until enters the u-boot command line

2. Configure the environment variables by executing:

   ```
   StarFive # setenv ipaddr 192.168.120.222;setenv serverip 192.168.120.99
   ```

3. Check the connectivity by pinging the host PC from evaluation board;

4. Initialize SPI flash:

   ```
   StarFive # sf probe
   ```

5. Update SPL binary

   ```
   StarFive # tftpboot 0xa0000000 ${serverip}:u-boot-spl.bin.normal.out
   StarFive # sf update 0xa0000000 0x0 $filesize
   ```

6. Update U-Boot binary

   ```
   StarFive # tftpboot 0xa0000000 ${serverip}:evb_fw_payload.img
   StarFive # sf update 0xa0000000 0x100000 $filesize
   ```

## APPENDIX IV:  How to Switch to 4G DDR or 8G DDR

The detail process will add it later
