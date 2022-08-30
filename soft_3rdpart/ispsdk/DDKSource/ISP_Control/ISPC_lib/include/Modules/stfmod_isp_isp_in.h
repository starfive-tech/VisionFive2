/**
  ******************************************************************************
  * @file  stfmod_isp_isp_in.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/24/2021
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


#ifndef __STFMOD_ISP_ISP_IN_H__
#define __STFMOD_ISP_ISP_IN_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_isp_in_rdma.h"
#include "stflib_isp_pipeline.h"


/* ISP input module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_ISP_IN_CAPTURE_SIZE_W           (1920)
#define ISP_DEF_ISP_IN_CAPTURE_SIZE_H           (1080)
#define ISP_DEF_ISP_IN_MULTIPLE_FRAME           (STF_TRUE)
#define ISP_DEF_ISP_IN_FIRST_PIXEL              (EN_ISP_1ST_PXL_B)


#pragma pack(push, 1)

typedef enum _EN_ISP_IN_UPDATE {
    EN_ISP_IN_UPDATE_NONE = 0x00,
    EN_ISP_IN_UPDATE_MULTIPLE_FRAME = 0x01,
    EN_ISP_IN_UPDATE_FIRST_PIXEL = 0x02,
    EN_ISP_IN_UPDATE_CAPTURE_SIZE = 0x04,
    EN_ISP_IN_UPDATE_ALL = (EN_ISP_IN_UPDATE_MULTIPLE_FRAME
                          | EN_ISP_IN_UPDATE_FIRST_PIXEL
                          | EN_ISP_IN_UPDATE_CAPTURE_SIZE
                          ),
} EN_ISP_IN_UPDATE, *PEN_ISP_IN_UPDATE;


typedef struct _ST_MOD_ISP_IN_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable ISP_IN module. */
    STF_BOOL8 bMultipleFrame;                   /** Enable/Disable continuous process image mode. */
    ST_SIZE stCaptureSize;                      /** Capture size of pipeline of ISP. */
} ST_MOD_ISP_IN_SETTING, *PST_MOD_ISP_IN_SETTING;

typedef struct _ST_MOD_ISP_IN_INFO {
    STF_U8 u8FirstPixel;                        /** First pixel for ISP domain. This information should be get from CSI_IN API. 0:R, 1:B, 2:Gr, 3:Gb. */
} ST_MOD_ISP_IN_INFO, *PST_MOD_ISP_IN_INFO;

typedef struct _ST_ISP_IN_PARAM {
    ST_MOD_ISP_IN_SETTING stSetting;            /** Module ISP_IN setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_ISP_IN_INFO stInfo;                  /** Module ISP_IN output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program ISP_IN module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_ISP_IN_PARAM, *PST_ISP_IN_PARAM;

#define ST_MOD_ISP_IN_SETTING_LEN       (sizeof(ST_MOD_ISP_IN_SETTING))
#define ST_MOD_ISP_IN_INFO_LEN          (sizeof(ST_MOD_ISP_IN_INFO))
#define ST_MOD_ISP_IN_GET_PARAM_LEN     (ST_MOD_ISP_IN_SETTING_LEN + ST_MOD_ISP_IN_INFO_LEN)
#define ST_MOD_ISP_IN_SET_PARAM_LEN     (ST_MOD_ISP_IN_SETTING_LEN)
#define ST_ISP_IN_PARAM_SIZE            (sizeof(ST_ISP_IN_PARAM))
#define ST_ISP_IN_PARAM_LEN             (ST_ISP_IN_PARAM_SIZE)

typedef struct _ST_ISP_MOD_ISP_IN {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstIspInRdmaBuf;              /** ISP_IN's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_ISP_IN_RDMA *pstIspIspInRdma;        /** ISP_IN's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_ISP_IN_RDMA *pstIspInRdma;               /** ISP_IN's RDMA structure pointer.*/
    ST_ISP_IN_PARAM *pstIspInParam;             /** ISP_IN's parameters structure pointer. */
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
} ST_ISP_MOD_ISP_IN, *PST_ISP_MOD_ISP_IN;

#pragma pack(pop)


/* ISP input module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_ISP_IN_InitRdma(
    ST_ISP_IN_RDMA *pstIspInRdma,
    ST_ISP_IN_PARAM *pstIspInParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_UpdateRdma(
    ST_ISP_IN_RDMA *pstIspInRdma,
    ST_ISP_IN_PARAM *pstIspInParam
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetNextRdma(
    ST_ISP_IN_RDMA *pstIspInRdma,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_UpdateAll(
    STF_VOID *pIspCtx,
    ST_ISP_IN_PARAM *pstIspInParam
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_UpdateMultipleFrame(
    STF_VOID *pIspCtx,
    ST_ISP_IN_PARAM *pstIspInParam
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_UpdateFirstPixel(
    STF_VOID *pIspCtx,
    ST_ISP_IN_PARAM *pstIspInParam
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_UpdateCaptureSize(
    STF_VOID *pIspCtx,
    ST_ISP_IN_PARAM *pstIspInParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_ISP_IN_SetEnable(
    ST_ISP_MOD_ISP_IN *pstModIspIn,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetUpdate(
    ST_ISP_MOD_ISP_IN *pstModIspIn,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetMultipleFrame(
    ST_ISP_MOD_ISP_IN *pstModIspIn,
    STF_BOOL8 bMultipleFrame
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetFirstPixel(
    ST_ISP_MOD_ISP_IN *pstModIspIn,
    STF_U8 u8FirstPixel
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetCaptureSize(
    ST_ISP_MOD_ISP_IN *pstModIspIn,
    ST_SIZE stCaptureSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_ISP_IN_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_ISP_IN_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_ISP_IN_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_ISP_IN_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_ISP_IN_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_ISP_IN_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_ISP_IN_H__
