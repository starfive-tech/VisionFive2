/** 
 *******************************************************************************
 @file           linux-kernel/stf_systypes.h

 @brief          Base type definitions using linux kernel headers

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license        <Strictly Confidential.>
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.
 
 ******************************************************************************/
#ifndef __STF_SYSTYPES_H__
#define __STF_SYSTYPES_H__


#ifdef __cplusplus
// this could even be erroneous because that's a kernel header!
extern "C" {
#endif


/* Posix environment */
#include <linux/stddef.h>  // size_t and ptrdiff_t
#include <linux/types.h>

/*
 * integral types
 */

typedef             char    STF_CHAR;
typedef              s16    STF_WCHAR;

typedef              int    STF_INT;
typedef               s8    STF_S8;
typedef              s16    STF_S16;
typedef              s32    STF_S32;
typedef              s64    STF_S64;

typedef     unsigned int    STF_UINT;
typedef               u8    STF_U8;
typedef              u16    STF_U16;
typedef              u32    STF_U32;
typedef              u64    STF_U64;

/*
 * memory related
 */
typedef               u8    STF_BYTE; /**< @brief Atom of memory */
/**
 * @brief Unsigned integer returned by sizeof operator (i.e. big enough to
 * hold any memory allocation) (C89)
 */
typedef           size_t    STF_SIZE;
/** @brief Integer vairable that can hold a pointer value (C99) */
typedef        uintptr_t    STF_UINTPTR;
/**
 * @brief Large enought to hold the signed difference of 2 pointer
 * values (C89)
 */
typedef        ptrdiff_t    STF_PTRDIFF;


#ifdef __cplusplus
}  //extern C
#endif


#endif /* __STF_SYSTYPES_H__ */
