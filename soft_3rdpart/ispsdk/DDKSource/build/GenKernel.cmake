#
# Several function to generate Linux kernel Makefile
#
# global variable used:
# LINUX_KERNEL_BUILD_DIR - used by GenKernel_addTarget to find the KBuild
#
# functions:
# GenSpaceList()
# GenKernel_Makefile()
# GenKernel_Kconfig()
# GenKernel_addTarget()
#
# e.g. of usage
#	set(KO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mantis_kn.c)
#	set(KO_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/kn)
#
#	if (NOT DEFINED LINUX_KERNEL_BUILD_DIR)
#		execute_process(COMMAND uname -r OUTPUT_VARIABLE KERNEL_VER OUTPUT_STRIP_TRAILING_WHITESPACE)
#		set (LINUX_KERNEL_BUILD_DIR "/lib/modules/${KERNEL_VER}/build")
#		message ("LINUX_KERNEL_BUILD_DIR not define: using kernel ${KERNEL_VER}")
#	endif()
#
#	get_directory_property(KO_INCLUDES INCLUDE_DIRECTORIES)
#	GenSpaceList("${KO_INCLUDES}" "-I" KO_INCLUDES)
#	get_directory_property(KO_CFLAGS DEFINITIONS) # spaces and -D
#
#	GenKernel_Makefile(${MANTISLIB_NAME} "${KO_CFLAGS}" "${KO_SOURCES}" "${KO_INCLUDES}" ${KO_BINARY_DIR} ${KO_MODE})
#	GenKernel_addTarget(${MANTISLIB_NAME}_KO ${KO_BINARY_DIR})

