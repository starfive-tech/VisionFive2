/**
  ******************************************************************************
  * @file stflib_isp_shot.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  09/01/2020
  * @brief Declaration of ISPC::Shot
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
  ******************************************************************************/


#ifndef __STFLIB_ISP_SHOT_H__
#define __STFLIB_ISP_SHOT_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "mc/mc_structs.h"

#include "stf_common/pixel_format.h"
#include "ISPC/stflib_isp_defs.h"


#define BUFFER_COUNT_MAX                (36)
#define BUFFER_ID_CNT_MAX               (256)
#define BUFFER_ID_INVALID               (0xFFFFFFFF)


typedef struct _ST_BUFFER_ID ST_BUFFER_ID;

typedef struct _ST_BUFFER_ID {
    STF_S32 u32Elements;
    STF_U32 au32BufferId[BUFFER_ID_CNT_MAX + 1];
    STF_RESULT (*Clear)(ST_BUFFER_ID *pstBufferId);
    STF_U32 (*Size)(ST_BUFFER_ID *pstBufferId);
    STF_BOOL8 (*IsEmpty)(ST_BUFFER_ID *pstBufferId);
    STF_BOOL8 (*IsFull)(ST_BUFFER_ID *pstBufferId);
    STF_RESULT (*PushFront)(ST_BUFFER_ID *pstBufferId, STF_U32 u32BufferId);
    STF_RESULT (*PushBack)(ST_BUFFER_ID *pstBufferId, STF_U32 u32BufferId);
    STF_U32 (*PopFront)(ST_BUFFER_ID *pstBufferId);
    STF_U32 (*PopBack)(ST_BUFFER_ID *pstBufferId);
} ST_BUFFER_ID, *PST_BUFFER_ID;

typedef struct _ST_BUFFER ST_BUFFER;

/**
 * @brief Contains the information for a generic image buffer returned
 * from CI
 *
 * Separates the CI_SHOT information per output (YUV, RGB, etc)
 */
typedef struct _ST_BUFFER {
    /**
     * @brief ID used when importing/allocating buffer
     */
    STF_U32 u32Id;

    /**
     * @brief desired output size in pixels
     *
     * Derived from CI_SHOT and CI_PIPELINE information
     */
    STF_U16 u16Width;
    /**
     * @brief desired output size in lines
     *
     * Derived from CI_SHOT and CI_PIPELINE information
     */
    STF_U16 u16Height;
    /**
     * @brief distance to the next line in bytes
     *
     * If pxlFormat is YUV this is the Luma stride and strideCbCr is the
     * chroma stride.
     *
     * Derived from CI_SHOT information
     */
    STF_U16 u16Stride;

    /**
     * @brief actual allocated height in bytes
     *
     * Useful when doing tiling to include the bottom of the image
     */
    STF_U16 u16VStride;
    /**
     * @brief distance to the next line in Chroma in bytes (only for YUV
     * output)
     *
     * If pxlFormat is YUV this is the chroma stride otherwise it is not
     * used
     */
    STF_U16 u16StrideCbCr;
    /**
     * @brief actual allocated height in bytes (only for YUV output)
     *
     * Useful when doing tiling to include the bottom of the image
     */
    STF_U16 u16VStrideCbCr;
    /**
     * @brief distance to the start of the data in bytes
     */
    STF_U32 u32Offset;
    /**
     * @brief distance to chroma data in bytes (only for YUV output)
     *
     * If pxlFormat is YUV this is the chroma offset in bytes from the
     * start of the buffer.
     */
    STF_U32 u32OffsetCbCr;
    /**
     * @brief Image pixel format
     *
     * Derived from CI_PIPELINE information
     */
    ePxlFormat enPxlFormat;
    /**
     * @brief Pointer to actual buffer data (available data from CI_SHOT)
     */
    STF_U8 *pu8Data;
    /**
     * @brief add for frame memory start physical address
     */
    STF_VOID *phyAddr;

    /** @brief uses data and offset to return first entry address */
    STF_U8* (*FirstData)(ST_BUFFER *pstBUffer);

    /** @brief uses data and offsetCbCr to return first entry address */
    STF_U8* (*FirstDataCbCr)(ST_BUFFER *pstBUffer);
} ST_BUFFER, *PST_BUFFER;

