/**
  ******************************************************************************
  * @file  stf_isp_oecf_rdma.h
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
#ifndef __STF_ISP_OECF_RDMA_H__
#define __STF_ISP_OECF_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add oecf registers structure */
typedef struct _ST_OECF_INPUT_REG {
    REG_OECF_X0_CFG_0 X0_1;         // 0x0100, OECF X 0 and 1 parameter[0.PW-2.0] register.
    REG_OECF_X0_CFG_1 X2_3;         // 0x0104, OECF X 2 and 3 parameter[0.PW-2.0] register.
    REG_OECF_X0_CFG_2 X4_5;         // 0x0108, OECF X 4 and 5 parameter[0.PW-2.0] register.
    REG_OECF_X0_CFG_3 X6_7;         // 0x010C, OECF X 6 and 7 parameter[0.PW-2.0] register.
    REG_OECF_X0_CFG_4 X8_9;         // 0x0110, OECF X 8 and 9 parameter[0.PW-2.0] register.
    REG_OECF_X0_CFG_5 X10_11;       // 0x0114, OECF X 10 and 11 parameter[0.PW-2.0] register.
    REG_OECF_X0_CFG_6 X12_13;       // 0x0118, OECF X 12 and 13 parameter[0.PW-2.0] register.
    REG_OECF_X0_CFG_7 X14_15;       // 0x011C, OECF X 14 and 15 parameter[0.PW-2.0] register.
} ST_OECF_INPUT_REG, *PST_OECF_INPUT_REG;

#define ST_OECF_INPUT_REG_SIZE  (sizeof(ST_OECF_INPUT_REG))
#define ST_OECF_INPUT_REG_LEN   (ST_OECF_INPUT_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OECF_OUTPUT_REG {
    REG_OECF_Y0_CFG_0 Y0_1;         // 0x0180, OECF Y 0 and 1 parameter[0.PW-2.0] register
    REG_OECF_Y0_CFG_1 Y2_3;         // 0x0184, OECF Y 2 and 3 parameter[0.PW-2.0] register
    REG_OECF_Y0_CFG_2 Y4_5;         // 0x0188, OECF Y 4 and 5 parameter[0.PW-2.0] register
    REG_OECF_Y0_CFG_3 Y6_7;         // 0x018C, OECF Y 6 and 7 parameter[0.PW-2.0] register
    REG_OECF_Y0_CFG_4 Y8_9;         // 0x0190, OECF Y 8 and 9 parameter[0.PW-2.0] register
    REG_OECF_Y0_CFG_5 Y10_11;       // 0x0194, OECF Y 10 and 11 parameter[0.PW-2.0] register
    REG_OECF_Y0_CFG_6 Y12_13;       // 0x0198, OECF Y 12 and 13 parameter[0.PW-2.0] register
    REG_OECF_Y0_CFG_7 Y14_15;       // 0x019C, OECF Y 14 and 15 parameter[0.PW-2.0] register
} ST_OECF_OUTPUT_REG, *PST_OECF_OUTPUT_REG;

#define ST_OECF_OUTPUT_REG_SIZE (sizeof(ST_OECF_OUTPUT_REG))
#define ST_OECF_OUTPUT_REG_LEN  (ST_OECF_OUTPUT_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OECF_SLOPE_REG {
    REG_OECF_S0_CFG_0 S0_1;         // 0x0200, OECF [0.3.7] S 0 and 1 parameter register
    REG_OECF_S0_CFG_1 S2_3;         // 0x0204, OECF [0.3.7] S 2 and 3 parameter register
    REG_OECF_S0_CFG_2 S4_5;         // 0x0208, OECF [0.3.7] S 4 and 5 parameter register
    REG_OECF_S0_CFG_3 S6_7;         // 0x020C, OECF [0.3.7] S 6 and 7 parameter register
    REG_OECF_S0_CFG_4 S8_9;         // 0x0210, OECF [0.3.7] S 8 and 9 parameter register
    REG_OECF_S0_CFG_5 S10_11;       // 0x0214, OECF [0.3.7] S 10 and 11 parameter register
    REG_OECF_S0_CFG_6 S12_13;       // 0x0218, OECF [0.3.7] S 12 and 13 parameter register
    REG_OECF_S0_CFG_7 S14_15;       // 0x021C, OECF [0.3.7] S 14 and 15 parameter register
} ST_OECF_SLOPE_REG, *PST_OECF_SLOPE_REG;

