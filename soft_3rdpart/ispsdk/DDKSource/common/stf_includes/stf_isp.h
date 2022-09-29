/**
  ******************************************************************************
  * @file  stf_isp.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  12/14/2019
  * @brief  ISP registers operation macro
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STARFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai StarFive</center></h2>
  */
#ifndef __STF_ISP_H__
#define __STF_ISP_H__


#include <linux/version.h>


#ifdef UNUSED_CODE_AND_VARIABLE
#define __FreeRTOS__


#endif //UNUSED_CODE_AND_VARIABLE
/** @brief major */
#define STF_ISP_SDK_VERSION_MAJ 2u
/** @brief minor */
#define STF_ISP_SDK_VERSION_MIN 0u
/** @brief maintenance */
#define STF_ISP_SDK_VERSION_MNT 1u
/** @brief build */
#define STF_ISP_SDK_VERSION_BLD 0u
#define ISP_SETTING_VERSION                     (0x00010000)


#ifdef UNUSED_CODE_AND_VARIABLE
#if defined(__FreeRTOS__)
#define DIRECT_IO_MAPPING
#elif defined(__linux__)
#define VIRTUAL_IO_MAPPING
#else
#define DIRECT_IO_MAPPING
#endif
#else //UNUSED_CODE_AND_VARIABLE
#define VIRTUAL_IO_MAPPING
#endif //UNUSED_CODE_AND_VARIABLE


//=============================================================================
#if defined(RISCV)
  #if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
#define DMA_NON_CACHE_ADDRESS_MAPPING
  #else
    #if defined(V4L2_DRIVER)
    #else
#define DMA_NON_CACHE_FLUSH
#define DMA_NON_CACHE_MEM_MAX                   (10 * 2)
    #endif //#if defined(V4L2_DRIVER)
  #endif //#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
