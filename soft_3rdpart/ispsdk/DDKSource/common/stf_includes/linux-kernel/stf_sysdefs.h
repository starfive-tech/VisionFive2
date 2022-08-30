/**
 *******************************************************************************
 @file           linux-kernel/stf_sysdefs.h

 @brief

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license        <Strictly Confidential.>
 No part of this software, either material or conceptual may be copied or
 distributed, transmitted, transcribed, stored in a retrieval system or
 translated into any human or computer language in any form by any means,
 electronic, mechanical, manual or other-wise, or disclosed to third
 parties without the express written permission of
 Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __STF_SYSDEFS_H__
#define __STF_SYSDEFS_H__


#include <linux/string.h>  // strcpy, ... and memcpy, ...
#include <linux/kernel.h> 
#include <linux/slab.h>  // kmalloc, kcalloc, ...
#include <linux/vmalloc.h>
// to check if in interrupt context when doing kmalloc
#include <linux/interrupt.h>
#include <asm-generic/bug.h>  // WARN_ON used in STF_ASSERT


#ifdef __cplusplus
// this could even be erroneous because that's a kernel header!
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
//#define STF_EXPORT    __declspec(dllexport)

// not very nice
#define STF_LITTLE_ENDIAN (1234)
#define STF_BIG_ENDIAN (4321)
#define STF_BYTE_ORDER STF_LITTLE_ENDIAN

/** @brief 64bit value prefix - e.g. printf("long %" STF_I64PR "d") */
#ifdef __GNUC__
#  ifdef __LP64__
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
/**
 * @brief STF_SIZE (i.e. size_t) format prefix
 * - e.g. STF_SIZE s -> printf("sizeof= %" STF_SIZEPR "u", s)
 */
#define STF_SIZEPR "z"
/**
 * @brief STF_PTRDIFF (i.e. ptrdiff_t) format prefix (in hexadecimal)
 * - e.g. STF_PTRDIFF diff -> printf(diff= 0x%" STF_PTRDPR "x", s)
 */
#define STF_PTRDPR "t"

/*
 * memory operation
 */

/**
 * @brief kmalloc flags used when calling kmalloc in STF_MALLOC
 *
 * The two most common flags are:
 * @li GFP_ATOMIC is high priority and cannot sleep (use in interrupt
 * handlers and bottom halves) when using a spin lock. As the kernel cannot
 * put the process to sleep it cannot swap pages etc to maybe make memory
 * available therefore it has more chances to fail. Therefore it should be
 * used only in the contexts where memory allocation must be done very fast
 * in a small amount.
 * @li GFP_KERNEL is a normal priority allocation that can sleep, therefore
 * the kernel can liberate memory if possible (unused pages etc).
 *
 * Therefore GFP_ATOMIC usage should be avoided in most of the cases.
 * It is sensible however to AVOID allocation in the cases GFP_ATOMIC should
 * be used.
 *
 * The allocation checks if the requiered memory fits in a page.
 * If it does not then malloc is attempted but warning is printed).
 */
#define STF_KMALLOC_FLAG GFP_KERNEL

#define STF_MEMCPY(dest,src,size) memcpy(dest,src,size)
#define STF_MEMSET(ptr,val,size) memset(ptr,val,size)
#define STF_MEMCLR(ptr,size) memset(ptr,0x00,size)
#define STF_MEMCMP(A,B,size) memcmp(A,B,size)
#define STF_MEMMOVE(dest,src,size) memmove(dest,src,size)

/** @return 0 if correct, more than 0 otherwise */
inline static int stf_verifalloc(size_t size, const char* fct,
    const char* file, uint32_t line, int verif)
{
    int ret = 0;
    if (irqs_disabled())
    {
        printk(KERN_WARNING "WARNING: %s with irq disabled! %s:%u",
            fct, file, line);
        if (STF_KMALLOC_FLAG != GFP_ATOMIC)
        {
            dump_stack();
            ret++; // so it only prints the warning and does not allocate
        }
    }
    if (0 == size)
    {
        printk(KERN_ERR \
            "ERROR: %s (%s:%u) tries to allocate a 0 sized buffer!",
            fct, file, line);
    }
    if (verif && size > 2 * PAGE_SIZE)
    {
        /* kernel buddy allocator (on kernels >2.6.35) does not suffer
         * from bad fragmentation. */
        // 2 contiguous pages are not very difficult to find.
        printk(KERN_INFO \
            "WARNING: %s (%zu) > 2 pages. Maybe use vmalloc. %s:%u",
            fct, size, file, line);
    }
    return ret;
}

