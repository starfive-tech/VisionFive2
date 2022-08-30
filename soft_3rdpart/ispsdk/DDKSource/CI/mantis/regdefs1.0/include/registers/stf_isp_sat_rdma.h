/**
  ******************************************************************************
  * @file  stf_isp_sat_rdma.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  12/14/2019
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
#ifndef __STF_ISP_SAT_RDMA_H__
#define __STF_ISP_SAT_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add brightness, contrast, hue and saturation registers structure */
typedef struct _ST_SAT_GAIN {
    STF_U16 u16AmplificationBase;
    STF_U16 u16AmplificationDelta;
} ST_SAT_GAIN, *PST_SAT_GAIN;

typedef struct _ST_SAT_GAIN_PARAM {
    STF_U8 u8IspIdx;
    ST_SAT_GAIN stSatGain;
} ST_SAT_GAIN_PARAM, *PST_SAT_GAIN_PARAM;

typedef struct _ST_SAT_THRESHOLD {
    STF_U16 u16Base;
    STF_U16 u16Delta;
} ST_SAT_THRESHOLD, *PST_SAT_THRESHOLD;

typedef struct _ST_SAT_THRESHOLD_PARAM {
    STF_U8 u8IspIdx;
    ST_SAT_THRESHOLD stSatThreshold;
} ST_SAT_THRESHOLD_PARAM, *PST_SAT_THRESHOLD_PARAM;

typedef struct _ST_SAT_UV_OFFSET {
    STF_U16 u16UOffset;
    STF_U16 u16VOffset;
} ST_SAT_UV_OFFSET, *PST_SAT_UV_OFFSET;

typedef struct _ST_SAT_UV_OFFSET_PARAM {
    STF_U8 u8IspIdx;
    ST_SAT_UV_OFFSET stSatUvOffset;
} ST_SAT_UV_OFFSET_PARAM, *PST_SAT_UV_OFFSET_PARAM;

typedef struct _ST_SAT_HUE_FACTOR {
    STF_U16 u16Cos;
    STF_U16 u16Sin;
} ST_SAT_HUE_FACTOR, *PST_SAT_HUE_FACTOR;

typedef struct _ST_SAT_HUE_FACTOR_PARAM {
    STF_U8 u8IspIdx;
    ST_SAT_HUE_FACTOR stSatHueFactor;
} ST_SAT_HUE_FACTOR_PARAM, *PST_SAT_HUE_FACTOR_PARAM;

typedef struct _ST_SAT_SCALING_FACTOR {
    STF_U8 u8IspIdx;
    STF_U8 u8ScalingFactor;
} ST_SAT_SCALING_FACTOR, *PST_SAT_SCALING_FACTOR;

typedef struct _ST_SAT_Y_ADJ {
    STF_U16 u16YiMin;
    STF_U16 u16Yoir;
    STF_U16 u16YoMin;
    STF_U16 u16YoMax;
} ST_SAT_Y_ADJ, *PST_SAT_Y_ADJ;

typedef struct _ST_SAT_Y_ADJ_PARAM {
    STF_U8 u8IspIdx;
    ST_SAT_Y_ADJ stSatYAdj;
} ST_SAT_Y_ADJ_PARAM, *PST_SAT_Y_ADJ_PARAM;

typedef struct _ST_SAT_CHROMA_REG {
    REG_ISAT0 Gain;                 // 0x0A30, Chroma Saturation Gain.
    REG_ISAT1 Threshold;            // 0x0A34, Chroma Saturation Threshold.
    REG_ISAT2 UvOffset;             // 0x0A38, Chroma Saturation UV Offset.
    REG_ISAT3 HueFactor;            // 0x0A3C, Chroma Saturation Hue Factor.
    REG_ISAT4 ScalingCoefficient;   // 0x0A40, Chroma Saturation scaling coefficient.
} ST_SAT_CHROMA_REG, *PST_SAT_CHROMA_REG;

