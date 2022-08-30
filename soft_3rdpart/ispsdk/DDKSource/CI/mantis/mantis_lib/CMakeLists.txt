cmake_minimum_required(VERSION 2.8)

project(MantisLib) # used in FindDG and in driver_test

message (STATUS "-- Capture Interface driver")

find_package(GTest QUIET) # optional for unit tests

#
# the internal part is only available to the StfDrvAPI
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${mantis_lib_SOURCE_DIR}/kernel/)

# user has to be first to define the location of the MantisUser project (to find includes needed for the kernel module)
if (BUILD_CAPTUREINTERFACE) 
    add_subdirectory(user)
endif()
if (BUILD_KERNEL_DRIVER) 
    add_subdirectory(kernel)
endif()
