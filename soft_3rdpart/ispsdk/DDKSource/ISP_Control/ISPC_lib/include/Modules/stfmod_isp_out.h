/**
  ******************************************************************************
  * @file  stfmod_isp_out.h
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


#ifndef __STFMOD_ISP_OUT_H__
#define __STFMOD_ISP_OUT_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "stf_common/ci_alloc_info.h"
#include "registers/stf_isp_out_rdma.h"
#include "stflib_isp_pipeline.h"


/* ISP video output module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define OUT_PIXEL_BITS                          (8)
#define OUT_PIXEL_VALUE_MAX                     ((1 << OUT_PIXEL_BITS) - 1)
#define OUT_ALIGN_BITS                          (3)
#define OUT_ALIGN_MASK                          ((1 << OUT_ALIGN_BITS) - 1)
#define OUT_ALIGNMENT                           (1 << OUT_ALIGN_BITS)
#define OUT_SCALE_FACTOR_BITS                   (12)
#define OUT_SCALE_FACTOR_MAX                    (1 << OUT_SCALE_FACTOR_BITS)

#define ISP_DEF_OUT_USING_IMG_SIZE_FOR_CALC     (STF_TRUE)          /** Turn this flag to using image size to calculate the scaling factor. */
#define ISP_DEF_OUT_UV_MODET                    (STF_FALSE)         /** Output image UV swap, 0 : NV12, 1 : NV21. */

#define ISP_DEF_OUT_UO_IMAGE_WIDTH              (ISP_IMG_WIDTH)     /** Unscaled output image width. */
#define ISP_DEF_OUT_UO_IMAGE_HEIGHT             (ISP_IMG_HEIGHT)    /** Unscaled output image Height. */
#define ISP_DEF_OUT_UO_AXI_ID                   (0)                 /** ID of AXI bus for unscaled output image write. */
#define ISP_DEF_OUT_UO_STRIDE                   (1920)              /** unscaled output buffer stride, 8 byte aligned. */
#define ISP_DEF_OUT_UO_BUFFER_ADDRESS           (0x6C800000)        /** Point to unscaled output buffer address. */

#if defined(V4L2_DRIVER)
#define ISP_DEF_OUT_SS0_IMAGE_WIDTH             (1920)              /** Scaled stream 0 output image width. */
#define ISP_DEF_OUT_SS0_IMAGE_HEIGHT            (1080)              /** Scaled stream 0 output image Height. */
#define ISP_DEF_OUT_SS0_UV_MODET                (ISP_DEF_OUT_UO_UV_MODET) /** Output image UV swap, 0 : NV12, 1 : NV21. */
#define ISP_DEF_OUT_SS0_H_SCALE_MODE            (2)                 /** Scaled stream 0 output image horizontal Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS0_H_SCALE_FACTOR          (0.0)               /** Scaled stream 0 output image horizontal Scaling factor. */
#define ISP_DEF_OUT_SS0_V_SCALE_MODE            (2)                 /** Scaled stream 0 output image vertical Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS0_V_SCALE_FACTOR          (0.0)               /** Scaled stream 0 output image vertical Scaling factor. */
#define ISP_DEF_OUT_SS0_AXI_ID                  (0)                 /** ID of AXI bus for scaled stream 0 output image write. */
#define ISP_DEF_OUT_SS0_STRIDE                  (1920)              /** scaled stream 0 output buffer stride, 8 byte aligned. */
#else
#define ISP_DEF_OUT_SS0_IMAGE_WIDTH             (1280)              /** Scaled stream 0 output image width. */
#define ISP_DEF_OUT_SS0_IMAGE_HEIGHT            (720)               /** Scaled stream 0 output image Height. */
#define ISP_DEF_OUT_SS0_UV_MODET                (ISP_DEF_OUT_UO_UV_MODET) /** Output image UV swap, 0 : NV12, 1 : NV21. */
#define ISP_DEF_OUT_SS0_H_SCALE_MODE            (0)                 /** Scaled stream 0 output image horizontal Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS0_H_SCALE_FACTOR          (0.666748047)       /** Scaled stream 0 output image horizontal Scaling factor. */
#define ISP_DEF_OUT_SS0_V_SCALE_MODE            (0)                 /** Scaled stream 0 output image vertical Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS0_V_SCALE_FACTOR          (0.666748047)       /** Scaled stream 0 output image vertical Scaling factor. */
#define ISP_DEF_OUT_SS0_AXI_ID                  (0)                 /** ID of AXI bus for scaled stream 0 output image write. */
#define ISP_DEF_OUT_SS0_STRIDE                  (1280)              /** scaled stream 0 output buffer stride, 8 byte aligned. */
#endif //#if defined(V4L2_DRIVER)
#define ISP_DEF_OUT_SS0_BUFFER_ADDRESS          (0x6CC00000)        /** Point to scaled stream 0 output buffer address. */

