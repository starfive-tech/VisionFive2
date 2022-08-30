cmake_minimum_required(VERSION 2.8)

set(STFINCLUDES_FORCE_KERNEL FALSE) # this part is not in a kernel module - make sure of it
find_package(StfIncludes REQUIRED)
find_package(StfDrvAPI REQUIRED) # mantis internal finds mantis API
#finds RegDefs
find_package(RegDefs REQUIRED)
find_package(LinkedList REQUIRED)
#find_package(StfCommon REQUIRED) # done when finding StfDrvAPI

if (DEFINED RUN_PLATFORM)
    if ("x${RUN_PLATFORM}" STREQUAL "xRISCV")
#        #message(STATUS "Building the cd_system for RISCV")
#        option(BUILD_CD_SYSTEM "Build the cd_system into kernel driver" ON)
        #message(STATUS "Build the kernel driver for RISCV")
        option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" OFF)
    elseif ("x${RUN_PLATFORM}" STREQUAL "xARM")
#        #message(STATUS "Building the cd_system for ARM")
#        option(BUILD_CD_SYSTEM "Build the fack cd_system into kernel driver" OFF)
        #message(STATUS "Build the kernel driver for ARM")
        option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" OFF)
    else()
#        #message(STATUS "Building the cd_system for Ubuntu")
#        option(BUILD_CD_SYSTEM "Build the fack cd_system into kernel driver" OFF)
        #message(STATUS "Build the kernel driver for Ubuntu")
        option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" ON)
    endif()
else()
#    #message(STATUS "Building cd_system for Ubuntu")
#    option(BUILD_CD_SYSTEM "Build the fack cd_system into kernel driver" OFF)
    #message(STATUS "Build the kernel driver for Ubuntu")
    option(TEST_ON_UBUNTU "Build the kernel driver for Ubuntu" ON)
endif()

#if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ci_version.h.cmakein)
#  # if the cmake template exists the ci_version is generated
#  set(VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/include/ci/ci_version.h)
#  configure_file(ci_version.h.cmakein ${VERSION_HEADER}) # uses CI_CHANGELIST and CI_DATE
#endif()
#
include_directories(
    ${STFINCLUDES_INCLUDE_DIRS}
    ${STFDRVAPI_INCLUDE_DIRS}
    ${MantisLib_SOURCE_DIR}/kernel/include # include dirs is needed for the ioctl
    ${LINKEDLIST_INCLUDE_DIRS}
    ${STFCOMMON_INCLUDE_DIRS}
    ${REGDEF_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIR}          #include for device libary support DRM device.
  )

add_definitions(
    ${STFINCLUDES_DEFINITIONS}
    ${LINKEDLIST_DEFINITIONS}
    ${STFDRVAPI_DEFINITIONS}
    ${STFCOMMON_DEFINITIONS}
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

if (RISCV)
    add_definitions(
        -DRISCV
    )
endif()

if (TEST_ON_UBUNTU)
    message(STATUS "Build the user driver library for Ubuntu")
    add_definitions(
        -DTEST_ON_UBUNTU
    )
else()
    message(STATUS "Build the user driver library for RISCV or ARM")
endif()

if (SUPPORT_LCBQ_MODULE)
    set(MOD_LCBQ_MODULE_HEADERS
        include/ci/driver/stfdrv_isp_lcbq.h
    )
else()
    set(MOD_LCBQ_MODULE_HEADERS
    )
endif()

if (SUPPORT_DUMP_MODULE)
    set(MOD_DUMP_MODULE_HEADERS
        include/ci/driver/stfdrv_isp_dump.h
    )
else()
    set(MOD_DUMP_MODULE_HEADERS
    )
endif()

if (SUPPORT_OECFHM_MODULE)
    set(MOD_OECFHM_MODULE_HEADERS
        include/ci/driver/stfdrv_isp_oecfhm.h
    )
else()
    set(MOD_OECFHM_MODULE_HEADERS
    )
endif()

if (SUPPORT_CFA_AHD_MODULE)
    set(MOD_CFA_MODULE_HEADERS
        include/ci/driver/stfdrv_isp_cfa_ahd.h
    )
else()
    set(MOD_CFA_MODULE_HEADERS
    )
endif()

set(MC_HEADERS
    include/mc/module_config.h
    include/mc/mc_structs.h
  )

set(V4L_HEADERS
    include/ci/driver/stfdrv_v4l_base.h
    )

set(HEADERS
    include/ci/ci_api.h
    include/ci/ci_api_structs.h
    include/ci/ci_api_internal.h
    include/ci/stf_isp_ioctrl.h
    include/ci/ci_modules_structs.h

    include/ci/driver/stfdrv_gen_base.h

    include/ci/driver/stfdrv_isp_base.h
    include/ci/driver/stfdrv_isp_sud_csi.h
    include/ci/driver/stfdrv_isp_sud_isp.h
    include/ci/driver/stfdrv_isp_csi_in.h
    include/ci/driver/stfdrv_isp_cbar.h
    include/ci/driver/stfdrv_isp_crop.h
    include/ci/driver/stfdrv_isp_dc.h
    include/ci/driver/stfdrv_isp_dec.h
    include/ci/driver/stfdrv_isp_oba.h
    include/ci/driver/stfdrv_isp_obc.h
    ${MOD_LCBQ_MODULE_HEADERS}
    include/ci/driver/stfdrv_isp_sc.h
    ${MOD_DUMP_MODULE_HEADERS}
    include/ci/driver/stfdrv_isp_isp_in.h
    include/ci/driver/stfdrv_isp_dbc.h
    include/ci/driver/stfdrv_isp_ctc.h
    include/ci/driver/stfdrv_isp_oecf.h
    ${MOD_OECFHM_MODULE_HEADERS}
    include/ci/driver/stfdrv_isp_lccf.h
    include/ci/driver/stfdrv_isp_awb.h
    ${MOD_CFA_MODULE_HEADERS}
    include/ci/driver/stfdrv_isp_car.h
    include/ci/driver/stfdrv_isp_ccm.h
    include/ci/driver/stfdrv_isp_gmargb.h
    include/ci/driver/stfdrv_isp_r2y.h
    include/ci/driver/stfdrv_isp_yhist.h
    include/ci/driver/stfdrv_isp_ycrv.h
    include/ci/driver/stfdrv_isp_shrp.h
    include/ci/driver/stfdrv_isp_dnyuv.h
    include/ci/driver/stfdrv_isp_sat.h
    include/ci/driver/stfdrv_isp_out.h
    include/ci/driver/stfdrv_isp_til.h
    include/ci/driver/stfdrv_isp_buf.h

#    include/ci/cd_system/ci_cd_system.h
#  
#    include/ci/driver/sivdrv_cd_system.h
#
    ${MC_HEADERS}

    ${V4L_HEADERS}

    include/sys/sys_userio.h
  )

source_group(MC_H FILES ${MC_HEADERS})

