/**
  ******************************************************************************
  * @file  stf_isp_dc_rdma.h
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
#ifndef __STF_ISP_DC_RDMA_H__
#define __STF_ISP_DC_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add defective pixel correction registers structure */
typedef union _ST_DC_DATA {
    STF_U32 value;
    struct
    {
        STF_U32 hpos         : 14 ; // [13:0] Horizontal position
        STF_U32 mode         : 2  ; // [15:14] Fixed mode (1: Left seek, 2: Right seek, 3: Average (Left+Right seek)
        STF_U32 vpos         : 14 ; // [:16] Vertical keep(zero base)
    } field;
} ST_DC_DATA, *PST_DC_DATA;

typedef struct _ST_DC_BUF_PARAM {
    STF_U8 u8IspIdx;
    STF_U32 u32DataBufferAddress;
} ST_DC_BUF_PARAM, *PST_DC_BUF_PARAM;

typedef struct _ST_DC_AXI_PARAM {
    STF_U8 u8IspIdx;
    STF_U8 u8AxiId;
} ST_DC_AXI_PARAM, *PST_DC_AXI_PARAM;

typedef struct _ST_DC_REG {
    REG_DC_CFG_0 BufAddr;           // 0x0040, DC axi base address register.
    REG_DC_CFG_1 AxiId;             // 0x0044, DC axi id.
    //REG_DC_RBADR ReturnBufAddr;     // Read only.
} ST_DC_REG, *PST_DC_REG;

#define ST_DC_REG_SIZE          (sizeof(ST_DC_REG))
#define ST_DC_REG_LEN           (ST_DC_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_DC_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_DC_REG stDcReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_DC_RDMA, *PST_DC_RDMA;

#define ST_DC_RDMA_SIZE         (sizeof(ST_DC_RDMA))
#define ST_DC_RDMA_LEN          (ST_DC_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_DC_RDMA {
    STF_U8 u8IspIdx;
    ST_DC_RDMA stDcRdma __attribute__ ((aligned (8)));
} ST_ISP_DC_RDMA, *PST_ISP_DC_RDMA;
#pragma pack(pop)


///* defective pixel correction registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_DC_SetReg(
//    STF_U8 u8IspIdx,
//    ST_DC_RDMA *pstDcRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_U32 STFHAL_ISP_DC_GetDataBufferAddress(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_DC_SetDataBufferAddress(
//    STF_U8 u8IspIdx,
//    STF_U32 u32DataBufferAddress
//    );
//extern
//STF_VOID STFHAL_ISP_DC_SetAxiId(
//    STF_U8 u8IspIdx,
//    STF_U8 u8AxiId
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_DC_RDMA_H__

