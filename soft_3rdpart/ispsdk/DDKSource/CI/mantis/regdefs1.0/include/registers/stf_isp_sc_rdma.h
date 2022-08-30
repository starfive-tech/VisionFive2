/**
  ******************************************************************************
  * @file  stf_isp_sc_rdma.h
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
#ifndef __STF_ISP_SC_RDMA_H__
#define __STF_ISP_SC_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add statistics registers structure */
//#define ST_SC_REG_INCLUDE_RSLT
//#if defined(ENABLE_USING_MEM_TO_SIM_REG)
#define SC_RESULT_HOR_MAX           (16)
#define SC_RESULT_VER_MAX           (16)
#define SC_RESULT_ADDR_MAX          (SC_RESULT_HOR_MAX * SC_RESULT_VER_MAX)
#define SC_RESULT_GENERAL_ITEM_SIZE ((EN_SC_ITEM_AE_ES + 1) * SC_RESULT_ADDR_MAX)
#define SC_RESULT_AE_HIST_ITEM_MAX  (4)
#define SC_RESULT_AE_HIST_ADDR_MAX  (64)
#define SC_RESULT_AE_HIST_ITEM_SIZE (SC_RESULT_AE_HIST_ITEM_MAX * SC_RESULT_AE_HIST_ADDR_MAX)
#define SC_RESULT_TOTAL_ITEM_SIZE   (SC_RESULT_GENERAL_ITEM_SIZE + SC_RESULT_AE_HIST_ITEM_SIZE)
//#endif //ENABLE_USING_MEM_TO_SIM_REG


typedef enum _EN_SC_IN_SEL {
    EN_SC_IN_SEL_AFTER_DEC = 0,             /** 0 : after DEC. */
    EN_SC_IN_SEL_AFTER_OBC,                 /** 1 : after OBC. */
    EN_SC_IN_SEL_AFTER_OECF,                /** 2 : after OECF. */
    EN_SC_IN_SEL_AFTER_AWB,                 /** 3 : after AWB WB Gain. */
    //EN_SC_IN_SEL_MAX
} EN_SC_IN_SEL, *PEN_SC_IN_SEL;             /** SC input MUX for AE/AF/AWB. */

typedef enum _EN_AF_SUM_MODE {
    EN_AF_SUM_MODE_ABSOLUTE = 0,            /** 0 : Absolute sum. */
    EN_AF_SUM_MODE_SQUARED,                 /** 1 : Squared sum. */
    //EN_AF_SUM_MODE_MAX
} EN_AF_SUM_MODE, *PEN_AF_SUM_MODE;

typedef enum _EN_SC_ITEM {
    EN_SC_ITEM_AE_WS_R = 0,
    EN_SC_ITEM_AE_WS_G,
    EN_SC_ITEM_AE_WS_B,
    EN_SC_ITEM_AE_WS_Y,
    EN_SC_ITEM_AWB_PS_R,
    EN_SC_ITEM_AWB_PS_G,
    EN_SC_ITEM_AWB_PS_B,
    EN_SC_ITEM_AWB_PS_CNT,
    EN_SC_ITEM_AWB_WGS_W,
    EN_SC_ITEM_AWB_WGS_RW,
    EN_SC_ITEM_AWB_WGS_GW,
    EN_SC_ITEM_AWB_WGS_BW,
    EN_SC_ITEM_AWB_WGS_GRW,
    EN_SC_ITEM_AWB_WGS_GBW,
    EN_SC_ITEM_AF_ES,
    EN_SC_ITEM_AE_ES,
    EN_SC_ITEM_AE_HIST_R,
    EN_SC_ITEM_AE_HIST_G,
    EN_SC_ITEM_AE_HIST_B,
    EN_SC_ITEM_AE_HIST_Y,
    //EN_SC_ITEM_MAX
} EN_SC_ITEM, *PEN_SC_ITEM;


typedef struct _ST_DUMP_BUF_ADDR {
    STF_U8 u8IspIdx;
    STF_U32 u32Address;
} ST_DUMP_BUF_ADDR, *PST_DUMP_BUF_ADDR;

typedef struct _ST_SC_AXI_PARAM {
    STF_U8 u8IspIdx;
    STF_U8 u8AxiId;
} ST_SC_AXI_PARAM, *PST_SC_AXI_PARAM;

