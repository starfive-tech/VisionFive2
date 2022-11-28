cmake_minimum_required(VERSION 2.8)

set(ISPSDK_VERSION 2.0.1)

if (DEFINED CROSS_COMPILE)
    message(STATUS "CROSS_COMPILE=${CROSS_COMPILE}")

    # specify the cross compiler
    set(CMAKE_C_COMPILER "${CROSS_COMPILE}gcc")
    set(CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")

    # could specify C and CXX flags here
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -fvisibility=hidden")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -fvisibility=hidden")

    # compiler flags should be modifiable in build/c_flag_overrides.cmake
    # and build/cxx_flag_overrides.cmake

    # search for lib and includes only in cross compile path
    set(CMAKE_FIND_ROOT_PATH "${CROSS_COMPILE}")

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

project (Mantis_DDK)

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

option(BUILD_RELEASE_VERSION "Build the ISP SDK as a release version" ON)
option(BUILD_KERNEL_DRIVER "Build the kernel driver (Mantis Device Driver)" OFF)
option(BUILD_CAPTUREINTERFACE "Build the Capture Interface (Mantis Hardware Driver)" ON)
option(BUILD_ISPCONTROL "Build the ISP Control" OFF)
option(BUILD_API "Build the ISP API" OFF)
option(BUILD_TEST_APPS "Build the Test applications" ON)
option(BUILD_STARFIVE_APPS "Build the StarFive applications" ON)
option(BUILD_SENSORAPI "Build the Mantis Sensor API" ON)
option(BUILD_MEDIA_CTRL "Build the media control API" OFF)
option(CI_MEM_DMA_ON "Build the CI memory machansim MEM DMA" OFF)
option(LOAD_TEST_DATA "Load test image or data into memory buffer" OFF)
option(ONE_OUT_MODULE "Merge UO, SS0 and SS1 into one out module architechure" OFF)
option(DISABLE_CSI_MODULES "Disable csi modules" OFF)
option(ENABLE_COLOR_BAR "Enable color bar module" OFF)
option(ENABLE_TIL_1_RD "Enable tiling 1 read module" OFF)
option(ENABLE_TIL_1_WR "Enable tiling 1 write module" OFF)
option(BUILD_HW_VIC_1_0 "Build for HW VIC 1.0" ON)
option(SENSOR_SC2235_DVP "Build SC2235 DVP sensor" ON)
option(SENSOR_OV4689_MIPI "Build OV4689 MIPI sensor" ON)
option(SENSOR_IMX219_MIPI "Build IMX219 MIPI sensor" ON)
option(BUILD_V4L2_DRIVER "Build for V4L2 driver" ON)
option(CONFIG_STF_DUAL_ISP "Build for V4L2 driver dual isp" OFF)
option(OLD_ISP_VIDEO_DEVICE_NO_ORDER "Build for V4L2 driver old video no order" OFF)

if (DEFINED ARCH_RISCV)
    if (${ARCH_RISCV} STREQUAL "Y")
        option(RISCV "Build for risc-v platform" ON)
    else()
        option(RISCV "Build for risc-v platform" OFF)
    endif()
else()
    option(RISCV "Build for risc-v platform" OFF)
endif()


option(DEBUG_MODULES "Display CMake debug information" ON)

if(NOT WIN32)
  option(ENABLE_GPROF "Enable gprof for gcc" OFF)
endif()
set(P4UTILS_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})


if (BUILD_V4L2_DRIVER)
    message(STATUS "Build for V4L2 driver")
else()
endif()

if (CONFIG_STF_DUAL_ISP)
    message(STATUS "Build for V4L2 driver dual isp")
else()
    message(STATUS "Build for V4L2 driver single isp")
endif()

if (OLD_ISP_VIDEO_DEVICE_NO_ORDER)
    message(STATUS "Build for V4L2 driver old video no order")
else()
    message(STATUS "Build for V4L2 driver new video no order")
endif()

if (BUILD_HW_VIC_1_0)
else()
    option(BUILD_HW_VIC_1_0 "Build for HW VIC 1.0" ON)
endif()

if (ENABLE_TIL_1_RD AND ENABLE_COLOR_BAR)
    #message(STATUS "ENABLE_COLOR_BAR is ON, and try to turn ENABLE_COLOR_BAR OFF")
    option(ENABLE_COLOR_BAR "Enable color bar module" OFF)