/** @brief C89 says: space is unitialized */
inline static void* stf_sys_malloc(size_t size,
    const char *fct, uint32_t line)
{
    void* ptr = NULL;
    if (!stf_verifalloc(size, "kmalloc", fct, line, 1))
    {
        ptr = kmalloc(size, STF_KMALLOC_FLAG);
    }
    return ptr;
}
#define STF_SYSMALLOC(size) stf_sys_malloc(size, __FILE__, __LINE__)

/** @brief C89 says: space is initialized to zero bytes */
inline static void* stf_sys_calloc(size_t nelem, size_t elem_size,
    const char *fct, uint32_t line)
{
    void* ptr = NULL;
    if (!stf_verifalloc(nelem*elem_size, "kcalloc", fct, line, 1))
    {
        ptr = kcalloc(nelem, elem_size, STF_KMALLOC_FLAG);
    }
    return ptr;
}
#define STF_SYSCALLOC(nelem, elem_size) stf_sys_calloc(nelem, elem_size, \
    __FILE__, __LINE__)

/** @brief C89 says: if the size is larger the new space is uninitialized. */
inline static void* stf_sys_realloc(void *ptr, size_t size, const char *fct,
    uint32_t line)
{
    void* reptr = NULL;
    if (!stf_verifalloc(size, "krealloc", fct, line, 1))
    {
        reptr = krealloc(ptr, size, STF_KMALLOC_FLAG);
    }
    return reptr;
}
#define STF_SYSREALLOC(ptr, size) stf_sys_realloc(ptr, size, __FILE__, __LINE__)

/**
 * @brief In the kernel allocation that are more than a few page (1 or 2)
 * should use vmalloc because it allows fragmented memory
 */
inline static void* stf_sys_vmalloc(size_t size, const char *fct,
    uint32_t line)
{
    void* ptr = NULL;
    if (!stf_verifalloc(size, "vmalloc", fct, line, 0))
    {
#if 1
        ptr = vmalloc(size);
#else
        ptr = vzalloc(size);
#endif
    }
    return ptr;
}
#define STF_SYSBIGALLOC(size) stf_sys_vmalloc(size, __FILE__, __LINE__)

#define STF_SYSFREE(ptr) kfree(ptr)
#define STF_SYSBIGFREE(ptr) vfree(ptr)
/** @brief With GCC this aligns the memory */
#define STF_ALIGN(bytes) __attribute__ ((aligned (bytes)))

/*
 * string operation
 */

inline static char* stf_sys_strdup(char *ptr, const char *fct, uint32_t line)
{
    char *str = NULL;
    if (!stf_verifalloc(strlen(ptr), "kstrdup", fct, line, 1))
    {
        str = kstrdup(ptr, STF_KMALLOC_FLAG);
    }
    return str;
}
#define STF_SYS_STRDUP(ptr) stf_sys_strdup(ptr, __FILE__, __LINE__)

#define STF_STRCMP(A,B) strcmp(A,B)
#define STF_STRCPY(dest,src) strcpy(dest,src)
#define STF_STRNCPY(dest,src,size) strncpy(dest,src,size)
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

// force to NO
#define STF_NO_FTELL64
// force to NO
#define STF_NO_FSEEK64


/*
 * assert behaviour
 *                               NDEBUG
 *                         0            1
 * EXIT_ON_ASSERT  0   test||print   void
 *                 1   assert()      assert() // ignored no print, no exit
 */

/* C89 says: if NDEBUG is defined at the time <assert.h> is included the
 *assert macro is ignored */
#ifdef EXIT_ON_ASSERT 

#define STF_ASSERT(expected) ({WARN_ON(!(expected));0;})

#else // NO_EXIT_ON_ASSERT is defined

#ifdef NDEBUG // assert should be ignored
#define STF_ASSERT(expected) ({0;})
#else
#define STF_ASSERT(expected) (void)( (expected) \
    || (printk(KERN_ERR "Assertion failed: %s, file %s, line %d\n", \
    #expected, __FILE__, __LINE__), dump_stack(),0) )
#endif

/*
 * Do not #define container_of because the linux kernel defines it
 */

#endif  // NO_EXIT_ON_ASSERT


#ifdef __cplusplus
}
#endif


#endif  /* __STF_SYSDEFS_H__ */
