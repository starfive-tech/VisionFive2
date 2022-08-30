/**
  ******************************************************************************
  * @file  stf_isp_oba_rdma.h
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
#ifndef __STF_ISP_OBA_RDMA_H__
#define __STF_ISP_OBA_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add optical black statistics registers structure */
typedef struct _ST_OBA_ACC {
    STF_U32 u32RAcc;
    STF_U32 u32GrAcc;
    STF_U32 u32GbAcc;
    STF_U32 u32BAcc;
} ST_OBA_ACC, *PST_OBA_ACC;

typedef struct _ST_ISP_OBA_ACC_PARAM {
    STF_U8 u8IspIdx;
    ST_OBA_ACC stObaAcc;
} ST_ISP_OBA_ACC_PARAM, *PST_ISP_OBA_ACC_PARAM;

typedef struct _ST_ISP_OBA_CROP_PARAM {
    STF_U8 u8IspIdx;
    ST_CROP stCrop;
} ST_ISP_OBA_CROP_PARAM, *PST_ISP_OBA_CROP_PARAM;

typedef struct _ST_ISP_OBA_ACCUMULATION_PARAM {
    STF_U8 u8IspIdx;
    STF_U32 au32Acc[4];
} ST_ISP_OBA_ACCUMULATION_PARAM, *PST_ISP_OBA_ACCUMULATION_PARAM;

typedef struct _ST_OBA_REG {
    REG_OBA_CFG_0 TopLeft;          // 0x0090, Vertical starting point/Horizontal ending point, each 16bits (black cropping).
    REG_OBA_CFG_1 BottomRight;      // 0x0094, Vertical ending point/Horizontal ending point, each 16bits.
} ST_OBA_REG, *PST_OBA_REG;

#define ST_OBA_REG_SIZE         (sizeof(ST_OBA_REG))
#define ST_OBA_REG_LEN          (ST_OBA_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OBA_ACC_REG {
    REG_OBA_ACC_0 R;                // 0x00A0, OBA value for symbol 0, Read only.
    REG_OBA_ACC_1 Gr;               // 0x00A4, OBA value for symbol 1, Read only.
    REG_OBA_ACC_2 Gb;               // 0x00A8, OBA value for symbol 2, Read only.
    REG_OBA_ACC_3 B;                // 0x00AC, OBA value for symbol 3, Read only.
} ST_OBA_ACC_REG, *PST_OBA_ACC_REG;

#define ST_OBA_ACC_REG_SIZE     (sizeof(ST_OBA_ACC_REG))
#define ST_OBA_ACC_REG_LEN      (ST_OBA_ACC_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_OBA_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OBA_REG stObaReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OBA_RDMA, *PST_OBA_RDMA;

#define ST_OBA_RDMA_SIZE        (sizeof(ST_OBA_RDMA))
#define ST_OBA_RDMA_LEN         (ST_OBA_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OBA_RDMA {
    STF_U8 u8IspIdx;
    ST_OBA_RDMA stObaRdma __attribute__ ((aligned (8)));
} ST_ISP_OBA_RDMA, *PST_ISP_OBA_RDMA;
#pragma pack(pop)


///* optical black statistics registers interface*/
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OBA_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OBA_RDMA *pstObaRdma
//    );
//extern
//ST_OBA_ACC STFHAL_ISP_OBA_GetAcc(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OBA_SetBlackCropping(
//    STF_U8 u8IspIdx,
//    ST_CROP *pstCrop
//    );
//extern
//STF_VOID STFHAL_ISP_OBA_GetAccumulation(
//    STF_U8 u8IspIdx,
//    STF_U32 au32Acc[]
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_OBA_RDMA_H__
