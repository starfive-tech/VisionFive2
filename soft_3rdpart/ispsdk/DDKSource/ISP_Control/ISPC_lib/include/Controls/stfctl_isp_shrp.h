/**
  ******************************************************************************
  * @file  stfctl_isp_shrp.h
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


#ifndef __STFCTL_ISP_SHRP_H__
#define __STFCTL_ISP_SHRP_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "Modules/stfmod_isp_shrp.h"


/* Sharpening control parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)

typedef struct _ST_CTL_SHRP_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable Y curve control. */
    ST_SHRP_CRV stYLevelCrvTbl[ISO_LEVEL_MAX];  /** Y level curve for adaptive sharpness. */
} ST_CTL_SHRP_SETTING, *PST_CTL_SHRP_SETTING;

typedef struct _ST_CTL_SHRP_PARAM {
    ST_CTL_SHRP_SETTING stSetting;              /** Control SHRP setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_U8 u8IsoLevel;                          /** Store the ISO level value. */
    STF_DOUBLE dIsoFactor;                      /** Store the ISO factor value. */
} ST_CTL_SHRP_PARAM, *PST_CTL_SHRP_PARAM;

#define ST_CTL_SHRP_SETTING_LEN          (sizeof(ST_CTL_SHRP_SETTING))
#define ST_CTL_SHRP_GET_PARAM_LEN        (ST_CTL_SHRP_SETTING_LEN)
#define ST_CTL_SHRP_SET_PARAM_LEN        (ST_CTL_SHRP_SETTING_LEN)
#define ST_CTL_SHRP_PARAM_SIZE          (sizeof(ST_CTL_SHRP_PARAM))
#define ST_CTL_SHRP_PARAM_LEN           (ST_CTL_SAT_PARAM_SIZE)

typedef struct _ST_ISP_CTL_SHRP {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Control ID. */
    STF_CHAR szName[16];                        /** Control Name. */
    //STF_VOID *pstCtlShrpRdma;                   /** SHRP's RDMA structure pointer. */
    ST_CTL_SHRP_PARAM *pstCtlShrpParam;         /** SHRP's control parameters structure pointer. */
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
} ST_ISP_CTL_SHRP, *PST_ISP_CTL_SHRP;

#pragma pack(pop)


/* Sharpening control parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_SHRP_SetEnable(
    ST_ISP_CTL_SHRP *pstCtlShrp,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFCTL_ISP_SHRP_SetIsoInfo(
    ST_ISP_CTL_SHRP *pstCtlShrp,
    STF_U8 u8IsoLevel,
    STF_DOUBLE dIsoFactor
    );
extern
STF_S32 STFCTL_ISP_SHRP_InterpolateYLevel(
    ST_ISP_CTL_SHRP *pstCtlShrp,
    STF_U8 u8IsoLevel,
    STF_DOUBLE dIsoFactor,
    ST_SHRP_CRV *pstShrpCurveTable,
    ST_SHRP_PNT pstYLevel[SHRP_CRV_PNT_MAX]
    );
extern
STF_U32 STFCTL_ISP_SHRP_SetYLevelCurveTable(
    ST_ISP_CTL_SHRP *pstCtlShrp,
    ST_SHRP_CRV pstYLevelCrvTbl[ISO_LEVEL_MAX]
    );
extern
STF_U32 STFCTL_ISP_SHRP_GetYLevelCurveTable(
    ST_ISP_CTL_SHRP *pstCtlShrp,
    ST_SHRP_CRV *ppstYLevelCrvTbl[ISO_LEVEL_MAX]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_SHRP_Init(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_SHRP_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFCTL_ISP_SHRP_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFCTL_ISP_SHRP_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_SHRP_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFCTL_ISP_SHRP_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_SHRP_Calculate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_SHRP_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_SHRP_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFCTL_ISP_SHRP_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_SHRP_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFCTL_ISP_SHRP_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFCTL_ISP_SHRP_H__
