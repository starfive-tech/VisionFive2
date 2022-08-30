/**
  ******************************************************************************
  * @file  stfmod_isp_ccm.h
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


#ifndef __STFMOD_ISP_CCM_H__
#define __STFMOD_ISP_CCM_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_ccm_rdma.h"
#include "stflib_isp_pipeline.h"


/* color correction matrix module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#if 1
#define ISP_DEF_CCM_C                           (32)    /** Noise compensation offset, range 0 - 1023. */
#define ISP_DEF_CCM_DNRM                        (6)     /** Color matrix threshold de-normalization factor, range 0 - 8. */
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_MIN                      (0)     /** Predefined saturation threshold value for min matrix. range 0 - 1023. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_00            (1.0)   /** Min matrix component 00.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_01            (0.0)   /** Min matrix component 01.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_02            (0.0)   /** Min matrix component 02.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_10            (0.0)   /** Min matrix component 10.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_11            (1.0)   /** Min matrix component 11.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_12            (0.0)   /** Min matrix component 12.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_20            (0.0)   /** Min matrix component 20.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_21            (0.0)   /** Min matrix component 21.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_22            (1.0)   /** Min matrix component 22.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_0             (0)     /** Min offset component 0.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_1             (0)     /** Min offset component 1.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_2             (0)     /** Min offset component 2.. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW                      (0)     /** Predefined saturation threshold value for low matrix. range 0 - 1023. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW_MATRIX_00            (1.0)   /** Low matrix component 00.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_01            (0.0)   /** Low matrix component 01.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_02            (0.0)   /** Low matrix component 02.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_10            (0.0)   /** Low matrix component 10.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_11            (1.0)   /** Low matrix component 11.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_12            (0.0)   /** Low matrix component 12.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_20            (0.0)   /** Low matrix component 20.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_21            (0.0)   /** Low matrix component 21.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_22            (1.0)   /** Low matrix component 22.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_0             (0)     /** Low offset component 0.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_1             (0)     /** Low offset component 1.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_2             (0)     /** Low offset component 2.. */
#define ISP_DEF_CCM_LUM_PNT_1_VALUE             (205)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_2_VALUE             (409)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_3_VALUE             (614)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_4_VALUE             (818)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_5_VALUE             (1023)  /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_1_FACTOR            (0.6)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_2_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_3_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_4_FACTOR            (0.9)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_5_FACTOR            (0.5)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_1_VALUE             (32)    /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_2_VALUE             (64)    /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_3_VALUE             (128)   /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_4_VALUE             (192)   /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_5_VALUE             (1023)  /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_1_FACTOR            (0.5)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_2_FACTOR            (0.65)  /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_3_FACTOR            (0.9)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_4_FACTOR            (0.95)  /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_5_FACTOR            (1.0)   /** Saturation point 1 factor. */
#elif 0
#define ISP_DEF_CCM_C                           (256)   /** Noise compensation offset, range 0 - 1023. */
#define ISP_DEF_CCM_DNRM                        (6)     /** Color matrix threshold de-normalization factor, range 0 - 8. */
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_MIN                      (418)   /** Predefined saturation threshold value for min matrix. range 0 - 1023. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_00            (1.6)   /** Min matrix component 00.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_01            (-0.4)  /** Min matrix component 01.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_02            (-0.2)  /** Min matrix component 02.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_10            (-0.4)  /** Min matrix component 10.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_11            (1.6)   /** Min matrix component 11.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_12            (-0.2)  /** Min matrix component 12.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_20            (0.0)   /** Min matrix component 20.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_21            (-0.8)  /** Min matrix component 21.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_22            (1.8)   /** Min matrix component 22.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_0             (0)     /** Min offset component 0.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_1             (0)     /** Min offset component 1.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_2             (0)     /** Min offset component 2.. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW                      (630)   /** Predefined saturation threshold value for low matrix. range 0 - 1023. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW_MATRIX_00            (1.4)   /** Low matrix component 00.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_01            (-0.4)  /** Low matrix component 01.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_02            (0.0)   /** Low matrix component 02.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_10            (-0.4)  /** Low matrix component 10.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_11            (1.4)   /** Low matrix component 11.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_12            (0.0)   /** Low matrix component 12.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_20            (-0.2)  /** Low matrix component 20.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_21            (-1.0)  /** Low matrix component 21.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_22            (2.2)   /** Low matrix component 22.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_0             (0)     /** Low offset component 0.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_1             (0)     /** Low offset component 1.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_2             (0)     /** Low offset component 2.. */
#define ISP_DEF_CCM_LUM_PNT_1_VALUE             (8)     /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_2_VALUE             (64)    /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_3_VALUE             (128)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_4_VALUE             (768)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_5_VALUE             (1023)  /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_1_FACTOR            (0.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_2_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_3_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_4_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_5_FACTOR            (0.7)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_1_VALUE             (2)     /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_2_VALUE             (32)    /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_3_VALUE             (96)    /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_4_VALUE             (128)   /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_5_VALUE             (1023)  /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_1_FACTOR            (0.5)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_2_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_3_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_4_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_5_FACTOR            (1.0)   /** Saturation point 1 factor. */
#elif 0
#define ISP_DEF_CCM_C                           (256)   /** Noise compensation offset, range 0 - 1023. */
#define ISP_DEF_CCM_DNRM                        (6)     /** Color matrix threshold de-normalization factor, range 0 - 8. */
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_MIN                      (418)   /** Predefined saturation threshold value for min matrix. range 0 - 1023. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_00            (2.4)   /** Min matrix component 00.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_01            (-1.0)  /** Min matrix component 01.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_02            (-0.4)  /** Min matrix component 02.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_10            (-0.6)  /** Min matrix component 10.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_11            (2.2)   /** Min matrix component 11.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_12            (-0.6)  /** Min matrix component 12.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_20            (-0.2)  /** Min matrix component 20.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_21            (-0.8)  /** Min matrix component 21.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_22            (2.0)   /** Min matrix component 22.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_0             (0)     /** Min offset component 0.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_1             (0)     /** Min offset component 1.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_2             (0)     /** Min offset component 2.. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW                      (630)   /** Predefined saturation threshold value for low matrix. range 0 - 1023. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW_MATRIX_00            (2.4)   /** Low matrix component 00.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_01            (-0.8)  /** Low matrix component 01.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_02            (-0.6)  /** Low matrix component 02.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_10            (-0.6)  /** Low matrix component 10.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_11            (2.2)   /** Low matrix component 11.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_12            (-0.6)  /** Low matrix component 12.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_20            (0.0)   /** Low matrix component 20.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_21            (-1.0)  /** Low matrix component 21.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_22            (2.0)   /** Low matrix component 22.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_0             (0)     /** Low offset component 0.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_1             (0)     /** Low offset component 1.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_2             (0)     /** Low offset component 2.. */
#define ISP_DEF_CCM_LUM_PNT_1_VALUE             (24)    /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_2_VALUE             (96)    /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_3_VALUE             (400)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_4_VALUE             (800)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_5_VALUE             (1023)  /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_1_FACTOR            (0.2)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_2_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_3_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_4_FACTOR            (0.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_5_FACTOR            (0.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_1_VALUE             (16)    /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_2_VALUE             (96)    /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_3_VALUE             (128)   /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_4_VALUE             (128)   /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_5_VALUE             (1023)  /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_1_FACTOR            (0.2)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_2_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_3_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_4_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_5_FACTOR            (1.0)   /** Saturation point 1 factor. */
#elif 0
#define ISP_DEF_CCM_C                           (256)   /** Noise compensation offset, range 0 - 1023. */
#define ISP_DEF_CCM_DNRM                        (6)     /** Color matrix threshold de-normalization factor, range 0 - 8. */
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_MIN                      (418)   /** Predefined saturation threshold value for min matrix. range 0 - 1023. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_00            (2.4)   /** Min matrix component 00.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_01            (-1.0)  /** Min matrix component 01.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_02            (-0.4)  /** Min matrix component 02.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_10            (-0.6)  /** Min matrix component 10.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_11            (2.2)   /** Min matrix component 11.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_12            (-0.6)  /** Min matrix component 12.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_20            (-0.2)  /** Min matrix component 20.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_21            (-0.8)  /** Min matrix component 21.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_22            (2.0)   /** Min matrix component 22.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_0             (0)     /** Min offset component 0.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_1             (0)     /** Min offset component 1.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_2             (0)     /** Min offset component 2.. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW                      (630)   /** Predefined saturation threshold value for low matrix. range 0 - 1023. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW_MATRIX_00            (2.4)   /** Low matrix component 00.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_01            (-0.8)  /** Low matrix component 01.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_02            (-0.6)  /** Low matrix component 02.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_10            (-0.6)  /** Low matrix component 10.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_11            (2.2)   /** Low matrix component 11.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_12            (-0.6)  /** Low matrix component 12.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_20            (0.0)   /** Low matrix component 20.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_21            (-1.0)  /** Low matrix component 21.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_22            (2.0)   /** Low matrix component 22.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_0             (0)     /** Low offset component 0.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_1             (0)     /** Low offset component 1.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_2             (0)     /** Low offset component 2.. */
#define ISP_DEF_CCM_LUM_PNT_1_VALUE             (0)     /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_2_VALUE             (96)    /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_3_VALUE             (400)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_4_VALUE             (900)   /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_5_VALUE             (1023)  /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_1_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_2_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_3_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_4_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_5_FACTOR            (1.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_1_VALUE             (0)     /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_2_VALUE             (48)    /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_3_VALUE             (128)   /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_4_VALUE             (128)   /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_5_VALUE             (1023)  /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_1_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_2_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_3_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_4_FACTOR            (1.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_5_FACTOR            (1.0)   /** Saturation point 1 factor. */
#elif 0
#define ISP_DEF_CCM_C                           (256)   /** Noise compensation offset, range 0 - 1023. */
#define ISP_DEF_CCM_DNRM                        (6)     /** Color matrix threshold de-normalization factor, range 0 - 8. */
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_MIN                      (418)   /** Predefined saturation threshold value for min matrix. range 0 - 1023. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_00            (2.4)   /** Min matrix component 00.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_01            (-1.0)  /** Min matrix component 01.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_02            (-0.4)  /** Min matrix component 02.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_10            (-0.6)  /** Min matrix component 10.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_11            (2.2)   /** Min matrix component 11.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_12            (-0.6)  /** Min matrix component 12.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_20            (-0.2)  /** Min matrix component 20.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_21            (-0.8)  /** Min matrix component 21.. */
#define ISP_DEF_CCM_SM_MIN_MATRIX_22            (2.0)   /** Min matrix component 22.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_0             (0)     /** Min offset component 0.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_1             (0)     /** Min offset component 1.. */
#define ISP_DEF_CCM_SM_MIN_OFFSET_2             (0)     /** Min offset component 2.. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
  #if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW                      (630)   /** Predefined saturation threshold value for low matrix. range 0 - 1023. */
  #endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#define ISP_DEF_CCM_SM_LOW_MATRIX_00            (2.4)   /** Low matrix component 00.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_01            (-0.8)  /** Low matrix component 01.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_02            (-0.6)  /** Low matrix component 02.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_10            (-0.6)  /** Low matrix component 10.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_11            (2.2)   /** Low matrix component 11.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_12            (-0.6)  /** Low matrix component 12.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_20            (0.0)   /** Low matrix component 20.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_21            (-1.0)  /** Low matrix component 21.. */
