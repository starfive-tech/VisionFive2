/**
  ******************************************************************************
  * @file  stf_isp_dnyuv_rdma.h
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
#ifndef __STF_ISP_DNYUV_RDMA_H__
#define __STF_ISP_DNYUV_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add 2D denoise DNYUV registers structure */
typedef struct _ST_DNYUV_REG {
    REG_DNYUV_YSWR0 YSWeight0;      // 0x0C00, DNYUV Y channel S weight table register 0.
    REG_DNYUV_YSWR1 YSWeight1;      // 0x0C04, DNYUV Y channel S weight table register 1.
    REG_DNYUV_CSWR0 UvSWeight0;     // 0x0C08, DNYUV UV channel S weight table register 0.
    REG_DNYUV_CSWR1 UvSWeight1;     // 0x0C0C, DNYUV UV channel S weight table register 1.

#if 1
    REG_DNYUV_YDR0 YILevel[3];      // 0x0C10 - 0x0C1B, DNYUV Y Curve D component registers.
    REG_DNYUV_CDR0 UvILevel[3];     // 0x0C1C - 0x0C27, DNYUV UV Curve D component registers.
#else
    REG_DNYUV_YDR0 YILevel0;        // 0x0C10, DNYUV Y Curve D component register 0.
    REG_DNYUV_YDR1 YILevel1;        // 0x0C14, DNYUV Y Curve D component register 1.
    REG_DNYUV_YDR2 YILevel2;        // 0x0C18, DNYUV Y Curve D component register 2.
    REG_DNYUV_CDR0 UvILevel0;       // 0x0C1C, DNYUV UV Curve D component register 0.
    REG_DNYUV_CDR1 UvILevel1;       // 0x0C20, DNYUV UV Curve D component register 1.
    REG_DNYUV_CDR2 UvILevel2;       // 0x0C24, DNYUV UV Curve D component register 2.
#endif
} ST_DNYUV_REG, *PST_DNYUV_REG;

#define ST_DNYUV_REG_SIZE       (sizeof(ST_DNYUV_REG))
#define ST_DNYUV_REG_LEN        (ST_DNYUV_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_DNYUV_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_DNYUV_REG stDnYuvReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_DNYUV_RDMA, *PST_DNYUV_RDMA;

#define ST_DNYUV_RDMA_SIZE      (sizeof(ST_DNYUV_RDMA))
#define ST_DNYUV_RDMA_LEN       (ST_DNYUV_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_DNYUV_RDMA {
    STF_U8 u8IspIdx;
    ST_DNYUV_RDMA stDnYuvRdma __attribute__ ((aligned (8)));
} ST_ISP_DNYUV_RDMA, *PST_ISP_DNYUV_RDMA;
#pragma pack(pop)


///* 2D denoise DNYUV registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DNYUV_SetReg(
//    STF_U8 u8IspIdx,
//    ST_DNYUV_RDMA *pstDnYuvRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_DNYUV_RDMA_H__
