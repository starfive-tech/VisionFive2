/**
  ******************************************************************************
  * @file  stfmod_isp_sc.h
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


#ifndef __STFMOD_ISP_SC_H__
#define __STFMOD_ISP_SC_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_sc_rdma.h"
#include "stflib_isp_pipeline.h"


#define SWITCH_WS_WEIGHTING_TABLE_ROW_COL


/* statistics collection module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define SC_DUMP_ITEM_NEXT_SEGMENT_OFFSET        (0x0400)

#define ISP_DEF_SC_USING_IMG_SIZE_FOR_CALC      (STF_TRUE)      /** Turn this flag to using image size to calculate the AE/AF/AWB decimation information. */
#define ISP_DEF_SC_IMAGE_WIDTH                  (ISP_IMG_WIDTH) /** Image width. */
#define ISP_DEF_SC_IMAGE_HEIGHT                 (ISP_IMG_HEIGHT) /** Image Height. */
#define ISP_DEF_SC_DUMPING_BUFFER_ADDRESS       (0x68000000)    /** Point to SC dumping buffer(8 x 16byte align), need buffer size is 4532 x 4byte. */
#define ISP_DEF_SC_AXI_ID                       (1)             /** SC dumping AXI ID. */
#define ISP_DEF_SC_CROP_DEC_H_START             (0)             /** Horizontal starting point for frame cropping. */
#define ISP_DEF_SC_CROP_DEC_V_START             (12)            /** Vertical starting point for frame cropping. */
#define ISP_DEF_SC_CROP_DEC_SUB_WIN_W           (29)            /** Width of SC sub-window. (u8SubWinWidth + 1) */
#define ISP_DEF_SC_CROP_DEC_SUB_WIN_H           (21)            /** Height of SC sub-window (u8SubWinHeight + 1) */
#define ISP_DEF_SC_CROP_DEC_H_PERIOD            (7)             /** Horizontal period(zero base) for input image decimation. */
#define ISP_DEF_SC_CROP_DEC_H_KEEP              (1)             /** Horizontal Keep(zero base) for input image decimation. */
#define ISP_DEF_SC_CROP_DEC_V_PERIOD            (5)             /** Vertical period(zero base) for input image decimation. */
#define ISP_DEF_SC_CROP_DEC_V_KEEP              (1)             /** Vertical Keep(zero base) for input image decimation. */
#define ISP_DEF_SC_AE_AF_CFG_INPUT_SEL          (EN_SC_IN_SEL_AFTER_AWB) /** SC input source select for AE/AF. 0 : after DEC, 1 : after OBC, 2 : after OECF, 3 : after AWB. */
#define ISP_DEF_SC_AF_ES_HOR_MODE               (0)             /** Horizontal mode. */
#define ISP_DEF_SC_AF_ES_SUM_MODE               (0)             /** Summation mode.  0:Absolute sum, 1:Squared sum */
#define ISP_DEF_SC_AF_HOR_EN                    (0)             /** Horizontal enable. */
#define ISP_DEF_SC_AF_VER_EN                    (1)             /** Vertical enable. */
#define ISP_DEF_SC_AF_ES_VER_THR                (191)           /** Vertical threshold. */
#define ISP_DEF_SC_AF_ES_HOR_THR                (497)           /** Horizontal threshold. */
#define ISP_DEF_SC_AWB_CFG_INPUT_SEL            (EN_SC_IN_SEL_AFTER_OECF) /** SC input source select for AWB. 0 : after DEC, 1 : after OBC, 2 : after OECF, 3 : after AWB. */
#define ISP_DEF_SC_AWB_PS_RL                    (0)             /** SC AWB pixel summation lower boundary of R value. */
#define ISP_DEF_SC_AWB_PS_RU                    (255)           /** SC AWB pixel summation upper boundary of R value. */
#define ISP_DEF_SC_AWB_PS_GL                    (0)             /** SC AWB pixel summation lower boundary of G value. */
#define ISP_DEF_SC_AWB_PS_GU                    (255)           /** SC AWB pixel summation upper boundary of G value. */
#define ISP_DEF_SC_AWB_PS_BL                    (0)             /** SC AWB pixel summation lower boundary of B value. */
#define ISP_DEF_SC_AWB_PS_BU                    (255)           /** SC AWB pixel summation upper boundary of B value. */
#define ISP_DEF_SC_AWB_PS_YL                    (0)             /** SC AWB pixel summation lower boundary of Y value. */
#define ISP_DEF_SC_AWB_PS_YU                    (255)           /** SC AWB pixel summation upper boundary of Y value. */
#define ISP_DEF_SC_AWB_PS_GRL                   (0)             /** SC AWB pixel summation lower boundary of G/R value. */
#define ISP_DEF_SC_AWB_PS_GRU                   (255.99609375)  /** SC AWB pixel summation upper boundary of G/R value. */
#define ISP_DEF_SC_AWB_PS_GBL                   (0)             /** SC AWB pixel summation lower boundary of G/B value. */
#define ISP_DEF_SC_AWB_PS_GBU                   (255.99609375)  /** SC AWB pixel summation upper boundary of G/B value. */
#define ISP_DEF_SC_AWB_PS_GRBL                  (0)             /** SC AWB pixel summation lower boundary of (Gr/R + b/a * Gb/B) value. */
#define ISP_DEF_SC_AWB_PS_GRBU                  (255.99609375)  /** SC AWB pixel summation upper boundary of (Gr/R + b/a * Gb/B) value. */
//#define ISP_DEF_SC_AWB_PS_GRB_B_A               (15.9375)       /** SC AWB pixel summation coefficient b/a value for GRB range, range form 0 to 15.9375. */
//#define ISP_DEF_SC_AWB_PS_GRB_B_A               (4.75)          /** SC AWB pixel summation coefficient b/a value for GRB range, range form 0 to 15.9375. */
//#define ISP_DEF_SC_AWB_PS_GRB_B_A               (0.5)           /** SC AWB pixel summation coefficient b/a value for GRB range, range form 0 to 15.9375. */
#define ISP_DEF_SC_AWB_PS_GRB_B_A               (1.0)           /** SC AWB pixel summation coefficient b/a value for GRB range, range form 0 to 15.9375. */
//#define ISP_DEF_SC_AWB_PS_GRB_B_A               (0.0)           /** SC AWB pixel summation coefficient b/a value for GRB range, range form 0 to 15.9375. */
#define ISP_DEF_SC_AWB_WS_RL                    (0)             /** SC AWB weighting summation lower boundary of R value. */
#define ISP_DEF_SC_AWB_WS_RU                    (255)           /** SC AWB weighting summation upper boundary of R value. */
#define ISP_DEF_SC_AWB_WS_GRL                   (0)             /** SC AWB weighting summation lower boundary of Gr value. */
#define ISP_DEF_SC_AWB_WS_GRU                   (255)           /** SC AWB weighting summation upper boundary of Gr value. */
#define ISP_DEF_SC_AWB_WS_GBL                   (0)             /** SC AWB weighting summation lower boundary of Gb value. */
#define ISP_DEF_SC_AWB_WS_GBU                   (255)           /** SC AWB weighting summation upper boundary of Gb value. */
#define ISP_DEF_SC_AWB_WS_BL                    (0)             /** SC AWB weighting summation lower boundary of B value. */
#define ISP_DEF_SC_AWB_WS_BU                    (255)           /** SC AWB weighting summation upper boundary of B value. */
#define ISP_DEF_SC_AWB_CW_00_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_00_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_01_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_02_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_03_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_04_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_05_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_06_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_07_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_08_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_09_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_10_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_11_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_00                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_01                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_02                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_03                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_04                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_05                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_06                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_07                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_08                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_09                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_10                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_11                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_CW_12_12                 (15)            /** SC AWB color weighting table, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_00                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_01                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_02                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_03                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_04                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_05                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_06                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_07                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_08                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_09                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_10                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_11                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_12                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_13                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_14                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_15                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */
#define ISP_DEF_SC_AWB_IW_16                    (15)            /** SC AWB intensity weighting curve, range from 0 to 15. */


