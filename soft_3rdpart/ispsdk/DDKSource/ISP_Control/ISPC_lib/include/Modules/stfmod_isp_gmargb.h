/**
  ******************************************************************************
  * @file  stfmod_isp_gmargb.h
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


#ifndef __STFMOD_ISP_GMARGB_H__
#define __STFMOD_ISP_GMARGB_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_gmargb_rdma.h"
#include "stflib_isp_pipeline.h"


/** RGB gamma module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#if 1
#define ISP_DEF_GMARGB_GAMMA_CURVE_GEN_BY_FACTOR (STF_FALSE)
#elif 0
#define ISP_DEF_GMARGB_GAMMA_CURVE_GEN_BY_FACTOR (STF_TRUE)
#endif
#if 1
// For PC-Cam.
#define ISP_DEF_GMARGB_FACTOR                   (2.2)
#elif 0
// 20201206_v01
#define ISP_DEF_GMARGB_FACTOR                   (2.0)
#elif 0
// 20210103_v01
#define ISP_DEF_GMARGB_FACTOR                   (1.9)
#endif
#define ISP_DEF_GMARGB_NEXT_STEP                (8)
#define ISP_DEF_GMARGB_MAX_SLOPE                (((1 << 4) - 1) + ((STF_DOUBLE)((1 << 11) - 1) / (1 << 11)))    // 15.99951171875
#if 0
#define ISP_DEF_GMARGB_FORCE_SLOPE              (3.2)
#define ISP_DEF_GMARGB_USING_FORCE_SLOPE_INPUT  (64)
#elif 0
#define ISP_DEF_GMARGB_FORCE_SLOPE              (4.0)
#define ISP_DEF_GMARGB_USING_FORCE_SLOPE_INPUT  (32)
#elif 1
#define ISP_DEF_GMARGB_FORCE_SLOPE              (4.0)
#define ISP_DEF_GMARGB_USING_FORCE_SLOPE_INPUT  (0)
#elif 0
#define ISP_DEF_GMARGB_FORCE_SLOPE              (((1 << 4) - 1) + ((STF_DOUBLE)((1 << 11) - 1) / (1 << 11)))    // 15.99951171875
#define ISP_DEF_GMARGB_USING_FORCE_SLOPE_INPUT  (0)
#endif


#define GMARGB_INPUT_MAX_VALUE 1024.0
#define GMARGB_OUTPUT_MAX_VALUE 1024.0
#define GMARGB_MAX_PNT                          (GMARGB_PNT + 1)
#define GMARGB_SLOPE_CHANGE_POINT               (8)


#pragma pack(push, 1)

typedef enum _EN_GMARGB_UPDATE {
    EN_GMARGB_UPDATE_NONE = 0x00,
    EN_GMARGB_UPDATE_GAMMA_CURVE = 0x01,
    EN_GMARGB_UPDATE_ALL = (EN_GMARGB_UPDATE_GAMMA_CURVE),
} EN_GMARGB_UPDATE, *PEN_GMARGB_UPDATE;


typedef struct _ST_GMA_CRV_SETTING {
    STF_BOOL8 bGammaCurveGenByFactor;           /** Enable this flag will using dFactor to generate gamma curve. */
    STF_DOUBLE dGammaFactor;                    /** Gamma curve factor. This variable works when the bGammaCurveGenByFactor is true. */
    STF_U8 u8NextStep;                          /** Gamma curve next step for interpolation, default value is 8. */
    STF_DOUBLE dMaxSlope;                       /** Gamma curve slope limit, maximum slope should be less than or equal to 15.99951171875, default suggest value is 15.99951171875. */
    STF_DOUBLE dForceSlope;                     /** Gamma curve force slope value, this slope should be less than or equal to 15.99951171875. This variable works when the bGammaCurveGenByFactor is true. */
    STF_U16 u16UsingForceSlopeInput;            /** If the input value is less than or equal to this value will using the force slope value. This variable works when the bGammaCurveGenByFactor is true. */
} ST_GMA_CRV_SETTING, *PST_GMA_CRV_SETTING;

typedef struct _ST_MOD_GMARGB_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable GMARGB module. */
    ST_POINT stCustomGammaCurve[GMARGB_PNT];    /** Customize gamma curve. */
} ST_MOD_GMARGB_SETTING, *PST_MOD_GMARGB_SETTING;

