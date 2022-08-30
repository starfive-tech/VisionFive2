
find_package(StfIncludes REQUIRED)
find_package(StfCommon REQUIRED)
find_package(GTest QUIET)

message (STATUS "-- Mantis Common")

include_directories(
	${STFINCLUDES_INCLUDE_DIRS}
	${STFCOMMON_INCLUDE_DIRS}
)

add_definitions(
	${STFINCLUDES_DEFINITIONS}
	${STFCOMMON_DEFINITIONS}
)

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

set (HEADERS
    include/stf_common/pixel_format.h
    include/stf_common/userlog.h
    include/stf_common/ci_alloc_info.h
)