endif()

if (BUILD_HW_VIC_1_0)
    option(SUPPORT_LCBQ_MODULE "Enable LCBQ module architechure" ON)
    option(SUPPORT_DUMP_MODULE "Enable DUMP module architechure" ON)
    option(SUPPORT_OECFHM_MODULE "Enable OECFHM module architechure" ON)
    option(SUPPORT_CFA_AHD_MODULE "Enable CFA_AHD module architechure" ON)
    option(SUPPORT_CFA_GRAD_MODULE "Enable CFA_GRAD module architechure" OFF)
else ()
endif()

#if (SENSOR_SC2235_DVP AND SENSOR_OV4689_MIPI AND SENSOR_IMX219_MIPI)
#    option(SENSOR_SC2235_DVP "Disable SC2235 DVP sensor build" OFF)
#    option(SENSOR_OV4689_MIPI "Disable OV4689 MIPI sensor build" OFF)
#elseif (SENSOR_SC2235_DVP AND SENSOR_OV4689_MIPI)
#    option(SENSOR_OV4689_MIPI "Disable OV4689 MIPI sensor build" OFF)
#elseif (SENSOR_SC2235_DVP AND SENSOR_IMX219_MIPI)
#    option(SENSOR_SC2235_DVP "Disable SC2235 DVP sensor build" OFF)
#elseif (SENSOR_OV4689_MIPI AND SENSOR_IMX219_MIPI)
#    option(SENSOR_OV4689_MIPI "Disable OV4689 MIPI sensor build" OFF)
#elseif (SENSOR_SC2235_DVP)
#elseif (SENSOR_OV4689_MIPI)
#elseif (SENSOR_IMX219_MIPI)
#else()
#    option(SENSOR_IMX219_MIPI "Build IMX219 MIPI sensor" ON)
#endif()
#
if (SUPPORT_CFA_AHD_MODULE)
else()
    option(SUPPORT_CFA_AHD_MODULE "Enable CFA_AHD module architechure" ON)
endif()

if (SUPPORT_LCBQ_MODULE)
    message(STATUS "Build LCBQ module")
endif()

if (SUPPORT_DUMP_MODULE)
    message(STATUS "Build DUMP module")
endif()

if (SUPPORT_OECFHM_MODULE)
    message(STATUS "Build OECFHM module")
endif()

if (SUPPORT_CFA_AHD_MODULE)
    message(STATUS "Build CFA_AHD module")
endif()

if (SUPPORT_CFA_GRAD_MODULE)
    message(STATUS "Build CFA_GRAD module")
endif()

if (DISABLE_CSI_MODULES)
    message(STATUS "Disable csi modules")
endif()

if (ENABLE_COLOR_BAR)
    message(STATUS "Enable color bar module")
endif()

if (ENABLE_TIL_1_RD)
    message(STATUS "Enable tiling 1 read module")
endif()

if (ENABLE_TIL_1_WR)
    message(STATUS "Enable tiling 1 write module")
endif()

if (LOAD_TEST_DATA)
else()
    if ((ENABLE_TIL_1_RD) AND TEST_ON_UBUNTU)
        #message(STATUS "LOAD_TEST_DATA is OFF, and try to turn LOAD_TEST_DATA ON")
        option(LOAD_TEST_DATA "Load test image or data into memory buffer" ON)
    endif()
endif()

#set(CMAKE_BUILD_TYPE "RelWithDebInfo")
if (NOT DEFINED CMAKE_BUILD_TYPE)
  message("NOT DEFINED CMAKE_BUILD_TYPE")
  if (WIN32)
    set(CMAKE_BUILD_TYPE "Debug")
  else()
    #set(CMAKE_BUILD_TYPE "Debug")
    #set(CMAKE_BUILD_TYPE "RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "Release")
  endif()
else()
  message("DEFINED CMAKE_BUILD_TYPE")
endif()

message("CMake Build type: ${CMAKE_BUILD_TYPE}")

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/build
)

if (NOT DEFINED STFCOMMON_NDEBUG)
	if (CMAKE_BUILD_TYPE MATCHES "Release")
		set(STFCOMMON_NDEBUG ON)
	else()
		set(STFCOMMON_NDEBUG OFF)
	endif()
