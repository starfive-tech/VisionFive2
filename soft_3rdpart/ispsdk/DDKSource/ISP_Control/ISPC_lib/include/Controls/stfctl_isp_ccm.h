/**
  ******************************************************************************
  * @file  stfctl_isp_ccm.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  11/25/2020
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


#ifndef __STFCTL_ISP_CCM_H__
#define __STFCTL_ISP_CCM_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "Modules/stfmod_isp_ccm.h"


/** color correction matrix control parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)

typedef struct _ST_TEMP_CCM {
#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    STF_DOUBLE dSMMinMatrix[3][3];              /** Min color matrix. */
    STF_S16 s16SMMinOffset[3];                  /** Min color offset. */
#endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    STF_DOUBLE dSMLowMatrix[3][3];              /** Low color matrix. */
    STF_S16 s16SMLowOffset[3];                  /** Low color offset. */
} ST_TEMP_CCM, *PST_TEMP_CCM;

typedef struct _ST_CCM_TBL {
    STF_U8 u8CcmTblCnt;                         /** Indicate CCM table counter. */
    ST_TEMP_CCM stTempCcm[TEMP_LEVEL_MAX];      /** Light temperature CCM matrix. */
} ST_CCM_TBL, *PST_CCM_TBL;

typedef struct _ST_CTL_CCM_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CCM control. */
    ST_CCM_TBL stCcmTbl;                        /** CCM light temperature table. */
} ST_CTL_CCM_SETTING, *PST_CTL_CCM_SETTING;

typedef struct _ST_CTL_CCM_PARAM {
    ST_CTL_CCM_SETTING stSetting;               /** Control CCM setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
} ST_CTL_CCM_PARAM, *PST_CTL_CCM_PARAM;

#define ST_CTL_CCM_SETTING_LEN          (sizeof(ST_CTL_CCM_SETTING))
#define ST_CTL_CCM_GET_PARAM_LEN        (ST_CTL_CCM_SETTING_LEN)
#define ST_CTL_CCM_SET_PARAM_LEN        (ST_CTL_CCM_SETTING_LEN)
#define ST_CTL_CCM_PARAM_SIZE           (sizeof(ST_CTL_CCM_PARAM))
#define ST_CTL_CCM_PARAM_LEN            (ST_CTL_CCM_PARAM_SIZE)

typedef struct _ST_ISP_CTL_CCM {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Control ID. */
    STF_CHAR szName[16];                        /** Control Name. */
    //STF_VOID *pstCtlCcmRdma;                    /** CCM's control RDMA structure pointer. */
    ST_CTL_CCM_PARAM *pstCtlCcmParam;           /** CCM's control parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Control register in pipeline. */
    STF_S32 (*Init)(STF_VOID *pIspCtx);
    STF_S32 (*LoadBinParam)(STF_VOID *pIspCtx, STF_U8 *pu8ParamBuf, STF_U16 u16ParamSize);
    STF_S32 (*Enable)(STF_VOID *pIspCtx, STF_BOOL8 bEnable);
    STF_BOOL8 (*IsEnable)(STF_VOID *pIspCtx);
    STF_S32 (*Registered)(STF_VOID *pIspCtx, STF_BOOL8 bRegister);
    STF_BOOL8 (*IsRegistered)(STF_VOID *pIspCtx);
    STF_S32 (*Calculate)(STF_VOID *pIspCtx);
    STF_S32 (*FreeResource)(STF_VOID *pIspCtx);
    STF_S32 (*GetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);
    STF_S32 (*SetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 u16ParamSize);
} ST_ISP_CTL_CCM, *PST_ISP_CTL_CCM;

#pragma pack(pop)


/* color correction matrix control parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_CCM_SetEnable(
    ST_ISP_CTL_CCM *pstCtlCcm,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFCTL_ISP_CCM_InterpolateCcm(
    ST_ISP_CTL_CCM *pstCtlCcm,
    ST_TEMP_INFO *pstTemperatureInfo,
    ST_CCM_TBL *pstCcmTbl,
#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    STF_DOUBLE dSMMinMatrix[][3],
    STF_S16 s16SMMinOffset[],
#endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    STF_DOUBLE dSMLowMatrix[][3],
    STF_S16 s16SMLowOffset[]
    );
extern
STF_S32 STFCTL_ISP_CCM_ResetCcmTblCnt(
    ST_ISP_CTL_CCM *pstCtlCcm
    );
// New temperature > previous temperature has to be guaranteed.
extern
STF_S32 STFCTL_ISP_CCM_AddCcmTbl(
    ST_ISP_CTL_CCM *pstCtlCcm,
    ST_TEMP_CCM *pstTempCmm
    );
extern
STF_S32 STFCTL_ISP_CCM_GetCcmTblCount(
    STF_VOID *pIspCtx,
    STF_U8 *pu8CcmTblCnt
    );
extern
STF_S32 STFCTL_ISP_CCM_GetCcmTbl(
    STF_VOID *pIspCtx,
    ST_CCM_TBL **pstCcmTbl
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_CCM_Init(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_CCM_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFCTL_ISP_CCM_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFCTL_ISP_CCM_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_CCM_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegister
    );
extern
STF_BOOL8 STFCTL_ISP_CCM_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_CCM_Calculate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_CCM_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_CCM_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFCTL_ISP_CCM_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_CCM_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFCTL_ISP_CCM_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFCTL_ISP_CCM_H__
