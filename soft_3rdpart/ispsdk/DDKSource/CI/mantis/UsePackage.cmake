# This file is designed to be included in a top level CMakeLists.txt like so:
#
#     include(path/to/here/UsePackage.cmake)
#
# It will add to the module path all subdirectories with a FindXXX.cmake file


# Do everything inside a function so as not to clash with any existing variables
function(AddDirsToModulePath)
	get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
 
#	include(${CURRENT_DIR}/mantis_lib/kernel/UsePackage.cmake)
	include(${CURRENT_DIR}/defaultarch.cmake)
 
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
			${CURRENT_DIR}/regdefs${CI_HW_ARCH})
	
	#	message("CMAKE_MODULE_PATH inside: ${CMAKE_MODULE_PATH}")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
		${CURRENT_DIR}/mantis_lib
		${CURRENT_DIR}/driver_test
		PARENT_SCOPE)
endfunction()

#message("CMAKE_MODULE_PATH before: ${CMAKE_MODULE_PATH}")
AddDirsToModulePath()
#message("CMAKE_MODULE_PATH  after: ${CMAKE_MODULE_PATH}")
