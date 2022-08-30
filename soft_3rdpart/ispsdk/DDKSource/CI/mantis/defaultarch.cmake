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

if (NOT DEFINED CI_HW_ARCH)
    if (BUILD_HW_VIC_1_0)
        set(CI_HW_ARCH 1.0)
    elseif (BUILD_HW_VIC_2_0)
        set(CI_HW_ARCH 2.0)
    else()
        set(CI_HW_ARCH 1.0)
    endif()
endif()