#define ISP_DEF_CCM_SM_LOW_MATRIX_22            (2.0)   /** Low matrix component 22.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_0             (0)     /** Low offset component 0.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_1             (0)     /** Low offset component 1.. */
#define ISP_DEF_CCM_SM_LOW_OFFSET_2             (0)     /** Low offset component 2.. */
#define ISP_DEF_CCM_LUM_PNT_1_VALUE             (0)     /** Luminance point 1 value. */
#define ISP_DEF_CCM_LUM_PNT_2_VALUE             (96)    /** Luminance point 2 value. */
#define ISP_DEF_CCM_LUM_PNT_3_VALUE             (400)   /** Luminance point 3 value. */
#define ISP_DEF_CCM_LUM_PNT_4_VALUE             (900)   /** Luminance point 4 value. */
#define ISP_DEF_CCM_LUM_PNT_5_VALUE             (1023)  /** Luminance point 5 value. */
#define ISP_DEF_CCM_LUM_PNT_1_FACTOR            (0.0)   /** Luminance point 1 factor. */
#define ISP_DEF_CCM_LUM_PNT_2_FACTOR            (1.0)   /** Luminance point 2 factor. */
#define ISP_DEF_CCM_LUM_PNT_3_FACTOR            (1.0)   /** Luminance point 3 factor. */
#define ISP_DEF_CCM_LUM_PNT_4_FACTOR            (0.0)   /** Luminance point 4 factor. */
#define ISP_DEF_CCM_LUM_PNT_5_FACTOR            (0.0)   /** Luminance point 5 factor. */
#define ISP_DEF_CCM_SAT_PNT_1_VALUE             (0)     /** Saturation point 1 value. */
#define ISP_DEF_CCM_SAT_PNT_2_VALUE             (48)    /** Saturation point 2 value. */
#define ISP_DEF_CCM_SAT_PNT_3_VALUE             (128)   /** Saturation point 3 value. */
#define ISP_DEF_CCM_SAT_PNT_4_VALUE             (128)   /** Saturation point 4 value. */
#define ISP_DEF_CCM_SAT_PNT_5_VALUE             (1023)  /** Saturation point 5 value. */
#define ISP_DEF_CCM_SAT_PNT_1_FACTOR            (0.0)   /** Saturation point 1 factor. */
#define ISP_DEF_CCM_SAT_PNT_2_FACTOR            (1.0)   /** Saturation point 2 factor. */
#define ISP_DEF_CCM_SAT_PNT_3_FACTOR            (1.0)   /** Saturation point 3 factor. */
#define ISP_DEF_CCM_SAT_PNT_4_FACTOR            (1.0)   /** Saturation point 4 factor. */
#define ISP_DEF_CCM_SAT_PNT_5_FACTOR            (1.0)   /** Saturation point 5 factor. */
#endif


