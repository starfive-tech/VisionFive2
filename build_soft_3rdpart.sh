#!/bin/bash
TOP_DIR=$(pwd)

# parse the args to install to initramfs or rootfs target path
if [ $# -eq 0 ]; then
    Target_DIR="$TOP_DIR/work/buildroot_initramfs_sysroot/root/"
else
    if [ "$1" = "initramfs" ]; then
        Target_DIR="$TOP_DIR/work/buildroot_initramfs_sysroot/root/"
    elif [ "$1" = "rootfs" ]; then
        Target_DIR="$TOP_DIR/work/buildroot_rootfs/target/root/"
    else
        echo "Usage: $0 [initramfs|rootfs]"
        echo "If not specify args, the default is initramfs"
        echo "For example:"
        echo "    $0";
        echo "    $0 initramfs";
        echo "    $0 rootfs";
        exit 1
    fi
fi
echo "$Target_DIR"

if [ ! -d $Target_DIR ]; then
    echo "Warning: need building the usdk firstly: $1"
    exit 1
fi

################################################################
# 1. jpu drvier and test script
JPU_SRC_DIR="./soft_3rdpart/codaj12/"
if [ -d $Target_DIR/jpu_driver ]; then
    rm -rf $Target_DIR/jpu_driver
fi
cd $JPU_SRC_DIR && ./build_for_riscv.sh && cp -ar jpu_driver/ $Target_DIR/
cd $TOP_DIR
echo "install jpu ok"

################################################################
# 2. vpu dec drvier and test script
VDEC_SRC_DIR="./soft_3rdpart/wave511/code/"
if [ -d $Target_DIR/vdec_driver ]; then
    rm -rf $Target_DIR/vdec_driver
fi
cd $VDEC_SRC_DIR && ./build_for_vdec.sh && cp -ar vdec_driver/ $Target_DIR/
cd $TOP_DIR
echo "install vdec ok"

################################################################
# 3. vpu enc drvier and test script
VENC_SRC_DIR="./soft_3rdpart/wave420l/code/"
if [ -d $Target_DIR/venc_driver ]; then
    rm -rf $Target_DIR/venc_driver
fi
cd $VENC_SRC_DIR && ./build.sh && cp -ar venc_driver/ $Target_DIR/
cd $TOP_DIR
echo "install venc ok"

################################################################
# 4. mailbox driver and test demo
# How to test: refer to soft_3rdpart/mailbox/mailbox_test.docx
if [ -d $Target_DIR/mailbox ]; then
    rm -rf $Target_DIR/mailbox
fi
mkdir -p $Target_DIR/mailbox
cp work/linux/drivers/mailbox/starfive_mailbox.ko \
    work/linux/drivers/mailbox/starfive_mailbox-test.ko \
    soft_3rdpart/mailbox/read_test $Target_DIR/mailbox
chmod +x $Target_DIR/mailbox/read_test
echo "install mailbox ok"

################################################################
# 5. e24 driver and test demo
# How to test: refer to soft_3rdpart/e24/e24_test.docx
if [ -d $Target_DIR/e24 ]; then
    rm -rf $Target_DIR/e24
fi
mkdir -p $Target_DIR/e24
# install e24 firmware
cp soft_3rdpart/e24/e24_elf $Target_DIR/../lib/firmware/
# install e24 driver and test demo
cp work/linux/drivers/e24/e24.ko \
    soft_3rdpart/e24/e24_share_mem $Target_DIR/e24
echo "install e24 ok"

################################################################
# 6. xrp driver
cp work/linux/drivers/xrp/xrp.ko $Target_DIR/
