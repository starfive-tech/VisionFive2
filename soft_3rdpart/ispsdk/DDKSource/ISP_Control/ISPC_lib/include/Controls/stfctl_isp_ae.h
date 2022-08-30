/**
  ******************************************************************************
  * @file  stfctl_isp_ae.h
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


#ifndef __STFCTL_ISP_AE_H__
#define __STFCTL_ISP_AE_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "ISPC/stflib_isp_sensor.h"


/* auto exposure control parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define AE_WEIGHT_SUM_Y_OFFSET                  (0x0084)
#define AE_HIST_R_OFFSET                        (0x4000)
#define AE_HIST_G_OFFSET                        (0x4004)
#define AE_HIST_B_OFFSET                        (0x4200)
#define AE_HIST_Y_OFFSET                        (0x4204)
#define AE_HIST_BIN_MAX                         (64)

#define AE_WT_TBL_COL_MAX                       (16)
#define AE_WT_TBL_ROW_MAX                       (16)
#define AE_WT_TBL_ITEM_MAX                      (AE_WT_TBL_ROW_MAX * AE_WT_TBL_COL_MAX)
#define EXPO_LOCK_ITEM_MAX                      (16)
#define DAMPING_BUFFER_SIZE                     (10)

#define SAT_WT_TBL_ITEM_MAX                     (32)

#if 0
#define ISP_DEF_AE_TARGET_B                     (80)
#elif 1
#define ISP_DEF_AE_TARGET_B                     (75)
#endif
#if 0
#define ISP_DEF_AT_TARGET_BRACKET               (255 * 0.125)   // roundup(255 * 0.125)
#elif 1
// 20201229_v01
#define ISP_DEF_AT_TARGET_BRACKET               (255 * 0.10)    // roundup(255 * 0.10)
#endif
#if 0
#define ISP_DEF_AE_OVER_SAT_COMP_T0             (0x04)
#define ISP_DEF_AE_OVER_SAT_COMP_T1             (0x30)
#define ISP_DEF_AE_OVER_SAT_COMP_MAX            (60)
#elif 0
// 20201229_v01
#define ISP_DEF_AE_OVER_SAT_COMP_T0             (255 * 0.0157)  // 4
#define ISP_DEF_AE_OVER_SAT_COMP_T1             (255 * 0.09)    // 23
#define ISP_DEF_AE_OVER_SAT_COMP_MAX            (40)
#elif 1
// 20210103_v01
#define ISP_DEF_AE_OVER_SAT_COMP_T0             (255 * 0.0157)  // 4
#define ISP_DEF_AE_OVER_SAT_COMP_T1             (255 * 0.07)    // 18
#define ISP_DEF_AE_OVER_SAT_COMP_MAX            (40)
#endif
#define ISP_DEF_AE_UNDER_SAT_COMP_T0            (0x08)
#define ISP_DEF_AE_UNDER_SAT_COMP_T1            (0x10)
#define ISP_DEF_AE_UNDER_SAT_COMP_MAX           (0)
#if 0
#define ISP_DEF_AE_GAIN_STEP                    (0x0180)
#elif 0
#define ISP_DEF_AE_GAIN_STEP                    (0x0130)
#else
#define ISP_DEF_AE_GAIN_STEP                    (0x0110)
#endif
#define ISP_DEF_AE_RISING_COARSE_THRESHOLD      (4.0)
#define ISP_DEF_AE_FALLING_COARSE_THRESHOLD     (2.0)
#define ISP_DEF_AE_COARSE_SPEED                 (0.5)
#define ISP_DEF_AE_COARSE_DEC_BASE              (0x100 * 1 / 2)
#define ISP_DEF_AE_INIT_EV                      (33333)
#if 0
#define ISP_DEF_AE_EXPOSURE_MAX                 (100000)
#elif 1
#define ISP_DEF_AE_EXPOSURE_MAX                 (200000)
#endif
#if 0
#define ISP_DEF_AE_GAIN_MAX                     (128.0)
#elif 0
// 20201229_v01
#define ISP_DEF_AE_GAIN_MAX                     (64.0)
#elif 1
// 20210104_v01
#define ISP_DEF_AE_GAIN_MAX                     (16.0)
#endif
#define ISP_DEF_EXPECT_ADC_MAX                  (2.0)
#define ISP_DEF_EXPO_LOCK_FREQ                  (0)
#define ISP_DEF_ISO_LEVEL_0                     (1.0)
#define ISP_DEF_ISO_LEVEL_1                     (2.0)
#define ISP_DEF_ISO_LEVEL_2                     (4.0)
#define ISP_DEF_ISO_LEVEL_3                     (8.0)
#define ISP_DEF_ISO_LEVEL_4                     (16.0)
#define ISP_DEF_ISO_LEVEL_5                     (32.0)
#define ISP_DEF_ISO_LEVEL_6                     (64.0)
#define ISP_DEF_ISO_LEVEL_7                     (128.0)
#define ISP_DEF_ISO_LEVEL_8                     (256.0)
#define ISP_DEF_ISO_LEVEL_9                     (512.0)


#pragma pack(push, 1)

typedef enum _EN_AE_STABLE_METHOD {
    EN_AE_STABLE_METHOD_SMALL_GAIN = 0,
    EN_AE_STABLE_METHOD_BRACKET,
    EN_AE_STABLE_METHOD_MAX
} EN_AE_STABLE_METHOD, *PEN_AE_STABLE_METHOD;

typedef enum _EN_EXPO_LOCK_FREQ {
    EN_EXPO_LOCK_FREQ_50 = 0,
    EN_EXPO_LOCK_FREQ_60,
    EN_EXPO_LOCK_FREQ_MAX
} EN_EXPO_LOCK_FREQ, *PEN_EXPO_LOCK_FREQ;

typedef enum _EN_TARGET_DIR {
    EN_TARGET_DIR_DEC = -1,
    EN_TARGET_DIR_STABLE = 0,
    EN_TARGET_DIR_INC = 1,
} EN_TARGET_DIR, *PEN_TARGET_DIR;

typedef enum _EN_SAT_WT_TBL_IDX {
    EN_SAT_WT_TBL_IDX_OVER = 0,
    EN_SAT_WT_TBL_IDX_UNDER,
} EN_SAT_WT_TBL_IDX, *PEN_SAT_WT_TBL_IDX;
#define EN_SAT_WT_TBL_IDX_MAX                   (EN_SAT_WT_TBL_IDX_UNDER + 1)


typedef struct _ST_SAT_COMP_CRV {
    STF_U8 u8T0;                                /** Threshold 0. */
    STF_U8 u8T1;                                /** Threshold 1. */
    STF_U8 u8BrightnessMax;                     /** Maximum compensation value. */
} ST_SAT_COMP_CRV, *PST_SAT_COMP_CRV;