#pragma pack(push, 1)

typedef enum _EN_CCM_UPDATE {
    EN_CCM_UPDATE_NONE = 0x00,
    EN_CCM_UPDATE_MATRIX_INFO = 0x01,
    EN_CCM_UPDATE_ALL = (EN_CCM_UPDATE_MATRIX_INFO),
} EN_CCM_UPDATE, *PEN_CCM_UPDATE;


typedef struct _ST_CCM_INTERPOLATE_CRV {
#if 0
    STF_U16 u16C;                               /** Noise compensation offset, range 0 - 1023. */
#endif
    STF_U8 u8Dnrm;                              /** Color matrix threshold de-normalization factor, range 0 - 8. */
    STF_S16 s16SmMin;                           /** Predefined min saturation threshold value. range 0 - 1023. */
    STF_S16 s16SmLow;                           /** Predefined low saturation threshold value. range 0 - 1023. */
} ST_CCM_INTERPOLATE_CRV, *PST_CCM_INTERPOLATE_CRV;

typedef struct _ST_CCM_INFO {
#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    STF_S16 s16Threshold;                       /** Predefined saturation threshold value. range -1024 - 1023. */
#endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    STF_DOUBLE dMatrix[3][3];                   /** Color correction matrix, range -3.9921875 - 3.9921875. */
    STF_S16 s16Offset[3];                       /** R, G and B channel offset, range -1024 - 1023. */
} ST_CCM_INFO, *PST_CCM_INFO;

