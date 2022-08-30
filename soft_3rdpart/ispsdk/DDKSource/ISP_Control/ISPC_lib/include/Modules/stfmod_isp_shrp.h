/**
  ******************************************************************************
  * @file  stfmod_isp_shrp.h
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


#ifndef __STFMOD_ISP_SHRP_H__
#define __STFMOD_ISP_SHRP_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_shrp_rdma.h"
#include "stflib_isp_pipeline.h"


/* Sharpening module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define SHRP_CRV_PNT_MAX                        (4)


#if 0
#define ISP_DEF_SHRP_WEIGHTING_TABLE_00         (0)             /** Sharpness weighting table 00. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_01         (0)             /** Sharpness weighting table 01. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_02         (0)             /** Sharpness weighting table 02. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_03         (0)             /** Sharpness weighting table 03. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_04         (0)             /** Sharpness weighting table 04. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_05         (0)             /** Sharpness weighting table 05. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_06         (0)             /** Sharpness weighting table 06. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_07         (0)             /** Sharpness weighting table 07. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_08         (0)             /** Sharpness weighting table 08. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_09         (0)             /** Sharpness weighting table 09. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_10         (0)             /** Sharpness weighting table 10. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_11         (0)             /** Sharpness weighting table 11. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_12         (0)             /** Sharpness weighting table 12. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_13         (0)             /** Sharpness weighting table 13. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_14         (15)            /** Sharpness weighting table 14. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_01            (12)            /** Difference 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_02            (24)            /** Difference 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_03            (64)            /** Difference 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_04            (96)            /** Difference 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_01          (3)             /** Factor 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_02          (4)             /** Factor 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_03          (4)             /** Factor 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_04          (2)             /** Factor 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_01             (12)            /** Difference 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_02             (24)            /** Difference 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_03             (64)            /** Difference 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_04             (96)            /** Difference 04 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_01           (3)             /** Factor 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_02           (4)             /** Factor 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_03           (4)             /** Factor 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_04           (2)             /** Factor 04 of Y mean curve. */
#define ISP_DEF_SHRP_POS_DIR_FACTOR             (1)             /** Position direction factor. */
#define ISP_DEF_SHRP_NEG_DIR_FACTOR             (1)             /** Negative direction factor. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_01          (16)            /** Difference 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_02          (32)            /** Difference 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_01        (0.99609375)    /** Factor 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_02        (0.99609375)    /** Factor 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_01      (64)            /** Difference 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_02      (160)           /** Difference 02 of UV color kill curve. */
//#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_01    (1)             /** Factor 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_02    (1)             /** Factor 02 of UV color kill curve. */
#define ISP_DEF_SHRP_EDGE_THRESHOLD             (0)             /** Edge threshold. */
#define ISP_DEF_SHRP_EDGE_ENABLE                (0)             /** Edge threshold enable. */
#elif 0
// 20201216_v01
#define ISP_DEF_SHRP_WEIGHTING_TABLE_00         (1)             /** Sharpness weighting table 00. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_01         (0)             /** Sharpness weighting table 01. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_02         (0)             /** Sharpness weighting table 02. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_03         (0)             /** Sharpness weighting table 03. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_04         (1)             /** Sharpness weighting table 04. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_05         (0)             /** Sharpness weighting table 05. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_06         (0)             /** Sharpness weighting table 06. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_07         (1)             /** Sharpness weighting table 07. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_08         (1)             /** Sharpness weighting table 08. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_09         (2)             /** Sharpness weighting table 09. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_10         (4)             /** Sharpness weighting table 10. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_11         (6)             /** Sharpness weighting table 11. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_12         (12)            /** Sharpness weighting table 12. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_13         (15)            /** Sharpness weighting table 13. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_14         (0)             /** Sharpness weighting table 14. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_01            (7)             /** Difference 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_02            (28)            /** Difference 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_03            (128)           /** Difference 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_04            (256)           /** Difference 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_01          (1)             /** Factor 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_02          (4)             /** Factor 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_03          (4)             /** Factor 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_04          (2)             /** Factor 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_01             (16)            /** Difference 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_02             (96)            /** Difference 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_03             (256)           /** Difference 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_04             (400)           /** Difference 04 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_01           (1)             /** Factor 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_02           (1)             /** Factor 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_03           (1)             /** Factor 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_04           (1)             /** Factor 04 of Y mean curve. */
#define ISP_DEF_SHRP_POS_DIR_FACTOR             (1)             /** Position direction factor. */
#define ISP_DEF_SHRP_NEG_DIR_FACTOR             (1)             /** Negative direction factor. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_01          (64)            /** Difference 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_02          (192)           /** Difference 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_01        (0.99609375)    /** Factor 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_02        (0.99609375)    /** Factor 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_01      (64)            /** Difference 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_02      (160)           /** Difference 02 of UV color kill curve. */
//#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_01    (1)             /** Factor 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_02    (1)             /** Factor 02 of UV color kill curve. */
#define ISP_DEF_SHRP_EDGE_THRESHOLD             (0)             /** Edge threshold. */
#define ISP_DEF_SHRP_EDGE_ENABLE                (0)             /** Edge threshold enable. */
#elif 0
// 20201219_v01
#define ISP_DEF_SHRP_WEIGHTING_TABLE_00         (0)             /** Sharpness weighting table 00. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_01         (0)             /** Sharpness weighting table 01. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_02         (0)             /** Sharpness weighting table 02. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_03         (0)             /** Sharpness weighting table 03. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_04         (0)             /** Sharpness weighting table 04. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_05         (0)             /** Sharpness weighting table 05. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_06         (1)             /** Sharpness weighting table 06. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_07         (1)             /** Sharpness weighting table 07. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_08         (1)             /** Sharpness weighting table 08. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_09         (4)             /** Sharpness weighting table 09. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_10         (4)             /** Sharpness weighting table 10. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_11         (4)             /** Sharpness weighting table 11. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_12         (15)            /** Sharpness weighting table 12. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_13         (15)            /** Sharpness weighting table 13. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_14         (0)             /** Sharpness weighting table 14. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_01            (7)             /** Difference 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_02            (14)            /** Difference 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_03            (60)            /** Difference 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_04            (100)           /** Difference 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_01          (3)             /** Factor 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_02          (3)             /** Factor 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_03          (3)             /** Factor 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_04          (3)             /** Factor 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_01             (16)            /** Difference 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_02             (96)            /** Difference 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_03             (256)           /** Difference 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_04             (400)           /** Difference 04 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_01           (1)             /** Factor 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_02           (1)             /** Factor 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_03           (1)             /** Factor 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_04           (1)             /** Factor 04 of Y mean curve. */
#define ISP_DEF_SHRP_POS_DIR_FACTOR             (1)             /** Position direction factor. */
#define ISP_DEF_SHRP_NEG_DIR_FACTOR             (1)             /** Negative direction factor. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_01          (64)            /** Difference 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_02          (192)           /** Difference 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_01        (0.99609375)    /** Factor 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_02        (0.99609375)    /** Factor 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_01      (64)            /** Difference 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_02      (160)           /** Difference 02 of UV color kill curve. */
//#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_01    (1)             /** Factor 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_02    (1)             /** Factor 02 of UV color kill curve. */
#define ISP_DEF_SHRP_EDGE_THRESHOLD             (0)             /** Edge threshold. */
#define ISP_DEF_SHRP_EDGE_ENABLE                (0)             /** Edge threshold enable. */
#elif 0
// 20201224_v01
#define ISP_DEF_SHRP_WEIGHTING_TABLE_00         (0)             /** Sharpness weighting table 00. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_01         (0)             /** Sharpness weighting table 01. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_02         (0)             /** Sharpness weighting table 02. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_03         (0)             /** Sharpness weighting table 03. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_04         (0)             /** Sharpness weighting table 04. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_05         (0)             /** Sharpness weighting table 05. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_06         (1)             /** Sharpness weighting table 06. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_07         (1)             /** Sharpness weighting table 07. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_08         (1)             /** Sharpness weighting table 08. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_09         (4)             /** Sharpness weighting table 09. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_10         (4)             /** Sharpness weighting table 10. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_11         (4)             /** Sharpness weighting table 11. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_12         (15)            /** Sharpness weighting table 12. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_13         (15)            /** Sharpness weighting table 13. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_14         (0)             /** Sharpness weighting table 14. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_01            (14)            /** Difference 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_02            (28)            /** Difference 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_03            (64)            /** Difference 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_04            (96)            /** Difference 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_01          (3)             /** Factor 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_02          (3)             /** Factor 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_03          (3)             /** Factor 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_04          (3)             /** Factor 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_01             (16)            /** Difference 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_02             (96)            /** Difference 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_03             (256)           /** Difference 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_04             (400)           /** Difference 04 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_01           (1)             /** Factor 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_02           (1)             /** Factor 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_03           (1)             /** Factor 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_04           (1)             /** Factor 04 of Y mean curve. */
#define ISP_DEF_SHRP_POS_DIR_FACTOR             (1)             /** Position direction factor. */
#define ISP_DEF_SHRP_NEG_DIR_FACTOR             (1)             /** Negative direction factor. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_01          (64)            /** Difference 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_02          (192)           /** Difference 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_01        (0.99609375)    /** Factor 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_02        (0.99609375)    /** Factor 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_01      (64)            /** Difference 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_02      (160)           /** Difference 02 of UV color kill curve. */
//#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_01    (1)             /** Factor 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_02    (1)             /** Factor 02 of UV color kill curve. */
#define ISP_DEF_SHRP_EDGE_THRESHOLD             (0)             /** Edge threshold. */
#define ISP_DEF_SHRP_EDGE_ENABLE                (0)             /** Edge threshold enable. */
#elif 0
// 20201228_v01
#define ISP_DEF_SHRP_WEIGHTING_TABLE_00         (0)             /** Sharpness weighting table 00. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_01         (0)             /** Sharpness weighting table 01. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_02         (0)             /** Sharpness weighting table 02. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_03         (0)             /** Sharpness weighting table 03. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_04         (0)             /** Sharpness weighting table 04. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_05         (0)             /** Sharpness weighting table 05. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_06         (1)             /** Sharpness weighting table 06. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_07         (1)             /** Sharpness weighting table 07. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_08         (1)             /** Sharpness weighting table 08. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_09         (4)             /** Sharpness weighting table 09. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_10         (4)             /** Sharpness weighting table 10. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_11         (4)             /** Sharpness weighting table 11. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_12         (15)            /** Sharpness weighting table 12. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_13         (15)            /** Sharpness weighting table 13. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_14         (0)             /** Sharpness weighting table 14. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_01            (7)             /** Difference 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_02            (30)            /** Difference 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_03            (50)            /** Difference 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_04            (100)           /** Difference 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_01          (3)             /** Factor 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_02          (3)             /** Factor 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_03          (3)             /** Factor 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_04          (3)             /** Factor 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_01             (16)            /** Difference 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_02             (96)            /** Difference 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_03             (256)           /** Difference 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_04             (400)           /** Difference 04 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_01           (1)             /** Factor 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_02           (1)             /** Factor 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_03           (1)             /** Factor 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_04           (1)             /** Factor 04 of Y mean curve. */
#define ISP_DEF_SHRP_POS_DIR_FACTOR             (1)             /** Position direction factor. */
#define ISP_DEF_SHRP_NEG_DIR_FACTOR             (1)             /** Negative direction factor. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_01          (64)            /** Difference 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_02          (192)           /** Difference 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_01        (0.99609375)    /** Factor 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_02        (0.99609375)    /** Factor 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_01      (64)            /** Difference 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_02      (160)           /** Difference 02 of UV color kill curve. */
//#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_01    (1)             /** Factor 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_02    (1)             /** Factor 02 of UV color kill curve. */
#define ISP_DEF_SHRP_EDGE_THRESHOLD             (0)             /** Edge threshold. */
#define ISP_DEF_SHRP_EDGE_ENABLE                (0)             /** Edge threshold enable. */
#elif 0
// 20210103_v01
#define ISP_DEF_SHRP_WEIGHTING_TABLE_00         (0)             /** Sharpness weighting table 00. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_01         (0)             /** Sharpness weighting table 01. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_02         (1)             /** Sharpness weighting table 02. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_03         (1)             /** Sharpness weighting table 03. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_04         (1)             /** Sharpness weighting table 04. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_05         (2)             /** Sharpness weighting table 05. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_06         (4)             /** Sharpness weighting table 06. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_07         (4)             /** Sharpness weighting table 07. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_08         (4)             /** Sharpness weighting table 08. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_09         (8)             /** Sharpness weighting table 09. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_10         (8)             /** Sharpness weighting table 10. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_11         (8)             /** Sharpness weighting table 11. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_12         (15)            /** Sharpness weighting table 12. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_13         (15)            /** Sharpness weighting table 13. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_14         (0)             /** Sharpness weighting table 14. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_01            (7)             /** Difference 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_02            (24)            /** Difference 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_03            (128)            /** Difference 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_04            (256)           /** Difference 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_01          (2)             /** Factor 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_02          (4)             /** Factor 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_03          (4)             /** Factor 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_04          (2)             /** Factor 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_01             (16)            /** Difference 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_02             (96)            /** Difference 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_03             (256)           /** Difference 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_04             (400)           /** Difference 04 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_01           (1)             /** Factor 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_02           (1)             /** Factor 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_03           (1)             /** Factor 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_04           (1)             /** Factor 04 of Y mean curve. */
#define ISP_DEF_SHRP_POS_DIR_FACTOR             (1)             /** Position direction factor. */
#define ISP_DEF_SHRP_NEG_DIR_FACTOR             (1)             /** Negative direction factor. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_01          (64)            /** Difference 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_02          (192)           /** Difference 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_01        (0.99609375)    /** Factor 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_02        (0.99609375)    /** Factor 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_01      (64)            /** Difference 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_02      (160)           /** Difference 02 of UV color kill curve. */
//#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_01    (1)             /** Factor 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_02    (1)             /** Factor 02 of UV color kill curve. */
#define ISP_DEF_SHRP_EDGE_THRESHOLD             (0)             /** Edge threshold. */
#define ISP_DEF_SHRP_EDGE_ENABLE                (0)             /** Edge threshold enable. */
#elif 1
// 20210104_v01
#define ISP_DEF_SHRP_WEIGHTING_TABLE_00         (0)             /** Sharpness weighting table 00. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_01         (0)             /** Sharpness weighting table 01. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_02         (1)             /** Sharpness weighting table 02. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_03         (1)             /** Sharpness weighting table 03. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_04         (1)             /** Sharpness weighting table 04. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_05         (2)             /** Sharpness weighting table 05. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_06         (4)             /** Sharpness weighting table 06. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_07         (4)             /** Sharpness weighting table 07. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_08         (4)             /** Sharpness weighting table 08. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_09         (8)             /** Sharpness weighting table 09. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_10         (8)             /** Sharpness weighting table 10. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_11         (8)             /** Sharpness weighting table 11. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_12         (15)            /** Sharpness weighting table 12. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_13         (15)            /** Sharpness weighting table 13. */
#define ISP_DEF_SHRP_WEIGHTING_TABLE_14         (0)             /** Sharpness weighting table 14. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_01            (7)             /** Difference 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_02            (24)            /** Difference 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_03            (128)            /** Difference 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_DIFF_04            (256)           /** Difference 04 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_01          (3)             /** Factor 01 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_02          (4)             /** Factor 02 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_03          (4)             /** Factor 03 of Y level curve. */
#define ISP_DEF_SHRP_Y_LEVEL_FACTOR_04          (3)             /** Factor 04 of Y level curve. */
#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
#define ISP_DEF_SHRP_Y_MEAN_DIFF_01             (16)            /** Difference 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_02             (96)            /** Difference 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_03             (256)           /** Difference 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_DIFF_04             (400)           /** Difference 04 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_01           (1)             /** Factor 01 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_02           (1)             /** Factor 02 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_03           (1)             /** Factor 03 of Y mean curve. */
#define ISP_DEF_SHRP_Y_MEAN_FACTOR_04           (1)             /** Factor 04 of Y mean curve. */
#endif //#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
#define ISP_DEF_SHRP_POS_DIR_FACTOR             (1.0)           /** Position direction factor. */
#define ISP_DEF_SHRP_NEG_DIR_FACTOR             (1.0)           /** Negative direction factor. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_01          (64)            /** Difference 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_DIFF_02          (192)           /** Difference 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_01        (0.99609375)    /** Factor 01 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_SMOOTH_FACTOR_02        (0.99609375)    /** Factor 02 of UV smooth curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_01      (64)            /** Difference 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_DIFF_02      (160)           /** Difference 02 of UV color kill curve. */
//#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_01    (1)             /** Factor 01 of UV color kill curve. */
#define ISP_DEF_SHRP_UV_COLOR_KILL_FACTOR_02    (1)             /** Factor 02 of UV color kill curve. */
#define ISP_DEF_SHRP_EDGE_THRESHOLD             (0)             /** Edge threshold. */
#define ISP_DEF_SHRP_EDGE_ENABLE                (0)             /** Edge threshold enable. */
#endif


