/**
  ******************************************************************************
  * @file  stfctl_isp_dnyuv.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  12/23/2020
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


#ifndef __STFCTL_ISP_DNYUV_H__
#define __STFCTL_ISP_DNYUV_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "Modules/stfmod_isp_dnyuv.h"


/* 2D denoise DNYUV control parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)


typedef struct _ST_CTL_DNYUV_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable Y curve control. */
    ST_DNYUV_CRV stDnYuvCurveTbl[ISO_LEVEL_MAX]; /** DnYuv curve table for adaptive 2d de-noise. */
} ST_CTL_DNYUV_SETTING, *PST_CTL_DNYUV_SETTING;

typedef struct _ST_CTL_DNYUV_PARAM {
    ST_CTL_DNYUV_SETTING stSetting;             /** Control DNYUV setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_U8 u8IsoLevel;                          /** Store the ISO level value. */
    STF_DOUBLE dIsoFactor;                      /** Store the ISO factor value. */
} ST_CTL_DNYUV_PARAM, *PST_CTL_DNYUV_PARAM;

#define ST_CTL_DNYUV_SETTING_LEN        (sizeof(ST_CTL_DNYUV_SETTING))
#define ST_CTL_DNYUV_GET_PARAM_LEN      (ST_CTL_DNYUV_SETTING_LEN)
#define ST_CTL_DNYUV_SET_PARAM_LEN      (ST_CTL_DNYUV_SETTING_LEN)
#define ST_CTL_DNYUV_PARAM_SIZE         (sizeof(ST_CTL_DNYUV_PARAM))
#define ST_CTL_DNYUV_PARAM_LEN          (ST_CTL_DNYUV_PARAM_SIZE)

typedef struct _ST_ISP_CTL_DNYUV {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Control ID. */
    STF_CHAR szName[16];                        /** Control Name. */
    //STF_VOID *pstCtlDnYuvRdma;                  /** DNYUV's RDMA structure pointer. */
    ST_CTL_DNYUV_PARAM *pstCtlDnYuvParam;       /** DNYUV's control parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Control register in pipeline. */
    STF_S32 (*Init)(STF_VOID *pIspCtx);
    STF_S32 (*LoadBinParam)(STF_VOID *pIspCtx, STF_U8 *pu8ParamBuf, STF_U16 u16ParamSize);
    STF_S32 (*Enable)(STF_VOID *pIspCtx, STF_BOOL8 bEnable);
    STF_BOOL8 (*IsEnable)(STF_VOID *pIspCtx);
    STF_S32 (*Registered)(STF_VOID *pIspCtx, STF_BOOL8 bRegistered);
    STF_BOOL8 (*IsRegistered)(STF_VOID *pIspCtx);
    STF_S32 (*Calculate)(STF_VOID *pIspCtx);
    STF_S32 (*FreeResource)(STF_VOID *pIspCtx);
    STF_S32 (*GetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);
    STF_S32 (*SetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 u16ParamSize);
} ST_ISP_CTL_DNYUV, *PST_ISP_CTL_DNYUV;

#pragma pack(pop)


/* 2D denoise DNYUV control parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_DNYUV_SetEnable(
    ST_ISP_CTL_DNYUV *pstCtlDnYuv,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFCTL_ISP_DNYUV_SetIsoInfo(
    ST_ISP_CTL_DNYUV *pstCtlDnYuv,
    STF_U8 u8IsoLevel,
    STF_DOUBLE dIsoFactor
    );
extern
STF_S32 STFCTL_ISP_DNYUV_InterpolateDnYuvCurve(
    ST_ISP_CTL_DNYUV *pstCtlDnYuv,
    STF_U8 u8IsoLevel,
    STF_DOUBLE dIsoFactor,
    ST_DNYUV_CRV *pstDnYuvCurveTable,
    ST_DNYUV_CRV *pstDnYuvCurve
    );
extern
STF_U32 STFCTL_ISP_DNYUV_SetDnYuvCurveTable(
    ST_ISP_CTL_DNYUV *pstCtlDnYuv,
    ST_DNYUV_CRV pstDnYuvCurveTbl[ISO_LEVEL_MAX]
    );
extern
STF_U32 STFCTL_ISP_DNYUV_GetDnYuvCurveTable(
    ST_ISP_CTL_DNYUV *pstCtlDnYuv,
    ST_DNYUV_CRV *ppstDnYuvCurveTbl[ISO_LEVEL_MAX]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_DNYUV_Init(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_DNYUV_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFCTL_ISP_DNYUV_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFCTL_ISP_DNYUV_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_DNYUV_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFCTL_ISP_DNYUV_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_DNYUV_Calculate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_DNYUV_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_DNYUV_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFCTL_ISP_DNYUV_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_DNYUV_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFCTL_ISP_DNYUV_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFCTL_ISP_DNYUV_H__