#if defined(ENABLE_CCM_SUPPORT_FADING_CURVE)
typedef struct _ST_CCM_PNT {
    STF_U16 u16Value;                           /** Luminance value of fading curve. */
    STF_DOUBLE dFactor;                         /** Factor value of fading curve. */
} ST_CCM_PNT, *PST_CCM_PNT;

#endif //ENABLE_CCM_SUPPORT_FADING_CURVE
typedef struct _ST_MATRIX_INFO {
#if 0
    STF_U16 u16C;                               /** Noise compensation offset, range 0 - 1023. */
#endif
#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    STF_U8 u8Dnrm;                              /** Color matrix threshold de-normalization factor, range 0 - 8. */
#endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    ST_CCM_INFO stSmMin;                        /** This CCM is based on the lab measured color correction matrix and offsets. */
#endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
    ST_CCM_INFO stSmLow;                        /** This CCM is low end color correction matrix and offsets. */
} ST_MATRIX_INFO, *PST_MATRIX_INFO;

typedef struct _ST_MOD_CCM_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CCM module. */
    ST_MATRIX_INFO stMatrixInfo;                /** Color matrix information. */
#if defined(ENABLE_CCM_SUPPORT_FADING_CURVE)
    ST_CCM_PNT stLumFadingCrv[5];               /** Luminance fading curve. */
    ST_CCM_PNT stSatFadingCrv[5];               /** Saturation fading curve. */
#endif //ENABLE_CCM_SUPPORT_FADING_CURVE
} ST_MOD_CCM_SETTING, *PST_MOD_CCM_SETTING;