#pragma pack(push, 1)

typedef enum _EN_SC_UPDATE {
    EN_SC_UPDATE_NONE = 0x0000,
    EN_SC_UPDATE_SC_DUMPING_AXI_ID = 0x0001,
    EN_SC_UPDATE_SC_CROPPING_DECIMATION = 0x0002,
    EN_SC_UPDATE_SC_AE_AF_CFG = 0x0004,
    EN_SC_UPDATE_SC_AF_CFG = 0x0008,
    EN_SC_UPDATE_SC_AWB_CFG = 0x0010,
    EN_SC_UPDATE_SC_AWB_PS_CFG = 0x0020,
    EN_SC_UPDATE_SC_AWB_WS_CFG = 0x0040,
    EN_SC_UPDATE_SC_AWB_WEIGHTING_TABLE = 0x0080,
    EN_SC_UPDATE_SC_AWB_WEIGHTING_CURVE = 0x0100,
    EN_SC_UPDATE_SC_DUMPING_ADDRESS = 0x0400,
    EN_SC_UPDATE_SC_DUMPING_CFG_WO_BUF_INFO = (EN_SC_UPDATE_SC_DUMPING_AXI_ID),
    EN_SC_UPDATE_SC_DUMPING_CFG = (EN_SC_UPDATE_SC_DUMPING_AXI_ID
                                 | EN_SC_UPDATE_SC_DUMPING_ADDRESS
                                 ),
    EN_SC_UPDATE_SC_AE_AF_PARAM = (EN_SC_UPDATE_SC_AE_AF_CFG
                                 | EN_SC_UPDATE_SC_AF_CFG
                                 ),
    EN_SC_UPDATE_SC_AWB_PARAM = (EN_SC_UPDATE_SC_AWB_CFG
                               | EN_SC_UPDATE_SC_AWB_PS_CFG
                               | EN_SC_UPDATE_SC_AWB_WS_CFG
                               | EN_SC_UPDATE_SC_AWB_WEIGHTING_TABLE
                               | EN_SC_UPDATE_SC_AWB_WEIGHTING_CURVE
                               ),
    EN_SC_UPDATE_ALL_WO_BUF_INFO = (EN_SC_UPDATE_SC_DUMPING_CFG_WO_BUF_INFO
                      | EN_SC_UPDATE_SC_CROPPING_DECIMATION
                      | EN_SC_UPDATE_SC_AE_AF_PARAM
                      | EN_SC_UPDATE_SC_AWB_PARAM
                      ),
    EN_SC_UPDATE_ALL = (EN_SC_UPDATE_SC_DUMPING_CFG
                      | EN_SC_UPDATE_SC_CROPPING_DECIMATION
                      | EN_SC_UPDATE_SC_AE_AF_PARAM
                      | EN_SC_UPDATE_SC_AWB_PARAM
                      ),
} EN_SC_UPDATE, *PEN_SC_UPDATE;


