/**
  ******************************************************************************
  * @file  stfctl_isp_ycrv.h
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


#ifndef __STFCTL_ISP_YCRV_H__
#define __STFCTL_ISP_YCRV_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "Modules/stfmod_isp_ycrv.h"


//#define ENABLE_YCRV_EQUAL_SUPPORT
//#define ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT


/* Y curve control parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#if 0
#define ISP_DEF_YCRV_UPDATE_SPEED                       (256 * 0.5)
#elif 1
#define ISP_DEF_YCRV_UPDATE_SPEED                       (256 * 0.125)
#endif
#if 0
#define ISP_DEF_YCRV_GENERATE_METHOD                    (EN_YCRV_METHOD_EQUALIZATION)
#else
#define ISP_DEF_YCRV_GENERATE_METHOD                    (EN_YCRV_METHOD_MIN_MAX_CRV_LMTD)
#endif
// For minimum and maximum limit Y curve.
#if 0
#define ISP_DEF_YCRV_MIN_MAX_LMTD_LIMIT_MAX             (0x0100)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_LIMIT_MIN             (0x0100)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_START_LEVEL           (0x0000)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_END_LEVEL             (0x0100)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_START_PERIOD          (12)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_END_PERIOD            (4)
#elif 1
#define ISP_DEF_YCRV_MIN_MAX_LMTD_LIMIT_MAX             (0x014D)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_LIMIT_MIN             (0x009A)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_START_LEVEL           (0x0000)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_END_LEVEL             (0x0100)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_START_PERIOD          (7)
#define ISP_DEF_YCRV_MIN_MAX_LMTD_END_PERIOD            (4)
#endif
#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
// For equalization Y curve.
#define ISP_DEF_YCRV_EQUAL_ENABLE_GLOBAL_TONE_MAPPING (STF_TRUE)
#if 0
#define ISP_DEF_YCRV_EQUAL_MAX_DARK_SUPPRESS            (0.10)
#define ISP_DEF_YCRV_EQUAL_MAX_BRIGHT_SUPPRESS          (0.15)
#define ISP_DEF_YCRV_EQUAL_DARK_SUPPRESS_RATIO          (0.03)
#define ISP_DEF_YCRV_EQUAL_BRIGHT_SUPPRESS_RATIO        (0.01)
#elif 0
#define ISP_DEF_YCRV_EQUAL_MAX_DARK_SUPPRESS            (0.07)
#define ISP_DEF_YCRV_EQUAL_MAX_BRIGHT_SUPPRESS          (0.10)
#define ISP_DEF_YCRV_EQUAL_DARK_SUPPRESS_RATIO          (0.01)
#define ISP_DEF_YCRV_EQUAL_BRIGHT_SUPPRESS_RATIO        (0.01)
#elif 1
#define ISP_DEF_YCRV_EQUAL_MAX_DARK_SUPPRESS            (0.15)
#define ISP_DEF_YCRV_EQUAL_MAX_BRIGHT_SUPPRESS          (0.15)
#define ISP_DEF_YCRV_EQUAL_DARK_SUPPRESS_RATIO          (0.01)
#define ISP_DEF_YCRV_EQUAL_BRIGHT_SUPPRESS_RATIO        (0.01)
#endif
#define ISP_DEF_YCRV_EQUAL_OVERSHOT_THRESHOLD           (2.0)
#define ISP_DEF_YCRV_EQUAL_WDR_CEILING                  (1.1)
#define ISP_DEF_YCRV_EQUAL_WDR_FLOOR                    (1.0)
#endif //#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
// For minimum curve and maximum line limit Y curve.
#if 1
// 20201226_v01
#define ISP_DEF_YCRV_MIN_CRV_MAX_LINE_LMTD_LIMIT_MAX    (0x0100 * 1.5)
#define ISP_DEF_YCRV_MIN_CRV_MAX_LINE_LMTD_LIMIT_MIN    (0x0100 * 1.0)
#define ISP_DEF_YCRV_MIN_CRV_MAX_LINE_LMTD_SLOPE        (0x0100 * (1.0 / 16.0))
#elif 1
// 20201228_v01
#define ISP_DEF_YCRV_MIN_CRV_MAX_LINE_LMTD_LIMIT_MAX    (0x0100 * 1.25)
#define ISP_DEF_YCRV_MIN_CRV_MAX_LINE_LMTD_LIMIT_MIN    (0x0100 * 1.0)
#define ISP_DEF_YCRV_MIN_CRV_MAX_LINE_LMTD_SLOPE        (0x0100 * (1.0 / 16.0))
#endif
#endif //#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
// For minimum and maximum curve limit Y curve.
#if 0
// 20210103_v01
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX         (1.0)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN         (1.0)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX_START   (2.0)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN_START   (0.8)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX_SLOPE   ((ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX - ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX_START) / 20.0)  // (-0.05)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN_SLOPE   ((ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN - ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN_START) / 4.0)   // (0.05)
#elif 1
// 20210106_v01
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX         (1.0)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN         (1.0)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX_START   (2.0)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN_START   (0.2)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX_SLOPE   ((ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX - ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MAX_START) / 20.0)  // (-0.05)
#define ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN_SLOPE   ((ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN - ISP_DEF_YCRV_MIN_MAX_CRV_LMTD_LIMIT_MIN_START) / 4.0)   // (0.2)
#endif


#pragma pack(push, 1)

typedef enum _EN_YCRV_METHOD {
    EN_YCRV_METHOD_MIN_MAX_LMTD_WITH_PERIOD = 0,
#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
    EN_YCRV_METHOD_EQUALIZATION,
#endif //#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
    EN_YCRV_METHOD_MIN_CRV_MAX_LINE_LMTD,       /** MAX curve slope is 0. */