typedef struct _ST_SC_FRAME_CROPPING {
    STF_U8 u8IspIdx;
    ST_POINT stStartPoint;
} ST_SC_FRAME_CROPPING, *PST_SC_FRAME_CROPPING;

typedef struct _ST_SC_DECIMATION {
    STF_U8 u8IspIdx;
    ST_DEC stDec;
} ST_SC_DECIMATION, *PST_SC_DECIMATION;

typedef struct _ST_SC_RANGE {
    STF_U8 u8IspIdx;
    ST_RANGE stRange;
} ST_SC_RANGE, *PST_SC_RANGE;

typedef struct _ST_DUMP_RSLT_CTRL {
    STF_U8 u8Addr;
    STF_U8 u8ItemIdx;
} ST_DUMP_RSLT_CTRL, *PST_DUMP_RSLT_CTRL;

typedef struct _ST_SC_DUMP_RSLT_CTRL_PARAM {
    STF_U8 u8IspIdx;
    ST_DUMP_RSLT_CTRL stDumpResultCtrl;
} ST_SC_DUMP_RSLT_CTRL_PARAM, *PST_SC_DUMP_RSLT_CTRL_PARAM;

typedef struct _ST_SC_RSLT_PARAM {
    STF_U8 u8IspIdx;
    STF_U32 u32Result;
} ST_SC_RSLT_PARAM, *PST_SC_RSLT_PARAM;

typedef struct _ST_SC_RSLT_CTRL {
    STF_U8 u8Addr;
    STF_U8 u8ItemIdx;
    STF_U16 u16Count;
} ST_SC_RSLT_CTRL, *PST_SC_RSLT_CTRL;

typedef struct _ST_SC_RSLTS {
    ST_SC_RSLT_CTRL stScResultCtrl;
    STF_U32 pu32Buf[];
} ST_SC_RSLTS, *PST_SC_RSLTS;

typedef struct _ST_SC_RSLTS_PARAM {
    STF_U8 u8IspIdx;
    ST_SC_RSLTS stResults;
} ST_SC_RSLTS_PARAM, *PST_SC_RSLTS_PARAM;

typedef struct _ST_AE_AF_CFG {
    STF_U8 u8SubWinWidth;
    STF_U8 u8SubWinHeight;
    EN_SC_IN_SEL enSelect;
} ST_AE_AF_CFG, *PST_AE_AF_CFG;

typedef struct _ST_SC_AE_AF_CFG_PARAM {
    STF_U8 u8IspIdx;
    ST_AE_AF_CFG stAeAfCfg;
} ST_SC_AE_AF_CFG_PARAM, *PST_SC_AE_AF_CFG_PARAM;

typedef struct _ST_AF_CFG {
    STF_BOOL8 bHorzMode;
    STF_BOOL8 bSumMode;
    STF_BOOL8 bHorzEnable;
    STF_BOOL8 bVertEnable;
    STF_U16 u16VertThres;
    STF_U16 u16HorzThres;
} ST_AF_CFG, *PST_AF_CFG;

typedef struct _ST_SC_AF_CFG_PARAM {
    STF_U8 u8IspIdx;
    ST_AF_CFG stAfCfg;
} ST_SC_AF_CFG_PARAM, *PST_SC_AF_CFG_PARAM;

typedef struct _ST_AWB_CFG {
    STF_U8 u8SubWinWidth;
    STF_U8 u8SubWinHeight;
    EN_SC_IN_SEL enSelect;
    STF_U8 u8AwbBaVal;
} ST_AWB_CFG, *PST_AWB_CFG;

typedef struct _ST_SC_AWB_CFG_PARAM {
    STF_U8 u8IspIdx;
    ST_AWB_CFG stAwbCfg;
} ST_SC_AWB_CFG_PARAM, *PST_SC_AWB_CFG_PARAM;

typedef struct _ST_AWB_WT_TBL {
    STF_U8 au8WeightingTable[13][13];
} ST_AWB_WT_TBL, *PST_AWB_WT_TBL;

typedef struct _ST_SC_AWB_WT_TBL_PARAM {
    STF_U8 u8IspIdx;
    ST_AWB_WT_TBL stWeightingTable;
} ST_SC_AWB_WT_TBL_PARAM, *PST_SC_AWB_WT_TBL_PARAM;

