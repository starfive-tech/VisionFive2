/**
  ******************************************************************************
  * @file  stfmod_isp_ycrv.h
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


#ifndef __STFMOD_ISP_YCRV_H__
#define __STFMOD_ISP_YCRV_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_ycrv_rdma.h"
#include "stflib_isp_pipeline.h"


/* y curve module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define Y_CURVE_SIGNED_BITS                     (0)
#define Y_CURVE_INTEGER_BITS                    (10)
#define Y_CURVE_FRACTION_BITS                   (0)

#define YCRV_MAX_PNT                            (YCRV_PNT + 1)


#pragma pack(push, 1)

typedef enum _EN_YCRV_UPDATE {
    EN_YCRV_UPDATE_NONE = 0x00,
    EN_YCRV_UPDATE_CURVE = 0x01,
    EN_YCRV_UPDATE_ALL = (EN_YCRV_UPDATE_CURVE),
} EN_YCRV_UPDATE, *PEN_YCRV_UPDATE;


typedef struct _ST_MOD_YCRV_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable YCRV module. */
    ST_POINT stYCurve[YCRV_MAX_PNT];            /** Y curve value. */
} ST_MOD_YCRV_SETTING, *PST_MOD_YCRV_SETTING;

typedef struct _ST_YCRV_PARAM {
    ST_MOD_YCRV_SETTING stSetting;              /** Module YCRV setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program YCRV module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_YCRV_PARAM, *PST_YCRV_PARAM;

#define ST_MOD_YCRV_SETTING_LEN         (sizeof(ST_MOD_YCRV_SETTING))
#define ST_MOD_YCRV_GET_PARAM_LEN       (ST_MOD_YCRV_SETTING_LEN)
#define ST_MOD_YCRV_SET_PARAM_LEN       (ST_MOD_YCRV_SETTING_LEN)
#define ST_YCRV_PARAM_SIZE              (sizeof(ST_YCRV_PARAM))
#define ST_YCRV_PARAM_LEN               (ST_YCRV_PARAM_SIZE)

typedef struct _ST_ISP_MOD_YCRV {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstYCrvRdmaBuf;               /** YCRV's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_YCRV_RDMA *pstIspYCrvRdma;           /** YCRV's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_YCRV_RDMA *pstYCrvRdma;                  /** YCRV's RDMA structure pointer.*/
    ST_YCRV_PARAM *pstYCrvParam;                /** YCRV's parameters structure pointer. */
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
} ST_ISP_MOD_YCRV, *PST_ISP_MOD_YCRV;

#pragma pack(pop)


/* y curve module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YCRV_InitRdma(
    ST_YCRV_RDMA *pstYCrvRdma,
    ST_YCRV_PARAM *pstYCrvParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_YCRV_UpdateRdma(
    ST_YCRV_RDMA *pstYCrvRdma,
    ST_YCRV_PARAM *pstYCrvParam
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetNextRdma(
    ST_YCRV_RDMA *pstYCrvRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YCRV_SetEnable(
    ST_ISP_MOD_YCRV *pstModYCrv,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetUpdate(
    ST_ISP_MOD_YCRV *pstModYCrv,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetCurve(
    ST_ISP_MOD_YCRV *pstModYCrv,
    ST_POINT stCurve[YCRV_MAX_PNT]
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetCurveInput(
    ST_ISP_MOD_YCRV *pstModYCrv,
    STF_U16 u16Input[YCRV_MAX_PNT]
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetCurveOutput(
    ST_ISP_MOD_YCRV *pstModYCrv,
    ST_POINT stCurve[YCRV_MAX_PNT]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YCRV_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_YCRV_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_YCRV_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YCRV_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_YCRV_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YCRV_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_YCRV_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YCRV_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_YCRV_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YCRV_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_YCRV_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_YCRV_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_YCRV_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_YCRV_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_YCRV_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_YCRV_H__
