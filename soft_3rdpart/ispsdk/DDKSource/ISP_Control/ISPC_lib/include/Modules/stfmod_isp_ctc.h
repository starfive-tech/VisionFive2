/**
  ******************************************************************************
  * @file  stfmod_isp_ctc.h
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


#ifndef __STFMOD_ISP_CTC_H__
#define __STFMOD_ISP_CTC_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_ctc_rdma.h"
#include "stflib_isp_pipeline.h"


/* crosstalk compensation filter module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#if 0
#define ISP_DEF_CTC_SMOOTH_AREA_MODE            (EN_CTC_SMOOTH_AREA_5x5_NEIGHBORHOOD)
#define ISP_DEF_CTC_DETAIL_ARE_MODE             (EN_CTC_DETAIL_AREA_5x5_NEIGHBORHOOD)
#define ISP_DEF_CTC_MAX_GR_GB_THRESHOLD         (300)
#define ISP_DEF_CTC_MIN_GR_GB_THRESHOLD         (300)
#elif 0
// 20201219_v01
#define ISP_DEF_CTC_SMOOTH_AREA_MODE            (EN_CTC_SMOOTH_AREA_CENTER_PIXEL)
#define ISP_DEF_CTC_DETAIL_ARE_MODE             (EN_CTC_DETAIL_AREA_5x5_NEIGHBORHOOD)
#define ISP_DEF_CTC_MAX_GR_GB_THRESHOLD         (160)
#define ISP_DEF_CTC_MIN_GR_GB_THRESHOLD         (32)
#elif 1
// 20210103_v01
#define ISP_DEF_CTC_SMOOTH_AREA_MODE            (EN_CTC_SMOOTH_AREA_CENTER_PIXEL)
#define ISP_DEF_CTC_DETAIL_ARE_MODE             (EN_CTC_DETAIL_AREA_5x5_NEIGHBORHOOD)
#define ISP_DEF_CTC_MAX_GR_GB_THRESHOLD         (320)
#define ISP_DEF_CTC_MIN_GR_GB_THRESHOLD         (64)
#endif


#pragma pack(push, 1)

typedef enum _EN_CTC_UPDATE {
    EN_CTC_UPDATE_NONE = 0x00,
    EN_CTC_UPDATE_FILTER_MODE = 0x01,
    EN_CTC_UPDATE_MAX_MIN_THRESHOLD = 0x02,
    EN_CTC_UPDATE_ALL = (EN_CTC_UPDATE_FILTER_MODE
                       | EN_CTC_UPDATE_MAX_MIN_THRESHOLD
                       ),
} EN_CTC_UPDATE, *PEN_CTC_UPDATE;

typedef enum _EN_CTC_SMOOTH_AREA {
    EN_CTC_SMOOTH_AREA_CENTER_PIXEL = 0,
    EN_CTC_SMOOTH_AREA_5x5_NEIGHBORHOOD,
    EN_CTC_SMOOTH_AREA_MAX
} EN_CTC_SMOOTH_AREA, *PEN_CTC_SMOOTH_AREA;

typedef enum _EN_CTC_DETAIL_AREA {
    EN_CTC_DETAIL_AREA_AVERAGING_CORRECTION = 0,
    EN_CTC_DETAIL_AREA_5x5_NEIGHBORHOOD,
    EN_CTC_DETAIL_AREA_MAX
} EN_CTC_DETAIL_AREA, *PEN_CTC_DETAIL_AREA;


typedef struct _ST_MOD_CTC_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CTC module. */
    STF_U8 u8SmoothAreaFilterMode;              /** CTC filter mode for smooth area. 0:EN_CTC_SMOOTH_AREA_CENTER_PIXEL, 1:EN_CTC_SMOOTH_AREA_5x5_NEIGHBORHOOD. */
    STF_U8 u8DetailAreaFilterMode;              /** CTC filter mode for detail area. 0:EN_CTC_DETAIL_AREA_AVERAGING_CORRECTION, 1:EN_CTC_DETAIL_AREA_5x5_NEIGHBORHOOD. */
    STF_U16 u16MaxGT;                           /** CTC filter maximum gr and gb channel threshold. */
    STF_U16 u16MinGT;                           /** CTC filter minimum gr and gb channel threshold. */
} ST_MOD_CTC_SETTING, *PST_MOD_CTC_SETTING;

typedef struct _ST_CTC_PARAM {
    ST_MOD_CTC_SETTING stSetting;               /** Module CTC setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program CTC module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_CTC_PARAM, *PST_CTC_PARAM;

#define ST_MOD_CTC_SETTING_LEN          (sizeof(ST_MOD_CTC_SETTING))
#define ST_MOD_CTC_GET_PARAM_LEN        (ST_MOD_CTC_SETTING_LEN)
#define ST_MOD_CTC_SET_PARAM_LEN        (ST_MOD_CTC_SETTING_LEN)
#define ST_CTC_PARAM_SIZE               (sizeof(ST_CTC_PARAM))
#define ST_CTC_PARAM_LEN                (ST_CTC_PARAM_SIZE)

typedef struct _ST_ISP_MOD_CTC {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstCtcRdmaBuf;                /** CTC's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_CTC_RDMA *pstIspCtcRdma;             /** CTC's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_CTC_RDMA *pstCtcRdma;                    /** CTC's RDMA structure pointer.*/
    ST_CTC_PARAM *pstCtcParam;                  /** CTC's parameters structure pointer. */
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
} ST_ISP_MOD_CTC, *PST_ISP_MOD_CTC;

#pragma pack(pop)


/* crosstalk compensation filter module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CTC_InitRdma(
    ST_CTC_RDMA *pstCtcRdma,
    ST_CTC_PARAM *pstCtcParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CTC_UpdateRdma(
    ST_CTC_RDMA *pstCtcRdma,
    ST_CTC_PARAM *pstCtcParam
    );
extern
STF_S32 STFMOD_ISP_CTC_UpdateFilterMode(
    ST_CTC_RDMA *pstCtcRdma,
    ST_CTC_PARAM *pstCtcParam
    );
extern
STF_S32 STFMOD_ISP_CTC_UpdateMaxMinThreshold(
    ST_CTC_RDMA *pstCtcRdma,
    ST_CTC_PARAM *pstCtcParam
    );
extern
STF_S32 STFMOD_ISP_CTC_SetNextRdma(
    ST_CTC_RDMA *pstCtcRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CTC_SetEnable(
    ST_ISP_MOD_CTC *pstModCtc,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CTC_SetUpdate(
    ST_ISP_MOD_CTC *pstModCtc,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_CTC_SetFilterMode(
    ST_ISP_MOD_CTC *pstModCtc,
    STF_U8 u8SmoothAreaFilterMode,
    STF_U8 u8DetailAreaFilterMode
    );
extern
STF_S32 STFMOD_ISP_CTC_SetMaxMinThreshold(
    ST_ISP_MOD_CTC *pstModCtc,
    STF_U16 u16MaxGT,
    STF_U16 u16MinGT
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CTC_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CTC_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CTC_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CTC_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_CTC_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CTC_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_CTC_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CTC_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_CTC_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CTC_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CTC_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CTC_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CTC_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CTC_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CTC_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CTC_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_CTC_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_CTC_H__