typedef struct _ST_AWB_INTSTY_WT_CRV {
    STF_U8 u8Y[16];
    STF_S8 s8Slope[16];
} ST_AWB_INTSTY_WT_CRV, *PST_AWB_INTSTY_WT_CRV;

typedef struct _ST_SC_AWB_INTSTY_WT_CRV_PARAM {
    STF_U8 u8IspIdx;
    ST_AWB_INTSTY_WT_CRV stIntensityWeightCurve;
} ST_SC_AWB_INTSTY_WT_CRV_PARAM, *PST_SC_AWB_INTSTY_WT_CRV_PARAM;

typedef struct _ST_SCD_CFG_REG {
#if defined(V4L2_DRIVER)
#else
    REG_SCD_CFG_0               DumpBufAddr;        // 00 - 0x0098, SC dumping base address.
#endif //#if defined(V4L2_DRIVER)
    REG_SCD_CFG_1               AxiId;              // 01 - 0x009C, SC dumping axi id.
} ST_SCD_CFG_REG, *PST_SCD_CFG_REG;

#define ST_SCD_CFG_REG_SIZE     (sizeof(ST_SCD_CFG_REG))
#define ST_SCD_CFG_REG_LEN      (ST_SCD_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_CFG_REG {
    REG_SC_CFG_0                ScCfg0;             // 02 - 0x00B8, SC frame cropping for sc ae/af/awb.
    REG_SC_CFG_1                ScCfg1;             // 03 - 0x00BC, SC cfg1 register.
} ST_SC_CFG_REG, *PST_SC_CFG_REG;

#define ST_SC_CFG_REG_SIZE      (sizeof(ST_SC_CFG_REG))
#define ST_SC_CFG_REG_LEN       (ST_SC_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_DEC_REG {
    REG_SC_DEC                  ScDec;              // 04 - 0x00D8, sc0 dec cfg register for sc awb, Horizontal/Vertical period/keep.
} ST_SC_DEC_REG, *PST_SC_DEC_REG;

#define ST_SC_DEC_REG_SIZE      (sizeof(ST_SC_DEC_REG))
#define ST_SC_DEC_REG_LEN       (ST_SC_DEC_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_AF_REG {
    REG_SC_AF                   ScAf;               // 05 - 0x00C0, SC AF register.
} ST_SC_AF_REG, *PST_SC_AF_REG;

#define ST_SC_AF_REG_SIZE       (sizeof(ST_SC_AF_REG))
#define ST_SC_AF_REG_LEN        (ST_SC_AF_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_PS_CFG_REG {
    REG_SC_AWB_PS_CFG_0         ScAwbPsCfg0;        // 06 - 0x00C4, SC AWB pixel sum registers0
    REG_SC_AWB_PS_CFG_1         ScAwbPsCfg1;        // 07 - 0x00C8, SC AWB pixel sum registers1
    REG_SC_AWB_PS_CFG_2         ScAwbPsCfg2;        // 08 - 0x00CC, SC AWB pixel sum registers2
    REG_SC_AWB_PS_CFG_3         ScAwbPsCfg3;        // 09 - 0x00D0, SC AWB pixel sum registers3
    REG_SC_AWB_PS_CFG_4         ScAwbPsCfg4;        // 10 - 0x00D4, SC AWB pixel sum registers4
} ST_SC_PS_CFG_REG, *PST_SC_PS_CFG_REG;

#define ST_SC_PS_CFG_REG_SIZE   (sizeof(ST_SC_PS_CFG_REG))
#define ST_SC_PS_CFG_REG_LEN    (ST_SC_PS_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_WS_CFG_REG {
    REG_SC_AWB_WS_CFG_0         ScAwbWsCfg0;        // 11 - 0x05D0, SC AWB weight sum registers0
    REG_SC_AWB_WS_CFG_1         ScAwbWsCfg1;        // 12 - 0x05D4, SC AWB weight sum registers1
} ST_SC_WS_CFG_REG, *PST_SC_WS_CFG_REG;

