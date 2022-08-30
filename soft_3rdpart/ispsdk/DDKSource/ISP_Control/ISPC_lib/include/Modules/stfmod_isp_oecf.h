/**
  ******************************************************************************
  * @file  stfmod_isp_oecf.h
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


#ifndef __STFMOD_ISP_OECF_H__
#define __STFMOD_ISP_OECF_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_oecf_rdma.h"
#include "stflib_isp_pipeline.h"


/* oecf module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define OECF_INPUT_BITS                         (10)
#define OECF_INPUT_VALUE_MAX                    ((1 << OECF_INPUT_BITS) - 1)
#define OECF_OUTPUT_BITS                        (10)
#define OECF_OUTPUT_VALUE_MAX                   ((1 << OECF_OUTPUT_BITS) - 1)
#define OECF_SLOPE_SIGNED_BITS                  (0)
#define OECF_SLOPE_INTEGER_BITS                 (3)
#define OECF_SLOPE_FRACTION_BITS                (7)
#define OECF_SLOPE_MAX_VALUE                    ((1 << (OECF_SLOPE_SIGNED_BITS + OECF_SLOPE_INTEGER_BITS + OECF_SLOPE_FRACTION_BITS)) - 1)
#define OECF_CURVE_POINT_MAX                    (16)

#define OECF_CURVE_USE_POINT_STRUCTURE
#define OECF_SLOPE_AUTO_CALCULATE


#pragma pack(push, 1)

typedef enum _EN_OECF_UPDATE {
    EN_OECF_UPDATE_NONE = 0x00,
    EN_OECF_UPDATE_INPUT = 0x01,
    EN_OECF_UPDATE_OUTPUT = 0x02,
#if !defined(OECF_SLOPE_AUTO_CALCULATE)
    EN_OECF_UPDATE_SLOPE = 0x04,
    EN_OECF_UPDATE_ALL = (EN_OECF_UPDATE_INPUT
                        | EN_OECF_UPDATE_OUTPUT
                        | EN_OECF_UPDATE_SLOPE
                        ),
#else
    EN_OECF_UPDATE_ALL = (EN_OECF_UPDATE_INPUT
                        | EN_OECF_UPDATE_OUTPUT
                        ),
#endif //OECF_SLOPE_AUTO_CALCULATE
} EN_OECF_UPDATE, *PEN_OECF_UPDATE;

typedef enum _EN_OECF_CHN {
    EN_OECF_CHN_R = 0,
    EN_OECF_CHN_GR,
    EN_OECF_CHN_GB,
    EN_OECF_CHN_B,
    EN_OECF_CHN_MAX,
} EN_OECF_CHN, *PEN_OECF_CHN;


#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
typedef struct _ST_OECF_CRV {
    ST_POINT stCurve[OECF_CURVE_POINT_MAX];
} ST_OECF_CRV, *PST_OECF_CRV;
#else //#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
typedef struct _ST_OECF_INPUT {
    STF_U16 u16X[OECF_CURVE_POINT_MAX];
} ST_OECF_INPUT, *PST_OECF_INPUT;

typedef struct _ST_OECF_OUTPUT {
    STF_U16 u16Y[OECF_CURVE_POINT_MAX];
} ST_OECF_OUTPUT, *PST_OECF_OUTPUT;
#endif //OECF_CURVE_USE_POINT_STRUCTURE

#if !defined(OECF_SLOPE_AUTO_CALCULATE)
typedef struct _ST_OECF_SLOPE {
    STF_U16 u16S[OECF_CURVE_POINT_MAX];
} ST_OECF_SLOPE, *PST_OECF_SLOPE;

#endif //OECF_SLOPE_AUTO_CALCULATE
typedef struct _ST_MOD_OECF_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable OECF module. */
#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
    ST_OECF_CRV stOecfCrv[EN_OECF_CHN_MAX];     /** R, Gr, Gb and B channel input. */
#else //#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
    ST_OECF_INPUT stInput[EN_OECF_CHN_MAX];     /** R, Gr, Gb and B channel input. */
    ST_OECF_OUTPUT stOutput[EN_OECF_CHN_MAX];   /** R, Gr, Gb and B channel output. */
#endif //#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
#if !defined(OECF_SLOPE_AUTO_CALCULATE)
    ST_OECF_SLOPE stSlope[EN_OECF_CHN_MAX];     /** R, Gr, Gb and B channel slope. */
#endif //OECF_SLOPE_AUTO_CALCULATE
} ST_MOD_OECF_SETTING, *PST_MOD_OECF_SETTING;

