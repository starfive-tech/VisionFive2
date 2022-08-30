/**
  ******************************************************************************
  * @file  stfmod_isp_yhist.h
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


#ifndef __STFMOD_ISP_YHIST_H__
#define __STFMOD_ISP_YHIST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_yhist_rdma.h"
#include "stflib_isp_pipeline.h"


/* y histogram module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define YHIST_PIXEL_BITS                        (10)
#define YHIST_PIXEL_VALUE_MAX                   ((1 << YHIST_PIXEL_BITS) - 1)
#define YHIST_ACC_BITS                          (18)
#define YHIST_ACC_VLAUE_MAX                     ((1 << YHIST_ACC_BITS) - 1)
#define YHIST_BIN_BITS                          (6)
#define YHIST_ACC_BIN                           (1 << YHIST_BIN_BITS)

#define ISP_DEF_YHIST_USING_IMG_SIZE_FOR_CALC   (STF_TRUE)      /** Turn this flag to using image size to calculate the Y Histogram decimation information. */
#define ISP_DEF_YHIST_DUMPING_BUFFER_ADDRESS    (0x68004400)    /** Point to Y histogram dumping buffer(8 x 16byte align), need buffer size is 64 x 4byte. */
#define ISP_DEF_YHIST_IMAGE_WIDTH               (ISP_IMG_WIDTH) /** Image width. */
#define ISP_DEF_YHIST_IMAGE_HEIGHT              (ISP_IMG_HEIGHT) /** Image Height. */
#define ISP_DEF_YHIST_INPUT_SEL                 (EN_YHIST_IN_SEL_BEFORE_YCRV) /** Y histogram input source select. 0: before y curve, 1: after y curve. */
#define ISP_DEF_YHIST_CROP_DEC_H_START          (0)             /** Horizontal starting point for frame cropping. */
#define ISP_DEF_YHIST_CROP_DEC_V_START          (0)             /** Vertical starting point for frame cropping. */
#define ISP_DEF_YHIST_CROP_DEC_SUB_WIN_W        (1919)          /** Width of Y Histogram sub-window. (u8SubWinWidth + 1) */
#define ISP_DEF_YHIST_CROP_DEC_SUB_WIN_H        (1079)          /** Height of Y Histogram sub-window (u8SubWinHeight + 1) */
#define ISP_DEF_YHIST_CROP_DEC_H_PERIOD         (2)             /** Horizontal period(zero base) for input image decimation. */
#define ISP_DEF_YHIST_CROP_DEC_V_PERIOD         (1)             /** Vertical period(zero base) for input image decimation. */


#pragma pack(push, 1)

typedef enum _EN_YHIST_UPDATE {
    EN_YHIST_UPDATE_NONE = 0X00,
    EN_YHIST_UPDATE_DECIMATION = 0X01,
    EN_YHIST_UPDATE_INPUT_SELECT = 0X02,
    EN_YHIST_UPDATE_DUMPING_ADDRESS = 0X04,
    EN_YHIST_UPDATE_ALL_WO_BUF_INFO = (EN_YHIST_UPDATE_DECIMATION
                         | EN_YHIST_UPDATE_INPUT_SELECT
                         ),
    EN_YHIST_UPDATE_ALL = (EN_YHIST_UPDATE_DECIMATION
                         | EN_YHIST_UPDATE_INPUT_SELECT
                         | EN_YHIST_UPDATE_DUMPING_ADDRESS
                         ),
} EN_YHIST_UPDATE, *PEN_YHIST_UPDATE;


typedef enum _EN_YHIST_IN_SEL {
    EN_YHIST_IN_SEL_BEFORE_YCRV = 0,            /** 0 : before y curve, after R2Y. */
    EN_YHIST_IN_SEL_AFTER_YCRV,                 /** 1 : after y curve. */
    EN_YHIST_IN_SEL_MAX
} EN_YHIST_IN_SEL, *PEN_YHIST_IN_SEL;           /** Y histogram input MUX select. */


typedef struct _ST_YHIST_CROP_DEC {
    STF_U16 u16HStart;                          /** Horizontal starting point for frame cropping. */
    STF_U16 u16VStart;                          /** Vertical starting point for frame cropping. */
    STF_U16 u16RegionWidth;                     /** All of sub-window mapping region width. (width = u16Width + 1) */
    STF_U16 u16RegionHeight;                    /** All of sub-window mapping region height. (height = u16Height + 1) */
    STF_U8 u8HPeriod;                           /** Horizontal period(zero base) for input image decimation. */
    STF_U8 u8VPeriod;                           /** Vertical period(zero base) for input image decimation. */
} ST_YHIST_CROP_DEC, *PST_YHIST_CROP_DEC;

