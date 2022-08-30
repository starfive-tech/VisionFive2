/**
 *******************************************************************************
  @file ci_api_structs.h

  @brief User-side driver structures

  @copyright StarFive Technology Co., Ltd. All Rights Reserved.

  @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __CI_API_STRUCTS_H__
#define __CI_API_STRUCTS_H__


#include <pthread.h>

#include "stf_types.h"
#include "stf_defs.h"

#include "sys/sys_userio.h"
#include "stf_common/pixel_format.h"
#include "ci/ci_modules_structs.h"
#include "mantis_hw_info.h"  // NOLINT
#include "registers/isp_reg.h"
#include "registers/stf_isp_rdma.h"
#include "registers/stf_isp_base_reg.h"
#include "registers/stf_isp_sud_csi_rdma.h"
#include "registers/stf_isp_sud_isp_rdma.h"
#include "registers/stf_isp_csi_in_rdma.h"
#include "registers/stf_isp_cbar_rdma.h"
#include "registers/stf_isp_crop_rdma.h"
#include "registers/stf_isp_dc_rdma.h"
#include "registers/stf_isp_dec_rdma.h"
#include "registers/stf_isp_oba_rdma.h"
#include "registers/stf_isp_obc_rdma.h"
#include "registers/stf_isp_lcbq_rdma.h"
#include "registers/stf_isp_sc_rdma.h"
#include "registers/stf_isp_dump_rdma.h"
#include "registers/stf_isp_isp_in_rdma.h"
#include "registers/stf_isp_dbc_rdma.h"
#include "registers/stf_isp_ctc_rdma.h"
#include "registers/stf_isp_oecf_rdma.h"
#include "registers/stf_isp_oecfhm_rdma.h"
#include "registers/stf_isp_lccf_rdma.h"
#include "registers/stf_isp_awb_rdma.h"
#include "registers/stf_isp_cfa_ahd_rdma.h"
#include "registers/stf_isp_car_rdma.h"
#include "registers/stf_isp_ccm_rdma.h"
#include "registers/stf_isp_gmargb_rdma.h"
#include "registers/stf_isp_r2y_rdma.h"
#include "registers/stf_isp_yhist_rdma.h"
#include "registers/stf_isp_ycrv_rdma.h"
#include "registers/stf_isp_shrp_rdma.h"
#include "registers/stf_isp_dnyuv_rdma.h"
#include "registers/stf_isp_sat_rdma.h"
#include "registers/stf_isp_out_rdma.h"
#include "registers/stf_isp_til_rdma.h"
#include "registers/stf_isp_buf_rdma.h"


#ifdef __cplusplus
extern "C" {
#endif


//=== Godspeed === Add new memory/buffer type support here.
/**
 * @brief Access the SC dump hw structure size in Bytes
 */
#define MANTIS_SC_AE_BYTE_SIZE       (((4 / 2) * 8 * 16) * 16)
#define MANTIS_SC_AF_BYTE_SIZE       (((2 / 2) * 8 * 16) * 16)
#define MANTIS_SC_AWB_BYTE_SIZE      (((10 / 2) * 8 * 16) * 16)
#define MANTIS_SC_HIST_BYTE_SIZE     (((4 / 2) * 8 * 64) * 1)
//#if defined(SUPPORT_VIC_2_0)
#define MANTIS_SC_AWB_WGS_BYTE_SIZE  (((2 / 2) * 8 * 256) * 1)
//#endif //#if defined(SUPPORT_VIC_2_0)
#if 0//defined(SUPPORT_VIC_1_0)
#define HW_CI_ScDumpSize()                                      \
({                                                              \
    (MANTIS_SC_AE_BYTE_SIZE                                     \
    + MANTIS_SC_AF_BYTE_SIZE                                    \
    + MANTIS_SC_AWB_BYTE_SIZE                                   \
    + MANTIS_SC_HIST_BYTE_SIZE                                  \
    );                                                          \
})
#elif 0//defined(SUPPORT_VIC_2_0) //#if defined(SUPPORT_VIC_2_0)
#define HW_CI_ScDumpSize()                                      \
({                                                              \
    (MANTIS_SC_AE_BYTE_SIZE                                     \
    + MANTIS_SC_AF_BYTE_SIZE                                    \
    + MANTIS_SC_AWB_BYTE_SIZE                                   \
    + MANTIS_SC_HIST_BYTE_SIZE                                  \
    + MANTIS_SC_AWB_WGS_BYTE_SIZE                               \
    );                                                          \
})
#else
#define HW_CI_ScDumpSize()                                      \
({                                                              \
    (MANTIS_SC_AE_BYTE_SIZE                                     \
    + MANTIS_SC_AF_BYTE_SIZE                                    \
    + MANTIS_SC_AWB_BYTE_SIZE                                   \
    + MANTIS_SC_HIST_BYTE_SIZE                                  \
    + MANTIS_SC_AWB_WGS_BYTE_SIZE                               \
    );                                                          \
})
#endif //#if defined(SUPPORT_VIC_1_0)

