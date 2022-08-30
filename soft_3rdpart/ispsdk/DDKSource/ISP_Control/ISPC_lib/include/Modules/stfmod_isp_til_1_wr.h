/**
  ******************************************************************************
  * @file  stfmod_isp_til_1_wr.h
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


#ifndef __STFMOD_ISP_TIL_1_WR_H__
#define __STFMOD_ISP_TIL_1_WR_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "stf_common/ci_alloc_info.h"
#include "registers/stf_isp_til_rdma.h"
#include "Modules/stfmod_isp_til.h"
#include "stflib_isp_pipeline.h"


/* ISP tiling 1 write module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values


#pragma pack(push, 1)

typedef struct _ST_MOD_TIL_1_WR_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable the write function of the TIL interface into DRAM buffer. */
} ST_MOD_TIL_1_WR_SETTING, *PST_MOD_TIL_1_WR_SETTING;

typedef struct _ST_TIL_1_WR_PARAM {
    ST_MOD_TIL_1_WR_SETTING stSetting;          /** Module TIL_1_WR setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program TIL module register. */
} ST_TIL_1_WR_PARAM, *PST_TIL_1_WR_PARAM;

#define ST_MOD_TIL_1_WR_SETTING_LEN     (sizeof(ST_MOD_TIL_1_WR_SETTING))
#define ST_MOD_TIL_1_WR_GET_PARAM_LEN   (ST_MOD_TIL_1_WR_SETTING_LEN)
#define ST_MOD_TIL_1_WR_SET_PARAM_LEN   (ST_MOD_TIL_1_WR_SETTING_LEN)
#define ST_TIL_1_WR_PARAM_SIZE          (sizeof(ST_TIL_1_WR_PARAM))
#define ST_TIL_1_WR_PARAM_LEN           (ST_TIL_1_WR_PARAM_SIZE)

typedef struct _ST_ISP_MOD_TIL_1_WR {
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
    ST_TIL_1_WR_PARAM *pstTil_1_WrParam;        /** TIL_1_WR's parameters structure pointer. */
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
} ST_ISP_MOD_TIL_1_WR, *PST_ISP_MOD_TIL_1_WR;

#pragma pack(pop)


/* ISP tiling 1 write module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetEnable(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetUpdate(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWrite(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    ST_TIL_WRT *pstTilWrite
    );
#if 0
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteSize(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    ST_SIZE stSize
    );
#endif
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteLineStride(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    STF_U32 u32LineStride
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteFormat(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    ST_TIL_WR_FMT *pstFormat,
    eMOSAIC enMosaic
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteDataPathSel(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    STF_U8 u8DataPathSel
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteNv21Format(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    STF_BOOL8 bWrNv21Format
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteAxiId(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    ST_TIL_AXI_ID *pstAxiId
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteBuffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvYBuf,
    STF_VOID *pvUvBuf
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetTilWriteBufferInfo(
    STF_VOID *pIspCtx,
    ST_TILING_BUF_INFO *pstTilingBufInfo
    );
extern
ePxlFormat STFMOD_ISP_TIL_1_WR_GetTilWritePixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_TILING_BUF_INFO STFMOD_ISP_TIL_1_WR_GetTilWriteBufferInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_GetTilWriteBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvYBuf,
    STF_VOID **ppvUvBuf
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetImageSize(
    STF_VOID *pIspCtx,
    ST_SIZE stSize
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetRaw10Mode(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
#if 0
    EN_TIL_WR_RAW_10_MODE enRaw10Mode
#else
    STF_U8 u8Raw10Mode
#endif
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetLoopback(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    STF_BOOL8 bLoopback
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetMoveDramData(
    ST_ISP_MOD_TIL_1_WR *pstModTil_1_Wr,
    STF_BOOL8 bMoveDramData
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_WR_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_WR_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_WR_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_WR_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_WR_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_TIL_1_WR_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_TIL_1_WR_H__
