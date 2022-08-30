cmake_minimum_required (VERSION 2.8)

project(MEDIACTRLLIB)

message(STATUS "== Media Control Library ==")

find_package(MediaCtrlLib REQUIRED) # find itself for config

if (DEFINED RUN_PLATFORM)
    if ("x${RUN_PLATFORM}" STREQUAL "xRISCV")
        #message(STATUS "Building the cd_system for RISCV")
        option(BUILD_CD_SYSTEM "Build the cd_system into kernel driver" ON)
        #message(STATUS "Build the kernel driver for RISCV")
        option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" OFF)
    elseif ("x${RUN_PLATFORM}" STREQUAL "xARM")
        #message(STATUS "Building the cd_system for ARM")
        option(BUILD_CD_SYSTEM "Build the fack cd_system into kernel driver" OFF)
        #message(STATUS "Build the kernel driver for ARM")
        option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" OFF)
    else()
        #message(STATUS "Building the cd_system for Ubuntu")
        option(BUILD_CD_SYSTEM "Build the fack cd_system into kernel driver" OFF)
        #message(STATUS "Build the kernel driver for Ubuntu")
        option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" ON)
    endif()
else()
    #message(STATUS "Building cd_system for Ubuntu")
    option(BUILD_CD_SYSTEM "Build the fack cd_system into kernel driver" OFF)
    #message(STATUS "Build the kernel driver for Ubuntu")
    option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" ON)
endif()

if(WIN32 OR NOT CI_BUILD_KERNEL_MODULE)
  add_definitions(-Dfile_i2c=1) # needs to be implemented
endif()

include_directories(
  ${MEDIACTRLLIB_INCLUDE_DIRS}
  )

add_definitions(
  )

if (BUILD_HW_VIC_1_0)
    add_definitions(
        -DSUPPORT_VIC_1_0
    )
elseif (BUILD_HW_VIC_2_0)
    add_definitions(
        -DSUPPORT_VIC_2_0
    )
else()
endif()

if (TEST_ON_UBUNTU)
    message(STATUS "Build the user mode sensor library for Ubuntu")
    add_definitions(
        -DTEST_ON_UBUNTU
    )
else()
    message(STATUS "Build the user mode sensor library for RISCV or ARM")
endif()

if (RISCV)
    add_definitions(
        -DRISCV
    )
endif()

set(MEDIACTRLLIB_H
    include/config.h
    include/media-bus-format-codes.h
    include/media-bus-format-names.h
    include/mediactl.h
    include/mediactl-priv.h
    include/options.h
    include/tools.h
    include/v4l2subdev.h
    )
