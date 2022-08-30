/**
  ******************************************************************************
  * @file  stfmod_isp_cfa_ahd.h
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


#ifndef __STFMOD_ISP_CFA_AHD_H__
#define __STFMOD_ISP_CFA_AHD_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_cfa_ahd_rdma.h"
#include "stflib_isp_pipeline.h"


/* demosaic module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#if 0
#define ISP_DEF_CFA_HV_WIDTH                    (4)
#define ISP_DEF_CFA_CROSS_COV                   (3)
#elif 1
// 20201219_v01
#define ISP_DEF_CFA_HV_WIDTH                    (2)
#define ISP_DEF_CFA_CROSS_COV                   (3)
#endif


#pragma pack(push, 1)

typedef enum _EN_CFA_UPDATE {
    EN_CFA_UPDATE_NONE = 0x00,
    EN_CFA_UPDATE_MODE = 0x01,
    EN_CFA_UPDATE_ALL = (EN_CFA_UPDATE_MODE),
} EN_CFA_UPDATE, *PEN_CFA_UPDATE;


typedef struct _ST_MOD_CFA_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CFA module. */
    STF_U8 u8HvWidth;                           /** HV_WIDTH = 2^u8HvWidth, range 0 - 15. */
    STF_U8 u8CrossCov;                          /** Cross covariance weighting, range 0 - 3. */
} ST_MOD_CFA_SETTING, *PST_MOD_CFA_SETTING;

typedef struct _ST_CFA_PARAM {
    ST_MOD_CFA_SETTING stSetting;               /** Module CFA setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program CFA module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_CFA_PARAM, *PST_CFA_PARAM;

#define ST_MOD_CFA_SETTING_LEN          (sizeof(ST_MOD_CFA_SETTING))
#define ST_MOD_CFA_GET_PARAM_LEN        (ST_MOD_CFA_SETTING_LEN)
#define ST_MOD_CFA_SET_PARAM_LEN        (ST_MOD_CFA_SETTING_LEN)
#define ST_CFA_PARAM_SIZE               (sizeof(ST_CFA_PARAM))
#define ST_CFA_PARAM_LEN                (ST_CFA_PARAM_SIZE)

typedef struct _ST_ISP_MOD_CFA {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstCfaRdmaBuf;                /** CFA's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_CFA_RDMA *pstIspCfaRdma;             /** CFA's RDMA structure pointer that include the ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_CFA_RDMA *pstCfaRdma;                    /** CFA's RDMA structure pointer.*/
    ST_CFA_PARAM *pstCfaParam;                  /** CFA's parameters structure pointer. */
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
} ST_ISP_MOD_CFA, *PST_ISP_MOD_CFA;

#pragma pack(pop)


/* demosaic module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CFA_InitRdma(
    ST_CFA_RDMA *pstCfaRdma,
    ST_CFA_PARAM *pstCfaParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CFA_UpdateRdma(
    ST_CFA_RDMA *pstCfaRdma,
    ST_CFA_PARAM *pstCfaParam
    );
extern
STF_S32 STFMOD_ISP_CFA_SetNextRdma(
    ST_CFA_RDMA *pstCfaRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CFA_SetEnable(
    ST_ISP_MOD_CFA *pstModCfa,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CFA_SetUpdate(
    ST_ISP_MOD_CFA *pstModCfa,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_CFA_SetMode(
    ST_ISP_MOD_CFA *pstModCfa,
    STF_U8 u8HvWidth,
    STF_U8 u8CrossCov
    );
extern
STF_S32 STFMOD_ISP_CFA_SetHvWidth(
    ST_ISP_MOD_CFA *pstModCfa,
    STF_U8 u8HvWidth
    );
extern
STF_S32 STFMOD_ISP_CFA_SetCrossCovWeight(
    ST_ISP_MOD_CFA *pstModCfa,
    STF_U8 u8CrossCov
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CFA_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CFA_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CFA_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CFA_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_CFA_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CFA_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_CFA_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CFA_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_CFA_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CFA_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CFA_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CFA_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CFA_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CFA_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CFA_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CFA_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_CFA_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_CFA_AHD_H__
