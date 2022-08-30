#
# this file defines the source and include files needed if a kernel module
# as to be build using this library
#
# module_KO_SOURCES - sources files to add to the kernel module
# module_KO_INCLUDE_DIRS - external headers path that could be needed
# module_KO_DEFINITIONS
#

set (STFCOMMON_KO_INCLUDE_DIRS ${STFCOMMON_INCLUDE_DIRS})
set (STFCOMMON_KO_DEFINITIONS)

set (STFCOMMON_KO_SOURCES ${STFCOMMON_PREFIX}/source/ci_alloc_info.c)