/**
 * @brief Contain some statistics output that are not converted
 */
typedef struct _ST_STATISTICS
{
    /**
     * @brief Pointer to actual buffer data (available data from CI_SHOT)
     */
    STF_U8 *pu8Data;
    /**
     * @brief add for statistics memory start physical address
     */
    STF_VOID *phyAddr;

    /** @brief in Bytes of accessible data */
    STF_U32 u32Stride;
    /** @brief in Bytes of useful data */
    STF_U32 u32Size;
    /** @brief in Bytes - element size if relevant */
    STF_U32 u32SlementSize;
} ST_STATISTICS, *PST_STATISTICS;

/**
 * @brief Output meta-data used for AAA
 */
typedef struct _ST_R_G {
    STF_U32 u32R;
    STF_U32 u32G;
} ST_R_G, *PST_R_G;

typedef struct _ST_B_Y {
    STF_U32 u32B;
    STF_U32 u32Y;
} ST_B_Y, *PST_B_Y;

typedef struct _ST_B_CNT {
    STF_U32 u32B;
    STF_U32 u32Cnt;
} ST_B_CNT, *PST_B_CNT;

typedef struct _ST_W_RW {
    STF_U32 u32W;
    STF_U32 u32Rw;
} ST_W_RW, *PST_W_RW;

typedef struct _ST_GW_BW {
    STF_U32 u32Gw;
    STF_U32 u32Bw;
} ST_GW_BW, *PST_GW_BW;

typedef struct _ST_GRW_GBW {
    STF_U32 u32Grw;
    STF_U32 u32Gbw;
} ST_GRW_GBW, *PST_GRW_GBW;

typedef struct _ST_DATA_CNT {
    STF_U32 u32Data;
    STF_U32 u32Cnt;
} ST_DATA_CNT, *PST_DATA_CNT;

typedef struct _ST_SC_AE_AF_AWB {
    ST_R_G stAeWs_R_G[16];
    ST_B_Y stAeWs_B_Y[16];
    ST_R_G stAwbPs_R_G[16];
    ST_B_CNT stAwbPs_B_Cnt[16];
    ST_W_RW stAwbWgs_W_RW[16];
    ST_GW_BW stAwbWgs_GW_BW[16];
    ST_GRW_GBW stAwbWgs_GRW_GBW[16];
    ST_DATA_CNT stAfEs_Data_Cnt[16];
} ST_SC_AE_AF_AWB, *PST_SC_AE_AF_AWB;

typedef struct _ST_METADATA
{
    /**
     * @brief AE/AF/AWB statistics.
     */
    ST_SC_AE_AF_AWB stScAeAfAwb[16];
    /**
     * @brief R/G of Y histogram statistics of AE.
     */
    ST_R_G stScAeHist_R_G[64];
    /**
     * @brief B/Y of Y histogram statistics of AE.
     */
    ST_B_Y stScAeHist_B_Y[64];
    /**
     * @brief Y histogram statistics.
     */
    STF_U32 u32YHist[64];

    /** @brief Timestamps information */
    MC_STATS_TIMESTAMP timestamps;
} ST_METADATA, *PST_METADATA;

typedef struct _ST_SHOT ST_SHOT;

//=== Godspeed === Add new memory/buffer type support here.
/**
 * @brief Structure containing all the information for a gathered shot
 * (actual image buffer, metadata, timestamp, etc.)
 */