if(NOT DEFINED GENKERN_PREFIX)
  # set MANTISLIB_PREFIX to directory where the current file file is located
  get_filename_component(GENKERN_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
endif()

#
# Transform a cmake list (; separated) into space separated with a prefix (e.g. -I)
#
function(GenSpaceList LIST PREFIX PARENT_SCOPE_LIST_NAME)

  set (TMP)
  foreach(v ${LIST})
    set (TMP "${TMP} ${PREFIX}${v}")
  endforeach()

  set (${PARENT_SCOPE_LIST_NAME} ${TMP} PARENT_SCOPE)

endfunction()

#
# Generate a kernel module Makefile usable with Kbuild
#
# KO_MODULE_NAME is the name of the kernel target
# KO_CFLAGS list of cflags as a string with -D and separated with spaces - protected with "${LIST}"
# KO_INCLUDES list of includes as string with -I and separated with spaces - protected with "${LIST}"
# KO_SOURCES is a list of files (normal CMake list work) to compile for the module - protected with "${LIST}"
#            WARNING: the path must be global (use CMAKE_CURRENT_SOURCE_DIR, see example above)
# KO_OUTDIR is the place where all C files will be copied and the Makefile generated
# KO_MODE is the mode for kernel compilation, "m" for module "y" for built-in
#
# Note you can use GenSpaceList and get_directory_property to get the spaced lists:
# e.g.
#  get_directory_property(KO_INCLUDES INCLUDE_DIRECTORIES)
#  GenSpaceList("${KO_INCLUDES}" "-I" KO_INCLUDES)
#
function(GenKernel_Makefile KO_MODULE_NAME KO_CFLAGS KO_SOURCES KO_INCLUDES KO_OUTDIR KO_MODE)

  if (NOT EXISTS ${KO_OUTDIR})
    file(MAKE_DIRECTORY ${KO_OUTDIR})
  endif()
  if (NOT EXISTS ${KO_OUTDIR}/include)
    file(MAKE_DIRECTORY ${KO_OUTDIR}/include)
  endif()

  set(TO_CLEAN
    ${KO_OUTDIR}/${KO_MODULE_NAME}.ko
    ${KO_OUTDIR}/${KO_MODULE_NAME}.o
    ${KO_OUTDIR}/${KO_MODULE_NAME}.mod.c
    ${KO_OUTDIR}/${KO_MODULE_NAME}.mod.o
    ${KO_OUTDIR}/modules.order
    ${KO_OUTDIR}/Module.symvers
    ${KO_OUTDIR}/built-in.o
    )

  set (KO_LOCAL_INCLUDES "-I\$(src)/include")
  set (KO_INCLUDES "${KO_INCLUDES} ${KO_LOCAL_INCLUDES}")
  
  set (TMP) # reset
  foreach(file ${KO_SOURCES})
    # copy file to local dir - c files in local, h files in ./include
    # extract filename without extention
    get_filename_component (filename ${file} NAME)
    #
    get_filename_component (extension ${file} EXT)

    # for a module called MyMod.ko a file MyMod.c is needed to define the point of entry
    # and other things but shouldn't be in the sources
    if (${extension} MATCHES ".c" )

      get_filename_component (filename_we ${file} NAME_WE)
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${file} ${KO_OUTDIR}/${filename})
      
	set (TMP "${TMP} ${filename_we}.o")
	set (TO_CLEAN ${TO_CLEAN} ${KO_OUTDIR}/${filename_we}.o)
      
    elseif (${extension} MATCHES ".h")
 
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${file} ${KO_OUTDIR}/include/${filename})

    elseif(IS_DIRECTORY ${file})

      file(GLOB files ${file}/*) # get files AND directories underneath

      if (DEBUG_MODULES)
	message("process ${file}")
      endif()
      foreach(elem ${files})
	get_filename_component(filen ${elem} NAME)
	get_filename_component(ext ${elem} EXT)

	if ( ${ext} MATCHES ".h" OR IS_DIRECTORY ${elem} )
	  
	  if (DEBUG_MODULES)
	    message("   link ${filen}")
	  endif()
	  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${elem} ${KO_OUTDIR}/include/${filen})

	endif()
      endforeach()

    else()
      
      message ("unknown extension for file ${file}")

    endif()
  endforeach()
  set (KO_SOURCES ${TMP})

  message (STATUS "generating ${KO_OUTDIR}/Makefile")
  configure_file(${GENKERN_PREFIX}/kernel_makefile.cmakein ${KO_OUTDIR}/Makefile)
  
  if(${KO_MODE} MATCHES "y")
    # generate an additional makefile for module building
    set(KO_MODE "m")
    configure_file(${GENKERN_PREFIX}/kernel_makefile.cmakein ${KO_OUTDIR}/Makefile_module)
  else()
    # generate an additional makefile for static building
    set(KO_MODE "y")
    configure_file(${GENKERN_PREFIX}/kernel_makefile.cmakein ${KO_OUTDIR}/Makefile_static)
  endif()
  
  set(KO_INCLUDES ${KO_LOCAL_INCLUDES}) # empty for gpl makefile but local includes
  configure_file(${GENKERN_PREFIX}/kernel_makefile.cmakein ${KO_OUTDIR}/Makefile_gpl)
  
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${TO_CLEAN}")

endfunction()

#
# Generate a simple Kconfig file - this is experimental
#
# KO_MODULE_NAME
# KO_BUILD_MODULE - if yes set the KO_BUILD to tristate (kernel module, inside kernel or not built) if no set to bool (inside kernel or not built)
# KO_BRIEF - brief description of the module
# KO_DEFAULT - default mode (y for inside kernel, m for module - if KO_BUILD_MODULE=YES - or n for not built)
# KO_HELP - description of the module
# KO_OUTDIR
# KO_DEPENDS - string that defines the dependencies e.g. "depends on V4L && !DMA"
#
function(GenKernel_Kconfig KO_MODULE_NAME KO_BUILD_MODULE KO_BRIEF KO_BUILD_DEFAULT KO_HELP KO_OUTDIR KO_DEPENDS)
  
  if (NOT EXISTS ${KO_OUTDIR})
    file(MAKE_DIRECTORY ${KO_OUTDIR})
  endif()

  set(KO_DEFAULT "n")
  
  if (KO_BUILD_MODULE)
    set(KO_BUILD "tristate")
    if (${KO_BUILD_DEFAULT})
      set(KO_DEFAULT "m")
    endif()
  else()
    set(KO_BUILD "bool")
    if (${KO_BUILD_DEFAULT})
      set(KO_DEFAULT "y")
    endif()
  endif()
    
  set(KO_EXTRA ${KO_DEPENDS})

  message (STATUS "generating ${KO_OUTDIR}/Kconfig")
  configure_file(${GENKERN_PREFIX}/kernel_kconfig.cmakein ${KO_OUTDIR}/Kconfig)

endfunction()

#
# Add custom target ${KO_MODULE_NAME}
#
# KO_MODULE_NAME is the name of the kernel target
# KO_OUTDIR is the place where all C files will be copied and the Makefile generated
#
# Uses a globally defined LINUX_KERNEL_BUILD_DIR that is the directory your kernel is in
#
function(GenKernel_addTarget KO_MODULE_NAME KO_OUTDIR)

  if(NOT DEFINED LINUX_KERNEL_BUILD_DIR)
    message(FATAL_ERROR "LINUX_KERNEL_BUILD_DIR should be defined")
  endif()

  add_custom_target(${KO_MODULE_NAME} ALL 
    COMMAND MAKEFLAGS= ${CMAKE_MAKE_PROGRAM} -C ${LINUX_KERNEL_BUILD_DIR} M=${KO_OUTDIR} modules
    WORKING_DIRECTORY ${KO_OUTDIR}
    COMMENT "Building the kernel module: ${KO_MODULE_NAME}"
    )
endfunction()

#
# Add custom target ${KO_MODULE_NAME}
#
# KO_MODULE_NAME is the name of the kernel target
# KO_OPT additional option to give to make when building the KO
# KO_OUTDIR is the place where all C files will be copied and the Makefile generated
#
# Uses a globally defined LINUX_KERNEL_BUILD_DIR that is the directory your kernel is in
#
function(GenKernel_addSpecialTarget KO_MODULE_NAME KO_OPT KO_OUTDIR)

  if(NOT DEFINED LINUX_KERNEL_BUILD_DIR)
    message(FATAL_ERROR "LINUX_KERNEL_BUILD_DIR should be defined")
  endif()

  add_custom_target(${KO_MODULE_NAME} ALL 
    COMMAND MAKEFLAGS= ${CMAKE_MAKE_PROGRAM} -C ${LINUX_KERNEL_BUILD_DIR} ${KO_OPT} M=${KO_OUTDIR} modules
    WORKING_DIRECTORY ${KO_OUTDIR}
    COMMENT "Building the kernel module: ${KO_MODULE_NAME}"
    )
endfunction()
