cmake_minimum_required (VERSION 2.8)

get_filename_component (ISPC_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

#find_package(StfIncludes REQUIRED) # needed in the headers - but should be included by application
find_package(StfDrvAPI REQUIRED) # needed in the headers
find_package(SensorApi REQUIRED) # needed in the headers

set (ISPC_FOUND TRUE)

set (ISPC_INCLUDE_DIRS 
    ${STFDRVAPI_INCLUDE_DIRS}
    ${SENSORAPI_INCLUDE_DIRS}
    ${ISPC_PREFIX}/include
    ${ISPC_PREFIX}/include/Modules
#    ${ISPC_PREFIX}/include/Auxiliar
    ${ISPC_PREFIX}/include/Controls
    ${ISPC_PREFIX}/include/ISPC
)

set (ISPC_DEFINITIONS 
    ${STFDRVAPI_DEFINITIONS}
    ${SENSORAPI_DEFINITIONS}
)
set (ISPC_DOXYGEN_SRC ${ISPC_PREFIX}/src)
set (ISPC_LIBRARIES ISPC)
