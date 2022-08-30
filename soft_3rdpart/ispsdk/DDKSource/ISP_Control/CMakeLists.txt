cmake_minimum_required(VERSION 2.8)

project(ISPCONTROL)

message(STATUS "== ISP Control and API ==")

if (BUILD_ISPCONTROL)
    add_subdirectory(ISPC_lib)
endif()
if (BUILD_API)
    add_subdirectory(ISP_api)
endif()