#pragma pack(push, 1)

typedef enum _EN_SHRP_UPDATE {
    EN_SHRP_UPDATE_NONE = 0x0000,
    EN_SHRP_UPDATE_WEIGHTING_TABLE = 0X0001,
    EN_SHRP_UPDATE_Y_LEVEL_CURVE = 0X0002,
#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
    EN_SHRP_UPDATE_Y_MEAN_CURVE = 0X0004,
#endif //#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
    EN_SHRP_UPDATE_DIRECTION_FACTOR = 0X0008,
    EN_SHRP_UPDATE_UV_SMOOTH_CURVE = 0X0010,
    EN_SHRP_UPDATE_UV_COLOR_KILL_CURVE = 0X0020,
    EN_SHRP_UPDATE_EDGE_PARAM = 0X0040,
    EN_SHRP_UPDATE_ALL = (EN_SHRP_UPDATE_WEIGHTING_TABLE
                        | EN_SHRP_UPDATE_Y_LEVEL_CURVE
#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
                        | EN_SHRP_UPDATE_Y_MEAN_CURVE
#endif //#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
                        | EN_SHRP_UPDATE_DIRECTION_FACTOR
                        | EN_SHRP_UPDATE_UV_SMOOTH_CURVE
                        | EN_SHRP_UPDATE_UV_COLOR_KILL_CURVE
                        | EN_SHRP_UPDATE_EDGE_PARAM
                        ),
} EN_SHRP_UPDATE, *PEN_SHRP_UPDATE;


