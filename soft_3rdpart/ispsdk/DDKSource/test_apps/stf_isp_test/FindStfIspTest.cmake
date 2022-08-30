cmake_minimum_required (VERSION 2.8)

get_filename_component (STF_ISP_TEST_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

find_package(StfIncludes REQUIRED) # needed in the headers - but should be included by application
find_package(StfDrvAPI REQUIRED) # needed in the headers
find_package(ISPC REQUIRED) # needed in the headers
#find_package(SensorApi REQUIRED) # needed in the headers

set (STF_ISP_TEST_FOUND TRUE)

set (STF_ISP_TEST_INCLUDE_DIRS 
    ${STFDRVAPI_INCLUDE_DIRS}
    ${ISPC_INCLUDE_DIRS}
#    ${SENSORAPI_INCLUDE_DIRS}
    ${STF_ISP_TEST_PREFIX}/include
)

set (STF_ISP_TEST_DEFINITIONS 
    ${STFDRVAPI_DEFINITIONS}
    ${ISPC_DEFINITIONS}
#    ${SENSORAPI_DEFINITIONS}
)
set (STF_ISP_TEST_DOXYGEN_SRC ${STF_ISP_TEST}/src)
set (STF_ISP_TEST_LIBRARIES STF_ISP_TEST)
