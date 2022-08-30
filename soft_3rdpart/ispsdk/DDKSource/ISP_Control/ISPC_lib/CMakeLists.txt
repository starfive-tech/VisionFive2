cmake_minimum_required(VERSION 2.8)

project(ISPC_LIBRARIES)

message("=== Godspeed === ${CMAKE_CURRENT_LIST_FILE} CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
find_package(StfIncludes REQUIRED)
find_package(StfCommon REQUIRED) # LOG
find_package(StfDrvAPI REQUIRED)
find_package(ISPC REQUIRED)
find_package(LinkedList REQUIRED)
find_package(MediaCtrlLib REQUIRED)
find_package(GTest QUIET) # optional

#find_package(SensorApi REQUIRED) # found in FindISPC.cmake

message (STATUS "-- ISPC lib")

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

if (NOT DEFINED ISPC_PERFLOG)
    set(ISPC_PERFLOG FALSE)
endif()
message(STATUS "--    PERFLOG ${ISPC_PERFLOG}")

include_directories(
    ${STFDRVAPI_INCLUDE_DIRS}
    ${STFINCLUDES_INCLUDE_DIRS}
    ${STFCOMMON_INCLUDE_DIRS}
    ${ISPC_INCLUDE_DIRS}
    ${SENSORAPI_INCLUDE_DIRS}
    ${LINKEDLIST_INCLUDE_DIRS}
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
#    -DVERBOSE_CONTROL_MODULES=1
#    -DISPC_PARAM_FOUND_VERBOSE=1
    )

if (${ISPC_PERFLOG})
    # warning: may not work in android
    add_definitions(-DISPC_HAS_LOGPERF)
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

if (ENABLE_COLOR_BAR)
    add_definitions(
        -DENABLE_COLOR_BAR
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
    message(STATUS "Build the user library for Ubuntu")
    add_definitions(
        -DTEST_ON_UBUNTU
    )
else()
    message(STATUS "Build the user library for RISCV or ARM")
endif()

if (SUPPORT_LCBQ_MODULE)
    set(MOD_LCBQ_MODULE_HEADERS
        include/Modules/stfmod_isp_lcbq.h
    )
else()
    set(MOD_LCBQ_MODULE_HEADERS
    )
endif()

if (SUPPORT_DUMP_MODULE)
    set(MOD_DUMP_MODULE_HEADERS
        include/Modules/stfmod_isp_dump.h
    )
else()
    set(MOD_DUMP_MODULE_HEADERS
    )
endif()

if (SUPPORT_OECFHM_MODULE)
    set(MOD_OECFHM_MODULE_HEADERS
        include/Modules/stfmod_isp_oecfhm.h
    )
else()
    set(MOD_OECFHM_MODULE_HEADERS
    )
endif()

if (SUPPORT_CFA_AHD_MODULE)
    set(MOD_CFA_MODULE_HEADERS
        include/Modules/stfmod_isp_cfa_ahd.h
    )
else()
    set(MOD_CFA_MODULE_HEADERS
    )
endif()

if (ONE_OUT_MODULE)
    message(STATUS "Build UO, SS0 and SS1 into one out module")
    set(MOD_OUT_MODULE_HEADERS
        include/Modules/stfmod_isp_out.h
    )
else()
    message(STATUS "Build UO, SS0 and SS1 as different module")
    set(MOD_OUT_MODULE_HEADERS
        include/Modules/stfmod_isp_out.h
        include/Modules/stfmod_isp_out_ss0.h
        include/Modules/stfmod_isp_out_ss1.h
        include/Modules/stfmod_isp_out_uo.h
    )
endif()

set(ISPC_MODULES_H
    include/Modules/stfmod_isp_sud_csi.h
    include/Modules/stfmod_isp_sud_isp.h
    include/Modules/stfmod_isp_csi_in.h
    include/Modules/stfmod_isp_cbar.h
    include/Modules/stfmod_isp_crop.h
    include/Modules/stfmod_isp_dc.h
    include/Modules/stfmod_isp_dec.h
    include/Modules/stfmod_isp_oba.h
    include/Modules/stfmod_isp_obc.h
    ${MOD_LCBQ_MODULE_HEADERS}
    include/Modules/stfmod_isp_sc.h
    ${MOD_DUMP_MODULE_HEADERS}
    include/Modules/stfmod_isp_isp_in.h
    include/Modules/stfmod_isp_dbc.h
    include/Modules/stfmod_isp_ctc.h
    include/Modules/stfmod_isp_oecf.h
    ${MOD_OECFHM_MODULE_HEADERS}
    include/Modules/stfmod_isp_lccf.h
    include/Modules/stfmod_isp_awb.h
    ${MOD_CFA_MODULE_HEADERS}
    include/Modules/stfmod_isp_car.h
    include/Modules/stfmod_isp_ccm.h
    include/Modules/stfmod_isp_gmargb.h
    include/Modules/stfmod_isp_r2y.h
    include/Modules/stfmod_isp_yhist.h
    include/Modules/stfmod_isp_ycrv.h
    include/Modules/stfmod_isp_shrp.h
    include/Modules/stfmod_isp_dnyuv.h
    include/Modules/stfmod_isp_sat.h
    ${MOD_OUT_MODULE_HEADERS}
    include/Modules/stfmod_isp_til.h
    include/Modules/stfmod_isp_til_1_rd.h
    include/Modules/stfmod_isp_til_1_wr.h
    include/Modules/stfmod_isp_buf.h
)

set(ISPC_AUXILIAR_H
)

set(ISPC_CONTROL_H
    include/Controls/stfctl_isp_ae.h
    include/Controls/stfctl_isp_awb.h
    include/Controls/stfctl_isp_lccf.h
    include/Controls/stfctl_isp_ccm.h
    include/Controls/stfctl_isp_ycrv.h
    include/Controls/stfctl_isp_shrp.h
    include/Controls/stfctl_isp_dnyuv.h
    include/Controls/stfctl_isp_sat.h
)
  
set(ISPC_H
    include/ISPC/stflib_isp_cfg.h
    include/ISPC/stflib_isp_base.h
    include/ISPC/stflib_isp_defs.h
    include/ISPC/stflib_isp_device.h
    include/ISPC/stflib_isp_pipeline.h
    include/ISPC/stflib_isp_sensor.h
    include/ISPC/stflib_isp_shot.h
#    include/ISPC/stflib_isp_cd_system.h
    include/ISPC/stflib_mod_config.h

    ${ISPC_MODULES_H}
    ${ISPC_AUXILIAR_H}
    ${ISPC_CONTROL_H}
)

add_library(${ISPC_LIBRARIES} ${ISPC_C} ${ISPC_H})

# Enable assembly code output.
set_target_properties(${ISPC_LIBRARIES} PROPERTIES COMPILE_FLAGS "-save-temps") 

source_group(Modules_H FILES ${ISPC_MODULES_H})

source_group(Auxiliar_H FILES ${ISPC_AUXILIAR_H})

source_group(Control_H FILES ${ISPC_CONTROL_H})