typedef struct _ST_EXPO_LOCK {
    STF_U32 u32ExpoValue;                       /** Exposure value. */
    STF_U32 u32SnrExpoTime;                     /** Sensor exposure time. */
} ST_EXPO_LOCK, *PST_EXPO_LOCK;

typedef struct _ST_EXPO_LOCK_TBL {
    STF_U8 u8ExpoLockCnt;                       /** Exposure lock table entry count. It depends on how many exposure lock item will be loading and using. */
    ST_EXPO_LOCK stExpoLock[EXPO_LOCK_ITEM_MAX]; /** Exposure time  lock tables. */
} ST_EXPO_LOCK_TBL, *PST_EXPO_LOCK_TBL;

typedef struct _ST_CTL_AE_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable auto exposure time and gain control. */
    STF_U8 u8TargetBrightness;                  /** Target brightness values 0-255. */
    STF_U8 u8TargetBracket;                     /** Target bracket value 0-255. Margin around the target brightness which we will consider as a proper exposure (no changes in the exposure settings will be carried out). */
    STF_U8 u8AeWeightTbl[AE_WT_TBL_ROW_MAX][AE_WT_TBL_COL_MAX]; /** Brightness measurement weight table. */
    STF_U16 u16ExpoGainStep;                    /** Maximum gain change step, format 0.8.8. The exposure gain change range is +/-u16ExpoGainStep. */
    STF_U32 u32AeExposureMax;                   /** The AE exposure time(us) maximum limit. */
    STF_DOUBLE dAeGainMax;                      /** The AE gain maximum limit. */
    STF_DOUBLE dAeAdcMax;                       /** The AE digital gain maximum value. */
    STF_U8 u8ExpoLockFreq;                      /** Exposure lock frequency. 0: 50Hz, and 1: 60Hz. */
    ST_EXPO_LOCK_TBL stExpoLockTbl[EN_EXPO_LOCK_FREQ_MAX]; /** Exposure lock tables,
                                                            * first exposure lock table is for 50Hz,
                                                            * the second exposure lock table is for 60Hz,
                                                            * maximum exposure lock item is 16.
                                                            */
    STF_DOUBLE dRisingCoarseThreshold;          /** EV threshold value for AE into rising coarse mode. Range from 1.1 to 16.0. */
    STF_DOUBLE dFallingCoarseThreshold;         /** EV threshold value for AE into falling coarse mode. Range from 1.1 to 3.0. */
    STF_DOUBLE dCoarseSpeed;                    /** AE update speed on coarse mode. 0.1 is the slowest and the 1.0 is the fastest. */
    ST_SAT_COMP_CRV stOverSatCompCurve;         /** Over saturation compensation curve, it's using for brightness value compensation. */
    ST_SAT_COMP_CRV stUnderSatCompCurve;        /** Under saturation compensation curve, it's using for brightness value compensation. */
    STF_U8 u8OverSatWeightTbl[SAT_WT_TBL_ITEM_MAX]; /** Saturation measurement weight table. */
    STF_U8 u8UnderSatWeightTbl[SAT_WT_TBL_ITEM_MAX]; /** Saturation measurement weight table. */
} ST_CTL_AE_SETTING, *PST_CTL_AE_SETTING;