/**
 * @brief Access the Y histogram hw structure size in Bytes
 */
#define MANTIS_Y_HIST_BYTE_SIZE      (8 * (64 / 2))
#define HW_CI_YHistSize()                                       \
({                                                              \
    (MANTIS_Y_HIST_BYTE_SIZE);                                  \
})

#define FILENAME_MAX_LEN                        (30)
#define STF_ISP_REG_BUF_SIZE                    (768)
#define STF_ISP_REG_TBL_BUF_SIZE                (STF_ISP_REG_BUF_SIZE / 2)
#define STF_ISP_REG_TBL_2_BUF_SIZE              (STF_ISP_REG_BUF_SIZE / 3)
#define STF_ISP_REG_TBL_3_BUF_SIZE              (STF_ISP_REG_BUF_SIZE / 4)
#define STF_ISP_REG_SMPL_PACK_BUF_SIZE          (STF_ISP_REG_BUF_SIZE / 2)
#define RDMA_WR_ONE                             (0xA0)
#define RDMA_WR_SRL                             (0xA1)
#define RDMA_LINK                               (0xA2)
#define RDMA_SINT                               (0xA3)
#define RDMA_END                                (0xAF)


/**
 * @ingroup CI_API
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the CI_API documentation module
 *---------------------------------------------------------------------------*/

typedef enum _EN_ISP_REG_METHOD {
    EN_ISP_REG_METHOD_ONE_REG = 0,
    EN_ISP_REG_METHOD_SERIES,
    EN_ISP_REG_METHOD_MODULE,
    EN_ISP_REG_METHOD_TABLE,
    EN_ISP_REG_METHOD_TABLE_2,
    EN_ISP_REG_METHOD_TABLE_3,
    EN_ISP_REG_METHOD_SMPL_PACK,
    EN_ISP_REG_METHOD_SOFT_RDMA,
    EN_ISP_REG_METHOD_MAX
} EN_ISP_REG_METHOD, *PEN_ISP_REG_METHOD;


/**
 * @brief Contains information about the currently available hardware
 */

typedef struct CI_HWINFO
{
    /**
     * @name Version attributes
     * @{
     */

#if defined(V4L2_DRIVER)
    /** @brief revision ID */
    STF_U8 rev_u8ID[4];
    /** @brief revision major */
    STF_U8 rev_u8Major;
    /** @brief revision minor */
    STF_U8 rev_u8Minor;
#else
    /**
     * @}
     * @name Configuration attributes
     * @{
     */

    /**
     * @brief Number of bits used at the input of the pipeline after the
     * imager interface block
     */
    STF_U8 config_u8BitDepth;

    /**
     * @brief Size of the HW waiting queue; one per context
     * - see @ref CI_HWINFO::config_u8NContexts
     */
    STF_U8 context_aPendingQueue;

    /**
     * @brief Size of the @ref CI_PIPELINE object the kernel-driver was
     * compiled with - dummy test of driver compatibility
     */
    STF_U32 uiSizeOfPipeline;

    STF_U32 u32RefClockMhz;

    STF_BOOL8 bCapture;
    STF_U32 u32PendingCount;
    STF_U32 u32CompletedCount;
#endif //#if defined(V4L2_DRIVER)

    /**
     * @}
     */
} CI_HWINFO;