#define ST_SC_WS_CFG_REG_SIZE   (sizeof(ST_SC_WS_CFG_REG))
#define ST_SC_WS_CFG_REG_LEN    (ST_SC_WS_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_WT_REG {
    REG_SC_AWB_WS_CW0_CFG_0     ScAwbWsCw0Cfg0;     // 13 - 0x04D0, Weighting value at point(0,0~7), each 4bits.
    REG_SC_AWB_WS_CW0_CFG_1     ScAwbWsCw0Cfg1;     // 14 - 0x04D4, Weighting value at point(0,8~12), each 4bits.
    REG_SC_AWB_WS_CW1_CFG_0     ScAwbWsCw1Cfg0;     // 15 - 0x04D8, Weighting value at point(1,0~7), each 4bits.
    REG_SC_AWB_WS_CW1_CFG_1     ScAwbWsCw1Cfg1;     // 16 - 0x04DC, Weighting value at point(1,8~12), each 4bits.
    REG_SC_AWB_WS_CW2_CFG_0     ScAwbWsCw2Cfg0;     // 17 - 0x04E0, Weighting value at point(2,0~7), each 4bits.
    REG_SC_AWB_WS_CW2_CFG_1     ScAwbWsCw2Cfg1;     // 18 - 0x04E4, Weighting value at point(2,8~12), each 4bits.
    REG_SC_AWB_WS_CW3_CFG_0     ScAwbWsCw3Cfg0;     // 19 - 0x04E8, Weighting value at point(3,0~7), each 4bits.
    REG_SC_AWB_WS_CW3_CFG_1     ScAwbWsCw3Cfg1;     // 20 - 0x04EC, Weighting value at point(3,8~12), each 4bits.
    REG_SC_AWB_WS_CW4_CFG_0     ScAwbWsCw4Cfg0;     // 21 - 0x04F0, Weighting value at point(4,0~7), each 4bits.
    REG_SC_AWB_WS_CW4_CFG_1     ScAwbWsCw4Cfg1;     // 22 - 0x04F4, Weighting value at point(4,8~12), each 4bits.
    REG_SC_AWB_WS_CW5_CFG_0     ScAwbWsCw5Cfg0;     // 23 - 0x04F8, Weighting value at point(5,0~7), each 4bits.
    REG_SC_AWB_WS_CW5_CFG_1     ScAwbWsCw5Cfg1;     // 24 - 0x04FC, Weighting value at point(5,8~12), each 4bits.
    REG_SC_AWB_WS_CW6_CFG_0     ScAwbWsCw6Cfg0;     // 25 - 0x0500, Weighting value at point(6,0~7), each 4bits.
    REG_SC_AWB_WS_CW6_CFG_1     ScAwbWsCw6Cfg1;     // 26 - 0x0504, Weighting value at point(6,8~12), each 4bits.
    REG_SC_AWB_WS_CW7_CFG_0     ScAwbWsCw7Cfg0;     // 27 - 0x0508, Weighting value at point(7,0~7), each 4bits.
    REG_SC_AWB_WS_CW7_CFG_1     ScAwbWsCw7Cfg1;     // 28 - 0x050C, Weighting value at point(7,8~12), each 4bits.
    REG_SC_AWB_WS_CW8_CFG_0     ScAwbWsCw8Cfg0;     // 29 - 0x0510, Weighting value at point(8,0~7), each 4bits.
    REG_SC_AWB_WS_CW8_CFG_1     ScAwbWsCw8Cfg1;     // 30 - 0x0514, Weighting value at point(8,8~12), each 4bits.
    REG_SC_AWB_WS_CW9_CFG_0     ScAwbWsCw9Cfg0;     // 31 - 0x0518, Weighting value at point(9,0~7), each 4bits.
    REG_SC_AWB_WS_CW9_CFG_1     ScAwbWsCw9Cfg1;     // 32 - 0x051C, Weighting value at point(9,8~12), each 4bits.
    REG_SC_AWB_WS_CW10_CFG_0    ScAwbWsCw10Cfg0;    // 33 - 0x0520, Weighting value at point(10,0~7), each 4bits.
    REG_SC_AWB_WS_CW10_CFG_1    ScAwbWsCw10Cfg1;    // 34 - 0x0524, Weighting value at point(10,8~12), each 4bits.
    REG_SC_AWB_WS_CW11_CFG_0    ScAwbWsCw11Cfg0;    // 35 - 0x0528, Weighting value at point(11,0~7), each 4bits.
    REG_SC_AWB_WS_CW11_CFG_1    ScAwbWsCw11Cfg1;    // 36 - 0x052C, Weighting value at point(11,8~12), each 4bits.
    REG_SC_AWB_WS_CW12_CFG_0    ScAwbWsCw12Cfg0;    // 37 - 0x0530, Weighting value at point(12,0~7), each 4bits.
    REG_SC_AWB_WS_CW12_CFG_1    ScAwbWsCw12Cfg1;    // 38 - 0x0534, Weighting value at point(12,8~12), each 4bits.
} ST_SC_WT_REG, *PST_SC_WT_REG;

