/**
  ******************************************************************************
  * @file  stfmod_isp_awb.h
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


#ifndef __STFMOD_ISP_AWB_H__
#define __STFMOD_ISP_AWB_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_awb_rdma.h"
#include "stflib_isp_pipeline.h"


/* auto white-balance module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
//#if 0
//#define ISP_DEF_AWB_RED_GAIN                    (1.5)
//#define ISP_DEF_AWB_BLUE_GAIN                   (1.5)
//#else
//#define ISP_DEF_AWB_RED_GAIN                    (1.66)      // D65 white-balance red gain. */
//#define ISP_DEF_AWB_BLUE_GAIN                   (1.32)      // D65 white-balance blue gain. */
//#endif
#define ISP_DEF_AWB_DIGITAL_GAIN                (1.0)


#pragma pack(push, 1)

typedef enum _EN_AWB_UPDATE {
    EN_AWB_UPDATE_NONE = 0x00,
    EN_AWB_UPDATE_R_B_GAIN = 0x01,
    EN_AWB_UPDATE_ALL = (EN_AWB_UPDATE_R_B_GAIN),
} EN_AWB_UPDATE, *PEN_AWB_UPDATE;


typedef struct _ST_MOD_AWB_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable AWB gain module. */
} ST_MOD_AWB_SETTING, *PST_MOD_AWB_SETTING;

typedef struct _ST_MOD_AWB_INFO {
    STF_DOUBLE dRedGain;                        /** AWB R channel gain. */
    STF_DOUBLE dBlueGain;                       /** AWB B channel gain. */
    STF_DOUBLE dDigitalGain;                    /** Digital gain. */
} ST_MOD_AWB_INFO, *PST_MOD_AWB_INFO;

typedef struct _ST_AWB_PARAM {
    ST_MOD_AWB_SETTING stSetting;               /** Module AWB setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_AWB_INFO stInfo;                     /** Module AWB output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program AWB module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_AWB_PARAM, *PST_AWB_PARAM;

#define ST_MOD_AWB_SETTING_LEN          (sizeof(ST_MOD_AWB_SETTING))
#define ST_MOD_AWB_INFO_LEN             (sizeof(ST_MOD_AWB_INFO))
#define ST_MOD_AWB_GET_PARAM_LEN        (ST_MOD_AWB_SETTING_LEN + ST_MOD_AWB_INFO_LEN)
#define ST_MOD_AWB_SET_PARAM_LEN        (ST_MOD_AWB_SETTING_LEN)
#define ST_AWB_PARAM_SIZE               (sizeof(ST_AWB_PARAM))
#define ST_AWB_PARAM_LEN                (ST_AWB_PARAM_SIZE)

typedef struct _ST_ISP_MOD_AWB {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstAwbRdmaBuf;                /** AWB's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_AWB_RDMA *pstIspAwbRdma;             /** AWB's RDMA structure pointer that include the ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_AWB_RDMA *pstAwbRdma;                    /** AWB's RDMA structure pointer.*/
    ST_AWB_PARAM *pstAwbParam;                  /** AWB's parameters structure pointer. */
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
} ST_ISP_MOD_AWB, *PST_ISP_MOD_AWB;

#pragma pack(pop)


/* auto white-balance module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_AWB_InitRdma(
    ST_AWB_RDMA *pstAwbRdma,
    ST_AWB_PARAM *pstAwbParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_AWB_UpdateRdma(
    ST_AWB_RDMA *pstAwbRdma,
    ST_AWB_PARAM *pstAwbParam
    );
extern
STF_S32 STFMOD_ISP_AWB_SetNextRdma(
    ST_AWB_RDMA *pstAwbRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_AWB_SetEnable(
    ST_ISP_MOD_AWB *pstModAwb,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_AWB_SetUpdate(
    ST_ISP_MOD_AWB *pstModAwb,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_AWB_SetAwbGain(
    STF_VOID *pIspCtx,
    STF_DOUBLE dRedGain,
    STF_DOUBLE dBlueGain
    );
#if defined(ISP_ADC_GAIN_USING_AWB_SIM)
extern
STF_S32 STFMOD_ISP_AWB_SetDigitalGain(
    STF_VOID *pIspCtx,
    STF_DOUBLE dDigitalGain
    );
#endif //ISP_ADC_GAIN_USING_AWB_SIM
extern
STF_S32 STFMOD_ISP_AWB_GetAwbGain(
    STF_VOID *pIspCtx,
    STF_DOUBLE *pdRedGain,
    STF_DOUBLE *pdBlueGain
    );
#if defined(ISP_ADC_GAIN_USING_AWB_SIM)
extern
STF_S32 STFMOD_ISP_AWB_GetDigitalGain(
    STF_VOID *pIspCtx,
    STF_DOUBLE *pdDigitalGain
    );
#endif //ISP_ADC_GAIN_USING_AWB_SIM
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_AWB_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_AWB_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_AWB_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_AWB_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_AWB_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_AWB_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_AWB_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_AWB_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_AWB_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_AWB_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_AWB_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_AWB_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_AWB_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_AWB_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_AWB_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_AWB_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_AWB_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_AWB_H__
