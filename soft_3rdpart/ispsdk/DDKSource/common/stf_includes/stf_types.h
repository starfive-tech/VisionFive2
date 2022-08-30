/** 
 **************************************************************************
 @file           stf_types.h

 @brief          Typedefs based on the basic STF types

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license        <Strictly Confidential.>
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.
 
 **************************************************************************/

#ifndef __STF_TYPES_H__
#define __STF_TYPES_H__


#ifndef SIFIVE_ISP
#define SIFIVE_ISP
#endif //SIFIVE_ISP
//=============================================================================
#ifndef V4L2_DRIVER
#define V4L2_DRIVER
#endif //#ifndef V4L2_DRIVER
//#define TEST_ON_UBUNTU
//#define USE_LINUX_SYSTEM_STARTAND_I2C
//#define ENABLE_DEBUG_DEVICE_POINTER_ADDR
//#define DISABLE_IRQ_SUPPORT
//#define ENABLE_USING_MEM_TO_SIM_REG
//#define DISABLE_MEM_ALLOC_SUPPORT
//#define DMA_MEM_ALLOC_HAVE_TO_PASS_DEVICE_POINTER
//#define DMA_MEM_ALLOC_HAVE_TO_CALL_DMA_DECLARE_COHERENT_MEMORY
#if defined(TEST_ON_UBUNTU)
//=============================================================================
  #if !defined(USING_PRINTK_OUTPUT_DEBUG_INFO)
#define USING_PRINTK_OUTPUT_DEBUG_INFO
  #endif //USING_PRINTK_OUTPUT_DEBUG_INFO
  #if !defined(DISABLE_IRQ_SUPPORT)
#define DISABLE_IRQ_SUPPORT
  #endif //DISABLE_IRQ_SUPPORT
  #if !defined(ENABLE_USING_MEM_TO_SIM_REG)
#define ENABLE_USING_MEM_TO_SIM_REG
  #endif //ENABLE_USING_MEM_TO_SIM_REG
//=============================================================================
#else //TEST_ON_UBUNTU
//=============================================================================
//#define THROUGH_CHIP_LINK
#define ENABLE_DTS_SUPPORT
//#define USING_OF_RESERVE_MEMORY
//-----------------------------------------------------------------------------
  #if !defined(DISABLE_IRQ_SUPPORT) && defined(THROUGH_CHIP_LINK)
#define USING_UNLEASH_GPIO_FOR_IRQ
  #endif //#if !defined(DISABLE_IRQ_SUPPORT) && defined(THROUGH_CHIP_LINK)
  #if defined(ENABLE_DTS_SUPPORT)
    #if !defined(DMA_MEM_ALLOC_HAVE_TO_PASS_DEVICE_POINTER)
#define DMA_MEM_ALLOC_HAVE_TO_PASS_DEVICE_POINTER
    #endif //#if !defined(DMA_MEM_ALLOC_HAVE_TO_PASS_DEVICE_POINTER)
    #if defined(DMA_MEM_ALLOC_HAVE_TO_CALL_DMA_DECLARE_COHERENT_MEMORY)
#undef DMA_MEM_ALLOC_HAVE_TO_CALL_DMA_DECLARE_COHERENT_MEMORY
    #endif //DMA_MEM_ALLOC_HAVE_TO_CALL_DMA_DECLARE_COHERENT_MEMORY
  #endif //ENABLE_DTS_SUPPORT
#define USE_CD_SYSTEM_LIB
  #if defined(V4L2_DRIVER)
    #if !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
#define USE_LINUX_SYSTEM_STARTAND_I2C
    #endif //#if !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    #if defined(USE_CD_SYSTEM_LIB)
#undef USE_CD_SYSTEM_LIB
    #endif //#if defined(USE_CD_SYSTEM_LIB)
    #define ENABLE_DMABUF_METHOD
    #define ENABLE_DRM_METHOD
    #define ENABLE_DRM_ALLOC_DMABUF
    #define ENABLE_DRM_MMAP_PING_PONG_BUF
  #endif //V4L2_DRIVER
//=============================================================================
#endif //TEST_ON_UBUNTU
#if defined(ENABLE_USING_MEM_TO_SIM_REG)
#define PESUDO_REG_SIZE                 (0x00010000)
#endif //ENABLE_USING_MEM_TO_SIM_REG
//=== Godspeed === Waiting to move below MARCO into stf_isp.h.

//=== Godspeed === Waiting to move below MARCO into cmake and/or make file.
#define SIFIVE_ISP_TUNING

#ifdef __CDT_PARSER__
// indexer only code
#include "linux-kernel/stf_sysdefs.h"
#include "linux-kernel/stf_systypes.h"
#endif //#ifdef __CDT_PARSER__
#include "stf_systypes.h" // system specific type definitions
#include "stf_isp.h"


#pragma pack(push, 1)
typedef union _ST_VER {
    STF_U64 u64Version;
    struct {
        STF_U64 Build :16;
        STF_U64 Revision : 16;
        STF_U64 Minor : 8;
        STF_U64 Major : 8;
    } field;
} ST_VER, *PST_VER;

typedef struct _ST_CROP {
    STF_U16 u16HStart;
    STF_U16 u16VStart;
    STF_U16 u16HEnd;
    STF_U16 u16VEnd;
} ST_CROP, *PST_CROP;

typedef struct _ST_DEC {
    STF_U8 u8HPeriod;
    STF_U8 u8HKeep;
    STF_U8 u8VPeriod;
    STF_U8 u8VKeep;
} ST_DEC, *PST_DEC;

typedef struct _ST_RECT
{
    STF_U16 u16Left;
    STF_U16 u16Top;
    STF_U16 u16Right;
    STF_U16 u16Bottom;
} ST_RECT, *PST_RECT;

typedef struct _ST_RECT_2
{
    STF_U16 u16Left;
    STF_U16 u16Top;
    STF_U16 u16Width;
    STF_U16 u16Height;
} ST_RECT_2, *PST_RECT_2;

typedef struct _ST_RANGE
{
    STF_U16 u16Low;
    STF_U16 u16Upper;
} ST_RANGE, *PST_RANGE;

typedef struct _ST_SIZE
{
    STF_U16 u16Cx;
    STF_U16 u16Cy;
} ST_SIZE, *PST_SIZE;

typedef struct _ST_POINT
{
    STF_U16 u16X;
    STF_U16 u16Y;
} ST_POINT, *PST_POINT;
#pragma pack(pop)


#define MAX_RES (5)

typedef struct _ST_RES {
    STF_U16 u16W;
    STF_U16 u16H;
} ST_RES, *PST_RES;

typedef struct _ST_RES_LIST {
    ST_RES stResolution[MAX_RES];
    STF_U16 u16Count;
} ST_RES_LIST, *PST_RES_LIST;

typedef union _ST_VAR {
    STF_S64 s64Value;
    STF_U64 u64Value;
    STF_DOUBLE dValue;
    STF_S32 s32Value;
    STF_U32 u32Value;
    STF_FLOAT fValue;
    STF_S16 s16Value;
    STF_U16 u16Value;
    STF_S8 s8Value;
    STF_U8 u8Value;
    STF_CHAR cValue;
    STF_BOOL8 bValue;
    STF_CHAR *pszValue;
} ST_VAR, *PST_VAR;


#endif /* __STF_TYPES_H__ */
