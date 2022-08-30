
#
# needs the MantisLib project to exists before called!
#

get_filename_component(STFDRVAPI_ROOT "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(STFDRVAPI_PREFIX ${STFDRVAPI_ROOT}/user)

find_package(RegDefs REQUIRED) # regdefs/mantis_config_pack.h
find_package(StfCommon REQUIRED)

set(STFDRVAPI_FOUND TRUE)
set(STFDRVAPI_INCLUDE_DIRS 
  ${STFDRVAPI_PREFIX}/include 
  ${MantisLib_BINARY_DIR}/user/include # used when ci_version.h is generated
  ${REGDEF_INCLUDE_DIRS}
  ${STFCOMMON_INCLUDE_DIRS}
)
set(STFDRVAPI_DEFINITIONS
	${STFCOMMON_DEFINITIONS}
)
set(STFDRVAPI_NAME CI) # used internally
set(STFDRVAPI_LIBRARIES ${STFDRVAPI_NAME}_User)
set(STFDRVAPI_DEPENDENCIES ${REGDEF_NAME}) # for generated files
set(STFDRVAPI_INSTALL) # files to copy to the run folder - on windows dlls

if (DEBUG_MODULES)
  message("STFDRVAPI_ROOT=${STFDRVAPI_ROOT}")
  message ("STFDRVAPI_INCLUDE_DIRS=${STFDRVAPI_INCLUDE_DIRS}")
  message ("STFDRVAPI_DEFINITIONS=${STFDRVAPI_DEFINITIONS}")
  message ("STFDRVAPI_LIBRARIES=${STFDRVAPI_LIBRARIES}")
endif()

mark_as_advanced(STFDRVAPI_ROOT STFDRVAPI_INCLUDE_DIRS STFDRVAPI_DEFINITIONS STFDRVAPI_NAME STFDRVAPI_LIBRARIES)
