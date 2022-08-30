/**
  ******************************************************************************
  * @file  stfmod_isp_lccf.h
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


#ifndef __STFMOD_ISP_LCCF_H__
#define __STFMOD_ISP_LCCF_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_lccf_rdma.h"
#include "stflib_isp_pipeline.h"


/* lens correction cos4 module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#if 0
#define ISP_DEF_LCCF_CENTER_X                   (960)   /** 960 = 1920 / 2. */
#define ISP_DEF_LCCF_CENTER_Y                   (540)   /** 540 = 1080 / 2. */
#define ISP_DEF_LCCF_RADIUS                     (11)    /** 2048 = 2^11. */
#define ISP_DEF_LCCF_R_FACTOR_1                 (1.25)
#define ISP_DEF_LCCF_R_FACTOR_2                 (0.5)
#define ISP_DEF_LCCF_GR_FACTOR_1                (1.25)
#define ISP_DEF_LCCF_GR_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_GB_FACTOR_1                (1.25)
#define ISP_DEF_LCCF_GB_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_B_FACTOR_1                 (1.25)
#define ISP_DEF_LCCF_B_FACTOR_2                 (0.5)
#elif 0
// 20201229_v01
#define ISP_DEF_LCCF_CENTER_X                   (960)   /** 960 = 1920 / 2. */
#define ISP_DEF_LCCF_CENTER_Y                   (540)   /** 540 = 1080 / 2. */
#define ISP_DEF_LCCF_RADIUS                     (11)    /** 2048 = 2^11. */
#define ISP_DEF_LCCF_R_FACTOR_1                 (2.0)
#define ISP_DEF_LCCF_R_FACTOR_2                 (0.5)
#define ISP_DEF_LCCF_GR_FACTOR_1                (2.0)
#define ISP_DEF_LCCF_GR_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_GB_FACTOR_1                (2.0)
#define ISP_DEF_LCCF_GB_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_B_FACTOR_1                 (2.0)
#define ISP_DEF_LCCF_B_FACTOR_2                 (0.5)
#elif 0
// 20210103_v01
#define ISP_DEF_LCCF_CENTER_X                   (960)   /** 960 = 1920 / 2. */
#define ISP_DEF_LCCF_CENTER_Y                   (540)   /** 540 = 1080 / 2. */
#define ISP_DEF_LCCF_RADIUS                     (11)    /** 2048 = 2^11. */
#define ISP_DEF_LCCF_R_FACTOR_1                 (1.5)
#define ISP_DEF_LCCF_R_FACTOR_2                 (0.5)
#define ISP_DEF_LCCF_GR_FACTOR_1                (1.5)
#define ISP_DEF_LCCF_GR_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_GB_FACTOR_1                (1.5)
#define ISP_DEF_LCCF_GB_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_B_FACTOR_1                 (1.5)
#define ISP_DEF_LCCF_B_FACTOR_2                 (0.5)
#elif 1
// 20210104_v01
#define ISP_DEF_LCCF_CENTER_X                   (960)   /** 960 = 1920 / 2. */
#define ISP_DEF_LCCF_CENTER_Y                   (540)   /** 540 = 1080 / 2. */
#define ISP_DEF_LCCF_RADIUS                     (11)    /** 2048 = 2^11. */
#define ISP_DEF_LCCF_R_FACTOR_1                 (1.8)
#define ISP_DEF_LCCF_R_FACTOR_2                 (0.5)
#define ISP_DEF_LCCF_GR_FACTOR_1                (1.8)
#define ISP_DEF_LCCF_GR_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_GB_FACTOR_1                (1.8)
#define ISP_DEF_LCCF_GB_FACTOR_2                (0.5)
#define ISP_DEF_LCCF_B_FACTOR_1                 (1.8)
#define ISP_DEF_LCCF_B_FACTOR_2                 (0.5)
#endif


#pragma pack(push, 1)

typedef enum _EN_LCCF_UPDATE {
    EN_LCCF_UPDATE_NONE = 0x00,
    EN_LCCF_UPDATE_CENTER = 0x01,
    EN_LCCF_UPDATE_RADIUS = 0x02,
    EN_LCCF_UPDATE_FACTOR = 0x04,
    EN_LCCF_UPDATE_ALL = (EN_LCCF_UPDATE_CENTER
                        | EN_LCCF_UPDATE_RADIUS
                        | EN_LCCF_UPDATE_FACTOR
                        ),
} EN_LCCF_UPDATE, *PEN_LCCF_UPDATE;

#if !defined(__STF_ISP_LCCF_RDMA_H__)
typedef enum _EN_LCCF_CHN {
    EN_LCCF_CHN_R = 0,
    EN_LCCF_CHN_GR,
    EN_LCCF_CHN_GB,
    EN_LCCF_CHN_B,
    EN_LCCF_CHN_MAX,
} EN_LCCF_CHN, *PEN_LCCF_CHN;

