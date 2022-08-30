#
# This file contains the function to use when converting Regdef file to headers
#
# Generate_MemIO, Generate_RegIO2, Generate_RegFields, Generate_RegConv take the same arguments:
# - IN_DEF_FILE as the path to the register definition file (e.g. core.def)
# - OUT_H_FILE the path to the header it is going to generate (including its name)
#
# These functions add a custom command invoking GENERATE_GENERATE_TOOL_DEF2CODE.
# If you are not building def2code as part of your project you can run:
#           find_package(SivBinTools REQUIRED)
#           find_program(GENERATE_TOOL_DEF2CODE def2code HINTS ${SivBinTools_DIR})
#
# They populate the parent scope variable GENERATE_HEADERS_LIST that is used for dependencies.
#
# Then you can call Generate_target.
#
#
# example:
#
#   # this is done in previous scope
#	include(path/RegdefFunctions.cmake)
#
#	set(GENERATE_HEADERS_LIST)
#	set(REGDEF_NAME RegisterDefinitions)
#	
#	function(Mantis_headers IN_DEF_FILE OUT_DIR OUT_FILE)
#	
#		if (NOT EXISTS ${REGDEF_BINARY_INCLUDE}/${OUT_DIR})
#			file(MAKE_DIRECTORY ${REGDEF_BINARY_INCLUDE}/${OUT_DIR})
#		endif()
#		set(OUT_DIR ${REGDEF_BINARY_INCLUDE}/${OUT_DIR})
#		
#		Generate_RegIO2(${CMAKE_CURRENT_SOURCE_DIR}/${IN_DEF_FILE} ${OUT_DIR}/${OUT_FILE})
#		Generate_RegFields(${CMAKE_CURRENT_SOURCE_DIR}/${IN_DEF_FILE} ${OUT_DIR}/fields_${OUT_FILE})
#		
#		set(GENERATE_HEADERS_LIST ${GENERATE_HEADERS_LIST} PARENT_SCOPE)
#		
#	endfunction()
#	
#	Mantis_headers(mantis_core_regs.def registers core.h)
#	Mantis_headers(mantis_load_structure.def hw_struct ctx_config.h)
#	
#	Generate_target(${REGDEF_NAME})
#

#
# searches for def2code in CMAKE_PROGRAM_PATH if it is not defined already
#
# if you are building def2code you may want to define it before calling this function
#
macro(FIND_DEF2CODE)
if (NOT DEFINED GENERATE_TOOL_DEF2CODE)
	find_program(GENERATE_TOOL_DEF2CODE NAMES def2code)
	if(${GENERATE_TOOL_DEF2CODE} MATCHES "GENERATE_TOOL_DEF2CODE-NOTFOUND")
		message(FATAL_ERROR "def2code was not found")	  
	endif()
endif()
endmacro()

macro(FIND_REGCONV)
if (NOT DEFINED GENERATE_TOOL_REGCONV)
	find_program(GENERATE_TOOL_REGCONV NAMES regconv)
	if(${GENERATE_TOOL_REGCONV} MATCHES "GENERATE_TOOL_REGCONV-NOTFOUND")
		message(FATAL_ERROR "regconv was not found")
	endif()
endif()
if (NOT DEFINED GENERATE_TOOL_REGCONV_PRJ)
	set(GENERATE_TOOL_REGCONV_PRJ regconv)
endif()
endmacro()

# ----------------------------------------------------------------------
# Generate MemIO function
# ----------------------------------------------------------------------
function(Generate_MemIO IN_DEF_FILE OUT_H_FILE)

	FIND_DEF2CODE()

    # Custom command to do the processing
    add_custom_command(OUTPUT ${OUT_H_FILE} 
        COMMAND ${GENERATE_TOOL_DEF2CODE} ARGS ${IN_DEF_FILE} -memio ${OUT_H_FILE} 
        DEPENDS ${IN_DEF_FILE}) # ${GENERATE_TOOL_DEF2CODE_PRJ})

    # Finally remember the output file for dependencies
    set(GENERATE_HEADERS_LIST ${GENERATE_HEADERS_LIST} ${OUT_H_FILE} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------
# Generate RegIO2 function
# ----------------------------------------------------------------------
function(Generate_RegIO2 IN_DEF_FILE OUT_H_FILE)

	FIND_DEF2CODE()
	
    # Custom command to do the processing
    add_custom_command(OUTPUT ${OUT_H_FILE} 
        COMMAND ${GENERATE_TOOL_DEF2CODE} ARGS ${IN_DEF_FILE} -regio2 ${OUT_H_FILE} 
        DEPENDS ${IN_DEF_FILE}) # ${GENERATE_TOOL_DEF2CODE_PRJ})
		
    # Finally remember the output file for dependencies
    set(GENERATE_HEADERS_LIST ${GENERATE_HEADERS_LIST} ${OUT_H_FILE} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------
# Generate RegFields function
# ----------------------------------------------------------------------
function(Generate_RegFields IN_DEF_FILE OUT_H_FILE)

	FIND_DEF2CODE()
	
    # Custom command to do the processing
    add_custom_command(OUTPUT ${OUT_H_FILE} 
        COMMAND ${GENERATE_TOOL_DEF2CODE} ARGS ${IN_DEF_FILE} -regfields ${OUT_H_FILE} 
        DEPENDS ${IN_DEF_FILE}) # ${GENERATE_TOOL_DEF2CODE_PRJ})
		
    # Finally remember the output file for dependencies
    set(GENERATE_HEADERS_LIST ${GENERATE_HEADERS_LIST} ${OUT_H_FILE} PARENT_SCOPE)
endfunction()

# ----------------------------------------------------------------------
# Generate RegConv function
# ----------------------------------------------------------------------
function(Generate_RegConv IN_DEF_FILE CHIP_NAME OUT_H_FILE)

	FIND_REGCONV()
	
    # Custom command to do the processing
    add_custom_command(OUTPUT ${REGDEG_BINARY_INCLUDE}/${OUT_H_FILE} 
        COMMAND ${GENERATE_TOOL_REGCONV} ARGS -chipname ${CHIP_NAME} ${IN_DEF_FILE} -simout ${OUT_H_FILE} 
        DEPENDS ${IN_DEF_FILE})

    # Finally remember the output file for dependencies
    set(GENERATE_HEADERS_LIST ${GENERATE_HEADERS_LIST} ${OUT_H_FILE} PARENT_SCOPE)
endfunction()

function(Generate_target REGDEF_NAME)

	if (NOT GENERATE_HEADERS_LIST)
		message (FATAL_ERROR "GENERATE_HEADERS_LIST is empty - call some of the generate functions before generating the target")
	endif()

	# ----------------------------------------------------------------------
	# Top level target for all register definitions generation
	# ----------------------------------------------------------------------
	add_custom_target(${REGDEF_NAME} ALL DEPENDS ${GENERATE_HEADERS_LIST})
	
	set_source_files_properties(${GENERATE_HEADERS_LIST} PROPERTIES GENERATED 1)
endfunction()
