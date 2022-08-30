/**
  ******************************************************************************
  * @file  stf_isp_lcbq_rdma.h
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
#ifndef __STF_ISP_LCBQ_RDMA_H__
#define __STF_ISP_LCBQ_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add lens correction bi-quadratic registers structure */
typedef struct _ST_LCBQ_WIN_REG {
    REG_LCBQ_CFG_0 SubWindowSize;       // 0x0074, LCBQ pseudo sub-window register.
} ST_LCBQ_WIN_REG, *PST_LCBQ_WIN_REG;

#define ST_LCBQ_WIN_REG_SIZE        (sizeof(ST_LCBQ_WIN_REG))
#define ST_LCBQ_WIN_REG_LEN         (ST_LCBQ_WIN_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCBQ_COOR_REG {
    REG_LCBQ_CFG_1 Coordinate;          // 0x007C, LCBQ X/Y coordinate register.
} ST_LCBQ_COOR_REG, *PST_LCBQ_COOR_REG;

#define ST_LCBQ_COOR_REG_SIZE       (sizeof(ST_LCBQ_COOR_REG))
#define ST_LCBQ_COOR_REG_LEN        (ST_LCBQ_COOR_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCBQ_CFG_REG {
    REG_LCBQ_CFG_0 SubWindowSize;       // 0x0074, LCBQ pseudo sub-window register.
    STF_U32 u32Reserved_0078[1];        // Added reserved variables to let ST_LCBQ_REG address is contiguous.
    REG_LCBQ_CFG_1 Coordinate;          // 0x007C, LCBQ X/Y coordinate register.
} ST_LCBQ_CFG_REG, *PST_LCBQ_CFG_REG;

#define ST_LCBQ_CFG_REG_SIZE        (sizeof(ST_LCBQ_CFG_REG))
#define ST_LCBQ_CFG_REG_LEN         (ST_LCBQ_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCBQ_GAIN_REG {
    REG_LCBQ_GAIN0_CFG_0 G0_1;          // 0x0300, LCBQ [0.1.8] Gain 0 and 1 parameter register.
    REG_LCBQ_GAIN0_CFG_1 G2_3;          // 0x0304, LCBQ [0.1.8] Gain 2 and 3 parameter register.
    REG_LCBQ_GAIN0_CFG_2 G4_5;          // 0x0308, LCBQ [0.1.8] Gain 4 and 5 parameter register.
    REG_LCBQ_GAIN0_CFG_3 G6_7;          // 0x030C, LCBQ [0.1.8] Gain 6 and 7 parameter register.
    REG_LCBQ_GAIN0_CFG_4 G8_9;          // 0x0310, LCBQ [0.1.8] Gain 8 and 9 parameter register.
    REG_LCBQ_GAIN0_CFG_5 G10_11;        // 0x0314, LCBQ [0.1.8] Gain 10 and 11 parameter register.
    REG_LCBQ_GAIN0_CFG_6 G12_13;        // 0x0318, LCBQ [0.1.8] Gain 12 and 13 parameter register.
    REG_LCBQ_GAIN0_CFG_7 G14_15;        // 0x031C, LCBQ [0.1.8] Gain 14 and 15 parameter register.
    REG_LCBQ_GAIN0_CFG_8 G16_17;        // 0x0320, LCBQ [0.1.8] Gain 16 and 17 parameter register.
    REG_LCBQ_GAIN0_CFG_9 G18_19;        // 0x0324, LCBQ [0.1.8] Gain 18 and 19 parameter register.
    REG_LCBQ_GAIN0_CFG_10 G20_21;       // 0x0328, LCBQ [0.1.8] Gain 20 and 21 parameter register.
    REG_LCBQ_GAIN0_CFG_11 G22_23;       // 0x032C, LCBQ [0.1.8] Gain 22 and 23 parameter register.
    REG_LCBQ_GAIN0_CFG_12 G24;          // 0x0330, LCBQ [0.1.8] Gain 24 parameter register.
} ST_LCBQ_GAIN_REG, *PST_LCBQ_GAIN_REG;