typedef struct _ST_CTL_AE_INFO {
    STF_U32 u32ExpectSnrExpo;                   /** The expect sensor exposure time in microseconds. */
    STF_DOUBLE dExpectSnrAgc;                   /** The expect sensor analog gain value. */
    STF_DOUBLE dExpectAdc;                      /** The expect digital gain value, this digital gain will be used on AWB module. */

    STF_U8 u8CurrentBrightness;                 /** The latest brightness value. */

    STF_U8 u8YOverPer;                          /** Measure over saturation percentage on Y channel, range 0-255(100%).*/
    STF_U8 u8YUnderPer;                         /** Measure under saturation percentage on Y channel, range 0-255(100%).*/
    STF_U8 u8OverOffset;                        /** Over saturation compensation value. */
    STF_U8 u8UnderOffset;                       /** Under saturation compensation value. */

    STF_U32 u32CurrentEV;                       /** The latest exposure value in microseconds. */
    STF_U32 u32ExpectEV;                        /** The expect exposure value in microseconds. */

    STF_U32 u32ActualSnrExpo;                   /** The actually program to sensor exposure time. */
    STF_DOUBLE dActualSnrAgc;                   /** The actually program to sensor analog gain. */
} ST_CTL_AE_INFO, *PST_CTL_AE_INFO;