endif()
if (NOT DEFINED STFCOMMON_NPERF)
	if (CMAKE_BUILD_TYPE MATCHES "Release")
		set(STFCOMMON_NPERF ON)
	else()
		set(STFCOMMON_NPERF OFF)
	endif()
elseif (DEFINED ISPC_PERFLOG)
    # ensure that if ISPC_PERFLOG is on then STFCOMMON_NPERF is on too
    if (${ISPC_PERFLOG})
        if (${STFCOMMON_NPERF})
            message("Forcing STFCOMMON_NPERF to OFF because ISPC_PERFLOG=${ISPC_PERFLOG}")
            set(STFCOMMON_NPERF OFF)
        endif()
    endif()
endif()
if (DEBUG_MODULES)
	message("STFCOMMON_NDEBUG = ${STFCOMMON_NDEBUG}")
	message("STFCOMMON_NPERF = ${STFCOMMON_NPERF}")
endif()

#
# as GTEST is part of the build enable testing
#
enable_testing() # add_test can be used from now on

#
# tools are independent
# - either not compiled
# - or compiled with their own env
#
# tools are not given to the customer
#

message("CMAKE_CURRENT_SOURCE_DIR/tools = ${CMAKE_CURRENT_SOURCE_DIR}/tools")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools")
  set(TOOL_INSTALL_DIR ${Mantis_DDK_BINARY_DIR}/tools/installed)
  set(GENERATE_TOOL_DEF2CODE ${TOOL_INSTALL_DIR}/def2code)
  set(CMAKE_PROGRAM_PATH ${CMAKE_PROGRAM_PATH} ${TOOL_INSTALL_DIR})
  add_subdirectory(tools)
endif()

