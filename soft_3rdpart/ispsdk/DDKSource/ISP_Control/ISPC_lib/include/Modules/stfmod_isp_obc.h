/**
  ******************************************************************************
  * @file  stfmod_isp_obc.h
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


#ifndef __STFMOD_ISP_OBC_H__
#define __STFMOD_ISP_OBC_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_obc_rdma.h"
#include "stflib_isp_pipeline.h"


/* optical black compensation module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_OBC_WITH                        (11)        // 2048 = 2^11
#define ISP_DEF_OBC_HEIGHT                      (11)        // 2048 = 2^11
#define ISP_DEF_OBC_R_GAIN                      (1.0)
#define ISP_DEF_OBC_GR_GAIN                     (1.0)
#define ISP_DEF_OBC_GB_GAIN                     (1.0)
#define ISP_DEF_OBC_B_GAIN                      (1.0)
#define ISP_DEF_OBC_R_OFFSET                    (0)
#define ISP_DEF_OBC_GR_OFFSET                   (0)
#define ISP_DEF_OBC_GB_OFFSET                   (0)
#define ISP_DEF_OBC_B_OFFSET                    (0)


#pragma pack(push, 1)

typedef enum _EN_OBC_UPDATE {
    EN_OBC_UPDATE_NONE = 0x00,
    EN_OBC_UPDATE_SIZE = 0x01,
    EN_OBC_UPDATE_GAIN = 0x02,
    EN_OBC_UPDATE_OFFSET = 0x04,
    EN_OBC_UPDATE_ALL = (EN_OBC_UPDATE_SIZE
                       | EN_OBC_UPDATE_GAIN
                       | EN_OBC_UPDATE_OFFSET
                       ),
} EN_OBC_UPDATE, *PEN_OBC_UPDATE;

typedef enum _EN_OBC_CHN {
    EN_OBC_CHN_R = 0,
    EN_OBC_CHN_GR,
    EN_OBC_CHN_GB,
    EN_OBC_CHN_B,
    EN_OBC_CHN_MAX,
} EN_OBC_CHN, *PEN_OBC_CHN;

typedef struct _ST_OBC_GAIN {
    STF_FLOAT fTopLeft;
    STF_FLOAT fTopRight;
    STF_FLOAT fBottomLeft;
    STF_FLOAT fBottomRight;
} ST_OBC_GAIN, *PST_OBC_GAIN;

typedef struct _ST_OBC_OFFSET {
    STF_U8 u8TopLeft;
    STF_U8 u8TopRight;
    STF_U8 u8BottomLeft;
    STF_U8 u8BottomRight;
} ST_OBC_OFFSET, *PST_OBC_OFFSET;

typedef struct _ST_MOD_OBC_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable OBC module. */
    STF_U8 u8Width;                             /** Width. 2^u8Width */
    STF_U8 u8Height;                            /** Height. 2^u8Height */
    ST_OBC_GAIN stRGain;                        /** R channel gain. */
    ST_OBC_GAIN stGrGain;                       /** Gr channel gain. */
    ST_OBC_GAIN stGbGain;                       /** Gb channel gain. */
    ST_OBC_GAIN stBGain;                        /** B channel gain. */
    ST_OBC_OFFSET stROffset;                    /** R channel offset. */
    ST_OBC_OFFSET stGrOffset;                   /** Gr channel offset. */
    ST_OBC_OFFSET stGbOffset;                   /** Gb channel offset. */
    ST_OBC_OFFSET stBOffset;                    /** B channel offset. */
} ST_MOD_OBC_SETTING, *PST_MOD_OBC_SETTING;

typedef struct _ST_OBC_PARAM {
    ST_MOD_OBC_SETTING stSetting;               /** Module OBC setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program OBC module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_OBC_PARAM, *PST_OBC_PARAM;

#define ST_MOD_OBC_SETTING_LEN          (sizeof(ST_MOD_OBC_SETTING))
#define ST_MOD_OBC_GET_PARAM_LEN        (ST_MOD_OBC_SETTING_LEN)
#define ST_MOD_OBC_SET_PARAM_LEN        (ST_MOD_OBC_SETTING_LEN)
#define ST_OBC_PARAM_SIZE               (sizeof(ST_OBC_PARAM))
#define ST_OBC_PARAM_LEN                (ST_OBC_PARAM_SIZE)

typedef struct _ST_ISP_MOD_OBC {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstObcRdmaBuf;                /** OBC's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_OBC_RDMA *pstIspObcRdma;             /** OBC's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_OBC_RDMA *pstObcRdma;                    /** OBC's RDMA structure pointer.*/
    ST_OBC_PARAM *pstObcParam;                  /** OBC's parameters structure pointer. */
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
} ST_ISP_MOD_OBC, *PST_ISP_MOD_OBC;

#pragma pack(pop)


/* optical black compensation module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBC_InitRdma(
    ST_OBC_RDMA *pstObcRdma,
    ST_OBC_PARAM *pstObcParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OBC_UpdateRdma(
    ST_OBC_RDMA *pstObcRdma,
    ST_OBC_PARAM *pstObcParam
    );
extern
STF_S32 STFMOD_ISP_OBC_UpdateSize(
    ST_OBC_RDMA *pstObcRdma,
    ST_OBC_PARAM *pstObcParam
    );
extern
STF_S32 STFMOD_ISP_OBC_UpdateGain(
    ST_OBC_RDMA *pstObcRdma,
    ST_OBC_PARAM *pstObcParam
    );
extern
STF_S32 STFMOD_ISP_OBC_UpdateOffset(
    ST_OBC_RDMA *pstObcRdma,
    ST_OBC_PARAM *pstObcParam
    );
extern
STF_S32 STFMOD_ISP_OBC_SetNextRdma(
    ST_OBC_RDMA *pstObcRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBC_SetEnable(
    ST_ISP_MOD_OBC *pstModObc,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OBC_SetUpdate(
    ST_ISP_MOD_OBC *pstModObc,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OBC_SetSize(
    ST_ISP_MOD_OBC *pstModObc,
    STF_U8 u8Width,
    STF_U8 u8Height
    );
extern
STF_S32 STFMOD_ISP_OBC_SetAllGain(
    ST_ISP_MOD_OBC *pstModObc,
    ST_OBC_GAIN *pstRGain,
    ST_OBC_GAIN *pstGrGain,
    ST_OBC_GAIN *pstGbGain,
    ST_OBC_GAIN *pstBGain
    );
extern
STF_S32 STFMOD_ISP_OBC_SetGain(
    ST_ISP_MOD_OBC *pstModObc,
    EN_OBC_CHN enChn,
    ST_OBC_GAIN *pstGain
    );
extern
STF_S32 STFMOD_ISP_OBC_SetAllOffset(
    ST_ISP_MOD_OBC *pstModObc,
    ST_OBC_OFFSET *pstROffset,
    ST_OBC_OFFSET *pstGrOffset,
    ST_OBC_OFFSET *pstGbOffset,
    ST_OBC_OFFSET *pstBOffset
    );
extern
STF_S32 STFMOD_ISP_OBC_SetOffset(
    ST_ISP_MOD_OBC *pstModObc,
    EN_OBC_CHN enChn,
    ST_OBC_OFFSET *pstOffset
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBC_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OBC_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OBC_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBC_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_OBC_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBC_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_OBC_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBC_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_OBC_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBC_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBC_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBC_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OBC_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OBC_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OBC_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OBC_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_OBC_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_OBC_H__
