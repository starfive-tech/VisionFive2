/**
  ******************************************************************************
  * @file  stfmod_isp_sud_isp.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  01/05/2021
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


#ifndef __STFMOD_ISP_SUD_ISP_H__
#define __STFMOD_ISP_SUD_ISP_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_sud_isp_rdma.h"
#include "stflib_isp_pipeline.h"


/** ISP shadow register update parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_SUD_ISP_SHADOW_ENABLE           (STF_TRUE)                  /** Enable ISP shadow registers update. */
#define ISP_DEF_SUD_ISP_SHADOW_MODE             (EN_SUD_MODE_NEXT_VSYNC)    /** Setup the ISP shadow mode as next v-sync (frame start). */


#pragma pack(push, 1)

typedef enum _EN_SUD_ISP_UPDATE {
    EN_SUD_ISP_UPDATE_NONE = 0X0000,
    EN_SUD_ISP_UPDATE_SHADOW_ENABLE = 0X00000001,
    EN_SUD_ISP_UPDATE_SHADOW_MODE = 0X00000002,
    EN_SUD_ISP_UPDATE_ALL = ( EN_SUD_ISP_UPDATE_SHADOW_ENABLE
                            | EN_SUD_ISP_UPDATE_SHADOW_MODE
                            ),
} EN_SUD_ISP_UPDATE, *PEN_SUD_ISP_UPDATE;


typedef struct _ST_SUD_ISP_CTRL {
    STF_BOOL8 bEnable;                          /** Shadow register update enable flag, Write 1 to trigger immediate shadow load when UDM=0, Write 1 to arm shadow load at frame start when UDM=1. */
    STF_U8 u8ShadowMode;                        /** Shadow mode, 0: shadow load immediately on write 1 to UD, 1: shadow load at frame start, must write 1 to UD to arm the shadow load. */
} ST_SUD_ISP_CTRL, *PST_SUD_ISP_CTRL;

typedef struct _ST_MOD_SUD_ISP_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bShadowUpdateEnable;              /** ISP modules shadow register update enable flag, Write 1 to trigger immediate shadow load when UDM=0, Write 1 to arm shadow load at frame start when UDM=1. */
    STF_U8 u8ShadowMode;                        /** ISP modules shadow mode, 0: shadow load immediately on write 1 to UD, 1: shadow load at frame start, must write 1 to UD to arm the shadow load. */
} ST_MOD_SUD_ISP_SETTING, *PST_MOD_SUD_ISP_SETTING;

typedef struct _ST_SUD_ISP_PARAM {
    ST_MOD_SUD_ISP_SETTING stSetting;           /** Module SUD_ISP setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program SUD_ISP module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_SUD_ISP_PARAM, *PST_SUD_ISP_PARAM;

#define ST_MOD_SUD_ISP_SETTING_LEN      (sizeof(ST_MOD_SUD_ISP_SETTING))
#define ST_MOD_SUD_ISP_GET_PARAM_LEN    (ST_MOD_SUD_ISP_SETTING_LEN)
#define ST_MOD_SUD_ISP_SET_PARAM_LEN    (ST_MOD_SUD_ISP_SETTING_LEN)
#define ST_SUD_ISP_PARAM_SIZE           (sizeof(ST_SUD_ISP_PARAM))
#define ST_SUD_ISP_PARAM_LEN            (ST_SUD_ISP_PARAM_SIZE)

typedef struct _ST_ISP_MOD_SUD_ISP_OFF {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstSudIspOffRdmaBuf;          /** SUD_ISP_OFF's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_SUD_ISP_RDMA *pstIspSudIspOffRdma;   /** SUD_ISP_OFF's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_SUD_ISP_RDMA *pstSudIspOffRdma;          /** SUD_ISP_OFF's RDMA structure pointer.*/
    ST_SUD_ISP_PARAM *pstSudIspOffParam;        /** SUD_ISP_OFF's parameters structure pointer. */
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
} ST_ISP_MOD_SUD_ISP_OFF, *PST_ISP_MOD_SUD_ISP_OFF;

