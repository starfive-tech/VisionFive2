/**
  ******************************************************************************
  * @file  stf_isp_dump_rdma.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  12/14/2019
  * @brief
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * TIME. AS A RESULT, STARFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai StarFive</center></h2>
  */
#ifndef __STF_ISP_DUMP_RDMA_H__
#define __STF_ISP_DUMP_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add CSI raw data dump interface registers structure */
//typedef enum _BURST_LEN {
//    BURST_LEN_0 = 0,
//    BURST_LEN_1,
//    BURST_LEN_2,
//    BURST_LEN_3,
//    BURST_LEN_MAX
//} BURST_LEN, *PBURST_LEN;
//
//
typedef struct _ST_DUMP_BUF_PARAM {
    STF_U8 u8IspIdx;
    STF_U32 u32BufAddr;
} ST_DUMP_BUF_PARAM, *PST_DUMP_BUF_PARAM;

typedef struct _ST_DUMP_STRIDE_PARAM {
    STF_U8 u8IspIdx;
    STF_U16 u16Stride;
} ST_DUMP_STRIDE_PARAM, *PST_DUMP_STRIDE_PARAM;

typedef struct _ST_DUMP_BURST_LEN_PARAM {
    STF_U8 u8IspIdx;
    STF_U8 u8BurstLen;
} ST_DUMP_BURST_LEN_PARAM, *PST_DUMP_BURST_LEN_PARAM;

typedef struct _ST_DUMP_PXL_R_SHIFT_PARAM {
    STF_U8 u8IspIdx;
    STF_U8 u8Shift;
} ST_DUMP_PXL_R_SHIFT_PARAM, *PST_DUMP_PXL_R_SHIFT_PARAM;

typedef struct _ST_DUMP_AXI_PARAM {
    STF_U8 u8IspIdx;
    STF_U8 u8AxiId;
} ST_DUMP_AXI_PARAM, *PST_DUMP_AXI_PARAM;

typedef struct _ST_DUMP_REG {
#if defined(V4L2_DRIVER)
#else
    REG_DUMP_CFG_0 DumpCfg0;        // 0x0024, base address for captured image, 128-byte alignment.
#endif //#if defined(V4L2_DRIVER)
    REG_DUMP_CFG_1 DumpCfg1;        // 0x0028, base address for captured image.
} ST_DUMP_REG, *PST_DUMP_REG;

#define ST_DUMP_REG_SIZE        (sizeof(ST_DUMP_REG))
#define ST_DUMP_REG_LEN         (ST_DUMP_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_DUMP_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_DUMP_REG stDumpReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_DUMP_RDMA, *PST_DUMP_RDMA;

#define ST_DUMP_RDMA_SIZE       (sizeof(ST_DUMP_RDMA))
#define ST_DUMP_RDMA_LEN        (ST_DUMP_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_DUMP_RDMA {
    STF_U8 u8IspIdx;
    ST_DUMP_RDMA stDumpRdma __attribute__ ((aligned (8)));
} ST_ISP_DUMP_RDMA, *PST_ISP_DUMP_RDMA;
#pragma pack(pop)


///* CSI raw data dump interface registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DUMP_SetReg(
//    STF_U8 u8IspIdx,
//    ST_DUMP_RDMA *pstDumpRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DUMP_SetBufAddr(
//    STF_U8 u8IspIdx,
//    STF_U32 u32BufAddr
//    );
//extern
//STF_VOID STFHAL_ISP_DUMP_SetStride(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Stride
//    );
//extern
//STF_VOID STFHAL_ISP_DUMP_SetBurstLen(
//    STF_U8 u8IspIdx,
//    STF_U8 u8BurstLen
//    );
//extern
//STF_VOID STFHAL_ISP_DUMP_SetPixelRightShift(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Shift
//    );
//extern
//STF_VOID STFHAL_ISP_DUMP_SetAxiId(
//    STF_U8 u8IspIdx,
//    STF_U8 u8AxiId
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_DUMP_RDMA_H__