typedef struct _ST_SHOT {
    /** @brief is frame correct? */
    STF_BOOL8 bFrameError;
    /**
     * @brief Number of missed frames before that one was acquired
     *
     * @note if negative cannot be trusted
     */
    STF_S32 s32MissedFrames;

    /** @brief Image buffer for Un-scaled output */
    ST_BUFFER stUo;
    /** @brief Image buffer for Scaled-stream 0 output */
    ST_BUFFER stSs0;
    /** @brief Image buffer for Scaled-stream 1 output */
    ST_BUFFER stSs1;
    /** @brief Image buffer for RAW data dump output */
    ST_BUFFER stDump;
    /** @brief Image buffer for Tiling 1 read input */
    ST_BUFFER stTiling_1_Rd;
    /** @brief Image buffer for Tiling 1 write output */
    ST_BUFFER stTiling_1_Wr;

    //=== Godspeed === Add new memory/buffer type support here.
    /**
     * @brief AE/AF/AWB statistics output
     *
     * @note size is computed from Metadata and HW_CI_ScDumpSize
     */
    ST_STATISTICS stSc;

    /**
     * @brief Y histogram statistics output
     *
     * @note size is computed from Metadata and HW_CI_YHistSize
     */
    ST_STATISTICS stYHist;

    /** @brief Shot metadata */
    ST_METADATA stMetadata;

    /** @brief Corresponding CI_SHOT */
    CI_SHOT *pstCIBuffer;

    /**
     * @brief Reset all Buffer and metadata to default
     */
    STF_RESULT (*Clear)(ST_SHOT *pstShot);
    /**
     * @brief Populates the Buffers and Metadata using CI_SHOT
     */
    STF_RESULT (*Configure)(ST_SHOT *pstShot, CI_SHOT *pstCIBuffer, STF_VOID *pPipeline, ST_GLOBAL_SETUP *pstGlobalSetup);
#if 0
    /**
     * @brief Populates the Buffers and Metadata using CI_SHOT
     */
    STF_VOID configure(ST_SHOT *pstShot, CI_SHOT *pCIBuffer,
        const ModuleOUT &globalConfig, const Global_Setup &globalSetup);
#endif
} ST_SHOT, *PST_SHOT;


//=============================================================================
extern
STF_RESULT STFLIB_ISP_BUFFER_ID_Clear(
    ST_BUFFER_ID *pstBufferId
    );

extern
STF_U32 STFLIB_ISP_BUFFER_ID_Size(
    ST_BUFFER_ID *pstBufferId
    );

extern
STF_BOOL8 STFLIB_ISP_BUFFER_ID_IsEmpty(
    ST_BUFFER_ID *pstBufferId
    );

extern
STF_BOOL8 STFLIB_ISP_BUFFER_ID_IsFull(
    ST_BUFFER_ID *pstBufferId
    );

extern
STF_RESULT STFLIB_ISP_BUFFER_ID_PushFront(
    ST_BUFFER_ID *pstBufferId,
    STF_U32 u32BufferId
    );

extern
STF_RESULT STFLIB_ISP_BUFFER_ID_PushBack(
    ST_BUFFER_ID *pstBufferId,
    STF_U32 u32BufferId
    );

extern
STF_U32 STFLIB_ISP_BUFFER_ID_PopFront(
    ST_BUFFER_ID *pstBufferId
    );

extern
STF_U32 STFLIB_ISP_BUFFER_ID_PopBack(
    ST_BUFFER_ID *pstBufferId
    );

extern
STF_RESULT STFLIB_ISP_BUFFER_ID_StructureInitialize(
    ST_BUFFER_ID *pstBufferId
    );

extern
STF_RESULT STFLIB_ISP_BUFFER_ID_StructureUninitialize(
    ST_BUFFER_ID *pstBufferId
    );

//=============================================================================
extern
STF_U8* STFLIB_ISP_BUFFER_FirstData(
    ST_BUFFER *pstBUffer
    );

extern
STF_U8* STFLIB_ISP_BUFFER_FirstDataCbCr(
    ST_BUFFER *pstBUffer
    );

extern
STF_RESULT STFLIB_ISP_BUFFER_Init(
    ST_BUFFER *pstBUffer
    );

extern
STF_RESULT STFLIB_ISP_SHOT_Clear(
    ST_SHOT *pstShot
    );

extern
STF_RESULT STFLIB_ISP_SHOT_Configure(
    ST_SHOT *pstShot,
    CI_SHOT *pstCIBuffer,
    STF_VOID *pPipeline,
    ST_GLOBAL_SETUP *pstGlobalSetup
    );

extern
STF_RESULT STFLIB_ISP_SHOT_Init(
    ST_SHOT *pstShot
    );


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_ISP_SHOT_H__
