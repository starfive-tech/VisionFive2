/**
  ******************************************************************************
  * @file  stf_isp_cfa_ahd_rdma.h
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
#ifndef __STF_ISP_CFA_AHD_RDMA_H__
#define __STF_ISP_CFA_AHD_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add demosaic registers structure */
typedef struct _ST_CFA_MODE_PARAM {
    STF_U8 u8HvWidth;
    STF_U8 u8CrossCovariance;
} ST_CFA_MODE_PARAM, *PST_CFA_MODE_PARAM;

typedef struct _ST_ISP_CFA_MODE_PARAM {
    STF_U8 u8IspIdx;
    ST_CFA_MODE_PARAM stCfaMode;
} ST_ISP_CFA_MODE_PARAM, *PST_ISP_CFA_MODE_PARAM;

typedef struct _ST_CFA_REG {
    REG_ICFAM Mode;                 // 0x0A1C, CFA Mode Register.
} ST_CFA_REG, *PST_CFA_REG;

#define ST_CFA_REG_SIZE         (sizeof(ST_CFA_REG))
#define ST_CFA_REG_LEN          (ST_CFA_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_CFA_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_CFA_REG stCfaReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_CFA_RDMA, *PST_CFA_RDMA;

#define ST_CFA_RDMA_SIZE        (sizeof(ST_CFA_RDMA))
#define ST_CFA_RDMA_LEN         (ST_CFA_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_CFA_RDMA {
    STF_U8 u8IspIdx;
    ST_CFA_RDMA stCfaRdma __attribute__ ((aligned (8)));
} ST_ISP_CFA_RDMA, *PST_ISP_CFA_RDMA;
#pragma pack(pop)


///* demosaic registers interface */
////-----------------------------------------------------------------------------
//extern STF_VOID STFHAL_ISP_CFA_SetReg(ST_CFA_RDMA *pstCfaRdma);
////-----------------------------------------------------------------------------
//extern STF_VOID STFHAL_ISP_CFA_SetMode(STF_U8 u8HvWidth, STF_U8 u8CrossCovariance);
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_CFA_AHD_RDMA_H__

