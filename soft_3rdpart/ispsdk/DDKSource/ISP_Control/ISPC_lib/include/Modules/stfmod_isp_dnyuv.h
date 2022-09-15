/**
  ******************************************************************************
  * @file  stfmod_isp_dnyuv.h
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


#ifndef __STFMOD_ISP_DNYUV_H__
#define __STFMOD_ISP_DNYUV_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_dnyuv_rdma.h"
#include "stflib_isp_pipeline.h"


/* 2D denoise DNYUV module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define DNYUV_WGHT_CNT_MAX                      (10)
#define DNYUV_CRV_PNT_ACTUAL_MAX                (6)
#define DNYUV_CRV_PNT_MAX                       (DNYUV_CRV_PNT_ACTUAL_MAX + 1)


#if 0
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_A     (0)             /** Y channel spacial weighting table A. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_B     (0)             /** Y channel spacial weighting table B. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_C     (0)             /** Y channel spacial weighting table C. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_D     (0)             /** Y channel spacial weighting table D. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_E     (1)             /** Y channel spacial weighting table E. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_F     (2)             /** Y channel spacial weighting table F. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_G     (3)             /** Y channel spacial weighting table G. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_H     (4)             /** Y channel spacial weighting table H. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_I     (4)             /** Y channel spacial weighting table I. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_J     (5)             /** Y channel spacial weighting table J. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_00          (64)            /** Y channel level 00. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_01          (128)           /** Y channel level 01. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_02          (256)           /** Y channel level 02. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_03          (384)           /** Y channel level 03. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_04          (512)           /** Y channel level 04. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_05          (640)           /** Y channel level 05. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_06          (1023)          /** Y channel level 06. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_00         (32)            /** Y channel weight 00. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_01         (16)            /** Y channel weight 01. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_02         (8)             /** Y channel weight 02. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_03         (4)             /** Y channel weight 03. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_04         (2)             /** Y channel weight 04. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_05         (1)             /** Y channel weight 05. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_06         (0)             /** Y channel weight 06. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_A    (1)             /** UV channel spacial weighting table A. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_B    (1)             /** UV channel spacial weighting table B. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_C    (1)             /** UV channel spacial weighting table C. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_D    (1)             /** UV channel spacial weighting table D. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_E    (1)             /** UV channel spacial weighting table E. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_F    (2)             /** UV channel spacial weighting table F. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_G    (3)             /** UV channel spacial weighting table G. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_H    (4)             /** UV channel spacial weighting table H. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_I    (5)             /** UV channel spacial weighting table I. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_J    (5)             /** UV channel spacial weighting table J. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_00         (64)            /** UV channel level 00. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_01         (128)           /** UV channel level 01. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_02         (256)           /** UV channel level 02. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_03         (384)           /** UV channel level 03. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_04         (512)           /** UV channel level 04. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_05         (640)           /** UV channel level 05. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_06         (1023)          /** UV channel level 06. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_00        (32)            /** UV channel weight 00. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_01        (16)            /** UV channel weight 01. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_02        (8)             /** UV channel weight 02. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_03        (4)             /** UV channel weight 03. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_04        (2)             /** UV channel weight 04. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_05        (1)             /** UV channel weight 05. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_06        (0)             /** UV channel weight 06. */
#elif 0
// 20201216_v01
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_A     (0)             /** Y channel spacial weighting table A. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_B     (0)             /** Y channel spacial weighting table B. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_C     (0)             /** Y channel spacial weighting table C. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_D     (0)             /** Y channel spacial weighting table D. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_E     (1)             /** Y channel spacial weighting table E. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_F     (2)             /** Y channel spacial weighting table F. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_G     (3)             /** Y channel spacial weighting table G. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_H     (4)             /** Y channel spacial weighting table H. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_I     (4)             /** Y channel spacial weighting table I. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_J     (5)             /** Y channel spacial weighting table J. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_00          (14)            /** Y channel level 00. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_01          (28)            /** Y channel level 01. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_02          (42)            /** Y channel level 02. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_03          (56)            /** Y channel level 03. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_04          (70)            /** Y channel level 04. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_05          (84)            /** Y channel level 05. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_06          (1023)          /** Y channel level 06. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_00         (32)            /** Y channel weight 00. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_01         (16)            /** Y channel weight 01. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_02         (8)             /** Y channel weight 02. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_03         (4)             /** Y channel weight 03. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_04         (2)             /** Y channel weight 04. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_05         (1)             /** Y channel weight 05. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_06         (0)             /** Y channel weight 06. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_A    (1)             /** UV channel spacial weighting table A. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_B    (1)             /** UV channel spacial weighting table B. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_C    (1)             /** UV channel spacial weighting table C. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_D    (1)             /** UV channel spacial weighting table D. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_E    (1)             /** UV channel spacial weighting table E. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_F    (2)             /** UV channel spacial weighting table F. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_G    (3)             /** UV channel spacial weighting table G. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_H    (4)             /** UV channel spacial weighting table H. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_I    (5)             /** UV channel spacial weighting table I. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_J    (5)             /** UV channel spacial weighting table J. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_00         (28)            /** UV channel level 00. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_01         (56)            /** UV channel level 01. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_02         (84)            /** UV channel level 02. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_03         (112)           /** UV channel level 03. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_04         (140)           /** UV channel level 04. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_05         (168)           /** UV channel level 05. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_06         (1023)          /** UV channel level 06. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_00        (32)            /** UV channel weight 00. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_01        (16)            /** UV channel weight 01. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_02        (8)             /** UV channel weight 02. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_03        (4)             /** UV channel weight 03. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_04        (2)             /** UV channel weight 04. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_05        (1)             /** UV channel weight 05. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_06        (0)             /** UV channel weight 06. */
#elif 0
// 20201219_v01
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_A     (2)             /** Y channel spacial weighting table A. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_B     (2)             /** Y channel spacial weighting table B. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_C     (2)             /** Y channel spacial weighting table C. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_D     (2)             /** Y channel spacial weighting table D. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_E     (3)             /** Y channel spacial weighting table E. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_F     (3)             /** Y channel spacial weighting table F. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_G     (3)             /** Y channel spacial weighting table G. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_H     (4)             /** Y channel spacial weighting table H. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_I     (4)             /** Y channel spacial weighting table I. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_J     (4)             /** Y channel spacial weighting table J. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_00          (40)            /** Y channel level 00. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_01          (60)            /** Y channel level 01. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_02          (80)            /** Y channel level 02. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_03          (100)           /** Y channel level 03. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_04          (120)           /** Y channel level 04. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_05          (140)           /** Y channel level 05. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_06          (1023)          /** Y channel level 06. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_00         (32)            /** Y channel weight 00. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_01         (16)            /** Y channel weight 01. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_02         (8)             /** Y channel weight 02. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_03         (4)             /** Y channel weight 03. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_04         (2)             /** Y channel weight 04. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_05         (1)             /** Y channel weight 05. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_06         (0)             /** Y channel weight 06. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_A    (1)             /** UV channel spacial weighting table A. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_B    (2)             /** UV channel spacial weighting table B. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_C    (3)             /** UV channel spacial weighting table C. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_D    (3)             /** UV channel spacial weighting table D. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_E    (3)             /** UV channel spacial weighting table E. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_F    (4)             /** UV channel spacial weighting table F. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_G    (4)             /** UV channel spacial weighting table G. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_H    (5)             /** UV channel spacial weighting table H. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_I    (5)             /** UV channel spacial weighting table I. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_J    (5)             /** UV channel spacial weighting table J. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_00         (42)            /** UV channel level 00. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_01         (56)            /** UV channel level 01. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_02         (70)            /** UV channel level 02. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_03         (84)            /** UV channel level 03. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_04         (98)            /** UV channel level 04. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_05         (112)           /** UV channel level 05. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_06         (1023)          /** UV channel level 06. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_00        (32)            /** UV channel weight 00. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_01        (16)            /** UV channel weight 01. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_02        (8)             /** UV channel weight 02. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_03        (4)             /** UV channel weight 03. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_04        (2)             /** UV channel weight 04. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_05        (1)             /** UV channel weight 05. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_06        (0)             /** UV channel weight 06. */
#elif 0
// 20210103_v01
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_A     (2)             /** Y channel spacial weighting table A. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_B     (2)             /** Y channel spacial weighting table B. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_C     (2)             /** Y channel spacial weighting table C. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_D     (2)             /** Y channel spacial weighting table D. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_E     (3)             /** Y channel spacial weighting table E. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_F     (3)             /** Y channel spacial weighting table F. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_G     (3)             /** Y channel spacial weighting table G. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_H     (4)             /** Y channel spacial weighting table H. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_I     (4)             /** Y channel spacial weighting table I. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_J     (4)             /** Y channel spacial weighting table J. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_00          (32)            /** Y channel level 00. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_01          (60)            /** Y channel level 01. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_02          (114)           /** Y channel level 02. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_03          (216)           /** Y channel level 03. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_04          (410)           /** Y channel level 04. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_05          (779)           /** Y channel level 05. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_06          (1023)          /** Y channel level 06. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_00         (32)            /** Y channel weight 00. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_01         (16)            /** Y channel weight 01. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_02         (8)             /** Y channel weight 02. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_03         (4)             /** Y channel weight 03. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_04         (2)             /** Y channel weight 04. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_05         (1)             /** Y channel weight 05. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_06         (0)             /** Y channel weight 06. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_A    (2)             /** UV channel spacial weighting table A. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_B    (2)             /** UV channel spacial weighting table B. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_C    (2)             /** UV channel spacial weighting table C. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_D    (2)             /** UV channel spacial weighting table D. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_E    (3)             /** UV channel spacial weighting table E. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_F    (3)             /** UV channel spacial weighting table F. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_G    (3)             /** UV channel spacial weighting table G. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_H    (4)             /** UV channel spacial weighting table H. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_I    (4)             /** UV channel spacial weighting table I. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_J    (4)             /** UV channel spacial weighting table J. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_00         (64)            /** UV channel level 00. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_01         (121)           /** UV channel level 01. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_02         (229)           /** UV channel level 02. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_03         (435)           /** UV channel level 03. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_04         (826)           /** UV channel level 04. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_05         (1023)          /** UV channel level 05. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_06         (1023)          /** UV channel level 06. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_00        (32)            /** UV channel weight 00. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_01        (16)            /** UV channel weight 01. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_02        (8)             /** UV channel weight 02. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_03        (4)             /** UV channel weight 03. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_04        (2)             /** UV channel weight 04. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_05        (1)             /** UV channel weight 05. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_06        (0)             /** UV channel weight 06. */
#elif 1
// 20210104_v01
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_A     (2)             /** Y channel spacial weighting table A. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_B     (2)             /** Y channel spacial weighting table B. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_C     (2)             /** Y channel spacial weighting table C. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_D     (2)             /** Y channel spacial weighting table D. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_E     (3)             /** Y channel spacial weighting table E. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_F     (3)             /** Y channel spacial weighting table F. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_G     (3)             /** Y channel spacial weighting table G. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_H     (4)             /** Y channel spacial weighting table H. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_I     (4)             /** Y channel spacial weighting table I. */
#define ISP_DEF_DNYUV_Y_S_WEIGHTING_TABLE_J     (4)             /** Y channel spacial weighting table J. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_00          (32)            /** Y channel level 00. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_01          (60)            /** Y channel level 01. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_02          (114)           /** Y channel level 02. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_03          (216)           /** Y channel level 03. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_04          (410)           /** Y channel level 04. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_05          (779)           /** Y channel level 05. */
#define ISP_DEF_DNYUV_Y_CURVE_LEVEL_06          (1023)          /** Y channel level 06. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_00         (32)            /** Y channel weight 00. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_01         (16)            /** Y channel weight 01. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_02         (8)             /** Y channel weight 02. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_03         (4)             /** Y channel weight 03. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_04         (2)             /** Y channel weight 04. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_05         (1)             /** Y channel weight 05. */
#define ISP_DEF_DNYUV_Y_CURVE_WEIGHT_06         (0)             /** Y channel weight 06. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_A    (2)             /** UV channel spacial weighting table A. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_B    (2)             /** UV channel spacial weighting table B. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_C    (2)             /** UV channel spacial weighting table C. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_D    (2)             /** UV channel spacial weighting table D. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_E    (3)             /** UV channel spacial weighting table E. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_F    (3)             /** UV channel spacial weighting table F. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_G    (3)             /** UV channel spacial weighting table G. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_H    (4)             /** UV channel spacial weighting table H. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_I    (4)             /** UV channel spacial weighting table I. */
#define ISP_DEF_DNYUV_UV_S_WEIGHTING_TABLE_J    (4)             /** UV channel spacial weighting table J. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_00         (64)            /** UV channel level 00. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_01         (115)           /** UV channel level 01. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_02         (207)           /** UV channel level 02. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_03         (372)           /** UV channel level 03. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_04         (669)           /** UV channel level 04. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_05         (1023)          /** UV channel level 05. */
#define ISP_DEF_DNYUV_UV_CURVE_LEVEL_06         (1023)          /** UV channel level 06. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_00        (32)            /** UV channel weight 00. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_01        (16)            /** UV channel weight 01. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_02        (8)             /** UV channel weight 02. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_03        (4)             /** UV channel weight 03. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_04        (2)             /** UV channel weight 04. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_05        (1)             /** UV channel weight 05. */
#define ISP_DEF_DNYUV_UV_CURVE_WEIGHT_06        (0)             /** UV channel weight 06. */
#endif