#endif //#if defined(RISCV)
//=============================================================================
#define SUPPORT_RDMA_FEATURE
//=============================================================================
#define ISP_AUTO_ADJUST_COPY_SIZE_IF_SIZE_LARGE_THEN_STRUCT_WHILE_SET_PARAM_AND_SETTING
//=============================================================================
//#define ENABLE_DOUBLE_POINTER_TO_CALCULATE
//=============================================================================
#define PIPELINE_ALLOCATE_IMG_BUF
//#define PIPELINE_BUF_PUSH_POP_DISABLE
//#define TRIGGER_SHADOW_AFTER_CONFIG_BUFFER
//#define ISP_MODULE_DISABLE_WHEN_NO_SHOT_BUF
//#define DEBUG_SHOT_BUF_PUDH_AND_POP
//#define DEBUG_SHOT_BUF_PUSH_AND_POP_FLOW
#define ENABEL_QUEUE_CNT_IS_0_ERROR_MESSAGE_WHILE_SHOT_BUF_POP
#define USE_MUTEX_PROTECT_PIPELINE_BUF
#define USE_MUTEX_PROTECT_MODULE_CONTROL_PARAM
//=============================================================================
#ifdef UNUSED_CODE_AND_VARIABLE
// Following are three trigger methods for pending buffer switch to complete queue.
// Just choose one method for switch pending buffer to complete queue.
#define PENDING_BUF_SWITCH_TO_COMPLETE_QUEUE_BY_ITI_WRITE_FRAME_END
//#define PENDING_BUF_SWITCH_TO_COMPLETE_QUEUE_BY_INT_MUX
//#define PENDING_BUF_SWITCH_TO_COMPLETE_QUEUE_BY_IFRE
#endif //UNUSED_CODE_AND_VARIABLE
//=============================================================================
#define FREE_RESOURCE_ENABLE
#ifdef UNUSED_CODE_AND_VARIABLE
#define TIL_ONE_BUFFER
//#define TIL_FOUR_BUFFER
#define ISP_MODULE_CLOCK_GATE_CONTROL
//=============================================================================
// Following are tiling write output formate.
//#define TILING_OUTPUT_FORMAT_FLEXIBLE_LSB
//-----------------------------------------------------------------------------
#define TILING_OUTPUT_FORMAT_YUV420_SEMI
//#define RAW_YUV_10BIT
#define RAW_YUV_8BIT
//=============================================================================
//#define RDMA_CHECK_REG_SIZE
#define ISP_REGISTER_PROGRAM_BY_RDMA
//#define ISP_NOT_ADD_ALL_OF_USED_MODULES_TO_PIPELINE
//#define ISP_PIPELINE_ADD_ENABLE_MODULES_AND_CONTROLS
//#define ISP_PIPELINE_REMOVE_DISABLE_MODULES_AND_CONTROLS
#endif //UNUSED_CODE_AND_VARIABLE
//=============================================================================
//#define ISP_CCM_MIN_MATRIX_SUPPORT
//=============================================================================
#define ISP_SWITCH_SC_INPUT_SEL
//#define ISP_AE_AWB_USE_FRAME_COUNT_3_FRAME_CYCLE
#define ISP_AE_AWB_USE_FRAME_COUNT_4_FRAME_CYCLE
//#define ISP_AE_AWB_USE_FRAME_COUNT_6_FRAME_CYCLE
#define ISP_AWB_CAL_AT_LAST_FRAME_CYCLE
#define ISP_ADC_GAIN_USING_AWB_SIM
//#define ISP_AWB_SUPPORT_D75
//-----------------------------------------------------------------------------
//#define FOR_GENERIC_LENS
//#define FOR_SC2235_SENSOR_LENS_0
#define FOR_SC2235_SENSOR_LENS_1
//=============================================================================
#define SPECIAL_MMAP_ID (987654321)
//=============================================================================
#define SUPPORT_MULTI_ISP
//#define ENABLE_CONTIGUOUS_CAPTURE
#if defined(ENABLE_CONTIGUOUS_CAPTURE)
#define CONTIGUOUS_CAPTURE_BUF_MAX              (100)
#else
#define CONTIGUOUS_CAPTURE_BUF_MAX              (0)
#endif //#if defined(ENABLE_CONTIGUOUS_CAPTURE)
//=============================================================================
#define ENABLE_SET_SHADOW_MODE_SUPPORT
#define EXTEND_IOCTL_COMMAND_BASE
#if defined(V4L2_DRIVER)
#define EXTEND_IOCTL_COMMAND_SUD_CSI
#define EXTEND_IOCTL_COMMAND_SUD_ISP
#define EXTEND_IOCTL_COMMAND_CSI_IN
#define EXTEND_IOCTL_COMMAND_CBAR
#define EXTEND_IOCTL_COMMAND_CROP
#define EXTEND_IOCTL_COMMAND_DC
#define EXTEND_IOCTL_COMMAND_DEC
#define EXTEND_IOCTL_COMMAND_OBA
#define EXTEND_IOCTL_COMMAND_OBC
#define EXTEND_IOCTL_COMMAND_LCBQ
#define EXTEND_IOCTL_COMMAND_SC
#define EXTEND_IOCTL_COMMAND_DUMP
#define EXTEND_IOCTL_COMMAND_ISP_IN
#define EXTEND_IOCTL_COMMAND_DBC
#define EXTEND_IOCTL_COMMAND_CTC
#define EXTEND_IOCTL_COMMAND_OECF
#define EXTEND_IOCTL_COMMAND_OECFHM
#define EXTEND_IOCTL_COMMAND_LCCF
#define EXTEND_IOCTL_COMMAND_AWB
#define EXTEND_IOCTL_COMMAND_CFA
#define EXTEND_IOCTL_COMMAND_CAR
#define EXTEND_IOCTL_COMMAND_CCM
#define EXTEND_IOCTL_COMMAND_GMARGB
#define EXTEND_IOCTL_COMMAND_R2Y
#define EXTEND_IOCTL_COMMAND_YHIST
#define EXTEND_IOCTL_COMMAND_YCRV
#define EXTEND_IOCTL_COMMAND_SHRP
#define EXTEND_IOCTL_COMMAND_DNYUV
#define EXTEND_IOCTL_COMMAND_SAT
#define EXTEND_IOCTL_COMMAND_OUT
#define EXTEND_IOCTL_COMMAND_OUT_UO
#define EXTEND_IOCTL_COMMAND_OUT_SS0
#define EXTEND_IOCTL_COMMAND_OUT_SS1
#define EXTEND_IOCTL_COMMAND_TIL
#define EXTEND_IOCTL_COMMAND_BUF
#endif //#if defined(V4L2_DRIVER)
//=============================================================================


