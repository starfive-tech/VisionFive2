cmake_minimum_required(VERSION 2.8)

project(STF_ISP_CTRL)

#get_filename_component (STF_ISP_CTRL_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
#
find_package(StfIncludes REQUIRED)
find_package(StfCommon REQUIRED) # LOG
find_package(LinkedList REQUIRED)
find_package(StfDrvAPI REQUIRED)
find_package(MediaCtrlLib REQUIRED)
find_package(ISPC REQUIRED)
find_package(Stf_Isp_Ctrl REQUIRED)

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
    ${STFDRVAPI_INCLUDE_DIRS}
    ${MEDIACTRLLIB_INCLUDE_DIRS}
    ${ISPC_INCLUDE_DIRS}
    ${STF_ISP_CTRL_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIR}
)

add_definitions(
    ${STFINCLUDES_DEFINITIONS}
    ${LINKEDLIST_DEFINITIONS}
    ${STFDRVAPI_DEFINITIONS}
    ${ISPC_DEFINITIONS}
    ${STF_ISP_CTRL_DEFINITIONS}
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
    message(STATUS "Build the user mode demo application for Ubuntu")
    add_definitions(
        -DTEST_ON_UBUNTU
    )
else()
    message(STATUS "Build the user mode demo application for RISCV or ARM")
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

if (SUPPORT_CPD_MODULE)
    add_definitions(
        -DSUPPORT_CPD_MODULE
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

if (SUPPORT_STNR_MODULE)
    add_definitions(
        -DSUPPORT_STNR_MODULE
    )
endif()

if (SUPPORT_OECFHM_MODULE)
    add_definitions(
        -DSUPPORT_OECFHM_MODULE
    )
endif()

if (SUPPORT_LS_MODULE)
    add_definitions(
        -DSUPPORT_LS_MODULE
    )
endif()

if (SUPPORT_CA_MODULE)
    add_definitions(
        -DSUPPORT_CA_MODULE
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

if (SUPPORT_LUT_MODULE)
    add_definitions(
        -DSUPPORT_LUT_MODULE
    )
endif()

if (SUPPORT_WDR_MODULE)
    add_definitions(
        -DSUPPORT_WDR_MODULE
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
        -lrt -lm -lpthread -lv4l2 -ljpeg -ldrm
    )
else()
    set(LINK_LIBS
        ${STFINCLUDES_LIBRARIES}
        ${STFCOMMON_LIBRARIES}
        ${LINKEDLIST_LIBRARIES}
        ${STFDRVAPI_LIBRARIES}
        ${SENSORAPI_LIBRARIES}
        ${MEDIACTRLLIB_LIBRARIES}
        ${ISPC_LIBRARIES} 
        -lrt -lm -lpthread -lv4l2 -ljpeg
    )
endif()

set(STF_ISP_CTRL_C
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stf_isp_ctrl.c
)


set(STF_ISP_CTRL_H
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stf_isp_ctrl.h
)


if (NOT WIN32)
#==============================================================================
    #set(CMAKE_BUILD_TYPE "RelWithDebInfo")
    message(STATUS "Build stf_isp_ctrl")
    message(STATUS "Build stf_isp_ctrl --- CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
#------------------------------------------------------------------------------
    # Enable mapfile output.
    add_custom_command(
        OUTPUT "output.map"
        COMMAND "${CMAKE_COMMAND}" -E touch "mapfile.map"
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(
            CMAKE_EXE_LINKER_FLAGS 
            "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=mapfile.map"
        )
    endif()

    add_executable(
        stf_isp_ctrl
        ${STF_ISP_CTRL_H}
        ${STF_ISP_CTRL_C}
        )
    # Enable assembly code output.
    #set_target_properties(stf_isp_ctrl PROPERTIES COMPILE_FLAGS "-save-temps" LINK_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/mapfile.map") 
    set_target_properties(stf_isp_ctrl PROPERTIES COMPILE_FLAGS "-save-temps") 
    target_link_libraries(stf_isp_ctrl ${LINK_LIBS})
#------------------------------------------------------------------------------
    install(TARGETS stf_isp_ctrl DESTINATION ./ISP)
endif()

if (DEFINED STF_ISP_CTRL_INSTALL)
    file(COPY ${STF_ISP_CTRL_INSTALL} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    install(FILES ${STF_ISP_CTRL_INSTALL} DESTINATION ./ISP/)
endif()
