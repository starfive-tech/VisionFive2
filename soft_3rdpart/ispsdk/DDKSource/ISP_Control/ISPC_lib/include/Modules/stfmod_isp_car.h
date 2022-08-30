/**
  ******************************************************************************
  * @file  stfmod_isp_car.h
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


#ifndef __STFMOD_ISP_CAR_H__
#define __STFMOD_ISP_CAR_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_car_rdma.h"
#include "stflib_isp_pipeline.h"


/* color artifact removal module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)

typedef enum _EN_CAR_UPDATE {
    EN_CAR_UPDATE_NONE = 0x00,
    EN_CAR_UPDATE_PARAM = 0x01,
    EN_CAR_UPDATE_ALL = (EN_CAR_UPDATE_PARAM),
} EN_CAR_UPDATE, *PEN_CAR_UPDATE;


typedef struct _ST_MOD_CAR_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CAR module. */
} ST_MOD_CAR_SETTING, *PST_MOD_CAR_SETTING;

typedef struct _ST_CAR_PARAM {
    ST_MOD_CAR_SETTING stSetting;               /** Module CAR setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program CAR module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_CAR_PARAM, *PST_CAR_PARAM;

#define ST_MOD_CAR_SETTING_LEN          (sizeof(ST_MOD_CAR_SETTING))
#define ST_MOD_CAR_GET_PARAM_LEN        (ST_MOD_CAR_SETTING_LEN)
#define ST_MOD_CAR_SET_PARAM_LEN        (ST_MOD_CAR_SETTING_LEN)
#define ST_CAR_PARAM_SIZE               (sizeof(ST_CAR_PARAM))
#define ST_CAR_PARAM_LEN                (ST_CAR_PARAM_SIZE)

typedef struct _ST_ISP_MOD_CAR {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstCarRdmaBuf;                /** CAR's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_CAR_RDMA *pstIspCarRdma;             /** CAR's RDMA structure pointer that include the ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_CAR_RDMA *pstCarRdma;                    /** CAR's RDMA structure pointer.*/
    ST_CAR_PARAM *pstCarParam;                  /** CAR's parameters structure pointer. */
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
} ST_ISP_MOD_CAR, *PST_ISP_MOD_CAR;

#pragma pack(pop)


/* color artifact removal module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CAR_InitRdma(
    ST_CAR_RDMA *pstCarRdma,
    ST_CAR_PARAM *pstCarParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CAR_UpdateRdma(
    ST_CAR_RDMA *pstCarRdma,
    ST_CAR_PARAM *pstCarParam
    );
extern
STF_S32 STFMOD_ISP_CAR_SetNextRdma(
    ST_CAR_RDMA *pstCarRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CAR_SetEnable(
    ST_ISP_MOD_CAR *pstModCar,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CAR_SetUpdate(
    ST_ISP_MOD_CAR *pstModCar,
    STF_BOOL8 bUpdate
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CAR_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CAR_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CAR_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CAR_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_CAR_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CAR_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_CAR_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CAR_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_CAR_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CAR_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CAR_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CAR_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CAR_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CAR_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CAR_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CAR_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_CAR_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_CAR_H__