/** @brief General information about the CI driver */
typedef struct CI_CONNECTION
{
    /**
     * @brief Information about the hardware loaded from registers
     */
    CI_HWINFO stHWInfo;
    pthread_mutex_t IoctlLock;
#if defined(ENABLE_SYS_IO_OPEN_FUN_PTR)
    SYS_FILE *(*open)(const char *pszDevName, int flags, mode_t mode, void *extra);
#endif //#if defined(ENABLE_SYS_IO_OPEN_FUN_PTR)
    int (*g_fd)(SYS_FILE *pFile);
    int (*close)(SYS_FILE *pFile);
    int (*ioctl)(SYS_FILE *pFile, unsigned int command, long int parameter);
    void *(*mmap)(SYS_FILE *pFile, size_t length, int prot, int flags, off_t offset);
    int (*munmap)(SYS_FILE *pFile, void *addr, size_t length);
} CI_CONNECTION;

/**
 * @brief Used to select the Statistics output of the pipeline
 *
 * @note Similar to the available fields of SAVE_CONFIG_FLAGS register but
 * does not include Display or Encoder output.
 * It does not have to be the same than the register offset
 */
enum CI_SAVE_CONFIG_FLAGS
{
    CI_SAVE_NO_STATS = 0,

    CI_SAVE_SC_AE = 1,
    CI_SAVE_SC_AF = CI_SAVE_SC_AE << 1,
    CI_SAVE_SC_AWB = CI_SAVE_SC_AF << 1,
    CI_SAVE_SC_AE_HIST = CI_SAVE_SC_AWB << 1,
    CI_SAVE_Y_HIST = CI_SAVE_SC_AE_HIST << 1,

    CI_SAVE_ALL = (CI_SAVE_Y_HIST << 1) - 1
};

enum CI_MODULE_UPDATE
{
    CI_UPD_NONE = 0,
    CI_UPD_ALL = (((CI_UPD_NONE + 1) << 1) - 1)
};

/** @brief The module that need access to registers when updated */
#if 0
#define CI_UPD_REG (CI_UPD_LSH | CI_UPD_DPF_INPUT)
#else
//=== Godspeed === waiting for modified
#define CI_UPD_REG (CI_UPD_ALL)
#endif

//=== Godspeed === Add new memory/buffer type support here.
/** @brief The pipeline configuration */
typedef struct CI_PIPELINE_CONFIG
{
    /** @brief Un-scaled stream output format */
    PIXELTYPE eUoType;
    /** @brief Scaled stream 0 output format */
    PIXELTYPE eSs0Type;
    /** @brief Scaled stream 1 output format */
    PIXELTYPE eSs1Type;

    /** @brief CSI dump output format */
    PIXELTYPE eDumpType;

    /** @brief Tiling 1 read format */
    PIXELTYPE eTiling_1_RdType;
    /** @brief Tiling 1 write format */
    PIXELTYPE eTiling_1_WrType;

    /**
     * @brief Configure the wanted output (Statistics, histograms, timestamps,
     * CRC) - accumulation of @ref CI_SAVE_CONFIG_FLAGS
     */
    STF_U32 eOutputConfig;

    /**
     * @brief Written to registers - this value is copied to the shot's
     * equivalent when added to the pending list
     *
     * @note E.g. use this value to monitor which capture's setup was used for
     * a shot.
     *
     * See @ref CI_SHOT::u8PrivateValue
     */
    STF_U8 u8PrivateValue;

    //=== Godspeed === Add new memory/buffer type support here.
    /**
     * @brief Used to un-scaled stream output buffer allocation (in pixels) - further
     * update of UO output size cannot be bigger than that
     */
    STF_U16 u16MaxUoOutWidth;
    /**
     * @brief Used to un-scaled stream output buffer allocation (in pixels) - further
     * update of UO output size cannot be bigger than that
     */
    STF_U16 u16MaxUoOutHeight;

    /**
     * @brief Used to scaled stream 0 output buffer allocation (in pixels) - further
     * update of SS0 output size cannot be bigger than that
     */
    STF_U16 u16MaxSs0OutWidth;
    /**
     * @brief Used to scaled stream 0 output buffer allocation (in pixels) - further
     * update of SS0 output size cannot be bigger than that
     */
    STF_U16 u16MaxSs0OutHeight;

    /**
     * @brief Used to scaled stream 1 output buffer allocation (in pixels) - further
     * update of SS1 output size cannot be bigger than that
     */
    STF_U16 u16MaxSs1OutWidth;
    /**
     * @brief Used to scaled stream 1 output buffer allocation (in pixels) - further
     * update of SS1 output size cannot be bigger than that
     */
    STF_U16 u16MaxSs1OutHeight;

    /**
     * @brief Used to CSI dump output buffer allocation (in pixels) - further
     * update of DUMP output size cannot be bigger than that
     */
    STF_U16 u16MaxDumpOutWidth;
    /**
     * @brief Used to CSI dump output buffer allocation (in pixels) - further
     * update of DUMP output size cannot be bigger than that
     */
    STF_U16 u16MaxDumpOutHeight;

    /**
     * @brief Used to tiling 1 read buffer allocation (in pixels) - further
     * update of TILING_1_RD output size cannot be bigger than that
     */
    STF_U16 u16MaxTiling_1_RdInWidth;
    /**
     * @brief Used to tiling 1 read buffer allocation (in pixels) - further
     * update of TILING_1_RD output size cannot be bigger than that
     */
    STF_U16 u16MaxTiling_1_RdInHeight;

    /**
     * @brief Used to tiling 1 write buffer allocation (in pixels) - further
     * update of TILING_1_WR output size cannot be bigger than that
     */
    STF_U16 u16MaxTiling_1_WrOutWidth;
    /**
     * @brief Used to tiling 1 write buffer allocation (in pixels) - further
     * update of TILING_1_WR output size cannot be bigger than that
     */
    STF_U16 u16MaxTiling_1_WrOutHeight;

} CI_PIPELINE_CONFIG;

