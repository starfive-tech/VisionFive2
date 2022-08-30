/**
  ******************************************************************************
  * @file  stf_isp_ctc_rdma.h
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
#ifndef __STF_ISP_CTC_RDMA_H__
#define __STF_ISP_CTC_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add crosstalk compensation filter registers structure */
typedef enum _EN_CTC_MODE {
    EN_CTC_MODE_DETAIL_AVERAGE_N_SMOOTH_CENTER = 0,
    EN_CTC_MODE_SMOOTH_CENTER,
    EN_CTC_MODE_DETAIL_AVERAGE,
    EN_CTC_MODE_5X5_NEIGHBORHOOD,
} EN_CTC_MODE, *PEN_CTC_MODE;


typedef struct _ST_CTC_MODE_PARAM {
    STF_U8 u8IspIdx;
    EN_CTC_MODE enCtcMode;
} ST_CTC_MODE_PARAM, *PST_CTC_MODE_PARAM;

typedef struct _ST_CTC_THRESHOLD_PARAM {
    STF_U16 u16Min;
    STF_U16 u16Max;
} ST_CTC_THRESHOLD_PARAM, *PST_CTC_THRESHOLD_PARAM;

typedef struct _ST_ISP_CTC_THRESHOLD_PARAM {
    STF_U8 u8IspIdx;
    ST_CTC_THRESHOLD_PARAM stCtcThreshold;
} ST_ISP_CTC_THRESHOLD_PARAM, *PST_ISP_CTC_THRESHOLD_PARAM;

typedef struct _ST_CTC_REG {
    REG_ICTC Mode_Threshold;        // 0x0A10, GrGb Filter threshold 1 Register.
} ST_CTC_REG, *PST_CTC_REG;

#define ST_CTC_REG_SIZE         (sizeof(ST_CTC_REG))
#define ST_CTC_REG_LEN          (ST_CTC_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_CTC_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_CTC_REG stCtcReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_CTC_RDMA, *PST_CTC_RDMA;

#define ST_CTC_RDMA_SIZE        (sizeof(ST_CTC_RDMA))
#define ST_CTC_RDMA_LEN         (ST_CTC_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_CTC_RDMA {
    STF_U8 u8IspIdx;
    ST_CTC_RDMA stCtcRdma __attribute__ ((aligned (8)));
} ST_ISP_CTC_RDMA, *PST_ISP_CTC_RDMA;
#pragma pack(pop)


///* crosstalk compensation filter registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CTC_SetReg(
//    STF_U8 u8IspIdx,
//    ST_CTC_RDMA *pstCtcRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CTC_SetMode(
//    STF_U8 u8IspIdx,
//    EN_CTC_MODE enCtcMode
//    );
//extern
//STF_VOID STFHAL_ISP_CTC_SetThreshold(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Min,
//    STF_U16 u16Max
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_CTC_RDMA_H__

