/**
  ******************************************************************************
  * @file  stfmod_isp_lcbq.h
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


#ifndef __STFMOD_ISP_LCBQ_H__
#define __STFMOD_ISP_LCBQ_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_lcbq_rdma.h"
#include "stflib_isp_pipeline.h"


/* lens correction bi-quadratic module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_LCBQ_WIDTH                      (9)     /** 512 = 2^9, 2048 = 512 * 4. */
#define ISP_DEF_LCBQ_HEIGHT                     (9)     /** 512 = 2^9, 2048 = 512 * 4. */
#define ISP_DEF_LCBQ_COORDINATE_X               (64)    /** 64 = ((2048 - 1920) / 2). */
#define ISP_DEF_LCBQ_COORDINATE_Y               (484)   /** 484 = ((2048 - 1080) / 2). */


#pragma pack(push, 1)

typedef enum _EN_LCBQ_UPDATE {
    EN_LCBQ_UPDATE_NONE = 0x00,
    EN_LCBQ_UPDATE_WINDOW_SIZE = 0x01,
    EN_LCBQ_UPDATE_COORDINATE = 0x02,
    EN_LCBQ_UPDATE_GAIN = 0x04,
    EN_LCBQ_UPDATE_OFFSET = 0x08,
    EN_LCBQ_UPDATE_ALL = (EN_LCBQ_UPDATE_WINDOW_SIZE
                        | EN_LCBQ_UPDATE_COORDINATE
                        | EN_LCBQ_UPDATE_GAIN
                        | EN_LCBQ_UPDATE_OFFSET
                        ),
} EN_LCBQ_UPDATE, *PEN_LCBQ_UPDATE;

typedef enum _EN_LCBQ_CHN {
    EN_LCBQ_CHN_R = 0,
    EN_LCBQ_CHN_GR,
    EN_LCBQ_CHN_GB,
    EN_LCBQ_CHN_B,
    EN_LCBQ_CHN_MAX,
} EN_LCBQ_CHN, *PEN_LCBQ_CHN;


typedef struct _ST_MOD_LCBQ_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable LCBQ module. */
    STF_U8 u8Width;                             /** LCBQ width of pseudo sub-window(2^u8Width). */
    STF_U8 u8Height;                            /** LCBQ height of pseudo sub-window(2^u8Height). */
    STF_U16 u16XCoordinate;                     /** X coordinate. */
    STF_U16 u16YCoordinate;                     /** Y coordinate. */
    STF_DOUBLE dRGain[5][5];                    /** R channel gain parameter. */
    STF_DOUBLE dGrGain[5][5];                   /** Gr channel gain parameter. */
    STF_DOUBLE dGbGain[5][5];                   /** Gb channel gain parameter. */
    STF_DOUBLE dBGain[5][5];                    /** B channel gain parameter. */
    STF_S16 s16ROffset[5][5];                   /** R channel offset parameter. */
    STF_S16 s16GrOffset[5][5];                  /** Gr channel offset parameter. */
    STF_S16 s16GbOffset[5][5];                  /** Gb channel offset parameter. */
    STF_S16 s16BOffset[5][5];                   /** B channel offset parameter. */
} ST_MOD_LCBQ_SETTING, *PST_MOD_LCBQ_SETTING;

typedef struct _ST_LCBQ_PARAM {
    ST_MOD_LCBQ_SETTING stSetting;              /** Module LCBQ setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program LCBQ module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_LCBQ_PARAM, *PST_LCBQ_PARAM;

#define ST_MOD_LCBQ_SETTING_LEN         (sizeof(ST_MOD_LCBQ_SETTING))
#define ST_MOD_LCBQ_GET_PARAM_LEN       (ST_MOD_LCBQ_SETTING_LEN)
#define ST_MOD_LCBQ_SET_PARAM_LEN       (ST_MOD_LCBQ_SETTING_LEN)
#define ST_LCBQ_PARAM_SIZE              (sizeof(ST_LCBQ_PARAM))
#define ST_LCBQ_PARAM_LEN               (ST_LCBQ_PARAM_SIZE)

typedef struct _ST_ISP_MOD_LCBQ {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstLcbqRdmaBuf;               /** LCBQ's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_LCBQ_RDMA *pstIspLcbqRdma;           /** LCBQ's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_LCBQ_RDMA *pstLcbqRdma;                  /** LCBQ's RDMA structure pointer.*/
    ST_LCBQ_PARAM *pstLcbqParam;                /** LCBQ's parameters structure pointer. */
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
} ST_ISP_MOD_LCBQ, *PST_ISP_MOD_LCBQ;

#pragma pack(pop)


/* lens correction bi-quadratic module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCBQ_InitRdma(
    ST_LCBQ_RDMA *pstLcbqRdma,
    ST_LCBQ_PARAM *pstLcbqParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_LCBQ_UpdateRdma(
    ST_LCBQ_RDMA *pstLcbqRdma,
    ST_LCBQ_PARAM *pstLcbqParam
    );
extern
STF_S32 STFMOD_ISP_LCBQ_UpdateWindowSize(
    ST_LCBQ_RDMA *pstLcbqRdma,
    ST_LCBQ_PARAM *pstLcbqParam
    );
extern
STF_S32 STFMOD_ISP_LCBQ_UpdateCoordinate(
    ST_LCBQ_RDMA *pstLcbqRdma,
    ST_LCBQ_PARAM *pstLcbqParam
    );
extern
STF_S32 STFMOD_ISP_LCBQ_UpdateGain(
    ST_LCBQ_RDMA *pstLcbqRdma,
    ST_LCBQ_PARAM *pstLcbqParam
    );
extern
STF_S32 STFMOD_ISP_LCBQ_UpdateOffset(
    ST_LCBQ_RDMA *pstLcbqRdma,
    ST_LCBQ_PARAM *pstLcbqParam
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetNextRdma(
    ST_LCBQ_RDMA *pstLcbqRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCBQ_SetEnable(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetUpdate(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetWindowSize(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    STF_U8 u8Width,
    STF_U8 u8Height
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetCoordinate(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    STF_U16 u16XCoordinate,
    STF_U16 u16YCoordinate
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetAllGain(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    STF_DOUBLE dRGain[5][5],
    STF_DOUBLE dGrGain[5][5],
    STF_DOUBLE dGbGain[5][5],
    STF_DOUBLE dBGain[5][5]
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetGain(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    EN_LCBQ_CHN enChn,
    STF_DOUBLE dGain[5][5]
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetAllOffset(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    STF_S16 s16ROffset[5][5],
    STF_S16 s16GrOffset[5][5],
    STF_S16 s16GbOffset[5][5],
    STF_S16 s16BOffset[5][5]
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetOffset(
    ST_ISP_MOD_LCBQ *pstModLcbq,
    EN_LCBQ_CHN enChn,
    STF_S16 s16Offset[5][5]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCBQ_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_LCBQ_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_LCBQ_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCBQ_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_LCBQ_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCBQ_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_LCBQ_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCBQ_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_LCBQ_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCBQ_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_LCBQ_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_LCBQ_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_LCBQ_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_LCBQ_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_LCBQ_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_LCBQ_H__