#define ST_LCBQ_GAIN_REG_SIZE       (sizeof(ST_LCBQ_GAIN_REG))
#define ST_LCBQ_GAIN_REG_LEN        (ST_LCBQ_GAIN_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCBQ_GAIN_ALL_REG {
#if 0
    REG_LCBQ_GAIN0_CFG_0 R_G0_1;        // 0x0300, LCBQ [0.1.8] R Gain 0 and 1 parameter register.
    REG_LCBQ_GAIN0_CFG_1 R_G2_3;        // 0x0304, LCBQ [0.1.8] R Gain 2 and 3 parameter register.
    REG_LCBQ_GAIN0_CFG_2 R_G4_5;        // 0x0308, LCBQ [0.1.8] R Gain 4 and 5 parameter register.
    REG_LCBQ_GAIN0_CFG_3 R_G6_7;        // 0x030C, LCBQ [0.1.8] R Gain 6 and 7 parameter register.
    REG_LCBQ_GAIN0_CFG_4 R_G8_9;        // 0x0310, LCBQ [0.1.8] R Gain 8 and 9 parameter register.
    REG_LCBQ_GAIN0_CFG_5 R_G10_11;      // 0x0314, LCBQ [0.1.8] R Gain 10 and 11 parameter register.
    REG_LCBQ_GAIN0_CFG_6 R_G12_13;      // 0x0318, LCBQ [0.1.8] R Gain 12 and 13 parameter register.
    REG_LCBQ_GAIN0_CFG_7 R_G14_15;      // 0x031C, LCBQ [0.1.8] R Gain 14 and 15 parameter register.
    REG_LCBQ_GAIN0_CFG_8 R_G16_17;      // 0x0320, LCBQ [0.1.8] R Gain 16 and 17 parameter register.
    REG_LCBQ_GAIN0_CFG_9 R_G18_19;      // 0x0324, LCBQ [0.1.8] R Gain 18 and 19 parameter register.
    REG_LCBQ_GAIN0_CFG_10 R_G20_21;     // 0x0328, LCBQ [0.1.8] R Gain 20 and 21 parameter register.
    REG_LCBQ_GAIN0_CFG_11 R_G22_23;     // 0x032C, LCBQ [0.1.8] R Gain 22 and 23 parameter register.
    REG_LCBQ_GAIN0_CFG_12 R_G24;        // 0x0330, LCBQ [0.1.8] R Gain 24 parameter register.

    REG_LCBQ_GAIN1_CFG_0 GR_G0_1;       // 0x0334, LCBQ [0.1.8] Gr Gain 0 and 1 parameter register.
    REG_LCBQ_GAIN1_CFG_1 GR_G2_3;       // 0x0338, LCBQ [0.1.8] Gr Gain 2 and 3 parameter register.
    REG_LCBQ_GAIN1_CFG_2 GR_G4_5;       // 0x033C, LCBQ [0.1.8] Gr Gain 4 and 5 parameter register.
    REG_LCBQ_GAIN1_CFG_3 GR_G6_7;       // 0x0340, LCBQ [0.1.8] Gr Gain 6 and 7 parameter register.
    REG_LCBQ_GAIN1_CFG_4 GR_G8_9;       // 0x0344, LCBQ [0.1.8] Gr Gain 8 and 9 parameter register.
    REG_LCBQ_GAIN1_CFG_5 GR_G10_11;     // 0x0348, LCBQ [0.1.8] Gr Gain 10 and 11 parameter register.
    REG_LCBQ_GAIN1_CFG_6 GR_G12_13;     // 0x034C, LCBQ [0.1.8] Gr Gain 12 and 13 parameter register.
    REG_LCBQ_GAIN1_CFG_7 GR_G14_15;     // 0x0350, LCBQ [0.1.8] Gr Gain 14 and 15 parameter register.
    REG_LCBQ_GAIN1_CFG_8 GR_G16_17;     // 0x0354, LCBQ [0.1.8] Gr Gain 16 and 17 parameter register.
    REG_LCBQ_GAIN1_CFG_9 GR_G18_19;     // 0x0358, LCBQ [0.1.8] Gr Gain 18 and 19 parameter register.
    REG_LCBQ_GAIN1_CFG_10 GR_G20_21;    // 0x035C, LCBQ [0.1.8] Gr Gain 20 and 21 parameter register.
    REG_LCBQ_GAIN1_CFG_11 GR_G22_23;    // 0x0360, LCBQ [0.1.8] Gr Gain 22 and 23 parameter register.
    REG_LCBQ_GAIN1_CFG_12 GR_G24;       // 0x0364, LCBQ [0.1.8] Gr Gain 24 parameter register.

    REG_LCBQ_GAIN2_CFG_0 GB_G0_1;       // 0x0368, LCBQ [0.1.8] Gb Gain 0 and 1 parameter register.
    REG_LCBQ_GAIN2_CFG_1 GB_G2_3;       // 0x036C, LCBQ [0.1.8] Gb Gain 2 and 3 parameter register.
    REG_LCBQ_GAIN2_CFG_2 GB_G4_5;       // 0x0370, LCBQ [0.1.8] Gb Gain 4 and 5 parameter register.
    REG_LCBQ_GAIN2_CFG_3 GB_G6_7;       // 0x0374, LCBQ [0.1.8] Gb Gain 6 and 7 parameter register.
    REG_LCBQ_GAIN2_CFG_4 GB_G8_9;       // 0x0378, LCBQ [0.1.8] Gb Gain 8 and 9 parameter register.
    REG_LCBQ_GAIN2_CFG_5 GB_G10_11;     // 0x037C, LCBQ [0.1.8] Gb Gain 10 and 11 parameter register.
    REG_LCBQ_GAIN2_CFG_6 GB_G12_13;     // 0x0380, LCBQ [0.1.8] Gb Gain 12 and 13 parameter register.
    REG_LCBQ_GAIN2_CFG_7 GB_G14_15;     // 0x0384, LCBQ [0.1.8] Gb Gain 14 and 15 parameter register.
    REG_LCBQ_GAIN2_CFG_8 GB_G16_17;     // 0x0388, LCBQ [0.1.8] Gb Gain 16 and 17 parameter register.
    REG_LCBQ_GAIN2_CFG_9 GB_G18_19;     // 0x038C, LCBQ [0.1.8] Gb Gain 18 and 19 parameter register.
    REG_LCBQ_GAIN2_CFG_10 GB_G20_21;    // 0x0390, LCBQ [0.1.8] Gb Gain 20 and 21 parameter register.
    REG_LCBQ_GAIN2_CFG_11 GB_G22_23;    // 0x0394, LCBQ [0.1.8] Gb Gain 22 and 23 parameter register.
    REG_LCBQ_GAIN2_CFG_12 GB_G24;       // 0x0398, LCBQ [0.1.8] Gb Gain 24 parameter register.

    REG_LCBQ_GAIN3_CFG_0 B_G0_1;        // 0x039C, LCBQ [0.1.8] B Gain 0 and 1 parameter register.
    REG_LCBQ_GAIN3_CFG_1 B_G2_3;        // 0x03A0, LCBQ [0.1.8] B Gain 2 and 3 parameter register.
    REG_LCBQ_GAIN3_CFG_2 B_G4_5;        // 0x03A4, LCBQ [0.1.8] B Gain 4 and 5 parameter register.
    REG_LCBQ_GAIN3_CFG_3 B_G6_7;        // 0x03A8, LCBQ [0.1.8] B Gain 6 and 7 parameter register.
    REG_LCBQ_GAIN3_CFG_4 B_G8_9;        // 0x03AC, LCBQ [0.1.8] B Gain 8 and 9 parameter register.
    REG_LCBQ_GAIN3_CFG_5 B_G10_11;      // 0x03B0, LCBQ [0.1.8] B Gain 10 and 11 parameter register.
    REG_LCBQ_GAIN3_CFG_6 B_G12_13;      // 0x03B4, LCBQ [0.1.8] B Gain 12 and 13 parameter register.
    REG_LCBQ_GAIN3_CFG_7 B_G14_15;      // 0x03B8, LCBQ [0.1.8] B Gain 14 and 15 parameter register.
    REG_LCBQ_GAIN3_CFG_8 B_G16_17;      // 0x03BC, LCBQ [0.1.8] B Gain 16 and 17 parameter register.
    REG_LCBQ_GAIN3_CFG_9 B_G18_19;      // 0x03C0, LCBQ [0.1.8] B Gain 18 and 19 parameter register.
    REG_LCBQ_GAIN3_CFG_10 B_G20_21;     // 0x03C4, LCBQ [0.1.8] B Gain 20 and 21 parameter register.
    REG_LCBQ_GAIN3_CFG_11 B_G22_23;     // 0x03C8, LCBQ [0.1.8] B Gain 22 and 23 parameter register.
    REG_LCBQ_GAIN3_CFG_12 B_G24;        // 0x03CC, LCBQ [0.1.8] B Gain 24 parameter register.