#define ST_SC_WT_REG_SIZE       (sizeof(ST_SC_WT_REG))
#define ST_SC_WT_REG_LEN        (ST_SC_WT_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_WC_REG {
    REG_SC_AWB_WS_IWV_CFG_0     ScAwbWsIwvCfg0;     // 39 - 0x0538, [0.4.0]Weighting value at point(0~7), each 4bits
    REG_SC_AWB_WS_IWV_CFG_1     ScAwbWsIwvCfg1;     // 40 - 0x053C, [0.4.0]Weighting value at point(8~15), each 4bits
    REG_SC_AWB_WS_IWS_CFG_0     ScAwbWsIwsCfg0;     // 41 - 0x0540, [1.2.5]Slope value at point(0~3), each 8bits
    REG_SC_AWB_WS_IWS_CFG_1     ScAwbWsIwsCfg1;     // 42 - 0x0544, [1.2.5]Slope value at point(4~7), each 8bits
    REG_SC_AWB_WS_IWS_CFG_2     ScAwbWsIwsCfg2;     // 43 - 0x0548, [1.2.5]Slope value at point(8~11), each 8bits
    REG_SC_AWB_WS_IWS_CFG_3     ScAwbWsIwsCfg3;     // 44 - 0x054C, [1.2.5]Slope value at point(12~15), each 8bits
} ST_SC_WC_REG, *PST_SC_WC_REG;

#define ST_SC_WC_REG_SIZE       (sizeof(ST_SC_WC_REG))
#define ST_SC_WC_REG_LEN        (ST_SC_WC_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_RSLT_REG {
    REG_SCR_ADR                 ScResultAddr;       // 45 - 0x00F8, SC read index, Write Only.
    REG_SCR_DAT                 ScResultDara;       // 46 - 0x00FC, SC read data, Read only.
} ST_SC_RSLT_REG, *PST_SC_RSLT_REG;