typedef struct _ST_MOD_GMARGB_INFO {
    //-------------------------------------------------------------------------
    ST_POINT stGammaCurve[GMARGB_MAX_PNT];      /** Final gamma curve for customer check only. */
} ST_MOD_GMARGB_INFO, *PST_MOD_GMARGB_INFO;

typedef struct _ST_GMARGB_PARAM {
    ST_MOD_GMARGB_SETTING stSetting;            /** Module GMARGB setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_GMARGB_INFO stInfo;                  /** Module GMARGB output information parameters. */
    ST_GMA_CRV_SETTING stGammaCurveSetting;     /** Gamma curve setting for convert customize curve to hardware register. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program GRGB module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_GMARGB_PARAM, *PST_GMARGB_PARAM;

#define ST_MOD_GMARGB_SETTING_LEN       (sizeof(ST_MOD_GMARGB_SETTING))
#define ST_MOD_GMARGB_INFO_LEN          (sizeof(ST_MOD_GMARGB_INFO))
#if 0
#define ST_MOD_GMARGB_GET_PARAM_LEN     (ST_MOD_GMARGB_SETTING_LEN + ST_MOD_GMARGB_INFO_LEN)
#else
#define ST_MOD_GMARGB_GET_PARAM_LEN     (ST_MOD_GMARGB_SETTING_LEN)
#endif //#if 0
#define ST_MOD_GMARGB_SET_PARAM_LEN     (ST_MOD_GMARGB_SETTING_LEN)
#define ST_GMARGB_PARAM_SIZE            (sizeof(ST_GMARGB_PARAM))
#define ST_GMARGB_PARAM_LEN             (ST_GMARGB_PARAM_SIZE)

typedef struct _ST_ISP_MOD_GMARGB {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstGmaRgbRdmaBuf;             /** GMARGB's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_GMARGB_RDMA *pstIspGmaRgbRdma;       /** GMARGB's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_GMARGB_RDMA *pstGmaRgbRdma;              /** GMARGB's RDMA structure pointer.*/
    ST_GMARGB_PARAM *pstGmaRgbParam;            /** GMARGB's parameters structure pointer. */
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
} ST_ISP_MOD_GMARGB, *PST_ISP_MOD_GMARGB;

#pragma pack(pop)


/* RGB gamma module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_GMARGB_InitRdma(
    ST_GMARGB_RDMA *pstGmaRgbRdma,
    ST_GMARGB_PARAM *pstGmaRgbParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_GMARGB_UpdateRdma(
    ST_GMARGB_RDMA *pstGmaRgbRdma,
    ST_GMARGB_PARAM *pstGmaRgbParam
    );
extern
STF_S32 STFMOD_ISP_GMARGB_UpdateGammaCrvByGammaFactor(
    ST_GMARGB_RDMA *pstGmaRgbRdma,
    ST_GMARGB_PARAM *pstGmaRgbParam
    );
extern
STF_S32 STFMOD_ISP_GMARGB_UpdateGammaCrvByCustomizeGammaCrv(
    ST_GMARGB_RDMA *pstGmaRgbRdma,
    ST_GMARGB_PARAM *pstGmaRgbParam
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetNextRdma(
    ST_GMARGB_RDMA *pstGmaRgbRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_GMARGB_SetEnable(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetUpdate(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetEnableGammaCurveGenByFactor(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_BOOL8 bGammaCurveGenByFactor
    );
extern
STF_S32 STFMOD_ISP_GMARGB_InitGammaCurve(
    ST_ISP_MOD_GMARGB *pstModGmaRgb
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetGammaFactor(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_DOUBLE dGammaFactor
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetNextStep(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_U8 u8NextStep
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetMaxSlope(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_DOUBLE dMaxSlope
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetForceSlope(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_DOUBLE dForceSlope
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetUsingForceSlopeInput(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    STF_U16 u16UsingForceSlopeInput
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetCustomizeGammaCurve(
    ST_ISP_MOD_GMARGB *pstModGmaRgb,
    ST_POINT stGammaCurve[GMARGB_PNT]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_GMARGB_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_GMARGB_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_GMARGB_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_GMARGB_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_GMARGB_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_GMARGB_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_GMARGB_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_GMARGB_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_GMARGB_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_GMARGB_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_GMARGB_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_GMARGB_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_GMARGB_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_GMARGB_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_GMARGB_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_GMARGB_H__
