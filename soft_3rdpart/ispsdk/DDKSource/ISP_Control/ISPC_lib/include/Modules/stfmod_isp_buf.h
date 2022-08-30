/**
  ******************************************************************************
  * @file  stfmod_isp_buf.h
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


#ifndef __STFMOD_ISP_BUF_H__
#define __STFMOD_ISP_BUF_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_buf_rdma.h"
#include "stflib_isp_pipeline.h"


/* ISP video output module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define BUF_PIXEL_BITS                          (8)
#define BUF_PIXEL_VALUE_MAX                     ((1 << BUF_PIXEL_BITS) - 1)
#define BUF_ALIGN_BITS                          (3)
#define BUF_ALIGN_MASK                          ((1 << BUF_ALIGN_BITS) - 1)
#define BUF_ALIGNMENT                           (1 << BUF_ALIGN_BITS)
#define BUF_SCALE_FACTOR_BITS                   (12)
#define BUF_SCALE_FACTOR_MAX                    (1 << BUF_SCALE_FACTOR_BITS)

#define ISP_DEF_BUF_USING_IMG_SIZE_FOR_CALC     (STF_TRUE)          /** Turn this flag to using image size to calculate the scaling factor. */
#define ISP_DEF_BUF_UO_IMAGE_WIDTH              (ISP_IMG_WIDTH)     /** Unscaled output image width. */
#define ISP_DEF_BUF_UO_IMAGE_HEIGHT             (ISP_IMG_HEIGHT)    /** Unscaled output image Height. */
#define ISP_DEF_BUF_UO_UV_MODET                 (STF_FALSE)         /** Output image UV swap, 0 : NV12, 1 : NV21. */
#define ISP_DEF_BUF_UO_AXI_ID                   (0)                 /** ID of AXI bus for unscaled output image write. */
#define ISP_DEF_BUF_UO_STRIDE                   (1920)              /** unscaled output buffer stride, 8 byte aligned. */
#define ISP_DEF_BUF_UO_BUFFER_ADDRESS           (0x6C800000)        /** Point to unscaled output buffer address. */

#define ISP_DEF_BUF_SS0_IMAGE_WIDTH             (1280)              /** Scaled stream 0 output image width. */
#define ISP_DEF_BUF_SS0_IMAGE_HEIGHT            (720)               /** Scaled stream 0 output image Height. */
#define ISP_DEF_BUF_SS0_UV_MODET                (ISP_DEF_BUF_UO_UV_MODET) /** Output image UV swap, 0 : NV12, 1 : NV21. */
#define ISP_DEF_BUF_SS0_H_SCALE_MODE            (0)                 /** Scaled stream 0 output image horizontal Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_BUF_SS0_H_SCALE_FACTOR          (0.666748047)       /** Scaled stream 0 output image horizontal Scaling factor. */
#define ISP_DEF_BUF_SS0_V_SCALE_MODE            (0)                 /** Scaled stream 0 output image vertical Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_BUF_SS0_V_SCALE_FACTOR          (0.666748047)       /** Scaled stream 0 output image vertical Scaling factor. */
#define ISP_DEF_BUF_SS0_AXI_ID                  (0)                 /** ID of AXI bus for scaled stream 0 output image write. */
#define ISP_DEF_BUF_SS0_STRIDE                  (1280)              /** scaled stream 0 output buffer stride, 8 byte aligned. */
#define ISP_DEF_BUF_SS0_BUFFER_ADDRESS          (0x6CC00000)        /** Point to scaled stream 0 output buffer address. */