#pragma pack(push, 1)

typedef enum _EN_DNYUV_UPDATE {
    EN_DNYUV_UPDATE_NONE = 0x00,
    EN_DNYUV_UPDATE_Y_S_WEIGHTING_TABLE = 0X01,
    EN_DNYUV_UPDATE_Y_CURVE = 0X02,
    EN_DNYUV_UPDATE_UV_S_WEIGHTING_TABLE = 0X04,
    EN_DNYUV_UPDATE_UV_CURVE = 0X08,
    EN_DNYUV_UPDATE_ALL = (EN_DNYUV_UPDATE_Y_S_WEIGHTING_TABLE
                         | EN_DNYUV_UPDATE_Y_CURVE
                         | EN_DNYUV_UPDATE_UV_S_WEIGHTING_TABLE
                         | EN_DNYUV_UPDATE_UV_CURVE
                         ),
} EN_DNYUV_UPDATE, *PEN_DNYUV_UPDATE;


typedef struct _ST_DNYUV_PNT {
    STF_U16 u16Level;                           /** Level difference value, range from 0 to 1023. */
    STF_U8 u8Weighting;                         /** Level weighting value, range from 0 to 7. */
} ST_DNYUV_PNT, *PST_DNYUV_PNT;

typedef struct _ST_DNYUV_CRV {
    ST_DNYUV_PNT stYLevel[DNYUV_CRV_PNT_MAX];   /** Level difference value, range from 0 to 1023. */
    ST_DNYUV_PNT stUvLevel[DNYUV_CRV_PNT_MAX];  /** Level difference value, range from 0 to 1023. */
} ST_DNYUV_CRV, *PST_DNYUV_CRV;

