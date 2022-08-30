/**
  ******************************************************************************
  * @file  stfmod_isp_sat.h
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


#ifndef __STFMOD_ISP_SAT_H__
#define __STFMOD_ISP_SAT_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_sat_rdma.h"
#include "stflib_isp_pipeline.h"


/* Brightness, contrast, hue and saturation module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define SAT_CRV_PNT_MAX                         (2)


#if 0
#define ISP_DEF_SAT_Y_CURVE_INPUT_1             (0)             /** Input level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_INPUT_2             (1023)          /** Input level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_1            (0)             /** Output level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_2            (1023)          /** Output level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_HUE_DEGREE                  (0.0)           /** Hue degree. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_1       (32)            /** Threshold 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_2       (64)            /** Threshold 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_1          (1.2)           /** Factor 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_2          (1.5)           /** Factor 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_U_OFFSET                (0)             /** U channel offset for saturation. */
#define ISP_DEF_SAT_SAT_V_OFFSET                (0)             /** V channel offset for saturation. */
#define ISP_DEF_SAT_SAT_SLOPE_SCALE_FACTOR_DNRM (0)             /** Chroma Magnitude Scaling Factor (2^DNRM). */
#elif 0
// 20201216_v01
#define ISP_DEF_SAT_Y_CURVE_INPUT_1             (0)             /** Input level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_INPUT_2             (1023)          /** Input level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_1            (0)             /** Output level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_2            (1023)          /** Output level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_HUE_DEGREE                  (0.0)           /** Hue degree. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_1       (0)             /** Threshold 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_2       (32)            /** Threshold 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_1          (1.5)           /** Factor 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_2          (1.5)           /** Factor 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_U_OFFSET                (0)             /** U channel offset for saturation. */
#define ISP_DEF_SAT_SAT_V_OFFSET                (0)             /** V channel offset for saturation. */
#define ISP_DEF_SAT_SAT_SLOPE_SCALE_FACTOR_DNRM (8)             /** Chroma Magnitude Scaling Factor (2^DNRM). */
#elif 0
// 20201219_v01
#define ISP_DEF_SAT_Y_CURVE_INPUT_1             (1)             /** Input level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_INPUT_2             (1022)          /** Input level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_1            (0)             /** Output level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_2            (1023)          /** Output level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_HUE_DEGREE                  (0.0)           /** Hue degree. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_1       (1)             /** Threshold 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_2       (32)            /** Threshold 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_1          (2.0)           /** Factor 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_2          (2.0)           /** Factor 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_U_OFFSET                (0)             /** U channel offset for saturation. */
#define ISP_DEF_SAT_SAT_V_OFFSET                (0)             /** V channel offset for saturation. */
#define ISP_DEF_SAT_SAT_SLOPE_SCALE_FACTOR_DNRM (8)             /** Chroma Magnitude Scaling Factor (2^DNRM). */
#elif 0
// 20201229_v01
#define ISP_DEF_SAT_Y_CURVE_INPUT_1             (1)             /** Input level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_INPUT_2             (1022)          /** Input level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_1            (0)             /** Output level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_2            (1023)          /** Output level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_HUE_DEGREE                  (0.0)           /** Hue degree. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_1       (1)             /** Threshold 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_2       (32)            /** Threshold 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_1          (1.8)           /** Factor 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_2          (1.8)           /** Factor 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_U_OFFSET                (0)             /** U channel offset for saturation. */
#define ISP_DEF_SAT_SAT_V_OFFSET                (0)             /** V channel offset for saturation. */
#define ISP_DEF_SAT_SAT_SLOPE_SCALE_FACTOR_DNRM (8)             /** Chroma Magnitude Scaling Factor (2^DNRM). */
#elif 0
// 20210104_v01
#define ISP_DEF_SAT_Y_CURVE_INPUT_1             (1)             /** Input level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_INPUT_2             (1022)          /** Input level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_1            (0)             /** Output level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_2            (1023)          /** Output level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_HUE_DEGREE                  (0.0)           /** Hue degree. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_1       (1)             /** Threshold 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_2       (32)            /** Threshold 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_1          (1.8)           /** Factor 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_2          (1.8)           /** Factor 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_U_OFFSET                (0)             /** U channel offset for saturation. */
#define ISP_DEF_SAT_SAT_V_OFFSET                (0)             /** V channel offset for saturation. */
#define ISP_DEF_SAT_SAT_SLOPE_SCALE_FACTOR_DNRM (8)             /** Chroma Magnitude Scaling Factor (2^DNRM). */
#elif 1
// 20210517_v01 for PC-Cam
#define ISP_DEF_SAT_Y_CURVE_INPUT_1             (1)             /** Input level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_INPUT_2             (1022)          /** Input level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_1            (0)             /** Output level 1 value for brightness and contrast. */
#define ISP_DEF_SAT_Y_CURVE_OUTPUT_2            (1023)          /** Output level 2 value for brightness and contrast. */
#define ISP_DEF_SAT_HUE_DEGREE                  (0.0)           /** Hue degree. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_1       (1)             /** Threshold 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_THRESHOLD_2       (32)            /** Threshold 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_1          (1.62)          /** Factor 1 of saturation curve. */
#define ISP_DEF_SAT_SAT_CURVE_FACTOR_2          (1.62)          /** Factor 2 of saturation curve. */
#define ISP_DEF_SAT_SAT_U_OFFSET                (0)             /** U channel offset for saturation. */
#define ISP_DEF_SAT_SAT_V_OFFSET                (0)             /** V channel offset for saturation. */
#define ISP_DEF_SAT_SAT_SLOPE_SCALE_FACTOR_DNRM (8)             /** Chroma Magnitude Scaling Factor (2^DNRM). */
#endif


