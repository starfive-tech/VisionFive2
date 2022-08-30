cmake_minimum_required(VERSION 2.8)

project(test_apps)

message(STATUS "== Build test Apps ==")

add_subdirectory(ISP_test)
#add_subdirectory(ISP_Demo)
add_subdirectory(stf_isp_ctrl)
add_subdirectory(stf_isp_test)