#endif //#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
    EN_YCRV_METHOD_MIN_MAX_CRV_LMTD,
    EN_YCRV_METHOD_MAX
} EN_YCRV_METHOD, *PEN_YCRV_METHOD;


typedef struct _ST_YHIST_PNT {
    STF_U8 u8Index;                             /** Bin index of Y histogram. */
    STF_U32 u32YHist;                           /** Backup the (YUV domain) Y histogram from statistical collection buffer. */
} ST_YHIST_PNT, *PST_YHIST_PNT;

typedef struct _ST_YHIST_MIN_MAX_LMTD_W_PER {
    STF_U16 u16MaxLimit;                        /** Y histogram maximum limit value. */
    STF_U16 u16MinLimit;                        /** Y histogram minimum limit value. */
    STF_U16 u16StartLevel;                      /** Y histogram start level value. */
    STF_U16 u16EndLevel;                        /** Y histogram end level value. */
    STF_U8 u8StartPeriod;                       /** u8StartPeriod indicates how many bin will be limited to the start level value at the beginning of the histogram. */
    STF_U8 u8EndPeriod;                         /** u8EndPeriod indicates how many bin will be limited to the end level value at the end of the histogram. */
} ST_YHIST_MIN_MAX_LMTD_W_PER, *PST_YHIST_MIN_MAX_LMTD_W_PER;

#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
typedef struct _ST_YHIST_EQUAL_PARAM {
    STF_BOOL8 bEnableGlobalToneMapping;         /** Enable the global tone mapping feature. */
    STF_DOUBLE dMaxDarkSuppress;                /** Maximum dark suppress ratio. */
    STF_DOUBLE dMaxBrightSuppress;              /** Maximum bright suppress ratio. */
    STF_DOUBLE dDarkSuppressRatio;              /** Desire dark suppress ratio. */
    STF_DOUBLE dBrightSuppressRatio;            /** Desire bright suppress ratio. */
    STF_DOUBLE dOvershotThreshold;              /** Overshot threshold. */
    STF_DOUBLE dWdrCeiling;                     /** WDR ceiling filter when bEnableGlobalToneMapping is enable. */
    STF_DOUBLE dWdrFloor;                       /** WDR floor filter when bEnableGlobalToneMapping is enable. */
} ST_YHIST_EQUAL_PARAM, *PST_YHIST_EQUAL_PARAM;

#endif //#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
typedef struct _ST_YHIST_MIN_CRV_MAX_LINE_LMTD {
    STF_U16 u16MaxLimit;                        /** Y histogram maximum limit value, format 0.8.8. */
    STF_U16 u16MinLimit;                        /** Y histogram minimum curve limit value, format 0.8.8. */
    STF_S16 s16Slope;                           /** Y histogram minimum curve slope, format 1.7.8. */
} ST_YHIST_MIN_CRV_MAX_LINE_LMTD, *PST_YHIST_MIN_CRV_MAX_LINE_LMTD;

#endif //#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
typedef struct _ST_YHIST_MIN_MAX_CRV_LMTD {
    STF_DOUBLE dMaxLimit;                     /** Y histogram maximum curve limit value. */
    STF_DOUBLE dMinLimit;                     /** Y histogram minimum curve limit value. */
    STF_DOUBLE dMaxStart;                     /** Y histogram maximum curve start value. */
    STF_DOUBLE dMinStart;                     /** Y histogram minimum curve start value. */
    STF_DOUBLE dMaxSlope;                     /** Y histogram maximum curve slope value. */
    STF_DOUBLE dMinSlope;                     /** Y histogram minimum curve slope value. */
} ST_YHIST_MIN_MAX_CRV_LMTD, *PST_YHIST_MIN_MAX_CRV_LMTD;

