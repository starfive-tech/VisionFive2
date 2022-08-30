/**
  ******************************************************************************
  * @file  stfmod_isp_til_1_rd.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  05/11/2021
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


#ifndef __STFMOD_ISP_TIL_1_RD_H__
#define __STFMOD_ISP_TIL_1_RD_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "stf_common/ci_alloc_info.h"
#include "registers/stf_isp_til_rdma.h"
#include "Modules/stfmod_isp_til.h"
#include "stflib_isp_pipeline.h"


/* ISP tiling 1 read module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)

typedef struct _ST_MOD_TIL_1_RD_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable the read function of the TIL interface from DRAM buffer. */
} ST_MOD_TIL_1_RD_SETTING, *PST_MOD_TIL_1_RD_SETTING;

typedef struct _ST_TIL_1_RD_PARAM {
    ST_MOD_TIL_1_RD_SETTING stSetting;          /** Module TIL_1_RD setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program TIL module register. */
} ST_TIL_1_RD_PARAM, *PST_TIL_1_RD_PARAM;

#define ST_MOD_TIL_1_RD_SETTING_LEN     (sizeof(ST_MOD_TIL_1_RD_SETTING))
#define ST_MOD_TIL_1_RD_GET_PARAM_LEN   (ST_MOD_TIL_1_RD_SETTING_LEN)
#define ST_MOD_TIL_1_RD_SET_PARAM_LEN   (ST_MOD_TIL_1_RD_SETTING_LEN)
#define ST_TIL_1_RD_PARAM_SIZE          (sizeof(ST_TIL_1_RD_PARAM))
#define ST_TIL_1_RD_PARAM_LEN           (ST_TIL_1_RD_PARAM_SIZE)

typedef struct _ST_ISP_MOD_TIL_1_RD {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstTilRdmaBuf;                /** TIL's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_TIL_RDMA *pstIspTilRdma;             /** TIL's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_TIL_RDMA *pstTilRdma;                    /** TIL's RDMA structure pointer.*/
    ST_TIL_1_RD_PARAM *pstTil_1_RdParam;        /** TIL_1_RD's parameters structure pointer. */
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
} ST_ISP_MOD_TIL_1_RD, *PST_ISP_MOD_TIL_1_RD;

#pragma pack(pop)


/* ISP tiling 1 read module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetEnable(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetUpdate(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilRead(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    ST_TIL_RD *pstTilRead
    );
#if 0
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadSize(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    ST_SIZE stSize
    );
#endif
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadLineStride(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_U32 u32LineStride
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadFormat(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    ST_TIL_RD_FMT *pstFormat,
    eMOSAIC enMosaic
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadDataPathSel(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_U8 u8DataPathSel
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadIndependentRdRvi(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_BOOL8 bIndependentRdRvi
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadNv21Format(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_BOOL8 bRdNv21Format
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadAxiId(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    ST_TIL_AXI_ID *pstAxiId
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadBuffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvYBuf,
    STF_VOID *pvUvBuf
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetTilReadBufferInfo(
    STF_VOID *pIspCtx,
    ST_TILING_BUF_INFO *pstTilingBufInfo
    );
extern
ePxlFormat STFMOD_ISP_TIL_1_RD_GetTilReadPixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_TILING_BUF_INFO STFMOD_ISP_TIL_1_RD_GetTilReadBufferInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_GetTilReadBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvYBuf,
    STF_VOID **ppvUvBuf
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetImageSize(
    STF_VOID *pIspCtx,
    ST_SIZE stSize
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetRaw10Mode(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
#if 0
    EN_TIL_RD_RAW_10_MODE enRaw10Mode
#else
    STF_U8 u8Raw10Mode
#endif
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetLoopback(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_BOOL8 bLoopback
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetMoveDramData(
    ST_ISP_MOD_TIL_1_RD *pstModTil_1_Rd,
    STF_BOOL8 bMoveDramData
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_RD_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_RD_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_RD_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_RD_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_RD_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_TIL_1_RD_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_TIL_1_RD_H__