typedef struct _ST_SHRP_PNT {
    STF_U16 u16Diff;
    STF_DOUBLE dFactor;
} ST_SHRP_PNT, *PST_SHRP_PNT;

typedef struct _ST_SHRP_CRV {
    ST_SHRP_PNT stYLevel[SHRP_CRV_PNT_MAX];
} ST_SHRP_CRV, *PST_SHRP_CRV;

typedef struct _ST_MOD_SHRP_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable SHRP module. */
    STF_U8 u8Weighting[15];                     /** Sharpness weighting table. */
    ST_SHRP_PNT stYLevel[SHRP_CRV_PNT_MAX];     /** Y level curve. This is a strength curve base on the Y level different. */
#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
    ST_SHRP_PNT stYMean[SHRP_CRV_PNT_MAX];      /** Y mean curve. */
#endif //#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
    STF_DOUBLE dPosDirFactor;                   /** Position direction factor. */
    STF_DOUBLE dNegDirFactor;                   /** Negative direction factor. */
#if defined(ENABLE_SHRP_UV_CHN_SUPPORT)
    ST_SHRP_PNT stUvSmooth[2];                  /** UV channel smooth curve base on the Y level different. */
    ST_SHRP_PNT stUvColorKill[2];               /** UV channel color kill curve. */