#define ST_OECF_SLOPE_REG_SIZE  (sizeof(ST_OECF_SLOPE_REG))
#define ST_OECF_SLOPE_REG_LEN   (ST_OECF_SLOPE_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OECF_REG {
    ST_OECF_INPUT_REG stRInput;     // 0x0100 - 0x011F, OECF X parameter[0.PW-2.0] register for symbol 0.
    ST_OECF_INPUT_REG stGrInput;    // 0x0120 - 0x013F, OECF X parameter[0.PW-2.0] register for symbol 1.
    ST_OECF_INPUT_REG stGbInput;    // 0x0140 - 0x015F, OECF X parameter[0.PW-2.0] register for symbol 2.
    ST_OECF_INPUT_REG stBInput;     // 0x0160 - 0x017F, OECF X parameter[0.PW-2.0] register for symbol 3.

    ST_OECF_OUTPUT_REG stROutput;   // 0x0180 - 0x019F, OECF Y parameter[0.PW-2.0] register for symbol 0.
    ST_OECF_OUTPUT_REG stGrOutput;  // 0x01A0 - 0x01BF, OECF Y parameter[0.PW-2.0] register for symbol 1.
    ST_OECF_OUTPUT_REG stGbOutput;  // 0x01C0 - 0x01DF, OECF Y parameter[0.PW-2.0] register for symbol 2.
    ST_OECF_OUTPUT_REG stBOutput;   // 0x01E0 - 0x01FF, OECF Y parameter[0.PW-2.0] register for symbol 3.

    ST_OECF_SLOPE_REG stRSlope;     // 0x0200 - 0x021F, OECF [0.3.7] S parameter register for symbol 0.
    ST_OECF_SLOPE_REG stGrSlope;    // 0x0220 - 0x023F, OECF [0.3.7] S parameter register for symbol 1.
    ST_OECF_SLOPE_REG stGbSlope;    // 0x0240 - 0x025F, OECF [0.3.7] S parameter register for symbol 2.
    ST_OECF_SLOPE_REG stBSlope;     // 0x0260 - 0x027F, OECF [0.3.7] S parameter register for symbol 3.
} ST_OECF_REG, *PST_OECF_REG;

#define ST_OECF_REG_SIZE        (sizeof(ST_OECF_REG))
#define ST_OECF_REG_LEN         (ST_OECF_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_OECF_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OECF_REG stOecfReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OECF_RDMA, *PST_OECF_RDMA;

#define ST_OECF_RDMA_SIZE       (sizeof(ST_OECF_RDMA))
#define ST_OECF_RDMA_LEN        (ST_OECF_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OECF_RDMA {
    STF_U8 u8IspIdx;
    ST_OECF_RDMA stOecfRdma __attribute__ ((aligned (8)));
} ST_ISP_OECF_RDMA, *PST_ISP_OECF_RDMA;
#pragma pack(pop)


///* oecf registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OECF_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OECF_RDMA *pstOecfRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OECF_SetInput(
//    STF_U8 u8IspIdx,
//    ST_OECF_RDMA *pstOecfRdma
//    );
//extern
//STF_VOID STFHAL_ISP_OECF_SetOutput(
//    STF_U8 u8IspIdx,
//    ST_OECF_RDMA *pstOecfRdma
//    );
//extern
//STF_VOID STFHAL_ISP_OECF_SetSlope(
//    STF_U8 u8IspIdx,
//    ST_OECF_RDMA *pstOecfRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_OECF_RDMA_H__