typedef struct _ST_MOD_YHIST_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable YHIST module. */
    STF_BOOL8 bUsingImageSizeForCalc;           /** Enable this flag to using the assign image size to calculate the offset, cropping and decimation information. */
    ST_YHIST_CROP_DEC stYHistCropDec;           /** Input image decimation for Y Histogram.
                                                 * You don't need to set this structure
                                                 * value, it will be calculated and
                                                 * assigned by call
                                                 * STFMOD_ISP_YHIST_UpdateDecimation()
                                                 * function. */
    STF_U8 u8InputSel;                          /** RSVD. Y histogram input source select. 0: before y curve, 1: after y curve. */
} ST_MOD_YHIST_SETTING, *PST_MOD_YHIST_SETTING;

typedef struct _ST_MOD_YHIST_INFO {
    ST_SIZE stImgSize;                          /** Image size. */
} ST_MOD_YHIST_INFO, *PST_MOD_YHIST_INFO;

typedef struct _ST_YHIST_PARAM {
    ST_MOD_YHIST_SETTING stSetting;             /** Module YHIST setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_YHIST_INFO stInfo;                   /** Module YHIST output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program YHIST module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
    STF_U32 *pu32YHistDumpBuf;                  /** Point to Y histogram dumping buffer(8 x 16byte align), need buffer size is 64 x 4 byte. */
} ST_YHIST_PARAM, *PST_YHIST_PARAM;

#define ST_MOD_YHIST_SETTING_LEN        (sizeof(ST_MOD_YHIST_SETTING))
#define ST_MOD_YHIST_INFO_LEN           (sizeof(ST_MOD_YHIST_INFO))
#define ST_MOD_YHIST_GET_PARAM_LEN      (ST_MOD_YHIST_SETTING_LEN + ST_MOD_YHIST_INFO_LEN)
#define ST_MOD_YHIST_SET_PARAM_LEN      (ST_MOD_YHIST_SETTING_LEN)
#define ST_YHIST_PARAM_SIZE             (sizeof(ST_YHIST_PARAM) - sizeof(STF_U32 *))
#define ST_YHIST_PARAM_LEN              (ST_YHIST_PARAM_SIZE)

typedef struct _ST_ISP_MOD_YHIST {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstYHistRdmaBuf;              /** YHIST's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_YHIST_RDMA *pstIspYHistRdma;         /** YHIST's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_YHIST_RDMA *pstYHistRdma;                /** YHIST's RDMA structure pointer.*/
    ST_YHIST_PARAM *pstYHistParam;              /** YHIST's parameters structure pointer. */
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
} ST_ISP_MOD_YHIST, *PST_ISP_MOD_YHIST;

#pragma pack(pop)


/* y histogram module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YHIST_InitRdma(
    ST_YHIST_RDMA *pstYHistRdma,
    ST_YHIST_PARAM *pstYHistParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_YHIST_UpdateRdma(
    ST_YHIST_RDMA *pstYHistRdma,
    ST_YHIST_PARAM *pstYHistParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YHIST_UpdateDecimation(
    ST_YHIST_RDMA *pstYHistRdma,
    ST_YHIST_PARAM *pstYHistParam
    );
extern
STF_S32 STFMOD_ISP_YHIST_UpdateInputSelect(
    ST_YHIST_RDMA *pstYHistRdma,
    ST_YHIST_PARAM *pstYHistParam
    );
extern
STF_S32 STFMOD_ISP_YHIST_UpdateDumpingAddress(
    ST_YHIST_RDMA *pstYHistRdma,
    ST_YHIST_PARAM *pstYHistParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YHIST_SetNextRdma(
    ST_YHIST_RDMA *pstYHistRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YHIST_SetEnable(
    ST_ISP_MOD_YHIST *pstModYHist,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetUpdate(
    ST_ISP_MOD_YHIST *pstModYHist,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetEnableUsingImageSizeForCalc(
    ST_ISP_MOD_YHIST *pstModYHist,
    STF_BOOL8 bUsingImageSizeForCalc
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetImageSize(
    ST_ISP_MOD_YHIST *pstModYHist,
    ST_SIZE stImgSize
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetDecimation(
    ST_ISP_MOD_YHIST *pstModYHist,
    PST_YHIST_CROP_DEC pstYHistCropDec
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetInputSelect(
    ST_ISP_MOD_YHIST *pstModYHist,
    EN_YHIST_IN_SEL enInputSel
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetDumpingBuffer(
    STF_VOID *pIspCtx,
    STF_U32 *pu32YHistDumpBuf
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YHIST_GetDumpingBuffer(
    STF_VOID *pIspCtx,
    STF_U32 **ppu32YHistDumpBuf
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YHIST_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_YHIST_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_YHIST_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YHIST_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_YHIST_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YHIST_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_YHIST_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YHIST_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_YHIST_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YHIST_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_YHIST_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YHIST_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_YHIST_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YHIST_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_YHIST_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_YHIST_H__