typedef struct _ST_CTL_AE_PARAM {
    ST_CTL_AE_SETTING stSetting;                /** Control AE setting file parameters. */
    //-------------------------------------------------------------------------
    ST_CTL_AE_INFO stInfo;                      /** Control AE output information parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_S8 s8TargetDirection;                   /** Indicated the direction of current brightness to target brightness. 0: stable, 1:current brightness less than the target brightness, -1:current brightness large than the target brightness. */

    STF_U32 u32AeTotalWeight;                   /** Weight sum. */
    STF_U32 u32SubWinPixelCnt;                  /** AE sub-window pixel count. */
    STF_U8 u8MeasureBrightness;                 /** Measure brightness, it's the 16x16 AE window brightness, values 0-255. */

    STF_U8 u8ROverPer;                          /** Measure over saturation percentage on R channel, range 0-255(100%).*/
    STF_U8 u8RUnderPer;                         /** Measure under saturation percentage on R channel, range 0-255(100%).*/
    STF_U8 u8GOverPer;                          /** Measure over saturation percentage on G channel, range 0-255(100%).*/
    STF_U8 u8GUnderPer;                         /** Measure under saturation percentage on G channel, range 0-255(100%).*/
    STF_U8 u8BOverPer;                          /** Measure over saturation percentage on B channel, range 0-255(100%).*/
    STF_U8 u8BUnderPer;                         /** Measure under saturation percentage on B channel, range 0-255(100%).*/

    STF_U16 u16ExpoGain;                        /** The exposure gain, format 0.8.8. */

    STF_U8 u8DampingBuf[DAMPING_BUFFER_SIZE];   /** The brightness damping buffer. */
    STF_U8 u8DampCnt;                           /** Indicate how many brightness value in damping buffer. */
    STF_U8 u8DampIdx;                           /** Indicate the next brightness value insert position. */

    STF_U32 u32SnrExposureMin;                  /** Sensor support minimum exposure time value. It's read from sensor driver. */
    STF_U32 u32SnrExposureMax;                  /** Sensor support maximum exposure time value. It's read from sensor driver. */
    STF_DOUBLE dSnrGainMin;                     /** Sensor support minimum analog gain value. It's read from sensor driver. */
    STF_DOUBLE dSnrGainMax;                     /** Sensor support maximum analog gain value. It's read from sensor driver. */
    STF_U32 u32SnrExposureBackup;               /** Backup the exposure time value of sensor. */
    STF_DOUBLE dSnrGainBackup;                  /** Backup the gain value of sensor. */
    STF_U32 u32NextFrameSnrExposure;            /** Next frame program to sensor exposure time value. */
    STF_DOUBLE dNextFrameSnrGain;               /** BNext frame program to sensor gain value. */
    EN_EXPO_GAIN_METHOD enExposureGainMethod;   /** Indicate the exposure time and gain program method. */

    STF_DOUBLE dIsoStep[ISO_LEVEL_MAX];         /** ISO level AGC gain value. */
    STF_U8 u8IsoLevel;                          /** Indicate the current ISO level value. */
    STF_DOUBLE dIsoFactor;                      /** ISO factor for adaptive control interpolation. */
    ST_SENSOR *pstSensor;                       /** A pointer to a Sensor object which serves as interface to configure the physical sensor. */
} ST_CTL_AE_PARAM, *PST_CTL_AE_PARAM;

#define ST_CTL_AE_SETTING_LEN           (sizeof(ST_CTL_AE_SETTING))
#define ST_CTL_AE_INFO_LEN              (sizeof(ST_CTL_AE_INFO))
#define ST_CTL_AE_GET_PARAM_LEN         (ST_CTL_AE_SETTING_LEN + ST_CTL_AE_INFO_LEN)
#define ST_CTL_AE_SET_PARAM_LEN         (ST_CTL_AE_SETTING_LEN)
//#define ST_CTL_AE_PARAM_SIZE            (sizeof(ST_CTL_AE_PARAM) - sizeof(ST_SENSOR *))
#define ST_CTL_AE_PARAM_SIZE            (sizeof(ST_CTL_AE_PARAM))
#define ST_CTL_AE_PARAM_LEN             (ST_CTL_AE_PARAM_SIZE)

