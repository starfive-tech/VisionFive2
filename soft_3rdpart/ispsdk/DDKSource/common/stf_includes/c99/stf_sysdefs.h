/**
 **************************************************************************
 @file           c99/stf_sysdefs.h

 @brief

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license        <Strictly Confidential.>
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 **************************************************************************/
#ifndef __STF_SYSDEFS_H__
#define __STF_SYSDEFS_H__


#include <string.h> // strcpy, ... and memcpy, ...
#include <assert.h>
#ifdef __android__
// in adroid use the system log
#include <android/log.h>
#endif
#include <stdlib.h> // malloc, ...
#include <stdio.h> // printf for STF_ASSERT


#ifdef __cplusplus
extern "C" {
#endif


/*
 * language abstraction
 */

#define STF_CONST const
#define STF_INLINE inline

// useful?
//#define STF_CALLCONV __stdcall
//#define STF_INTERNAL
//#define STF_EXPORT	__declspec(dllexport)

// not very nice
#define STF_LITTLE_ENDIAN	(1234)
#define STF_BIG_ENDIAN		(4321)
/* Win32 is little endian */
#define	STF_BYTE_ORDER		STF_LITTLE_ENDIAN

/**
 * @brief 64bit value prefix - e.g. printf("long %" STF_I64PR "d")
 */
#ifdef __GNUC__
#  ifdef __LP64__
#    define STF_I64PR "l"
#  else
#    define STF_I64PR "ll"
#  endif
#elif defined(__WORDSIZE)
#  if __WORDSIZE == 64
#    define STF_I64PR "l"
#  else
#    define STF_I64PR "ll"
#  endif
#else
#  if defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#    define STF_I64PR "l"
#  else
#    define STF_I64PR "ll"
#  endif
#endif
/** @brief STF_SIZE (i.e. size_t) format prefix - e.g. STF_SIZE s -> printf("sizeof= %" STF_SIZEPR "u", s) */
#define STF_SIZEPR "z"
/** @brief STF_PTRDIFF (i.e. ptrdiff_t) format prefix - e.g. STF_PTRDIFF diff -> printf(diff= 0x%" STF_PTRDPR "x", s) */
#define STF_PTRDPR "t"
/*
 * memory operation
 */

#define	STF_MEMCPY(dest,src,size)	memcpy	(dest,src,size)
#define	STF_MEMSET(ptr,val,size)	memset	(ptr,val,size)
#define STF_MEMCLR(ptr,size)        memset  (ptr,0x00,size)
#define STF_MEMCMP(A,B,size)		memcmp	(A,B,size)
#define STF_MEMMOVE(dest,src,size)	memmove	(dest,src,size)

/** @brief C89 says: space is unitialized */
#define STF_SYSMALLOC(size)       malloc(size)
/** @brief C89 says: space is initialized to zero bytes */
#define STF_SYSCALLOC(nelem, elem_size)    calloc(nelem, elem_size)
/** @brief C89 says: if the size is larger the new space is uninitialized. */
#define STF_SYSREALLOC(ptr, size)   realloc(ptr, size)
#define STF_SYSFREE(ptr)            free(ptr)
#define STF_SYSBIGALLOC(size)       STF_SYSMALLOC(size)
#define STF_SYSBIGFREE(ptr)         STF_SYSFREE(ptr)
/** @brief With GCC this aligns the memory */
#define STF_ALIGN(bytes)       __attribute__ ((aligned (bytes)))

/*
 * string operation
 */
#define STF_SYS_STRDUP(ptr)		strdup(ptr)
#define STF_STRCMP(A,B)		strcmp(A,B)
#define STF_STRCPY(dest,src)		strcpy(dest,src)
#define STF_STRNCPY(dest,src,size)	strncpy(dest,src,size)
#define STF_STRLEN(ptr) strlen(ptr)

/*
 * file operation
 */

/*
 * If STF_NO_FSEEK64 is defined then the FSEEK64 is not set
 * This allows the projects to choose if they want FSEEK64 support
 *
 * Similar operation for FTELL64
 */
#if defined(__APPLE__)
#ifndef STF_NO_FTELL64
#define STF_FTELL64 ftello
#endif
#ifndef STF_NO_FSEEK64
#define STF_FSEEK64 fseeko
#endif
#else
#ifndef STF_NO_FTELL64
#define STF_FTELL64 ftello64
#endif
#ifndef STF_NO_FSEEK64
#define STF_FSEEK64 fseeko64
#endif
#endif
/*
 * assert behaviour
 *                               NDEBUG
 *                      0                 1
 * EXIT_ON_ASSERT  0   test||print        void
 *                 1   assert()           assert() // ignored: no print, no exit
 *
 * on android uses the system log: __android_log_assert and __android_log_print
 */

// C89 says: if NDEBUG is defined at the time <assert.h> is included the assert macro is ignored
#ifndef EXIT_ON_ASSERT // NO_EXIT_ON_ASSERT is defined

#ifdef NDEBUG // assert should be ignored
#define STF_ASSERT(expected) ((void)0)
#else

#ifdef __android__
#define STF_ASSERT(expected) (void)( (expected) || (__android_log_print(ANDROID_LOG_ERROR, "STF", "Assertion failed: %s, file %s, line %d\n", #expected, __FILE__, __LINE__),0) )
#else
#define STF_ASSERT(expected) (void)( (expected) || (fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #expected, __FILE__, __LINE__),0) )
#endif

#endif // NDEBUG

#endif // NO_EXIT_ON_ASSERT
#ifndef STF_ASSERT
#ifdef __android__
#define STF_ASSERT(expected) (void)((expected) ? 0 : (__android_log_assert(#expected, "STF", "ERROR: Assert '%s' at %s:%d", #expected, __FILE__,__LINE__),0))
#else
#define STF_ASSERT(expected) assert(expected)
#endif //__android__
#endif // not def STF_ASSERT

/**
 * @brief Cast a member of a structure out to the containing structure
 * @param ptr the pointer to the member.
 * @param type the type of the container struct.
 * @param member the name of the member within the container struct.
 */
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))


#ifdef __cplusplus
}
#endif


#endif // __STF_SYSDEFS_H__
