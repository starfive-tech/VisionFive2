#
# Defines the register definitions headers location
#
#
cmake_minimum_required (VERSION 2.8)

get_filename_component(REGDEF_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

set (REGDEF_FOUND TRUE)
set (REGDEF_NAME Mantis_RegistersDefinitions) # needed otherwise add_dependencies may fail (compatibility with generated headers)
set (REGDEF_VHC_OUT ${REGDEF_PREFIX}/vhc_out)
set (REGDEF_BINARY_INCLUDE ${REGDEF_PREFIX}/include) # contains only the registers so that they are ignored in doxygen (compatibility with generated headers)
set (REGDEF_INCLUDE_DIRS ${REGDEF_PREFIX}/include ${REGDEF_VHC_OUT})
set (REGDEF_DEFINITIONS)

#
# this function is used when packaging
#
function(CopyVHCOutput TARGET_NAME INPUT_FILE OUTPUT_FILE)

add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${REGDEF_PREFIX}/vhc_out/${INPUT_FILE} ${OUTPUT_FILE}
)
add_dependencies(${TARGET_NAME} ${REGDEF_NAME})

endfunction()