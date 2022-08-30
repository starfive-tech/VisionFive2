/** 
 **************************************************************************
 @file           c99/stf_systypes.h

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
#ifndef __STF_SYSTYPES_H__
#define __STF_SYSTYPES_H__


#ifdef __cplusplus
extern "C" {
#endif


/* Posix environment */
#include <stdint.h>
#include <stddef.h> // size_t and ptrdiff_t
#include <wchar.h>

/*
 * integral types
 */

typedef             char    STF_CHAR;
typedef          wchar_t    STF_WCHAR;

typedef	             int	STF_INT;
typedef	          int8_t	STF_S8;
typedef	         int16_t	STF_S16;
typedef	         int32_t	STF_S32;
typedef	         int64_t	STF_S64;

typedef	    unsigned int	STF_UINT;
typedef          uint8_t	STF_U8;
typedef	        uint16_t	STF_U16;
typedef	        uint32_t	STF_U32;
typedef	        uint64_t	STF_U64;

/*
 * memory related
 */
typedef	         uint8_t	STF_BYTE;   	/**< @brief Atom of memory */
typedef           size_t    STF_SIZE;     /**< @brief Unsigned integer returned by sizeof operator (i.e. big enough to hold any memory allocation) (C89) */
typedef        uintptr_t    STF_UINTPTR;    /**< @brief Integer vairable that can hold a pointer value (C99) */
typedef        ptrdiff_t    STF_PTRDIFF;    /**< @brief Large enought to hold the signed difference of 2 pointer values (C89) */


#ifdef __cplusplus
}//extern C
#endif


#endif /* __STF_SYSTYPES_H__ */
