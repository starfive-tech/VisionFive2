//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#ifndef _VPU_TYPES_H_
#define _VPU_TYPES_H_

#include <stdint.h>

/**
* @brief    This type is an 8-bit unsigned integral type, which is used for declaring pixel data.
*/
typedef uint8_t         Uint8;

/**
* @brief    This type is a 32-bit unsigned integral type, which is used for declaring variables with wide ranges and no signs such as size of buffer.
*/
typedef uint32_t        Uint32;

/**
* @brief    This type is a 16-bit unsigned integral type.
*/
typedef uint16_t        Uint16;

/**
* @brief    This type is an 8-bit signed integral type.
*/
typedef int8_t          Int8;

/**
* @brief    This type is a 32-bit signed integral type.
*/
typedef int32_t         Int32;

/**
* @brief    This type is a 16-bit signed integral type.
*/
typedef int16_t         Int16;
#if defined(_MSC_VER)
typedef unsigned __int64 Uint64;
typedef __int64          Int64;
#else
typedef uint64_t        Uint64;
typedef int64_t         Int64;
#endif
#ifndef PhysicalAddress
/**
* @brief    This is a type for representing physical addresses which are recognizable by VPU. 
In general, VPU hardware does not know about virtual address space 
which is set and handled by host processor. All these virtual addresses are 
translated into physical addresses by Memory Management Unit. 
All data buffer addresses such as stream buffer and frame buffer should be given to
VPU as an address on physical address space.
*/
typedef Uint32 PhysicalAddress;
#endif
#ifndef BYTE
/**
* @brief This type is an 8-bit unsigned integral type.
*/
typedef unsigned char   BYTE;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE            1
#endif /* TRUE */
#ifndef FALSE
#define FALSE           0
#endif /* FALSE */
#ifndef NULL
#define NULL	0
#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          \
    /*lint -save -e527 -e530 */ \
{ \
    (P) = (P); \
} \
    /*lint -restore */
#endif

#ifdef __GNUC__
#define UNREFERENCED_FUNCTION __attribute__ ((unused))
#else
#define UNREFERENCED_FUNCTION
#endif

#endif	/* _VPU_TYPES_H_ */
 
