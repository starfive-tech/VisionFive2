/**
  ******************************************************************************
  * @file  stfmod_isp_oecfhm.h
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


#ifndef __STFMOD_ISP_OECFHM_H__
#define __STFMOD_ISP_OECFHM_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_oecfhm_rdma.h"
#include "stflib_isp_pipeline.h"


/* oecf module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define OECFHM_INPUT_BITS                       (12)
#define OECFHM_INPUT_VALUE_MAX                  ((1 << OECFHM_INPUT_BITS) - 1)
#define OECFHM_SLOPE_SIGNED_BITS                (1)
#define OECFHM_SLOPE_INTEGER_BITS               (4)
#define OECFHM_SLOPE_FRACTION_BITS              (11)
#define OECFHM_CURVE_POINT_MAX                  (5)


#pragma pack(push, 1)

typedef enum _EN_OECFHM_UPDATE {
    EN_OECFHM_UPDATE_NONE = 0x00,
    EN_OECFHM_UPDATE_INPUT = 0x01,
    EN_OECFHM_UPDATE_SLOPE = 0x02,
    EN_OECFHM_UPDATE_ALL = (EN_OECFHM_UPDATE_INPUT
                          | EN_OECFHM_UPDATE_SLOPE
                          ),
} EN_OECFHM_UPDATE, *PEN_OECFHM_UPDATE;


typedef struct _ST_OECFHM_INPUT {
    STF_U16 u16X[OECFHM_CURVE_POINT_MAX];
} ST_OECFHM_INPUT, *PST_OECFHM_INPUT;

typedef struct _ST_OECFHM_SLOPE {
    STF_U16 u16S[OECFHM_CURVE_POINT_MAX];
} ST_OECFHM_SLOPE, *PST_OECFHM_SLOPE;

typedef struct _ST_MOD_OECFHM_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable OECFHM module. */
    ST_OECFHM_INPUT stInput;                    /** Y channel input. */
    ST_OECFHM_SLOPE stSlope;                    /** Y channel slope. */
} ST_MOD_OECFHM_SETTING, *PST_MOD_OECFHM_SETTING;

typedef struct _ST_OECFHM_PARAM {
    ST_MOD_OECFHM_SETTING stSetting;            /** Module OECFHM setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program OECFHM module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_OECFHM_PARAM, *PST_OECFHM_PARAM;

#define ST_MOD_OECFHM_SETTING_LEN       (sizeof(ST_MOD_OECFHM_SETTING))
#define ST_MOD_OECFHM_GET_PARAM_LEN     (ST_MOD_OECFHM_SETTING_LEN)
#define ST_MOD_OECFHM_SET_PARAM_LEN     (ST_MOD_OECFHM_SETTING_LEN)
#define ST_OECFHM_PARAM_SIZE            (sizeof(ST_OECFHM_PARAM))
#define ST_OECFHM_PARAM_LEN             (ST_OECFHM_PARAM_SIZE)

typedef struct _ST_ISP_MOD_OECFHM {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstOecfHmRdmaBuf;             /** OECFHM's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_OECFHM_RDMA *pstIspOecfHmRdma;       /** OECFHM's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_OECFHM_RDMA *pstOecfHmRdma;              /** OECFHM's RDMA structure pointer.*/
    ST_OECFHM_PARAM *pstOecfHmParam;            /** OECFHM's parameters structure pointer. */
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
} ST_ISP_MOD_OECFHM, *PST_ISP_MOD_OECFHM;

#pragma pack(pop)


/* oecf module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECFHM_InitRdma(
    ST_OECFHM_RDMA *pstOecfHmRdma,
    ST_OECFHM_PARAM *pstOecfHmParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OECFHM_UpdateRdma(
    ST_OECFHM_RDMA *pstOecfHmRdma,
    ST_OECFHM_PARAM *pstOecfHmParam
    );
extern
STF_S32 STFMOD_ISP_OECFHM_SetNextRdma(
    ST_OECFHM_RDMA *pstOecfHmRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECFHM_SetEnable(
    ST_ISP_MOD_OECFHM *pstModOecfHm,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_OECFHM_SetUpdate(
    ST_ISP_MOD_OECFHM *pstModOecfHm,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_OECFHM_SetInput(
    ST_ISP_MOD_OECFHM *pstModOecfHm,
    ST_OECFHM_INPUT *pstInput
    );
extern
STF_S32 STFMOD_ISP_OECFHM_SetSlope(
    ST_ISP_MOD_OECFHM *pstModOecfHm,
    ST_OECFHM_SLOPE *pstSlope
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECFHM_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OECFHM_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OECFHM_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECFHM_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_OECFHM_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECFHM_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_OECFHM_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECFHM_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_OECFHM_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECFHM_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECFHM_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECFHM_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_OECFHM_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_OECFHM_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_OECFHM_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_OECFHM_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_OECFHM_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_OECFHM_H__
