/**
  ******************************************************************************
  * @file  stf_isp_csi_in_rdma.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/24/2021
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
#ifndef __STF_ISP_CSI_IN_RDMA_H__
#define __STF_ISP_CSI_IN_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add CSI input registers structure */
// 0x0014 - REG_SENSOR: [0]
typedef enum _EN_IMAGER_SEL {
    EN_IMAGER_SEL_DVP = 0,
    EN_IMAGER_SEL_MIPI
} EN_IMAGER_SEL, *PEN_IMAGER_SEL;

// 0x0014 - REG_SENSOR: [1]
typedef enum _EN_DVP_ITU656 {
    EN_DVP_ITU656_DISABLE = 0,
    EN_DVP_ITU656_ENABLE,
    //EN_DVP_ITU656_MAX
} EN_DVP_ITU656, *PEN_DVP_ITU656;

// 0x0014 - REG_SENSOR: [3:2]
typedef enum _EN_DVP_SYNC_POLARITY {
    EN_DVP_SYNC_POL_H_NEG_V_NEG = 0,
    EN_DVP_SYNC_POL_H_NEG_V_POS,
    EN_DVP_SYNC_POL_H_POS_V_NEG,
    EN_DVP_SYNC_POL_H_POS_V_POS,
    //EN_DVP_SYNC_POL_MAX
} EN_DVP_SYNC_POLARITY, *PEN_DVP_SYNC_POLARITY;

// 0x0018 - REG_CFA: [15:0]
typedef enum _EN_CFA_RAW_FMT {
    EN_CFA_RAW_FMT_RGGB = 0x0000EE44,
    EN_CFA_RAW_FMT_GRBG = 0x0000BB11,
    EN_CFA_RAW_FMT_GBRG = 0x000044EE,
    EN_CFA_RAW_FMT_BGGR = 0x000011BB,
} EN_CFA_RAW_FMT, *PEN_CFA_RAW_FMT;

typedef enum _EN_CSI_1ST_PXL {
    EN_CSI_1ST_PXL_R = 0,
    EN_CSI_1ST_PXL_GR = 1,
    EN_CSI_1ST_PXL_GB = 2,
    EN_CSI_1ST_PXL_B = 3,
    //EN_CSI_1ST_PXL_MAX
} EN_CSI_1ST_PXL, *PEN_CSI_1ST_PXL;


typedef struct _ST_ISP_CSI_LINE {
    STF_U8 u8IspIdx;
    STF_U16 u16Line;
} ST_ISP_CSI_LINE, *PST_ISP_CSI_LINE;

typedef struct _ST_CSI_IN_REG {
    REG_SENSOR  Sensor;                             // 0x0014, input dvp format & signal polarity cfg.
    REG_CFA FmtCfg;                                 // 0x0018, input RAW format cfg.
} ST_CSI_IN_REG, *PST_CSI_IN_REG;

#define ST_CSI_IN_REG_SIZE      (sizeof(ST_CSI_IN_REG))
#define ST_CSI_IN_REG_LEN       (ST_CSI_IN_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_CSI_IN_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_CSI_IN_REG stCsiInReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_CSI_IN_RDMA, *PST_CSI_IN_RDMA;

#define ST_CSI_IN_RDMA_SIZE     (sizeof(ST_CSI_IN_RDMA))
#define ST_CSI_IN_RDMA_LEN      (ST_CSI_IN_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_CSI_IN_RDMA {
    STF_U8 u8IspIdx;
    ST_CSI_IN_RDMA stCsiInRdma __attribute__ ((aligned (8)));
} ST_ISP_CSI_IN_RDMA, *PST_ISP_CSI_IN_RDMA;
#pragma pack(pop)


///* CSI input registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CSI_IN_SetReg(
//    STF_U8 u8IspIdx,
//    ST_CSI_IN_RDMA *pstCsiInRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CSI_IN_SetLine(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Line
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_CSI_IN_RDMA_H__