//typedef enum _EN_SC_IN_SEL {
//    EN_SC_IN_SEL_AFTER_DEC = 0,                 /** 0 : after DEC. */
//    EN_SC_IN_SEL_AFTER_OBC,                     /** 1 : after OBC. */
//    EN_SC_IN_SEL_AFTER_OECF,                    /** 2 : after OECF. */
//    EN_SC_IN_SEL_AFTER_AWB,                     /** 3 : after AWB WB Gain. */
//    //EN_SC_IN_SEL_MAX
//} EN_SC_IN_SEL, *PEN_SC_IN_SEL;                 /** SC input MUX for AE/AF/AWB. */
//
//
typedef struct _ST_SC_AE_WS_R_G {
    STF_U32 u32R;
    STF_U32 u32G;
} ST_SC_AE_WS_R_G, *PST_SC_AE_WS_R_G;

typedef struct _ST_SC_AE_WS_B_Y {
    STF_U32 u32B;
    STF_U32 u32Y;
} ST_SC_AE_WS_B_Y, *PST_SC_AE_WS_B_Y;

typedef struct _ST_SC_AWB_PS_R_G {
    STF_U32 u32R;
    STF_U32 u32G;
} ST_SC_AWB_PS_R_G, *PST_SC_AWB_PS_R_G;

