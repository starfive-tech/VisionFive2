PWD=`pwd`
RISCV_TOOLCHAIN_BASE=${PWD}/../../../work/buildroot_initramfs/host
export PATH=${RISCV_TOOLCHAIN_BASE}/bin:$PATH
export LD_LIBRARY_PATH=${RISCV_TOOLCHAIN_BASE}/lib:$LD_LIBRARY_PATH
export ARCH=riscv
export SUBARCH=riscv
export CROSS_COMPILE=riscv64-buildroot-linux-gnu-
export BUILD_CONFIGURATION=RiscvLinux