typedef struct _ST_CTL_YCRV_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable Y curve control. */
    STF_U16 u16UpdateSpeed;                     /** Y curve update speed, range 0 (lowest) - 256 (fastest). */
    STF_U8 u8Method;                            /** Y curve dynamic generate method. 0:EN_YCRV_METHOD_MIN_MAX_LMTD_WITH_PERIOD, 1:EN_YCRV_METHOD_EQUALIZATION, 2:EN_YCRV_METHOD_MIN_CRV_MAX_LINE_LMTD, 3:EN_YCRV_METHOD_MIN_MAX_CRV_LMTD. */
    ST_YHIST_MIN_MAX_LMTD_W_PER stYHistMinMaxLmtdWithPeriod; /** Y histogram min-maxed control parameters. */
#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
    ST_YHIST_EQUAL_PARAM stYHistEqualParam;     /** Y histogram equalization control parameters. */
#endif //#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
    ST_YHIST_MIN_CRV_MAX_LINE_LMTD stYHistMinCrvMaxLineLmtd; /** Y histogram min-maxed curve control parameters. */
#endif //#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
    ST_YHIST_MIN_MAX_CRV_LMTD stYHistMinMaxCrvLmtd; /** Y histogram min-maxed curve control parameters. */
} ST_CTL_YCRV_SETTING, *PST_CTL_YCRV_SETTING;

typedef struct _ST_CTL_YCRV_INFO {
    ST_YHIST_PNT stYHist[YCRV_PNT];             /** Backup the (YUV domain) Y histogram from statistical collection buffer. */
    ST_POINT stYCurveWoDamping[YCRV_MAX_PNT];   /** Y curve, this is a global tone mapping curve that do not have apply the update speed factor. */
    ST_POINT stYCurve[YCRV_MAX_PNT];            /** Y curve, this is a global tone mapping curve that have apply the update speed factor. */
} ST_CTL_YCRV_INFO, *PST_CTL_YCRV_INFO;

typedef struct _ST_CTL_YCRV_PARAM {
    ST_CTL_YCRV_SETTING stSetting;              /** Control YCRV setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_CTL_YCRV_INFO stInfo;                    /** Control YCRV output information parameters. */
    STF_U32 u32MinMaxYHist[YCRV_PNT];           /** Min-maxed Y histogram. */
} ST_CTL_YCRV_PARAM, *PST_CTL_YCRV_PARAM;

#define ST_CTL_YCRV_SETTING_LEN         (sizeof(ST_CTL_YCRV_SETTING))
#define ST_CTL_YCRV_INFO_LEN            (sizeof(ST_CTL_YCRV_INFO))
#define ST_CTL_YCRV_GET_PARAM_LEN       (ST_CTL_YCRV_SETTING_LEN + ST_CTL_YCRV_INFO_LEN)
#define ST_CTL_YCRV_SET_PARAM_LEN       (ST_CTL_YCRV_SETTING_LEN)
#define ST_CTL_YCRV_PARAM_SIZE          (sizeof(ST_CTL_YCRV_PARAM))
#define ST_CTL_YCRV_PARAM_LEN           (ST_CTL_SAT_PARAM_SIZE)

typedef struct _ST_ISP_CTL_YCRV {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Control ID. */
    STF_CHAR szName[16];                        /** Control Name. */
    //STF_VOID *pstCtlYCrvRdma;                   /** YCRV's RDMA structure pointer. */
    ST_CTL_YCRV_PARAM *pstCtlYCrvParam;         /** YCRV's control parameters structure pointer. */
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
} ST_ISP_CTL_YCRV, *PST_ISP_CTL_YCRV;

#pragma pack(pop)