typedef struct _ST_ISP_MOD_SUD_ISP {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstSudIspRdmaBuf;             /** SUD_ISP's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_SUD_ISP_RDMA *pstIspSudIspRdma;      /** SUD_ISP's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_SUD_ISP_RDMA *pstSudIspRdma;             /** SUD_ISP's RDMA structure pointer.*/
    ST_SUD_ISP_PARAM *pstSudIspParam;           /** SUD_ISP's parameters structure pointer. */
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
} ST_ISP_MOD_SUD_ISP, *PST_ISP_MOD_SUD_ISP;

#pragma pack(pop)


/* ISP shadow register update parameters interface */
//-----------------------------------------------------------------------------
/**
 * @brief Initial all of modules shadow update off, this RDMA has to put on
 *        RDMA list head to turn off all of modules parameter update
 *        before any modules parameter change.
 * @param pstSudIspOffRdma this structure collected all of modules shadow update control.
 * @param pstSudIspOffParam this structure collected all of modules shadow update parameter.
 * @param pNextRdma point to next RDMA list item.
 */
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_InitRdma(
    ST_SUD_ISP_RDMA *pstSudIspOffRdma,
    ST_SUD_ISP_PARAM *pstSudIspOffParam,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
/**
 * @brief Initial all of modules shadow update status, this RDMA has to put on
 *        RDMA list tail to turn on active modules parameter update
 *        after active modules parameter change completed.
 * @param pstSudIspRdma this structure collected all of modules shadow update control.
 * @param pstSudIspParam this structure collected all of modules shadow update parameter.
 * @param pNextRdma point to next RDMA list item, in this RDMA, the pNextRdma
 *        should be point to NULL to terminate the RDMA function.
 */
extern
STF_S32 STFMOD_ISP_SUD_ISP_InitRdma(
    ST_SUD_ISP_RDMA *pstSudIspRdma,
    ST_SUD_ISP_PARAM *pstSudIspParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_UpdateRdma(
    ST_SUD_ISP_RDMA *pstSudIspRdma,
    ST_SUD_ISP_PARAM *pstSudIspParam
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_UpdateShadowEnable(
    ST_SUD_ISP_RDMA *pstSudIspRdma,
    ST_SUD_ISP_PARAM *pstSudIspParam
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_UpdateShadowMode(
    ST_SUD_ISP_RDMA *pstSudIspRdma,
    ST_SUD_ISP_PARAM *pstSudIspParam
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetNextRdma(
    ST_SUD_ISP_RDMA *pstSudIspRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_SetEnable(
    ST_ISP_MOD_SUD_ISP_OFF *pstModSudIspOff,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_SetShadowMode(
    ST_ISP_MOD_SUD_ISP_OFF *pstModSudIspOff,
    STF_U8 u8ShadowMode
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_SetShadow(
    ST_ISP_MOD_SUD_ISP_OFF *pstModSudIspOff,
    ST_SUD_ISP_CTRL *pstSudIspCtrl
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_SetUpdate(
    ST_ISP_MOD_SUD_ISP_OFF *pstModSudIspOff,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetEnable(
    ST_ISP_MOD_SUD_ISP *pstModSudIsp,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetShadowMode(
    ST_ISP_MOD_SUD_ISP *pstModSudIsp,
    STF_U8 u8ShadowMode
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetShadow(
    ST_ISP_MOD_SUD_ISP *pstModSudIsp,
    ST_SUD_ISP_CTRL *pstSudIspCtrl
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetUpdate(
    ST_ISP_MOD_SUD_ISP *pstModSudIsp,
    STF_BOOL8 bUpdate
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_SUD_ISP_OFF_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_SUD_ISP_OFF_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_SUD_ISP_OFF_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_OFF_Destroy(
    STF_VOID *pPipeline
    );
//=============================================================================
extern
STF_S32 STFMOD_ISP_SUD_ISP_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_SUD_ISP_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_SUD_ISP_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_SUD_ISP_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SUD_ISP_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_SUD_ISP_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_SUD_ISP_H__