typedef struct _ST_OECF_PARAM {
    ST_MOD_OECF_SETTING stSetting;              /** Module OECF setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program OECF module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_OECF_PARAM, *PST_OECF_PARAM;

#define ST_MOD_OECF_SETTING_LEN         (sizeof(ST_MOD_OECF_SETTING))
#define ST_MOD_OECF_GET_PARAM_LEN       (ST_MOD_OECF_SETTING_LEN)
#define ST_MOD_OECF_SET_PARAM_LEN       (ST_MOD_OECF_SETTING_LEN)
#define ST_OECF_PARAM_SIZE              (sizeof(ST_OECF_PARAM))
#define ST_OECF_PARAM_LEN               (ST_OECF_PARAM_SIZE)

typedef struct _ST_ISP_MOD_OECF {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstOecfRdmaBuf;               /** OECF's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_OECF_RDMA *pstIspOecfRdma;           /** OECF's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_OECF_RDMA *pstOecfRdma;                  /** OECF's RDMA structure pointer.*/
    ST_OECF_PARAM *pstOecfParam;                /** OECF's parameters structure pointer. */
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
} ST_ISP_MOD_OECF, *PST_ISP_MOD_OECF;

#pragma pack(pop)


/* oecf module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECF_InitRdma(
    ST_OECF_RDMA *pstOecfRdma,
    ST_OECF_PARAM *pstOecfParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OECF_UpdateRdma(
    ST_OECF_RDMA *pstOecfRdma,
    ST_OECF_PARAM *pstOecfParam
    );
extern
STF_S32 STFMOD_ISP_OECF_SetNextRdma(
    ST_OECF_RDMA *pstOecfRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECF_SetEnable(
    ST_ISP_MOD_OECF *pstModOecf,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OECF_SetUpdate(
    ST_ISP_MOD_OECF *pstModOecf,
    STF_BOOL8 bUpdate
    );
#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
extern
STF_S32 STFMOD_ISP_OECF_SetAllInput(
    ST_ISP_MOD_OECF *pstModOecf,
    ST_OECF_CRV *pstOecfCrvR,
    ST_OECF_CRV *pstOecfCrvGr,
    ST_OECF_CRV *pstOecfCrvGb,
    ST_OECF_CRV *pstOecfCrvB
    );
extern
STF_S32 STFMOD_ISP_OECF_SetInput(
    ST_ISP_MOD_OECF *pstModOecf,
    EN_OECF_CHN enChn,
    ST_OECF_CRV *pstOecfCrv
    );
extern
STF_S32 STFMOD_ISP_OECF_SetAllOutput(
    ST_ISP_MOD_OECF *pstModOecf,
    ST_OECF_CRV *pstOecfCrvR,
    ST_OECF_CRV *pstOecfCrvGr,
    ST_OECF_CRV *pstOecfCrvGb,
    ST_OECF_CRV *pstOecfCrvB
    );
extern
STF_S32 STFMOD_ISP_OECF_SetOutput(
    ST_ISP_MOD_OECF *pstModOecf,
    EN_OECF_CHN enChn,
    ST_OECF_CRV *pstOecfCrv
    );
#else //#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
extern
STF_S32 STFMOD_ISP_OECF_SetAllInput(
    ST_ISP_MOD_OECF *pstModOecf,
    ST_OECF_INPUT *pstRInput,
    ST_OECF_INPUT *pstGrInput,
    ST_OECF_INPUT *pstGbInput,
    ST_OECF_INPUT *pstBInput
    );
extern
STF_S32 STFMOD_ISP_OECF_SetInput(
    ST_ISP_MOD_OECF *pstModOecf,
    EN_OECF_CHN enChn,
    ST_OECF_INPUT *pstInput
    );
extern
STF_S32 STFMOD_ISP_OECF_SetAllOutput(
    ST_ISP_MOD_OECF *pstModOecf,
    ST_OECF_OUTPUT *pstROutput,
    ST_OECF_OUTPUT *pstGrOutput,
    ST_OECF_OUTPUT *pstGbOutput,
    ST_OECF_OUTPUT *pstBOutput
    );
extern
STF_S32 STFMOD_ISP_OECF_SetOutput(
    ST_ISP_MOD_OECF *pstModOecf,
    EN_OECF_CHN enChn,
    ST_OECF_OUTPUT *pstOutput
    );
#endif //#if defined(OECF_CURVE_USE_POINT_STRUCTURE)
#if !defined(OECF_SLOPE_AUTO_CALCULATE)
extern
STF_S32 STFMOD_ISP_OECF_SetAllSlope(
    ST_ISP_MOD_OECF *pstModOecf,
    ST_OECF_SLOPE *pstRSlope,
    ST_OECF_SLOPE *pstGrSlope,
    ST_OECF_SLOPE *pstGbSlope,
    ST_OECF_SLOPE *pstSlope
    );
extern
STF_S32 STFMOD_ISP_OECF_SetSlope(
    ST_ISP_MOD_OECF *pstModOecf,
    EN_OECF_CHN enChn,
    ST_OECF_SLOPE *pstSlope
    );
#endif //OECF_SLOPE_AUTO_CALCULATE
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECF_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OECF_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OECF_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECF_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_OECF_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECF_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_OECF_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECF_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_OECF_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECF_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECF_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECF_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OECF_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECF_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OECF_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECF_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_OECF_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_OECF_H__
