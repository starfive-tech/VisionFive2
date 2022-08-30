/**
  ******************************************************************************
  * @file  stf_isp_sud_isp_rdma.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  01/05/2021
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
#ifndef __STF_ISP_SUD_ISP_RDMA_H__
#define __STF_ISP_SUD_ISP_RDMA_H__


#include "registers/stf_isp_rdma.h"
#include "registers/stf_isp_sud_rdma.h"


/* add ISP shadow update registers structure */
typedef struct _ST_SUD_ISP_REG {
    REG_IESHD IspShadowCtrl;        // 0x0A50, Image engine shadow control Register.
} ST_SUD_ISP_REG, *PST_SUD_ISP_REG;

#define ST_SUD_ISP_REG_SIZE     (sizeof(ST_SUD_ISP_REG))
#define ST_SUD_ISP_REG_LEN      (ST_SUD_ISP_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_SUD_ISP_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SUD_ISP_REG stSudIspReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_SUD_ISP_RDMA, *PST_SUD_ISP_RDMA;

#define ST_SUD_ISP_RDMA_SIZE    (sizeof(ST_SUD_ISP_RDMA))
#define ST_SUD_ISP_RDMA_LEN     (ST_SUD_ISP_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_SUD_ISP_RDMA {
    STF_U8 u8IspIdx;
    ST_SUD_ISP_RDMA stSudIspRdma __attribute__ ((aligned (8)));
} ST_ISP_SUD_ISP_RDMA, *PST_ISP_SUD_ISP_RDMA;
#pragma pack(pop)


///* ISP shadow update registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_SetReg(
//    STF_U8 u8IspIdx,
//    ST_SUD_ISP_RDMA *pstSudIspRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_SetShadowEnable(
//    STF_U8 u8IspIdx,
//    ST_SUD_ISP_RDMA *pstSudIspRdma
//    );
//// Since the shadow update enable bit cannot be read back,
//// therefore disable below feature support.
//#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_SetShadowMode(
//    STF_U8 u8IspIdx,
//    ST_SUD_ISP_RDMA *pstSudIspRdma
//    );
//#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_SetShadowImmediatelyAndEnable(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_SetShadowNextVSyncAndEnable(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_SetShadowImmediatelyAndTrigger(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_SetShadowNextVSyncAndTrigger(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_SUD_ISP_TriggerShadow(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_SUD_ISP_RDMA_H__
