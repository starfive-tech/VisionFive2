/**
  ******************************************************************************
  * @file  stfctl_isp_lccf.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  01/03/2021
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


#ifndef __STFCTL_ISP_LCCF_H__
#define __STFCTL_ISP_LCCF_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "Modules/stfmod_isp_lccf.h"


/* lens correction cos4 control parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)

typedef struct _ST_CTL_LCCF_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable Y curve control. */
    ST_LCCF_FACTOR stFactorTbl[ISO_LEVEL_MAX];  /** Four channel factor parameters table for ISO level control. */
} ST_CTL_LCCF_SETTING, *PST_CTL_LCCF_SETTING;

typedef struct _ST_CTL_LCCF_PARAM {
    ST_CTL_LCCF_SETTING stSetting;              /** Control LCCF setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_U8 u8IsoLevel;                          /** Store the ISO level value. */
    STF_DOUBLE dIsoFactor;                      /** Store the ISO factor value. */
} ST_CTL_LCCF_PARAM, *PST_CTL_LCCF_PARAM;

#define ST_CTL_LCCF_SETTING_LEN          (sizeof(ST_CTL_LCCF_SETTING))
#define ST_CTL_LCCF_GET_PARAM_LEN        (ST_CTL_LCCF_SETTING_LEN)
#define ST_CTL_LCCF_SET_PARAM_LEN        (ST_CTL_LCCF_SETTING_LEN)
#define ST_CTL_LCCF_PARAM_SIZE          (sizeof(ST_CTL_LCCF_PARAM))
#define ST_CTL_LCCF_PARAM_LEN           (ST_CTL_LCCF_PARAM_SIZE)

typedef struct _ST_ISP_CTL_LCCF {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Control ID. */
    STF_CHAR szName[16];                        /** Control Name. */
    //STF_VOID *pstCtlLccfRdma;                   /** LCCF's RDMA structure pointer. */
    ST_CTL_LCCF_PARAM *pstCtlLccfParam;         /** LCCF's control parameters structure pointer. */
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
} ST_ISP_CTL_LCCF, *PST_ISP_CTL_LCCF;

#pragma pack(pop)


/* lens correction cos4 control parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_LCCF_SetEnable(
    ST_ISP_CTL_LCCF *pstCtlLccf,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFCTL_ISP_LCCF_SetIsoInfo(
    ST_ISP_CTL_LCCF *pstCtlLccf,
    STF_U8 u8IsoLevel,
    STF_DOUBLE dIsoFactor
    );
extern
STF_S32 STFCTL_ISP_LCCF_InterpolateFactor(
    ST_ISP_CTL_LCCF *pstCtlLccf,
    STF_U8 u8IsoLevel,
    STF_DOUBLE dIsoFactor,
    ST_LCCF_FACTOR *pstLccfFactorTable,
    ST_LCCF_FACTOR *pstLccfFactor
    );
extern
STF_S32 STFCTL_ISP_LCCF_SetFactorTable(
    ST_ISP_CTL_LCCF *pstCtlLccf,
    ST_LCCF_FACTOR pstFactorTbl[ISO_LEVEL_MAX]
    );
extern
STF_S32 STFCTL_ISP_LCCF_GetFactorTable(
    ST_ISP_CTL_LCCF *pstCtlLccf,
    ST_LCCF_FACTOR *ppstFactorTbl[ISO_LEVEL_MAX]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_LCCF_Init(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_LCCF_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFCTL_ISP_LCCF_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFCTL_ISP_LCCF_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_LCCF_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFCTL_ISP_LCCF_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_LCCF_Calculate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_LCCF_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_LCCF_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFCTL_ISP_LCCF_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_LCCF_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFCTL_ISP_LCCF_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFCTL_ISP_LCCF_H__
