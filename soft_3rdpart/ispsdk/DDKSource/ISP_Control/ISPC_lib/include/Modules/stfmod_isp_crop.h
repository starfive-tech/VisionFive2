/**
  ******************************************************************************
  * @file  stfmod_isp_crop.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/23/2021
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


#ifndef __STFMOD_ISP_CROP_H__
#define __STFMOD_ISP_CROP_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_crop_rdma.h"
#include "stflib_isp_pipeline.h"


/* CSI cropping filter module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_CROP_OUTPUT_SIZE_W              (1920)
#define ISP_DEF_CROP_OUTPUT_SIZE_H              (1080)


#pragma pack(push, 1)

typedef enum _EN_CROP_UPDATE {
    EN_CROP_UPDATE_NONE = 0x00,
    EN_CROP_UPDATE_OUTPUT_SIZE = 0x01,
    EN_CROP_UPDATE_ALL = (EN_CROP_UPDATE_OUTPUT_SIZE),
} EN_CROP_UPDATE, *PEN_CROP_UPDATE;


typedef struct _ST_MOD_CROP_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CROP module. */
    ST_SIZE stOutputSize;                       /** Output image size after cropping. */
} ST_MOD_CROP_SETTING, *PST_MOD_CROP_SETTING;

typedef struct _ST_CROP_PARAM {
    ST_MOD_CROP_SETTING stSetting;              /** Module CROP setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program CROP module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
    ST_SIZE stInputSize;                        /** Input image size before cropping. */
} ST_CROP_PARAM, *PST_CROP_PARAM;

#define ST_MOD_CROP_SETTING_LEN         (sizeof(ST_MOD_CROP_SETTING))
#define ST_MOD_CROP_GET_PARAM_LEN       (ST_MOD_CROP_SETTING_LEN)
#define ST_MOD_CROP_SET_PARAM_LEN       (ST_MOD_CROP_SETTING_LEN)
#define ST_CROP_PARAM_SIZE              (sizeof(ST_CROP_PARAM))
#define ST_CROP_PARAM_LEN               (ST_CROP_PARAM_SIZE)

typedef struct _ST_ISP_MOD_CROP {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstCropRdmaBuf;               /** CROP's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_CROP_RDMA *pstIspCropRdma;           /** CROP's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_CROP_RDMA *pstCropRdma;                  /** CROP's RDMA structure pointer.*/
    ST_CROP_PARAM *pstCropParam;                /** CROP's parameters structure pointer. */
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
} ST_ISP_MOD_CROP, *PST_ISP_MOD_CROP;

#pragma pack(pop)


/* CSI cropping filter module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CROP_InitRdma(
    ST_CROP_RDMA *pstCropRdma,
    ST_CROP_PARAM *pstCropParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CROP_UpdateRdma(
    ST_CROP_RDMA *pstCropRdma,
    ST_CROP_PARAM *pstCropParam
    );
extern
STF_S32 STFMOD_ISP_CROP_UpdateOutputSize(
    ST_CROP_RDMA *pstCropRdma,
    ST_CROP_PARAM *pstCropParam
    );
extern
STF_S32 STFMOD_ISP_CROP_SetNextRdma(
    ST_CROP_RDMA *pstCropRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CROP_SetEnable(
    ST_ISP_MOD_CROP *pstModCrop,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CROP_SetUpdate(
    ST_ISP_MOD_CROP *pstModCrop,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_CROP_SetInputSize(
    ST_ISP_MOD_CROP *pstModCrop,
    ST_SIZE stInputSize
    );
extern
STF_S32 STFMOD_ISP_CROP_SetOutputSize(
    ST_ISP_MOD_CROP *pstModCrop,
    ST_SIZE stOutputSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CROP_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CROP_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CROP_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CROP_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_CROP_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CROP_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_CROP_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CROP_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_CROP_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CROP_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CROP_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CROP_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CROP_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CROP_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CROP_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CROP_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_CROP_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_CROP_H__