#endif //#if defined(ENABLE_SHRP_UV_CHN_SUPPORT)
#if defined(ENABLE_SHRP_EDGE_SUPPORT)
    STF_U16 u16EdgeThreshold;                   /** Edge threshold. */
    STF_BOOL8 bEdgeEnable;                      /** Edge threshold enable. */
#endif //#if defined(ENABLE_SHRP_EDGE_SUPPORT)
} ST_MOD_SHRP_SETTING, *PST_MOD_SHRP_SETTING;

typedef struct _ST_SHRP_PARAM {
    ST_MOD_SHRP_SETTING stSetting;              /** Module SHRP setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program SHRP module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_SHRP_PARAM, *PST_SHRP_PARAM;

#define ST_MOD_SHRP_SETTING_LEN         (sizeof(ST_MOD_SHRP_SETTING))
#define ST_MOD_SHRP_GET_PARAM_LEN       (ST_MOD_SHRP_SETTING_LEN)
#define ST_MOD_SHRP_SET_PARAM_LEN       (ST_MOD_SHRP_SETTING_LEN)
#define ST_SHRP_PARAM_SIZE              (sizeof(ST_SHRP_PARAM))
#define ST_SHRP_PARAM_LEN               (ST_SHRP_PARAM_SIZE)

typedef struct _ST_ISP_MOD_SHRP {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstShrpRdmaBuf;               /** SHRP's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_SHRP_RDMA *pstIspShrpRdma;           /** SHRP's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_SHRP_RDMA *pstShrpRdma;                  /** SHRP's RDMA structure pointer.*/
    ST_SHRP_PARAM *pstShrpParam;                /** SHRP's parameters structure pointer. */
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
} ST_ISP_MOD_SHRP, *PST_ISP_MOD_SHRP;

