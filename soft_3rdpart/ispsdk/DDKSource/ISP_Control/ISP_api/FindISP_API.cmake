cmake_minimum_required (VERSION 2.8)

get_filename_component (ISP_API_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

find_package(StfIncludes REQUIRED) # needed in the headers - but should be included by application
find_package(StfDrvAPI REQUIRED) # needed in the headers
find_package(ISPC REQUIRED) # needed in the headers
#find_package(SensorApi REQUIRED) # needed in the headers

set (ISP_API_FOUND TRUE)

set (ISP_API_INCLUDE_DIRS 
    ${STFDRVAPI_INCLUDE_DIRS}
    ${ISPC_INCLUDE_DIRS}
    ${SENSORAPI_INCLUDE_DIRS}
    ${ISP_API_PREFIX}/include
)

set (ISP_API_DEFINITIONS 
    ${STFDRVAPI_DEFINITIONS}
    ${ISPC_DEFINITIONS}
    ${SENSORAPI_DEFINITIONS}
)
set (ISP_API_DOXYGEN_SRC ${ISP_API}/src)
set (ISP_API_LIBRARIES ISP_API)
