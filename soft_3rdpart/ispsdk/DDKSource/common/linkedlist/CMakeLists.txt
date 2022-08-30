cmake_minimum_required(VERSION 2.8)

project (LinkedList)

message (STATUS "-- Template Linked List")

find_package(StfIncludes REQUIRED)
find_package(LinkedList REQUIRED)

include_directories(${STFINCLUDES_INCLUDE_DIRS} ${LINKEDLIST_INCLUDE_DIRS})
add_definitions(${STFINCLUDES_DEFINITIONS} ${LINKEDLIST_DEFINITIONS})

if (BUILD_HW_VIC_1_0)
    add_definitions(
        -DSUPPORT_VIC_1_0
    )
elseif (BUILD_HW_VIC_2_0)
    add_definitions(
        -DSUPPORT_VIC_2_0
    )
else()
endif()

if (BUILD_V4L2_DRIVER)
    add_definitions(
        -DV4L2_DRIVER
    )
endif()

if (CONFIG_STF_DUAL_ISP)
    add_definitions(
        -DCONFIG_STF_DUAL_ISP
    )
endif()

if (OLD_ISP_VIDEO_DEVICE_NO_ORDER)
    add_definitions(
        -DOLD_ISP_VIDEO_DEVICE_NO_ORDER
    )
endif()

set (HEADERS include/linkedlist.h)