typedef struct _ST_SC_AWB_PS_B_CNT {
    STF_U32 u32B;
    STF_U32 u32CNT;
} ST_SC_AWB_PS_B_CNT, *PST_SC_AWB_PS_B_CNT;

typedef struct _ST_SC_AWB_WGS_W_RW {
    STF_U32 u32W;
    STF_U32 u32RW;
} ST_SC_AWB_WGS_W_RW, *PST_SC_AWB_WGS_W_RW;

typedef struct _ST_SC_AWB_WGS_GW_BW {
    STF_U32 u32GW;
    STF_U32 u32BW;
} ST_SC_AWB_WGS_GW_BW, *PST_SC_AWB_WGS_GW_BW;

typedef struct _ST_SC_AWB_WGS_GRW_GBW {
    STF_U32 u32GRW;
    STF_U32 u32GBW;
} ST_SC_AWB_WGS_GRW_GBW, *PST_SC_AWB_WGS_GRW_GBW;

typedef struct _ST_SC_AF_ES_DAT_CNT {
    STF_U32 u32DAT;
    STF_U32 u32CNT;
} ST_SC_AF_ES_DAT_CNT, *PST_SC_AF_ES_DAT_CNT;

typedef struct _ST_SC_AE_HIST_R_G {
    STF_U32 u32R;
    STF_U32 u32G;
} ST_SC_AE_HIST_R_G, *PST_SC_AE_HIST_R_G;

typedef struct _ST_SC_AE_HIST_B_Y {
    STF_U32 u32B;
    STF_U32 u32Y;
} ST_SC_AE_HIST_B_Y, *PST_SC_AE_HIST_B_Y;

typedef struct _ST_SC_DATA
{
    ST_SC_AE_WS_R_G AeWsRG[16];
    ST_SC_AE_WS_B_Y AeWsBY[16];
    ST_SC_AWB_PS_R_G AwbPsRG[16];
    ST_SC_AWB_PS_B_CNT AwbPsBCNT[16];
    ST_SC_AWB_WGS_W_RW AwbWgsWRW[16];
    ST_SC_AWB_WGS_GW_BW AwbWgsGWBW[16];
    ST_SC_AWB_WGS_GRW_GBW AwbWgsGRWGBW[16];
    ST_SC_AF_ES_DAT_CNT AfEsDATCNT[16];
} ST_SC_DATA, *PST_SC_DATA;

typedef struct _ST_SC_HIST
{
    ST_SC_AE_HIST_R_G AeHistRG[64];
    ST_SC_AE_HIST_B_Y AeHistBY[64];
} ST_SC_HIST, *PST_SC_HIST;

typedef struct _ST_SC_DUMP
{
    ST_SC_DATA ScData[16];
    ST_SC_HIST ScHist;
} ST_SC_DUMP, *PST_SC_DUMP;

typedef struct _ST_SC_CROP_DEC {
    STF_U16 u16HStart;                          /** Horizontal starting point for SC cropping. Since hardware bug issue, H_Start cannot less than 4. */
    STF_U16 u16VStart;                          /** Vertical starting point for SC cropping. Since hardware bug issue, V_Start cannot less than 4. */
    STF_U8 u8SubWinWidth;                       /** Width of SC sub-window, u8SubWinWidth have to be odd number. (u8SubWinWidth + 1) */
    STF_U8 u8SubWinHeight;                      /** Height of SC sub-window, u8SubWinHeight have to be odd number. (u8SubWinHeight + 1) */
    STF_U8 u8HPeriod;                           /** Horizontal period(zero base) for input image decimation. */
    STF_U8 u8HKeep;                             /** Horizontal Keep(zero base) for input image decimation. */
    STF_U8 u8VPeriod;                           /** Vertical period(zero base) for input image decimation. */
    STF_U8 u8VKeep;                             /** Vertical Keep(zero base) for input image decimation. */
    ST_SIZE stDecSize;                          /** stDecSize is the size after SC decimation, it's only for check. */
} ST_SC_CROP_DEC, *PST_SC_CROP_DEC;

