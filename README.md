# StarFiveTech Freedom Unleashed SDK

This builds a complete RISC-V cross-compile toolchain for the StarFiveTech Freedom
JH7100 SoC. It also builds U-boot and a flattened image tree (FIT)
image with a Opensbi binary, linux kernel, device tree, ramdisk and rootdisk for the 
Starlight development board.

## Tested Configurations

### Ubuntu 16.04/18.04 x86_64 host

- Status: Working
- Build dependencies: `build-essential git autotools texinfo bison flex
  libgmp-dev libmpfr-dev libmpc-dev gawk libz-dev libssl-dev`
- Additional build deps for QEMU: `libglib2.0-dev libpixman-1-dev`
- Additional build deps for Spike: `device-tree-compiler`
- tools require for 'format-boot-loader' target: mtools

## Build Instructions

Checkout this repository and checkout to vic_starlight branch. Then you will need to checkout all of the linked
submodules using:

`git submodule update --recursive --init`

This will take some time and require around 7GB of disk space. Some modules may
fail because certain dependencies don't have the best git hosting. The only
solution is to wait and try again later (or ask someone for a copy of that
source repository).

Once the submodules are initialized, 4 modules buildroot, u-boot, linux and opensbi will checkout to vic_starlight branch.
After update submodules, run `make` or `make -jx` and the complete toolchain and
fw_payload.bin.out & image.fit will be built. The completed build tree will consume about 14G of
disk space.


