# StarFiveTech Freedom Unleashed SDK

This builds a complete RISC-V cross-compile toolchain for the StarFiveTech Freedom
`JH7110` SoC. It also builds U-boot SPL, U-boot and a flattened image tree (FIT)
image with a Opensbi binary, linux kernel, device tree, ramdisk and rootdisk for the 
`JH7110 EVB` board.

## Prerequisites

### Ubuntu 16.04/18.04/20.04 x86_64 host

- Status: Working
- Build dependencies: `build-essential g++ git autoconf automake autotools texinfo bison xxd curl flex gawk gdisk gperf libgmp-dev libmpfr-dev libmpc-dev libz-dev libssl-dev libncurses-dev libtool patchutils python screen texinfo unzip zlib1g-dev libyaml-dev`
- Additional build deps for **QEMU**: `libglib2.0-dev libpixman-1-dev`
- Additional build deps for **Spike**: `device-tree-compiler`
- tools require for  **format-boot-loader** target: `mtools`

## Fetch code Instructions ##

Checkout this repository  (branch `jh7110-devel`). Then you will need to checkout all of the linked submodules using:

	$ git checkout --track origin/jh7110-devel
	$ git submodule update --init --recursive

This will take some time and require around 7GB of disk space. Some modules may fail because certain dependencies don't have the best git hosting. The only solution is to wait and try again later (or ask someone for a copy of that source repository).

Once the submodules are initialized, 4 submodules `buildroot`, `u-boot`,
`linux` and `opensbi` need checkout to corresponding branches manually, seeing `.gitmodule`

```
$ cd buildroot && git checkout jh7110-devel && cd ..
$ cd u-boot && git checkout jh7110-devel && cd ..
$ cd linux && git checkout --track origin/jh7110-5.15.y-devel && cd ..
$ cd opensbi && git checkout master && cd ..
```

## Build Instructions

After update submodules, run `make` or `make -jx` and the complete toolchain and
`evb_fw_payload.img` & `image.fit` will be built. The completed build tree will consume about 14G of disk space.

By default, the above generated image does not contain VPU module (wave511, the video hard decode driver and wave420l, the video hard encode driver) , JPU module (codaj12, the jpeg/mjpeg hard decode&encode driver), mailbox test app, e24 test app.  The following instructions will add VPU module, JPU module, mailbox test app and e24 test app according to your requirement:

	$ make -jx
	$ ./build_soft_3rdpart.sh
	$ rm -rf work/initramfs.cpio.gz
	$ make -jx

Then the below target files will be generated, copy files to tftp server workspace path:

```
work/image.fit
work/evb_fw_payload.img
work/u-boot-spl.bin.normal.out

work/initramfs.cpio.gz
work/linux/arch/riscv/boot/Image.gz

$ tree work/linux/arch/riscv/boot/dts/starfive
├── evb-overlay
│   ├── jh7110-evb-overlay-can.dtbo
│   ├── jh7110-evb-overlay-rgb2hdmi.dtbo
│   ├── jh7110-evb-overlay-sdio.dtbo
│   ├── jh7110-evb-overlay-spi.dtbo
│   ├── jh7110-evb-overlay-uart4-emmc.dtbo
│   └── jh7110-evb-overlay-uart5-pwm.dtbo
├── jh7110-evb-can-pdm-pwmdac.dtb
├── jh7110-evb.dtb
├── jh7110-evb-dvp-rgb2hdmi.dtb
├── jh7110-evb-pcie-i2s-sd.dtb
├── jh7110-evb-spi-uart2.dtb
├── jh7110-evb-uart1-rgb2hdmi.dtb
├── jh7110-evb-uart4-emmc-spdif.dtb
└── jh7110-evb-uart5-pwm-i2c-tdm.dtb
```

Note the make command to config buildroot, uboot, linux, busybox configuration:

```
$ make buildroot_initramfs-menuconfig   # initramfs menuconfig
$ make buildroot_rootfs-menuconfig      # rootfs menuconfig
$ make uboot-menuconfig      # uboot menuconfig
$ make linux-menuconfig      # Kernel menuconfig
$ make -C ./work/buildroot_initramfs/ O=./work/buildroot_initramfs busybox-menuconfig  # for initramfs busybox menuconfig
$ make -C ./work/buildroot_rootfs/ O=./work/buildroot_rootfs busybox-menuconfig  # for rootfs busybox menuconfig
```

## Running on JH7110 EVB Board via Network

After the JH7110 EVB Board is properly connected to the serial port cable, network cable and power cord, turn on the power from the wall power socket to power and you will see the startup information as follows:

```
U-Boot 2021.10 (Jul 01 2022 - 21:30:49 +0800)

CPU:   rv64imacu
Model: StarFive JH7110 EVB
DRAM:  4 GiB
FREQ:  1250 MHz
MMC:   sdio0@16010000: 0
Loading Environment from nowhere... OK
In:    serial@10000000
Out:   serial@10000000
Err:   serial@10000000
Model: StarFive JH7110 EVB
Net:   eth0: ethernet@16030000, eth1: ethernet@16040000
Hit any key to stop autoboot:  0 
StarFive # 
```

Then press any key to stop and enter uboot terminal, there are two way to boot the board

### Running image.fit with the default dtb `jh7110-evb.dtb`

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

### Running the other dtb with the Image.gz and initramfs.cpio.gz

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

## Building TF Card Booting Image

If you don't already use a local tftp server, then you probably want to make the TF card target; the default size is 16 GBs. **NOTE THIS WILL DESTROY ALL EXISTING DATA** on the target TF card; The `GPT` Partition Table for the TF card is recommended. 

Please insert the TF card to the host and run command `df -h` to check the device name `/dev/sdXX`, then run command `umount /dev/sdXX`",  then run the following instructions to build TF card image:

```
$ make -jx
$ make buildroot_rootfs -jx
$ make DISK=/dev/sdX format-rootfs-image && sync
```

## Using DTB Overlay Dynamically

The system support load dtb overlay dynamically when the board is running. The detail process to use the dtbo please reference to the dtbo documents.


