/**
  ******************************************************************************
  * @file  stfmod_isp_dump.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  05/26/2020
  * @brief
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


#ifndef __STFMOD_ISP_DUMP_H__
#define __STFMOD_ISP_DUMP_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "stf_common/ci_alloc_info.h"
#include "registers/stf_isp_dump_rdma.h"
#include "stflib_isp_pipeline.h"


/* ISP CSI raw data dump module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define DUMP_PIXEL_BITS                         (12)
#define DUMP_PIXEL_VALUE_MAX                    ((1 << DUMP_PIXEL_BITS) - 1)
#define DUMP_ALIGN_BITS                         (7)
#define DUMP_ALIGN_MASK                         ((1 << DUMP_ALIGN_BITS) - 1)
#define DUMP_ALIGNMENT                          (1 << DUMP_ALIGN_BITS)

#define ISP_DEF_DUMP_BURST_LEN                  (3)               /** AXI memory write burst length. */
#define ISP_DEF_DUMP_PXL_R_SHIFT                (0)               /** Pixel right shift number. */
#define ISP_DEF_DUMP_MOSAIC                     (MOSAIC_RGGB)     /** Bayer raw data mosaic type. */
#define ISP_DEF_DUMP_AXI_ID                     (0)               /** AXI memory write ID. */
#define ISP_DEF_DUMP_STRIDE                     (0x00000F00)      /** Stride address for captured image 8x16 byte aligned. */
#define ISP_DEF_DUMP_BUFFER_ADDRESS             (0x6E000000)      /** Point to CSI raw data dump buffer address. */


#pragma pack(push, 1)

typedef enum _EN_DUMP_UPDATE {
    EN_DUMP_UPDATE_NONE = 0X00,
    EN_DUMP_UPDATE_BURST_LEN = 0x01,
    EN_DUMP_UPDATE_PIXEL_RIGHT_SHIFT_BITS = 0x02,
    EN_DUMP_UPDATE_AXI_ID = 0x04,
    EN_DUMP_UPDATE_STRIDE = 0x08,
    EN_DUMP_UPDATE_DUMP_BUFFER = 0x10,
    EN_DUMP_UPDATE_ALL_WO_BUF_INFO = (EN_DUMP_UPDATE_BURST_LEN
                        | EN_DUMP_UPDATE_PIXEL_RIGHT_SHIFT_BITS
                        | EN_DUMP_UPDATE_AXI_ID
                       ),
    EN_DUMP_UPDATE_ALL = (EN_DUMP_UPDATE_BURST_LEN
                        | EN_DUMP_UPDATE_PIXEL_RIGHT_SHIFT_BITS
                        | EN_DUMP_UPDATE_AXI_ID
                        | EN_DUMP_UPDATE_STRIDE
                        | EN_DUMP_UPDATE_DUMP_BUFFER
                       ),
} EN_DUMP_UPDATE, *PEN_DUMP_UPDATE;


typedef struct _ST_DUMP_BUF_INFO {
    CI_BUFFTYPE enAllocBufferType;              /** Output image buffer type.
                                                  * 0:CI_TYPE_NONE,
                                                  * 1:CI_TYPE_UO,
                                                  * 2:CI_TYPE_SS0,
                                                  * 3:CI_TYPE_SS1,
                                                  * 4:CI_TYPE_DUMP,
                                                  * 5:CI_TYPE_TILING_1_RD,
                                                  * 6:CI_TYPE_TILING_1_WR,
                                                  * 7:CI_TYPE_SC_DUMP,
                                                  *  8:CI_TYPE_Y_HIST
                                                  */
    eFORMAT_TYPES enFormatType;                 /** output image buffer format type. 0:TYPE_NONE, 1:TYPE_RGB, 2:TYPE_YUV and 3:TYPE_BAYER.  */
    ePxlFormat enPixelFormat;                   /** Output image pixel format. 0:PXL_NONE, 1:YVU_420_PL12_8, 2:YUV_420_PL12_8, ..., PXL_N and PXL_INVALID. */
    eMOSAIC enMosaic;                           /** Output image mosaic type. 0:MOSAIC_NONE, 1:MOSAIC_RGGB, 2:MOSAIC_GRBG, 3:MOSAIC_GBRG and 4:MOSAIC_BGGR. */
    ST_SIZE stSize;                             /** Output image size. */
    STF_U32 u32Stride;                          /** Stride address for captured image 8x16 byte aligned. */
    STF_U16 *pu16Buffer;                        /** Bayer RAW data store buffer for DUMP. */
} ST_DUMP_BUF_INFO, *PST_DUMP_BUF_INFO;

