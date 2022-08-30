/** 
 **************************************************************************
 @file           ms/stf_systypes.h

 @brief          Base type definitions using Microsoft headers

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


/* Windows environment */
#include <stddef.h>

/*
 * integral types
 */

typedef             char    STF_CHAR;
typedef          wchar_t    STF_WCHAR;

typedef	             int	STF_INT;
typedef	          __int8	STF_S8;
typedef	         __int16	STF_S16;
typedef	         __int32	STF_S32;
typedef	         __int64	STF_S64;

typedef	    unsigned int	STF_UINT;
typedef	 unsigned __int8	STF_U8;
typedef	unsigned __int16	STF_U16;
typedef	unsigned __int32	STF_U32;
typedef	unsigned __int64	STF_U64;

/*
 * memory related
 */
typedef	 unsigned __int8	STF_BYTE;   	/**< @brief Atom of memory */
typedef           size_t    STF_SIZE;     /**< @brief Unsigned integer returned by sizeof operator (i.e. big enough to hold any memory allocation) (C89) */
typedef        uintptr_t    STF_UINTPTR;    /**< @brief Integer vairable that can hold a pointer value (C99) */
typedef        ptrdiff_t    STF_PTRDIFF;    /**< @brief Large enought to hold the signed difference of 2 pointer values (C89) */


#ifdef __cplusplus
}//extern C
#endif


#endif /* __STF_SYSTYPES_H__ */
