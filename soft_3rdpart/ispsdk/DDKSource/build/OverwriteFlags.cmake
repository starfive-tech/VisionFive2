
# The overrides (which seem to be required to make everything use the same version of the windows runtime libraries
# Have to be included prior to the project line

set (BUILD_SHARED_LIBS FALSE)

if (WIN32)

	# for some reasons VS 9 generator doesn't overwrite the flags
	# but generator VS 2010 does
	if (BUILD_SHARED_LIBS)
		set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -MDd")
		set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -MDd")
		set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MD")
		set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MD")
	else()
		set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -MTd")
		set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -MTd")
		set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MT")
		set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MT")
	endif()

	add_definitions(
		-D_M_IX86
		-D__STDC__=0
        -D_CRT_NONSDC_NO_WARNINGS
       	-D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
	)
else()
  add_definitions(-g) # enable debugging flags

  if (ENABLE_GPROF)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pg")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_C_FLAGS} -pg")
  endif()

endif()