if (BUILD_RELEASE_VERSION)
    string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
    string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE})
    get_filename_component(CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE} NAME)
    get_filename_component(PARENT_FOLDER_ABSOLUTE ${CURRENT_FOLDER_ABSOLUTE} PATH)
    get_filename_component(PARENT_FOLDER ${PARENT_FOLDER_ABSOLUTE} NAME)
    get_filename_component(TOP_FOLDER_ABSOLUTE ${PARENT_FOLDER_ABSOLUTE} PATH)
    get_filename_component(TOPT_FOLDER ${TOP_FOLDER_ABSOLUTE} NAME)
    #message("===== Godspeed ===== CURRENT_FOLDER_ABSOLUTE = ${CURRENT_FOLDER_ABSOLUTE}")
    #message("===== Godspeed ===== CURRENT_FOLDER = ${CURRENT_FOLDER}")
    #message("===== Godspeed ===== PARENT_FOLDER_ABSOLUTE = ${PARENT_FOLDER_ABSOLUTE}")
    #message("===== Godspeed ===== PARENT_FOLDER = ${PARENT_FOLDER}")
    #message("===== Godspeed ===== TOP_FOLDER_ABSOLUTE = ${TOP_FOLDER_ABSOLUTE}")
    #message("===== Godspeed ===== TOPT_FOLDER = ${TOPT_FOLDER}")
    
    if (${CURRENT_FOLDER} STREQUAL "DDKSource")
        set(LIBISPSDK_PATH ${CURRENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${CURRENT_FOLDER} MATCHES "^ispsdk")
        set(LIBISPSDK_PATH ${CURRENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${CURRENT_FOLDER} MATCHES "^isp_release")
        set(LIBISPSDK_PATH ${CURRENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${CURRENT_FOLDER} MATCHES "^isp")
        set(LIBISPSDK_PATH ${CURRENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${PARENT_FOLDER} STREQUAL "DDKSource")
        set(LIBISPSDK_PATH ${PARENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${PARENT_FOLDER} MATCHES "^ispsdk")
        set(LIBISPSDK_PATH ${PARENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${PARENT_FOLDER} MATCHES "^isp_release")
        set(LIBISPSDK_PATH ${PARENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${PARENT_FOLDER} MATCHES "^isp")
        set(LIBISPSDK_PATH ${PARENT_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${TOPT_FOLDER} STREQUAL "DDKSource")
        set(LIBISPSDK_PATH ${TOP_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${TOPT_FOLDER} MATCHES "^ispsdk")
        set(LIBISPSDK_PATH ${TOP_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${TOPT_FOLDER} MATCHES "^isp_release")
        set(LIBISPSDK_PATH ${TOP_FOLDER_ABSOLUTE}/libispsdk)
    elseif (${TOPT_FOLDER} MATCHES "^isp")
        set(LIBISPSDK_PATH ${TOP_FOLDER_ABSOLUTE}/libispsdk)
    else()
        message(FATAL_ERROR "The libispsdk path is not found!!!")
    endif()

    if (CHECK_ISP_SDK_LIB_FILES)
        file(GLOB files ${LIBISPSDK_PATH}/*)
        foreach(file IN LISTS files)
            message(STATUS ${file})
        endforeach(file)
    endif()

    #=== Since the find_library does not work for the static library, so use add_library ===
    if (USE_FIND_LIBRARY_METHOD)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
        find_library(StfIncludesLib
            NAMES libStfIncludes.a StfIncludes.a StfIncludes
            PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (StfIncludesLib)
            #message(STATUS "Library (StfIncludesLib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (StfIncludesLib) not found")
        endif()
        find_library(StfCommonLib
            NAMES "libStfCommon.a"
            PATHS PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (StfCommonLib)
            #message(STATUS "Library (StfCommonLib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (StfCommonLib) not found")
        endif()
        find_library(LinkedListLib
            NAMES "libStfLib_LinkedList.a"
            PATHS PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (LinkedListLib)
            #message(STATUS "Library (LinkedListLib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (LinkedListLib) not found")
        endif()
        find_library(CI_UserLib
            NAMES "libCI_User.a"
            PATHS PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (CI_UserLib)
            #message(STATUS "Library (CI_UserLib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (CI_UserLib) not found")
        endif()
        find_library(SensorApiLib
            NAMES "libsensorapi.a"
            PATHS PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (SensorApiLib)
            #message(STATUS "Library (SensorApiLib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (SensorApiLib) not found")
        endif()
        find_library(MediaCtrlLib
            NAMES "libmedia-ctrl.a"
            PATHS PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (MediaCtrlLib)
            #message(STATUS "Library (MediaCtrlLib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (MediaCtrlLib) not found")
        endif()
        find_library(ISPCLib
            NAMES "libISPC.a"
            PATHS PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (ISPCLib)
            #message(STATUS "Library (ISPCLib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (ISPCLib) not found")
        endif()
        find_library(ISP_APILib
            NAMES "libISP_API.a"
            PATHS PATHS ${LIBISPSDK_PATH}
            NO_DEFAULT_PATH
        )
        if (ISP_APILib)
            #message(STATUS "Library (ISP_APILib) found in ${LIBISPSDK_PATH}")
        else()
            message(FATAL_ERROR "Library (ISP_APILib) not found")
        endif()
    else()
        #===== stf_includes =====
        add_library(StfIncludesLib STATIC IMPORTED GLOBAL)
        #set_property(TARGET StfIncludesLib PROPERTY IMPORTED_LOCATION ${STFINCLUDES_LIB_DIRS}/libStfIncludes.a)
        set_property(TARGET StfIncludesLib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libStfIncludes.a)
        #set_target_properties(StfIncludesLib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libStfIncludes.a)
        
        #===== stf_common =====
        add_library(StfCommonLib STATIC IMPORTED)
        #set_property(TARGET StfCommonLib PROPERTY IMPORTED_LOCATION ${STFCOMMON_LIB_DIRS}/libStfCommon.a)
        set_property(TARGET StfCommonLib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libStfCommon.a)
        #set_target_properties(StfCommonLib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libStfCommon.a)
        
        #===== linkedlist =====
        add_library(LinkedListLib STATIC IMPORTED)
        #set_property(TARGET LinkedListLib PROPERTY IMPORTED_LOCATION ${LINKEDLIST_LIB_DIRS}/libStfLib_LinkedList.a)
        set_property(TARGET LinkedListLib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libStfLib_LinkedList.a)
        #set_target_properties(LinkedListLib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libStfLib_LinkedList.a)
        
        #===== CI_User =====
        add_library(CI_UserLib STATIC IMPORTED)
        #set_property(TARGET CI_UserLib PROPERTY IMPORTED_LOCATION ${STFDRVAPI_LIB_DIRS}/libCI_User.a)
        set_property(TARGET CI_UserLib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libCI_User.a)
        #set_target_properties(CI_UserLib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libCI_User.a)
        
        #===== SensorApi =====
        #add_library(SensorApiLib STATIC IMPORTED)
        #set_property(TARGET SensorApiLib PROPERTY IMPORTED_LOCATION ${SENSORAPI_LIB_DIRS}/libsensorapi.a)
        #set_property(TARGET SensorApiLib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libsensorapi.a)
        #set_target_properties(SensorApiLib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libsensorapi.a)
    
        #===== MediaCtrl =====
        add_library(MediaCtrlLib STATIC IMPORTED)
        #set_property(TARGET MediaCtrlLib PROPERTY IMPORTED_LOCATION ${MEDIACTRLLIB_LIB_DIRS}/libmedia-ctrl.a)
        set_property(TARGET MediaCtrlLib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libmedia-ctrl.a)
        #set_target_properties(MediaCtrlLib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libmedia-ctrl.a)
        
        #===== ISPC =====
        add_library(ISPCLib STATIC IMPORTED)
        #set_property(TARGET ISPCLib PROPERTY IMPORTED_LOCATION ${SENSORAPI_LIB_DIRS}/libISPC.a)
        set_property(TARGET ISPCLib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libISPC.a)
        #set_target_properties(ISPCLib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libISPC.a)
    
        #===== ISP_API =====
        add_library(ISP_APILib STATIC IMPORTED)
        #set_property(TARGET ISP_APILib PROPERTY IMPORTED_LOCATION ${SENSORAPI_LIB_DIRS}/libISP_API.a)
        set_property(TARGET ISP_APILib PROPERTY IMPORTED_LOCATION ${LIBISPSDK_PATH}/libISP_API.a)
        #set_target_properties(ISP_APILib PROPERTIES IMPORTED_LOCATION ${LIBISPSDK_PATH}/libISP_API.a)
    endif()
endif ()

include (common/UsePackage.cmake)
if (BUILD_KERNEL_DRIVER)
else()
    option(CI_LOG_LEVEL "Disable CI_LOG_LEVEL definition" OFF)
endif()
if (BUILD_CAPTUREINTERFACE OR BUILD_KERNEL_DRIVER OR BUILD_RELEASE_VERSION) 
    include (CI/UsePackage.cmake)
endif()
if(BUILD_SENSORAPI OR BUILD_RELEASE_VERSION)
    include(sensorapi/UsePackage.cmake)
endif()
if(BUILD_MEDIA_CTRL OR BUILD_RELEASE_VERSION)
    include(Media_Control/UsePackage.cmake)
endif()
if (BUILD_ISPCONTROL OR BUILD_RELEASE_VERSION)
    include(ISP_Control/UsePackage.cmake)
endif()
if (BUILD_TEST_APPS)
    include(test_apps/UsePackage.cmake)
endif()
include (build/OverwriteFlags.cmake)
include (build/GenKernel.cmake)
include (build/RegdefFunctions.cmake)
include (build/PerforceUtils.cmake)

set(SAVE_LOCKS TRUE) # use pthread locks in sim_image common library

#
# the current directory can contain some cpack configuration
#
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cpack_sources.cmake")
    include(cpack_sources.cmake)
endif()

if (NOT BUILD_RELEASE_VERSION)
    add_subdirectory(common)
endif()

if (BUILD_CAPTUREINTERFACE)
    add_subdirectory(CI)
endif()

if (BUILD_SENSORAPI)
    if (NOT BUILD_CAPTUREINTERFACE)
        if (NOT BUILD_RELEASE_VERSION)
            message(FATAL_ERROR "ISP Control requires building the Capture Interface (CI) check BUILD_CAPTUREINTERFACE flag.")
        endif()
    else()
        add_subdirectory(sensorapi)
    endif()
endif()

if (BUILD_MEDIA_CTRL)
    add_subdirectory(Media_Control)
endif()

if (BUILD_ISPCONTROL OR BUILD_API)
    if (NOT BUILD_CAPTUREINTERFACE)
        if (NOT BUILD_RELEASE_VERSION)
            message(FATAL_ERROR "ISP Control requires building the Capture Interface (CI) check BUILD_CAPTUREINTERFACE flag.")
        endif()
    else()
        add_subdirectory(ISP_Control)
    endif()
endif()

if (BUILD_TEST_APPS)
    add_subdirectory(test_apps)
endif()

if (BUILD_STARFIVE_APPS)
    add_subdirectory(startfive_apps)
endif()

#install(FILES changelist.txt DESTINATION ./) 
