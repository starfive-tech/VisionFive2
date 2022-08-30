#!/bin/bash


TOP_FILE_PATH=$(cd "$(dirname "$0")/../.."; pwd)
PARENT_FILE_PATH=$(cd "$(dirname "$0")/.."; pwd)
FILE_PATH=$(cd "$(dirname "$0")"; pwd)


mkdir -p ${FILE_PATH}/cmake
mkdir -p ${FILE_PATH}/install


TargetSystem="${1,,}"   # Convert parameter 1 to lower case
#TargetSystem="${1^^}"   # Convert parameter 1 to upper case


if [ $# -gt 0 ]; then
    case "$TargetSystem" in
        "linux" )
            echo "Build DDK for Linux system..."
            #CROSS_COMPILE=
            CROSS_COMPILE=/usr/bin/
            #CROSS_COMPILE=/usr/bin/linux-
            DRM_INCLUDE_DIR=
            Arch="x86"
            ;;
         "embedded")
            echo "Build DDK for Embedded Linux system on ARM platform..."
            #CROSS_COMPILE=${TOP_FILE_PATH}/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-
            CROSS_COMPILE=${TOP_FILE_PATH}/buildroot/prebuilts/uclibc-4.7.3/bin/arm-buildroot-linux-uclibcgnueabihf-
            LINUX_KERNEL_BUILD_DIR=${TOP_FILE_PATH}/output/build/linux-local
            DRM_INCLUDE_DIR=
            Arch="arm"
            ;;
         "riscv")
            echo "Build DDK for Embedded Linux system on RISC-V platform..."
            CROSS_COMPILE=${TOP_FILE_PATH}/work/buildroot_initramfs/host/usr/bin/riscv64-buildroot-linux-gnu-
            LINUX_KERNEL_BUILD_DIR=${TOP_FILE_PATH}/work/linux
            DRM_INCLUDE_DIR=${TOP_FILE_PATH}/work/buildroot_initramfs/host/riscv64-buildroot-linux-gnu/sysroot/usr/include/drm
            Arch="risc-v"
            ;;
	esac
else
    read -p "Which system you would like to build, riscv, embedded or linux?" build_system
    TargetSystem="${build_system,,}"
    if [ "${TargetSystem}" == "linux" ]; then
        echo "Build DDK for Linux system..."
        #CROSS_COMPILE=
        CROSS_COMPILE=/usr/bin/
        #CROSS_COMPILE=/usr/bin/linux-
        DRM_INCLUDE_DIR=
        Arch="x86"
    elif [ "${TargetSystem}" == "embedded" ]; then
        echo "Build DDK for Embedded Linux system for ARM platform..."
        #CROSS_COMPILE=${TOP_FILE_PATH}/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-
        CROSS_COMPILE=${TOP_FILE_PATH}/buildroot/prebuilts/uclibc-4.7.3/bin/arm-buildroot-linux-uclibcgnueabihf-
        LINUX_KERNEL_BUILD_DIR=${TOP_FILE_PATH}/output/build/linux-local
        DRM_INCLUDE_DIR=
        Arch="arm"
    elif [ "${TargetSystem}" == "riscv" ]; then
        echo "Build DDK for Embedded Linux systemv for RISC-V platform..."
        CROSS_COMPILE=${TOP_FILE_PATH}/work/buildroot_initramfs/host/usr/bin/riscv64-buildroot-linux-gnu-
        LINUX_KERNEL_BUILD_DIR=${TOP_FILE_PATH}/work/linux
        DRM_INCLUDE_DIR=${TOP_FILE_PATH}/work/buildroot_initramfs/host/riscv64-buildroot-linux-gnu/sysroot/usr/include/drm
        Arch="risc-v"
    elif [ -z "${build_system}" ]; then
        echo "Default build DDK for Embedded Linux system for ARM platform..."
        #CROSS_COMPILE=${TOP_FILE_PATH}/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-
        CROSS_COMPILE=${TOP_FILE_PATH}/buildroot/prebuilts/uclibc-4.7.3/bin/arm-buildroot-linux-uclibcgnueabihf-
        LINUX_KERNEL_BUILD_DIR=${TOP_FILE_PATH}/output/build/linux-local
        DRM_INCLUDE_DIR=
        Arch="arm"
    else
        echo "Unknown system, quit build process!!!"
        exit 0
    fi
fi


