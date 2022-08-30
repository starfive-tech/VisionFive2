
get_filename_component (GENDOXYFILE_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

set (GENDOXYFILE_FOUND TRUE)
set (DOXYGEN_HTML_TEMPLATE ${GENDOXYFILE_PREFIX}/DoxyfileHTMLTemplate.cmakein)

#
# Defines these extra variables before calling the GenerateDoxyfile function
# Having an undefined variable just doesn't have effects
# Path with space must be protected with quotes set(VAR "\"path with space\"")
#
# DOXY_<NAME> refers to the <NAME> doxyfile option
# DOXY_EXAMPLE_PATH - protected path
# DOXY_HTML_EXTRA_FILES - watch out for the list, must be space separated not semi colon like cmake (look what GenerateDoxyfile does to DOXY_INCLUDE_FILES) 
# DOXY_IMAGE_PATH - protected path to images, must be space separated not semi colon like cmake (look what GenerateDoxyfile does to DOXY_INCLUDE_FILES) 
# DOXY_PREDEFINED - a normal CMake list without -D of #define that are present at compilation
# DOXY_INCLUDE_PATH - extra folder, protected path
# DOXY_HAVE_DOT - to enable usage of DOT for diagrams - can be derived from DOXYGEN_HAS_DOT from find_package(Doxygen)
# DOXY_DOT_PATH - to enable usage of DOT for diagrams - can be derived from DOXYGEN_DOT_PATH from find_package(Doxygen)
# DOXY_HTML_OUTPUT - specifies name of the HTML output
# DOXY_HTML_HEADER - header file template to use when generating HTML (add used file in DOXY_HTML_EXTRA_FILES)
# DOXY_HTML_FOOTER - footer file template to use when generating HTML (add used files in DOXY_HTML_EXTRA_FILES)
# DOXY_HTML_CSS - CSS file to use when generating HTML (add used files in DOXY_HTML_EXTRA_FILES)
# DOXY_DISABLE_INDEX - YES|NO disable/enable to top-level menu
# DOXY_GENERATE_TAGFILE - filename for tags
#
# example of extra files:
#	set (DOXY_HTML_EXTRA_FILES)
#	file(GLOB_RECURSE extra_files ${CMAKE_CURRENT_SOURCE_DIR}/doc_base/*)
#	foreach(found ${extra_files})
#		set (DOXY_HTML_EXTRA_FILES "${DOXY_HTML_EXTRA_FILES} \"${found}\"")
#	endforeach()
#

if (NOT DEFINED DOXYGEN_ALIASES)
    set (DOXYGEN_ALIASES   "\"Title=\\brief\" \\ 
                        \"Description=\\brief\" \\
                        \"Input=\\param[in]\" \\
                        \"Output=\\param[out]\" \\
						\"InOut=\\param[in,out]\" \\
                        \"Modified=\\param[out]\" \\
                        \"Return=\\return\" \\
                        \"Function=\" \\
						\"function=\\Function\" \\
                        \"Copyright=\\copyright\" \\
                        \"Date=\\date\" \\
                        \"License=\\par (License)\" \\
						\"license=\\License\" \\
						\"Author=\\author\" \\
						\"Version=\\par (Version)\" \\
						\"Platform=\\par (Platform)\" \\
						\"Macro=\\def\" \\
						\"File=\\file\"
						"
    )
endif()
if (NOT DEFINED DOXY_HTML_OUTPUT)
    set(DOXY_HTML_OUTPUT ".")
endif()
if (NOT DEFINED DOXY_DISABLE_INDEX)
    set(DOXY_DISABLE_INDEX "NO")
endif()

#
# Generate a doxyfile with some options from CMake
# - template name and doxyfile output
# - projectName documentation name
# - docOutputDir where to generate the documentation
# - isPublicDoc TRUE|FALSE that triggers generation of test/bug lists and other warnings if FALSE
# - inputFiles CMake list of files or directory to use as input for doxygen
#
# note that doxygen doesn't like lists separated with semicolon therefore this functions transform them in list separated by spaces
# WARNING that implies you to protect your list call with quotes
# 
# Example of usage: 
#   set (DOC_GEN ${MANTISLIB_INCLUDE_DIRS} ${PUBLIC_HEADERS} ${HEADERS} ${SOURCES})
#   GenerateDoxyfile (	${DOXYGEN_TEMPLATE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.private"
#      "${MANTISLIB_NAME} Internal documentation"
#      "${MANTIS_BINARY_DIR}/doc_private"
#      FALSE
#      "${DOC_GEN}"
#   )
#
function(GenerateDoxyfile DoxyfileTemplate DoxyfileOutputFile
 projectName 
 docOutputDir
 isPublicDoc
 inputFiles
 )

	set (DOXY_PROJ_NAME ${projectName})
	set (DOXY_OUTPUT_DIR ${docOutputDir})

	if(NOT EXISTS ${DOXY_OUTPUT_DIR})
		file(MAKE_DIRECTORY ${DOXY_OUTPUT_DIR})
	endif()
	
	if (${isPublicDoc})
		set (DOXY_EXTRAT_ALL "NO")
		set (DOXY_EXTRA_DOC "NO")
		if(DEBUG_MODULES)
			message (STATUS "generate public Doxyfile: ${DoxyfileOutputFile}")
		endif()
	else()
		set (DOXY_EXTRAT_ALL "YES")
		set (DOXY_EXTRA_DOC "YES")
		if(DEBUG_MODULES)
			message (STATUS "generate private Doxyfile: ${DoxyfileOutputFile}")
		endif()
	endif()

	set (DOXY_INCLUDE_FILES) # reset
	foreach (inc ${inputFiles})
		set (DOXY_INCLUDE_FILES "${DOXY_INCLUDE_FILES} \"${inc}\"")
	endforeach()
	 
	set (DOXY_PREDEFINED ${DOXY_PREDEFINED} "DOXYGEN_CREATE_MODULES DOXYGEN_WILL_SEE_THIS")
	set (tmp_DOXY_PREDEFINED)
	foreach (define ${DOXY_PREDEFINED})
		set (tmp_DOXY_PREDEFINED "${tmp_DOXY_PREDEFINED} ${define}")
	endforeach()
	set (DOXY_PREDEFINED ${tmp_DOXY_PREDEFINED})

	configure_file ( ${DoxyfileTemplate} ${DoxyfileOutputFile} )
 
endfunction()
