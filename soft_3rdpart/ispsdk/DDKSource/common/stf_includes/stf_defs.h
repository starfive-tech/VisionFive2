/** 
 **************************************************************************
 @file           c99/stf_defs.h

 @brief          Base type definitions using C99 headers

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license        <Strictly Confidential.>
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.
 
 **************************************************************************/
#ifndef __STF_DEFS_H__
#define __STF_DEFS_H__


#include "stf_sysdefs.h" // system specific definitions


#ifdef __cplusplus
extern "C" {
#endif


/*
 * maybe endianness could be guessed:
 * #define is_bigendian() ( ((char)1) == 0 )
 * #define BYTE_ORDER (is_bigendian() ? BIG : SMALL)
 */

/*
 * 3 different allocation
 * - default: use of SYSMALLOC and SYSFREE
 * - check: count the number of calls to malloc, calloc and free
 * - test: uses a global variable to know if malloc or calloc will fail
 */

#ifdef STF_MALLOC_CHECK
/*
 * this version uses external global variable that the main program must define
 *
 * WARNING: this is not thread safe!
 */
#include "stf_types.h"

/**
 * @brief The number of successful calls to malloc AND calloc (used if STF_MALLOC_CHECK)
 */
extern STF_U32 g_u32Alloc;
/**
 * @brief The number of calls to free (used if STF_MALLOC_CHECK)
 */
extern STF_U32 g_u32Free;

static void* stf_malloc_check(STF_SIZE size)
{
	void* ptr = STF_SYSMALLOC(size);
	if ( ptr != NULL ) g_u32Alloc++;
	return ptr;
}

static void* stf_calloc_check(STF_SIZE nelem, STF_SIZE elem_size)
{
	void* ptr = STF_SYSCALLOC(nelem, elem_size);
	if ( ptr != NULL ) g_u32Alloc++;
	return ptr;
}

static void stf_free_check(void* ptr)
{
	STF_SYSFREE(ptr);
	g_u32Free++;
}

#define STF_MALLOC(size) stf_malloc_check(size)
#define STF_CALLOC(nelem, elem_size) stf_calloc_check(nelem, elem_size)
#define STF_FREE(ptr) stf_free_check(ptr)

#endif

#ifdef STF_MALLOC_TEST
/*
 * this version uses and external global variable to know if the memory allocation should fail
 */
#include "stf_types.h"
 
/**
 * @brief To know if memory allocation should fail (used if STF_MALLOC_TEST)
 *
 * The value must be set before calling STF_MALLOC or STF_CALLOC.
 * The functions will behave:
 * @li if the value is 0 allocation succeeds
 * @li if value is 1 allocations fails (and values becomes 0)
 * @li if value is > 0 value is decremented
 * 
 * This behaviour can be used to test allocation process (e.g. set g_u32AllocFails to 5 and the 5th allocation after that will fail)
 *
 * @warning your code needs to define this variable (e.g. STF_U32 guiAllocFails = 0;).
 * If your code is C++ don't forget the extern "C"
 */ 
extern STF_U32 g_u32AllocFails;

// return 1 when should allocate, 0 when should not
static int stf_test_alloc(void)
{
	if (g_u32AllocFails >= 1)
	{
		g_u32AllocFails--;
		if ( g_u32AllocFails == 0 )
		{
			// it was 1 -> should fail
			return 0;
		}
	}
	return 1;
}

static void* stf_malloc_test(STF_SIZE size)
{
	if (stf_test_alloc())
	{
#ifdef STF_MALLOC_CHECK
	  void* ptr = stf_malloc_check(size);
#else
	  void* ptr = STF_SYSMALLOC(size);
#endif
		return ptr;
	}
	return NULL;
}
static void* stf_calloc_test(STF_SIZE nelem, STF_SIZE elem_size)
{
	if (stf_test_alloc())
	{
#ifdef STF_MALLOC_CHECK
	  void* ptr = stf_calloc_check(nelem, elem_size);
#else
	  void* ptr = STF_SYSCALLOC(nelem, elem_size);
#endif
		return ptr;
	}
	return NULL;
}

#ifdef STF_MALLOC_CHECK
#undef STF_MALLOC
#undef STF_CALLOC
#endif

#define STF_MALLOC(size) stf_malloc_test(size)
#define STF_CALLOC(nelem, elem_size) stf_calloc_test(nelem, elem_size)

#endif

/*
 * default use system allocation
 */
#ifndef STF_MALLOC
#define STF_MALLOC(size) STF_SYSMALLOC(size)
#endif
#ifndef STF_CALLOC
#define STF_CALLOC(nelem, elem_size) STF_SYSCALLOC(nelem, elem_size)
#endif
#ifndef STF_FREE
#define STF_FREE(ptr) { if (ptr) { STF_SYSFREE(ptr); ptr = NULL; } }
#endif
#ifndef STF_REALLOC
#define STF_REALLOC(ptr, size) STF_SYSREALLOC(ptr, size)
#endif
#ifndef STF_STRDUP
#define STF_STRDUP(ptr) STF_SYS_STRDUP(ptr)
#endif
#ifndef STF_BIGALLOC
#define STF_BIGALLOC(size) STF_SYSBIGALLOC(size)
#endif
#ifndef STF_BIGFREE
#define STF_BIGFREE(ptr) { if (ptr) { STF_SYSBIGFREE(ptr); ptr = NULL; } }
#endif

/// @note maybe this should be done in a function to know if it worked with another way than just assert...
#define STF_U64_TO_UINT32(u64Check) (STF_ASSERT(((u64Check) >> 32) == 0), (STF_U32)(u64Check))

/**
 * @brief Maximum of two integers without branches
 *
 * From http://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
 */
#define STF_MAX_INT(x, y) ( (x) ^ ( ((x) ^ (y)) & -((x) < (y)) ) )

/**
 * @brief Minimum of two integers without branches
 *
 * From http://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
 */
#define STF_MIN_INT(x, y) ( (y) ^ ( ((x) ^ (y)) & -((x) < (y)) ) )

/// compile time assert (e.g. for enum values)
#define STF_STATIC_ASSERT( condition, name )\
	typedef char assert_failed_ ## name [ (condition) ? 1 : -1 ];

// add functions here


#ifdef __cplusplus
}
#endif


#endif // __STF_DEFS_H__