/**
 * @brief Statistics configurations
 */
typedef struct CI_PIPELINE_STATS
{
#ifdef UNUSED_CODE_AND_VARIABLE
    CI_MODULE_EXS sExposureStats;
    CI_MODULE_FOS sFocusStats;
    CI_MODULE_WBS sWhiteBalanceStats;
    CI_MODULE_AWS sAutoWhiteBalanceStats;
    CI_MODULE_HIS sHistStats;
#endif //UNUSED_CODE_AND_VARIABLE
} CI_PIPELINE_STATS;

typedef struct CI_PIPELINE
{
    CI_PIPELINE_CONFIG config;

    /**
     * @name Main Modules
     * @{
     */

    /**
     * @}
     */

    /** @brief contains the statistics configuration modules */
    CI_PIPELINE_STATS stats;
} CI_PIPELINE;

//=== Godspeed === Add new memory/buffer type support here.
/**
 * @brief Buffer ID structure - 1 ID per importable buffer
 *
 * Used to simplify API calls that would change when additional IDs are needed
 *
 * When using ION these are the ION IDs
 *
 * @warning Strides CANNOT be changed if the buffer is tiled (HW limitation).
 * If the buffer is tiled ensure a stride of 0 is given.
 */
typedef struct CI_BUFFID
{
    /** @brief Uo buffer Identifier */
    STF_U32 u32UoId;
    /**
     * @brief Uo Y buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32UoStrideY;
    /**
     * @brief Uo CbCr buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32UoStrideC;
    /**
     * @brief start of luma plane offset in bytes
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32UoOffsetY;
    /**
     * @brief chroma offset in bytes - if 0 computed by kernel-side to be
     * just after luma plane
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32UoOffsetC;

    /** @brief Ss0 buffer Identifier */
    STF_U32 u32Ss0Id;
    /**
     * @brief Ss0 Y buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss0StrideY;
    /**
     * @brief Ss0 CbCr buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss0StrideC;
    /**
     * @brief start of luma plane offset in bytes
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss0OffsetY;
    /**
     * @brief chroma offset in bytes - if 0 computed by kernel-side to be
     * just after luma plane
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss0OffsetC;

    /** @brief Ss1 buffer Identifier */
    STF_U32 u32Ss1Id;
    /**
     * @brief Ss1 Y buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss1StrideY;
    /**
     * @brief Ss1 CbCr buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss1StrideC;
    /**
     * @brief start of luma plane offset in bytes
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss1OffsetY;
    /**
     * @brief chroma offset in bytes - if 0 computed by kernel-side to be
     * just after luma plane
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Ss1OffsetC;

    /** @brief CSI Dump buffer Identifier */
    STF_U32 u32DumpId;
    /**
     * @brief CSI dump buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32DumpStride;
    /**
     * @brief Start of CSI dump buffer offset in bytes
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32DumpOffset;

    /** @brief Tiling_1_Rd buffer Identifier */
    STF_U32 u32Tiling_1_RdId;
    /**
     * @brief Tiling_1_Rd Y buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_RdStrideY;
    /**
     * @brief Tiling_1_Rd CbCr buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_RdStrideC;
    /**
     * @brief start of luma plane offset in bytes
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_RdOffsetY;
    /**
     * @brief chroma offset in bytes - if 0 computed by kernel-side to be
     * just after luma plane
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_RdOffsetC;

    /** @brief Tiling_1_Wr buffer Identifier */
    STF_U32 u32Tiling_1_WrId;
    /**
     * @brief Tiling_1_Wr Y buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_WrStrideY;
    /**
     * @brief Tiling_1_Wr CbCr buffer stride in bytes - if 0 ignored
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_WrStrideC;
    /**
     * @brief start of luma plane offset in bytes
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_WrOffsetY;
    /**
     * @brief chroma offset in bytes - if 0 computed by kernel-side to be
     * just after luma plane
     *
     * @note Needs to be a multiple of SYSMEM_ALIGNMENT
     */
    STF_U32 u32Tiling_1_WrOffsetC;
} CI_BUFFID;