typedef struct _ST_MOD_DNYUV_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable DNYUV module. */
    STF_U8 u8YSWeighting[DNYUV_WGHT_CNT_MAX];   /** Y channel (spacial) distance to center weighting value, range from 0 to 7. */
    ST_DNYUV_PNT stYCurve[DNYUV_CRV_PNT_MAX];   /** Y channel filter curve. */
    STF_U8 u8UvSWeighting[DNYUV_WGHT_CNT_MAX];  /** UV channel (spacial) distance to center weighting value, range from 0 to 7. */
    ST_DNYUV_PNT stUvCurve[DNYUV_CRV_PNT_MAX];  /** UV channel filter curve. */
} ST_MOD_DNYUV_SETTING, *PST_MOD_DNYUV_SETTING;

typedef struct _ST_DNYUV_PARAM {
    ST_MOD_DNYUV_SETTING stSetting;             /** Module DNYUV setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program DNYUV module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_DNYUV_PARAM, *PST_DNYUV_PARAM;

#define ST_MOD_DNYUV_SETTING_LEN        (sizeof(ST_MOD_DNYUV_SETTING))
#define ST_MOD_DNYUV_GET_PARAM_LEN      (ST_MOD_DNYUV_SETTING_LEN)
#define ST_MOD_DNYUV_SET_PARAM_LEN      (ST_MOD_DNYUV_SETTING_LEN)
#define ST_DNYUV_PARAM_SIZE             (sizeof(ST_DNYUV_PARAM))
#define ST_DNYUV_PARAM_LEN              (ST_DNYUV_PARAM_SIZE)

typedef struct _ST_ISP_MOD_DNYUV {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstDnYuvRdmaBuf;              /** DNYUV's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_DNYUV_RDMA *pstIspDnYuvRdma;         /** DNYUV's RDMA structure pointer that include the ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_DNYUV_RDMA *pstDnYuvRdma;                /** DNYUV's RDMA structure pointer.*/
    ST_DNYUV_PARAM *pstDnYuvParam;              /** DNYUV's parameters structure pointer. */
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
} ST_ISP_MOD_DNYUV, *PST_ISP_MOD_DNYUV;

