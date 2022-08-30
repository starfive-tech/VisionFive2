cmake_minimum_required(VERSION 2.8)

#
# output:
# LINKEDLIST_PREFIX - top level folder of the library
# LINKEDLIST_INCLUDE_DIRS - include path
# LINKEDLIST_DEFINITIONS - preprocessor macros
# LINKEDLIST_NAME - library name (internal use)
# LINKEDLIST_LIBRARIES - libraries to link against to have the link list
# LINKEDLIST_BUILD_KO - cmake file to include when kernel module is built
#
# compilation option:
# LINKEDLIST_TESTS (TRUE/FALSE) default FALSE - if Gtest is found build or not the unit tests
#

get_filename_component(LINKEDLIST_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

set (LINKEDLIST_FOUND TRUE)

set (LINKEDLIST_INCLUDE_DIRS ${LINKEDLIST_PREFIX}/include)
set (LINKEDLIST_DEFINITIONS)
set (LINKEDLIST_DOXYGEN_SRC ${LINKEDLIST_PREFIX}/code)

set (LINKEDLIST_NAME StfLib_LinkedList)
set (LINKEDLIST_LIBRARIES ${LINKEDLIST_NAME})
set (LINKEDLIST_BUILD_KO ${LINKEDLIST_PREFIX}/LinkedList_kernel.cmake)

mark_as_advanced(LINKEDLIST_INCLUDE_DIRS LINKEDLIST_DEFINITIONS LINKEDLIST_NAME LINKEDLIST_LIBRARIES)