//=== Godspeed === Add new memory/buffer type support here.
typedef enum _CI_BUFFTYPE
{
    CI_TYPE_NONE = 0,
    /** @brief YUV on the Un-scaled stream output */
    CI_TYPE_UO,
    /** @brief YUV on the Scaled stream 0 output */
    CI_TYPE_SS0,
    /** @brief YUV on the Scaled stream 1 output */
    CI_TYPE_SS1,
    /** @brief RAW on the CSI dump output. */
    CI_TYPE_DUMP,
    /** @brief RAW/RGB/YUV on the Tiling 1 read input */
    CI_TYPE_TILING_1_RD,
    /** @brief RAW/RGB/YUV on the Tiling 1 write output */
    CI_TYPE_TILING_1_WR,
    CI_TYPE_SC_DUMP,
    CI_TYPE_Y_HIST,
    CI_TYPE_GEN_MEM,            // Unnecessary put this kind of buffer into shot structure.
    CI_TYPE_GEN_MEM_ALIGN_128,  // Unnecessary put this kind of buffer into shot structure.
    CI_TYPE_MAX,
} CI_BUFFTYPE, *PCI_BUFFTYPE;

//=== Godspeed === Add new memory/buffer type support here.
/** @brief Information about a single captured frame */
typedef struct CI_SHOT
{
    /** @brief stride & height */
    STF_U32 aUoYSize[2];
    /** @brief stride & height */
    STF_U32 aUoCbCrSize[2];
    /** @brief Y offset and CbCr offset - assumes chroma is after luma */
    STF_U32 aUoOffset[2];

    /** @brief stride & height */
    STF_U32 aSs0YSize[2];
    /** @brief stride & height */
    STF_U32 aSs0CbCrSize[2];
    /** @brief Y offset and CbCr offset - assumes chroma is after luma */
    STF_U32 aSs0Offset[2];

    /** @brief stride & height */
    STF_U32 aSs1YSize[2];
    /** @brief stride & height */
    STF_U32 aSs1CbCrSize[2];
    /** @brief Y offset and CbCr offset - assumes chroma is after luma */
    STF_U32 aSs1Offset[2];

    /** @brief stride & height */
    STF_U32 aDumpSize[2];
    /** @brief start of buffer offset in bytes */
    STF_U32 u32DumpOffset;

    /** @brief stride & height */
    STF_U32 aTiling_1_RdYSize[2];
    /** @brief stride & height */
    STF_U32 aTiling_1_RdCbCrSize[2];
    /** @brief Y offset and CbCr offset - assumes chroma is after luma */
    STF_U32 aTiling_1_RdOffset[2];

    /** @brief stride & height */
    STF_U32 aTiling_1_WrYSize[2];
    /** @brief stride & height */
    STF_U32 aTiling_1_WrCbCrSize[2];
    /** @brief Y offset and CbCr offset - assumes chroma is after luma */
    STF_U32 aTiling_1_WrOffset[2];

    /** @brief Size of the statistics in bytes */
    STF_U32 u32ScDumpSize;

    /** @brief Size of the Y histogram in bytes */
    STF_U32 u32YHistSize;

    /**
     * @brief Easy access to the used configuration number
     *
     * See @ref CI_PIPELINE::u8PrivateValue
     */
    STF_U8 u8PrivateValue;
    /**
     * @brief Set to STF_TRUE if an error occurred while capturing the frame
     */
    STF_BOOL8 bFrameError;
    /**
     * @brief Number of missed frames before acquiring that one
     *
     * @note if the value is negative it means issues with gasket frame counter
     */
    STF_S32 s32MissedFrames;

    STF_U32 u32UoId;
    void *pUoOutput;
    void *pUoOutputPhyAddr;

    STF_U32 u32Ss0Id;
    void *pSs0Output;
    void *pSs0OutputPhyAddr;

    STF_U32 u32Ss1Id;
    void *pSs1Output;
    void *pSs1OutputPhyAddr;

    STF_U32 u32DumpId;
    void *pDumpOutput;
    void *pDumpOutputPhyAddr;

    STF_U32 u32Tiling_1_RdId;
    void *pTiling_1_RdInput;
    void *pTiling_1_RdInputPhyAddr;

    STF_U32 u32Tiling_1_WrId;
    void *pTiling_1_WrOutput;
    void *pTiling_1_WrOutputPhyAddr;

    void *pScDump;
    void *pScDumpPhyAddr;
    void *pYHist;
    void *pYHistPhyAddr;

    /** @brief timestamps when pushed into register - read by drivers */
    STF_U32 u32PushTS;
    /** @brief timestamps when interrupt was serviced - read by drivers */
    STF_U32 u32InterruptTS;

#ifdef SIFIVE_ISP
    STF_U64 u64SystemTS;
#endif //SIFIVE_ISP

    /* statistics configurations */
    /** @brief configuration of the output - copied from pipeline config */
    STF_U32 eOutputConfig;
} CI_SHOT;