#if defined(V4L2_DRIVER)
#define ISP_DEF_OUT_SS1_IMAGE_WIDTH             (1920)              /** Scaled stream 1 output image width. */
#define ISP_DEF_OUT_SS1_IMAGE_HEIGHT            (1080)              /** Scaled stream 1 output image Height. */
#define ISP_DEF_OUT_SS1_UV_MODET                (ISP_DEF_OUT_UO_UV_MODET) /** Output image UV swap, 0 : NV12, 1 : NV21. */
#define ISP_DEF_OUT_SS1_H_SCALE_MODE            (2)                 /** Scaled stream 1 output image horizontal Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS1_H_SCALE_FACTOR          (0.0)               /** Scaled stream 1 output image horizontal Scaling factor. */
#define ISP_DEF_OUT_SS1_V_SCALE_MODE            (2)                 /** Scaled stream 1 output image vertical Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS1_V_SCALE_FACTOR          (0.0)               /** Scaled stream 1 output image vertical Scaling factor. */
#define ISP_DEF_OUT_SS1_AXI_ID                  (0)                 /** ID of AXI bus for scaled stream 1 output image write. */
#define ISP_DEF_OUT_SS1_STRIDE                  (1920)              /** scaled stream 1 output buffer stride, 8 byte aligned. */
#else
#define ISP_DEF_OUT_SS1_IMAGE_WIDTH             (960)               /** Scaled stream 1 output image width. */
#define ISP_DEF_OUT_SS1_IMAGE_HEIGHT            (540)               /** Scaled stream 1 output image Height. */
#define ISP_DEF_OUT_SS1_UV_MODET                (ISP_DEF_OUT_UO_UV_MODET) /** Output image UV swap, 0 : NV12, 1 : NV21. */
#define ISP_DEF_OUT_SS1_H_SCALE_MODE            (0)                 /** Scaled stream 1 output image horizontal Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS1_H_SCALE_FACTOR          (0.5)               /** Scaled stream 1 output image horizontal Scaling factor. */
#define ISP_DEF_OUT_SS1_V_SCALE_MODE            (0)                 /** Scaled stream 1 output image vertical Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_OUT_SS1_V_SCALE_FACTOR          (0.5)               /** Scaled stream 1 output image vertical Scaling factor. */
#define ISP_DEF_OUT_SS1_AXI_ID                  (0)                 /** ID of AXI bus for scaled stream 1 output image write. */
#define ISP_DEF_OUT_SS1_STRIDE                  (960)               /** scaled stream 1 output buffer stride, 8 byte aligned. */
#endif //#if defined(V4L2_DRIVER)
#define ISP_DEF_OUT_SS1_BUFFER_ADDRESS          (0x6D000000)        /** Point to scaled stream 1 output buffer address. */


#pragma pack(push, 1)

