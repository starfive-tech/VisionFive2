/**
  ******************************************************************************
  * @file  stf_isp_ycrv_rdma.h
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
#ifndef __STF_ISP_YCRV_RDMA_H__
#define __STF_ISP_YCRV_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add y curve registers structure */
#define YCRV_PNT                (64)
#define YCRV_MAX_VALUE          (1 << 10)


typedef struct _ST_YCRV_REG {
	REG_YCURVE_0 YCurve[YCRV_PNT];  // 0x0F00 - 0x0FFF, Y Curve PARAM table Register 0 - 63, Format: 0.10.0.
} ST_YCRV_REG, *PST_YCRV_REG;

#define ST_YCRV_REG_SIZE        (sizeof(ST_YCRV_REG))
#define ST_YCRV_REG_LEN         (ST_YCRV_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_YCRV_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_YCRV_REG stYCrvReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_YCRV_RDMA, *PST_YCRV_RDMA;

#define ST_YCRV_RDMA_SIZE       (sizeof(ST_YCRV_RDMA))
#define ST_YCRV_RDMA_LEN        (ST_YCRV_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_YCRV_RDMA {
    STF_U8 u8IspIdx;
    ST_YCRV_RDMA stYCrvRdma __attribute__ ((aligned (8)));
} ST_ISP_YCRV_RDMA, *PST_ISP_YCRV_RDMA;
#pragma pack(pop)


///* y curve registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_YCRV_SetReg(
//    STF_U8 u8IspIdx,
//    ST_YCRV_RDMA *pstYCrvRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_YCRV_RDMA_H__