#pragma pack(push, 1)

typedef enum _EN_SAT_UPDATE {
    EN_SAT_UPDATE_NONE = 0x00,
    EN_SAT_UPDATE_Y_CURVE_INFO = 0X01,
    EN_SAT_UPDATE_HUE_INFO = 0X02,
    EN_SAT_UPDATE_SATURATION_INFO = 0X04,
    EN_SAT_UPDATE_ALL = (EN_SAT_UPDATE_Y_CURVE_INFO
                       | EN_SAT_UPDATE_HUE_INFO
                       | EN_SAT_UPDATE_SATURATION_INFO
                       ),
} EN_SAT_UPDATE, *PEN_SAT_UPDATE;


typedef struct _ST_SAT_PNT {
    STF_U16 u16Threshold;                       /** Chroma magnitude amplification threshold, range from 0 to 2047. */
    STF_DOUBLE dFactor;                         /** Chroma magnitude amplification factor, range from 0 to 7.99609375. */
} ST_SAT_PNT, *PST_SAT_PNT;

typedef struct _ST_SAT_CRV {
    ST_SAT_PNT stPoint[2];                      /** Chroma magnitude amplification curve. */
} ST_SAT_CRV, *PST_SAT_CRV;

typedef struct _ST_YCRV_PNT {
    STF_U16 u16YInput;                          /** Y channel mapping curve input, range form 0 to 1023. */
    STF_U16 u16YOutput;                         /** Y channel mapping curve output, range form 0 to 1023. */
} ST_YCRV_PNT, *PST_YCRV_PNT;

typedef struct _ST_HUE_INFO {
    STF_DOUBLE dDegree;                         /** Hue degree value, range form 0 to 90 degree. */
} ST_HUE_INFO, *PST_HUE_INFO;

typedef struct _ST_SAT_INFO {
    ST_SAT_CRV stSatCurve;                      /** Chroma magnitude amplification curve. */
    STF_U8 u8DeNrml;                            /** Chroma magnitude amplification slope scale down factor (2^-u8DNrm), range from 0 to 15. */
    STF_S16 s16UOffset;                         /** Chroma saturation U offset, range from -1024 to 1023. */
    STF_S16 s16VOffset;                         /** Chroma saturation U offset, range from -1024 to 1023. */
} ST_SAT_INFO, *PST_SAT_INFO;

