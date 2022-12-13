# StarFiveTech VisionFive2 SDK

This builds a complete RISC-V cross-compile toolchain for the `StarFiveTech` `JH7110` SoC. It also builds U-boot SPL, U-boot and a flattened image tree (FIT) image with a Opensbi binary, linux kernel, device tree, ramdisk image and rootfs image for the `JH7110 VisionFive2` board.

## Prerequisites

Recommend OS: Ubuntu 16.04/18.04/20.04 x86_64

Install required additional packages:

```
$ sudo apt update
$ sudo apt-get install build-essential g++ git autoconf automake autotools-dev texinfo bison xxd curl flex gawk gdisk gperf libgmp-dev libmpfr-dev libmpc-dev libz-dev libssl-dev libncurses-dev libtool patchutils python screen texinfo unzip zlib1g-dev libyaml-dev wget cpio bc dosfstools mtools device-tree-compiler libglib2.0-dev libpixman-1-dev kpartx
```

Additional packages for Git LFS support:

```
$ curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | sudo bash
$ sudo apt-get install git-lfs
```

## Fetch Code Instructions ##

Checkout this repository  (e.g.: branch `JH7110_VisionFive2_devel`). Then checkout all of the linked submodules using:

	$ git clone git@github.com:starfive-tech/VisionFive2.git
	$ cd VisionFive2
   	$ git checkout JH7110_VisionFive2_devel
	$ git submodule update --init --recursive

This will take some time and require around 5GB of disk space. Some modules may fail because certain dependencies don't have the best git hosting. The only solution is to wait and try again later (or ask someone for a copy of that source repository).

For user who build the release tag version, the above command is enough. For developer, need to switch the 4 submodules `buildroot`, `u-boot`, `linux`, `opensbi` to correct branch manually, also could refer to `.gitmodule`

```
$ cd buildroot && git checkout --track origin/JH7110_VisionFive2_devel && cd ..
$ cd u-boot && git checkout --track origin/JH7110_VisionFive2_devel && cd ..
$ cd linux && git checkout --track origin/JH7110_VisionFive2_devel && cd ..
$ cd opensbi && git checkout master && cd ..
```

## Quick Build Instructions

Below are the quick building for the initramfs image `image.fit` which could be translated to board through tftp and run on board. The completed toolchain, `u-boot-spl.bin.normal.out`, `visionfive2_fw_payload.img`, `image.fit` will be generated under `work/` directory. The completed build tree will consume about 16G of disk space.

	$ make -j$(nproc)

Then the below target files will be generated, copy files to tftp server workspace path:

```
work/
├── visionfive2_fw_payload.img
├── image.fit
├── initramfs.cpio.gz
├── u-boot-spl.bin.normal.out
├── linux/arch/riscv/boot
    ├── dts
    │   └── starfive
    │       ├── jh7110-visionfive-v2-A10.dtb
    │       ├── jh7110-visionfive-v2-A11.dtb
    │       ├── jh7110-visionfive-v2-ac108.dtb
    │       ├── jh7110-visionfive-v2.dtb
    │       ├── jh7110-visionfive-v2-wm8960.dtb
    │       ├── vf2-overlay
    │       │   └── vf2-overlay-uart3-i2c.dtbo
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
$ make vmlinux   # build linux kernel
$ make -C ./work/buildroot_rootfs/ O=./work/buildroot_rootfs busybox-rebuild   # build busybox package
$ make -C ./work/buildroot_rootfs/ O=./work/buildroot_rootfs ffmpeg-rebuild    # build ffmpeg package
```

## Running on JH7110 VisionFive2 Board via Network

After the JH7110 VisionFive2 Board is properly connected to the serial port cable, network cable and power cord, turn on the power from the wall power socket to power and you will see the startup information as follows:

```
U-Boot SPL 2021.10 (Oct 31 2022 - 12:11:37 +0800)
DDR version: dc2e84f0.
Trying to boot from SPI

OpenSBI v1.0
   ____                    _____ ____ _____
  / __ \                  / ____|  _ \_   _|
 | |  | |_ __   ___ _ __ | (___ | |_) || |
 | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
 | |__| | |_) |  __/ | | |____) | |_) || |_
  \____/| .__/ \___|_| |_|_____/|____/_____|
        | |
        |_|

Platform Name             : StarFive VisionFive V2
Platform Features         : medeleg
Platform HART Count       : 5
Platform IPI Device       : aclint-mswi
Platform Timer Device     : aclint-mtimer @ 4000000Hz
Platform Console Device   : uart8250
Platform HSM Device       : ---
Platform Reboot Device    : ---
Platform Shutdown Device  : ---
Firmware Base             : 0x40000000
Firmware Size             : 360 KB
Runtime SBI Version       : 0.3

Domain0 Name              : root
Domain0 Boot HART         : 3
Domain0 HARTs             : 0*,1*,2*,3*,4*
Domain0 Region00          : 0x0000000002000000-0x000000000200ffff (I)
Domain0 Region01          : 0x0000000040000000-0x000000004007ffff ()
Domain0 Region02          : 0x0000000000000000-0xffffffffffffffff (R,W,X)
Domain0 Next Address      : 0x0000000040200000
Domain0 Next Arg1         : 0x0000000042200000
Domain0 Next Mode         : S-mode
Domain0 SysReset          : yes

Boot HART ID              : 3
Boot HART Domain          : root
Boot HART Priv Version    : v1.11
Boot HART Base ISA        : rv64imafdcbx
Boot HART ISA Extensions  : none
Boot HART PMP Count       : 8
Boot HART PMP Granularity : 4096
Boot HART PMP Address Bits: 34
Boot HART MHPM Count      : 2
Boot HART MIDELEG         : 0x0000000000000222
Boot HART MEDELEG         : 0x000000000000b109


U-Boot 2021.10 (Oct 31 2022 - 12:11:37 +0800), Build: jenkins-VF2_515_Branch_SDK_Release-10

CPU:   rv64imacu
Model: StarFive VisionFive V2
DRAM:  8 GiB
MMC:   sdio0@16010000: 0, sdio1@16020000: 1
Loading Environment from SPIFlash... SF: Detected gd25lq128 with page size 256 Bytes, erase size 4 KiB, total 16 MiB
*** Warning - bad CRC, using default environment

StarFive EEPROM format v2

--------EEPROM INFO--------
Vendor : StarFive Technology Co., Ltd.
Product full SN: VF7110A1-2243-D008E000-00000001
data version: 0x2
PCB revision: 0xa1
BOM revision: A
Ethernet MAC0 address: 6c:cf:39:00:14:5b
Ethernet MAC1 address: 6c:cf:39:00:14:5c
--------EEPROM INFO--------

In:    serial@10000000
Out:   serial@10000000
Err:   serial@10000000
Model: StarFive VisionFive V2
Net:   eth0: ethernet@16030000, eth1: ethernet@16040000
switch to partitions #0, OK
mmc1 is current device
found device 1
bootmode flash device 1
Failed to load 'uEnv.txt'
Can't set block device
Hit any key to stop autoboot:  0 
StarFive # 
```

Then press any key to stop and enter uboot terminal, there are two way to boot the board

#### 1. Running image.fit with the default dtb `jh7110-visionfive-v2.dtb`

transfer image.fit through TFTP:

Step1: set enviroment parameter:

```
setenv bootfile vmlinuz; setenv fileaddr a0000000; setenv fdtcontroladdr 0xffffffffffffffff; setenv ipaddr 192.168.xxx.xxx; setenv serverip 192.168.xxx.xxx;
```

Step2: upload image file to ddr:

```
tftpboot ${fileaddr} ${serverip}:image.fit;
```

Step3: load and excute:

```
bootm start ${fileaddr};bootm loados ${fileaddr};run chipa_set_linux;booti 0x40200000 0x46100000:${filesize} 0x46000000
```

When you see the `buildroot login:` message, then congratulations, the launch was successful

```
buildroot login:root
Password: starfive
```

#### 2. Running the other dtb with the Image.gz and initramfs.cpio.gz

If we want to loading the other dtb, e.g. `jh7110-visionfive-v2-wm8960.dtb`, follow the below

Step1: set enviroment parameter:

```
setenv bootfile vmlinuz; setenv fileaddr a0000000; setenv fdtcontroladdr 0xffffffffffffffff; setenv ipaddr 192.168.xxx.xxx; setenv serverip 192.168.xxx.xxx;
setenv kernel_comp_addr_r 0xb0000000;setenv kernel_comp_size 0x10000000;
```

Step2: upload files to ddr:

```
tftpboot ${fdt_addr_r} jh7110-visionfive-v2-wm8960.dtb;
tftpboot ${kernel_addr_r} Image.gz;
tftpboot ${ramdisk_addr_r} initramfs.cpio.gz;
run chipa_set_linux;
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
$ make img
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

The second way could be done on VisionFive2 board, use fdisk and resize2fs command：

```
# fdisk /dev/mmcblk1
Welcome to fdisk (util-linux 2.37.2).
Changes will remain in memory only, until you decide to write them.
Be careful before using the write command.
This disk is currently in use - repartitioning is probably a bad idea.
It's recommended to umount all file systems, and swapoff all swap
partitions on this disk.
Command (m for help): d
Partition number (1-4, default 4): 4
Partition 4 has been deleted.
Command (m for help): n
Partition number (4-128, default 4): 4
First sector (614400-62333918, default 614400):
): t sector, +/-sectors or +/-size{K,M,G,T,P} (614400-62333918, default 62333918)
Created a new partition 4 of type 'Linux filesystem' and of size 29.4 GiB.
Partition #4 contains a ext4 signature.
Do you want to remove the signature? [Y]es/[N]o: N
Command (m for help): w
The partition table has been altered.
Syncing disks.

# resize2fs /dev/mmcblk1p4
resize2fs 1.46.4 (18-Aug-2021)
Filesystem at /d[
111.756178] EXT4-fs (mmcblk1p4): resizing filesystem from 512000
to 30859756 blocks
ev/mmcblk1p4 is [
111.765203] EXT4-fs (mmcblk1p4): resizing filesystem from 512000
to 30859265 blocks
mounted on /; on-line resizing required
old_desc_blocks = 2, new_desc_blocks = 118
[ 112.141953] random: crng init done
[ 112.145369] random: 7 urandom warning(s) missed due to ratelimiting
[ 115.474184] EXT4-fs (mmcblk1p4): resized filesystem to 30859265
The filesystem on /dev/mmcblk1p4 is now 30859756 (1k) blocks long.
```

## APPENDIX II: Using DTB Overlay Dynamically

The system support loading dtb overlay dynamically when the board is running. Run below on board:

```
# mount -t configfs none /sys/kernel/config
# mkdir -p /sys/kernel/config/device-tree/overlays/dtoverlay
# cd <the dtoverlay.dtbo path>
# cat vf2-overlay-uart3-i2c.dtbo > /sys/kernel/config/device-tree/overlays/dtoverlay/dtbo
```

Additional, you could remove the dtbo feature:

```
# rmdir /sys/kernel/config/device-tree/overlays/dtoverlay
```

## APPENDIX III: Updating SPL and U-Boot binaries Under U-boot

Prepare the tftp sever. e.g. `sudo apt install tftpd-hpa` for Ubuntu host.

1. Power on the VisionFive2 board and wait until enters the u-boot command line

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
   StarFive # tftpboot 0xa0000000 ${serverip}:visionfive2_fw_payload.img
   StarFive # sf update 0xa0000000 0x100000 $filesize
   ```