#else
    ST_LCBQ_GAIN_REG stRGain;           // 0x0300 - 0x0333, LCBQ [0.1.8] Gain parameter register for symbol 0.
    ST_LCBQ_GAIN_REG stGrGain;          // 0x0334 - 0x0367, LCBQ [0.1.8] Gain parameter register for symbol 1.
    ST_LCBQ_GAIN_REG stGbGain;          // 0x0368 - 0x039B, LCBQ [0.1.8] Gain parameter register for symbol 2.
    ST_LCBQ_GAIN_REG stBGain;           // 0x039C - 0x03CF, LCBQ [0.1.8] Gain parameter register for symbol 3.
#endif //#if 0
} ST_LCBQ_GAIN_ALL_REG, *PST_LCBQ_GAIN_ALL_REG;

#define ST_LCBQ_GAIN_ALL_REG_SIZE   (sizeof(ST_LCBQ_GAIN_ALL_REG))
#define ST_LCBQ_GAIN_ALL_REG_LEN    (ST_LCBQ_GAIN_ALL_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCBQ_OFST_REG {
    REG_LCBQ_OFFSET0_CFG_0 S0_1;        // 0x0400, LCBQ [1.8.0] Offset 0 and 1 parameter register.
    REG_LCBQ_OFFSET0_CFG_1 S2_3;        // 0x0404, LCBQ [1.8.0] Offset 2 and 3 parameter register.
    REG_LCBQ_OFFSET0_CFG_2 S4_5;        // 0x0408, LCBQ [1.8.0] Offset 4 and 5 parameter register.
    REG_LCBQ_OFFSET0_CFG_3 S6_7;        // 0x040C, LCBQ [1.8.0] Offset 6 and 7 parameter register.
    REG_LCBQ_OFFSET0_CFG_4 S8_9;        // 0x0410, LCBQ [1.8.0] Offset 8 and 9 parameter register.
    REG_LCBQ_OFFSET0_CFG_5 S10_11;      // 0x0414, LCBQ [1.8.0] Offset 10 and 11 parameter register.
    REG_LCBQ_OFFSET0_CFG_6 S12_13;      // 0x0418, LCBQ [1.8.0] Offset 12 and 13 parameter register.
    REG_LCBQ_OFFSET0_CFG_7 S14_15;      // 0x041C, LCBQ [1.8.0] Offset 14 and 15 parameter register.
    REG_LCBQ_OFFSET0_CFG_8 S16_17;      // 0x0420, LCBQ [1.8.0] Offset 16 and 17 parameter register.
    REG_LCBQ_OFFSET0_CFG_9 S18_19;      // 0x0424, LCBQ [1.8.0] Offset 18 and 19 parameter register.
    REG_LCBQ_OFFSET0_CFG_10 S20_21;     // 0x0428, LCBQ [1.8.0] Offset 20 and 21 parameter register.
    REG_LCBQ_OFFSET0_CFG_11 S22_23;     // 0x042C, LCBQ [1.8.0] Offset 22 and 23 parameter register.
    REG_LCBQ_OFFSET0_CFG_12 S24;        // 0x0430, LCBQ [1.8.0] Offset 24 parameter register.
} ST_LCBQ_OFST_REG, *PST_LCBQ_OFST_REG;