if [ $# -gt 1 ]; then
	case "$2" in
		"y" | "Y" | "yes" | "Yes" | "YES" )
			#make clean;
			echo "Removing cmake and install folder files..."
			rm -rf cmake/* install/*
			#read -p "Press any key to continue..." ans
			;;
	esac
fi


#echo "===== Comment the STF_KERNEL_MODULE macro ====="
#sed -i "s/^#define STF_KERNEL_MODULE/\/\/#define STF_KERNEL_MODULE/g" DDKSource/common/stf_includes/stf_types.h
echo "===== Starting make ISPDDK... ====="
cd ${FILE_PATH}/cmake

##CROSS_COMPILE=${FILE_PATH}/../output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-
#CROSS_COMPILE=${FILE_PATH}/../buildroot/prebuilts/uclibc-4.7.3/bin/arm-buildroot-linux-uclibcgnueabihf-
#LINUX_KERNEL_BUILD_DIR=${FILE_PATH}/../output/build/linux-local

if [ "${Arch}" == "risc-v" ]; then
    CMAKE_TOOLCHAIN_FILE_PATH_NAME=${TOP_FILE_PATH}/work/buildroot_initramfs/host/usr/share/buildroot/toolchainfile.cmake
elif [ "${Arch}" == "arm" ]; then
    CMAKE_TOOLCHAIN_FILE_PATH_NAME=${TOP_FILE_PATH}/output/host/usr/share/buildroot/toolchainfile.cmake
fi

if  [ -z $LINUX_KERNEL_BUILD_DIR ] && [ -z $CROSS_COMPILE ]; then
#echo "No define LINUX_KERNEL_BUILD_DIR and CROSS_COMPILE"
    if [ "${Arch}" == "risc-v" ]; then
        #${TOP_FILE_PATH}/work/buildroot_initramfs/host/usr/bin/cmake ${FILE_PATH}/DDKSource \
        #    -DCI_LOG_LEVEL=4 \
        #    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
        #    -DARCH_RISCV="Y" \
        #    -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="RISCV" \
            -DDRM_INCLUDE_DIR=${DRM_INCLUDE_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DARCH_RISCV="Y" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    elif [ "${Arch}" == "arm" ]; then
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="ARM" \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    else
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="UBUNTU" \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    fi
elif [ -z $LINUX_KERNEL_BUILD_DIR ]; then
#echo "No define LINUX_KERNEL_BUILD_DIR"
    if [ "${Arch}" == "risc-v" ]; then
        #${TOP_FILE_PATH}/work/buildroot_initramfs/host/usr/bin/cmake ${FILE_PATH}/DDKSource \
        #    -DCROSS_COMPILE=$CROSS_COMPILE \
        #    -DCI_LOG_LEVEL=4 \
        #    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
        #    -DARCH_RISCV="Y" \
        #    -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="RISCV" \
            -DCROSS_COMPILE=$CROSS_COMPILE \
            -DDRM_INCLUDE_DIR=${DRM_INCLUDE_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DARCH_RISCV="Y" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    elif [ "${Arch}" == "arm" ]; then
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="ARM" \
            -DCROSS_COMPILE=$CROSS_COMPILE \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    else
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="UBUNTU" \
            -DCROSS_COMPILE=$CROSS_COMPILE \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    fi
elif [ -z $CROSS_COMPILE ]; then
#echo "No define CROSS_COMPILE"
    if [ "${Arch}" == "risc-v" ]; then
        #${TOP_FILE_PATH}/work/buildroot_initramfs/host/usr/bin/cmake ${FILE_PATH}/DDKSource \
        #    -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
        #    -DCI_LOG_LEVEL=4 \
        #    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
        #    -DARCH_RISCV="Y" \
        #    -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="RISCV" \
            -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
            -DDRM_INCLUDE_DIR=${DRM_INCLUDE_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DARCH_RISCV="Y" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    elif [ "${Arch}" == "arm" ]; then
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="ARM" \
            -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    else
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="UBUNTU" \
            -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    fi
else
#echo "Defined LINUX_KERNEL_BUILD_DIR and CROSS_COMPILE"
    if [ "${Arch}" == "risc-v" ]; then
        #${TOP_FILE_PATH}/work/buildroot_initramfs/host/usr/bin/cmake ${FILE_PATH}/DDKSource \
        #    -DCROSS_COMPILE=$CROSS_COMPILE \
        #    -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
        #    -DCI_LOG_LEVEL=4 \
        #    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
        #    -DARCH_RISCV="Y" \
        #    -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="RISCV" \
            -DCROSS_COMPILE=$CROSS_COMPILE \
            -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
            -DDRM_INCLUDE_DIR=${DRM_INCLUDE_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DARCH_RISCV="Y" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    elif [ "${Arch}" == "arm" ]; then
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="ARM" \
            -DCROSS_COMPILE=$CROSS_COMPILE \
            -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE_PATH_NAME}" \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    else
        cmake ${FILE_PATH}/DDKSource \
            -DRUN_PLATFORM="UBUNTU" \
            -DCROSS_COMPILE=$CROSS_COMPILE \
            -DLINUX_KERNEL_BUILD_DIR=${LINUX_KERNEL_BUILD_DIR} \
            -DCI_LOG_LEVEL=4 \
            -DCMAKE_INSTALL_PREFIX=${FILE_PATH}/install
    fi
fi

if [ "${Arch}" == "risc-v" ]; then
    export ARCH=riscv
elif [ "${Arch}" == "arm" ]; then
    export ARCH=arm
fi

#make -p -d
#make -p -d install
make 
make install

#${FILE_PATH}/makelib.sh ${FILE_PATH}
#${CROSS_COMPILE}strip -d ${FILE_PATH}/libmantis.a

cd ${FILE_PATH}
#cp ./install/km/Mantis.ko ../image/patch_q3_nand/ko/Mantis.ko
#cp ./install/km/Mantis.ko ../image/ramdisk/root/Mantis.ko
#cp install/ISPC/isp.bin ../image/ramdisk/root/
#cp ./install/ISPC/ISPC_loop ../image/patch_q3_nand/opt/ipnc/ISPC_loop

#echo "===== Un-comment the STF_KERNEL_MODULE macro ====="
#sed -i "s/^\/\/#define STF_KERNEL_MODULE/#define STF_KERNEL_MODULE/g" DDKSource/common/stf_includes/stf_types.h