typedef struct _ST_MOD_DUMP_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable the CSI bayer RAW data dump module. */
    STF_U8 u8BurstLength;                       /** AXI memory write burst length. */
    STF_U8 u8PixelRightShiftBits;               /** The number of pixels shifted right. */
    STF_U8 u8AxiId;                             /** ID of AXI bus for the CSI RAW data dump controller. */
} ST_MOD_DUMP_SETTING, *PST_MOD_DUMP_SETTING;

typedef struct _ST_DUMP_PARAM {
    ST_MOD_DUMP_SETTING stSetting;              /** Module DUMP setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program DUMP module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
    ST_DUMP_BUF_INFO stDumpBufInfo;             /** Image buffer information for bayer raw data dump. */
} ST_DUMP_PARAM, *PST_DUMP_PARAM;

#define ST_MOD_DUMP_SETTING_LEN         (sizeof(ST_MOD_DUMP_SETTING))
#define ST_MOD_DUMP_GET_PARAM_LEN       (ST_MOD_DUMP_SETTING_LEN)
#define ST_MOD_DUMP_SET_PARAM_LEN       (ST_MOD_DUMP_SETTING_LEN)
//#define ST_DUMP_PARAM_SIZE              (sizeof(ST_DUMP_PARAM) - sizeof(ST_DUMP_BUF_INFO))
#define ST_DUMP_PARAM_SIZE              (sizeof(ST_DUMP_PARAM))
#define ST_DUMP_PARAM_LEN               (ST_DUMP_PARAM_SIZE)

typedef struct _ST_ISP_MOD_DUMP {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstDumpRdmaBuf;               /** DUMP's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_DUMP_RDMA *pstIspDumpRdma;           /** DUMP's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_DUMP_RDMA *pstDumpRdma;                  /** DUMP's RDMA structure pointer.*/
    ST_DUMP_PARAM *pstDumpParam;                /** DUMP's parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Module register in pipeline. */
    STF_S32 (*Init)(STF_VOID *pIspCtx, STF_U32 *pNextRdma);
    STF_S32 (*LoadBinParam)(STF_VOID *pIspCtx, STF_U8 *pu8ParamBuf, STF_U16 u16ParamSize);
    STF_S32 (*LoadBinClbrt)(STF_VOID *pIspCtx);
    STF_S32 (*Enable)(STF_VOID *pIspCtx, STF_BOOL8 bEnable);
    STF_BOOL8 (*IsEnable)(STF_VOID *pIspCtx);
    STF_S32 (*Update)(STF_VOID *pIspCtx, STF_BOOL8 bUpdate);
    STF_BOOL8 (*IsUpdate)(STF_VOID *pIspCtx);
    STF_S32 (*Registered)(STF_VOID *pIspCtx, STF_BOOL8 bRegistered);
    STF_BOOL8 (*IsRegistered)(STF_VOID *pIspCtx);
    STF_S32 (*UpdateReg)(STF_VOID *pIspCtx);
    STF_S32 (*SetReg)(STF_VOID *pIspCtx);
    STF_S32 (*SetNext)(STF_VOID *pIspCtx, STF_U32 *pNextRdma);
    STF_S32 (*FreeResource)(STF_VOID *pIspCtx);
    STF_S32 (*GetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);
    STF_S32 (*SetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 u16ParamSize);
} ST_ISP_MOD_DUMP, *PST_ISP_MOD_DUMP;