#pragma pack(pop)


/* Sharpening module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SHRP_InitRdma(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SHRP_UpdateRdma(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
extern
STF_S32 STFMOD_ISP_SHRP_UpdateWeightingTable(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
extern
STF_S32 STFMOD_ISP_SHRP_UpdateYLevelCurve(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_UpdateYMeanCurve(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
#endif //#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_UpdateDirectionFactor(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
#if defined(ENABLE_SHRP_UV_CHN_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_UpdateUvSmoothCurve(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
extern
STF_S32 STFMOD_ISP_SHRP_UpdateUvColorKillCurve(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
#endif //#if defined(ENABLE_SHRP_UV_CHN_SUPPORT)
#if defined(ENABLE_SHRP_EDGE_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_UpdateEdgeParam(
    ST_SHRP_RDMA *pstShrpRdma,
    ST_SHRP_PARAM *pstShrpParam
    );
#endif //#if defined(ENABLE_SHRP_EDGE_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_SetNextRdma(
    ST_SHRP_RDMA *pstShrpRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SHRP_SetEnable(
    ST_ISP_MOD_SHRP *pstModShrp,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_SHRP_SetUpdate(
    ST_ISP_MOD_SHRP *pstModShrp,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_SHRP_SetWeightingTable(
    ST_ISP_MOD_SHRP *pstModShrp,
    STF_U8 u8Weighting[]
    );
extern
STF_S32 STFMOD_ISP_SHRP_SetYLevelCurve(
    ST_ISP_MOD_SHRP *pstModShrp,
    ST_SHRP_PNT stYLevel[SHRP_CRV_PNT_MAX]
    );
#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_SetYMeanCurve(
    ST_ISP_MOD_SHRP *pstModShrp,
    ST_SHRP_PNT stYMean[SHRP_CRV_PNT_MAX]
    );
#endif //#if defined(ENABLE_SHRP_Y_MEAN_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_SetDirectionFactor(
    ST_ISP_MOD_SHRP *pstModShrp,
    STF_DOUBLE dPosDirFactor,
    STF_DOUBLE dNegDirFactor
    );
#if defined(ENABLE_SHRP_UV_CHN_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_SetUvSmoothCurve(
    ST_ISP_MOD_SHRP *pstModShrp,
    ST_SHRP_PNT stUvSmooth[2]
    );
extern
STF_S32 STFMOD_ISP_SHRP_SetUvColorKillCurve(
    ST_ISP_MOD_SHRP *pstModShrp,
    ST_SHRP_PNT stUvColorKill[2]
    );
#endif //#if defined(ENABLE_SHRP_UV_CHN_SUPPORT)
#if defined(ENABLE_SHRP_EDGE_SUPPORT)
extern
STF_S32 STFMOD_ISP_SHRP_SetEdgeParam(
    ST_ISP_MOD_SHRP *pstModShrp,
    STF_BOOL8 bEdgeEnable,
    STF_U16 u16EdgeThreshold
    );
#endif //#if defined(ENABLE_SHRP_EDGE_SUPPORT)
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SHRP_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SHRP_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SHRP_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SHRP_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_SHRP_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SHRP_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_SHRP_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SHRP_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_SHRP_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SHRP_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SHRP_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SHRP_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SHRP_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SHRP_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SHRP_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SHRP_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_SHRP_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_SHRP_H__
