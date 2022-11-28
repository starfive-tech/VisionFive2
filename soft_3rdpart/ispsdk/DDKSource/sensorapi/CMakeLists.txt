cmake_minimum_required (VERSION 2.8)

project(SENSORAPI)

message(STATUS "== Sensor API ==")

find_package(StfIncludes REQUIRED)
find_package(StfDrvAPI REQUIRED) # StfDrvAPI used to acquire the gasket
find_package(StfCommon REQUIRED) # pixel formats

find_package(SensorApi REQUIRED) # find itself for config
find_package(GTest QUIET) # optional

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

if(NOT DEFINED SENSORAPI_UNIT_TESTS)
  set(SENSORAPI_UNIT_TESTS FALSE)
endif()

include_directories(
  ${STFINCLUDES_INCLUDE_DIRS}
  ${STFDRVAPI_INCLUDE_DIRS}
  ${STFCOMMON_INCLUDE_DIRS}
  ${SENSORAPI_INCLUDE_DIRS}
  )

add_definitions(
  ${STFINCLUDES_DEFINITIONS}
  ${STFDRVAPI_DEFINITIONS}
  ${MANTISOMMON_DEFINITIONS}
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

if (BUILD_V4L2_DRIVER)
    add_definitions(
        -DV4L2_DRIVER
    )
endif()

if (CONFIG_STF_DUAL_ISP)
    add_definitions(
        -DCONFIG_STF_DUAL_ISP
    )
endif()

if (OLD_ISP_VIDEO_DEVICE_NO_ORDER)
    add_definitions(
        -DOLD_ISP_VIDEO_DEVICE_NO_ORDER
    )
endif()

if (SUPPORT_LCBQ_MODULE)
    add_definitions(
        -DSUPPORT_LCBQ_MODULE
    )
endif()

if (SUPPORT_DUMP_MODULE)
    add_definitions(
        -DSUPPORT_DUMP_MODULE
    )
endif()

if (SUPPORT_OECFHM_MODULE)
    add_definitions(
        -DSUPPORT_OECFHM_MODULE
    )
endif()

if (SUPPORT_CFA_AHD_MODULE)
    add_definitions(
        -DSUPPORT_CFA_AHD_MODULE
    )
elseif (SUPPORT_CFA_GRAD_MODULE)
    add_definitions(
        -DSUPPORT_CFA_GRAD_MODULE
    )
endif()

if (ONE_OUT_MODULE)
    add_definitions(
        -DONE_OUT_MODULE
    )
endif()

if (SENSOR_SC2235_DVP)
    add_definitions(
        -DSENSOR_SC2235_DVP
    )
endif()

if (SENSOR_OV4689_MIPI)
    add_definitions(
        -DSENSOR_OV4689_MIPI
    )
endif()

if (SENSOR_IMX219_MIPI)
    add_definitions(
        -DSENSOR_IMX219_MIPI
    )
endif()

if (RISCV)
    add_definitions(
        -DRISCV
    )
endif()

if(NOT CI_BUILD_KERNEL_MODULE)
    add_definitions(-DMANTIS_FAKE)
endif()

set(SENSORAPI_H
    include/sensorapi/sensor_phy.h
    include/sensorapi/sensorapi.h
)

set(SENSORAPI_C
    src/sensorapi/sensor_phy.c
    src/sensorapi/sensorapi.c
)

if (SENSOR_SC2235_DVP)
    message(STATUS "add sensor SC2235 DVP")
    set(SENSORS_H ${SENSORS_H} include/sensors/sc2235dvp.h)
    set(SENSORS_C ${SENSORS_C} src/sensors/sc2235dvp.c)
endif()

if (SENSOR_OV4689_MIPI)
    message(STATUS "add sensor OV4689 MIPI")
    set(SENSORS_H ${SENSORS_H} include/sensors/ov4689mipi.h)
    set(SENSORS_C ${SENSORS_C} src/sensors/ov4689mipi.c)
endif()

if (SENSOR_IMX219_MIPI)
    message(STATUS "add sensor IMX219 MIPI")
    set(SENSORS_H ${SENSORS_H} include/sensors/imx219mipi.h)
    set(SENSORS_C ${SENSORS_C} src/sensors/imx219mipi.c)
endif()

add_library(${SENSORAPI_NAME} ${SENSORAPI_H} ${SENSORS_H} ${SENSORAPI_C} ${SENSORS_C})
add_dependencies(${SENSORAPI_NAME} ${STFDRVAPI_DEPENDENCIES})
source_group(Sensors_H FILES ${SENSORS_H})
source_group(Sensors_C FILES ${SENSORS_C})