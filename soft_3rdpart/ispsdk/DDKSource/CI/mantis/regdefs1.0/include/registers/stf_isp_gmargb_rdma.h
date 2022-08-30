/**
  ******************************************************************************
  * @file  stf_isp_gmargb_rdma.h
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
#ifndef __STF_ISP_GMARGB_RDMA_H__
#define __STF_ISP_GMARGB_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add RGB gamma registers structure */
#define GMARGB_PNT              15


typedef struct _ST_GMARGB_REG {
    REG_IGRVAL_0 Gamma[GMARGB_PNT]; // 0x0E00 - 0x0E3B, RGB Gamma table Register 0 - 14.
} ST_GMARGB_REG, *PST_GMARGB_REG;

#define ST_GMARGB_REG_SIZE      (sizeof(ST_GMARGB_REG))
#define ST_GMARGB_REG_LEN       (ST_GMARGB_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_GMARGB_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_GMARGB_REG stGmaRgbReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_GMARGB_RDMA, *PST_GMARGB_RDMA;

#define ST_GMARGB_RDMA_SIZE     (sizeof(ST_GMARGB_RDMA))
#define ST_GMARGB_RDMA_LEN      (ST_GMARGB_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_GMARGB_RDMA {
    STF_U8 u8IspIdx;
    ST_GMARGB_RDMA stGmaRgbRdma __attribute__ ((aligned (8)));
} ST_ISP_GMARGB_RDMA, *PST_ISP_GMARGB_RDMA;
#pragma pack(pop)


///* RGB gamma registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_GMARGB_SetReg(
//    STF_U8 u8IspIdx,
//    ST_GMARGB_RDMA *pstGmaRgbRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_GMARGB_RDMA_H__
