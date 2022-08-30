cmake_minimum_required(VERSION 2.8)

project(CAPTURE_INTERFACE)

# windows C and CXX flags for shared library must be set up before reaching that project
   
## Set this to avoid zlib to be installed in the system
set (SKIP_INSTALL_ALL true)

#
# project options are included when using mantis/UsePackage.cmake
#

message(STATUS "== Capture Interface ==")

# ----------------------------------------------------------------------
# Component Libraries in CMake Path
# ----------------------------------------------------------------------
include(mantis/defaultarch.cmake)
message(STATUS "CI_HW_ARCH=${CI_HW_ARCH}")
if (${CI_HW_ARCH} EQUAL 3)
	message("")
	message("HW ARCH 3 IS EXPERIMENTAL - IT MAY NOT COMPILE AND WILL NOT WORK YET")
	message("")
endif()
include(mantis/UsePackage.cmake)

PerforceFound(P4_FOUND)
if (${P4_FOUND})
	PerforceChangelist(${CMAKE_CURRENT_SOURCE_DIR} CI_CHANGELIST)
	set(CI_CHANGELIST_STR "${CI_CHANGELIST}")
	PerforceUpdateDate(${CMAKE_CURRENT_SOURCE_DIR} CI_DATE_STR)
else()
	set(CI_CHANGELIST "-1")
	set(CI_CHANGELIST_STR "unknown-changelist")
	set(CI_DATE_STR "unkown-date")
endif()

add_subdirectory(mantis)