#define ST_LCBQ_OFST_REG_SIZE   (sizeof(ST_LCBQ_OFST_REG))
#define ST_LCBQ_OFST_REG_LEN    (ST_LCBQ_OFST_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCBQ_OFST_ALL_REG {
#if 0
    REG_LCBQ_OFFSET0_CFG_0 R_S0_1;      // 0x0400, LCBQ [1.8.0] Offset 0 and 1 parameter register.
    REG_LCBQ_OFFSET0_CFG_1 R_S2_3;      // 0x0404, LCBQ [1.8.0] Offset 2 and 3 parameter register.
    REG_LCBQ_OFFSET0_CFG_2 R_S4_5;      // 0x0408, LCBQ [1.8.0] Offset 4 and 5 parameter register.
    REG_LCBQ_OFFSET0_CFG_3 R_S6_7;      // 0x040C, LCBQ [1.8.0] Offset 6 and 7 parameter register.
    REG_LCBQ_OFFSET0_CFG_4 R_S8_9;      // 0x0410, LCBQ [1.8.0] Offset 8 and 9 parameter register.
    REG_LCBQ_OFFSET0_CFG_5 R_S10_11;    // 0x0414, LCBQ [1.8.0] Offset 10 and 11 parameter register.
    REG_LCBQ_OFFSET0_CFG_6 R_S12_13;    // 0x0418, LCBQ [1.8.0] Offset 12 and 13 parameter register.
    REG_LCBQ_OFFSET0_CFG_7 R_S14_15;    // 0x041C, LCBQ [1.8.0] Offset 14 and 15 parameter register.
    REG_LCBQ_OFFSET0_CFG_8 R_S16_17;    // 0x0420, LCBQ [1.8.0] Offset 16 and 17 parameter register.
    REG_LCBQ_OFFSET0_CFG_9 R_S18_19;    // 0x0424, LCBQ [1.8.0] Offset 18 and 19 parameter register.
    REG_LCBQ_OFFSET0_CFG_10 R_S20_21;   // 0x0428, LCBQ [1.8.0] Offset 20 and 21 parameter register.
    REG_LCBQ_OFFSET0_CFG_11 R_S22_23;   // 0x042C, LCBQ [1.8.0] Offset 22 and 23 parameter register.
    REG_LCBQ_OFFSET0_CFG_12 R_S24;      // 0x0430, LCBQ [1.8.0] Offset 24 parameter register.

    REG_LCBQ_OFFSET1_CFG_0 GR_S0_1;     // 0x0434, LCBQ [1.8.0] Offset 0 and 1 parameter register.
    REG_LCBQ_OFFSET1_CFG_1 GR_S2_3;     // 0x0438, LCBQ [1.8.0] Offset 2 and 3 parameter register.
    REG_LCBQ_OFFSET1_CFG_2 GR_S4_5;     // 0x043C, LCBQ [1.8.0] Offset 4 and 5 parameter register.
    REG_LCBQ_OFFSET1_CFG_3 GR_S6_7;     // 0x0440, LCBQ [1.8.0] Offset 6 and 7 parameter register.
    REG_LCBQ_OFFSET1_CFG_4 GR_S8_9;     // 0x0444, LCBQ [1.8.0] Offset 8 and 9 parameter register.
    REG_LCBQ_OFFSET1_CFG_5 GR_S10_11;   // 0x0448, LCBQ [1.8.0] Offset 10 and 11 parameter register.
    REG_LCBQ_OFFSET1_CFG_6 GR_S12_13;   // 0x044C, LCBQ [1.8.0] Offset 12 and 13 parameter register.
    REG_LCBQ_OFFSET1_CFG_7 GR_S14_15;   // 0x0450, LCBQ [1.8.0] Offset 14 and 15 parameter register.
    REG_LCBQ_OFFSET1_CFG_8 GR_S16_17;   // 0x0454, LCBQ [1.8.0] Offset 16 and 17 parameter register.
    REG_LCBQ_OFFSET1_CFG_9 GR_S18_19;   // 0x0458, LCBQ [1.8.0] Offset 18 and 19 parameter register.
    REG_LCBQ_OFFSET1_CFG_10 GR_S20_21;  // 0x045C, LCBQ [1.8.0] Offset 20 and 21 parameter register.
    REG_LCBQ_OFFSET1_CFG_11 GR_S22_23;  // 0x0460, LCBQ [1.8.0] Offset 22 and 23 parameter register.
    REG_LCBQ_OFFSET1_CFG_12 GR_S24;     // 0x0464, LCBQ [1.8.0] Offset 24 parameter register.

    REG_LCBQ_OFFSET2_CFG_0 GB_S0_1;     // 0x0468, LCBQ [1.8.0] Offset 0 and 1 parameter register.
    REG_LCBQ_OFFSET2_CFG_1 GB_S2_3;     // 0x046C, LCBQ [1.8.0] Offset 2 and 3 parameter register.
    REG_LCBQ_OFFSET2_CFG_2 GB_S4_5;     // 0x0470, LCBQ [1.8.0] Offset 4 and 5 parameter register.
    REG_LCBQ_OFFSET2_CFG_3 GB_S6_7;     // 0x0474, LCBQ [1.8.0] Offset 6 and 7 parameter register.
    REG_LCBQ_OFFSET2_CFG_4 GB_S8_9;     // 0x0478, LCBQ [1.8.0] Offset 8 and 9 parameter register.
    REG_LCBQ_OFFSET2_CFG_5 GB_S10_11;   // 0x047C, LCBQ [1.8.0] Offset 10 and 11 parameter register.
    REG_LCBQ_OFFSET2_CFG_6 GB_S12_13;   // 0x0480, LCBQ [1.8.0] Offset 12 and 13 parameter register.
    REG_LCBQ_OFFSET2_CFG_7 GB_S14_15;   // 0x0484, LCBQ [1.8.0] Offset 14 and 15 parameter register.
    REG_LCBQ_OFFSET2_CFG_8 GB_S16_17;   // 0x0488, LCBQ [1.8.0] Offset 16 and 17 parameter register.
    REG_LCBQ_OFFSET2_CFG_9 GB_S18_19;   // 0x048C, LCBQ [1.8.0] Offset 18 and 19 parameter register.
    REG_LCBQ_OFFSET2_CFG_10 GB_S20_21;  // 0x0490, LCBQ [1.8.0] Offset 20 and 21 parameter register.
    REG_LCBQ_OFFSET2_CFG_11 GB_S22_23;  // 0x0494, LCBQ [1.8.0] Offset 22 and 23 parameter register.
    REG_LCBQ_OFFSET2_CFG_12 GB_S24;     // 0x0498, LCBQ [1.8.0] Offset 24 parameter register.

    REG_LCBQ_OFFSET3_CFG_0 B_S0_1;      // 0x049C, LCBQ [1.8.0] Offset 0 and 1 parameter register.
    REG_LCBQ_OFFSET3_CFG_1 B_S2_3;      // 0x04A0, LCBQ [1.8.0] Offset 2 and 3 parameter register.
    REG_LCBQ_OFFSET3_CFG_2 B_S4_5;      // 0x04A4, LCBQ [1.8.0] Offset 4 and 5 parameter register.
    REG_LCBQ_OFFSET3_CFG_3 B_S6_7;      // 0x04A8, LCBQ [1.8.0] Offset 6 and 7 parameter register.
    REG_LCBQ_OFFSET3_CFG_4 B_S8_9;      // 0x04AC, LCBQ [1.8.0] Offset 8 and 9 parameter register.
    REG_LCBQ_OFFSET3_CFG_5 B_S10_11;    // 0x04B0, LCBQ [1.8.0] Offset 10 and 11 parameter register.
    REG_LCBQ_OFFSET3_CFG_6 B_S12_13;    // 0x04B4, LCBQ [1.8.0] Offset 12 and 13 parameter register.
    REG_LCBQ_OFFSET3_CFG_7 B_S14_15;    // 0x04B8, LCBQ [1.8.0] Offset 14 and 15 parameter register.
    REG_LCBQ_OFFSET3_CFG_8 B_S16_17;    // 0x04BC, LCBQ [1.8.0] Offset 16 and 17 parameter register.
    REG_LCBQ_OFFSET3_CFG_9 B_S18_19;    // 0x04C0, LCBQ [1.8.0] Offset 18 and 19 parameter register.
    REG_LCBQ_OFFSET3_CFG_10 B_S20_21;   // 0x04C4, LCBQ [1.8.0] Offset 20 and 21 parameter register.
    REG_LCBQ_OFFSET3_CFG_11 B_S22_23;   // 0x04C8, LCBQ [1.8.0] Offset 22 and 23 parameter register.
    REG_LCBQ_OFFSET3_CFG_12 B_S24;      // 0x04CC, LCBQ [1.8.0] Offset 24 parameter register.
