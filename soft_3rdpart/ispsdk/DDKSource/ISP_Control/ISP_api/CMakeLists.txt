cmake_minimum_required(VERSION 2.8)

project(ISP_API)

message("=== Godspeed === ${CMAKE_CURRENT_LIST_FILE} CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
find_package(StfIncludes REQUIRED)
find_package(StfCommon REQUIRED) # LOG
find_package(StfDrvAPI REQUIRED)
find_package(ISPC REQUIRED)
find_package(LinkedList REQUIRED)
find_package(ISP_API REQUIRED)
find_package(MediaCtrlLib REQUIRED)
find_package(GTest QUIET) # optional

#find_package(SensorApi REQUIRED) # found in FindISPC.cmake

message (STATUS "-- ISP API")

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

if (NOT DEFINED ISP_API_PERFLOG)
    set(ISP_API_PERFLOG FALSE)
endif()
message(STATUS "--    PERFLOG ${ISP_API_PERFLOG}")

include_directories(
    ${STFDRVAPI_INCLUDE_DIRS}
    ${STFINCLUDES_INCLUDE_DIRS}
    ${STFCOMMON_INCLUDE_DIRS}
    ${ISPC_INCLUDE_DIRS}
    ${SENSORAPI_INCLUDE_DIRS}
    ${LINKEDLIST_INCLUDE_DIRS}
    ${ISP_API_INCLUDE_DIRS}
    ${MEDIACTRLLIB_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIR}          #include for device libary support DRM device.
    )

add_definitions(
    ${STFDRVAPI_DEFINITIONS}
    ${STFINCLUDES_DEFINITIONS}
    ${STFCOMMON_DEFINITIONS}
    ${ISPC_DEFINITIONS}
    ${SENSORAPI_DEFINITIONS}
    ${LINKEDLIST_DEFINITIONS}
    ${ISP_API_DEFINITIONS}
    )

if (${ISP_API_PERFLOG})
    # warning: may not work in android
    add_definitions(-DISP_API_HAS_LOGPERF)
endif()

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

if (DISABLE_CSI_MODULES)
    add_definitions(
        -DDISABLE_CSI_MODULES
    )
endif()

if (ENABLE_COLOR_BAR)
    add_definitions(
        -DENABLE_COLOR_BAR
    )
endif()

if (ENABLE_TIL_1_RD)
    add_definitions(
        -DENABLE_TIL_1_RD
    )
endif()

if (ENABLE_TIL_1_WR)
    add_definitions(
        -DENABLE_TIL_1_WR
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

if (TEST_ON_UBUNTU)
    message(STATUS "Build the user API library for Ubuntu")
    add_definitions(
        -DTEST_ON_UBUNTU
    )
else()
    message(STATUS "Build the user API library for RISCV or ARM")
endif()

if (LOAD_TEST_DATA)
    add_definitions(
        -DLOAD_TEST_DATA
    )
endif()

# use CMAKE_CURRENT_SOURCE_DIR because also used in unit tests
set(ISP_API_H
    include/stf_isp_api.h
    include/convert.h
)

add_library(${ISP_API_LIBRARIES} ${ISP_API_C} ${ISP_API_H})

# Enable assembly code output.
set_target_properties(${ISP_API_LIBRARIES} PROPERTIES COMPILE_FLAGS "-save-temps") 

if (LOAD_TEST_DATA)
    #install(FILES test_data/TestImage1_1920x1080_8bit_NV12.yuv DESTINATION ./ISP)
    #install(FILES test_data/TestImage2_1920x1080_8bit_NV12.yuv DESTINATION ./ISP)
    #install(FILES test_data/TestImage3_1920x1080_8bit_NV12.yuv DESTINATION ./ISP)
    #install(FILES test_data/TestImage4_1920x1080_8bit_NV12.yuv DESTINATION ./ISP)
    #install(FILES test_data/TestImage5_1536x864_8bit_NV12.yuv DESTINATION ./ISP)
    #install(FILES test_data/TestImage6_1280x720_8bit_NV12.yuv DESTINATION ./ISP)
    #install(FILES test_data/TestImage7_1920x1080_8bit_444.yuv DESTINATION ./ISP)
    #install(FILES test_data/TestImage8_1280x720_8bit_444.yuv DESTINATION ./ISP)
    #install(FILES test_data/f000-sc_dump.bin DESTINATION ./ISP)
    install(DIRECTORY test_data/ DESTINATION ./ISP/data)
endif()