#pragma pack(pop)


/* ISP CSI raw data dump module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DUMP_CalDumpBufFormat(
    STF_U8 u8RightShiftBits,
    eMOSAIC enMosaic,
    ST_DUMP_BUF_INFO *pstDumpBufInfo,
    eFORMAT_TYPES *penFormatTypes
    );
extern
STF_S32 STFMOD_ISP_DUMP_CalDumpBufInfo(
    STF_U8 u8RightShiftBits,
    eMOSAIC enMosaic,
    ST_SIZE *pstSize,
    ST_DUMP_BUF_INFO *pstDumpBufInfo,
    eFORMAT_TYPES *penFormatTypes,
    PIXELTYPE *pstPxlType,
    struct CI_SIZEINFO *pstSizeInfo
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DUMP_InitRdma(
    ST_DUMP_RDMA *pstDumpRdma,
    ST_DUMP_PARAM *pstDumpParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DUMP_UpdateRdma(
    ST_DUMP_RDMA *pstDumpRdma,
    ST_DUMP_PARAM *pstDumpParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DUMP_UpdateBurstLength(
    ST_DUMP_RDMA *pstDumpRdma,
    ST_DUMP_PARAM *pstDumpParam
    );
extern
STF_S32 STFMOD_ISP_DUMP_UpdatePixelRightShiftBits(
    ST_DUMP_RDMA *pstDumpRdma,
    ST_DUMP_PARAM *pstDumpParam
    );
extern
STF_S32 STFMOD_ISP_DUMP_UpdateAxiId(
    ST_DUMP_RDMA *pstDumpRdma,
    ST_DUMP_PARAM *pstDumpParam
    );
extern
STF_S32 STFMOD_ISP_DUMP_UpdateStride(
    ST_DUMP_RDMA *pstDumpRdma,
    ST_DUMP_PARAM *pstDumpParam
    );
extern
STF_S32 STFMOD_ISP_DUMP_UpdateDumpBuffer(
    ST_DUMP_RDMA *pstDumpRdma,
    ST_DUMP_PARAM *pstDumpParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DUMP_SetNextRdma(
    ST_DUMP_RDMA *pstDumpRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DUMP_SetEnable(
    ST_ISP_MOD_DUMP *pstModDump,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetUpdate(
    ST_ISP_MOD_DUMP *pstModDump,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetSize(
    ST_ISP_MOD_DUMP *pstModDump,
    ST_SIZE *pstSize
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetBurstLength(
    ST_ISP_MOD_DUMP *pstModDump,
    STF_U8 u8BurstLength
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetPixelRIghtShiftBits(
    ST_ISP_MOD_DUMP *pstModDump,
    STF_U8 u8PixelRightShiftBits,
    eMOSAIC enMosaic
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetAxiId(
    ST_ISP_MOD_DUMP *pstModDump,
    STF_U8 u8AxiId
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetStride(
    ST_ISP_MOD_DUMP *pstModDump,
    STF_U32 u32Stride
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetDumpBuffer(
    STF_VOID *pIspCtx,
    STF_U16 *pu16DumpBuffer
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetDumpBufferInfo(
    STF_VOID *pIspCtx,
    ST_DUMP_BUF_INFO *pstDumpBufferInfo
    );
extern
ePxlFormat STFMOD_ISP_DUMP_GetPixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_DUMP_BUF_INFO STFMOD_ISP_DUMP_GetDumpBufferInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_GetDumpBuffer(
    STF_VOID *pIspCtx,
    STF_U16 **ppu16DumpBuffer
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DUMP_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DUMP_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DUMP_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_DUMP_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_DUMP_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_DUMP_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DUMP_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DUMP_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DUMP_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DUMP_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_DUMP_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_DUMP_H__