typedef struct CI_ION_CONNECTION
{
    int (*g_fd)(SYS_FILE *pFile);
    int (*close)(SYS_FILE *pFile);
    void *(*mmap)(SYS_FILE *pFile, size_t length, int prot, int flags, off_t offset);
    int (*munmap)(SYS_FILE *pFile, void *addr, size_t length);
} CI_ION_CONNECTION;

/**
 * @brief Used for physical linear alloc
 */
#if defined(V4L2_DRIVER)
typedef struct _CI_DMA_MEM_PARAM {
    STF_U32 u32IonFd;
    STF_U32 u32Size;
    STF_U32 u32PhysAddress;
} CI_DMA_MEM_PARAM, *PCI_DMA_MEM_PARAM;
#else
typedef struct _CI_DMA_MEM_PARAM {
    STF_U64 BasePhysAddr;
    void *BaseVirtAddress;
    STF_S64 s64Size;
    /**
     * @brief physical address is the HW accessible address
     * (can be just an offset in DEV mem instead of global system address)
     * @note 64b to support extra addressing on 32b OS
     */
    STF_U64 uiPhysAddress;
    /**
     * @brief virtual address is the SW accessible address
     */
    STF_U64 uiVirtAddress;
} CI_DMA_MEM_PARAM, *PCI_DMA_MEM_PARAM;
#endif //#if defined(V4L2_DRIVER)