#define ST_SC_RSLT_REG_SIZE     (sizeof(ST_SC_RSLT_REG))
#define ST_SC_RSLT_REG_LEN      (ST_SC_RSLT_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_SC_REG {
    //-------------------------------------------------------------------------
    REG_SCD_CFG_0               DumpBufAddr;        // 00 - 0x0098, SC dumping base address.
    REG_SCD_CFG_1               AxiId;              // 01 - 0x009C, SC dumping axi id.
    //-------------------------------------------------------------------------
    REG_SC_CFG_0                ScCfg0;             // 02 - 0x00B8, SC frame cropping for sc ae/af/awb.
    REG_SC_CFG_1                ScCfg1;             // 03 - 0x00BC, SC cfg1 register.
    //-------------------------------------------------------------------------
    REG_SC_DEC                  ScDec;              // 04 - 0x00D8, sc0 dec cfg register for sc awb, Horizontal/Vertical period/keep.
    //-------------------------------------------------------------------------
    REG_SC_AF                   ScAf;               // 05 - 0x00C0, SC AF register.
    //-------------------------------------------------------------------------
    REG_SC_AWB_PS_CFG_0         ScAwbPsCfg0;        // 06 - 0x00C4, SC AWB pixel sum registers0
    REG_SC_AWB_PS_CFG_1         ScAwbPsCfg1;        // 07 - 0x00C8, SC AWB pixel sum registers1
    REG_SC_AWB_PS_CFG_2         ScAwbPsCfg2;        // 08 - 0x00CC, SC AWB pixel sum registers2
    REG_SC_AWB_PS_CFG_3         ScAwbPsCfg3;        // 09 - 0x00D0, SC AWB pixel sum registers3
    REG_SC_AWB_PS_CFG_4         ScAwbPsCfg4;        // 10 - 0x00D4, SC AWB pixel sum registers4
    //-------------------------------------------------------------------------
    REG_SC_AWB_WS_CFG_0         ScAwbWsCfg0;        // 11 - 0x05D0, SC AWB weight sum registers0
    REG_SC_AWB_WS_CFG_1         ScAwbWsCfg1;        // 12 - 0x05D4, SC AWB weight sum registers1
    //-------------------------------------------------------------------------
    REG_SC_AWB_WS_CW0_CFG_0     ScAwbWsCw0Cfg0;     // 13 - 0x04D0, Weighting value at point(0,0~7), each 4bits.
    REG_SC_AWB_WS_CW0_CFG_1     ScAwbWsCw0Cfg1;     // 14 - 0x04D4, Weighting value at point(0,8~12), each 4bits.
    REG_SC_AWB_WS_CW1_CFG_0     ScAwbWsCw1Cfg0;     // 15 - 0x04D8, Weighting value at point(1,0~7), each 4bits.
    REG_SC_AWB_WS_CW1_CFG_1     ScAwbWsCw1Cfg1;     // 16 - 0x04DC, Weighting value at point(1,8~12), each 4bits.
    REG_SC_AWB_WS_CW2_CFG_0     ScAwbWsCw2Cfg0;     // 17 - 0x04E0, Weighting value at point(2,0~7), each 4bits.
    REG_SC_AWB_WS_CW2_CFG_1     ScAwbWsCw2Cfg1;     // 18 - 0x04E4, Weighting value at point(2,8~12), each 4bits.
    REG_SC_AWB_WS_CW3_CFG_0     ScAwbWsCw3Cfg0;     // 19 - 0x04E8, Weighting value at point(3,0~7), each 4bits.
    REG_SC_AWB_WS_CW3_CFG_1     ScAwbWsCw3Cfg1;     // 20 - 0x04EC, Weighting value at point(3,8~12), each 4bits.
    REG_SC_AWB_WS_CW4_CFG_0     ScAwbWsCw4Cfg0;     // 21 - 0x04F0, Weighting value at point(4,0~7), each 4bits.
    REG_SC_AWB_WS_CW4_CFG_1     ScAwbWsCw4Cfg1;     // 22 - 0x04F4, Weighting value at point(4,8~12), each 4bits.
    REG_SC_AWB_WS_CW5_CFG_0     ScAwbWsCw5Cfg0;     // 23 - 0x04F8, Weighting value at point(5,0~7), each 4bits.
    REG_SC_AWB_WS_CW5_CFG_1     ScAwbWsCw5Cfg1;     // 24 - 0x04FC, Weighting value at point(5,8~12), each 4bits.
    REG_SC_AWB_WS_CW6_CFG_0     ScAwbWsCw6Cfg0;     // 25 - 0x0500, Weighting value at point(6,0~7), each 4bits.
    REG_SC_AWB_WS_CW6_CFG_1     ScAwbWsCw6Cfg1;     // 26 - 0x0504, Weighting value at point(6,8~12), each 4bits.
    REG_SC_AWB_WS_CW7_CFG_0     ScAwbWsCw7Cfg0;     // 27 - 0x0508, Weighting value at point(7,0~7), each 4bits.
    REG_SC_AWB_WS_CW7_CFG_1     ScAwbWsCw7Cfg1;     // 28 - 0x050C, Weighting value at point(7,8~12), each 4bits.
    REG_SC_AWB_WS_CW8_CFG_0     ScAwbWsCw8Cfg0;     // 29 - 0x0510, Weighting value at point(8,0~7), each 4bits.
    REG_SC_AWB_WS_CW8_CFG_1     ScAwbWsCw8Cfg1;     // 30 - 0x0514, Weighting value at point(8,8~12), each 4bits.
    REG_SC_AWB_WS_CW9_CFG_0     ScAwbWsCw9Cfg0;     // 31 - 0x0518, Weighting value at point(9,0~7), each 4bits.
    REG_SC_AWB_WS_CW9_CFG_1     ScAwbWsCw9Cfg1;     // 32 - 0x051C, Weighting value at point(9,8~12), each 4bits.
    REG_SC_AWB_WS_CW10_CFG_0    ScAwbWsCw10Cfg0;    // 33 - 0x0520, Weighting value at point(10,0~7), each 4bits.
    REG_SC_AWB_WS_CW10_CFG_1    ScAwbWsCw10Cfg1;    // 34 - 0x0524, Weighting value at point(10,8~12), each 4bits.
    REG_SC_AWB_WS_CW11_CFG_0    ScAwbWsCw11Cfg0;    // 35 - 0x0528, Weighting value at point(11,0~7), each 4bits.
    REG_SC_AWB_WS_CW11_CFG_1    ScAwbWsCw11Cfg1;    // 36 - 0x052C, Weighting value at point(11,8~12), each 4bits.
    REG_SC_AWB_WS_CW12_CFG_0    ScAwbWsCw12Cfg0;    // 37 - 0x0530, Weighting value at point(12,0~7), each 4bits.
    REG_SC_AWB_WS_CW12_CFG_1    ScAwbWsCw12Cfg1;    // 38 - 0x0534, Weighting value at point(12,8~12), each 4bits.
    //-------------------------------------------------------------------------
    REG_SC_AWB_WS_IWV_CFG_0     ScAwbWsIwvCfg0;     // 39 - 0x0538, [0.4.0]Weighting value at point(0~7), each 4bits
    REG_SC_AWB_WS_IWV_CFG_1     ScAwbWsIwvCfg1;     // 40 - 0x053C, [0.4.0]Weighting value at point(8~15), each 4bits
    REG_SC_AWB_WS_IWS_CFG_0     ScAwbWsIwsCfg0;     // 41 - 0x0540, [1.2.5]Slope value at point(0~3), each 8bits
    REG_SC_AWB_WS_IWS_CFG_1     ScAwbWsIwsCfg1;     // 42 - 0x0544, [1.2.5]Slope value at point(4~7), each 8bits
    REG_SC_AWB_WS_IWS_CFG_2     ScAwbWsIwsCfg2;     // 43 - 0x0548, [1.2.5]Slope value at point(8~11), each 8bits
    REG_SC_AWB_WS_IWS_CFG_3     ScAwbWsIwsCfg3;     // 44 - 0x054C, [1.2.5]Slope value at point(12~15), each 8bits
    //-------------------------------------------------------------------------
