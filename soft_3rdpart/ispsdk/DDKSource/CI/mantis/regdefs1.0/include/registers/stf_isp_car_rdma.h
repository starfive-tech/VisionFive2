/**
  ******************************************************************************
  * @file  stf_isp_car_rdma.h
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
#ifndef __STF_ISP_CAR_RDMA_H__
#define __STF_ISP_CAR_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add color artifact removal registers structure */
typedef struct _ST_CAR_REG {
    //REG_ICFAM cfa_mode;
} ST_CAR_REG, *PST_CAR_REG;

#define ST_CAR_REG_SIZE         (sizeof(ST_CAR_REG))
#define ST_CAR_REG_LEN          (ST_CAR_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_CAR_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_CAR_REG stCarReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_CAR_RDMA, *PST_CAR_RDMA;

#define ST_CAR_RDMA_SIZE        (sizeof(ST_CAR_RDMA))
#define ST_CAR_RDMA_LEN         (ST_CAR_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_CAR_RDMA {
    STF_U8 u8IspIdx;
    ST_CAR_RDMA stCarRdma __attribute__ ((aligned (8)));
} ST_ISP_CAR_RDMA, *PST_ISP_CAR_RDMA;
#pragma pack(pop)


///* color artifact removal registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CAR_SetReg(
//    STF_U8 u8IspIdx,
//    ST_CAR_RDMA *pstCarRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_CAR_RDMA_H__