typedef struct _ST_ISP_CTL_AE {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Control ID. */
    STF_CHAR szName[16];                        /** Control Name. */
    //STF_VOID *pstCtlAeRdma;                     /** AE's control RDMA structure pointer. */
    ST_CTL_AE_PARAM *pstCtlAeParam;             /** AE's control parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Control register in pipeline. */
    STF_S32 (*Init)(STF_VOID *pIspCtx);
    STF_S32 (*LoadBinParam)(STF_VOID *pIspCtx, STF_U8 *pu8ParamBuf, STF_U16 u16ParamSize);
    STF_S32 (*Enable)(STF_VOID *pIspCtx, STF_BOOL8 bEnable);
    STF_BOOL8 (*IsEnable)(STF_VOID *pIspCtx);
    STF_S32 (*Registered)(STF_VOID *pIspCtx, STF_BOOL8 bRegistered);
    STF_BOOL8 (*IsRegistered)(STF_VOID *pIspCtx);
    STF_S32 (*Calculate)(STF_VOID *pIspCtx);
    STF_S32 (*FreeResource)(STF_VOID *pIspCtx);
    STF_S32 (*GetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);
    STF_S32 (*SetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 u16ParamSize);
} ST_ISP_CTL_AE, *PST_ISP_CTL_AE;

#pragma pack(pop)