typedef struct _ST_MOD_SAT_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable SAT module. */
    ST_YCRV_PNT stYCurve[2];                    /** Brightness and contrast adjust parameters. */
    ST_HUE_INFO stHueInfo;                      /** Hue adjust parameters. */
    ST_SAT_INFO stSatInfo;                      /** Saturation adjust parameters. */
} ST_MOD_SAT_SETTING, *PST_MOD_SAT_SETTING;

typedef struct _ST_SAT_PARAM {
    ST_MOD_SAT_SETTING stSetting;               /** Module SAT setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program SAT module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_SAT_PARAM, *PST_SAT_PARAM;

#define ST_MOD_SAT_SETTING_LEN          (sizeof(ST_MOD_SAT_SETTING))
#define ST_MOD_SAT_GET_PARAM_LEN        (ST_MOD_SAT_SETTING_LEN)
#define ST_MOD_SAT_SET_PARAM_LEN        (ST_MOD_SAT_SETTING_LEN)
#define ST_SAT_PARAM_SIZE               (sizeof(ST_SAT_PARAM))
#define ST_SAT_PARAM_LEN                (ST_SAT_PARAM_SIZE)

typedef struct _ST_ISP_MOD_SAT {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstSatRdmaBuf;                /** SAT's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_SAT_RDMA *pstIspSatRdma;             /** SAT's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_SAT_RDMA *pstSatRdma;                    /** SAT's RDMA structure pointer.*/
    ST_SAT_PARAM *pstSatParam;                  /** SAT's parameters structure pointer. */
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
} ST_ISP_MOD_SAT, *PST_ISP_MOD_SAT;

#pragma pack(pop)


/* Brightness, contrast, hue and saturation module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SAT_InitRdma(
    ST_SAT_RDMA *pstSatRdma,
    ST_SAT_PARAM *pstSatParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SAT_UpdateRdma(
    ST_SAT_RDMA *pstSatRdma,
    ST_SAT_PARAM *pstSatParam
    );
extern
STF_S32 STFMOD_ISP_SAT_UpdateYCurve(
    ST_SAT_RDMA *pstSatRdma,
    ST_SAT_PARAM *pstSatParam
    );
extern
STF_S32 STFMOD_ISP_SAT_UpdateHueInfo(
    ST_SAT_RDMA *pstSatRdma,
    ST_SAT_PARAM *pstSatParam
    );
extern
STF_S32 STFMOD_ISP_SAT_UpdateSaturationInfo(
    ST_SAT_RDMA *pstSatRdma,
    ST_SAT_PARAM *pstSatParam
    );
extern
STF_S32 STFMOD_ISP_SAT_SetNextRdma(
    ST_SAT_RDMA *pstSatRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SAT_SetEnable(
    ST_ISP_MOD_SAT *pstModSat,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_SAT_SetUpdate(
    ST_ISP_MOD_SAT *pstModSat,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_SAT_SetYCurve(
    ST_ISP_MOD_SAT *pstModSat,
    ST_YCRV_PNT stYCurve[2]
    );
extern
STF_S32 STFMOD_ISP_SAT_SetHueInfo(
    ST_ISP_MOD_SAT *pstModSat,
    ST_HUE_INFO *pstHueInfo
    );
extern
STF_S32 STFMOD_ISP_SAT_SetSaturationInfo(
    ST_ISP_MOD_SAT *pstModSat,
    ST_SAT_INFO *pstSatInfo
    );
extern
STF_S32 STFMOD_ISP_SAT_SetSaturationCurve(
    ST_ISP_MOD_SAT *pstModSat,
    ST_SAT_CRV *pstSatCurve
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SAT_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SAT_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SAT_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SAT_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_SAT_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SAT_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_SAT_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SAT_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_SAT_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SAT_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SAT_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SAT_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SAT_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SAT_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SAT_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SAT_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_SAT_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_SAT_H__