typedef struct _ST_CCM_PARAM {
    ST_MOD_CCM_SETTING stSetting;               /** Module CCM setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program CCM module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_CCM_PARAM, *PST_CCM_PARAM;

#define ST_MOD_CCM_SETTING_LEN          (sizeof(ST_MOD_CCM_SETTING))
#define ST_MOD_CCM_GET_PARAM_LEN        (ST_MOD_CCM_SETTING_LEN)
#define ST_MOD_CCM_SET_PARAM_LEN        (ST_MOD_CCM_SETTING_LEN)
#define ST_CCM_PARAM_SIZE               (sizeof(ST_CCM_PARAM))
#define ST_CCM_PARAM_LEN                (ST_CCM_PARAM_SIZE)

typedef struct _ST_ISP_MOD_CCM {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstCcmRdmaBuf;                /** CCM's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_CCM_RDMA *pstIspCcmRdma;             /** CCM's RDMA structure pointer that include the ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_CCM_RDMA *pstCcmRdma;                    /** CCM's RDMA structure pointer.*/
    ST_CCM_PARAM *pstCcmParam;                  /** CCM's parameters structure pointer. */
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
} ST_ISP_MOD_CCM, *PST_ISP_MOD_CCM;

#pragma pack(pop)


/* color correction matrix module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CCM_InitRdma(
    ST_CCM_RDMA *pstCcmRdma,
    ST_CCM_PARAM *pstCcmParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CCM_UpdateRdma(
    ST_CCM_RDMA *pstCcmRdma,
    ST_CCM_PARAM *pstCcmParam
    );
extern
STF_S32 STFMOD_ISP_CCM_UpdateMatrixInfo(
    ST_CCM_RDMA *pstCcmRdma,
    ST_CCM_PARAM *pstCcmParam
    );
extern
STF_S32 STFMOD_ISP_CCM_SetNextRdma(
    ST_CCM_RDMA *pstCcmRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CCM_SetEnable(
    ST_ISP_MOD_CCM *pstModCcm,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CCM_SetUpdate(
    ST_ISP_MOD_CCM *pstModCcm,
    STF_BOOL8 bUpdate
    );
#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
extern
STF_S32 STFMOD_ISP_CCM_SetCcmInterpolationCurve(
    ST_ISP_MOD_CCM *pstModCcm,
    ST_CCM_INTERPOLATE_CRV *pstCcmInterpolateCrv
    );
extern
STF_S32 STFMOD_ISP_CCM_SetMinMatrixOffset(
    ST_ISP_MOD_CCM *pstModCcm,
    STF_DOUBLE dMatrix[3][3],
    STF_S16 s16Offset[3]
    );
#endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
extern
STF_S32 STFMOD_ISP_CCM_SetLowMatrixOffset(
    ST_ISP_MOD_CCM *pstModCcm,
    STF_DOUBLE dMatrix[3][3],
    STF_S16 s16Offset[3]
    );
extern
STF_S32 STFMOD_ISP_CCM_SetMatrixInfo(
    ST_ISP_MOD_CCM *pstModCcm,
    ST_MATRIX_INFO *pstMatrixInfo
    );
#if 0
extern
STF_S32 STFMOD_ISP_CCM_SetNoiseCompersationOffset(
    ST_ISP_MOD_CCM *pstModCcm,
    STF_U16 u16C
    ST_ISP_MOD_CCM *pstModCcm
    );
#endif //#if 0
#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
extern
STF_S32 STFMOD_ISP_CCM_SetDenormalizationFactor(
    ST_ISP_MOD_CCM *pstModCcm,
    STF_U8 u8Dnrm
    );
extern
STF_S32 STFMOD_ISP_CCM_SetMinCcmInfo(
    ST_ISP_MOD_CCM *pstModCcm,
    ST_CCM_INFO *pstSmMin
    );
#endif //#if defined(ISP_CCM_MIN_MATRIX_SUPPORT)
extern
STF_S32 STFMOD_ISP_CCM_SetLowCcmInfo(
    ST_ISP_MOD_CCM *pstModCcm,
    ST_CCM_INFO *pstSmLow
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CCM_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CCM_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CCM_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CCM_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_CCM_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CCM_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_CCM_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CCM_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_CCM_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CCM_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CCM_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CCM_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CCM_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CCM_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CCM_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CCM_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_CCM_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_CCM_H__