/* auto exposure control parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AE_CalcTotalWeight(
    STF_U32 *pu32W,
    STF_U8 *pu8Tbl
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AE_ResetDampingCtrl(
    ST_ISP_CTL_AE *pstCtlAe
    );
extern
STF_S32 STFCTL_ISP_AE_AddDampingElement(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 u8Brightness
    );
extern
STF_S32 STFCTL_ISP_AE_CalcDampingMedian(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 *pu8Brightbess
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AE_EvalScAeBrightness(
    STF_U8 *pu8Brightness,
    STF_U8 *pu8Tbl,
    STF_U32 u32TotalWeight,
    STF_U32 *pu32ScDumpingBuffer,
    STF_U32 u32SubWinPixCnt
    );
extern
STF_S32 STFCTL_ISP_AE_CalcHistYSat(
    STF_U8 *pu8YOver,
    STF_U8 *pu8YUnder,
    STF_U32 *pu32ScDumpingBuffer,
    STF_U8 u8OverSatWeightTbl[SAT_WT_TBL_ITEM_MAX],
    STF_U8 u8UnderSatWeightTbl[SAT_WT_TBL_ITEM_MAX]
    );
extern
STF_S32 STFCTL_ISP_AE_LookupSatCurve(
    STF_U8 *pu8BOffset,
    STF_U8 u8In,
    ST_SAT_COMP_CRV *pstSatCompCrv
    );
extern
STF_S32 STFCTL_ISP_AE_CalcBrightnessValue(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 *pu8CurrentBrightness,
    STF_U8 u8MeasureBrightness,
    STF_U8 u8YOverPer,
    STF_U8 u8YUnderPer
    );
extern
STF_S8 STFCTL_ISP_AE_CheckTargetDirection(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 u8CurrentBrightness
    );
extern
STF_S32 STFCTL_ISP_AE_CalcExpectEV(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U32 *pu32ExpectEV,
    STF_U8 u8NewBrightness
    );
extern
STF_S32 STFCTL_ISP_AE_CalcIsoFactor(
    ST_ISP_CTL_AE *pstCtlAe
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AE_SetExposureTimeAndGain(
    STF_VOID *pIspCtx,
    STF_U32 *pu32ActualSnrExpo,
    STF_DOUBLE *pdActualSnrAgc,
    STF_U32 u32ExpectSnrExpo,
    STF_DOUBLE dExpectSnrAgc
    );
extern
STF_S32 STFCTL_ISP_AE_SetExposureTime(
    STF_VOID *pIspCtx,
    STF_U32 *pu32ActualSnrExpo,
    STF_U32 u32ExpectSnrExpo
    );
extern
STF_S32 STFCTL_ISP_AE_SetGain(
    STF_VOID *pIspCtx,
    STF_DOUBLE *pdActualSnrAgc,
    STF_DOUBLE dExpectSnrAgc
    );
extern
STF_S32 STFCTL_ISP_AE_SetEV(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U32 u32EV,
    STF_BOOL8 bUpdateToSensor,
    STF_BOOL8 bIsInit
    );
extern
STF_S32 STFCTL_ISP_AE_SetEV_Step2(
    ST_ISP_CTL_AE *pstCtlAe
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AE_SetEnable(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFCTL_ISP_AE_SetTargetBrightness(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 u8TargetBrightness
    );
extern
STF_S32 STFCTL_ISP_AE_SetTargetBracket(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 u8TargetDirection
    );
extern
STF_S32 STFCTL_ISP_AE_SetTargetDirection(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_S8 u8TargetDirection
    );
extern
STF_S32 STFCTL_ISP_AE_SetAeWeightingTable(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 u8AeWeightTbl[AE_WT_TBL_ITEM_MAX]
    );
extern
STF_S32 STFCTL_ISP_AE_SetExposureGainStep(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U16 u16ExpoGainStep
    );
extern
STF_S32 STFCTL_ISP_AE_SetRisingCoarseThreshold(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_DOUBLE dCoarseThreshold
    );
extern
STF_S32 STFCTL_ISP_AE_SetFallingCoarseThreshold(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_DOUBLE dCoarseThreshold
    );
extern
STF_S32 STFCTL_ISP_AE_SetCoarseSpeed(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_DOUBLE dCoarseSpeed
    );
extern
STF_S32 STFCTL_ISP_AE_SetOverSaturationCompensationCurve(
    ST_ISP_CTL_AE *pstCtlAe,
    ST_SAT_COMP_CRV *pstOverSatCompCurve
    );
extern
STF_S32 STFCTL_ISP_AE_SetUnderSaturationCompensationCurve(
    ST_ISP_CTL_AE *pstCtlAe,
    ST_SAT_COMP_CRV *pstUnderSatCompCurve
    );
extern
STF_S32 STFCTL_ISP_AE_SetSatWeightingTable(
    ST_ISP_CTL_AE *pstCtlAe,
    EN_SAT_WT_TBL_IDX enSatWtTblIdx,
    STF_U8 u8SatWeightTbl[SAT_WT_TBL_ITEM_MAX]
    );
extern
STF_S32 STFCTL_ISP_AE_SetAeExposureMax(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U32 u32AeExposureMax
    );
extern
STF_S32 STFCTL_ISP_AE_SetAeGainMax(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_DOUBLE dAeGainMax
    );
extern
STF_S32 STFCTL_ISP_AE_SetAeAdcMax(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_DOUBLE dAeAdcMax
    );
extern
STF_S32 STFCTL_ISP_AE_SetExpoLockFreq(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 u8ExpoLockFreq
    );
extern
STF_S32 STFCTL_ISP_AE_SetExposureLockTable(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_U8 u8ExpoLockFreq,
    ST_EXPO_LOCK stExpoLockTbl[],
    STF_U8 u8ExpoLockCnt
    );
extern
STF_S32 STFCTL_ISP_AE_SetIsoStep(
    ST_ISP_CTL_AE *pstCtlAe,
    STF_DOUBLE dIsoStep[]
    );
extern
STF_S32 STFCTL_ISP_AE_SetExposureAndGain(
    STF_VOID *pIspCtx,
    STF_U32 u32ExpectSnrExpo,
    STF_DOUBLE dExpectSnrAgc
    );
extern
STF_S32 STFCTL_ISP_AE_GetExposureAndGain(
    STF_VOID *pIspCtx,
    STF_U32 *pu32ActualSnrExpo,
    STF_DOUBLE *pdActualSnrAgc
    );
extern
STF_S32 STFCTL_ISP_AE_GetIsoInfo(
    STF_VOID *pIspCtx,
    STF_U8 *pu8IsoLevel,
    STF_DOUBLE *pdIsoFactor
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AE_Init(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AE_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFCTL_ISP_AE_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFCTL_ISP_AE_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AE_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFCTL_ISP_AE_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AE_Calculate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AE_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_AE_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFCTL_ISP_AE_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_AE_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFCTL_ISP_AE_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFCTL_ISP_AE_H__
