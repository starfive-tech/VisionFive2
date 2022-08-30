/**
  ******************************************************************************
  * @file  stf_isp_oecfhm_rdma.h
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
#ifndef __STF_ISP_OECFHM_RDMA_H__
#define __STF_ISP_OECFHM_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add oecf hermite registers structure */
typedef struct _ST_OECFHM_INPUT_REG {
    REG_OECFHM_Y_CFG_0 YInput0_1;  // 0x03D0, OECF Hermite Y0/Y1 [0.12.0].
    REG_OECFHM_Y_CFG_1 YInput2_3;  // 0x03D4, OECF Hermite Y2/Y3 [0.12.0].
    REG_OECFHM_Y_CFG_2 YInput4;    // 0x03D8, OECF Hermite Y4 [0.12.0].
} ST_OECFHM_INPUT_REG, *PST_OECFHM_INPUT_REG;

#define ST_OECFHM_INPUT_REG_SIZE    (sizeof(ST_OECFHM_INPUT_REG))
#define ST_OECFHM_INPUT_REG_LEN     (ST_OECFHM_INPUT_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OECFHM_SLOPE_REG {
    REG_OECFHM_S_CFG_0 Slope0_1;   // 0x03DC, OECF Hermite S0/S1 [1.4.11].
    REG_OECFHM_S_CFG_1 Slope2_3;   // 0x03E0, OECF Hermite S2/S3 [1.4.11].
    REG_OECFHM_S_CFG_2 Slope4;     // 0x03E4, OECF Hermite S4 [1.4.11].
} ST_OECFHM_SLOPE_REG, *PST_OECFHM_SLOPE_REG;

#define ST_OECFHM_SLOPE_REG_SIZE    (sizeof(ST_OECFHM_SLOPE_REG))
#define ST_OECFHM_SLOPE_REG_LEN     (ST_OECFHM_SLOPE_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OECFHM_REG {
    REG_OECFHM_Y_CFG_0 YInput0_1;  // 0x03D0, OECF Hermite Y0/Y1 [0.12.0].
    REG_OECFHM_Y_CFG_1 YInput2_3;  // 0x03D4, OECF Hermite Y2/Y3 [0.12.0].
    REG_OECFHM_Y_CFG_2 YInput4;    // 0x03D8, OECF Hermite Y4 [0.12.0].

    REG_OECFHM_S_CFG_0 Slope0_1;   // 0x03DC, OECF Hermite S0/S1 [1.4.11].
    REG_OECFHM_S_CFG_1 Slope2_3;   // 0x03E0, OECF Hermite S2/S3 [1.4.11].
    REG_OECFHM_S_CFG_2 Slope4;     // 0x03E4, OECF Hermite S4 [1.4.11].
} ST_OECFHM_REG, *PST_OECFHM_REG;

#define ST_OECFHM_REG_SIZE      (sizeof(ST_OECFHM_REG))
#define ST_OECFHM_REG_LEN       (ST_OECFHM_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_OECFHM_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OECFHM_REG stOecfHmReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OECFHM_RDMA, *PST_OECFHM_RDMA;

#define ST_OECFHM_RDMA_SIZE     (sizeof(ST_OECFHM_RDMA))
#define ST_OECFHM_RDMA_LEN      (ST_OECFHM_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OECFHM_RDMA {
    STF_U8 u8IspIdx;
    ST_OECFHM_RDMA stOecfHmRdma __attribute__ ((aligned (8)));
} ST_ISP_OECFHM_RDMA, *PST_ISP_OECFHM_RDMA;
#pragma pack(pop)


///* oecf hermite registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OECFHM_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OECFHM_RDMA *pstOecfHmRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OECFHM_SetInput(
//    STF_U8 u8IspIdx,
//    ST_OECFHM_RDMA *pstOecfHmRdma
//    );
//extern
//STF_VOID STFHAL_ISP_OECFHM_SetSlope(
//    STF_U8 u8IspIdx,
//    ST_OECFHM_RDMA *pstOecfHmRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_OECFHM_RDMA_H__
