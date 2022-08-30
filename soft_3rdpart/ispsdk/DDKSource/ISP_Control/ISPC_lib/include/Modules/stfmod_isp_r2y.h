/**
  ******************************************************************************
  * @file  stfmod_isp_r2y.h
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


#ifndef __STFMOD_ISP_R2Y_H__
#define __STFMOD_ISP_R2Y_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_r2y_rdma.h"
#include "stflib_isp_pipeline.h"


/* RGB to YUV module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define CCM_COEFFICIENT_SIGNED_BITS             (1)
#define CCM_COEFFICIENT_INTEGER_BITS            (0)
#define CCM_COEFFICIENT_FRACTION_BITS           (8)

#if 0
#define ISP_DEF_R2Y_MATRIX_00                   (0.299)         /** RGB to YUV matrix component 00.. */
#define ISP_DEF_R2Y_MATRIX_01                   (0.587)         /** RGB to YUV matrix component 01.. */
#define ISP_DEF_R2Y_MATRIX_02                   (0.114)         /** RGB to YUV matrix component 02.. */
#define ISP_DEF_R2Y_MATRIX_10                   (-0.14713)      /** RGB to YUV matrix component 10.. */
#define ISP_DEF_R2Y_MATRIX_11                   (-0.28886)      /** RGB to YUV matrix component 11.. */
#define ISP_DEF_R2Y_MATRIX_12                   (0.436)         /** RGB to YUV matrix component 12.. */
#define ISP_DEF_R2Y_MATRIX_20                   (0.615)         /** RGB to YUV matrix component 20.. */
#define ISP_DEF_R2Y_MATRIX_21                   (-0.51499)      /** RGB to YUV matrix component 21.. */
#define ISP_DEF_R2Y_MATRIX_22                   (-0.10001)      /** RGB to YUV matrix component 22.. */
#elif 1
// 20201219_v01
#define ISP_DEF_R2Y_MATRIX_00                   (0.296875)      /** RGB to YUV matrix component 00.. */
#define ISP_DEF_R2Y_MATRIX_01                   (0.58984375)    /** RGB to YUV matrix component 01.. */
#define ISP_DEF_R2Y_MATRIX_02                   (0.11328125)    /** RGB to YUV matrix component 02.. */
#define ISP_DEF_R2Y_MATRIX_10                   (-0.16796875)   /** RGB to YUV matrix component 10.. */
#define ISP_DEF_R2Y_MATRIX_11                   (-0.328125)     /** RGB to YUV matrix component 11.. */
#define ISP_DEF_R2Y_MATRIX_12                   (0.5)           /** RGB to YUV matrix component 12.. */
#define ISP_DEF_R2Y_MATRIX_20                   (0.5)           /** RGB to YUV matrix component 20.. */
#define ISP_DEF_R2Y_MATRIX_21                   (-0.421875)     /** RGB to YUV matrix component 21.. */
#define ISP_DEF_R2Y_MATRIX_22                   (-0.078125)     /** RGB to YUV matrix component 22.. */
#endif


#pragma pack(push, 1)

typedef enum _EN_R2Y_UPDATE {
    EN_R2Y_UPDATE_NONE = 0x00,
    EN_R2Y_UPDATE_MATRIX = 0x01,
    EN_R2Y_UPDATE_ALL = (EN_R2Y_UPDATE_MATRIX),
} EN_R2Y_UPDATE, *PEN_R2Y_UPDATE;


typedef struct _ST_MOD_R2Y_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable R2Y module. */
    STF_DOUBLE dMatrix[3][3];                   /** RGB to YUV convert matrix. */
} ST_MOD_R2Y_SETTING, *PST_MOD_R2Y_SETTING;

typedef struct _ST_R2Y_PARAM {
    ST_MOD_R2Y_SETTING stSetting;               /** Module R2Y setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program R2Y module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_R2Y_PARAM, *PST_R2Y_PARAM;

#define ST_MOD_R2Y_SETTING_LEN          (sizeof(ST_MOD_R2Y_SETTING))
#define ST_MOD_R2Y_GET_PARAM_LEN        (ST_MOD_R2Y_SETTING_LEN)
#define ST_MOD_R2Y_SET_PARAM_LEN        (ST_MOD_R2Y_SETTING_LEN)
#define ST_R2Y_PARAM_SIZE               (sizeof(ST_R2Y_PARAM))
#define ST_R2Y_PARAM_LEN                (ST_R2Y_PARAM_SIZE)

typedef struct _ST_ISP_MOD_R2Y {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstR2yRdmaBuf;                /** R2Y's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_R2Y_RDMA *pstIspR2yRdma;             /** R2Y's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_R2Y_RDMA *pstR2yRdma;                    /** R2Y's RDMA structure pointer.*/
    ST_R2Y_PARAM *pstR2yParam;                  /** R2Y's parameters structure pointer. */
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
} ST_ISP_MOD_R2Y, *PST_ISP_MOD_R2Y;

#pragma pack(pop)


/* RGB to YUV module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_R2Y_InitRdma(
    ST_R2Y_RDMA *pstR2yRdma,
    ST_R2Y_PARAM *pstR2yParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_R2Y_UpdateRdma(
    ST_R2Y_RDMA *pstR2yRdma,
    ST_R2Y_PARAM *pstR2yParam
    );
extern
STF_S32 STFMOD_ISP_R2Y_SetNextRdma(
    ST_R2Y_RDMA *pstR2yRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_R2Y_SetEnable(
    ST_ISP_MOD_R2Y *pstModR2y,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_R2Y_SetUpdate(
    ST_ISP_MOD_R2Y *pstModR2y,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_R2Y_SetMatrix(
    ST_ISP_MOD_R2Y *pstModR2y,
    STF_DOUBLE dMatrix[3][3]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_R2Y_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_R2Y_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_R2Y_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_R2Y_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_R2Y_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_R2Y_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_R2Y_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_R2Y_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_R2Y_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_R2Y_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_R2Y_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_R2Y_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_R2Y_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_R2Y_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_R2Y_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_R2Y_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_R2Y_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_R2Y_H__