typedef struct _ST_SC_DUMP_CFG {
    STF_U8 u8AxiId;                             /** ID of AXI bus for SC dumping buffer write. */
    STF_U32 *pu32ScDumpBuf;                     /** Point to SC dumping buffer(8 x 16byte align), need buffer size is 4532 x 4byte. */
} ST_SC_DUMP_CFG, *PST_SC_DUMP_CFG;

#if 0
typedef struct _ST_SC_READ_CTRL {
    STF_U8 u8ScItemIdx;                         /** SC item select index.
                                                 * 0: ae_ws_r
                                                 * 1: ae_ws_g
                                                 * 2: ae_ws_b
                                                 * 3: ae_ws_y
                                                 * 4: awb_ps_r
                                                 * 5: awb_ps_g
                                                 * 6: awb_ps_b
                                                 * 7: awb_ps_cnt
                                                 * 8: awb_wgs_w
                                                 * 9: awb_wgs_rw
                                                 * 10: awb_wgs_gw
                                                 * 11: awb_wgs_bw
                                                 * 12: awb_wgs_grw
                                                 * 13: awb_wgs_gbw
                                                 * 14: af_es
                                                 * 15: ae_es
                                                 * 16: ae_hist_r
                                                 * 17: ae_hist_g
                                                 * 18: ae_hist_b
                                                 * 19: ae_hist_y
                                                 */
    STF_U8 u8Addr;                              /** Indicate the SC data offset address. */
    STF_U32 u32Data;                            /** Using to store the SC return data. (Read-Only) */
} ST_SC_READ_CTRL, *PST_SC_READ_CTRL;

#endif
typedef struct _ST_SC_AE_AF_CFG {
    STF_U8 u8InputSel;                          /** SC input source select for AE/AF. 0 : after DEC, 1 : after OBC, 2 : after OECF, 3 : after AWB WB Gain. */
} ST_SC_AE_AF_CFG, *PST_SC_AE_AF_CFG;

typedef struct _ST_SC_AF_CFG {
    STF_U8 u8EsHorMode;                         /** Horizontal mode. */
    STF_U8 u8EsSumMode;                         /** Summation mode.  0:Absolute sum, 1:Squared sum. */
    STF_BOOL8 bHorEn;                            /** Horizontal enable. */
    STF_BOOL8 bVerEn;                            /** Vertical enable. */
    STF_U8 u8EsVerThr;                          /** Vertical threshold. */
    STF_U16 u16EsHorThr;                        /** Horizontal threshold. */
} ST_SC_AF_CFG, *PST_SC_AF_CFG;

typedef struct _ST_SC_AE_AF_PARAM {
    ST_SC_AF_CFG stAfCfg;                       /** AF configuration for SC. */
} ST_SC_AE_AF_PARAM, *PST_SC_AE_AF_PARAM;

typedef struct _ST_SC_AWB_CFG {
    STF_U8 u8InputSel;                          /** SC input source select for AWB. 0 : after DEC, 1 : after OBC, 2 : after OECF, 3 : after AWB WB Gain. */
} ST_SC_AWB_CFG, *PST_SC_AWB_CFG;

