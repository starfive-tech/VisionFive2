/**
  ******************************************************************************
  * @file  stf_isp_awb_rdma.h
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
#ifndef __STF_ISP_AWB_RDMA_H__
#define __STF_ISP_AWB_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add auto white-balance registers structure */
typedef enum _EN_AWB_CHN {
    EN_AWB_CHN_R = 0,
    EN_AWB_CHN_GR,
    EN_AWB_CHN_GB,
    EN_AWB_CHN_B,
    EN_AWB_CHN_MAX
} EN_AWB_CHN, *PEN_AWB_CHN;


typedef struct _ST_AWB_CHN_CRV_PARAM {
    STF_U8 u8Chn;
    STF_U16 au16Pi[4];
    STF_U16 au16Po[4];
    STF_U16 au16Ps[4];
} ST_AWB_CHN_CRV_PARAM, *PST_AWB_CHN_CRV_PARAM;

typedef struct _ST_ISP_AWB_CHN_CRV_PARAM {
    STF_U8 u8IspIdx;
    ST_AWB_CHN_CRV_PARAM stAwbChnCrv;
} ST_ISP_AWB_CHN_CRV_PARAM, *PST_ISP_AWB_CHN_CRV_PARAM;

typedef struct _ST_AWB_INPUT_PARAM {
    STF_U16 au16RPi[4];
    STF_U16 au16GrPi[4];
    STF_U16 au16GbPi[4];
    STF_U16 au16BPi[4];
} ST_AWB_INPUT_PARAM, *PST_AWB_INPUT_PARAM;

typedef struct _ST_ISP_AWB_INPUT_PARAM {
    STF_U8 u8IspIdx;
    ST_AWB_INPUT_PARAM stAwbInput;
} ST_ISP_AWB_INPUT_PARAM, *PST_ISP_AWB_INPUT_PARAM;

typedef struct _ST_AWB_OUTPUT_PARAM {
    STF_U16 au16RPo[4];
    STF_U16 au16GrPo[4];
    STF_U16 au16GbPo[4];
    STF_U16 au16BPo[4];
} ST_AWB_OUTPUT_PARAM, *PST_AWB_OUTPUT_PARAM;

typedef struct _ST_ISP_AWB_OUTPUT_PARAM {
    STF_U8 u8IspIdx;
    ST_AWB_OUTPUT_PARAM stAwbOutput;
} ST_ISP_AWB_OUTPUT_PARAM, *PST_ISP_AWB_OUTPUT_PARAM;

typedef struct _ST_AWB_SLOPE_PARAM {
    STF_U16 au16RPs[4];
    STF_U16 au16GrPs[4];
    STF_U16 au16GbPs[4];
    STF_U16 au16BPs[4];
} ST_AWB_SLOPE_PARAM, *PST_AWB_SLOPE_PARAM;

typedef struct _ST_ISP_AWB_SLOPE_PARAM {
    STF_U8 u8IspIdx;
    ST_AWB_SLOPE_PARAM stAwbSlope;
} ST_ISP_AWB_SLOPE_PARAM, *PST_ISP_AWB_SLOPE_PARAM;

