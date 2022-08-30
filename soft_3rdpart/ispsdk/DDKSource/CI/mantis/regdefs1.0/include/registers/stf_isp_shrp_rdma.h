/**
  ******************************************************************************
  * @file  stf_isp_shrp_rdma.h
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
#ifndef __STF_ISP_SHRP_RDMA_H__
#define __STF_ISP_SHRP_RDMA_H__


#include "registers/stf_isp_rdma.h"


#define SHRP_WT_Y_DIFF_LEN      ((ADDR_REG_ISHRP1_14 - ADDR_REG_ISHRP1_0) / sizeof(STF_U32) + 1)
#define SHRP_FACTOR_SLOPE_LEN   ((ADDR_REG_ISHRP2_5 - ADDR_REG_ISHRP2_0) / sizeof(STF_U32) + 1)
#define SHRP_WT_N11N_LEN        ((ADDR_REG_ISHRP3 - ADDR_REG_ISHRP3) / sizeof(STF_U32) + 1)
#define SHRP_UV_SMTH_CRV_LEN    ((ADDR_REG_IUVS2 - ADDR_REG_IUVS1) / sizeof(STF_U32) + 1)
#define SHRP_UV_COLR_KILL_LEN   ((ADDR_REG_IUVCKS2 - ADDR_REG_IUVCKS1) / sizeof(STF_U32) + 1)
#define SHRP_EDGE_SMTH_LEN      ((ADDR_REG_ISHRPET - ADDR_REG_ISHRPET) / sizeof(STF_U32) + 1)


/* add sharpening registers structure */
typedef struct _ST_SHRP_REG {
    REG_ISHRP1_0 W0_LD1;            // 0x0E80, Sharpen Edge Amplification Weight and Difference 0.
    REG_ISHRP1_1 W1_LD2;            // 0x0E84, Sharpen Edge Amplification Weight and Difference 1.
    REG_ISHRP1_2 W2_LD3;            // 0x0E88, Sharpen Edge Amplification Weight and Difference 2.
    REG_ISHRP1_3 W3_LD4;            // 0x0E8C, Sharpen Edge Amplification Weight and Difference 3.
    REG_ISHRP1_4 W4_MD1;            // 0x0E90, Sharpen Edge Amplification Weight and Difference 4.
    REG_ISHRP1_5 W5_MD2;            // 0x0E94, Sharpen Edge Amplification Weight and Difference 5.
    REG_ISHRP1_6 W6_MD3;            // 0x0E98, Sharpen Edge Amplification Weight and Difference 6.
    REG_ISHRP1_7 W7_MD4;            // 0x0E9C, Sharpen Edge Amplification Weight and Difference 7.
    REG_ISHRP1_8 W8;                // 0x0EA0, Sharpen Edge Amplification Weight 8.
    REG_ISHRP1_9 W9;                // 0x0EA4, Sharpen Edge Amplification Weight 9.
    REG_ISHRP1_10 W10;              // 0x0EA8, Sharpen Edge Amplification Weight 10.
    REG_ISHRP1_11 W11;              // 0x0EAC, Sharpen Edge Amplification Weight 11.
    REG_ISHRP1_12 W12;              // 0x0EB0, Sharpen Edge Amplification Weight 12.
    REG_ISHRP1_13 W13;              // 0x0EB4, Sharpen Edge Amplification Weight 13.
    REG_ISHRP1_14 W14;              // 0x0EB8, Sharpen Edge Amplification Weight 14.

    REG_ISHRP2_0 LF1_LS1;           // 0x0EBC, Sharpen Edge Amplification Factor and Slope 0.
    REG_ISHRP2_1 LF2_LS2;           // 0x0EC0, Sharpen Edge Amplification Factor and Slope 1.
    REG_ISHRP2_2 LF3_LS3;           // 0x0EC4, Sharpen Edge Amplification Factor and Slope 2.
    REG_ISHRP2_3 MF1_MS1;           // 0x0EC8, Sharpen Edge Amplification Factor and Slope 3.
    REG_ISHRP2_4 MF2_MS2;           // 0x0ECC, Sharpen Edge Amplification Factor and Slope 4.
    REG_ISHRP2_5 MF3_MS3;           // 0x0ED0, Sharpen Edge Amplification Factor and Slope 5.

    REG_ISHRP3 WSum_NDirF_PDirF;    // 0x0ED4, Sharpen Edge Amplification Weight Normalization.

    REG_IUVS1 UvSmoothDiff1_2;      // 0x0ED8, UV smoothing Register 1.
    REG_IUVS2 UvSmoothSlope_Factor; // 0x0EDC, UV smoothing Register 2.

    REG_IUVCKS1 UvCKDiff1_2;        // 0x0EE0, UV Color Kill Register 1.
    REG_IUVCKS2 UvCKSlope;          // 0x0EE4, UV Color Kill Register 2.

    REG_ISHRPET EdgeSmoothCfg;      // 0x0EE8, Sharpen Edge Threshold.
} ST_SHRP_REG, *PST_SHRP_REG;

#define ST_SHRP_REG_SIZE        (sizeof(ST_SHRP_REG))
#define ST_SHRP_REG_LEN         (ST_SHRP_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_SHRP_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SHRP_REG stShrpReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_SHRP_RDMA, *PST_SHRP_RDMA;

#define ST_SHRP_RDMA_SIZE       (sizeof(ST_SHRP_RDMA))
#define ST_SHRP_RDMA_LEN        (ST_SHRP_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_SHRP_RDMA {
    STF_U8 u8IspIdx;
    ST_SHRP_RDMA stShrpRdma __attribute__ ((aligned (8)));
} ST_ISP_SHRP_RDMA, *PST_ISP_SHRP_RDMA;
#pragma pack(pop)


///* sharpening registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SHRP_SetReg(
//    STF_U8 u8IspIdx,
//    ST_SHRP_RDMA *pstShrpRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SHRP_SetWeightAndYDifferent(
//    STF_U8 u8IspIdx,
//    ST_SHRP_RDMA *pstShrpRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SHRP_SetFactorAndSlope(
//    STF_U8 u8IspIdx,
//    ST_SHRP_RDMA *pstShrpRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SHRP_SetWeightNormalization(
//    STF_U8 u8IspIdx,
//    ST_SHRP_RDMA *pstShrpRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SHRP_SetUvSmoothCurve(
//    STF_U8 u8IspIdx,
//    ST_SHRP_RDMA *pstShrpRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SHRP_SetUvColorKillCurve(
//    STF_U8 u8IspIdx,
//    ST_SHRP_RDMA *pstShrpRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SHRP_SetEdgeParam(
//    STF_U8 u8IspIdx,
//    ST_SHRP_RDMA *pstShrpRdma
//    );
//
//
#endif //__STF_ISP_SHRP_RDMA_H__
