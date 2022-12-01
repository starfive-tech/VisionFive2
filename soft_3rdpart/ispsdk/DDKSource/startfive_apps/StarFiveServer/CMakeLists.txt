cmake_minimum_required(VERSION 2.8)

message(STATUS "== build StarFiveServer ==")

find_package(StfIncludes REQUIRED)
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
    ${ISPC_INCLUDE_DIRS}
    ${ISP_API_INCLUDE_DIRS}
    )

add_definitions(
    ${STFINCLUDES_DEFINITIONS}
    ${ISPC_DEFINITIONS}
    ${ISP_API_DEFINITIONS}
    -D__LINUX__ -Wno-unused-variable -pthread
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
    message(STATUS "Build the user mode starfive tuning server application for Ubuntu")
    add_definitions(
        -DTEST_ON_UBUNTU
    )
else()
    message(STATUS "Build the user mode starfive tuning server application for RISCV or ARM")
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

# use CMAKE_CURRENT_SOURCE_DIR because also used in unit tests
set(ISPSERV_H
    ${CMAKE_CURRENT_SOURCE_DIR}/src/broadcast_main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/comm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ispc_main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtp_enc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtsp_demo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtsp_msg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtsp_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tinycthread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning_base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning_main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning_service.h
)

set(ISPSERV_C
    ${CMAKE_CURRENT_SOURCE_DIR}/src/broadcast_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ispc_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtp_enc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtsp_demo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtsp_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtsp_msg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtsp_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tinycthread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning_base.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning_service.c
)

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
        ${ISPC_LIBRARIES}
        ${ISP_API_LIBRARIES}
        -lrt -lm -lpthread -ldrm
    )
endif()

if (NOT WIN32)
    message(STATUS "Build StarFiveServer")

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

    #add_executable(StarFiveServer ${ISPSERV_C} ${ISPSERV_H} "mapfile.map")
    add_executable(StarFiveServer ${ISPSERV_C} ${ISPSERV_H})

    # Enable assembly code output.
    #set_target_properties(StarFiveServer PROPERTIES COMPILE_FLAGS "-save-temps" LINK_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/mapfile.map") 
    set_target_properties(StarFiveServer PROPERTIES COMPILE_FLAGS "-save-temps") 

    target_link_libraries(StarFiveServer ${LINK_LIBS})

    install(TARGETS StarFiveServer DESTINATION ./ISP)
    install(FILES ${ISP_API_PREFIX}/test_data/StarFiveServer.cfg DESTINATION ./ISP/)
    install(FILES ${ISP_API_PREFIX}/test_data/stf_isp_api.cfg DESTINATION ./ISP/)
    install(FILES ${ISP_API_PREFIX}/test_data/IspSetting_sc2235dvp_1920x1080.ybn DESTINATION ./ISP/)
    install(FILES ${ISP_API_PREFIX}/test_data/IspSetting_imx219mipi_1920x1080.ybn DESTINATION ./ISP/)
    install(FILES ${ISP_API_PREFIX}/test_data/IspSetting_imx219mipi_1280x720.ybn DESTINATION ./ISP/)
    install(FILES ${ISP_API_PREFIX}/test_data/IspSetting_imx219mipi_640x480.ybn DESTINATION ./ISP/)
    install(FILES ${ISP_API_PREFIX}/test_data/IspSetting_ov4689mipi_1920x1080.ybn DESTINATION ./ISP/)
endif()
