cmake_minimum_required (VERSION 2.8)

# ----------------------------------------------------------------------
# Locate and configure the StfIncludes library.
#
# Defines the following variables:
#
#   STFINCLUDES_FOUND        - Found the library
#   STFINCLUDES_PREFIX       - The directory where this file is located
#   STFINCLUDES_INCLUDE_DIRS - The directories needed on the include paths
#                              for this library
#   STFINCLUDES_DEFINITIONS  - The definitions to apply for this library
#   STFINCLUDES_LIBRARIES    - The libraries to link to
#   STFINCLUDES_BUILD_KO     - The file to include to know which files are needed when building a kernel module
# ----------------------------------------------------------------------

get_filename_component(STFINCLUDES_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

#set (StfIncludes_VERSION "2")
#message ("version def=${StfIncludes_VERSION} pck_find=${PACKAGE_FIND_VERSION_MAJOR} pck=${PACKAGE_VERSION}")

if (NOT DEFINED STFINCLUDES_FORCE_WIN32)
    set (STFINCLUDES_FORCE_WIN32 FALSE)
endif()
if (NOT DEFINED STFINCLUDES_FORCE_C99)
    set (STFINCLUDES_FORCE_C99 FALSE)
endif()
if (NOT DEFINED STFINCLUDES_FORCE_KERNEL)
    set (STFINCLUDES_FORCE_KERNEL FALSE)
endif()
if (NOT DEFINED STFINCLUDES_EXIT_ON_ASSERT)
    set (STFINCLUDES_EXIT_ON_ASSERT TRUE)
endif()
if (NOT DEFINED STFINCLUDES_MALLOC_TEST)
    set (STFINCLUDES_MALLOC_TEST FALSE)
endif()
if (NOT DEFINED STFINCLUDES_MALLOC_CHECK)
    set (STFINCLUDES_MALLOC_CHECK FALSE)
endif()

set(STFINCLUDES_NAME StfIncludes)
set(STFINCLUDES_INCLUDE_DIRS ${STFINCLUDES_PREFIX})
set(STFINCLUDES_DEFINITIONS)
set(STFINCLUDES_LIBRARIES ${STFINCLUDES_NAME}) #Used for error to string function
set(STFINCLUDES_FOUND TRUE)
#set(STFINCLUDES_BUILD_KO ${STFINCLUDES_PREFIX}/StfIncludes_kernel.cmake)
set(STFINCLUDES_BUILD_KO ${STFINCLUDES_PREFIX}/${STFINCLUDES_NAME}_kernel.cmake)

if (STFINCLUDES_EXIT_ON_ASSERT)
    set (STFINCLUDES_DEFINITIONS ${STFINCLUDES_DEFINITIONS} -DEXIT_ON_ASSERT)
endif()
if (STFINCLUDES_MALLOC_TEST)
	if (DEBUG_MODULES)
		message ("STF Includes uses STF_MALLOC_TEST")
	endif()
	set (STFINCLUDES_DEFINITIONS ${STFINCLUDES_DEFINITIONS} -DSTF_MALLOC_TEST)
endif()
if (STFINCLUDES_MALLOC_CHECK)
	if (DEBUG_MODULES)
		message ("STF Includes uses STF_MALLOC_CHECK")
	endif()
	set (STFINCLUDES_DEFINITIONS ${STFINCLUDES_DEFINITIONS} -DSTF_MALLOC_CHECK)
endif()

if (WIN32 OR STFINCLUDES_FORCE_WIN32)

	set (STFINCLUDES_INCLUDE_DIRS ${STFINCLUDES_INCLUDE_DIRS}
		${STFINCLUDES_PREFIX}/ms
	)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux"         OR
       ${CMAKE_SYSTEM_NAME} MATCHES "Linux-Android" OR
       ${CMAKE_SYSTEM_NAME} MATCHES "Darwin"        OR
       ${CMAKE_SYSTEM_NAME} MATCHES "MTX"           OR
       STFINCLUDES_FORCE_C99                        OR
       STFINCLUDES_FORCE_KERNEL)
    if (NOT STFINCLUDES_FORCE_KERNEL)
        set (STFINCLUDES_INCLUDE_DIRS ${STFINCLUDES_INCLUDE_DIRS}
            ${STFINCLUDES_PREFIX}/c99
        )
    else() # kernel
        set (STFINCLUDES_INCLUDE_DIRS ${STFINCLUDES_INCLUDE_DIRS}
            ${STFINCLUDES_PREFIX}/linux-kernel
        )
    endif()
else ( )
    message(FATAL_ERROR "Target system '${CMAKE_SYSTEM_NAME}' not suported by StfIncludes!")
endif()

if (STFINCLUDES_OLD_VARS)
    set (STFINC_INCLUDE_DIRS ${STFINCLUDES_INCLUDE_DIRS})
    set (STFINC_DEFINITIONS ${STFINCLUDES_DEFINITIONS})
    set (STFINC_FOUND TRUE)
    mark_as_advanced(STFINC_INCLUDE_DIRS STFINC_DEFINITIONS)
    message("STF includes: STFINCLDUES_OLD_VARS should not be used")
endif()

if (DEBUG_MODULES)
    message ("STFINCLUDES_INCLUDE_DIRS=${STFINCLUDES_INCLUDE_DIRS}")
    message ("STFINCLUDES_DEFINITIONS=${STFINCLUDES_DEFINITIONS}")
endif()

mark_as_advanced(STFINCLUDES_INCLUDE_DIRS STFINCLUDES_DEFINITIONS STFINCLUDES_LIBRARIES)