typedef struct _ST_AWB_REG {
    REG_AWB_X0_CFG_0 RIn0_1;        // 0x0280, X0/X1 for symbol 0, each 10bits.
    REG_AWB_X0_CFG_1 RIn2_3;        // 0x0284, X2/X3 for symbol 0, each 10bits.
    REG_AWB_X1_CFG_0 GrIn0_1;       // 0x0288, X0/X1 for symbol 1, each 10bits.
    REG_AWB_X1_CFG_1 GrIn2_3;       // 0x028C, X2/X3 for symbol 1, each 10bits.
    REG_AWB_X2_CFG_0 GbIn0_1;       // 0x0290, X0/X1 for symbol 2, each 10bits.
    REG_AWB_X2_CFG_1 GbIn2_3;       // 0x0294, X2/X3 for symbol 2, each 10bits.
    REG_AWB_X3_CFG_0 BIn0_1;        // 0x0298, X0/X1 for symbol 3, each 10bits.
    REG_AWB_X3_CFG_1 BIn2_3;        // 0x029C, X2/X3 for symbol 3, each 10bits.

    REG_AWB_Y0_CFG_0 ROut0_1;       // 0x02A0, Y0/Y1 for symbol 0, each 10bits.
    REG_AWB_Y0_CFG_1 ROut2_3;       // 0x02A4, Y2/Y3 for symbol 0, each 10bits.
    REG_AWB_Y1_CFG_0 GrOut0_1;      // 0x02A8, Y0/Y1 for symbol 1, each 10bits.
    REG_AWB_Y1_CFG_1 GrOut2_3;      // 0x02AC, Y2/Y3 for symbol 1, each 10bits.
    REG_AWB_Y2_CFG_0 GbOut0_1;      // 0x02B0, Y0/Y1 for symbol 2, each 10bits.
    REG_AWB_Y2_CFG_1 GbOut2_3;      // 0x02B4, Y2/Y3 for symbol 2, each 10bits.
    REG_AWB_Y3_CFG_0 BOut0_1;       // 0x02B8, Y0/Y1 for symbol 3, each 10bits.
    REG_AWB_Y3_CFG_1 BOut2_3;       // 0x02BC, Y2/Y3 for symbol 3, each 10bits.

    REG_AWB_S0_CFG_0 RSlope0_1;     // 0x02C0, [0.3.7] S0/S1 for symbol 0, each 10bits.
    REG_AWB_S0_CFG_1 RSlope2_3;     // 0x02C4, [0.3.7] S2/S3 for symbol 0, each 10bits.
    REG_AWB_S1_CFG_0 GrSlope0_1;    // 0x02C8, [0.3.7] S0/S1 for symbol 1, each 10bits.
    REG_AWB_S1_CFG_1 GrSlope2_3;    // 0x02CC, [0.3.7] S2/S3 for symbol 1, each 10bits.
    REG_AWB_S2_CFG_0 GbSlope0_1;    // 0x02D0, [0.3.7] S0/S1 for symbol 2, each 10bits.
    REG_AWB_S2_CFG_1 GbSlope2_3;    // 0x02D4, [0.3.7] S2/S3 for symbol 2, each 10bits.
    REG_AWB_S3_CFG_0 BSlope0_1;     // 0x02D8, [0.3.7] S0/S1 for symbol 3, each 10bits.
    REG_AWB_S3_CFG_1 BSlope2_3;     // 0x02DC, [0.3.7] S2/S3 for symbol 3, each 10bits.
} ST_AWB_REG, *PST_AWB_REG;

#define ST_AWB_REG_SIZE         (sizeof(ST_AWB_REG))
#define ST_AWB_REG_LEN          (ST_AWB_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_AWB_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_AWB_REG stAwbReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_AWB_RDMA, *PST_AWB_RDMA;

#define ST_AWB_RDMA_SIZE        (sizeof(ST_AWB_RDMA))
#define ST_AWB_RDMA_LEN         (ST_AWB_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_AWB_RDMA {
    STF_U8 u8IspIdx;
    ST_AWB_RDMA stAwbRdma __attribute__ ((aligned (8)));
} ST_ISP_AWB_RDMA, *PST_ISP_AWB_RDMA;
#pragma pack(pop)


///* auto white-balance registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_AWB_SetReg(
//    STF_U8 u8IspIdx,
//    ST_AWB_RDMA *pstAwbRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_RESULT STFHAL_ISP_AWB_SetChnCrv(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Chn,
//    STF_U16 au16Pi[],
//    STF_U16 au16Po[],
//    STF_U16 au16Ps[]
//    );
//extern
//STF_VOID STFHAL_ISP_AWB_SetPi(
//    STF_U8 u8IspIdx,
//    STF_U16 au16RPi[],
//    STF_U16 au16GrPi[],
//    STF_U16 au16GbPi[],
//    STF_U16 au16BPi[]);
//extern
//STF_VOID STFHAL_ISP_AWB_SetPo(
//    STF_U8 u8IspIdx,
//    STF_U16 au16RPo[],
//    STF_U16 au16GrPo[],
//    STF_U16 au16GbPo[],
//    STF_U16 au16BPo[]
//    );
//extern
//STF_VOID STFHAL_ISP_AWB_SetPs(
//    STF_U8 u8IspIdx,
//    STF_U16 au16RPs[],
//    STF_U16 au16GrPs[],
//    STF_U16 au16GbPs[],
//    STF_U16 au16BPs[]
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_AWB_RDMA_H__