typedef enum _EN_OUT_UPDATE {
    EN_OUT_UPDATE_NONE = 0X00000000,
    EN_OUT_UPDATE_UV_MODE = 0x00000001,
    EN_OUT_UPDATE_UO_AXI_ID = 0x00000002,
    EN_OUT_UPDATE_UO_SIZE = 0x00000004,
    EN_OUT_UPDATE_UO_STRIDE = 0x00000008,
    EN_OUT_UPDATE_UO_BUFFER = 0x00000010,
    EN_OUT_UPDATE_SS0_H_SCALE_MODE = 0x00000020,
    EN_OUT_UPDATE_SS0_H_SCALE_FACTOR = 0x00000040,
    EN_OUT_UPDATE_SS0_V_SCALE_MODE = 0x00000080,
    EN_OUT_UPDATE_SS0_V_SCALE_FACTOR = 0x00000100,
    EN_OUT_UPDATE_SS0_AXI_ID = 0x00000200,
    EN_OUT_UPDATE_SS0_SIZE = 0x00000400,
    EN_OUT_UPDATE_SS0_STRIDE = 0x00000800,
    EN_OUT_UPDATE_SS0_BUFFER = 0x00001000,
    EN_OUT_UPDATE_SS1_H_SCALE_MODE = 0x00002000,
    EN_OUT_UPDATE_SS1_H_SCALE_FACTOR = 0x00004000,
    EN_OUT_UPDATE_SS1_V_SCALE_MODE = 0x00008000,
    EN_OUT_UPDATE_SS1_V_SCALE_FACTOR = 0x00010000,
    EN_OUT_UPDATE_SS1_AXI_ID = 0x00020000,
    EN_OUT_UPDATE_SS1_SIZE = 0x00040000,
    EN_OUT_UPDATE_SS1_STRIDE = 0x00080000,
    EN_OUT_UPDATE_SS1_BUFFER = 0x00100000,
    EN_OUT_UPDATE_UO_WO_BUF_INFO = (EN_OUT_UPDATE_UO_AXI_ID
                      ),
    EN_OUT_UPDATE_UO = (EN_OUT_UPDATE_UO_AXI_ID
                      | EN_OUT_UPDATE_UO_SIZE
                      | EN_OUT_UPDATE_UO_STRIDE
                      | EN_OUT_UPDATE_UO_BUFFER
                      ),
    EN_OUT_UPDATE_SS0_WO_BUF_INFO = (EN_OUT_UPDATE_SS0_H_SCALE_MODE
                       | EN_OUT_UPDATE_SS0_H_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS0_V_SCALE_MODE
                       | EN_OUT_UPDATE_SS0_V_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS0_AXI_ID
                       ),
    EN_OUT_UPDATE_SS0 = (EN_OUT_UPDATE_SS0_H_SCALE_MODE
                       | EN_OUT_UPDATE_SS0_H_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS0_V_SCALE_MODE
                       | EN_OUT_UPDATE_SS0_V_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS0_AXI_ID
                       | EN_OUT_UPDATE_SS0_SIZE
                       | EN_OUT_UPDATE_SS0_STRIDE
                       | EN_OUT_UPDATE_SS0_BUFFER
                       ),
    EN_OUT_UPDATE_SS1_WO_BUF_INFO = (EN_OUT_UPDATE_SS1_H_SCALE_MODE
                       | EN_OUT_UPDATE_SS1_H_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS1_V_SCALE_MODE
                       | EN_OUT_UPDATE_SS1_V_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS1_AXI_ID
                       ),
    EN_OUT_UPDATE_SS1 = (EN_OUT_UPDATE_SS1_H_SCALE_MODE
                       | EN_OUT_UPDATE_SS1_H_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS1_V_SCALE_MODE
                       | EN_OUT_UPDATE_SS1_V_SCALE_FACTOR
                       | EN_OUT_UPDATE_SS1_AXI_ID
                       | EN_OUT_UPDATE_SS1_SIZE
                       | EN_OUT_UPDATE_SS1_STRIDE
                       | EN_OUT_UPDATE_SS1_BUFFER
                       ),
    EN_OUT_UPDATE_ALL_WO_BUF_INFO = (EN_OUT_UPDATE_UV_MODE
                       | EN_OUT_UPDATE_UO_WO_BUF_INFO
                       | EN_OUT_UPDATE_SS0_WO_BUF_INFO
                       | EN_OUT_UPDATE_SS1_WO_BUF_INFO
                       ),
    EN_OUT_UPDATE_ALL = (EN_OUT_UPDATE_UV_MODE
                       | EN_OUT_UPDATE_UO
                       | EN_OUT_UPDATE_SS0
                       | EN_OUT_UPDATE_SS1
                       ),
} EN_OUT_UPDATE, *PEN_OUT_UPDATE;