#define ISP_DEF_BUF_SS1_IMAGE_WIDTH             (960)               /** Scaled stream 1 output image width. */
#define ISP_DEF_BUF_SS1_IMAGE_HEIGHT            (540)               /** Scaled stream 1 output image Height. */
#define ISP_DEF_BUF_SS1_UV_MODET                (ISP_DEF_BUF_UO_UV_MODET) /** Output image UV swap, 0 : NV12, 1 : NV21. */
#define ISP_DEF_BUF_SS1_H_SCALE_MODE            (0)                 /** Scaled stream 1 output image horizontal Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_BUF_SS1_H_SCALE_FACTOR          (0.666748047)       /** Scaled stream 1 output image horizontal Scaling factor. */
#define ISP_DEF_BUF_SS1_V_SCALE_MODE            (0)                 /** Scaled stream 1 output image vertical Scaling mode, 0 : scaling down, 2 : no scaling. */
#define ISP_DEF_BUF_SS1_V_SCALE_FACTOR          (0.666748047)       /** Scaled stream 1 output image vertical Scaling factor. */
#define ISP_DEF_BUF_SS1_AXI_ID                  (0)                 /** ID of AXI bus for scaled stream 1 output image write. */
#define ISP_DEF_BUF_SS1_STRIDE                  (960)               /** scaled stream 1 output buffer stride, 8 byte aligned. */
#define ISP_DEF_BUF_SS1_BUFFER_ADDRESS          (0x6D000000)        /** Point to scaled stream 1 output buffer address. */


#pragma pack(push, 1)

//=== Godspeed === Add new memory/buffer type support here.
typedef enum _EN_BUF_UPDATE {
    EN_BUF_UPDATE_NONE = 0X00000000,
    EN_BUF_UPDATE_UO_BUFFER = 0x00000001,
    EN_BUF_UPDATE_SS0_BUFFER = 0x00000002,
    EN_BUF_UPDATE_SS1_BUFFER = 0x00000004,
    EN_BUF_UPDATE_DUMP_BUFFER = 0x00000008,
    EN_BUF_UPDATE_TILING_1_READ_BUFFER = 0x00000010,
    EN_BUF_UPDATE_TILING_1_WRITE_BUFFER = 0x00000020,
    EN_BUF_UPDATE_SC_DUMP_BUFFER = 0x00000100,
    EN_BUF_UPDATE_Y_HIST_BUFFER = 0x00000200,
    EN_BUF_UPDATE_ALL = (EN_BUF_UPDATE_UO_BUFFER
                       | EN_BUF_UPDATE_SS0_BUFFER
                       | EN_BUF_UPDATE_SS1_BUFFER
                       | EN_BUF_UPDATE_DUMP_BUFFER
                       | EN_BUF_UPDATE_TILING_1_READ_BUFFER
                       | EN_BUF_UPDATE_TILING_1_WRITE_BUFFER
                       | EN_BUF_UPDATE_SC_DUMP_BUFFER
                       | EN_BUF_UPDATE_Y_HIST_BUFFER
                       ),
} EN_BUF_UPDATE, *PEN_BUF_UPDATE;


typedef struct _ST_BUF_INFO {
    STF_U16 u16Stride;                          /** Output/Input image line stride. */
    STF_U16 u16Height;                          /** Output/Input image height. */
    STF_VOID *pvBuffer;                         /** Output/Input image RAW/RGB/Y plane data store buffer. */
} ST_BUF_INFO, *PST_BUF_INFO;

typedef struct _ST_MOD_BUF_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable the BUF module. */
} ST_MOD_BUF_SETTING, *PST_MOD_BUF_SETTING;

//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ST_BUF_PARAM {
    ST_MOD_BUF_SETTING stSetting;               /** Module BUF setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program BUF module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
    ST_BUF_INFO stUoBuf;                        /** Unscaled output image buffer information. */
    ST_BUF_INFO stSs0Buf;                       /** Scaled stream 0 output image buffer information. */
    ST_BUF_INFO stSs1Buf;                       /** Scaled stream 1 output image buffer information. */
    ST_BUF_INFO stDumpBuf;                      /** CSI raw data dump image buffer information. */
    ST_BUF_INFO stTiling_1_ReadBuf;             /** Tiling 1 read input image buffer information. */
    ST_BUF_INFO stTiling_1_WriteBuf;            /** Tiling 1 write input image buffer information. */
    ST_BUF_INFO stScDumpBuf;                    /** statistics collection dump buffer information. */
    ST_BUF_INFO stYHistBuf;                     /** Y histogram dump buffer information. */
} ST_BUF_PARAM, *PST_BUF_PARAM;