#pragma pack(pop)


/* 2D denoise DNYUV module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DNYUV_InitRdma(
    ST_DNYUV_RDMA *pstDnYuvRdma,
    ST_DNYUV_PARAM *pstDnYuvParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DNYUV_UpdateRdma(
    ST_DNYUV_RDMA *pstDnYuvRdma,
    ST_DNYUV_PARAM *pstDnYuvParam
    );
extern
STF_S32 STFMOD_ISP_DNYUV_UpdateYSWeightingTable(
    ST_DNYUV_RDMA *pstDnYuvRdma,
    ST_DNYUV_PARAM *pstDnYuvParam
    );
extern
STF_S32 STFMOD_ISP_DNYUV_UpdateYCurve(
    ST_DNYUV_RDMA *pstDnYuvRdma,
    ST_DNYUV_PARAM *pstDnYuvParam
    );
extern
STF_S32 STFMOD_ISP_DNYUV_UpdateUvSWeightingTable(
    ST_DNYUV_RDMA *pstDnYuvRdma,
    ST_DNYUV_PARAM *pstDnYuvParam
    );
extern
STF_S32 STFMOD_ISP_DNYUV_UpdateUvCurve(
    ST_DNYUV_RDMA *pstDnYuvRdma,
    ST_DNYUV_PARAM *pstDnYuvParam);
extern
STF_S32 STFMOD_ISP_DNYUV_SetNextRdma(
    ST_DNYUV_RDMA *pstDnYuvRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DNYUV_SetEnable(
    ST_ISP_MOD_DNYUV *pstModDnYuv,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetUpdate(
    ST_ISP_MOD_DNYUV *pstModDnYuv,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetYSWeightingTable(
    ST_ISP_MOD_DNYUV *pstModDnYuv,
    STF_U8 u8YSWeighting[DNYUV_WGHT_CNT_MAX]
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetYCurve(
    ST_ISP_MOD_DNYUV *pstModDnYuv,
    ST_DNYUV_PNT stYCurve[DNYUV_CRV_PNT_MAX]
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetUvSWeightingTable(
    ST_ISP_MOD_DNYUV *pstModDnYuv,
    STF_U8 u8UvSWeighting[DNYUV_WGHT_CNT_MAX]
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetUvCurve(
    ST_ISP_MOD_DNYUV *pstModDnYuv,
    ST_DNYUV_PNT stUvCurve[DNYUV_CRV_PNT_MAX]
    );
extern
STF_U32 STFMOD_ISP_DNYUV_SetDnYuvCurve(
    ST_ISP_MOD_DNYUV *pstModDnYuv,
    ST_DNYUV_CRV *pstDnYuvCurve
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DNYUV_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DNYUV_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DNYUV_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DNYUV_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_DNYUV_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DNYUV_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_DNYUV_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DNYUV_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_DNYUV_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DNYUV_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DNYUV_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DNYUV_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DNYUV_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DNYUV_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_DNYUV_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_DNYUV_H__