#endif //__STF_ISP_LCCF_RDMA_H__

typedef struct _ST_LCCF_FACTOR {
    STF_DOUBLE dFactor[EN_LCCF_CHN_MAX][2];
} ST_LCCF_FACTOR, *PST_LCCF_FACTOR;

typedef struct _ST_MOD_LCCF_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable LCCF module. */
    STF_U16 u16CenterX;                         /** X distance from capture window, 15bits. */
    STF_U16 u16CenterY;                         /** Y distance from capture window, 15bits. */
    STF_U8 u8Radius;                            /** Parameter for typical maximum distance (dM = 2^M), 4bits */
    STF_DOUBLE dRFactor[2];                     /** R channel factor parameters. */
    STF_DOUBLE dGrFactor[2];                    /** Gr channel factor parameters. */
    STF_DOUBLE dGbFactor[2];                    /** Gb channel factor parameters. */
    STF_DOUBLE dBFactor[2];                     /** R channel factor parameters. */
} ST_MOD_LCCF_SETTING, *PST_MOD_LCCF_SETTING;

typedef struct _ST_LCCF_PARAM {
    ST_MOD_LCCF_SETTING stSetting;              /** Module LCCF setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program LCCF module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_LCCF_PARAM, *PST_LCCF_PARAM;

#define ST_MOD_LCCF_SETTING_LEN         (sizeof(ST_MOD_LCCF_SETTING))
#define ST_MOD_LCCF_GET_PARAM_LEN       (ST_MOD_LCCF_SETTING_LEN)
#define ST_MOD_LCCF_SET_PARAM_LEN       (ST_MOD_LCCF_SETTING_LEN)
#define ST_LCCF_PARAM_SIZE              (sizeof(ST_LCCF_PARAM))
#define ST_LCCF_PARAM_LEN               (ST_LCCF_PARAM_SIZE)

typedef struct _ST_ISP_MOD_LCCF {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstLccfRdmaBuf;               /** LCCF's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_LCCF_RDMA *pstIspLccfRdma;           /** LCCF's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_LCCF_RDMA *pstLccfRdma;                  /** LCCF's RDMA structure pointer.*/
    ST_LCCF_PARAM *pstLccfParam;                /** LCCF's parameters structure pointer. */
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
} ST_ISP_MOD_LCCF, *PST_ISP_MOD_LCCF;

#pragma pack(pop)


/* lens correction cos4 module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCCF_InitRdma(
    ST_LCCF_RDMA *pstLccfRdma,
    ST_LCCF_PARAM *pstLccfParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_LCCF_UpdateRdma(
    ST_LCCF_RDMA *pstLccfRdma,
    ST_LCCF_PARAM *pstLccfParam
    );
extern
STF_S32 STFMOD_ISP_LCCF_UpdateCenter(
    ST_LCCF_RDMA *pstLccfRdma,
    ST_LCCF_PARAM *pstLccfParam
    );
extern
STF_S32 STFMOD_ISP_LCCF_UpdateRadius(
    ST_LCCF_RDMA *pstLccfRdma,
    ST_LCCF_PARAM *pstLccfParam
    );
extern
STF_S32 STFMOD_ISP_LCCF_UpdateFactor(
    ST_LCCF_RDMA *pstLccfRdma,
    ST_LCCF_PARAM *pstLccfParam
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetNextRdma(
    ST_LCCF_RDMA *pstLccfRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCCF_SetEnable(
    ST_ISP_MOD_LCCF *pstModLccf,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetUpdate(
    ST_ISP_MOD_LCCF *pstModLccf,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetCenter(
    ST_ISP_MOD_LCCF *pstModLccf,
    STF_U16 u16CenterX,
    STF_U16 u16CenterY
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetRadius(
    ST_ISP_MOD_LCCF *pstModLccf,
    STF_U8 u8Radius
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetAllFactor(
    ST_ISP_MOD_LCCF *pstModLccf,
    STF_DOUBLE dRFactor[2],
    STF_DOUBLE dGrFactor[2],
    STF_DOUBLE dGbFactor[2],
    STF_DOUBLE dBFactor[2]
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetFactor(
    ST_ISP_MOD_LCCF *pstModLccf,
    EN_LCCF_CHN enChn,
    STF_DOUBLE dFactor[2]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCCF_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_LCCF_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_LCCF_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCCF_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_LCCF_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCCF_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_LCCF_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCCF_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_LCCF_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCCF_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_LCCF_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCCF_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_LCCF_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCCF_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_LCCF_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_LCCF_H__
