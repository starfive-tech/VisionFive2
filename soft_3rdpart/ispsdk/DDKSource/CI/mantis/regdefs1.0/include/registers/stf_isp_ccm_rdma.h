/**
  ******************************************************************************
  * @file  stf_isp_ccm_rdma.h
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
#ifndef __STF_ISP_CCM_RDMA_H__
#define __STF_ISP_CCM_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add color correction matrix registers structure */
typedef struct _ST_CCM_REG {
    REG_ICAMD_0 MatrixSlope0_Dnrm;  // 0x0C40, Color correction matrix register 0.
    REG_ICAMD_1 MatrixSlope1;       // 0x0C44, Color correction matrix register 1.
    REG_ICAMD_2 MatrixSlope2;       // 0x0C48, Color correction matrix register 2.
    REG_ICAMD_3 MatrixSlope3;       // 0x0C4C, Color correction matrix register 3.
    REG_ICAMD_4 MatrixSlope4;       // 0x0C50, Color correction matrix register 4.
    REG_ICAMD_5 MatrixSlope5;       // 0x0C54, Color correction matrix register 5.
    REG_ICAMD_6 MatrixSlope6;       // 0x0C58, Color correction matrix register 6.
    REG_ICAMD_7 MatrixSlope7;       // 0x0C5C, Color correction matrix register 7.
    REG_ICAMD_8 MatrixSlope8;       // 0x0C60, Color correction matrix register 8.

    REG_ICAMD_9 OffsetSlope0;       // 0x0C64, Color correction matrix register 9.
    REG_ICAMD_10 OffsetSlope1;      // 0x0C68, Color correction matrix register 10.
    REG_ICAMD_11 OffsetSlope2;      // 0x0C6C, Color correction matrix register 11.

    REG_ICAMD_12 SmLowMatrix0;      // 0x0C70, Color correction matrix register 12.
    REG_ICAMD_13 SmLowMatrix1;      // 0x0C74, Color correction matrix register 13.
    REG_ICAMD_14 SmLowMatrix2;      // 0x0C78, Color correction matrix register 14.
    REG_ICAMD_15 SmLowMatrix3;      // 0x0C7C, Color correction matrix register 15.
    REG_ICAMD_16 SmLowMatrix4;      // 0x0C80, Color correction matrix register 16.
    REG_ICAMD_17 SmLowMatrix5;      // 0x0C84, Color correction matrix register 17.
    REG_ICAMD_18 SmLowMatrix6;      // 0x0C88, Color correction matrix register 18.
    REG_ICAMD_19 SmLowMatrix7;      // 0x0C8C, Color correction matrix register 19.
    REG_ICAMD_20 SmLowMatrix8;      // 0x0C90, Color correction matrix register 20.

    REG_ICAMD_21 SmLowOffset0;      // 0x0C94, Color correction matrix register 21.
    REG_ICAMD_22 SmLowOffset1;      // 0x0C98, Color correction matrix register 22.
    REG_ICAMD_23 SmLowOffset2;      // 0x0C9C, Color correction matrix register 23.

    REG_ICAMD_24 Distance;          // 0x0CA0, Color correction matrix register 24, dth.

    REG_ICAMD_25 SmLow;             // 0x0CA4, Color correction matrix register 25, th1.
} ST_CCM_REG,*PST_CCM_REG;

#define ST_CCM_REG_SIZE         (sizeof(ST_CCM_REG))
#define ST_CCM_REG_LEN          (ST_CCM_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_CCM_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_CCM_REG stCcmReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_CCM_RDMA, *PST_CCM_RDMA;

#define ST_CCM_RDMA_SIZE        (sizeof(ST_CCM_RDMA))
#define ST_CCM_RDMA_LEN         (ST_CCM_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_CCM_RDMA {
    STF_U8 u8IspIdx;
    ST_CCM_RDMA stCcmRdma __attribute__ ((aligned (8)));
} ST_ISP_CCM_RDMA, *PST_ISP_CCM_RDMA;
#pragma pack(pop)


///* color correction matrix registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CCM_SetReg(
//    STF_U8 u8IspIdx,
//    ST_CCM_RDMA *pstCcmRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_CCM_RDMA_H__