#if defined(ST_SC_REG_INCLUDE_RSLT)
    REG_SCR_ADR                 ScResultAddr;       // 45 - 0x00F8, SC read index, Write Only.
    REG_SCR_DAT                 ScResultDara;       // 46 - 0x00FC, SC read data, Read only.
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
    //-------------------------------------------------------------------------
} ST_SC_REG, *PST_SC_REG;

#define ST_SC_REG_SIZE          (sizeof(ST_SC_REG))
#define ST_SC_REG_LEN           (ST_SC_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_SC_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SCD_CFG_REG stScdCfgReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaCfgCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_CFG_REG stScCfgReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaDecCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_DEC_REG stScDecgReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaAfCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_AF_REG stScAfReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaPsCfgCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_PS_CFG_REG stScPsCfgReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaWsCfgCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_WS_CFG_REG stScWsCfgReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaWtCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_WT_REG stScWtReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaWcCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_WC_REG stScWcReg __attribute__ ((aligned (8)));
#if defined(ST_SC_REG_INCLUDE_RSLT)
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaRsltCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_SC_RSLT_REG stScRsltReg __attribute__ ((aligned (8)));
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_SC_RDMA, *PST_SC_RDMA;

#define ST_SC_RDMA_SIZE         (sizeof(ST_SC_RDMA))
#define ST_SC_RDMA_LEN          (ST_SC_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SCD_CFG_RDMA_SIZE    (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SCD_CFG_REG) + 7) / 8) * 8))
#define ST_SCD_CFG_RDMA_LEN     (ST_SCD_CFG_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SC_CFG_RDMA_SIZE     (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_CFG_REG) + 7) / 8) * 8))
#define ST_SC_CFG_RDMA_LEN      (ST_SC_CFG_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SC_DEC_RDMA_SIZE     (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_DEC_REG) + 7) / 8) * 8))
#define ST_SC_DEC_RDMA_LEN      (ST_SC_DEC_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SC_AF_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_AF_REG) + 7) / 8) * 8))
#define ST_SC_AF_RDMA_LEN       (ST_SC_AF_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SC_PS_CFG_RDMA_SIZE  (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_PS_CFG_REG) + 7) / 8) * 8))
#define ST_SC_PS_CFG_RDMA_LEN   (ST_SC_PS_CFG_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SC_WS_CFG_RDMA_SIZE  (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_WS_CFG_REG) + 7) / 8) * 8))
#define ST_SC_WS_CFG_RDMA_LEN   (ST_SC_WS_CFG_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SC_WT_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_WT_REG) + 7) / 8) * 8))
#define ST_SC_WT_RDMA_LEN       (ST_SC_WT_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_SC_WC_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_WC_REG) + 7) / 8) * 8))
#define ST_SC_WC_RDMA_LEN       (ST_SC_WC_RDMA_SIZE / (sizeof(STF_U32) * 2))
#if defined(ST_SC_REG_INCLUDE_RSLT)
#define ST_SC_RSLT_RDMA_SIZE    (sizeof(ST_RDMA_CMD) + (((sizeof(ST_SC_RSLT_REG) + 7) / 8) * 8))
#define ST_SC_RSLT_RDMA_LEN     (ST_SC_RSLT_RDMA_SIZE / (sizeof(STF_U32) * 2))
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)

