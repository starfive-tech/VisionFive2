/**
  ******************************************************************************
  * @file  stfmod_isp_oba.h
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


#ifndef __STFMOD_ISP_OBA_H__
#define __STFMOD_ISP_OBA_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_oba_rdma.h"
#include "stflib_isp_pipeline.h"


/** optical black statistics module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
//#define ISP_DEF_OBA_WINDOW_LEFT                 (0)                     /** Horizontal starting position of acquisition window for optical black statistical collection. */
//#define ISP_DEF_OBA_WINDOW_TOP                  (ISP_IMG_HEIGHT)        /** Vertical starting position of acquisition window for optical black statistical collection. */
//#define ISP_DEF_OBA_WINDOW_RIGHT                (ISP_IMG_WIDTH)         /** Horizontal ending position of acquisition window for optical black statistical collection. */
//#define ISP_DEF_OBA_WINDOW_BOTTOM               (ISP_IMG_HEIGHT + 1)    /** Vertical ending position of acquisition window for optical black statistical collection. */


#pragma pack(push, 1)

typedef enum _EN_OBA_UPDATE {
    EN_OBA_UPDATE_NONE = 0x00,
    EN_OBA_UPDATE_WINDOW = 0X01,
    EN_OBA_UPDATE_ALL = (EN_OBA_UPDATE_WINDOW),
} EN_OBA_UPDATE, *PEN_OBA_UPDATE;


typedef struct _ST_MOD_OBA_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable OBA module. */
#if 0
    STF_U16 u16HorStart;                        /** Horizontal starting position of acquisition window for optical black statistical collection. */
    STF_U16 u16VerStart;                        /** Vertical starting position of acquisition window for optical black statistical collection. */
    STF_U16 u16HorEnd;                          /** Horizontal ending position of acquisition window for optical black statistical collection. */
    STF_U16 u16VerEnd;                          /** Vertical ending position of acquisition window for optical black statistical collection. */
#else
    ST_RECT stObaWindow;                        /** Acquisition window for optical black statistical collection. */
#endif
} ST_MOD_OBA_SETTING, *PST_MOD_OBA_SETTING;

typedef struct _ST_MOD_OBA_INFO {
    STF_U32 u32R;                               /** R channel optical black statistics value. */
    STF_U32 u32Gr;                              /** Gr channel optical black statistics value. */
    STF_U32 u32Gb;                              /** Gb channel optical black statistics value. */
    STF_U32 u32B;                               /** B channel optical black statistics value. */
} ST_MOD_OBA_INFO, *PST_MOD_OBA_INFO;

typedef struct _ST_OBA_PARAM {
    ST_MOD_OBA_SETTING stSetting;               /** Module OBA setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_OBA_INFO stInfo;                     /** Module OBA output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program OBA module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_OBA_PARAM, *PST_OBA_PARAM;

#define ST_MOD_OBA_SETTING_LEN          (sizeof(ST_MOD_OBA_SETTING))
#define ST_MOD_OBA_INFO_LEN             (sizeof(ST_MOD_OBA_INFO))
#define ST_MOD_OBA_GET_PARAM_LEN        (ST_MOD_OBA_SETTING_LEN + ST_MOD_OBA_INFO_LEN)
#define ST_MOD_OBA_SET_PARAM_LEN        (ST_MOD_OBA_SETTING_LEN)
#define ST_OBA_PARAM_SIZE               (sizeof(ST_OBA_PARAM))
#define ST_OBA_PARAM_LEN                (ST_OBA_PARAM_SIZE)

typedef struct _ST_ISP_MOD_OBA {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstObaRdmaBuf;                /** OBA's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_OBA_RDMA *pstIspObaRdma;             /** OBA's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_OBA_RDMA *pstObaRdma;                    /** OBA's RDMA structure pointer.*/
    ST_OBA_PARAM *pstObaParam;                  /** OBA's parameters structure pointer. */
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
} ST_ISP_MOD_OBA, *PST_ISP_MOD_OBA;

#pragma pack(pop)


/* optical black statistics module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBA_InitRdma(
    ST_OBA_RDMA *pstObaRdma,
    ST_OBA_PARAM *pstObaParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OBA_UpdateRdma(
    ST_OBA_RDMA *pstObaRdma,
    ST_OBA_PARAM *pstObaParam
    );
extern
STF_S32 STFMOD_ISP_OBA_SetNextRdma(
    ST_OBA_RDMA *pstObaRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBA_SetEnable(
    ST_ISP_MOD_OBA *pstModOba,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OBA_SetUpdate(
    ST_ISP_MOD_OBA *pstModOba,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OBA_SetObaWindow(
    ST_ISP_MOD_OBA *pstModOba,
    ST_RECT *pstObaWindow
    );
extern
STF_S32 STFMOD_ISP_OBA_GetStatisticsInfo(
    ST_ISP_MOD_OBA *pstModOba,
    STF_U32 *pu32R,
    STF_U32 *pu32Gr,
    STF_U32 *pu32Gb,
    STF_U32 *pu32B
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBA_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OBA_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OBA_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBA_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_OBA_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBA_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_OBA_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBA_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_OBA_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBA_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBA_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBA_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OBA_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBA_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OBA_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBA_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_OBA_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_OBA_H__
