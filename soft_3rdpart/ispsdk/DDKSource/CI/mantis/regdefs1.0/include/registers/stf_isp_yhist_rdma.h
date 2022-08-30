/**
  ******************************************************************************
  * @file  stf_isp_yhist_rdma.h
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
#ifndef __STF_ISP_YHIST_RDMA_H__
#define __STF_ISP_YHIST_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add y histogram registers structure */
#define USE_YHIST_REG_READ_ACC
#define Y_HIST_ITEM_SIZE            (64)
#define Y_HIST_WAITING_COUNT        (10000)


typedef struct _ST_YHIST_DUMP_STATUS {
    STF_U8 u8IspIdx;
    STF_BOOL8 bDumpStatus;
} ST_YHIST_DUMP_STATUS, *PST_YHIST_DUMP_STATUS;

typedef struct _ST_YHIST_ACC_VALID_STATUS {
    STF_U8 u8IspIdx;
    STF_BOOL8 bAccValidStatus;
} ST_YHIST_ACC_VALID_STATUS, *PST_YHIST_ACC_VALID_STATUS;

typedef struct _ST_YHIST_RESULT {
    STF_U32 u32Accumulate[Y_HIST_ITEM_SIZE];
} ST_YHIST_RESULT, *PST_YHIST_RESULT;

typedef struct _ST_ISP_YHIST_RESULT_PARAM {
    STF_U8 u8IspIdx;
    ST_YHIST_RESULT stYHistResult;
} ST_ISP_YHIST_RESULT_PARAM, *PST_ISP_YHIST_RESULT_PARAM;

typedef struct _ST_YHIST_REG {
    REG_YHIST_CFG_0 YHistCfg0;      // 0x0CC8, Y Histogram Start Location Register.
    REG_YHIST_CFG_1 YHistCfg1;      // 0x0CCC, Y Histogram Size Register.
    REG_YHIST_CFG_2 YHistCfg2;      // 0x0CD0, Y Histogram Decimation Register.
    REG_YHIST_CFG_3 YHistCfg3;      // 0x0CD4, Y Histogram mux sel register, only one select 0: before y curve.
#if defined(V4L2_DRIVER)
#else
    REG_YHIST_CFG_4 YHistCfg4;      // 0x0CD8, Y Histogram dumping base address.
#endif //#if defined(V4L2_DRIVER)
} ST_YHIST_REG, *PST_YHIST_REG;

typedef struct _ST_YHIST_ACC_REG {
    REG_YHIST_ACC_0 YHistAcc[Y_HIST_ITEM_SIZE];   // 0x0D00- 0x0DFF, Y Histogram result register 0, Read only.
} ST_YHIST_ACC_REG, *PST_YHIST_ACC_REG;

#define ST_YHIST_REG_SIZE       (sizeof(ST_YHIST_REG))
#define ST_YHIST_REG_LEN        (ST_YHIST_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_YHIST_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_YHIST_REG stYHistReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_YHIST_RDMA, *PST_YHIST_RDMA;

#define ST_YHIST_RDMA_SIZE      (sizeof(ST_YHIST_RDMA))
#define ST_YHIST_RDMA_LEN       (ST_YHIST_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_YHIST_RDMA {
    STF_U8 u8IspIdx;
    ST_YHIST_RDMA stYHistRdma __attribute__ ((aligned (8)));
} ST_ISP_YHIST_RDMA, *PST_ISP_YHIST_RDMA;
#pragma pack(pop)


///* y histogram registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_YHIST_SetReg(
//    STF_U8 u8IspIdx,
//    ST_YHIST_RDMA *pstYHistRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_YHIST_SetStartLocation(
//    STF_U8 u8IspIdx,
//    ST_YHIST_RDMA *pstYHistRdma
//    );
//extern
//STF_VOID STFHAL_ISP_YHIST_SetWindowSize(
//    STF_U8 u8IspIdx,
//    ST_YHIST_RDMA *pstYHistRdma
//    );
//extern
//STF_VOID STFHAL_ISP_YHIST_SetDecimation(
//    STF_U8 u8IspIdx,
//    ST_YHIST_RDMA *pstYHistRdma
//    );
//extern
//STF_VOID STFHAL_ISP_YHIST_SetMuxSel(
//    STF_U8 u8IspIdx,
//    ST_YHIST_RDMA *pstYHistRdma
//    );
//extern
//STF_VOID STFHAL_ISP_YHIST_SetDumpBufAddr(
//    STF_U8 u8IspIdx,
//    ST_YHIST_RDMA *pstYHistRdma
//    );
//extern
//STF_BOOL8 STFHAL_ISP_YHIST_GetDumpStatus(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_YHIST_GetAccValidStatus(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_YHIST_GetAccResult(
//    STF_U8 u8IspIdx,
//    ST_YHIST_RESULT *pstYHistResult
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_YHIST_RDMA_H__