typedef struct _CI_SYS_MEM_PARAM {
    /**
     * @brief opaque type storing the allocation - chosen according to MMU
     * choice
     */
    CI_DMA_MEM_PARAM *pAlloc;
    STF_SIZE uiAllocated;
    /**
     * @brief additional virtual memory - 1st address has to be a multiple of
     * that
     */
    STF_U32 u32VirtualAlignment;
    STF_BOOL8 bWasImported;
    char szMemName[20];
    void *pVirtualAddress;
    void *pPhysAddress;
} CI_SYS_MEM_PARAM, *PCI_SYS_MEM_PARAM;

typedef struct _CI_MEM_PARAM {
    STF_U32 u32Width;
    STF_U32 u32Height;
    CI_BUFFTYPE enAllocBufferType;
    PIXELTYPE stBufType;
    CI_SYS_MEM_PARAM stSysMem;
    STF_U32 au32YSize[2];
    STF_U32 au32CbCrSize[2];
    STF_U32 au32Offset[2];
    STF_U32 u32BufSize;
    void *pBuffer;
#if defined(V4L2_DRIVER)
    STF_U32 u32Index;
    CI_ION_CONNECTION *pstIonConnection;
#endif //#if defined(V4L2_DRIVER)
} CI_MEM_PARAM, *PCI_MEM_PARAM;

//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ST_SHOT_INFO {
#if defined(V4L2_DRIVER)
    CI_MEM_PARAM *pstUoMem;                     /** Uo buffer information. */
    CI_MEM_PARAM *pstSs0Mem;                    /** Ss0 buffer information. */
    CI_MEM_PARAM *pstSs1Mem;                    /** Ss1 buffer information. */
    CI_MEM_PARAM *pstDumpMem;                   /** Dump buffer information. */
    CI_MEM_PARAM *pstTiling_1_ReadMem;          /** Tiling 1 read buffer information. */
    CI_MEM_PARAM *pstTiling_1_WriteMem;         /** Tiling 1 write buffer information. */
    CI_MEM_PARAM *pstScDumpMem;                 /** SC dump buffer information. */
    CI_MEM_PARAM *pstYHistMem;                  /** Y histogram buffer information. */
#else
    CI_MEM_PARAM stUoMem;                       /** Uo buffer information. */
    CI_MEM_PARAM stSs0Mem;                      /** Ss0 buffer information. */
    CI_MEM_PARAM stSs1Mem;                      /** Ss1 buffer information. */
    CI_MEM_PARAM stDumpMem;                     /** Dump buffer information. */
    CI_MEM_PARAM stTiling_1_ReadMem;            /** Tiling 1 read buffer information. */
    CI_MEM_PARAM stTiling_1_WriteMem;           /** Tiling 1 write buffer information. */
    CI_MEM_PARAM stScDumpMem;                   /** SC dump buffer information. */
    CI_MEM_PARAM stYHistMem;                    /** Y histogram buffer information. */
#endif //#if defined(V4L2_DRIVER)
    STF_BOOL8 bIsScDumpForAe;                   /** It used to indentify the SC dump buffer is for AE or AWB. */
    STF_U32 u32ScDumpCount;                     /** It used to count the same extract point times for AE or AWB. */
    STF_BOOL8 bIsContiguousCapture;             /** Is using for contiguous capture mode? */
} ST_SHOT_INFO, *PST_SHOT_INFO;

typedef struct _ST_FW_INFO {
    STF_CHAR szFilename[FILENAME_MAX_LEN];
} ST_FW_INFO, *PST_FW_INFO;

typedef struct _ST_REG_INFO {
    /** @brief [in] access method of register */
    STF_U8 u8Method;
    /** @brief [in] offset indicated which register will be read/write */
    STF_U32 u32Offset;
    /** @brief [in] length for indicated how much register will be read/write */
    STF_U32 u32Length;
} ST_REG_INFO, *PST_REG_INFO;

typedef struct _ST_REG_TBL {
    STF_U32 u32Offset;
    STF_U32 u32Value;
} ST_REG_TBL, *PST_REG_TBL;

typedef struct _ST_REG_TBL_2 {
    STF_U32 u32Offset;
    STF_U32 u32Value;
    STF_U32 u32Mask;
} ST_REG_TBL_2, *PST_REG_TBL_2;

