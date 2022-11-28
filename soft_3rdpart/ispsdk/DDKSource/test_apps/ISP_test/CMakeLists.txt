cmake_minimum_required(VERSION 2.8)

get_filename_component (ISP_TEST_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

find_package(StfIncludes REQUIRED)
find_package(LinkedList REQUIRED)
find_package(ISPC REQUIRED)
find_package(ISP_API REQUIRED)

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

include_directories(
    ${STFINCLUDES_INCLUDE_DIRS}
    ${LINKEDLIST_INCLUDE_DIRS}
    ${ISPC_INCLUDE_DIRS}
    ${ISP_API_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIR}
    ${ISP_TEST_PREFIX}/src
    ${ISP_TEST_PREFIX}/src/test
)

add_definitions(
    ${STFINCLUDES_DEFINITIONS}
    ${LINKEDLIST_DEFINITIONS}
    ${ISPC_DEFINITIONS}
    ${ISP_API_DEFINITIONS}
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
    message(STATUS "Build the user mode test application for Ubuntu")
    add_definitions(
        -DTEST_ON_UBUNTU
    )
else()
    message(STATUS "Build the user mode test application for RISCV or ARM")
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

if (BUILD_RELEASE_VERSION)
    set(LINK_LIBS
        -Wl,--whole-archive
        StfIncludesLib
        StfCommonLib
        LinkedListLib
        CI_UserLib
        SensorApi
        MediaCtrlLib
        ISPCLib
        ISP_APILib
        -Wl,--no-whole-archive
        -lrt -lm -lpthread -lv4l2 -ldrm
    )
else()
    set(LINK_LIBS
        ${ISPC_LIBRARIES}
        ${ISP_API_LIBRARIES}
        -lrt -lm -lpthread
    )
endif()

if (SUPPORT_LCBQ_MODULE)
    set(MOD_LCBQ_MODULE_SOURCES
        src/test/ISP_mod_lcbq_test.c
    )
else()
    set(MOD_LCBQ_MODULE_SOURCES
    )
endif()

if (SUPPORT_DUMP_MODULE)
    set(MOD_DUMP_MODULE_SOURCES
        src/test/ISP_mod_dump_test.c
    )
else()
    set(MOD_DUMP_MODULE_SOURCES
    )
endif()

if (SUPPORT_OECFHM_MODULE)
    set(MOD_OECFHM_MODULE_SOURCES
        src/test/ISP_mod_oecfhm_test.c
    )
else()
    set(MOD_OECFHM_MODULE_SOURCES
    )
endif()

if (SUPPORT_CFA_AHD_MODULE)
    set(MOD_CFA_MODULE_SOURCES
        src/test/ISP_mod_cfa_ahd_test.c
    )
else()
    set(MOD_CFA_MODULE_SOURCES
    )
endif()

if (NOT WIN32)
#==============================================================================
    message(STATUS "Build ISP_test")
#------------------------------------------------------------------------------
    add_executable(
        ISP_test
        src/ISP_test.c
        src/test/ISP_reg_test.c
        src/test/ISP_mem_test.c
        src/test/ISP_isp_base_test.c
        src/test/ISP_mod_sud_test.c
        src/test/ISP_mod_csi_in_test.c
        src/test/ISP_mod_cbar_test.c
        src/test/ISP_mod_crop_test.c
        src/test/ISP_mod_dc_test.c
        src/test/ISP_mod_dec_test.c
        src/test/ISP_mod_oba_test.c
        src/test/ISP_mod_obc_test.c
        ${MOD_LCBQ_MODULE_SOURCES}
        src/test/ISP_mod_sc_test.c
        ${MOD_DUMP_MODULE_SOURCES}
        src/test/ISP_mod_isp_in_test.c
        src/test/ISP_mod_dbc_test.c
        src/test/ISP_mod_ctc_test.c
        src/test/ISP_mod_oecf_test.c
        ${MOD_OECFHM_MODULE_SOURCES}
        src/test/ISP_mod_lccf_test.c
        src/test/ISP_mod_awb_test.c
        ${MOD_CFA_MODULE_SOURCES}
        src/test/ISP_mod_car_test.c
        src/test/ISP_mod_ccm_test.c
        src/test/ISP_mod_gmargb_test.c
        src/test/ISP_mod_r2y_test.c
        src/test/ISP_mod_yhist_test.c
        src/test/ISP_mod_ycrv_test.c
        src/test/ISP_mod_shrp_test.c
        src/test/ISP_mod_dnyuv_test.c
        src/test/ISP_mod_sat_test.c
        src/test/ISP_mod_out_test.c
        src/test/ISP_mod_til_test.c
        src/test/ISP_mod_buf_test.c
        )
    target_link_libraries(ISP_test ${LINK_LIBS})
#------------------------------------------------------------------------------
    install(TARGETS ISP_test DESTINATION ./ISP)
endif()

if (DEFINED STFDRVAPI_INSTALL)
    file(COPY ${STFDRVAPI_INSTALL} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    install(FILES ${STFDRVAPI_INSTALL} DESTINATION ./ISP/)
endif()
