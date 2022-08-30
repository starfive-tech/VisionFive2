/**
  ******************************************************************************
  * @file  stfmod_isp_csi_in.h
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


#ifndef __STFMOD_ISP_CSI_IN_H__
#define __STFMOD_ISP_CSI_IN_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_csi_in_rdma.h"
#include "stflib_isp_pipeline.h"


/* CSI input module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_CSI_IN_IMAGER_SEL               (EN_IMAGER_SEL_DVP)
#define ISP_DEF_CSI_IN_ITU656_EN                (STF_FALSE)
#define ISP_DEF_CSI_IN_POLARITY                 (EN_DVP_SYNC_POL_H_POS_V_NEG)
#define ISP_DEF_CSI_IN_FIRST_PIXEL              (EN_CSI_1ST_PXL_B)


#pragma pack(push, 1)

typedef enum _EN_CSI_IN_UPDATE {
    EN_CSI_IN_UPDATE_NONE = 0x00,
    EN_CSI_IN_UPDATE_INTERFACE = 0x01,
    EN_CSI_IN_UPDATE_FIRST_PIXEL = 0x02,
    EN_CSI_IN_UPDATE_LINE = 0x04,
    EN_CSI_IN_UPDATE_ALL = (EN_CSI_IN_UPDATE_INTERFACE
                          | EN_CSI_IN_UPDATE_FIRST_PIXEL
                          | EN_CSI_IN_UPDATE_LINE
                          ),
} EN_CSI_IN_UPDATE, *PEN_CSI_IN_UPDATE;


typedef struct _ST_MOD_CSI_IN_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CSI_IN module. */
    STF_BOOL8 bITU656;                          /** Enable/Disable ITU656 interface. */
    STF_U16 u16Line;                            /** Line no for CSI line interrupt. */
} ST_MOD_CSI_IN_SETTING, *PST_MOD_CSI_IN_SETTING;

typedef struct _ST_MOD_CSI_IN_INFO {
    STF_U8 u8ImagerSel;                         /** Imager select. This information should be get from sensor API. 0:DVP, 1:MIPI. */
    STF_U8 u8Polarity;                          /** Hsync and Vsync polarity. This information should be get from sensor API. 0:H_NEG_V_NEG, 1:H_NEG_V_POS, 2:H_POS_V_NEG, 3:H_POS_V_POS. */
    STF_U8 u8FirstPixel;                        /** First pixel for CSI domain. This information should be get from sensor API. 0:R, 1:Gr, 2:Gb, 3:B. */
} ST_MOD_CSI_IN_INFO, *PST_MOD_CSI_IN_INFO;

typedef struct _ST_CSI_IN_PARAM {
    ST_MOD_CSI_IN_SETTING stSetting;            /** Module CSI_IN setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_CSI_IN_INFO stInfo;                  /** Module CSI_IN output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program CSI_IN module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_CSI_IN_PARAM, *PST_CSI_IN_PARAM;

#define ST_MOD_CSI_IN_SETTING_LEN       (sizeof(ST_MOD_CSI_IN_SETTING))
#define ST_MOD_CSI_IN_INFO_LEN          (sizeof(ST_MOD_CSI_IN_INFO))
#define ST_MOD_CSI_IN_GET_PARAM_LEN     (ST_MOD_CSI_IN_SETTING_LEN + ST_MOD_CSI_IN_INFO_LEN)
#define ST_MOD_CSI_IN_SET_PARAM_LEN     (ST_MOD_CSI_IN_SETTING_LEN)
#define ST_CSI_IN_PARAM_SIZE            (sizeof(ST_CSI_IN_PARAM))
#define ST_CSI_IN_PARAM_LEN             (ST_CSI_IN_PARAM_SIZE)

typedef struct _ST_ISP_MOD_CSI_IN {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstCsiInRdmaBuf;              /** CSI_IN's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_CSI_IN_RDMA *pstIspCsiInRdma;        /** CSI_IN's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_CSI_IN_RDMA *pstCsiInRdma;               /** CSI_IN's RDMA structure pointer.*/
    ST_CSI_IN_PARAM *pstCsiInParam;             /** CSI_IN's parameters structure pointer. */
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
} ST_ISP_MOD_CSI_IN, *PST_ISP_MOD_CSI_IN;

#pragma pack(pop)


/* CSI input module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CSI_IN_InitRdma(
    ST_CSI_IN_RDMA *pstCsiInRdma,
    ST_CSI_IN_PARAM *pstCsiInParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_UpdateRdma(
    ST_CSI_IN_RDMA *pstCsiInRdma,
    ST_CSI_IN_PARAM *pstCsiInParam
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_UpdateInterface(
    ST_CSI_IN_RDMA *pstCsiInRdma,
    ST_CSI_IN_PARAM *pstCsiInParam
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_UpdateFirstPixel(
    ST_CSI_IN_RDMA *pstCsiInRdma,
    ST_CSI_IN_PARAM *pstCsiInParam
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetNextRdma(
    ST_CSI_IN_RDMA *pstCsiInRdma,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_UpdateLine(
    STF_VOID *pIspCtx,
    ST_CSI_IN_PARAM *pstCsiInParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CSI_IN_SetEnable(
    ST_ISP_MOD_CSI_IN *pstModCsiIn,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetUpdate(
    ST_ISP_MOD_CSI_IN *pstModCsiIn,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetImagerSelect(
    ST_ISP_MOD_CSI_IN *pstModCsiIn,
    STF_U8 u8ImagerSel
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetITU656Enable(
    ST_ISP_MOD_CSI_IN *pstModCsiIn,
    STF_BOOL8 bITU656
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetPolarity(
    ST_ISP_MOD_CSI_IN *pstModCsiIn,
    STF_U8 u8Polarity
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetFirstPixel(
    ST_ISP_MOD_CSI_IN *pstModCsiIn,
    STF_U8 u8FirstPixel
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_GetFirstPixel(
    STF_VOID *pIspCtx,
    STF_U8 *pu8FirstPixel
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetLine(
    STF_VOID *pIspCtx,
    STF_U16 u16Line
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CSI_IN_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_CSI_IN_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_CSI_IN_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_CSI_IN_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CSI_IN_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_CSI_IN_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_CSI_IN_H__