typedef struct _ST_REG_TBL_3 {
    STF_U32 u32Offset;
    STF_U32 u32Value;
    STF_U32 u32Mask;
    STF_U32 u32DelayMs;
} ST_REG_TBL_3, *PST_REG_TBL_3;

typedef union _UN_REG_BUF {
    STF_U32 u32Buffer[STF_ISP_REG_BUF_SIZE];
    ST_REG_TBL u32RegTbl[STF_ISP_REG_TBL_BUF_SIZE];
    ST_REG_TBL_2 u32RegTbl2[STF_ISP_REG_TBL_2_BUF_SIZE];
    ST_REG_TBL_3 u32RegTbl3[STF_ISP_REG_TBL_3_BUF_SIZE];
    ST_RDMA_CMD stRdmaCmd[STF_ISP_REG_SMPL_PACK_BUF_SIZE];
} UN_REG_BUF, *PUN_REG_BUF;

typedef struct _ST_ISP_REG_PARAM {
    /** @brief [in, out] register read/write information */
    ST_REG_INFO stRegInfo;
    /** @brief [in, out] buffer */
    UN_REG_BUF *pBuffer;
} ST_ISP_REG_PARAM, *PST_ISP_REG_PARAM;

typedef struct CI_GASKET
{
    /** @brief The gasket it refers to */
    STF_U8 u8Gasket;

    /** @brief Parallel or Mipi gasket (false=mipi) */
    STF_BOOL8 bParallel;

    /**
     * @name Parallel configuration only
     * @{
     */
    /**
     * @brief Width-1 of the parallel frame
     *
     * @note Gasket Register: PARALLEL_FRAME_SIZE:PARALLEL_FRAME_WIDTH
     */
    STF_U16 uiWidth;
    /**
     * @brief Height-1 of the parallel frame
     *
     * @note Gasket Register: PARALLEL_FRAME_SIZE:PARALLEL_FRAME_HEIGHT
     */
    STF_U16 uiHeight;
    /**
     * @brief High or Low VSync (true=high)
     *
     * @note Gasket Register: PARALLEL_CTRL:PARALLEL_V_SYNC_POLARITY
     */
    STF_BOOL8 bVSync;
    /**
     * @brief High or Low HSync (true=high)
     *
     * @note Gasket Register: PARALLEL_CTRL:PARALLEL_H_SYNC_POLARITY
     */
    STF_BOOL8 bHSync;
    /**
     * @brief Bitdepth of the parallel (10 or 12)
     *
     * @note Gasket Register: PARALLEL_CTRL:PARALLEL_PIXEL_FORMAT
     */
    STF_U8 u8ParallelBitdepth;

    /**
     * @}
     */
} CI_GASKET;

typedef struct CI_GASKET_INFO
{
    /**
     * @name Information
     * @{
     */

    /**
     * @brief if None then gasket is disabled otherwise combination of
     * @ref CI_GASKETTYPE
     *
     * @note For the moment it is not very useful: this information is
     * available in the @ref CI_CONNECTION.
     * But it may be useful in the future if a gasket can have multiple types
     * and only 1 enabled.
     */
    STF_U8 eType;

    /**
     * @}
     * @name Live information
     * @brief May change while running - need gasket to be enabled to be valid
     * @{
     */

    /** @warning only relevant if gasket is enabled */
    STF_U32 u32FrameCount;

    /** @warning only relevant if gasket is MIPI and enabled */
    STF_U8 u8MipiFifoFull;
    /** @warning only relevant if gasket is MIPI and enabled */
    STF_U8 u8MipiEnabledLanes;
    /** @warning only relevant if gasket is MIPI and enabled */
    STF_U8 u8MipiCrcError;
    /** @warning only relevant if gasket is MIPI and enabled */
    STF_U8 u8MipiHdrError;
    /** @warning only relevant if gasket is MIPI and enabled */
    STF_U8 u8MipiEccError;
    /** @warning only relevant if gasket is MIPI and enabled */
    STF_U8 u8MipiEccCorrected;

    /**
     * @}
     */
} CI_GASKET_INFO;


/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the CI_API documentation module
 *---------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif


#endif /* __CI_API_STRUCTS_H__ */