typedef struct _ST_SC_AWB_PS_CFG {
    STF_U8 u8AwbPsRL;                           /** SC AWB pixel summation lower boundary of R value. */
    STF_U8 u8AwbPsRU;                           /** SC AWB pixel summation upper boundary of R value. */
    STF_U8 u8AwbPsGL;                           /** SC AWB pixel summation lower boundary of G value. */
    STF_U8 u8AwbPsGU;                           /** SC AWB pixel summation upper boundary of G value. */
    STF_U8 u8AwbPsBL;                           /** SC AWB pixel summation lower boundary of B value. */
    STF_U8 u8AwbPsBU;                           /** SC AWB pixel summation upper boundary of B value. */
    STF_U8 u8AwbPsYL;                           /** SC AWB pixel summation lower boundary of Y value. */
    STF_U8 u8AwbPsYU;                           /** SC AWB pixel summation upper boundary of Y value. */
    STF_DOUBLE dAwbPsGRL;                       /** SC AWB pixel summation lower boundary of G/R value. */
    STF_DOUBLE dAwbPsGRU;                       /** SC AWB pixel summation upper boundary of G/R value. */
    STF_DOUBLE dAwbPsGBL;                       /** SC AWB pixel summation lower boundary of G/B value. */
    STF_DOUBLE dAwbPsGBU;                       /** SC AWB pixel summation upper boundary of G/B value. */
    STF_DOUBLE dAwbPsGRBL;                      /** SC AWB pixel summation lower boundary of (Gr/R + b/a * Gb/B) value. */
    STF_DOUBLE dAwbPsGRBU;                      /** SC AWB pixel summation upper boundary of (Gr/R + b/a * Gb/B) value. */
    STF_DOUBLE dAwbPsGrbBa;                     /** SC AWB pixel summation coefficient b/a value for GRB range, range form 0 to 15.9375. */
} ST_SC_AWB_PS_CFG, *PST_SC_AWB_PS_CFG;

typedef struct _ST_SC_AWB_WS_CFG {
    STF_U8 u8AwbWsRL;                           /** SC AWB weighting summation lower boundary of R value. */
    STF_U8 u8AwbWsRU;                           /** SC AWB weighting summation upper boundary of R value. */
    STF_U8 u8AwbWsGrL;                          /** SC AWB weighting summation lower boundary of Gr value. */
    STF_U8 u8AwbWsGrU;                          /** SC AWB weighting summation upper boundary of Gr value. */
    STF_U8 u8AwbWsGbL;                          /** SC AWB weighting summation lower boundary of Gb value. */
    STF_U8 u8AwbWsGbU;                          /** SC AWB weighting summation upper boundary of Gb value. */
    STF_U8 u8AwbWsBL;                           /** SC AWB weighting summation lower boundary of B value. */
    STF_U8 u8AwbWsBU;                           /** SC AWB weighting summation upper boundary of B value. */
} ST_SC_AWB_WS_CFG, *PST_SC_AWB_WS_CFG;

typedef struct _ST_AWB_IW_PNT {
    STF_U16 u16Intensity;                       /** SC AWB intensity value. Range from 0 to 256. */
    STF_U8 u8Weight;                            /** SC AWB intensity weight value. Range from 0 to 15. */
} ST_AWB_IW_PNT, *PST_AWB_IW_PNT;

typedef struct _ST_SC_AWB_PARAM {
    ST_SC_AWB_WS_CFG stAwbWsCfg;                /** AWB weighting summation method configuration for SC. */
    STF_U8 u8AwbCW[13][13];                     /** SC AWB color weighting table, range from 0 to 15. */
    ST_AWB_IW_PNT stAwbIWCurve[17];             /** SC AWB intensity weighting curve, weight range from 0 to 15. */
} ST_SC_AWB_PARAM, *PST_SC_AWB_PARAM;

typedef struct _ST_SC_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable SC AE/AF/AWB module. */
#if 0
    STF_BOOL8 bEnableScAeAf;                    /** Enable/Disable SC AE/AF module. */
    STF_BOOL8 bEnableScAwb;                     /** Enable/Disable SC AWB module. */
#endif
    ST_SC_AE_AF_PARAM stScAeAfParam;            /** AE and AF parameters for SC. */
    ST_SC_AWB_PARAM stScAwbParam;               /** AWB parameters for SC. */
} ST_SC_SETTING, *PST_SC_SETTING;

typedef struct _ST_MOD_SC_INFO {
    ST_SIZE stImgSize;                          /** Image size. */
    ST_SC_CROP_DEC stCropDec;                   /** Input image cropping and decimation for SC.
                                                 * You don't need to set this structure
                                                 * value, it will be calculated and
                                                 * assigned by call
                                                 * STFMOD_ISP_SC_UpdateScDecimation()
                                                 * function.
                                                 */
} ST_MOD_SC_INFO, *PST_MOD_SC_INFO;

