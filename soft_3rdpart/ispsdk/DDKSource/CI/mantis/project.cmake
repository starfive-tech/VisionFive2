cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE)
  if (WIN32)
    set(CMAKE_BUILD_TYPE "Debug")
  else()
    set(CMAKE_BUILD_TYPE "Release")
  endif()
endif()

##option (MANTIS_MEM_CHECK "Enable memory checking for mantis" OFF)
#option (CI_DEBUG_FUNCTIONS "enable debugFS for CI - for fake driver enable use of debug functions" ON)
#option (CI_MEMSET_ADDITIONAL_MEMORY "enable memset to 0xAA of additional device memory rounding up allocations to CPU page size" ON)
#option (CI_MEMSET_ALLOCATED_MEMORY "enable memset of device memory to 0x0" OFF)
#
if (WIN32)
  set (MANTIS_USE_LOCAL_ZLIB ON)

  set(CI_BUILD_KERNEL_MODULE FALSE)
  
else(WIN32)
  #option (MANTIS_USE_LOCAL_ZLIB "Build Mantis using the local ZLib" OFF)
  option (CI_BUILD_KERNEL_MODULE "Build Mantis API as a kernel module" ON)

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl -Wl,-rpath-link -Wl,")

  option(FORCE_32BIT_BUILD "Build a 32bit binary on a 64bit system" OFF)
  if(${FORCE_32BIT_BUILD} MATCHES "ON")
      add_definitions(-m32)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
      set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -m32")
  endif()
  
  option (CI_CODE_COVERAGE "Use GCovr to compute code coverage (use with GCC)" OFF)
endif()

#if (NOT DEFINED CI_DEVICE)
#    set(CI_DEVICE "MEMMAPPED")
#    if (CI_BUILD_KERNEL_MODULE) # when building fake driver the choice does not matter
#        message("Selecting default device ${CI_DEVICE}")
#    endif()
#endif()
#
#if (NOT DEFINED CI_ALLOC)
#    if (${CI_MEM_DMA_ON} MATCHES "ON")
#        set(CI_ALLOC "MEMDMA")
#        add_definitions(-DSIFIVE_CI_MEM_DMA_ON)
#    else()	
#        set(CI_ALLOC "CARVEOUT")
#    endif()
#    if (CI_BUILD_KERNEL_MODULE) # when building fake driver the chocie does not matter
#        message("Selecting default allocator ${CI_ALLOC}")
#    endif()
#endif()
#

if (NOT DEFINED PLATFORM)
    set(PLATFORM "unknown") # just to be able to pass STREQUAL tests
endif()
if (NOT DEFINED PORTFWRK_MEMALLOC_UNIFIED_VMALLOC)
    set(PORTFWRK_MEMALLOC_UNIFIED_VMALLOC FALSE)
endif()
if (NOT DEFINED PORTFWRK_SYSMEM_CARVEOUT)
    set(PORTFWRK_SYSMEM_CARVEOUT FALSE)
endif()


if (CI_BUILD_KERNEL_MODULE)
  
  if (NOT DEFINED LINUX_KERNEL_BUILD_DIR)
    execute_process(COMMAND uname -r OUTPUT_VARIABLE KERNEL_VER OUTPUT_STRIP_TRAILING_WHITESPACE)
    set (LINUX_KERNEL_BUILD_DIR "/lib/modules/${KERNEL_VER}/build")
    message ("LINUX_KERNEL_BUILD_DIR not define: using kernel ${KERNEL_VER}")
  else()
	message ("Building with linux kernel '${LINUX_KERNEL_BUILD_DIR}'")
  endif()

endif(CI_BUILD_KERNEL_MODULE)

# ----------------------------------------------------------------------
# Configure StfIncludes
# ----------------------------------------------------------------------
if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(STFINCLUDES_EXIT_ON_ASSERT TRUE)
else()
    set(STFINCLUDES_EXIT_ON_ASSERT FALSE)
endif()


## ----------------------------------------------------------------------
## Configure TAL
## ----------------------------------------------------------------------
#set(TAL_USING_DEVIF                 TRUE)
#if (NOT CI_BUILD_KERNEL_MODULE)
#  set(DEVIF_USE_MEOS FALSE)
#  set(DEVIF_USE_OSA TRUE)
#  set(TAL_USE_OSA TRUE)
#  set(TAL_TYPE "normal")
#  set(TAL_EXCLUDE_DEVIF pci bmem dash pdump1 posted dummy) # direct or transif?
#else()
#  set(TAL_TYPE "light")
#  set(TAL_TARGET_NAME "mantis")
#endif()
#set(TAL_MEMORY_BITMAP               TRUE)
#set(TAL_DEINIT_ON_EXIT              FALSE)
#
## ----------------------------------------------------------------------
## Other configurations
## ----------------------------------------------------------------------
## MMU size
#set(SIVMMU_SIZE 32) # use 32b physical size
#set(SIVMMU_TALMEM_MMU_NAME "MMU_PAGE") # optional - name of the memory block in pdump
##SIVMMU_TALMEM_NAME not use (default is BLOCK_X)
#
include(TestBigEndian)

TEST_BIG_ENDIAN(MANTIS_SYSTEM_BIGENDIAN)