#define ST_SAT_CHROMA_REG_SIZE  (sizeof(ST_SAT_CHROMA_REG))
#define ST_SAT_CHROMA_REG_LEN   (ST_SAT_CHROMA_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SAT_Y_REG {
    REG_IYADJ0 YAdjustment0;        // 0x0A54, Y Adjustment Register 0.
    REG_IYADJ1 YAdjustment1;        // 0x0A58, Y Adjustment Register 1.
} ST_SAT_Y_REG, *PST_SAT_Y_REG;

#define ST_SAT_Y_REG_SIZE       (sizeof(ST_SAT_Y_REG))
#define ST_SAT_Y_REG_LEN        (ST_SAT_Y_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SAT_REG {
    REG_ISAT0 Gain;                 // 0x0A30, Chroma Saturation Gain.
    REG_ISAT1 Threshold;            // 0x0A34, Chroma Saturation Threshold.
    REG_ISAT2 UvOffset;             // 0x0A38, Chroma Saturation UV Offset.
    REG_ISAT3 HueFactor;            // 0x0A3C, Chroma Saturation Hue Factor.
    REG_ISAT4 ScalingCoefficient;   // 0x0A40, Chroma Saturation scaling coefficient.
    //STF_U32 u32Reserved_0A44[4];    // Added reserved variables to let ST_SAT_REG address is contiguous.
    //-------------------------------------------------------------------------
    REG_IYADJ0 YAdjustment0;        // 0x0A54, Y Adjustment Register 0.
    REG_IYADJ1 YAdjustment1;        // 0x0A58, Y Adjustment Register 1.
} ST_SAT_REG, *PST_SAT_REG;

#define ST_SAT_REG_SIZE         (sizeof(ST_SAT_REG))
#define ST_SAT_REG_LEN          (ST_SAT_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_SAT_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SAT_CHROMA_REG stSatChromaReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaYCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SAT_Y_REG stSatYReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_SAT_RDMA, *PST_SAT_RDMA;

#define ST_SAT_RDMA_SIZE        (sizeof(ST_SAT_RDMA))
#define ST_SAT_RDMA_LEN         (ST_SAT_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SAT_CHROMA_RDMA_SIZE (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SAT_CHROMA_REG) + 7) / 8) * 8))
#define ST_SAT_CHROMA_RDMA_LEN  (ST_SAT_CHROMA_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SAT_Y_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SAT_Y_REG) + 7) / 8) * 8))
#define ST_SAT_Y_RDMA_LEN       (ST_SAT_Y_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_SAT_RDMA {
    STF_U8 u8IspIdx;
    ST_SAT_RDMA stSatRdma __attribute__ ((aligned (8)));
} ST_ISP_SAT_RDMA, *PST_ISP_SAT_RDMA;
#pragma pack(pop)


///* Brightness, contrast, hue and saturation registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SAT_SetReg(
//    STF_U8 u8IspIdx,
//    ST_SAT_RDMA *pstSatRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SAT_SetSaturationGain(
//    STF_U8 u8IspIdx,
//    STF_U16 u16AmplificationBase,
//    STF_U16 u16AmplificationDelta
//    );
//extern
//STF_VOID STFHAL_ISP_SAT_SetSaturationThreshold(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Base,
//    STF_U16 u16Delta
//    );
//extern
//STF_VOID STFHAL_ISP_SAT_SetSaturationUVOffset(
//    STF_U8 u8IspIdx,
//    STF_U16 u16UOffset,
//    STF_U16 u16VOffset
//    );
//extern
//STF_VOID STFHAL_ISP_SAT_SetSaturationHueFactor(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Cos,
//    STF_U16 u16Sin
//    );
//extern
//STF_VOID STFHAL_ISP_SAT_SetSaturationScalingCoefficient(
//    STF_U8 u8IspIdx,
//    STF_U8 u8ScalingFactor
//    );
//extern
//STF_VOID STFHAL_ISP_SAT_SetSaturationYAdjustment(
//    STF_U8 u8IspIdx,
//    ST_SAT_Y_ADJ *pstYAdjustment
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_SAT_RDMA_H__
