/**
  ******************************************************************************
  * @file  stfmod_isp_out_uo.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  01/27/2021
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


#ifndef __STFMOD_ISP_OUT_UO_H__
#define __STFMOD_ISP_OUT_UO_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "stf_common/ci_alloc_info.h"
#include "registers/stf_isp_out_rdma.h"
#include "Modules/stfmod_isp_out.h"
#include "stflib_isp_pipeline.h"


/* ISP video output UO module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)

typedef struct _ST_MOD_OUT_UO_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable the unscaled output module. */
} ST_MOD_OUT_UO_SETTING, *PST_MOD_OUT_UO_SETTING;

typedef struct _ST_OUT_UO_PARAM {
    ST_MOD_OUT_UO_SETTING stSetting;            /** Module OUT_UO setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program OUT module register. */
} ST_OUT_UO_PARAM, *PST_OUT_UO_PARAM;

#define ST_MOD_OUT_UO_SETTING_LEN       (sizeof(ST_MOD_OUT_UO_SETTING))
#define ST_MOD_OUT_UO_GET_PARAM_LEN     (ST_MOD_OUT_UO_SETTING_LEN)
#define ST_MOD_OUT_UO_SET_PARAM_LEN     (ST_MOD_OUT_UO_SETTING_LEN)
#define ST_OUT_UO_PARAM_SIZE            (sizeof(ST_OUT_UO_PARAM))
#define ST_OUT_UO_PARAM_LEN             (ST_OUT_UO_PARAM_SIZE)

typedef struct _ST_ISP_MOD_OUT_UO {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstOutRdmaBuf;                /** OUT's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_OUT_RDMA *pstIspOutRdma;             /** OUT's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_OUT_RDMA *pstOutRdma;                    /** OUT's RDMA structure pointer.*/
    ST_OUT_UO_PARAM *pstOutUoParam;             /** OUT_UO's parameters structure pointer. */
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
} ST_ISP_MOD_OUT_UO, *PST_ISP_MOD_OUT_UO;

#pragma pack(pop)


/* ISP video output UO module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_UO_SetEnable(
    ST_ISP_MOD_OUT_UO *pstModOutUo,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetUpdate(
    ST_ISP_MOD_OUT_UO *pstModOutUo,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetInfo(
    ST_ISP_MOD_OUT_UO *pstModOutUo,
    ST_UO_INFO *pstUoInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetBufInfo(
    STF_VOID *pIspCtx,
    ST_OUT_BUF_INFO *pstBufInfo
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetUvMode(
    ST_ISP_MOD_OUT_UO *pstModOutUo,
    STF_BOOL8 bIsNV21
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetAxiId(
    ST_ISP_MOD_OUT_UO *pstModOutUo,
    STF_U8 u8AxiId
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetSize(
    ST_ISP_MOD_OUT_UO *pstModOutUo,
    ST_SIZE stSize
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetStride(
    STF_VOID *pIspCtx,
    STF_U16 u16Stride
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetBuffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvBuffer
    );
extern
ePxlFormat STFMOD_ISP_OUT_UO_GetPixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_OUT_BUF_INFO STFMOD_ISP_OUT_UO_GetBufInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_GetBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvBuffer
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_UO_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_UO_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_UO_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_OUT_UO_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OUT_UO_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_OUT_UO_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_OUT_UO_H__
