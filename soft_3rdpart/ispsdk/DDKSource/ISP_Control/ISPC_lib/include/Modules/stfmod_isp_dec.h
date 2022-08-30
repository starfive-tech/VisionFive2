/**
  ******************************************************************************
  * @file  stfmod_isp_dec.h
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


#ifndef __STFMOD_ISP_DEC_H__
#define __STFMOD_ISP_DEC_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_dec_rdma.h"
#include "Modules/stfmod_isp_crop.h"
#include "stflib_isp_pipeline.h"


/* raw decimation module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_DEC_H_PERIOD                    (0)
#define ISP_DEF_DEC_H_KEEP                      (0)
#define ISP_DEF_DEC_V_PERIOD                    (0)
#define ISP_DEF_DEC_V_KEEP                      (0)


#pragma pack(push, 1)

typedef enum _EN_DEC_UPDATE {
    EN_DEC_UPDATE_NONE = 0x00,
    EN_DEC_UPDATE_DECIMATION = 0x01,
    EN_DEC_UPDATE_ALL = (EN_DEC_UPDATE_DECIMATION),
} EN_DEC_UPDATE, *PEN_DEC_UPDATE;


typedef struct _ST_MOD_DEC_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable DEC module. */
    STF_U8 u8HPeriod;                           /** Horizontal period. (zero base) */
    STF_U8 u8HKeep;                             /** Horizontal keep. (zero base) */
    STF_U8 u8VPeriod;                           /** Vertical period. (zero base) */
    STF_U8 u8VKeep;                             /** Vertical keep. (zero base) */
} ST_MOD_DEC_SETTING, *PST_MOD_DEC_SETTING;

typedef struct _ST_MOD_DEC_INFO {
    ST_SIZE stOutputSize;                       /** Output image size of DEC module. */
} ST_MOD_DEC_INFO, *PST_MOD_DEC_INFO;

typedef struct _ST_DEC_PARAM {
    ST_MOD_DEC_SETTING stSetting;               /** Module DEC setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_DEC_INFO stInfo;                     /** Module DEC output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program DEC module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_DEC_PARAM, *PST_DEC_PARAM;

#define ST_MOD_DEC_SETTING_LEN          (sizeof(ST_MOD_DEC_SETTING))
#define ST_MOD_DEC_INFO_LEN             (sizeof(ST_MOD_DEC_INFO))
#define ST_MOD_DEC_GET_PARAM_LEN        (ST_MOD_DEC_SETTING_LEN + ST_MOD_DEC_INFO_LEN)
#define ST_MOD_DEC_SET_PARAM_LEN        (ST_MOD_DEC_SETTING_LEN)
#define ST_DEC_PARAM_SIZE               (sizeof(ST_DEC_PARAM))
#define ST_DEC_PARAM_LEN                (ST_DEC_PARAM_SIZE)

typedef struct _ST_ISP_MOD_DEC {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstDecRdmaBuf;                /** DEC's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_DEC_RDMA *pstIspDecRdma;             /** DEC's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_DEC_RDMA *pstDecRdma;                    /** DEC's RDMA structure pointer.*/
    ST_DEC_PARAM *pstDecParam;                  /** DEC's parameters structure pointer. */
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
} ST_ISP_MOD_DEC, *PST_ISP_MOD_DEC;

#pragma pack(pop)


/* raw decimation module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DEC_InitRdma(
    ST_DEC_RDMA *pstDecRdma,
    ST_DEC_PARAM *pstDecParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DEC_UpdateRdma(
    ST_DEC_RDMA *pstDecRdma,
    ST_DEC_PARAM *pstDecParam
    );
extern
STF_S32 STFMOD_ISP_DEC_SetNextRdma(
    ST_DEC_RDMA *pstDecRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DEC_SetEnable(
    ST_ISP_MOD_DEC *pstModDec,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_DEC_SetUpdate(
    ST_ISP_MOD_DEC *pstModDec,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_DEC_SetDecimation(
    ST_ISP_MOD_DEC *pstModDec,
    STF_U8 u8HPeriod,
    STF_U8 u8HKeep,
    STF_U8 u8VPeriod,
    STF_U8 u8VKeep
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DEC_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DEC_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DEC_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DEC_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_DEC_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DEC_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_DEC_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DEC_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_DEC_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DEC_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DEC_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DEC_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DEC_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DEC_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DEC_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DEC_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_DEC_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_DEC_H__