#define ST_MOD_BUF_SETTING_LEN          (sizeof(ST_MOD_BUF_SETTING))
#define ST_MOD_BUF_GET_PARAM_LEN        (ST_MOD_BUF_SETTING_LEN)
#define ST_MOD_BUF_SET_PARAM_LEN        (ST_MOD_BUF_SETTING_LEN)
//#define ST_BUF_PARAM_SIZE               (sizeof(ST_BUF_PARAM) - (sizeof(ST_BUF_INFO) * 8))
#define ST_BUF_PARAM_SIZE               (sizeof(ST_BUF_PARAM))
#define ST_BUF_PARAM_LEN                (ST_BUF_PARAM_SIZE)

typedef struct _ST_ISP_MOD_BUF {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstBufRdmaBuf;                /** BUF's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_BUF_RDMA *pstIspBufRdma;             /** BUF's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_BUF_RDMA *pstBufRdma;                    /** BUF's RDMA structure pointer.*/
    ST_BUF_PARAM *pstBufParam;                  /** BUF's parameters structure pointer. */
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
} ST_ISP_MOD_BUF, *PST_ISP_MOD_BUF;

#pragma pack(pop)


/* ISP video output module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_BUF_InitRdma(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateRdma(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
//-----------------------------------------------------------------------------
//=== Godspeed === Add new memory/buffer type support here.
extern
STF_S32 STFMOD_ISP_BUF_UpdateUoBuffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateSs0Buffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateSs1Buffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateDumpBuffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateTiling_1_ReadBuffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateTiling_1_WriteBuffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateScDumpBuffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateYHistBuffer(
    ST_BUF_RDMA *pstBufRdma,
    ST_BUF_PARAM *pstBufParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_BUF_SetNextRdma(
    ST_BUF_RDMA *pstBufRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_BUF_SetEnable(
    ST_ISP_MOD_BUF *pstModBuf,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_BUF_SetUpdate(
    ST_ISP_MOD_BUF *pstModBuf,
    STF_BOOL8 bUpdate
    );
//=== Godspeed === Add new memory/buffer type support here.
extern
STF_S32 STFMOD_ISP_BUF_SetUoBuffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetUoBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
extern
STF_S32 STFMOD_ISP_BUF_SetSs0Buffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetSs0Buffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
extern
STF_S32 STFMOD_ISP_BUF_SetSs1Buffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetSs1Buffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
extern
STF_S32 STFMOD_ISP_BUF_SetDumpBuffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetDumpBuffer(
    STF_VOID *pIspCtx,
    STF_U16 **ppu16Buffer
    );
extern
STF_S32 STFMOD_ISP_BUF_SetTiling_1_ReadBuffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetTiling_1_ReadBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
extern
STF_S32 STFMOD_ISP_BUF_SetTiling_1_WriteBuffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetTiling_1_WriteBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
extern
STF_S32 STFMOD_ISP_BUF_SetScDumpBuffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetScDumpBuffer(
    STF_VOID *pIspCtx,
    STF_U16 **ppu16Buffer
    );
extern
STF_S32 STFMOD_ISP_BUF_SetYHistBuffer(
    STF_VOID *pIspCtx,
    ST_BUF_INFO *pstBufferInfo
    );
extern
STF_S32 STFMOD_ISP_BUF_GetYHistBuffer(
    STF_VOID *pIspCtx,
    STF_U16 **ppu16Buffer
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_BUF_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_BUF_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_BUF_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_BUF_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_BUF_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_BUF_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_BUF_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_BUF_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_BUF_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_BUF_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_BUF_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_BUF_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_BUF_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_BUF_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_BUF_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_BUF_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_BUF_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_BUF_H__