/* Y curve control parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_YCRV_CaptureYHistogram(
    ST_YHIST_PNT stYHist[YCRV_PNT],
    STF_U32 *pu32YHistDumpingBuffer
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistDuplicate(
    STF_DOUBLE dNewHist[],
    STF_DOUBLE dOriHist[]
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistApplyMin(
    STF_DOUBLE dHist[],
    double dValue,
    STF_U32 u32StartIdx,
    STF_U32 u32EndIdx
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistApplyMax(
    STF_DOUBLE dHist[],
    double dValue,
    STF_U32 u32StartIdx,
    STF_U32 u32EndIdx
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistSumInU32(
    ST_YHIST_PNT stOriHist[],
    STF_U32 *pu32HistSum
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistSumInDouble(
    STF_DOUBLE dHist[],
    STF_DOUBLE *pdHistSum
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistNormaliseFromU32(
    STF_DOUBLE dNewHist[],
    ST_YHIST_PNT stOriHist[]
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistNormaliseFromDouble(
    STF_DOUBLE dNewHist[],
    STF_DOUBLE dOriHist[]
    );
extern
STF_S32 STFCTL_ISP_YCRV_HistAccumulate(
    ST_POINT stCurve[],
    STF_DOUBLE dHist[]
    );
extern
STF_S32 STFCTL_ISP_YCRV_ConvertHistToMappingCvr(
    ST_POINT *pstCurve,
    STF_U32 *pu32Hist
    );
extern
STF_S32 STFCTL_ISP_YCRV_CalcMinMaxLimitedWithPeriodHist(
    STF_U32 *pu32Hist,
    ST_YHIST_PNT stOriHist[],
    ST_YHIST_MIN_MAX_LMTD_W_PER *pstYHistMinMaxLmtdWithPeriod
    );
#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
extern
STF_S32 STFCTL_ISP_YCRV_CalcEqualizationHist(
    ST_POINT stCurve[],
    ST_YHIST_PNT stOriHist[],
    ST_YHIST_EQUAL_PARAM *pstYHistEqualParam
    );
#endif //#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
extern
STF_S32 STFCTL_ISP_YCRV_CalcMinCurveMaxLineLimitedHist(
    STF_U32 *pu32Hist,
    ST_YHIST_PNT stOriHist[],
    ST_YHIST_MIN_CRV_MAX_LINE_LMTD *pstYHistMinCrvMaxLineLmtd
    );
#endif //#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
extern
STF_S32 STFCTL_ISP_YCRV_CalcMinMaxCurveLimitedHist(
    STF_U32 *pu32Hist,
    ST_YHIST_PNT stOriHist[],
    ST_YHIST_MIN_MAX_CRV_LMTD *pstYHistMinMaxCrvLmtd
    );
extern
STF_S32 STFCTL_ISP_YCRV_ApplyUpdateSpeed(
    ST_POINT *pstOldCurve,
    ST_POINT *pstNewCurve,
    STF_U16 u16Speed
    );
extern
STF_S32 STFCTL_ISP_YCRV_MakeLinearCurve(
    ST_POINT *pstCurve
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_YCRV_SetEnable(
    ST_ISP_CTL_YCRV *pstCtlYCrv,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFCTL_ISP_YCRV_SetUpdateSpeed(
    ST_ISP_CTL_YCRV *pstCtlYCrv,
    STF_U16 u16Speed
    );
extern
STF_S32 STFCTL_ISP_YCRV_SetMethod(
    ST_ISP_CTL_YCRV *pstCtlYCrv,
    STF_U8 u8Method
    );
extern
STF_S32 STFCTL_ISP_YCRV_SetYHistMinMaxLmtdWithPeriod(
    ST_ISP_CTL_YCRV *pstCtlYCrv,
    ST_YHIST_MIN_MAX_LMTD_W_PER *pstYHistMinMaxLmtdWithPeriod
    );
#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
extern
STF_S32 STFCTL_ISP_YCRV_SetYHistEqualParam(
    ST_ISP_CTL_YCRV *pstCtlYCrv,
    ST_YHIST_EQUAL_PARAM *pstYHistEqualParam
    );
#endif //#if defined(ENABLE_YCRV_EQUAL_SUPPORT)
#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
extern
STF_S32 STFCTL_ISP_YCRV_SetYHistMinCrvMaxLineLmtd(
    ST_ISP_CTL_YCRV *pstCtlYCrv,
    ST_YHIST_MIN_CRV_MAX_LINE_LMTD *pstYHistMinCrvMaxLineLmtd
    );
#endif //#if defined(ENABLE_YCRV_MIN_CRV_MAX_LINE_LMTD_SUPPORT)
extern
STF_S32 STFCTL_ISP_YCRV_SetYHistMinMaxCrvLmtd(
    ST_ISP_CTL_YCRV *pstCtlYCrv,
    ST_YHIST_MIN_MAX_CRV_LMTD *pstYHistMinMaxCrvLmtd
    );
extern
STF_S32 STFCTL_ISP_YCRV_InitializeYHist(
    ST_ISP_CTL_YCRV *pstCtlYCrv
    );
extern
STF_S32 STFCTL_ISP_YCRV_InitializeYCurveWoDamping(
    ST_ISP_CTL_YCRV *pstCtlYCrv
    );
extern
STF_S32 STFCTL_ISP_YCRV_InitializeYCurve(
    ST_ISP_CTL_YCRV *pstCtlYCrv
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_YCRV_Init(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_YCRV_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFCTL_ISP_YCRV_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFCTL_ISP_YCRV_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_YCRV_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFCTL_ISP_YCRV_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_YCRV_Calculate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_YCRV_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFCTL_ISP_YCRV_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFCTL_ISP_YCRV_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFCTL_ISP_YCRV_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFCTL_ISP_YCRV_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFCTL_ISP_YCRV_H__