typedef struct _ST_SC_PARAM {
    ST_SC_SETTING stSetting;                    /** Module SC setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_SC_INFO stInfo;                      /** Module SC output information parameters. */
    STF_BOOL8 bUsingImageSizeForCalc;           /** Enable this flag to using the assign image size to calculate the offset, max window number and the scale down factor. */
    ST_SC_AE_AF_CFG stAeAfCfg;                  /** AE and AF configuration for SC. */
    ST_SC_AWB_CFG stAwbCfg;                     /** AWB configuration for SC. */
    ST_SC_AWB_PS_CFG stAwbPsCfg;                /** AWB pixel summation method configuration for SC. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program SC module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
#if 0
    ST_SC_READ_CTRL stScReadCtrl;               /** SC result read back control by CPU method. */
#endif
    ST_SC_DUMP_CFG stScDumpCfg;                 /** SC dumping configure. */
} ST_SC_PARAM, *PST_SC_PARAM;

#define ST_MOD_SC_SETTING_LEN           (sizeof(ST_MOD_SC_SETTING))
#define ST_MOD_SC_INFO_LEN              (sizeof(ST_MOD_SC_INFO))
#define ST_MOD_SC_GET_PARAM_LEN         (ST_MOD_SC_SETTING_LEN + ST_MOD_SC_INFO_LEN)
#define ST_MOD_SC_SET_PARAM_LEN         (ST_MOD_SC_SETTING_LEN)
#define ST_SC_PARAM_SIZE                (sizeof(ST_SC_PARAM) - sizeof(STF_U32 *))
#define ST_SC_PARAM_LEN                 (ST_SC_PARAM_SIZE)

typedef struct _ST_ISP_MOD_SC {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstScRdmaBuf;                 /** SC's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_SC_RDMA *pstIspScRdma;               /** SC's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_SC_RDMA *pstScRdma;                      /** SC's RDMA structure pointer.*/
    ST_SC_PARAM *pstScParam;                    /** SC's parameters structure pointer. */
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
} ST_ISP_MOD_SC, *PST_ISP_MOD_SC;

#pragma pack(pop)


