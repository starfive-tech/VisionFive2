# This file is designed to be included in a top level CMakeLists.txt like so:
#
#     include(path/to/here/UsePackage.cmake)
#
# It will add to the module path all subdirectories with a FindXXX.cmake file

# Do everything inside a function so as not to clash with any existing variables
function(AddDirsToModulePath)
   get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
     ${CURRENT_DIR}
     PARENT_SCOPE
     )
endfunction()

if(DEBUG_MODULES)
  message("CMAKE_MODULE_PATH before: ${CMAKE_MODULE_PATH}")
endif()

AddDirsToModulePath()

if(DEBUG_MODULES)
  message("CMAKE_MODULE_PATH  after: ${CMAKE_MODULE_PATH}")
endif()