/**
  ******************************************************************************
  * @file  stfctl_isp_awb.h
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


#ifndef __STFCTL_ISP_AWB_H__
#define __STFCTL_ISP_AWB_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "Modules/stfmod_isp_awb.h"


/* auto white-balance control parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define AWB_WEIGHTING_SUM_W_OFFSET              (0x0200)
#define AWB_WEIGHTING_SUM_RW_OFFSET             (0x0204)
#define AWB_WEIGHTING_SUM_GW_OFFSET             (0x0280)
#define AWB_WEIGHTING_SUM_BW_OFFSET             (0x0284)

#if 0
#define ISP_DEF_AWB_UPDATE_SPEED                (0.25)
#elif 0
#define ISP_DEF_AWB_UPDATE_SPEED                (0.125)
#elif 1
#define ISP_DEF_AWB_UPDATE_SPEED                (0.0625)
#endif
#define ISP_DEF_PROJECTION_GAIN_WEIGHTING       (0.0)


#pragma pack(push, 1)

typedef struct _ST_SC_AWB_WS_INFO {
    STF_U32 u32WAccum;                          /** SC AWB weighting summation. */
    STF_U32 u32RWAccum;                         /** SC AWB weighting summation for R channel. */
    STF_U32 u32GWAccum;                         /** SC AWB weighting summation for Gr and Gb channel. */
    STF_U32 u32BWAccum;                         /** SC AWB weighting summation for B channel. */
} ST_SC_AWB_WS_INFO, *PST_SC_AWB_WS_INFO;

typedef struct _ST_AWB_TEMP {
    STF_U16 u16Temperature;                     /** Light temperature degree. */
    STF_DOUBLE dRedGain;                        /** AWB R channel gain. */
    STF_DOUBLE dBlueGain;                       /** AWB B channel gain. */
} ST_AWB_TEMP, *PST_AWB_TEMP;

typedef struct _ST_AWB_TEMP_TBL {
    STF_U8 u8AwbTempTblCnt;                     /** Indicate light temperature table counter. */
    ST_AWB_TEMP stAwbTemp[TEMP_LEVEL_MAX];      /** AWB light temperature and white-balance gain table. */
} ST_AWB_TEMP_TBL, *PST_AWB_TEMP_TBL;

typedef struct _ST_CTL_AWB_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable AWB gain control. */
    STF_DOUBLE dUpdateSpeed;                    /** White balance gain change speed parameters. */
    ST_AWB_TEMP_TBL stAwbTempTbl;               /** AWB temperature information. */
} ST_CTL_AWB_SETTING, *PST_CTL_AWB_SETTING;

typedef struct _ST_CTL_AWB_INFO {
    STF_DOUBLE dEvalRedGain;                    /** SC evaluation red gain value. */
    STF_DOUBLE dEvalBlueGain;                   /** SC evaluation blue gain value. */
    STF_DOUBLE dExpectRedGain;                  /** AWB next balance R gain. */
    STF_DOUBLE dExpectBlueGain;                 /** AWB next balance B gain. */
    STF_U16 u16MeasureTemperature;              /** AWB measure temperature degree. */
} ST_CTL_AWB_INFO, *PST_CTL_AWB_INFO;

typedef struct _ST_CTL_AWB_PARAM {
    ST_CTL_AWB_SETTING stSetting;               /** Control AWB setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_CTL_AWB_INFO stInfo;                     /** Control AWB output information parameters. */
    STF_DOUBLE dProjectionGainWeighting;        /** AWB calibration curve segment projection gain weighting. */
    ST_SC_AWB_WS_INFO stScAwbWsInfo;            /** SC AWB weighting summation information. */
    STF_U8 u8GainCnt;                           /** Indicate the AWB gain input counter for first time special process. */
    ST_TEMP_INFO stTemperatureInfo;             /** Measure temperature information. */
    STF_DOUBLE dHistRedGain;                    /** AWB historical balance R gain. */
    STF_DOUBLE dHistBlueGain;                   /** AWB historical balance R gain. */
    //STF_U16 u16TargetTemperature;               /** The target light temperature. */
} ST_CTL_AWB_PARAM, *PST_CTL_AWB_PARAM;

#define ST_CTL_AWB_SETTING_LEN          (sizeof(ST_CTL_AWB_SETTING))
#define ST_CTL_AWB_INFO_LEN             (sizeof(ST_CTL_AWB_INFO))
#define ST_CTL_AWB_GET_PARAM_LEN        (ST_CTL_AWB_SETTING_LEN + ST_CTL_AWB_INFO_LEN)
#define ST_CTL_AWB_SET_PARAM_LEN        (ST_CTL_AWB_SETTING_LEN)
#define ST_CTL_AWB_PARAM_SIZE           (sizeof(ST_CTL_AWB_PARAM))
#define ST_CTL_AWB_PARAM_LEN            (ST_CTL_AWB_PARAM_SIZE)

