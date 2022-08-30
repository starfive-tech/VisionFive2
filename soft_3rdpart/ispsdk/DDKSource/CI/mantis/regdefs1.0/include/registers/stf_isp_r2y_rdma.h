/**
  ******************************************************************************
  * @file  stf_isp_r2y_rdma.h
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
#ifndef __STF_ISP_R2Y_RDMA_H__
#define __STF_ISP_R2Y_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add RGB to YUV registers structure */
typedef struct _ST_R2Y_MATRIX {
    STF_U16 au16Matrix[3][3];
} ST_R2Y_MATRIX, *PST_R2Y_MATRIX;

typedef struct _ST_R2Y_MATRIX_PARAM {
    STF_U8 u8IspIdx;
    ST_R2Y_MATRIX stR2yMatrix;
} ST_R2Y_MATRIX_PARAM, *PST_R2Y_MATRIX_PARAM;

typedef struct _ST_R2Y_REG {
	REG_ICCONV_0 Matrix[3][3];      // 0x0E40 - 0x0E63, RGB to YUV color space conversion Register.
} ST_R2Y_REG, *PST_R2Y_REG;

#define ST_R2Y_REG_SIZE         (sizeof(ST_R2Y_REG))
#define ST_R2Y_REG_LEN          (ST_R2Y_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_R2Y_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_R2Y_REG stR2yReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_R2Y_RDMA, *PST_R2Y_RDMA;

#define ST_R2Y_RDMA_SIZE        (sizeof(ST_R2Y_RDMA))
#define ST_R2Y_RDMA_LEN         (ST_R2Y_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_R2Y_RDMA {
    STF_U8 u8IspIdx;
    ST_R2Y_RDMA stR2yRdma __attribute__ ((aligned (8)));
} ST_ISP_R2Y_RDMA, *PST_ISP_R2Y_RDMA;
#pragma pack(pop)


///* RGB to YUV registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_R2Y_SetReg(
//    STF_U8 u8IspIdx,
//    ST_R2Y_RDMA *pstR2yRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_R2Y_SetColorMatrix(
//    STF_U8 u8IspIdx,
//    STF_U16 au16Matrix[][3]
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_R2Y_RDMA_H__