#else
    ST_LCBQ_OFST_REG stROffset;         // 0x0400 - 0x0433, LCBQ [1.8.0] Offset parameter register for symbol 0.
    ST_LCBQ_OFST_REG stGrOffset;        // 0x0434 - 0x0467, LCBQ [1.8.0] Offset parameter register for symbol 1.
    ST_LCBQ_OFST_REG stGbOffset;        // 0x0468 - 0x049B, LCBQ [1.8.0] Offset parameter register for symbol 2.
    ST_LCBQ_OFST_REG stBOffset;         // 0x049C - 0x04CF, LCBQ [1.8.0] Offset parameter register for symbol 3.
#endif //#if 0
} ST_LCBQ_OFST_ALL_REG, *PST_LCBQ_OFST_ALL_REG;

#define ST_LCBQ_OFST_ALL_REG_SIZE   (sizeof(ST_LCBQ_OFST_ALL_REG))
#define ST_LCBQ_OFST_ALL_REG_LEN    (ST_LCBQ_OFST_ALL_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCBQ_REG {
    REG_LCBQ_CFG_0 SubWindowSize;   // 0x0074, LCBQ pseudo sub-window register.
    //STF_U32 u32Reserved_0078[1];    // Added reserved variables to let ST_LCBQ_REG address is contiguous.
    REG_LCBQ_CFG_1 Coordinate;      // 0x007C, LCBQ X/Y coordinate register.
    //STF_U32 u32Reserved[?];         // Added reserved variables to let ST_LCBQ_REG address is contiguous.
    //-------------------------------------------------------------------------
    ST_LCBQ_GAIN_REG stRGain;       // 0x0300 - 0x0333, LCBQ [0.1.8] Gain parameter register for symbol 0.
    ST_LCBQ_GAIN_REG stGrGain;      // 0x0334 - 0x0367, LCBQ [0.1.8] Gain parameter register for symbol 1.
    ST_LCBQ_GAIN_REG stGbGain;      // 0x0368 - 0x039B, LCBQ [0.1.8] Gain parameter register for symbol 2.
    ST_LCBQ_GAIN_REG stBGain;       // 0x039C - 0x03CF, LCBQ [0.1.8] Gain parameter register for symbol 3.
    //STF_U32 u32Reserved_03D0[12];   // Added reserved variables to let ST_LCBQ_REG address is contiguous.
    //-------------------------------------------------------------------------
    ST_LCBQ_OFST_REG stROffset;     // 0x0400 - 0x0433, LCBQ [1.8.0] Offset parameter register for symbol 0.
    ST_LCBQ_OFST_REG stGrOffset;    // 0x0434 - 0x0467, LCBQ [1.8.0] Offset parameter register for symbol 1.
    ST_LCBQ_OFST_REG stGbOffset;    // 0x0468 - 0x049B, LCBQ [1.8.0] Offset parameter register for symbol 2.
    ST_LCBQ_OFST_REG stBOffset;     // 0x049C - 0x04CF, LCBQ [1.8.0] Offset parameter register for symbol 3.
} ST_LCBQ_REG, *PST_LCBQ_REG;