//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ST_OUT_BUF_INFO {
    CI_BUFFTYPE enAllocBufferType;              /** Output image buffer type.
                                                  *  0 : CI_TYPE_NONE,
                                                  *  1 : CI_TYPE_UO,
                                                  *  2 : CI_TYPE_SS0,
                                                  *  3 : CI_TYPE_SS1,
                                                  *  4 : CI_TYPE_DUMP,
                                                  *  5 : CI_TYPE_TILING_1_RD,
                                                  *  6 : CI_TYPE_TILING_1_WR,
                                                  *  7 : CI_TYPE_SC_DUMP,
                                                  *  8 : CI_TYPE_Y_HIST
                                                  */
    eFORMAT_TYPES enFormatType;                 /** output image buffer format type. 0:TYPE_NONE, 1:TYPE_RGB, 2:TYPE_YUV and 3:TYPE_BAYER.  */
    ePxlFormat enPixelFormat;                   /** Output image pixel format. 0:PXL_NONE, 1:YVU_420_PL12_8, 2:YUV_420_PL12_8, ..., PXL_N and PXL_INVALID. */
    eMOSAIC enMosaic;                           /** Output image mosaic type. 0:MOSAIC_NONE, 1:MOSAIC_RGGB, 2:MOSAIC_GRBG, 3:MOSAIC_GBRG and 4:MOSAIC_BGGR. */
    ST_SIZE stSize;                             /** Output image size. */
    STF_U16 u16Stride;                          /** Output image line stride, 8-byte(64bit) granularity. */
    STF_VOID *pvBuffer;                         /** Output image Y plane data store buffer. */
} ST_OUT_BUF_INFO, *PST_OUT_BUF_INFO;

typedef struct _ST_UO_INFO {
    ST_SIZE stSize;                             /** Unscaled output image size. */
    STF_U8 u8AxiId;                             /** ID of AXI bus for unscaled output image write. */
} ST_UO_INFO, *PST_UO_INFO;

typedef struct _ST_SS_INFO {
    ST_SIZE stSize;                             /** Scaled stream output image size. */
    STF_U8 u8HorScalingMode;                    /** Scaled stream output image horizontal Scaling mode, 0 : scaling down, 2 : no scaling, 1 and 3 are reserved values. */
    STF_DOUBLE dHorScalingFactor;               /** Scaled stream output image horizontal Scaling factor, format 0.0.12 (0.999755859375). */
    STF_U8 u8VerScalingMode;                    /** Scaled stream output image vertical Scaling mode, 0 : scaling down, 2 : no scaling, 1 and 3 are reserved values. */
    STF_DOUBLE dVerScalingFactor;               /** Scaled stream output image vertical Scaling factor, format 0.0.12 (0.999755859375). */
    STF_U8 u8AxiId;                             /** ID of AXI bus for scaled stream output image write. */
} ST_SS_INFO, *PST_SS_INFO;

typedef struct _ST_MOD_OUT_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnableUo;                        /** Enable/Disable the unscaled output module. */
    STF_BOOL8 bEnableSs0;                       /** Enable/Disable the scaled stream 0 output module. */
    STF_BOOL8 bEnableSs1;                       /** Enable/Disable the scaled stream 1 output module. */
    STF_BOOL8 bUsingImageSizeForCalc;           /** Enable this flag to using the assign image size to calculate the scaling factor. */
    STF_BOOL8 bIsNV21;                          /** UO/SS0/SS1 image UV swap, 0 : NV12, 1 : NV21. */
    ST_UO_INFO stUoInfo;                        /** Unscaled output image information. */
    ST_SS_INFO stSs0Info;                       /** Scaled stream 0 output image information. */
    ST_SS_INFO stSs1Info;                       /** Scaled stream 1 output image information. */
} ST_MOD_OUT_SETTING, *PST_MOD_OUT_SETTING;

