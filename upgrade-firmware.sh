#!/bin/bash

os_release="$(cat /etc/os-release)"

date_now="$(date +%Y%m%d%H%M%S%Z)"
releases_page="visionfive2_releases.${date_now}.html"
spl_file="u-boot-spl.bin.normal.${date_now}.out"
u_boot_file="visionfive2_fw_payload.${date_now}.img"
vf2_repo_url="https://github.com/starfive-tech/VisionFive2"
fw_release_url="${vf2_repo_url}/releases"
fw_url="${fw_release_url}/download"

echo "Download releases page......"
wget -nv -O ${releases_page} ${fw_release_url}
latest_version=$( \
    grep -oP 'VF2_v\d+.\d+.\d+' ${releases_page} | \
    head -1 \
)

upgrade_through_debian12(){

    devices=$(ls /dev)

    if [[ $devices == *"mmcblk0p1"* ]];
    then
        echo "'/dev/mmcblk0p1'(eMMC) exists, updating......"
        sudo dd if=${spl_file} of=/dev/mmcblk0p1 conv=fsync 
    fi

    if [[ $devices == *"mmcblk1p1"* ]];
    then
        echo "'/dev/mmcblk1p1'(SD card) exists, updating......"
        sudo dd if=${spl_file} of=/dev/mmcblk1p1 conv=fsync 
    fi

    if [[ $devices == *"mmcblk0p2"* ]];
    then
        echo "'/dev/mmcblk0p2'(eMMC) exists, updating......"
        sudo dd if=${u_boot_file} of=/dev/mmcblk0p2 conv=fsync 
    fi
    if [[ $devices == *"mmcblk1p2"* ]];
    then
        echo "'/dev/mmcblk1p2'(SD card) exists, updating......"
        sudo dd if=${u_boot_file} of=/dev/mmcblk1p2 conv=fsync 
    fi

    echo "Update SPL & U-Boot of Flash......"
    sudo flashcp -v ${spl_file}    /dev/mtd0
    sudo flashcp -v ${u_boot_file} /dev/mtd1

}

read \
    -p "The latest version is ${latest_version}, upgrade to it?([Y]y/Nn)" \
    upgrade_yn

if [[ $upgrade_yn =~ ^[Yy]$ ]] || [[ $upgrade_yn == "" ]] ;
then

    echo "Download SPL file......"
    wget \
        -nv \
        -O ${spl_file} \
        ${fw_url}/${latest_version}/u-boot-spl.bin.normal.out

    echo "Download U-Boot file......"
    wget \
        -nv \
        -O ${u_boot_file} \
        ${fw_url}/${latest_version}/visionfive2_fw_payload.img

    echo ""
    ls -hl ${spl_file} ${u_boot_file}
    echo ""

    echo "${spl_file} and ${u_boot_file} have already been downloaded."
    read \
        -p "Upgrade now?(Yy/[N]n,irrevocable.)" \
        upgrade_yn0

    if [[ $upgrade_yn0 =~ ^[Yy]$ ]] ;
    then
        # Debian 12
        if [[ $os_release == *"Debian GNU/Linux 12 (bookworm)"* ]];
        then
            echo "Debian 12 was detected, upgrading......"
            upgrade_through_debian12
        # Other distros will be added by you. (^_^)
        else
            echo "Unable to update the firmware on your distro."
            echo "Feel free to contribute your code on:"
            echo -e "\t${vf2_repo_url}/blob/JH7110_VisionFive2_devel/upgrade-firmware.sh"
        fi
    fi
fi

read \
    -p "Do you wish to delete downloaded files?([Y]y/Nn)" \
    del_yn

if [[ $del_yn =~ ^[Yy]$ ]] || [[ $del_yn == "" ]] ;
then
    echo "Remove ${spl_file}, ${u_boot_file}, ${releases_page}......"
    rm -rf ${spl_file} ${u_boot_file} ${releases_page}
fi


