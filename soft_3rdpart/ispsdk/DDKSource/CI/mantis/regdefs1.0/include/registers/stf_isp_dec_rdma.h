/**
  ******************************************************************************
  * @file  stf_isp_dec_rdma.h
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
#ifndef __STF_ISP_DEC_RDMA_H__
#define __STF_ISP_DEC_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add raw decimation registers structure */
typedef struct _ST_DEC_DECIMATION_PARAM {
    STF_U8 u8IspIdx;
    ST_DEC stDec;
} ST_DEC_DECIMATION_PARAM, *PST_DEC_DECIMATION_PARAM;

typedef struct _ST_DEC_REG {
    REG_DEC_CFG Cfg;                // 0x0030, csi pipeline dec cfg register, Horizontal/Vertical period/keep, must set to odd.
} ST_DEC_REG, *PST_DEC_REG;

#define ST_DEC_REG_SIZE         (sizeof(ST_DEC_REG))
#define ST_DEC_REG_LEN          (ST_DEC_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_DEC_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_DEC_REG stDecReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_DEC_RDMA, *PST_DEC_RDMA;

#define ST_DEC_RDMA_SIZE        (sizeof(ST_DEC_RDMA))
#define ST_DEC_RDMA_LEN         (ST_DEC_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_DEC_RDMA {
    STF_U8 u8IspIdx;
    ST_DEC_RDMA stDecRdma __attribute__ ((aligned (8)));
} ST_ISP_DEC_RDMA, *PST_ISP_DEC_RDMA;
#pragma pack(pop)


////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DEC_SetReg(
//    STF_U8 u8IspIdx,
//    ST_DEC_RDMA *pstDecRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DEC_SetDecimation(
//    STF_U8 u8IspIdx,
//    ST_DEC *pstDec
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_DEC_RDMA_H__

