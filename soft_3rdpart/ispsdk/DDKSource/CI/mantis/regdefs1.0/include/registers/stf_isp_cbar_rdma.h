/**
  ******************************************************************************
  * @file  stf_isp_cbar_rdma.h
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
#ifndef __STF_ISP_CBAR_RDMA_H__
#define __STF_ISP_CBAR_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add color bar registers structure */
// 0x0014 - REG_SENSOR: [8]
typedef enum _EN_H_POLARITY {
    EN_H_POLARITY_NEG = 0,
    EN_H_POLARITY_POS,
    //EN_H_POLARITY_MAX
} EN_H_POLARITY, *PEN_H_POLARITY;

// 0x0014 - REG_SENSOR: [9]
typedef enum _EN_V_POLARITY {
    EN_V_POLARITY_NEG = 0,
    EN_V_POLARITY_POS,
    //EN_V_POLARITY_MAX
} EN_V_POLARITY, *PEN_V_POLARITY;

// 0x060C - REG_CSICBMD0: [15:12]
typedef enum _EN_PATTERN_TYPE {
    EN_PATTERN_TYPE_V_BAR = 0,
    EN_PATTERN_TYPE_H_BAR,
    EN_PATTERN_TYPE_V_GRADIENT,
    EN_PATTERN_TYPE_H_GRADIENT,
    EN_PATTERN_TYPE_MAX
} EN_PATTERN_TYPE, *PEN_PATTERN_TYPE;


typedef struct _ST_CBAR_REG {
    REG_CSICBFS FrameSize;          // 0x0600, Color Bar Frame Size.

    REG_CSICBS FrameStart;          // 0x0604, Color Bar Frame Start.
    REG_CSICBE FrameEnd;            // 0x0608, Color Bar Frame End.

    REG_CSICBMD0 Mode0;             // 0x060C, Color Bar Mode 0.
    REG_CSICBMD1 Mode1;             // 0x0610, Color Bar Mode 1.

    REG_CSICBP01 Pixel0_1;          // 0x0614, Color Bar Pixel 01.
    REG_CSICBP23 Pixel2_3;          // 0x0618, Color Bar Pixel 23.
    REG_CSICBP45 Pixel4_5;          // 0x061C, Color Bar Pixel 45.
    REG_CSICBP67 Pixel6_7;          // 0x0620, Color Bar Pixel 67.
    REG_CSICBP89 Pixel8_9;          // 0x0624, Color Bar Pixel 89.
    REG_CSICBPAB PixelA_B;          // 0x0628, Color Bar Pixel AB.
    REG_CSICBPCD PixelC_D;          // 0x062C, Color Bar Pixel CD.
    REG_CSICBPEF PixelE_F;          // 0x0630, Color Bar Pixel EF.

    REG_CSICBL1P01 Line1_Pixel0_1;  // 0x0634, Color Bar Line 1 Pixel 01.
    REG_CSICBL1P23 Line1_Pixel2_3;  // 0x0638, Color Bar Line 1 Pixel 23.
    REG_CSICBL1P45 Line1_Pixel4_5;  // 0x063C, Color Bar Line 1 Pixel 45.
    REG_CSICBL1P67 Line1_Pixel6_7;  // 0x0640, Color Bar Line 1 Pixel 67.
    REG_CSICBL1P89 Line1_Pixel8_9;  // 0x0644, Color Bar Line 1 Pixel 89.
    REG_CSICBL1PAB Line1_PixelA_B;  // 0x0648, Color Bar Line 1 Pixel AB.
    REG_CSICBL1PCD Line1_PixelC_D;  // 0x064C, Color Bar Line 1 Pixel CD.
    REG_CSICBL1PEF Line1_PixelE_F;  // 0x0650, Color Bar Line 1 Pixel EF.
} ST_CBAR_REG,*PST_CBAR_REG;

#define ST_CBAR_REG_SIZE        (sizeof(ST_CBAR_REG))
#define ST_CBAR_REG_LEN         (ST_CBAR_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_CBAR_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_CBAR_REG stCBarReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_CBAR_RDMA, *PST_CBAR_RDMA;

#define ST_CBAR_RDMA_SIZE       (sizeof(ST_CBAR_RDMA))
#define ST_CBAR_RDMA_LEN        (ST_CBAR_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_CBAR_RDMA {
    STF_U8 u8IspIdx;
    ST_CBAR_RDMA stCBarRdma __attribute__ ((aligned (8)));
} ST_ISP_CBAR_RDMA, *PST_ISP_CBAR_RDMA;
#pragma pack(pop)


///* color bar registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CBAR_SetReg(
//    STF_U8 u8IspIdx,
//    ST_CBAR_RDMA *pstCBarRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_CBAR_RDMA_H__