typedef struct _ST_ISP_CTL_AWB {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Control ID. */
    STF_CHAR szName[16];                        /** Control Name. */
    //STF_VOID *pstCtlAwbRdma;                    /** AWB's control RDMA structure pointer. */
    ST_CTL_AWB_PARAM *pstCtlAwbParam;           /** AWB's control parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Control register in pipeline. */
    STF_S32 (*Init)(STF_VOID *pIspCtx);
    STF_S32 (*LoadBinParam)(STF_VOID *pIspCtx, STF_U8 *pu8ParamBuf, STF_U16 u16ParamSize);
    STF_S32 (*Enable)(STF_VOID *pIspCtx, STF_BOOL8 bEnable);
    STF_BOOL8 (*IsEnable)(STF_VOID *pIspCtx);
    STF_S32 (*Registered)(STF_VOID *pIspCtx, STF_BOOL8 bRegister);
    STF_BOOL8 (*IsRegistered)(STF_VOID *pIspCtx);
    STF_S32 (*Calculate)(STF_VOID *pIspCtx);
    STF_S32 (*FreeResource)(STF_VOID *pIspCtx);
    STF_S32 (*GetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);
    STF_S32 (*SetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 u16ParamSize);
} ST_ISP_CTL_AWB, *PST_ISP_CTL_AWB;

#pragma pack(pop)


/* auto white-balance control parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AWB_EvalScAwbGains(
    STF_U32 *pu32ScDumpBuf,
    ST_SC_AWB_WS_INFO *pstScAwbWsInfo,
    STF_DOUBLE *pdEvalRedGain,
    STF_DOUBLE *pdEvalBlueGain
    );
extern
STF_S32 STFCTL_ISP_AWB_CalcPointsDistanceSqr(
    STF_DOUBLE *pdDist,
    STF_DOUBLE dX1,
    STF_DOUBLE dY1,
    STF_DOUBLE dX2,
    STF_DOUBLE dY2
    );
extern
STF_S32 STFCTL_ISP_AWB_CalcSegmentProjection(
    STF_DOUBLE *pdIX,
    STF_DOUBLE *pdIY,
    STF_DOUBLE *pdScale,
    STF_DOUBLE dPX,
    STF_DOUBLE dPY,
    STF_DOUBLE dX1,
    STF_DOUBLE dY1,
    STF_DOUBLE dX2,
    STF_DOUBLE dY2
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AWB_CalcAwbTargetGains(
    ST_ISP_CTL_AWB *pstCtlAwb,
    STF_DOUBLE *pdExpectRedGain,
    STF_DOUBLE *pdExpectBlueGain,
    STF_DOUBLE dScRedGain,
    STF_DOUBLE dScBlueGain
    );
extern
STF_S32 STFCTL_ISP_AWB_CalcAwbTemperature(
    STF_VOID *pIspCtx,
    ST_TEMP_INFO *pstTemperatureInfo,
    STF_DOUBLE *pdRGainProjection,
    STF_DOUBLE *pdBGainProjection,
    STF_DOUBLE dRGain,
    STF_DOUBLE dBGain,
    ST_AWB_TEMP_TBL *pstAwbTempTbl
    );
extern
STF_S32 STFCTL_ISP_AWB_SetAwbGain(
    STF_VOID *pIspCtx,
    STF_DOUBLE dExpRedGain,
    STF_DOUBLE dExpBlueGain
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AWB_SetEnable(
    ST_ISP_CTL_AWB *pstCtlAwb,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFCTL_ISP_AWB_ResetGainCount(
    ST_ISP_CTL_AWB *pstCtlAwb
    );
extern
STF_S32 STFCTL_ISP_AWB_SetAwbUpdateSpeed(
    ST_ISP_CTL_AWB *pstCtlAwb,
    STF_DOUBLE dUpdateSpeed
    );
extern
STF_S32 STFCTL_ISP_AWB_SetProjectionGainWeighting(
    ST_ISP_CTL_AWB *pstCtlAwb,
    STF_DOUBLE dProjectionGainWeighting
    );
extern
STF_S32 STFCTL_ISP_AWB_ResetAwbTempTableCnt(
    ST_ISP_CTL_AWB *pstCtlAwb
    );
// New temperature > previous temperature has to be guaranteed.
extern
STF_S32 STFCTL_ISP_AWB_AddAwbTemperature(
    ST_ISP_CTL_AWB *pstCtlAwb,
    ST_AWB_TEMP *pstAwbTemp
    );
extern
STF_S32 STFCTL_ISP_AWB_GetAwbTempTbl(
    STF_VOID *pIspCtx,
    ST_AWB_TEMP_TBL **pstAwbTempTbl
    );
extern
STF_S32 STFCTL_ISP_AWB_GetAwbTemperatureInfo(
    STF_VOID *pIspCtx,
    ST_TEMP_INFO **pstTemperatureInfo
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AWB_Init(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AWB_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFCTL_ISP_AWB_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFCTL_ISP_AWB_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AWB_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegister
    );
extern
STF_BOOL8 STFCTL_ISP_AWB_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AWB_Calculate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AWB_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AWB_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFCTL_ISP_AWB_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AWB_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFCTL_ISP_AWB_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFCTL_ISP_AWB_H__