/* statistics collection module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CalcScDecimation(
    ST_SIZE stImgSize,
    ST_SC_CROP_DEC *pstScCropDec
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_InitRdma(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateRdma(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_UpdateScDumpingCfg(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateScDumpingAddress(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateScDumpingAxiId(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateScCroppingDecimation(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_UpdateScAeAfRdma(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateAeAfCfg(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateAfCfg(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_UpdateScAwbRdma(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateAwbCfg(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateAwbPsCfg(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateAwbWsCfg(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateAwbWeightingTable(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateAwbWeightingCurve(
    ST_SC_RDMA *pstScRdma,
    ST_SC_PARAM *pstScParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_SetNextRdma(
    ST_SC_RDMA *pstScRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
#if 1
extern
STF_S32 STFMOD_ISP_SC_SetEnable(
    ST_ISP_MOD_SC *pstModSc,
    STF_BOOL8 bEnable
    );
#else
extern
STF_S32 STFMOD_ISP_SC_SetEnableScAeAf(
    ST_ISP_MOD_SC *pstModSc,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_SC_SetEnableScAwb(
    ST_ISP_MOD_SC *pstModSc,
    STF_BOOL8 bEnable
    );
#endif
extern
STF_S32 STFMOD_ISP_SC_SetUpdate(
    ST_ISP_MOD_SC *pstModSc,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_SC_SetEnableUsingImageSizeForCalc(
    ST_ISP_MOD_SC *pstModSc,
    STF_BOOL8 bUsingImageSizeForCalc
    );
extern
STF_S32 STFMOD_ISP_SC_SetImageSize(
    ST_ISP_MOD_SC *pstModSc,
    ST_SIZE stImgSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_SetScDumpingCfg(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_DUMP_CFG *pstScDumpCfg
    );
extern
STF_S32 STFMOD_ISP_SC_SetScDumpingCfgBuffer(
    ST_ISP_MOD_SC *pstModSc,
    STF_U32 *pu32ScDumpBuf
    );
extern
STF_S32 STFMOD_ISP_SC_SetScDumpingCfgAxiId(
    ST_ISP_MOD_SC *pstModSc,
    STF_U8 u8AxiId
    );
extern
STF_S32 STFMOD_ISP_SC_SetScCroppingDecimation(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_CROP_DEC *pstCropDec
    );
//-----------------------------------------------------------------------------
#if 0
extern
STF_S32 STFMOD_ISP_SC_SetScReadCtrl(
    ST_ISP_MOD_SC *pstModSc,
    ST_DUMP_RSLT_CTRL *pstDumpResultCtrl
    );
//-----------------------------------------------------------------------------
#endif
extern
STF_S32 STFMOD_ISP_SC_SetScAeAfParam(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_AE_AF_PARAM *pstScAeAfParam
    );
extern
STF_S32 STFMOD_ISP_SC_SetAeAfCfg(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_AE_AF_CFG *pstAeAfCfg
    );
extern
STF_S32 STFMOD_ISP_SC_SetAfCfg(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_AF_CFG *pstAfCfg
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_SetScAwbParam(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_AWB_PARAM *pstScAwbParam
    );
extern
STF_S32 STFMOD_ISP_SC_SetAwbCfg(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_AWB_CFG *pstAwbCfg
    );
extern
STF_S32 STFMOD_ISP_SC_SetAwbPsCfg(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_AWB_PS_CFG *pstAwbPsCfg
    );
extern
STF_S32 STFMOD_ISP_SC_SetAwbWsCfg(
    ST_ISP_MOD_SC *pstModSc,
    ST_SC_AWB_WS_CFG *pstAwbWsCfg
    );
extern
STF_S32 STFMOD_ISP_SC_SetAwbWeightingTable(
    ST_ISP_MOD_SC *pstModSc,
    STF_U8 u8AwbCW[13][13]
    );
extern
STF_S32 STFMOD_ISP_SC_SetAwbWeightingCurve(
    ST_ISP_MOD_SC *pstModSc,
    ST_AWB_IW_PNT stAwbIWCurve[17]
    );
extern
STF_S32 STFMOD_ISP_SC_SetAwbWeightingCurveWeight(
    ST_ISP_MOD_SC *pstModSc,
    STF_U8 u8Weight[17]
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_GetSubWinPixCnt(
    STF_VOID *pIspCtx,
    STF_U32 *pu32PixCnt
    );
extern
STF_S32 STFMOD_ISP_SC_SetScDumpingBuffer(
    STF_VOID *pIspCtx,
    STF_U32 *pu32ScDumpBuf
    );
extern
STF_S32 STFMOD_ISP_SC_GetScDumpingBuffer(
    STF_VOID *pIspCtx,
    STF_U32 **ppu32ScDumpBuf
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_SetInputSel(
    STF_VOID *pIspCtx,
    STF_U8 u8AeAfInputSel,
    STF_U8 u8AwbInputSel
    );
//-----------------------------------------------------------------------------
#if 0
extern
STF_BOOL8 STFMOD_ISP_SC_IsSc0Enable(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_SC_IsSc1Enable(
    STF_VOID *pIspCtx
    );
//-----------------------------------------------------------------------------
#endif
extern
STF_S32 STFMOD_ISP_SC_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SC_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SC_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
#if 1
STF_S32 STFMOD_ISP_SC_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
#else
extern
STF_S32 STFMOD_ISP_SC_EnableAeAf(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_SC_EnableAwb(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
#endif
extern
STF_BOOL8 STFMOD_ISP_SC_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SC_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_SC_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SC_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_SC_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SC_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SC_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SC_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_SC_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_SC_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_SC_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_SC_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_SC_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_SC_H__