typedef struct _ST_OUT_PARAM {
    ST_MOD_OUT_SETTING stSetting;               /** Module OUT setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdateUo;                        /** Enable/Disable program OUT_UO module register. */
    STF_BOOL8 bUpdateSs0;                       /** Enable/Disable program OUT_SS0 module register. */
    STF_BOOL8 bUpdateSs1;                       /** Enable/Disable program OUT_SS1 module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
    ST_OUT_BUF_INFO stUoBufInfo;                /** Unscaled output image buffer information. */
    ST_OUT_BUF_INFO stSs0BufInfo;               /** Scaled stream 0 output image buffer information. */
    ST_OUT_BUF_INFO stSs1BufInfo;               /** Scaled stream 1 output image buffer information. */
} ST_OUT_PARAM, *PST_OUT_PARAM;

#define ST_MOD_OUT_SETTING_LEN          (sizeof(ST_MOD_OUT_SETTING))
#define ST_MOD_OUT_GET_PARAM_LEN        (ST_MOD_OUT_SETTING_LEN)
#define ST_MOD_OUT_SET_PARAM_LEN        (ST_MOD_OUT_SETTING_LEN)
//#define ST_OUT_PARAM_SIZE               (sizeof(ST_OUT_PARAM) - (sizeof(ST_OUT_BUF_INFO) * 3))
#define ST_OUT_PARAM_SIZE               (sizeof(ST_OUT_PARAM))
#define ST_OUT_PARAM_LEN                (ST_OUT_PARAM_SIZE)

typedef struct _ST_ISP_MOD_OUT {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstOutRdmaBuf;                /** OUT's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_OUT_RDMA *pstIspOutRdma;             /** OUT's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_OUT_RDMA *pstOutRdma;                    /** OUT's RDMA structure pointer.*/
    ST_OUT_PARAM *pstOutParam;                  /** OUT's parameters structure pointer. */
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
} ST_ISP_MOD_OUT, *PST_ISP_MOD_OUT;

#pragma pack(pop)