#define PIPELINE_IMG_BUF_MAX                    (6)
#define PIPELINE_IMG_BUF_MAX_TOTAL              (PIPELINE_IMG_BUF_MAX + CONTIGUOUS_CAPTURE_BUF_MAX)
#define ISO_LEVEL_MAX                           (10)
#define TEMP_LEVEL_MAX                          (6)
#define ISP_CAL_SKIP_FRAME_CNT                  (3)
//=============================================================================
#define ISP_LOOP_MAX_EXPECTED_BLOCK_TIME        (10)    // 10ms
//=============================================================================


#ifndef __STF_SYSTYPES_H__
//-----------------------------------------------------------------------------
#if 0
typedef char                    STF_CHAR;
typedef s16                     STF_WCHAR;      // STF_WCHAR have defined on stf_systypes.h header file.

typedef int                     STF_INT;
typedef s8                      STF_S8;
typedef s16                     STF_S16;
typedef s32                     STF_S32;
typedef s64                     STF_S64;

typedef unsigned int            STF_UINT;
typedef u8                      STF_U8;
typedef u16                     STF_U16;
typedef u32                     STF_U32;
typedef u64                     STF_U64;

/*
 * memory related
 */
typedef u8                      STF_BYTE; /**< @brief Atom of memory */
#else
typedef char                    STF_CHAR;
typedef signed short            STF_WCHAR;      // STF_WCHAR have defined on stf_systypes.h header file.

typedef int                     STF_INT;
typedef signed char             STF_S8;
typedef signed short            STF_S16;
typedef signed int              STF_S32;
typedef signed long long        STF_S64;

typedef unsigned char           STF_U8;
typedef unsigned short          STF_U16;
typedef unsigned int            STF_U32;
typedef unsigned long long      STF_U64;

/*
 * memory related
 */
typedef unsigned char           STF_BYTE; /**< @brief Atom of memory */
#endif

/**
 * @brief Unsigned integer returned by sizeof operator (i.e. big enough to
 * hold any memory allocation) (C89)
 */
typedef size_t                  STF_SIZE;
/** @brief Integer vairable that can hold a pointer value (C99) */
typedef uintptr_t               STF_UINTPTR;
/**
 * @brief Large enought to hold the signed difference of 2 pointer
 * values (C89)
 */
typedef ptrdiff_t               STF_PTRDIFF;

#endif //__STF_SYSTYPES_H__
//-----------------------------------------------------------------------------
typedef STF_WCHAR               *STF_PWCHAR;

typedef STF_S8                  *STF_PS8;
typedef STF_S16                 *STF_PS16;
typedef STF_S32                 *STF_PS32;
typedef STF_S64                 *STF_PS64;

typedef unsigned int            *STF_PUINT;
typedef STF_U8                  *STF_PU8;
typedef STF_U16                 *STF_PU16;
typedef STF_U32                 *STF_PU32;
typedef STF_U64                 *STF_PU64;

/*
 * Typedefs of void are synonymous with the void keyword in C,
 * but not in C++. In order to support the use of STF_VOID
 * in place of the void keyword to specify that a function takes no
 * arguments, it must be a macro rather than a typedef.
 */
#if 1
#define STF_VOID                void
#else
typedef void                    STF_VOID;
#endif
typedef void                    *STF_PVOID;
typedef void                    *STF_HANDLE;
typedef STF_S32                 STF_RESULT;

/*
 * integral types that are not system specific
 */
