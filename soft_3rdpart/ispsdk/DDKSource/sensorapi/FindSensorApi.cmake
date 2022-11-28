cmake_minimum_required (VERSION 2.8)

get_filename_component (SENSORAPI_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

#
# needs the MantisLib project to exists before called!
#

#find_package(StfIncludes REQUIRED) # should be found by the application
find_package(StfCommon REQUIRED)

#if (NOT DEFINED SENSOR_SC2235_DVP)
#  set(SENSOR_SC2235_DVP TRUE)
#endif()
#
#if (NOT DEFINED SENSOR_OV4689_MIPI)
#  set(SENSOR_OV4689_MIPI TRUE)
#endif()
#
#if (NOT DEFINED SENSOR_IMX219_MIPI)
#  set(SENSOR_IMX219_MIPI TRUE)
#endif()
#

set (SENSORAPI_FOUND TRUE)
set (SENSORAPI_INCLUDE_DIRS ${SENSORAPI_PREFIX}/include ${STFCOMMON_INCLUDE_DIRS})
set (SENSORAPI_DOXYGEN_SRC ${SENSORAPI_PREFIX}/src ${SENSORAPI_PREFIX}/src/sensors)
set (SENSORAPI_DEFINITIONS)
set (SENSORAPI_NAME SensorApi)
set (SENSORAPI_LIBRARIES ${SENSORAPI_NAME})