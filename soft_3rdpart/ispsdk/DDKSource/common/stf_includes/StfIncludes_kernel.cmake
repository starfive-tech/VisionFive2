#
# this file defines the source and include files needed if a kernel module
# as to be build using this library
#
# module_KO_SOURCES - sources files to add to the kernel module
# module_KO_INCLUDE_DIRS - external headers path that could be needed
# module_KO_DEFINITIONS
#
find_package(StfIncludes REQUIRED)

# defines the needed headers so that we don't need to copy the whole folder into the kernel module

set (STFINCLUDES_KO_SOURCES
    ${STFINCLUDES_PREFIX}/stf_defs.h
    ${STFINCLUDES_PREFIX}/stf_errors.h
    ${STFINCLUDES_PREFIX}/stf_include.h
    ${STFINCLUDES_PREFIX}/stf_types.h
    # linux kernel specific one
    ${STFINCLUDES_PREFIX}/linux-kernel/stf_sysdefs.h
    ${STFINCLUDES_PREFIX}/linux-kernel/stf_systypes.h

    ${STFINCLUDES_PREFIX}/stf_isp.h
)

set (STFINCLUDES_KO_INCLUDE_DIRS) # empty

set (STFINCLUDES_KO_DEFINITIONS ${STFINCLUDES_DEFINITIONS})
