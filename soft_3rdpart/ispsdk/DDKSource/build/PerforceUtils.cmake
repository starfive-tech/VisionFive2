cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED P4UTILS_BASE_DIR)
	set(P4UTILS_BASE_DIR ${CMAKE_SOURCE_DIR})
endif()

function(PerforceFilename OUTPUT_VAR_NAME)

	set(${OUTPUT_VAR_NAME} "${P4UTILS_BASE_DIR}/changelist.txt" PARENT_SCOPE)
	
endfunction()

#
# Store in a variable the fact that p4 has been found or not
#
function(PerforceFound OUTPUT_VAR_NAME)

	set(${OUTPUT_VAR_NAME} FALSE PARENT_SCOPE)
	set(P4_FILENAME)
	PerforceFilename(P4_FILENAME)
	
	if (EXISTS "${P4_FILENAME}")
		set(${OUTPUT_VAR_NAME} TRUE PARENT_SCOPE)
	else() # try to use perforce

		find_program(P4_EXE p4 HINT ${PERFORCE_PATH})
		
		if (${P4_EXE} MATCHES "P4_EXE-NOTFOUND")
			message("p4 not found - try to define PERFORCE_PATH")
		else()
			set(${OUTPUT_VAR_NAME} TRUE PARENT_SCOPE)
		endif()
	
	endif()
	
endfunction()

#
# Get the Perforce Changelist number currently used in SOURCE_DIR
#         This assumes that just 'SOURCE_DIR' will refer to a particular workspace - oherwise result may be incorrect!
#         i.e. it will find a p4 config file in current or parent dir with P4CLIENT set 
#         or that the SOURCE_DIR is a the directory is depot path
# Output in parent scope variable name OUTPUT_VAR_NAME
#
# Dies if p4 is not found (see PerforceFound).
# Error message is put in the output variable if p4 is found but the result of the command failed (e.g. wrong workspace)
#
# example:
#
# PerforceFound(P4FOUND)
# if(${P4FOUND})
#    PerforceChangelist(${CMAKE_CURRENT_SOURCE_DIR}/mantis mantisCL)
#    PerforceChangelist(${CMAKE_CURRENT_SOURCE_DIR}/sim simCL)
#    message("using mantis ${mantisCL} and Sim ${SimCL})
# else()
#    message(FATAL_ERROR "cannot find P4")
# endif()
#
function(PerforceChangelist SOURCE_DIR OUTPUT_VAR_NAME)

	PerforceFound(P4_FOUND)
	PerforceFilename(P4_FILENAME)

	if (EXISTS "${P4_FILENAME}")
	
		set(CHANGELIST)
		set(CHANGELISTNB)
		file(STRINGS "${P4_FILENAME}" CHANGELIST REGEX "Changelist")
		string(REGEX REPLACE ".* ([0-9]+) .*" "\\1" CHANGELISTNB ${CHANGELIST})

		if (DEBUG_MODULES)
			message("Found changelist in file ${CHANGELIST}")
			message("Found changelist parsing ${CHANGELISTNB}")
		endif()
		
		set(${OUTPUT_VAR_NAME} ${CHANGELISTNB} PARENT_SCOPE)
	
	elseif (${P4_FOUND})
		set(cmd ${P4_EXE} changes -m 1 -t "${SOURCE_DIR}/...#have") #important - the command must NOT be in double quotes! it must be separate items, not one string
		
		execute_process(COMMAND ${cmd} OUTPUT_VARIABLE p4changesLine WORKING_DIRECTORY ${SOURCE_DIR})
		if (p4changesLine)
			string(REGEX MATCH "[0-9]+" p4changelistNumber ${p4changesLine})	#extract the number
		else()
			message("Failed to find p4 changelist for ${SOURCE_DIR}")
			set(p4changelistNumber "p4-error")
		endif()
		
		set(${OUTPUT_VAR_NAME} ${p4changelistNumber} PARENT_SCOPE)
		
	else()
		message(FATAL_ERROR "Could not find p4 - add perforce to your path or define PERFORCE_PATH")
	endif()

endfunction()

#
# Similar to PerforceChangelist but gets the date of the last submit
# Perforce date is yyyy/mm/dd
#
# Dies if p4 is not found (see PerforceFound).
# Error message is put in the output variable if p4 is found but the result of the command failed (e.g. wrong workspace)
#
function(PerforceUpdateDate SOURCE_DIR OUTPUT_VAR_NAME)

	PerforceFound(P4_FOUND)
	PerforceFilename(P4_FILENAME)

	if (EXISTS "${P4_FILENAME}")
	
		set(CHANGELIST)
		set(CHANGELISTDATE)
		file(STRINGS "${P4_FILENAME}" CHANGELIST REGEX "DateTime")
		string(REGEX REPLACE ".* ([0-9]+/[0-9]+/[0-9]+) .*" "\\1" CHANGELISTDATE ${CHANGELIST})

		if (DEBUG_MODULES)
			message("Found changelist in file ${CHANGELIST}")
			message("Found changelist parsing ${CHANGELISTDATE}")
		endif()
		
		set(${OUTPUT_VAR_NAME} ${CHANGELISTDATE} PARENT_SCOPE)
	
	elseif (${P4_FOUND})
	
		set(cmd ${P4_EXE} changes -m 1 -t "#have") #important - the command must NOT be in double quotes! it must be separate items, not one string
		
		execute_process(COMMAND ${cmd} OUTPUT_VARIABLE p4changesLine  WORKING_DIRECTORY ${SOURCE_DIR})
		if (p4changesLine)
			string(REGEX MATCH "[0-9]*/[0-9]*/[0-9]*" p4changeDate ${p4changesLine})	#extract the date
		else()
			message("Failed to find p4 last submit date for ${SOURCE_DIR}")
			set(p4changeDate "p4-error")
		endif()
		
		set(${OUTPUT_VAR_NAME} ${p4changeDate} PARENT_SCOPE)
		
	else()
		message(FATAL_ERROR "Could not find p4 - add perforce to your path or define PERFORCE_PATH")
	endif()

endfunction()
