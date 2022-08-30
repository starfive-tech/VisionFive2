/**
  ******************************************************************************
  * @file  stfmod_isp_dbc.h
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


#ifndef __STFMOD_ISP_DBC_H__
#define __STFMOD_ISP_DBC_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_dbc_rdma.h"
#include "stflib_isp_pipeline.h"


/** dynamic bad pixel correction filter module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#if 0
#define ISP_DEF_DBC_BAD_GR_GB_THRESHOLD         (256)
#define ISP_DEF_DBC_BAD_R_B_THRESHOLD           (256)
#elif 1
// 20210103_v01
#define ISP_DEF_DBC_BAD_GR_GB_THRESHOLD         (512)
#define ISP_DEF_DBC_BAD_R_B_THRESHOLD           (512)
#endif


#pragma pack(push, 1)

typedef enum _EN_DBC_UPDATE {
    EN_DBC_UPDATE_NONE = 0x00,
    EN_DBC_UPDATE_BAD_PIXEL_THRESHOLD = 0x01,
    EN_DBC_UPDATE_ALL = (EN_DBC_UPDATE_BAD_PIXEL_THRESHOLD),
} EN_DBC_UPDATE, *PEN_DBC_UPDATE;


typedef struct _ST_MOD_DBC_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable DBC module. */
    STF_U16 u16BadGT;                           /** DBC filter bad pixel threshold for Gr and Gb channel. */
    STF_U16 u16BadXT;                           /** DBC filter bad pixel threshold for R and B channel. */
} ST_MOD_DBC_SETTING, *PST_MOD_DBC_SETTING;

typedef struct _ST_DBC_PARAM {
    ST_MOD_DBC_SETTING stSetting;               /** Module DBC setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program DBC module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_DBC_PARAM, *PST_DBC_PARAM;

#define ST_MOD_DBC_SETTING_LEN          (sizeof(ST_MOD_DBC_SETTING))
#define ST_MOD_DBC_GET_PARAM_LEN        (ST_MOD_DBC_SETTING_LEN)
#define ST_MOD_DBC_SET_PARAM_LEN        (ST_MOD_DBC_SETTING_LEN)
#define ST_DBC_PARAM_SIZE               (sizeof(ST_DBC_PARAM))
#define ST_DBC_PARAM_LEN                (ST_DBC_PARAM_SIZE)

typedef struct _ST_ISP_MOD_DBC {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstDbcRdmaBuf;                /** DBC's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_DBC_RDMA *pstIspDbcRdma;             /** DBC's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_DBC_RDMA *pstDbcRdma;                    /** DBC's RDMA structure pointer.*/
    ST_DBC_PARAM *pstDbcParam;                  /** DBC's parameters structure pointer. */
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
} ST_ISP_MOD_DBC, *PST_ISP_MOD_DBC;

#pragma pack(pop)


/* dynamic bad pixel correction filter module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DBC_InitRdma(
    ST_DBC_RDMA *pstDbcRdma,
    ST_DBC_PARAM *pstDbcParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DBC_UpdateRdma(
    ST_DBC_RDMA *pstDbcRdma,
    ST_DBC_PARAM *pstDbcParam
    );
extern
STF_S32 STFMOD_ISP_DBC_UpdateBadPixelThreshold(
    ST_DBC_RDMA *pstDbcRdma,
    ST_DBC_PARAM *pstDbcParam
    );
extern
STF_S32 STFMOD_ISP_DBC_SetNextRdma(
    ST_DBC_RDMA *pstDbcRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DBC_SetEnable(
    ST_ISP_MOD_DBC *pstModDbc,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_DBC_SetUpdate(
    ST_ISP_MOD_DBC *pstModDbc,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_DBC_SetBadPixelThreshold(
    ST_ISP_MOD_DBC *pstModDbc,
    STF_U16 u16BadGT,
    STF_U16 u16BadXT
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DBC_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DBC_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DBC_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DBC_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_DBC_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DBC_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_DBC_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DBC_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_DBC_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DBC_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DBC_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DBC_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DBC_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DBC_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern STF_S32 STFMOD_ISP_DBC_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DBC_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_DBC_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_DBC_H__
