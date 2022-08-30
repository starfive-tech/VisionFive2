#!/bin/bash


TOP_FILE_PATH=$(cd "$(dirname "$0")/../.."; pwd)
PARENT_FILE_PATH=$(cd "$(dirname "$0")/.."; pwd)
FILE_PATH=$(cd "$(dirname "$0")"; pwd)


TargetSystem="${1,,}"   # Convert parameter 1 to lower case
#TargetSystem="${1^^}"   # Convert parameter 1 to upper case


#echo "Copying image.fit, Mantis.ko and ISP_test to ShareTemp/.../TestOnFPGA folder..."
if [ $# -gt 0 ]; then
    case "$TargetSystem" in
        "fpga-linux" )
            echo "Copy uboot, Linux image and ISP files for FPGA Linux system..."
            cp ${TOP_FILE_PATH}/work/HiFive_U-Boot/u-boot /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            #cp ${TOP_FILE_PATH}/work/HiFive_U-Boot/u-boot.bin /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive/u-boot.bin
            cp ${TOP_FILE_PATH}/work/HiFive_U-Boot/u-boot.bin.out /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ${TOP_FILE_PATH}/work/image.fit /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive/image.fit
            cp ./install/km/stf_camss.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov5640.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov4689_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_sc2235.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_imx219_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/StarFiveServer /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_ctrl /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./DDKSource/ISP_Control/ISP_api/test_data/only_uo.cfg /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
           ;;
        "evb-bare")
            echo "Copy uboot, Linux image and ISP files for EVB Linux system..."
            cp ${TOP_FILE_PATH}/work/HiFive_U-Boot/u-boot /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            #cp ${TOP_FILE_PATH}/work/HiFive_U-Boot/u-boot.bin /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ${TOP_FILE_PATH}/work/HiFive_U-Boot/u-boot.bin.out /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ${TOP_FILE_PATH}/work/opensbi/platform/starfive/vic7100/firmware/fw_payload.bin.out /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ${TOP_FILE_PATH}/work/image.fit /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive/image.fit
            cp ./install/km/stf_camss.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov5640.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov4689_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_sc2235.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_imx219_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/StarFiveServer /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_ctrl /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./DDKSource/ISP_Control/ISP_api/test_data/only_uo.cfg /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            ;;
        "evb")
            echo "Copy files for EVB Linux system with uboot ready..."
            cp ${TOP_FILE_PATH}/work/image.fit /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive/image.fit
            cp ./install/km/stf_camss.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov5640.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov4689_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_sc2235.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_imx219_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_Demo /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/StarFiveServer /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_ctrl /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./DDKSource/ISP_Control/ISP_api/test_data/only_uo.cfg /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            ;;
        "isp")
            echo "Copy ISP files for EVB Linux system..."
            cp ./install/km/stf_camss.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov5640.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov4689_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_sc2235.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_imx219_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_Demo /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/StarFiveServer /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_ctrl /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./DDKSource/ISP_Control/ISP_api/test_data/only_uo.cfg /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            ;;
        "uboot")
            echo "Copy uboot files..."
            cp ${TOP_FILE_PATH}/work/HiFive_U-Boot/u-boot /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ${TOP_FILE_PATH}/work/opensbi/platform/starfive/vic7100/firmware/fw_payload.bin.out /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            ;;
        *)
            echo "Copy files for EVB Linux system with uboot ready..."
            #cp ${TOP_FILE_PATH}/work/image.fit /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive/hifiveu.fit
            cp ${TOP_FILE_PATH}/work/image.fit /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive/image.fit
            cp ./install/km/stf_camss.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov5640.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_ov4689_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_sc2235.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/km/stf_imx219_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/ISP_Demo /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/StarFiveServer /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./install/ISP/stf_isp_ctrl /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
            cp ./DDKSource/ISP_Control/ISP_api/test_data/only_uo.cfg /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
	esac
else
    cp ${TOP_FILE_PATH}/work/image.fit /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive/hifiveu.fit
    cp ./install/km/stf_camss.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/km/stf_ov5640.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/km/stf_ov4689_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/km/stf_sc2235.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/km/stf_imx219_mipi.ko /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/ISP/ISP_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/ISP/ISP_Demo /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/ISP/StarFiveServer /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/ISP/stf_isp_test /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./install/ISP/stf_isp_ctrl /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
    cp ./DDKSource/ISP_Control/ISP_api/test_data/only_uo.cfg /mnt/hgfs/ShareTemp/Unleash_FPGA/TestOnFPGA/VisionFive
fi


