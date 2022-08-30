cmake_minimum_required(VERSION 2.8)

project(MANTIS)

#
# these libraries have no scope higher than here therefore they are not in UsePackage.cmake
#

#
# define the kernel module location (not used when building FAKE interface)
#
if (NOT DEFINED ISP_KM_BINARY_DIR)
  set (ISP_KM_BINARY_DIR ${MANTIS_BINARY_DIR}/mantis_lib/km)
endif()

if (NOT DEFINED ISP_KM_OV5640_BINARY_DIR)
  set (ISP_KM_OV5640_BINARY_DIR ${MANTIS_BINARY_DIR}/mantis_lib/km_ov5640)
endif()

if (NOT DEFINED ISP_KM_OV4689_MIPI_BINARY_DIR)
  set (ISP_KM_OV4689_MIPI_BINARY_DIR ${MANTIS_BINARY_DIR}/mantis_lib/km_ov4689_mipi)
endif()

if (NOT DEFINED ISP_KM_SC2235_BINARY_DIR)
  set (ISP_KM_SC2235_BINARY_DIR ${MANTIS_BINARY_DIR}/mantis_lib/km_sc2235)
endif()

if (NOT DEFINED ISP_KM_IMX219_MIPI_BINARY_DIR)
  set (ISP_KM_IMX219_MIPI_BINARY_DIR ${MANTIS_BINARY_DIR}/mantis_lib/km_imx219_mipi)
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/regdefs${CI_HW_ARCH}")
    add_subdirectory(regdefs${CI_HW_ARCH})
else()
    message(FATAL_ERROR "Unable to find HW Architecture ${CI_HW_ARCH} register definitions")
endif()

add_subdirectory(mantis_lib)
# system is not a library