#define ST_LCBQ_REG_SIZE        (sizeof(ST_LCBQ_REG))
#define ST_LCBQ_REG_LEN         (ST_LCBQ_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_LCBQ_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_LCBQ_WIN_REG stLcbqWinReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaCoorCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_LCBQ_COOR_REG stLcbqCoorReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaGainCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_LCBQ_GAIN_ALL_REG stLcbqGainReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaOfstCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_LCBQ_OFST_ALL_REG stLcbqOfstReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_LCBQ_RDMA, *PST_LCBQ_RDMA;

#define ST_LCBQ_RDMA_SIZE       (sizeof(ST_LCBQ_RDMA))
#define ST_LCBQ_RDMA_LEN        (ST_LCBQ_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCBQ_WIN_RDMA_SIZE   (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCBQ_WIN_REG) + 7) / 8) * 8))
#define ST_LCBQ_WIN_RDMA_LEN    (ST_LCBQ_WIN_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCBQ_COOR_RDMA_SIZE  (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCBQ_COOR_REG) + 7) / 8) * 8))
#define ST_LCBQ_COOR_RDMA_LEN   (ST_LCBQ_COOR_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCBQ_CFG_RDMA_SIZE   (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCBQ_CFG_REG) + 7) / 8) * 8))
#define ST_LCBQ_CFG_RDMA_LEN    (ST_LCBQ_CFG_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCBQ_GAIN_RDMA_SIZE  (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCBQ_GAIN_ALL_REG) + 7) / 8) * 8))
#define ST_LCBQ_GAIN_RDMA_LEN   (ST_LCBQ_GAIN_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCBQ_OFST_RDMA_SIZE  (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCBQ_OFST_ALL_REG) + 7) / 8) * 8))
#define ST_LCBQ_OFST_RDMA_LEN   (ST_LCBQ_OFST_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_LCBQ_RDMA {
    STF_U8 u8IspIdx;
    ST_LCBQ_RDMA stLcbqRdma __attribute__ ((aligned (8)));
} ST_ISP_LCBQ_RDMA, *PST_ISP_LCBQ_RDMA;
#pragma pack(pop)


///* lens correction bi-quadratic registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_LCBQ_SetReg(
//    STF_U8 u8IspIdx,
//    ST_LCBQ_RDMA *pstLcbqRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_LCBQ_SetSubWindowSize(
//    STF_U8 u8IspIdx,
//    ST_LCBQ_RDMA *pstLcbqRdma
//    );
//extern
//STF_VOID STFHAL_ISP_LCBQ_SetCoordinate(
//    STF_U8 u8IspIdx,
//    ST_LCBQ_RDMA *pstLcbqRdma
//    );
//extern
//STF_VOID STFHAL_ISP_LCBQ_SetGain(
//    STF_U8 u8IspIdx,
//    ST_LCBQ_RDMA *pstLcbqRdma
//    );
//extern
//STF_VOID STFHAL_ISP_LCBQ_SetOffset(
//    STF_U8 u8IspIdx,
//    ST_LCBQ_RDMA *pstLcbqRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_LCBQ_RDMA_H__
