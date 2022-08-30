/**
  ******************************************************************************
  * @file  stf_isp_dbc_rdma.h
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
#ifndef __STF_ISP_DBC_RDMA_H__
#define __STF_ISP_DBC_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add dynamic bad pixel correction filter registers structure */
typedef struct _ST_DBC_BAD_PXL_THRESHOLD_PARAM {
    STF_U16 u16Xt;
    STF_U16 u16Gt;
} ST_DBC_BAD_PXL_THRESHOLD_PARAM, *PST_DBC_BAD_PXL_THRESHOLD_PARAM;

typedef struct _ST_ISP_DBC_BAD_PXL_THRESHOLD_PARAM {
    STF_U8 u8IspIdx;
    ST_DBC_BAD_PXL_THRESHOLD_PARAM stDbcBadPixelThreshold;
} ST_ISP_DBC_BAD_PXL_THRESHOLD_PARAM, *PST_ISP_DBC_BAD_PXL_THRESHOLD_PARAM;

typedef struct _ST_DBC_REG {
    REG_IDBC Threshold;             // 0x0A14, GrGb filter threshold 2 Register.
} ST_DBC_REG, *PST_DBC_REG;

#define ST_DBC_REG_SIZE         (sizeof(ST_DBC_REG))
#define ST_DBC_REG_LEN          (ST_DBC_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_DBC_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_DBC_REG stDbcReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_DBC_RDMA, *PST_DBC_RDMA;

#define ST_DBC_RDMA_SIZE        (sizeof(ST_DBC_RDMA))
#define ST_DBC_RDMA_LEN         (ST_DBC_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_DBC_RDMA {
    STF_U8 u8IspIdx;
    ST_DBC_RDMA stDbcRdma __attribute__ ((aligned (8)));
} ST_ISP_DBC_RDMA, *PST_ISP_DBC_RDMA;
#pragma pack(pop)


///* dynamic bad pixel correction filter registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DBC_SetReg(
//    STF_U8 u8IspIdx,
//    ST_DBC_RDMA *pstDbcRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DBC_SetBadPixel(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Xt,
//    STF_U16 u16Gt
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_DBC_RDMA_H__