typedef struct _ST_ISP_SC_RDMA {
    STF_U8 u8IspIdx;
    ST_SC_RDMA stScRdma __attribute__ ((aligned (8)));
} ST_ISP_SC_RDMA, *PST_ISP_SC_RDMA;
#pragma pack(pop)


///* statistics registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SC_SetReg(
//    STF_U8 u8IspIdx,
//    ST_SC_RDMA *pstScRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetCfgReg(
//    STF_U8 u8IspIdx,
//    ST_SC_RDMA *pstScRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetCropReg(
//    STF_U8 u8IspIdx,
//    ST_SC_RDMA *pstScRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetDecReg(
//    STF_U8 u8IspIdx,
//    ST_SC_RDMA *pstScRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAeAfReg(
//    STF_U8 u8IspIdx,
//    ST_SC_RDMA *pstScRdma
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbReg(
//    STF_U8 u8IspIdx,
//    ST_SC_RDMA *pstScRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SC_SetDumpingAddress(
//    STF_U8 u8IspIdx,
//    STF_U32 u32Address
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAxiId(
//    STF_U8 u8IspIdx,
//    STF_U8 u8AxiId
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAeAfFrameCropping(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Hstart,
//    STF_U16 u16Vstart
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAeAfConfig(
//    STF_U8 u8IspIdx,
//    STF_U8 u8SubWinWidth,
//    STF_U8 u8SubWinHeight,
//    EN_SC_IN_SEL enSelect
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAeAfDecimation(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Hperiod,
//    STF_U8 u8Hkeep,
//    STF_U8 u8Vperiod,
//    STF_U8 u8Vkeep
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAfConfig(
//    STF_U8 u8IspIdx,
//    STF_BOOL8 bHorzMode,
//    STF_BOOL8 bSumMode,
//    STF_BOOL8 bHorzEnable,
//    STF_BOOL8 bVertEnable,
//    STF_U16 u16HorzThres,
//    STF_U16 u16VertThres
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbFrameCropping(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Hstart,
//    STF_U16 u16Vstart
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbConfig(
//    STF_U8 u8IspIdx,
//    STF_U8 u8SubWinWidth,
//    STF_U8 u8SubWinHeight,
//    EN_SC_IN_SEL enSelect,
//    STF_U8 u8AwbBaVal
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbDecimation(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Hperiod,
//    STF_U8 u8Hkeep,
//    STF_U8 u8Vperiod,
//    STF_U8 u8Vkeep
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbPixelSumRangeR(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbPixelSumRangeG(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbPixelSumRangeB(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbPixelSumRangeY(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbPixelSumRangeGR(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Low,
//    STF_U16 u16Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbPixelSumRangeGB(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Low,
//    STF_U16 u16Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbPixelSumRangeGRB(
//    STF_U8 u8IspIdx,
//    STF_U16 u16Low,
//    STF_U16 u16Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbWeightSumRangeR(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbWeightSumRangeGr(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbWeightSumRangeGb(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbWeightSumRangeB(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Low,
//    STF_U8 u8Upper
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbWeightTable(
//    STF_U8 u8IspIdx,
//    STF_U8 pau8WightingTable[][13]
//    );
//extern
//STF_VOID STFHAL_ISP_SC_SetAwbIntensityWeightCurve(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Y[],
//    STF_S8 s8Slope[]
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_SC_SetDumpingResultControl(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Addr,
//    STF_U8 u8ItemIdx
//    );
//extern
//STF_U32 STFHAL_ISP_SC_GetResult(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_SC_GetResults(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Addr,
//    STF_U8 u8ItemIdx,
//    STF_U16 u16Count,
//    STF_U32 *pu32Buf
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_SC_RDMA_H__