/* ISP video output module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_U16 STFMOD_ISP_OUT_ScalingFactorCalculate(
    STF_U32 u32SrcSize,
    STF_U32 u32DstSize
    );
extern
STF_U32 STFMOD_ISP_OUT_DestinationSizeCalculateByFixPointScaling(
    STF_U32 u32SrcSize,
    STF_U16 u16ScalingFactor
    );
extern
STF_U32 STFMOD_ISP_OUT_DestinationSizeCalculateDoubleScaling(
    STF_U32 u32SrcSize,
    STF_DOUBLE dScalingFactor
    );
extern
STF_S32 STFMOD_ISP_OUT_CalOutputBufInfo(
    CI_BUFFTYPE enAllocBufferType,
    ST_SIZE *pstSize,
    ST_OUT_BUF_INFO *pstOutBufInfo,
    PIXELTYPE *pstPxlType,
    struct CI_SIZEINFO *pstSizeInfo
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_InitRdma(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateRdma(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_UpdateUvModeRdma(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_UpdateUoRdma(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateUoAxiId(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
#if defined(ENABLE_UPDATE_OUT_UO_SIZE)
extern
STF_S32 STFMOD_ISP_OUT_UpdateUoSize(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
#endif //ENABLE_UPDATE_OUT_UO_SIZE
extern
STF_S32 STFMOD_ISP_OUT_UpdateUoStride(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateUoBuffer(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0Rdma(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0HorizontalScaleMode(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0HorizontalScaleFactor(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0VerticalScaleMode(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0VerticalScaleFactor(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0AxiId(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0Size(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0Stride(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0Buffer(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1Rdma(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1HorizontalScaleMode(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1HorizontalScaleFactor(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1VerticalScaleMode(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1VerticalScaleFactor(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1AxiId(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1Size(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1Stride(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1Buffer(
    ST_OUT_RDMA *pstOutRdma,
    ST_OUT_PARAM *pstOutParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_SetNextRdma(
    ST_OUT_RDMA *pstOutRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_SetEnableUsingImageSizeForCalc(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bUsingImageSizeForCalc
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUvMode(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bIsNV21
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_SetEnable(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUpdate(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bUpdate
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_SetEnableUo(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUpdateUo(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUoInfo(
    ST_ISP_MOD_OUT *pstModOut,
    ST_UO_INFO *pstUoInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUoBufInfo(
    STF_VOID *pIspCtx,
    ST_OUT_BUF_INFO *pstBufInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUoAxiId(
    ST_ISP_MOD_OUT *pstModOut,
    STF_U8 u8AxiId
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUoSize(
    ST_ISP_MOD_OUT *pstModOut,
    ST_SIZE stSize
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUoStride(
    STF_VOID *pIspCtx,
    STF_U16 u16Stride
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUoBuffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvBuffer
    );
extern
ePxlFormat STFMOD_ISP_OUT_GetUoPixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_OUT_BUF_INFO STFMOD_ISP_OUT_GetUoBufInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_GetUoBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_SetEnableSs0(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUpdateSs0(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0Info(
    ST_ISP_MOD_OUT *pstModOut,
    ST_SS_INFO *pstSSInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0BufInfo(
    STF_VOID *pIspCtx,
    ST_OUT_BUF_INFO *pstBufInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0HorizontalScalingMode(
    ST_ISP_MOD_OUT *pstModOut,
    STF_U8 u8ScalingMode
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0HorizontalScalingFactor(
    ST_ISP_MOD_OUT *pstModOut,
    STF_DOUBLE dScalingFactor
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0VerticalScalingMode(
    ST_ISP_MOD_OUT *pstModOut,
    STF_U8 u8ScalingMode
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0VerticalScalingFactor(
    ST_ISP_MOD_OUT *pstModOut,
    STF_DOUBLE dScalingFactor
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0AxiId(
    ST_ISP_MOD_OUT *pstModOut,
    STF_U8 u8AxiId
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0Size(
    ST_ISP_MOD_OUT *pstModOut,
    ST_SIZE stSize
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0Stride(
    STF_VOID *pIspCtx,
    STF_U16 u16Stride
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs0Buffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvBuffer
    );
extern
ePxlFormat STFMOD_ISP_OUT_GetSs0PixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_OUT_BUF_INFO STFMOD_ISP_OUT_GetSs0BufInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_GetSs0Buffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_SetEnableSs1(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OUT_SetUpdateSs1(
    ST_ISP_MOD_OUT *pstModOut,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1Info(
    ST_ISP_MOD_OUT *pstModOut,
    ST_SS_INFO *pstSSInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1BufInfo(
    STF_VOID *pIspCtx,
    ST_OUT_BUF_INFO *pstBufInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1HorizontalScalingMode(
    ST_ISP_MOD_OUT *pstModOut,
    STF_U8 u8ScalingMode
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1HorizontalScalingFactor(
    ST_ISP_MOD_OUT *pstModOut,
    STF_DOUBLE dScalingFactor
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1VerticalScalingMode(
    ST_ISP_MOD_OUT *pstModOut,
    STF_U8 u8ScalingMode
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1VerticalScalingFactor(
    ST_ISP_MOD_OUT *pstModOut,
    STF_DOUBLE dScalingFactor
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1AxiId(
    ST_ISP_MOD_OUT *pstModOut,
    STF_U8 u8AxiId
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1Size(
    ST_ISP_MOD_OUT *pstModOut,
    ST_SIZE stSize
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1Stride(
    STF_VOID *pIspCtx,
    STF_U16 u16Stride
    );
extern
STF_S32 STFMOD_ISP_OUT_SetSs1Buffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvBuffer
    );
extern
ePxlFormat STFMOD_ISP_OUT_GetSs1PixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_OUT_BUF_INFO STFMOD_ISP_OUT_GetSs1BufInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_GetSs1Buffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OUT_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OUT_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OUT_EnableUo(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OUT_EnableSs0(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OUT_EnableSs1(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsUoEnable(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsSs0Enable(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsSs1Enable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateUo(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs0(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateSs1(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsUpdateUo(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsUpdateSs0(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsUpdateSs1(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OUT_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OUT_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_OUT_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_OUT_H__
