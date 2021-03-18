/*
 * Copyright (c) 2018, Chips&Media
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _JPU_TYPES_H_
#define _JPU_TYPES_H_

#include <stdint.h>
/**
* @brief    This type is an 8-bit unsigned integral type, which is used for declaring pixel data.
*/
typedef uint8_t             Uint8;

/**
* @brief    This type is a 16-bit unsigned integral type.
*/
typedef uint16_t            Uint16;

/**
* @brief    This type is a 32-bit unsigned integral type, which is used for declaring variables with wide ranges and no signs such as size of buffer.
*/
typedef uint32_t            Uint32;

/**
* @brief    This type is a 64-bit unsigned integral type, which is used for declaring variables with wide ranges and no signs such as size of buffer.
*/
typedef uint64_t            Uint64;

/**
* @brief    This type is an 8-bit signed integral type.
*/
typedef int8_t              Int8;

/**
* @brief    This type is a 16-bit signed integral type.
*/
typedef int16_t             Int16;

/**
* @brief    This type is a 32-bit signed integral type.
*/
typedef int32_t             Int32;

/**
* @brief    This type is a 64-bit signed integral type.
*/
typedef int64_t             Int64;

/**
* @brief
This is a type for representing physical addresses which is recognizable by the JPU. In general,
the JPU hardware does not know about virtual address space which is set and handled by host
processor. All these virtual addresses are translated into physical addresses by Memory Management
Unit. All data buffer addresses such as stream buffer, frame buffer, should be given to
the JPU as an address on physical address space.
*/
typedef uint32_t            PhysicalAddress;

/**
* @brief This type is an 8-bit unsigned character type.
*/
typedef unsigned char       BYTE;

/**
* @brief This type is an 8-bit boolean data type to indicate TRUE or FALSE.
*/
typedef int32_t             BOOL;

#ifndef NULL
#define NULL    0
#endif

#ifndef TRUE
#define TRUE                        1
#endif /* TRUE */

#define STATIC              static

#ifndef FALSE
#define FALSE                       0
#endif /* FALSE */

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          \
    /*lint -save -e527 -e530 */ \
{ \
    (P) = (P); \
} \
    /*lint -restore */
#endif

#endif    /* _JPU_TYPES_H_ */
