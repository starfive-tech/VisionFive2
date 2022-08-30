/**
  ******************************************************************************
  * @file  ISP_mod_sc_test.c
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

#include <unistd.h>

#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"

#include "ISPC/stflib_isp_base.h"

#define LOG_TAG "Mod_Sc_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_sc_test.h"
#include "ISP_test.h"


/* ISPC SC driver test structure */


/* ISPC SC driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;
static STF_CHAR g_szScInSel[][64] = {
    "EN_SC_IN_SEL_AFTER_DEC",
    "EN_SC_IN_SEL_AFTER_OBC",
    "EN_SC_IN_SEL_AFTER_OECF",
    "EN_SC_IN_SEL_AFTER_AWB",
};
static STF_CHAR g_szScItem[][64] = {
    "EN_SC_ITEM_AE_WS_R",
    "EN_SC_ITEM_AE_WS_G",
    "EN_SC_ITEM_AE_WS_B",
    "EN_SC_ITEM_AE_WS_Y",
    "EN_SC_ITEM_AWB_PS_R",
    "EN_SC_ITEM_AWB_PS_G",
    "EN_SC_ITEM_AWB_PS_B",
    "EN_SC_ITEM_AWB_PS_CNT",
    "EN_SC_ITEM_AWB_WGS_W",
    "EN_SC_ITEM_AWB_WGS_RW",
    "EN_SC_ITEM_AWB_WGS_GW",
    "EN_SC_ITEM_AWB_WGS_BW",
    "EN_SC_ITEM_AWB_WGS_GRW",
    "EN_SC_ITEM_AWB_WGS_GBW",
    "EN_SC_ITEM_AF_ES",
    "EN_SC_ITEM_AE_ES",
    "EN_SC_ITEM_AE_HIST_R",
    "EN_SC_ITEM_AE_HIST_G",
    "EN_SC_ITEM_AE_HIST_B",
    "EN_SC_ITEM_AE_HIST_Y",
};


/* ISPC SC driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_SC_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
#if defined(ST_SC_REG_INCLUDE_RSLT)
    STF_U32 u32Buffer[(ST_SC_REG_LEN - 1) * 2];
#else
    STF_U32 u32Buffer[ST_SC_REG_LEN * 2];
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if defined(ST_SC_REG_INCLUDE_RSLT)
    u32Length = ST_SC_REG_LEN - 1;
#else
    u32Length = ST_SC_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
    for (i = 0; i < u32Length; i++) {
        switch (i) {
            case 0 ... 1:
                u32Buffer[i * 2] = ADDR_REG_SCD_CFG_0 + ((i - 0) * 4);
                break;

            case 2 ... 3:
                u32Buffer[i * 2] = ADDR_REG_SC_CFG_0 + ((i - 2) * 4);
                break;

            case 4:
                u32Buffer[i * 2] = ADDR_REG_SC_DEC + ((i - 4) * 4);
                break;

            case 5:
                u32Buffer[i * 2] = ADDR_REG_SC_AF + ((i - 5) * 4);
                break;

            case 6 ... 10:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_PS_CFG_0 + ((i - 6) * 4);
                break;

            case 11 ... 12:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CFG_0 + ((i - 11) * 4);
                break;

            case 13 ... 38:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CW0_CFG_0
                    + ((i - 13) * 4);
                break;

            case 39 ... 44:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_IWV_CFG_0
                    + ((i - 39) * 4);
                break;
#if defined(ST_SC_REG_INCLUDE_RSLT)

            case 45:
                u32Buffer[i * 2] = ADDR_REG_SCR_ADR + ((i - 45) * 4);
                break;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        }
        u32Buffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U16 *pu16Temp = NULL;
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SC_RDMA stIspScRdma;
#if defined(ST_SC_REG_INCLUDE_RSLT)
        ST_SC_DUMP_RSLT_CTRL_PARAM stScDumpResultCtrl;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        REG_SC_AWB_WS_CW0_CFG_0 *pScAwbWsCwCfg0 = NULL;
        REG_SC_AWB_WS_CW0_CFG_1 *pScAwbWsCwCfg1 = NULL;

        stIspScRdma.stScRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_1;
#else
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_0;
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.u32Param = ST_SCD_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaCfgCmd.RegAddr = ADDR_REG_SC_CFG_0;
        stIspScRdma.stScRdma.stRdmaCfgCmd.u32Param = ST_SC_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaDecCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaDecCmd.RegAddr = ADDR_REG_SC_DEC;
        stIspScRdma.stScRdma.stRdmaDecCmd.u32Param = ST_SC_DEC_REG_LEN;
        stIspScRdma.stScRdma.stRdmaAfCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaAfCmd.RegAddr = ADDR_REG_SC_AF;
        stIspScRdma.stScRdma.stRdmaAfCmd.u32Param = ST_SC_AF_REG_LEN;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.RegAddr = ADDR_REG_SC_AWB_PS_CFG_0;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.u32Param = ST_SC_PS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.RegAddr = ADDR_REG_SC_AWB_WS_CFG_0;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.u32Param = ST_SC_WS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWtCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWtCmd.RegAddr = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
        stIspScRdma.stScRdma.stRdmaWtCmd.u32Param = ST_SC_WT_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWcCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWcCmd.RegAddr = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
        stIspScRdma.stScRdma.stRdmaWcCmd.u32Param = ST_SC_WC_REG_LEN;
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaRsltCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.RegAddr = ADDR_REG_SCR_ADR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.u32Param = ST_SC_RSLT_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaEndCmd.Tag = DESP_END;

#if defined(V4L2_DRIVER)
#else
        stIspScRdma.stScRdma.stScdCfgReg.DumpBufAddr.value =
            (rand() & 0xFFFFFF80);
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stScdCfgReg.AxiId.value = ((rand() & 0xFF) << 24);
        stIspScRdma.stScRdma.stScCfgReg.ScCfg0.value = (rand() & 0xFFFF)
            | ((rand() & 0xFFFF) << 16);
        stIspScRdma.stScRdma.stScCfgReg.ScCfg1.value = (rand() & 0x1F)
            | ((rand() & 0x1F) << 8)
            | ((rand() & 0xFF) << 16)
            | ((rand() & 0x3) << 30);
        stIspScRdma.stScRdma.stScDecgReg.ScDec.value = (rand() & 0x1F)
            | ((rand() & 0x1F) << 8)
            | ((rand() & 0x1F) << 16)
            | ((rand() & 0x1F) << 24);
        stIspScRdma.stScRdma.stScAfReg.ScAf.value = (rand() & 0x1)
            | ((rand() & 0x1) << 1)
            | ((rand() & 0x1) << 2)
            | ((rand() & 0x1) << 3)
            | ((rand() & 0xFF) << 8)
            | ((rand() & 0x1FF) << 16);
        u32Cnt = 8;
        pu8Temp = (STF_U8 *)&stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0xFF);
        }
        u32Cnt = 6;
        pu16Temp = (STF_U16 *)&stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0xFFFF);
        }
        u32Cnt = 8;
        pu8Temp = (STF_U8 *)&stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0xFF);
        }
        u32Cnt = 13;
        pu32Temp = (STF_U32 *)&stIspScRdma.stScRdma.stScWtReg.ScAwbWsCw0Cfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx * 2] = (rand() & 0xF)
                | ((rand() & 0xF) << 4)
                | ((rand() & 0xF) << 8)
                | ((rand() & 0xF) << 12)
                | ((rand() & 0xF) << 16)
                | ((rand() & 0xF) << 20)
                | ((rand() & 0xF) << 24)
                | ((rand() & 0xF) << 28);
            pu32Temp[u32Idx * 2 + 1] = (rand() & 0xF)
                | ((rand() & 0xF) << 4)
                | ((rand() & 0xF) << 8)
                | ((rand() & 0xF) << 12)
                | ((rand() & 0xF) << 16);
        }
        u32Cnt = 2;
        pu32Temp = (STF_U32 *)&stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0xF)
                | ((rand() & 0xF) << 4)
                | ((rand() & 0xF) << 8)
                | ((rand() & 0xF) << 12)
                | ((rand() & 0xF) << 16)
                | ((rand() & 0xF) << 20)
                | ((rand() & 0xF) << 24)
                | ((rand() & 0xF) << 28);
        }
        u32Cnt = 16;
        pu8Temp = (STF_U8 *)&stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0xFF);
        }
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.value = 0;
        do {
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.idx =
                (rand() & 0x1F);
        } while (EN_SC_ITEM_AE_HIST_Y
            < stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.idx);
        if (EN_SC_ITEM_AE_HIST_R
            > stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.idx) {
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.adr =
                (rand() & 0xFF);
        } else {
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.adr =
                (rand() & 0x3F);
        }
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetReg(
            pstCiConnection,
            &stIspScRdma
            );
        if (ret) {
            return ret;
        }
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stScDumpResultCtrl.stDumpResultCtrl.u8Addr =
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.adr;
        stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx =
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.idx;
        stScDumpResultCtrl.u8IspIdx = stIspScRdma.u8IspIdx;
        ret = STFDRV_ISP_SC_SetDumpingResultControl(
            pstCiConnection,
            &stScDumpResultCtrl
            );
        if (ret) {
            return ret;
        }
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetReg()\n", stIspScRdma.u8IspIdx);
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("DumpBufAddr = 0x%08X\n",
            stIspScRdma.stScRdma.stScdCfgReg.DumpBufAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("AxiId = 0x%02X\n",
            stIspScRdma.stScRdma.stScdCfgReg.AxiId.field.axi_id
            );
        LOG_INFO("Frame cropping - (0x%08X) H_Start = 0x%04X(%d), "\
            "V_Start = 0x%04X(%d)\n",
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.value,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.hstart,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.hstart,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.vstart,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.vstart
            );
        LOG_INFO("Sub-Window - (0x%08X) Width = 0x%02X(%d), "\
            "Height = 0x%02X(%d), Awb PS B/a = 0x%02X, "\
            "AeAf Select = 0x%01X(%s), Awb Select = 0x%01X(%s)\n",
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.value,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.width,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.width,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.height,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.height,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.awb_ps_grb_ba,
#if defined(SUPPORT_VIC_1_0)
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel],
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel]
#elif defined(SUPPORT_VIC_2_0) //#if defined(SUPPORT_VIC_2_0)
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel1,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel1],
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel0,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel0]
#endif //#if defined(SUPPORT_VIC_1_0)
            );
        LOG_INFO("Decimation - (0x%08X) HPeriod = 0x%02X, "\
            "HKeep = 0x%02X, VPeriod = 0x%02X, VKeep = 0x%02X\n",
            stIspScRdma.stScRdma.stScDecgReg.ScDec.value,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.hperiod,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.hkeep,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.vperiod,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.vkeep
            );
        LOG_INFO("AF Config - (0x%08X) af_es_hm = %d, af_es_sm = %d, "\
            "af_es_he = %d, af_es_ve = %d, af_es_vthr = 0x%02X, "\
            "af_es_hthr = 0x%03X\n",
            stIspScRdma.stScRdma.stScAfReg.ScAf.value,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_hm,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_sm,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_he,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_ve,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_vthr,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_hthr
            );
        LOG_INFO("AWB PS Config - (0x%08X) R_Lower = 0x%02X, "\
            "R_Upper = 0x%02X, G_Lower = 0x%02X, G_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_rl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_ru,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_gl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_gu
            );
        LOG_INFO("AWB PS Config - (0x%08X) B_Lower = 0x%02X, "\
            "B_Upper = 0x%02X, Y_Lower = 0x%02X, Y_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_bl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_bu,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_yl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_yu
            );
        LOG_INFO("AWB PS Config - (0x%08X) G/R_Lower = 0x%04X, "
            "G/R_Upper = 0x%04X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2.field.awb_ps_grl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2.field.awb_ps_gru
            );
        LOG_INFO("AWB PS Config - (0x%08X) G/B_Lower = 0x%04X, "\
            "G/B_Upper = 0x%04X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg3.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg3.field.awb_ps_gbl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg3.field.awb_ps_gbu
            );
        LOG_INFO("AWB PS Config - (0x%08X) "\
            "(Gr/R + b/a * Gb/B)_Lower = 0x%04X, "\
            "(Gr/R + b/a * Gb/B)_Upper = 0x%04X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg4.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg4.field.awb_ps_grbl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg4.field.awb_ps_grbu
            );
        LOG_INFO("AWB WS Config - (0x%08X) R_Lower = 0x%02X, "\
            "R_Upper = 0x%02X, Gr_Lower = 0x%02X, Gr_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.value,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_rl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_ru,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_grl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_gru
            );
        LOG_INFO("AWB WS Config - (0x%08X) Gb_Lower = 0x%02X, "\
            "Gb_Upper = 0x%02X, B_Lower = 0x%02X, B_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.value,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_gbl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_gbu,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_bl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_bu
            );
        u32Cnt = 13;
        pScAwbWsCwCfg0 = &stIspScRdma.stScRdma.stScWtReg.ScAwbWsCw0Cfg0;
        pScAwbWsCwCfg1 = &stIspScRdma.stScRdma.stScWtReg.ScAwbWsCw0Cfg1;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("Weighting Table[%02d] = %01X, %01X, %01X, %01X, %01X, "\
                "%01X, %01X, %01X, %01X, %01X, %01X, %01X, %01X\n",
                u32Idx,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_0,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_1,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_2,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_3,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_4,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_5,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_6,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_7,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_8,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_9,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_10,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_11,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_12
                );
        }
#if 0
        LOG_INFO("Intensity curve weighting value = %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, "\
            "%02X, %02X\n",
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_0,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_1,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_2,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_3,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_4,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_5,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_6,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_7,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_8,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_9,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_10,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_11,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_12,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_13,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_14,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_15
            );
#else
        LOG_INFO("Intensity curve weighting value = %2X, %2X, %2X, %2X, "\
            "%2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X\n",
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_0,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_1,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_2,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_3,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_4,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_5,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_6,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_7,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_8,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_9,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_10,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_11,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_12,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_13,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_14,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_15
            );
#endif
        LOG_INFO("Intensity curve slope value     = %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, "\
            "%02X, %02X\n",
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_0,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_1,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_2,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_3,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_4,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_5,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_6,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_7,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_8,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_9,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_10,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_11,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_12,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_13,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_14,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_15
            );
#if defined(ST_SC_REG_INCLUDE_RSLT)
        LOG_INFO("SC result - (0x%08X) Address_Offset = 0x%02X(%02d), "\
            "ItemIdx = 0x%02X(%s)\n",
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.value,
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.adr,
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.adr,
            stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.idx,
            g_szScItem[stIspScRdma.stScRdma.stScRsltReg.ScResultAddr.field.idx]
            );
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
#if defined(ST_SC_REG_INCLUDE_RSLT)
    u32Length = ST_SC_REG_LEN - 1;
#else
    u32Length = ST_SC_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
    for (i = 0; i < u32Length; i++) {
        switch (i) {
            case 0 ... 1:
                u32Buffer[i * 2] = ADDR_REG_SCD_CFG_0 + ((i - 0) * 4);
                break;

            case 2 ... 3:
                u32Buffer[i * 2] = ADDR_REG_SC_CFG_0 + ((i - 2) * 4);
                break;

            case 4:
                u32Buffer[i * 2] = ADDR_REG_SC_DEC + ((i - 4) * 4);
                break;

            case 5:
                u32Buffer[i * 2] = ADDR_REG_SC_AF + ((i - 5) * 4);
                break;

            case 6 ... 10:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_PS_CFG_0 + ((i - 6) * 4);
                break;

            case 11 ... 12:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CFG_0 + ((i - 11) * 4);
                break;

            case 13 ... 38:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CW0_CFG_0
                    + ((i - 13) * 4);
                break;

            case 39 ... 44:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_IWV_CFG_0
                    + ((i - 39) * 4);
                break;
#if defined(ST_SC_REG_INCLUDE_RSLT)

            case 45:
                u32Buffer[i * 2] = ADDR_REG_SCR_ADR + ((i - 45) * 4);
                break;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        }
        u32Buffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_SC)
STF_RESULT Test_SC_SetCfgReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SCD_CFG_0;
    u32Length = 2;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_SCD_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SC_RDMA stIspScRdma;

        stIspScRdma.stScRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_1;
#else
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_0;
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.u32Param = ST_SCD_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaCfgCmd.RegAddr = ADDR_REG_SC_CFG_0;
        stIspScRdma.stScRdma.stRdmaCfgCmd.u32Param = ST_SC_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaDecCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaDecCmd.RegAddr = ADDR_REG_SC_DEC;
        stIspScRdma.stScRdma.stRdmaDecCmd.u32Param = ST_SC_DEC_REG_LEN;
        stIspScRdma.stScRdma.stRdmaAfCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaAfCmd.RegAddr = ADDR_REG_SC_AF;
        stIspScRdma.stScRdma.stRdmaAfCmd.u32Param = ST_SC_AF_REG_LEN;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.RegAddr = ADDR_REG_SC_AWB_PS_CFG_0;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.u32Param = ST_SC_PS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.RegAddr = ADDR_REG_SC_AWB_WS_CFG_0;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.u32Param = ST_SC_WS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWtCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWtCmd.RegAddr = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
        stIspScRdma.stScRdma.stRdmaWtCmd.u32Param = ST_SC_WT_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWcCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWcCmd.RegAddr = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
        stIspScRdma.stScRdma.stRdmaWcCmd.u32Param = ST_SC_WC_REG_LEN;
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaRsltCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.RegAddr = ADDR_REG_SCR_ADR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.u32Param = ST_SC_RSLT_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaEndCmd.Tag = DESP_END;

#if defined(V4L2_DRIVER)
#else
        stIspScRdma.stScRdma.stScdCfgReg.DumpBufAddr.value =
            (rand() & 0xFFFFFF80);
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stScdCfgReg.AxiId.value = ((rand() & 0xFF) << 24);
        stIspScRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetCfgReg(
            pstCiConnection,
            &stIspScRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetCfgReg()\n", stIspScRdma.u8IspIdx);
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("DumpBufAddr = 0x%08X\n",
            stIspScRdma.stScRdma.stScdCfgReg.DumpBufAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("AxiId = 0x%02X\n",
            stIspScRdma.stScRdma.stScdCfgReg.AxiId.field.axi_id
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SCD_CFG_0;
    u32Length = 2;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_SCD_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetCropReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_CFG_0;
    u32Length = 2;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_SC_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SC_RDMA stIspScRdma;

        stIspScRdma.stScRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_1;
#else
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_0;
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.u32Param = ST_SCD_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaCfgCmd.RegAddr = ADDR_REG_SC_CFG_0;
        stIspScRdma.stScRdma.stRdmaCfgCmd.u32Param = ST_SC_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaDecCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaDecCmd.RegAddr = ADDR_REG_SC_DEC;
        stIspScRdma.stScRdma.stRdmaDecCmd.u32Param = ST_SC_DEC_REG_LEN;
        stIspScRdma.stScRdma.stRdmaAfCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaAfCmd.RegAddr = ADDR_REG_SC_AF;
        stIspScRdma.stScRdma.stRdmaAfCmd.u32Param = ST_SC_AF_REG_LEN;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.RegAddr = ADDR_REG_SC_AWB_PS_CFG_0;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.u32Param = ST_SC_PS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.RegAddr = ADDR_REG_SC_AWB_WS_CFG_0;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.u32Param = ST_SC_WS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWtCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWtCmd.RegAddr = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
        stIspScRdma.stScRdma.stRdmaWtCmd.u32Param = ST_SC_WT_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWcCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWcCmd.RegAddr = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
        stIspScRdma.stScRdma.stRdmaWcCmd.u32Param = ST_SC_WC_REG_LEN;
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaRsltCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.RegAddr = ADDR_REG_SCR_ADR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.u32Param = ST_SC_RSLT_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspScRdma.stScRdma.stScCfgReg.ScCfg0.value = (rand() & 0xFFFF)
            | ((rand() & 0xFFFF) << 16);
        stIspScRdma.stScRdma.stScCfgReg.ScCfg1.value = (rand() & 0x1F)
            | ((rand() & 0x1F) << 8)
            | ((rand() & 0xFF) << 16)
            | ((rand() & 0x3) << 28)
            | ((rand() & 0x3) << 30);
        stIspScRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetCropReg(
            pstCiConnection,
            &stIspScRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetCropReg()\n", stIspScRdma.u8IspIdx);
        LOG_INFO("Frame cropping - (0x%08X) H_Start = 0x%04X(%d), "\
            "V_Start = 0x%04X(%d)\n",
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.value,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.hstart,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.hstart,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.vstart,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg0.field.vstart
            );
        LOG_INFO("Sub-Window - (0x%08X) Width = 0x%02X(%d), "\
            "Height = 0x%02X(%d), Awb PS B/a = 0x%02X, "\
            "AeAf Select = 0x%01X(%s), Awb Select = 0x%01X(%s)\n",
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.value,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.width,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.width,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.height,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.height,
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.awb_ps_grb_ba,
#if defined(SUPPORT_VIC_1_0)
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel],
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel]
#elif defined(SUPPORT_VIC_2_0) //#if defined(SUPPORT_VIC_2_0)
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel1,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel1],
            stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel0,
            g_szScInSel[stIspScRdma.stScRdma.stScCfgReg.ScCfg1.field.sel0]
#endif //#if defined(SUPPORT_VIC_1_0)
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_CFG_0;
    u32Length = 2;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_SC_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetDecReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_DEC;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SC_RDMA stIspScRdma;

        stIspScRdma.stScRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_1;
#else
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_0;
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.u32Param = ST_SCD_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaCfgCmd.RegAddr = ADDR_REG_SC_CFG_0;
        stIspScRdma.stScRdma.stRdmaCfgCmd.u32Param = ST_SC_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaDecCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaDecCmd.RegAddr = ADDR_REG_SC_DEC;
        stIspScRdma.stScRdma.stRdmaDecCmd.u32Param = ST_SC_DEC_REG_LEN;
        stIspScRdma.stScRdma.stRdmaAfCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaAfCmd.RegAddr = ADDR_REG_SC_AF;
        stIspScRdma.stScRdma.stRdmaAfCmd.u32Param = ST_SC_AF_REG_LEN;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.RegAddr = ADDR_REG_SC_AWB_PS_CFG_0;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.u32Param = ST_SC_PS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.RegAddr = ADDR_REG_SC_AWB_WS_CFG_0;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.u32Param = ST_SC_WS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWtCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWtCmd.RegAddr = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
        stIspScRdma.stScRdma.stRdmaWtCmd.u32Param = ST_SC_WT_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWcCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWcCmd.RegAddr = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
        stIspScRdma.stScRdma.stRdmaWcCmd.u32Param = ST_SC_WC_REG_LEN;
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaRsltCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.RegAddr = ADDR_REG_SCR_ADR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.u32Param = ST_SC_RSLT_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspScRdma.stScRdma.stScDecgReg.ScDec.value = (rand() & 0x1F)
            | ((rand() & 0x1F) << 8)
            | ((rand() & 0x1F) << 16)
            | ((rand() & 0x1F) << 24);
        stIspScRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetDecReg(
            pstCiConnection,
            &stIspScRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetDecReg()\n", stIspScRdma.u8IspIdx);
        LOG_INFO("Decimation - (0x%08X) HPeriod = 0x%02X, "\
            "HKeep = 0x%02X, VPeriod = 0x%02X, VKeep = 0x%02X\n",
            stIspScRdma.stScRdma.stScDecgReg.ScDec.value,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.hperiod,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.hkeep,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.vperiod,
            stIspScRdma.stScRdma.stScDecgReg.ScDec.field.vkeep
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_DEC;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAeAfReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[1 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 1;
    for (i = 0; i < u32Length; i++) {
        switch (i) {
            case 0:
                u32Buffer[i * 2] = ADDR_REG_SC_AF + ((i - 0) * 4);
                break;
        }
        u32Buffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SC_RDMA stIspScRdma;

        stIspScRdma.stScRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_1;
#else
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_0;
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.u32Param = ST_SCD_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaCfgCmd.RegAddr = ADDR_REG_SC_CFG_0;
        stIspScRdma.stScRdma.stRdmaCfgCmd.u32Param = ST_SC_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaDecCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaDecCmd.RegAddr = ADDR_REG_SC_DEC;
        stIspScRdma.stScRdma.stRdmaDecCmd.u32Param = ST_SC_DEC_REG_LEN;
        stIspScRdma.stScRdma.stRdmaAfCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaAfCmd.RegAddr = ADDR_REG_SC_AF;
        stIspScRdma.stScRdma.stRdmaAfCmd.u32Param = ST_SC_AF_REG_LEN;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.RegAddr = ADDR_REG_SC_AWB_PS_CFG_0;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.u32Param = ST_SC_PS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.RegAddr = ADDR_REG_SC_AWB_WS_CFG_0;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.u32Param = ST_SC_WS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWtCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWtCmd.RegAddr = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
        stIspScRdma.stScRdma.stRdmaWtCmd.u32Param = ST_SC_WT_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWcCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWcCmd.RegAddr = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
        stIspScRdma.stScRdma.stRdmaWcCmd.u32Param = ST_SC_WC_REG_LEN;
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaRsltCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.RegAddr = ADDR_REG_SCR_ADR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.u32Param = ST_SC_RSLT_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspScRdma.stScRdma.stScAfReg.ScAf.value = (rand() & 0x1)
            | ((rand() & 0x1) << 1)
            | ((rand() & 0x1) << 2)
            | ((rand() & 0x1) << 3)
            | ((rand() & 0xFF) << 8)
            | ((rand() & 0x1FF) << 16);
        stIspScRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAeAfReg(
            pstCiConnection,
            &stIspScRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAeAfReg()\n", stIspScRdma.u8IspIdx);
        LOG_INFO("AF Config - (0x%08X) af_es_hm = %d, af_es_sm = %d, "\
            "af_es_he = %d, af_es_ve = %d, af_es_vthr = 0x%02X, "\
            "af_es_hthr = 0x%03X\n",
            stIspScRdma.stScRdma.stScAfReg.ScAf.value,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_hm,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_sm,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_he,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_ve,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_vthr,
            stIspScRdma.stScRdma.stScAfReg.ScAf.field.af_es_hthr
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 1;
    for (i = 0; i < u32Length; i++) {
        switch (i) {
            case 0:
                u32Buffer[i * 2] = ADDR_REG_SC_AF + ((i - 0) * 4);
                break;
        }
        u32Buffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[39 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 39;
    for (i = 0; i < u32Length; i++) {
        switch (i) {
            case 0 ... 4:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_PS_CFG_0 + ((i - 0) * 4);
                break;

            case 5 ... 6:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CFG_0 + ((i - 5) * 4);
                break;

            case 7 ... 32:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CW0_CFG_0 + ((i - 7) * 4);
                break;

            case 33 ... 38:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_IWV_CFG_0
                    + ((i - 33) * 4);
                break;
        }
        u32Buffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U16 *pu16Temp = NULL;
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SC_RDMA stIspScRdma;
        REG_SC_AWB_WS_CW0_CFG_0 *pScAwbWsCwCfg0 = NULL;
        REG_SC_AWB_WS_CW0_CFG_1 *pScAwbWsCwCfg1 = NULL;

        stIspScRdma.stScRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_1;
#else
        stIspScRdma.stScRdma.stRdmaStartCmd.RegAddr = ADDR_REG_SCD_CFG_0;
#endif //#if defined(V4L2_DRIVER)
        stIspScRdma.stScRdma.stRdmaStartCmd.u32Param = ST_SCD_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaCfgCmd.RegAddr = ADDR_REG_SC_CFG_0;
        stIspScRdma.stScRdma.stRdmaCfgCmd.u32Param = ST_SC_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaDecCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaDecCmd.RegAddr = ADDR_REG_SC_DEC;
        stIspScRdma.stScRdma.stRdmaDecCmd.u32Param = ST_SC_DEC_REG_LEN;
        stIspScRdma.stScRdma.stRdmaAfCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaAfCmd.RegAddr = ADDR_REG_SC_AF;
        stIspScRdma.stScRdma.stRdmaAfCmd.u32Param = ST_SC_AF_REG_LEN;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.RegAddr = ADDR_REG_SC_AWB_PS_CFG_0;
        stIspScRdma.stScRdma.stRdmaPsCfgCmd.u32Param = ST_SC_PS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.RegAddr = ADDR_REG_SC_AWB_WS_CFG_0;
        stIspScRdma.stScRdma.stRdmaWsCfgCmd.u32Param = ST_SC_WS_CFG_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWtCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWtCmd.RegAddr = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
        stIspScRdma.stScRdma.stRdmaWtCmd.u32Param = ST_SC_WT_REG_LEN;
        stIspScRdma.stScRdma.stRdmaWcCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaWcCmd.RegAddr = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
        stIspScRdma.stScRdma.stRdmaWcCmd.u32Param = ST_SC_WC_REG_LEN;
#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaRsltCmd.Tag = MREG_WR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.RegAddr = ADDR_REG_SCR_ADR;
        stIspScRdma.stScRdma.stRdmaRsltCmd.u32Param = ST_SC_RSLT_REG_LEN;
#endif //#if defined(ST_SC_REG_INCLUDE_RSLT)
        stIspScRdma.stScRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 8;
        pu8Temp = (STF_U8 *)&stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0xFF);
        }
        u32Cnt = 6;
        pu16Temp = (STF_U16 *)&stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0xFFFF);
        }
        u32Cnt = 8;
        pu8Temp = (STF_U8 *)&stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0xFF);
        }
        u32Cnt = 13;
        pu32Temp = (STF_U32 *)&stIspScRdma.stScRdma.stScWtReg.ScAwbWsCw0Cfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx * 2] = (rand() & 0xF)
                | ((rand() & 0xF) << 4)
                | ((rand() & 0xF) << 8)
                | ((rand() & 0xF) << 12)
                | ((rand() & 0xF) << 16)
                | ((rand() & 0xF) << 20)
                | ((rand() & 0xF) << 24)
                | ((rand() & 0xF) << 28);
            pu32Temp[u32Idx * 2 + 1] = (rand() & 0xF)
                | ((rand() & 0xF) << 4)
                | ((rand() & 0xF) << 8)
                | ((rand() & 0xF) << 12)
                | ((rand() & 0xF) << 16);
        }
        u32Cnt = 2;
        pu32Temp = (STF_U32 *)&stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0xF)
                | ((rand() & 0xF) << 4)
                | ((rand() & 0xF) << 8)
                | ((rand() & 0xF) << 12)
                | ((rand() & 0xF) << 16)
                | ((rand() & 0xF) << 20)
                | ((rand() & 0xF) << 24)
                | ((rand() & 0xF) << 28);
        }
        u32Cnt = 16;
        pu8Temp = (STF_U8 *)&stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0xFF);
        }
        stIspScRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbReg(
            pstCiConnection,
            &stIspScRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbReg()\n", stIspScRdma.u8IspIdx);
        LOG_INFO("AWB PS Config - (0x%08X) R_Lower = 0x%02X, "\
            "R_Upper = 0x%02X, G_Lower = 0x%02X, G_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_rl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_ru,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_gl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg0.field.awb_ps_gu
            );
        LOG_INFO("AWB PS Config - (0x%08X) B_Lower = 0x%02X, "\
            "B_Upper = 0x%02X, Y_Lower = 0x%02X, Y_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_bl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_bu,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_yl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg1.field.awb_ps_yu
            );
        LOG_INFO("AWB PS Config - (0x%08X) G/R_Lower = 0x%04X, "
            "G/R_Upper = 0x%04X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2.field.awb_ps_grl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg2.field.awb_ps_gru
            );
        LOG_INFO("AWB PS Config - (0x%08X) G/B_Lower = 0x%04X, "\
            "G/B_Upper = 0x%04X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg3.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg3.field.awb_ps_gbl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg3.field.awb_ps_gbu
            );
        LOG_INFO("AWB PS Config - (0x%08X) "\
            "(Gr/R + b/a * Gb/B)_Lower = 0x%04X, "\
            "(Gr/R + b/a * Gb/B)_Upper = 0x%04X\n",
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg4.value,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg4.field.awb_ps_grbl,
            stIspScRdma.stScRdma.stScPsCfgReg.ScAwbPsCfg4.field.awb_ps_grbu
            );
        LOG_INFO("AWB WS Config - (0x%08X) R_Lower = 0x%02X, "\
            "R_Upper = 0x%02X, Gr_Lower = 0x%02X, Gr_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.value,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_rl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_ru,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_grl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg0.field.awb_ws_gru
            );
        LOG_INFO("AWB WS Config - (0x%08X) Gb_Lower = 0x%02X, "\
            "Gb_Upper = 0x%02X, B_Lower = 0x%02X, B_Upper = 0x%02X\n",
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.value,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_gbl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_gbu,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_bl,
            stIspScRdma.stScRdma.stScWsCfgReg.ScAwbWsCfg1.field.awb_ws_bu
            );
        u32Cnt = 13;
        pScAwbWsCwCfg0 = &stIspScRdma.stScRdma.stScWtReg.ScAwbWsCw0Cfg0;
        pScAwbWsCwCfg1 = &stIspScRdma.stScRdma.stScWtReg.ScAwbWsCw0Cfg1;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("Weighting Table[%02d] = %01X, %01X, %01X, %01X, %01X, "\
                "%01X, %01X, %01X, %01X, %01X, %01X, %01X, %01X\n",
                u32Idx,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_0,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_1,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_2,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_3,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_4,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_5,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_6,
                pScAwbWsCwCfg0[u32Idx * 2].field.awb_ws_cw_w_0_7,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_8,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_9,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_10,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_11,
                pScAwbWsCwCfg1[u32Idx * 2].field.awb_ws_cw_w_0_12
                );
        }
#if 0
        LOG_INFO("Intensity curve weighting value = %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, "\
            "%02X, %02X\n",
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_0,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_1,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_2,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_3,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_4,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_5,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_6,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_7,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_8,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_9,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_10,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_11,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_12,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_13,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_14,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_15
            );
#else
        LOG_INFO("Intensity curve weighting value = %2X, %2X, %2X, %2X, "\
            "%2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X\n",
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_0,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_1,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_2,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_3,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_4,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_5,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_6,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg0.field.awb_ws_iw_v_7,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_8,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_9,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_10,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_11,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_12,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_13,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_14,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwvCfg1.field.awb_ws_iw_v_15
            );
#endif
        LOG_INFO("Intensity curve slope value     = %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, "\
            "%02X, %02X\n",
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_0,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_1,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_2,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg0.field.awb_ws_iw_s_3,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_4,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_5,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_6,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg1.field.awb_ws_iw_s_7,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_8,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_9,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_10,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg2.field.awb_ws_iw_s_11,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_12,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_13,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_14,
            stIspScRdma.stScRdma.stScWcReg.ScAwbWsIwsCfg3.field.awb_ws_iw_s_15
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 39;
    for (i = 0; i < u32Length; i++) {
        switch (i) {
            case 0 ... 4:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_PS_CFG_0 + ((i - 0) * 4);
                break;

            case 5 ... 6:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CFG_0 + ((i - 5) * 4);
                break;

            case 7 ... 32:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_CW0_CFG_0 + ((i - 7) * 4);
                break;

            case 33 ... 38:
                u32Buffer[i * 2] = ADDR_REG_SC_AWB_WS_IWV_CFG_0
                    + ((i - 33) * 4);
                break;
        }
        u32Buffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT Test_SC_SetDumpingAddress(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SCD_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        STF_U32 u32Address = (rand() & 0xFFFFFF80);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetDumpingAddress(
            pstCiConnection,
            u8IspIdx,
            u32Address
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetDumpingAddress()\n", u8IspIdx);
        LOG_INFO("DumpBufAddr = 0x%08X\n",
            u32Address
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SCD_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAxiId(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SCD_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        STF_U8 u8AxiId = (rand() & 0xFF);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAxiId(
            pstCiConnection,
            u8IspIdx,
            u8AxiId
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAxiId()\n", u8IspIdx);
        LOG_INFO("AxiId = 0x%02X\n",
            u8AxiId
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SCD_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAeAfFrameCropping(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_FRAME_CROPPING stScFrameCropping;

        stScFrameCropping.stStartPoint.u16X = (rand() & 0xFFFF);
        stScFrameCropping.stStartPoint.u16Y = (rand() & 0xFFFF);
        stScFrameCropping.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAeAfFrameCropping(
            pstCiConnection,
            &stScFrameCropping
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAeAfFrameCropping()\n",
            stScFrameCropping.u8IspIdx);
        LOG_INFO("Frame cropping - H_Start = 0x%04X(%d), "\
            "V_Start = 0x%04X(%d)\n",
            stScFrameCropping.stStartPoint.u16X,
            stScFrameCropping.stStartPoint.u16X,
            stScFrameCropping.stStartPoint.u16Y,
            stScFrameCropping.stStartPoint.u16Y
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAeAfConfig(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_AE_AF_CFG_PARAM stScAeAfCfg;

        stScAeAfCfg.stAeAfCfg.u8SubWinWidth = (rand() & 0x1F);
        stScAeAfCfg.stAeAfCfg.u8SubWinHeight = (rand() & 0x1F);
        stScAeAfCfg.stAeAfCfg.enSelect = (rand() & 0x3);
        stScAeAfCfg.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAeAfConfig(
            pstCiConnection,
            &stScAeAfCfg
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_etAeAfConfig()\n",
            stScAeAfCfg.u8IspIdx);
        LOG_INFO("Sub-Window - Width = 0x%02X(%d), Height = 0x%02X(%d), "\
            "Select = 0x%01X(%s)\n",
            stScAeAfCfg.stAeAfCfg.u8SubWinWidth,
            stScAeAfCfg.stAeAfCfg.u8SubWinWidth,
            stScAeAfCfg.stAeAfCfg.u8SubWinHeight,
            stScAeAfCfg.stAeAfCfg.u8SubWinHeight,
            stScAeAfCfg.stAeAfCfg.enSelect,
            g_szScInSel[stScAeAfCfg.stAeAfCfg.enSelect]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAeAfDecimation(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_DEC;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_DECIMATION stScDecimation;

        stScDecimation.stDec.u8HPeriod = (rand() & 0x1F);
        stScDecimation.stDec.u8HKeep = (rand() & 0x1F);
        stScDecimation.stDec.u8VPeriod = (rand() & 0x1F);
        stScDecimation.stDec.u8VKeep = (rand() & 0x1F);
        stScDecimation.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAeAfDecimation(
            pstCiConnection,
            &stScDecimation
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAeAfDecimation()\n",
            stScDecimation.u8IspIdx);
        LOG_INFO("Decimation - HPeriod = 0x%02X, HKeep = 0x%02X, "\
            "VPeriod = 0x%02X, VKeep = 0x%02X\n",
            stScDecimation.stDec.u8HPeriod,
            stScDecimation.stDec.u8HKeep,
            stScDecimation.stDec.u8VPeriod,
            stScDecimation.stDec.u8VKeep
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_DEC;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAfConfig(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AF;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_AF_CFG_PARAM stScAfCfg;

        stScAfCfg.stAfCfg.bHorzMode = (rand() & 0x1);
        stScAfCfg.stAfCfg.bSumMode = (rand() & 0x1);
        stScAfCfg.stAfCfg.bHorzEnable = (rand() & 0x1);
        stScAfCfg.stAfCfg.bVertEnable = (rand() & 0x1);
        stScAfCfg.stAfCfg.u16VertThres = (rand() & 0xFF);
        stScAfCfg.stAfCfg.u16HorzThres = (rand() & 0x1FF);
        stScAfCfg.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAfConfig(
            pstCiConnection,
            &stScAfCfg
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAfConfig()\n", stScAfCfg.u8IspIdx);
        LOG_INFO("AF Config - af_es_hm = %d, af_es_sm = %d, "\
            "af_es_he = %d, af_es_ve = %d, af_es_vthr = 0x%02X, "
            "af_es_hthr = 0x%03X\n",
            stScAfCfg.stAfCfg.bHorzMode,
            stScAfCfg.stAfCfg.bSumMode,
            stScAfCfg.stAfCfg.bHorzEnable,
            stScAfCfg.stAfCfg.bVertEnable,
            stScAfCfg.stAfCfg.u16VertThres,
            stScAfCfg.stAfCfg.u16HorzThres
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AF;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbFrameCropping(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_FRAME_CROPPING stScFrameCropping;

        stScFrameCropping.stStartPoint.u16X = (rand() & 0xFFFF);
        stScFrameCropping.stStartPoint.u16Y = (rand() & 0xFFFF);
        stScFrameCropping.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbFrameCropping(
            pstCiConnection,
            &stScFrameCropping
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbFrameCropping()\n",
            stScFrameCropping.u8IspIdx);
        LOG_INFO("Frame cropping - H_Start = 0x%04X(%d), "\
            "V_Start = 0x%04X(%d)\n",
            stScFrameCropping.stStartPoint.u16X,
            stScFrameCropping.stStartPoint.u16X,
            stScFrameCropping.stStartPoint.u16Y,
            stScFrameCropping.stStartPoint.u16Y
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbConfig(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_AWB_CFG_PARAM stScAwbCfg;

        stScAwbCfg.stAwbCfg.u8SubWinWidth = (rand() & 0x1F);
        stScAwbCfg.stAwbCfg.u8SubWinHeight = (rand() & 0x1F);
        stScAwbCfg.stAwbCfg.u8AwbBaVal = (rand() & 0xFF);
        stScAwbCfg.stAwbCfg.enSelect = (rand() & 0x3);
        stScAwbCfg.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbConfig(
            pstCiConnection,
            &stScAwbCfg
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_etAwbConfig()\n",
            stScAwbCfg.u8IspIdx);
        LOG_INFO("Sub-Window - Width = 0x%02X(%d), Height = 0x%02X(%d), "\
            "Awb PS B/a = 0x%02X, Select = 0x%01X(%s)\n",
            stScAwbCfg.stAwbCfg.u8SubWinWidth,
            stScAwbCfg.stAwbCfg.u8SubWinWidth,
            stScAwbCfg.stAwbCfg.u8SubWinHeight,
            stScAwbCfg.stAwbCfg.u8SubWinHeight,
            stScAwbCfg.stAwbCfg.u8AwbBaVal,
            stScAwbCfg.stAwbCfg.enSelect,
            g_szScInSel[stScAwbCfg.stAwbCfg.enSelect]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbDecimation(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_DEC;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_DECIMATION stScDecimation;

        stScDecimation.stDec.u8HPeriod = (rand() & 0x1F);
        stScDecimation.stDec.u8HKeep = (rand() & 0x1F);
        stScDecimation.stDec.u8VPeriod = (rand() & 0x1F);
        stScDecimation.stDec.u8VKeep = (rand() & 0x1F);
        stScDecimation.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbDecimation(
            pstCiConnection,
            &stScDecimation
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbDecimation()\n",
            stScDecimation.u8IspIdx);
        LOG_INFO("Decimation - HPeriod = 0x%02X, HKeep = 0x%02X, "\
            "VPeriod = 0x%02X, VKeep = 0x%02X\n",
            stScDecimation.stDec.u8HPeriod,
            stScDecimation.stDec.u8HKeep,
            stScDecimation.stDec.u8VPeriod,
            stScDecimation.stDec.u8VKeep
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_DEC;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbPixelSumRangeR(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbPixelSumRangeR(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbPixelSumRangeR()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB PS Config - R_Lower = 0x%02X, R_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbPixelSumRangeG(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbPixelSumRangeG(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbPixelSumRangeG()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB PS Config - G_Lower = 0x%02X, G_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbPixelSumRangeB(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbPixelSumRangeB(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbPixelSumRangeB()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB PS Config - B_Lower = 0x%02X, B_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbPixelSumRangeY(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbPixelSumRangeY(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbPixelSumRangeY()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB PS Config - Y_Lower = 0x%02X, Y_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbPixelSumRangeGR(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_2;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFFFF);
        stScRange.stRange.u16Upper = (rand() & 0xFFFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbPixelSumRangeGR(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbPixelSumRangeGR()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB PS Config - G/R_Lower = 0x%04X, G/R_Upper = 0x%04X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_2;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbPixelSumRangeGB(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_3;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFFFF);
        stScRange.stRange.u16Upper = (rand() & 0xFFFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbPixelSumRangeGB(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbPixelSumRangeGB()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB PS Config - G/B_Lower = 0x%04X, G/B_Upper = 0x%04X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_3;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbPixelSumRangeGRB(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_4;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFFFF);
        stScRange.stRange.u16Upper = (rand() & 0xFFFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbPixelSumRangeGRB(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbPixelSumRangeGRB()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB PS Config - (Gr/R + b/a * Gb/B)_Lower = 0x%04X, "\
            "(Gr/R + b/a * Gb/B)_Upper = 0x%04X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_PS_CFG_4;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbWeightSumRangeR(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbWeightSumRangeR(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbWeightSumRangeR()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB WS Config - R_Lower = 0x%02X, R_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbWeightSumRangeGr(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbWeightSumRangeGr(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbWeightSumRangeGr()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB WS Config - Gr_Lower = 0x%02X, Gr_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbWeightSumRangeGb(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbWeightSumRangeGb(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbWeightSumRangeGb()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB WS Config - Gb_Lower = 0x%02X, Gb_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbWeightSumRangeB(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_RANGE stScRange;

        stScRange.stRange.u16Low = (rand() & 0xFF);
        stScRange.stRange.u16Upper = (rand() & 0xFF);
        stScRange.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbWeightSumRangeB(
            pstCiConnection,
            &stScRange
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbWeightSumRangeB()\n",
            stScRange.u8IspIdx);
        LOG_INFO("AWB WS Config - Gb_Lower = 0x%02X, Gb_Upper = 0x%02X\n",
            stScRange.stRange.u16Low,
            stScRange.stRange.u16Upper
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_WS_CFG_1;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbWeightTable(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[13 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
    u32Length = 13 * 2;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_SC_AWB_WS_CW0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        STF_U32 u32RowIdx;
        STF_U32 u32ColIdx;
        STF_U32 u32Cnt;
        ST_SC_AWB_WT_TBL_PARAM stScAwbWeightingTable;

        u32Cnt = 13;
        for (u32RowIdx = 0; u32RowIdx < u32Cnt; u32RowIdx++) {
            for (u32ColIdx = 0; u32ColIdx < 13; u32ColIdx++) {
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][u32ColIdx] =
                    (rand() & 0xF);
            }
        }
        stScAwbWeightingTable.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbWeightTable(
            pstCiConnection,
            &stScAwbWeightingTable
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbWeightTable()\n",
            stScAwbWeightingTable.u8IspIdx);
        u32Cnt = 13;
        for (u32RowIdx = 0; u32RowIdx < u32Cnt; u32RowIdx++) {
            LOG_INFO("Weighting Table[%02d] = %01X, %01X, %01X, %01X, "\
                "%01X, %01X, %01X, %01X, %01X, %01X, %01X, %01X, %01X\n",
                u32RowIdx,
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][0],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][1],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][2],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][3],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][4],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][5],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][6],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][7],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][8],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][9],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][10],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][11],
                stScAwbWeightingTable.stWeightingTable.au8WeightingTable[u32RowIdx][12]
                );
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_WS_CW0_CFG_0;
    u32Length = 13 * 2;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_SC_AWB_WS_CW0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

STF_RESULT Test_SC_SetAwbIntensityWeightCurve(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[6];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
    u32Length = 6;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_SC_AWB_WS_IWV_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_SC_AWB_INTSTY_WT_CRV_PARAM stScAwbIntensityWeightCurve;

        u32Cnt = 16;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[u32Idx] =
                (rand() & 0xF);
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[u32Idx] =
                (rand() & 0xFF);
        }
        stScAwbIntensityWeightCurve.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetAwbIntensityWeightCurve(
            pstCiConnection,
            &stScAwbIntensityWeightCurve
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetAwbIntensityWeightCurve()\n",
            stScAwbIntensityWeightCurve.u8IspIdx);
#if 0
        LOG_INFO("Intensity curve weighting value = %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X\n",
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[0],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[1],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[2],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[3],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[4],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[5],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[6],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[7],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[8],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[9],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[10],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[11],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[12],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[13],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[14],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[15]
            );
#else
        LOG_INFO("Intensity curve weighting value = %2X, %2X, %2X, %2X, "\
        "%2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X, %2X\n",
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[0],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[1],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[2],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[3],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[4],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[5],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[6],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[7],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[8],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[9],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[10],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[11],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[12],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[13],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[14],
            stScAwbIntensityWeightCurve.stIntensityWeightCurve.u8Y[15]
            );
#endif
        LOG_INFO("Intensity curve slope value     = %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X, "\
            "%02X, %02X, %02X, %02X\n",
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[0],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[1],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[2],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[3],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[4],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[5],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[6],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[7],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[8],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[9],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[10],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[11],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[12],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[13],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[14],
            (STF_U8)stScAwbIntensityWeightCurve.stIntensityWeightCurve.s8Slope[15]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SC_AWB_WS_IWV_CFG_0;
    u32Length = 6;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_SC_AWB_WS_IWV_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
//-----------------------------------------------------------------------------
STF_RESULT Test_SC_SetDumpingResultControl(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if 0
    u32Offset = ADDR_REG_SCR_ADR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
#endif
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_DUMP_RSLT_CTRL_PARAM stScDumpResultCtrl;

        do {
            stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx = (rand() & 0x1F);
        } while (EN_SC_ITEM_AE_HIST_Y
            < stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx);

        if (EN_SC_ITEM_AE_HIST_R
            > stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx) {
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr = (rand() & 0xFF);
        } else {
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr = (rand() & 0x3F);
        }
        stScDumpResultCtrl.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetDumpingResultControl(
            pstCiConnection,
            &stScDumpResultCtrl
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_SetDumpingResultControl()\n",
            stScDumpResultCtrl.u8IspIdx);
        LOG_INFO("SC result - Address_Offset = 0x%02X(%03d), "\
            "ItemIdx = 0x%02X(%s)\n",
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr,
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr,
            stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx,
            g_szScItem[stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx]
            );
        LOG_INFO("---------------------------------\n");
    }
#if 0
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SCR_ADR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
#endif
   //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_SC)
STF_RESULT Test_SC_GetResult(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if 0
    u32Offset = ADDR_REG_SCR_ADR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
#endif
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_SC_DUMP_RSLT_CTRL_PARAM stScDumpResultCtrl;
        STF_U32 u32Address;
        STF_U32 u32Result;

        do {
            stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx = (rand() & 0x1F);
        } while (EN_SC_ITEM_AE_HIST_Y
            < stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx);
        if (EN_SC_ITEM_AE_HIST_R
            > stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx) {
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr = (rand() & 0xFF);
        } else {
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr = (rand() & 0x3F);
        }
        stScDumpResultCtrl.u8IspIdx = 0;
        ret = STFDRV_ISP_SC_SetDumpingResultControl(
            pstCiConnection,
            &stScDumpResultCtrl
            );
        if (ret) {
            return ret;
        }
        u32Result = STFDRV_ISP_SC_GetResult(
            pstCiConnection,
            stScDumpResultCtrl.u8IspIdx
            );
        if (ret) {
            return ret;
        }
        if (EN_SC_ITEM_AE_HIST_R
            > stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx) {
            u32Address = stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx
                * SC_RESULT_ADDR_MAX
                + stScDumpResultCtrl.stDumpResultCtrl.u8Addr;
        } else {
            u32Address = (stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx
                    - EN_SC_ITEM_AE_HIST_R)
                * SC_RESULT_AE_HIST_ADDR_MAX
                + stScDumpResultCtrl.stDumpResultCtrl.u8Addr;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_GetResult()\n",
            stScDumpResultCtrl.u8IspIdx);
        LOG_INFO("SC result - Address_Offset = 0x%02X(%03d), "\
            "ItemIdx = 0x%02X(%s)\n",
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr,
            stScDumpResultCtrl.stDumpResultCtrl.u8Addr,
            stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx,
            g_szScItem[stScDumpResultCtrl.stDumpResultCtrl.u8ItemIdx]
            );
        LOG_INFO("SC result - Result[0x%08X] = 0x%08X = "\
            "(0x%08X * 4 = 0x%08X)\n",
            u32Address,
            u32Result,
            u32Address,
            (u32Address * 4)
            );
        LOG_INFO("---------------------------------\n");
    }
#if 0
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SCR_ADR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
#endif
   //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
STF_RESULT Test_SC_GetResults(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if 0
    u32Offset = ADDR_REG_SCR_ADR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
#endif
    {
        //STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_SC_RSLTS_PARAM *pstScResults = NULL;
        STF_U32 u32Address;
        STF_U32 u32AddressTemp;

        LOG_INFO("Enter Test_SC_GetResults() function\n");
        pstScResults = STF_CALLOC(1,
            (sizeof(ST_SC_RSLT_CTRL) + ((64 + 1) * sizeof(STF_U32))));
        do {
            pstScResults->stResults.stScResultCtrl.u8ItemIdx = (rand() & 0x1F);
        } while (EN_SC_ITEM_AE_HIST_Y
            < pstScResults->stResults.stScResultCtrl.u8ItemIdx);
#if 0
        if (EN_SC_ITEM_AE_HIST_R
            > pstScResults->stResults.stScResultCtrl.u8ItemIdx) {
            pstScResults->stResults.stScResultCtrl.u8Addr = (rand() & 0xFF);
        } else {
            pstScResults->stResults.stScResultCtrl.u8Addr = (rand() & 0x3F);
        }
#else
        pstScResults->stResults.stScResultCtrl.u8Addr = 0;
#endif
        pstScResults->stResults.stScResultCtrl.u16Count = 64;
        pstScResults->u8IspIdx = 0;
        ret = STFDRV_ISP_SC_GetResults(
            pstCiConnection,
            pstScResults
            );
        if (ret) {
            STF_FREE(pstScResults);
            return ret;
        }
        if (EN_SC_ITEM_AE_HIST_R
            > pstScResults->stResults.stScResultCtrl.u8ItemIdx) {
            u32Address = (pstScResults->stResults.stScResultCtrl.u8ItemIdx / 2
                * SC_RESULT_HOR_MAX * 8)
                + ((pstScResults->stResults.stScResultCtrl.u8ItemIdx % 2) * 4);
            u32Idx = pstScResults->stResults.stScResultCtrl.u8Addr
                / SC_RESULT_HOR_MAX;
            u32Cnt = pstScResults->stResults.stScResultCtrl.u8Addr
                % SC_RESULT_HOR_MAX;
            u32Address = u32Address + u32Idx * (SC_RESULT_ADDR_MAX * 4);
            u32AddressTemp = u32Cnt;
        } else if (EN_SC_ITEM_AE_HIST_B
            > pstScResults->stResults.stScResultCtrl.u8ItemIdx) {
            u32Address = (EN_SC_ITEM_AE_HIST_R * SC_RESULT_ADDR_MAX * 4)
                + ((pstScResults->stResults.stScResultCtrl.u8ItemIdx
                    - EN_SC_ITEM_AE_HIST_R) * 4);
            u32Address += (pstScResults->stResults.stScResultCtrl.u8Addr * 8);
        } else {
            u32Address = (EN_SC_ITEM_AE_HIST_R * SC_RESULT_ADDR_MAX * 4)
                + (2 * SC_RESULT_AE_HIST_ADDR_MAX * 4)
                + ((pstScResults->stResults.stScResultCtrl.u8ItemIdx
                    - EN_SC_ITEM_AE_HIST_B) * 4);
            u32Address += (pstScResults->stResults.stScResultCtrl.u8Addr * 8);
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SC_GetResults()\n",
            pstScResults->u8IspIdx);
        LOG_INFO("SC result - Address_Offset = 0x%02X(%03d), "\
            "ItemIdx = 0x%02X(%s)\n",
            pstScResults->stResults.stScResultCtrl.u8Addr,
            pstScResults->stResults.stScResultCtrl.u8Addr,
            pstScResults->stResults.stScResultCtrl.u8ItemIdx,
            g_szScItem[pstScResults->stResults.stScResultCtrl.u8ItemIdx]
            );
#if 1
        u32Cnt = 64;
        if (EN_SC_ITEM_AE_HIST_R
            > pstScResults->stResults.stScResultCtrl.u8ItemIdx) {
            for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
                LOG_INFO("Buffer[0x%02X(%03d)][0x%08X] = 0x%008X\n",
                    u32Idx,
                    u32Idx,
                    u32Address + u32AddressTemp * 8,
                    pstScResults->stResults.pu32Buf[u32Idx]
                    );
                u32AddressTemp++;
                if (SC_RESULT_HOR_MAX <= u32AddressTemp) {
                    u32AddressTemp = 0;
                    u32Address += (SC_RESULT_ADDR_MAX * 4);
                }

            }
        } else {
            for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
                LOG_INFO("Buffer[0x%02X(%03d)][0x%08X] = 0x%008X\n",
                    u32Idx,
                    u32Idx,
                    (u32Address + u32Idx * 8),
                    pstScResults->stResults.pu32Buf[u32Idx]
                    );
            }
        }
#endif
        STF_FREE(pstScResults);
        LOG_INFO("---------------------------------\n");
    }
#if 0
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SCR_ADR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
#endif
   //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Sc_Test(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    g_pstCiConnection = (CI_CONNECTION *)STFLIB_ISP_BASE_GetConnection(
        TEST_ISP_DEVICE
        );

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetReg(
            );
        LOG_INFO("Test SC_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_SC)

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetCfgReg(
            );
        LOG_INFO("Test SC_SetCfgReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetCropReg(
            );
        LOG_INFO("Test SC_SetCropReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetDecReg(
            );
        LOG_INFO("Test SC_SetDecReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAeAfReg(
            );
        LOG_INFO("Test SC_SetAeAfReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbReg(
            );
        LOG_INFO("Test SC_SetAwbReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetDumpingAddress(
            );
        LOG_INFO("Test SC_SetDumpingAddress command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAxiId(
            );
        LOG_INFO("Test SC_SetAxiId command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAeAfFrameCropping(
            );
        LOG_INFO("Test SC_SetAeAfFrameCropping command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAeAfConfig(
            );
        LOG_INFO("Test SC_SetAeAfConfig command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAeAfDecimation(
            );
        LOG_INFO("Test SC_SetAeAfDecimation command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAfConfig(
            );
        LOG_INFO("Test SC_SetAfConfig command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbFrameCropping(
            );
        LOG_INFO("Test SC_SetAwbFrameCropping command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbConfig(
            );
        LOG_INFO("Test SC_SetAwbConfig command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbDecimation(
            );
        LOG_INFO("Test SC_SetAwbDecimation command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbPixelSumRangeR(
            );
        LOG_INFO("Test SC_SetAwbPixelSumRangeR command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbPixelSumRangeG(
            );
        LOG_INFO("Test SC_SetAwbPixelSumRangeG command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbPixelSumRangeB(
            );
        LOG_INFO("Test SC_SetAwbPixelSumRangeB command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbPixelSumRangeY(
            );
        LOG_INFO("Test SC_SetAwbPixelSumRangeY command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbPixelSumRangeGR(
            );
        LOG_INFO("Test SC_SetAwbPixelSumRangeGR command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbPixelSumRangeGB(
            );
        LOG_INFO("Test SC_SetAwbPixelSumRangeGB command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbPixelSumRangeGRB(
            );
        LOG_INFO("Test SC_SetAwbPixelSumRangeGRB command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbWeightSumRangeR(
            );
        LOG_INFO("Test SC_SetAwbWeightSumRangeR command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbWeightSumRangeGr(
            );
        LOG_INFO("Test SC_SetAwbWeightSumRangeGr command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbWeightSumRangeGb(
            );
        LOG_INFO("Test SC_SetAwbWeightSumRangeGb command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbWeightSumRangeB(
            );
        LOG_INFO("Test SC_SetAwbWeightSumRangeB command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbWeightTable(
            );
        LOG_INFO("Test SC_SetAwbWeightTable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetAwbIntensityWeightCurve(
            );
        LOG_INFO("Test SC_SetAwbIntensityWeightCurve command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
    //=========================================================================

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SC_SetDumpingResultControl(
            );
        LOG_INFO("Test SC_SetDumpingResultControl command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#if defined(EXTEND_IOCTL_COMMAND_SC)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        //usleep(5000000);
        ret = Test_SC_GetResult(
            );
        LOG_INFO("Test SC_GetResult command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        //usleep(5000000);
        ret = Test_SC_GetResults(
            );
        LOG_INFO("Test SC_GetResults command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