#ifndef __STF_SYSTYPES__
typedef char                    STF_CHAR;
typedef int                     STF_INT;
typedef unsigned int            STF_UINT;
#endif //__STF_SYSTYPES__
typedef int                     STF_BOOL;

/*
 * boolean
 */
//#ifndef __cplusplus
#define STF_NULL                NULL
//#else
//#define STF_NULL 0
//#endif

typedef STF_U8                  STF_BOOL8;

#define STF_FALSE               (0) /* STF_FALSE is known to be zero */
#define STF_TRUE                (1) /* 1 so it's defined and it is not 0 */

#define STF_DISABLE             (0) /* STF_DISABLE is known to be zero */
#define STF_ENABLE              (1) /* 1 so it's defined and it is not 0 */

#define STF_CLR                 (0) /* STF_DISABLE is known to be zero */
#define STF_SET                 (1) /* 1 so it's defined and it is not 0 */

/*
 * floating point
 */
typedef float                   STF_FLOAT;
typedef double                  STF_DOUBLE;
//-----------------------------------------------------------------------------


#if defined(USE_NEW_MEMORY_MANAGER)
#define STF_MALLOC(size)        malloc(size)
#define STF_CALLOC(nelem, elem_size) calloc(nelem, elem_size)
#define STF_FREE(ptr)   \
{                       \
    if (ptr) {          \
        free(ptr);      \
        ptr = NULL;     \
    }                   \
}


#endif //USE_NEW_MEMORY_MANAGER
#define ISP_IMG_WIDTH                   (1920)  /* This constant has to reference to SENSOR_WIDTH. */
#define ISP_IMG_HEIGHT                  (1080)  /* This constant has to reference to SENSOR_HEIGHT. */


#ifdef UNUSED_CODE_AND_VARIABLE
#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(condition)         ((void)sizeof(char[1 - 2*!!(condition)]))
#endif //BUILD_BUG_ON


#endif //UNUSED_CODE_AND_VARIABLE
#define REG24POS                        (24)
#define REG16POS                        (16)
#define REG8POS                         (8)
#define REG4POS                         (4)

//#define ISP_ENABLE                      (1)
//#define ISP_DISABLE                     (0)
//#define ISP_DEFAULT_VALUE               (0)
//
//#define ISP_TRUE                        (1)
//#define ISP_FALSE                       (0)
//
//#define S_OK                            (0x00000000)    /* Operation successful. */
//#define E_ABORT                         (0x80000001)    /* Operation aborted. */
//#define E_ACCESSDENIED                  (0x80000002)    /* General access denied error. */
//#define E_FAIL                          (0x80000003)    /* Unspecified failure. */
//#define E_HANDLE                        (0x80000004)    /* Handle that is not valid. */
//#define E_INVALIDARG                    (0x80000005)    /* One or more arguments are not valid. */
//#define E_NOINTERFACE                   (0x80000006)    /* No such interface supported. */
//#define E_NOTIMPL                       (0x80000007)    /* Not implemented. */
//#define E_OUTOFMEMORY                   (0x80000008)    /* Failed to allocate necessary memory. */
//#define E_POINTER                       (0x80000009)    /* Pointer that is not valid. */
//#define E_UNEXPECTED                    (0x8000000A)    /* Unexpected failure. */

#define LO8(w)                          ((STF_U8)(w))
#define HI8(w)                          ((STF_U8)(((STF_U16)(w) >> 8) & 0xFF))
#define LO16(dw)                        ((STF_U16)(dw))
#define HI16(dw)                        ((STF_U16)(((STF_U32)(dw) >> 16) & 0xFFFF))
#define MAKE16(lb,hb)                   ((STF_U16)(((STF_U8)(lb)) | ((STF_U16)((STF_U8)(hb))) << 8))
#define MAKE32(lw,hw)                   ((STF_U32)(((STF_U16)(lw)) | ((STF_U32)((STF_U16)(hw))) << 16))
#define MAKE64(ldw,hdw)                 ((STF_U64)(((STF_U64)(ldw)) | ((STF_U64)((STF_U64)(hdw))) << 32))


#endif //__STF_ISP_H__
