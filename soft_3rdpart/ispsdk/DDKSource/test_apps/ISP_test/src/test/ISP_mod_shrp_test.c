/**
  ******************************************************************************
  * @file  ISP_mod_shrp_test.c
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

#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"

#include "ISPC/stflib_isp_base.h"

#define LOG_TAG "Mod_Shrp_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_shrp_test.h"
#include "ISP_test.h"


/* ISPC SHRP driver test structure */


/* ISPC SHRP driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC SHRP driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_SHRP_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_SHRP_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ISHRP1_0;
    u32Length = ST_SHRP_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SHRP_RDMA stIspShrpRdma;
        STF_CHAR szMessage[1024] = "";
        STF_CHAR szTemp[128] = "";

        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISHRP1_0;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.u32Param = ST_SHRP_REG_LEN;
        stIspShrpRdma.stShrpRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = ST_SHRP_REG_LEN;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            switch (u32Idx) {
                case 0 ... 7:
                    pu32Temp[u32Idx] = ((rand() & 0xF) << 8)
                        | ((rand() & 0x03FF) << 16);
                    break;

                case 8 ... 14:
                    pu32Temp[u32Idx] = ((rand() & 0xF) << 8);
                    break;

                case 15 ... 17:
                    pu32Temp[u32Idx] = (rand() & 0x1FFFFF)
                        | ((rand() & 0x7F) << 24);
                    break;

                case 18 ... 20:
                    pu32Temp[u32Idx] = (rand() & 0x1FFFFF)
                        | ((rand() & 0x7F) << 24);
                    break;

                case 21:
                    pu32Temp[u32Idx] = (rand() & 0x3FFFFF)
                        | ((rand() & 0xF) << 24) | ((rand() & 0xF) << 28);
                    break;

                case 22:
                    pu32Temp[u32Idx] = (rand() & 0x03FF)
                        | ((rand() & 0x03FF) << 16);
                    break;

                case 23:
                    pu32Temp[u32Idx] = (rand() & 0x3FFFF)
                        | ((rand() & 0xFF) << 24);
                    break;

                case 24:
                    pu32Temp[u32Idx] = (rand() & 0x03FF)
                        | ((rand() & 0x03FF) << 16);
                    break;

                case 25:
                    pu32Temp[u32Idx] = (rand() & 0x3FFFF);
                    break;

                case 26:
                    pu32Temp[u32Idx] = (rand() & 0x1)
                        | ((rand() & 0xFFFF) << 8);
                    break;
            }
        }
        stIspShrpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SHRP_SetReg(
            pstCiConnection,
            &stIspShrpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SHRP_SetReg()\n", stIspShrpRdma.u8IspIdx);
        szMessage[0] = '\0';
        u32Cnt = 15;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg.W0_LD1;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if (0 == u32Idx) {
                sprintf(szTemp, "Weighting = 0x%01X(%d)",
                    ((pu32Temp[u32Idx] >> 8) & 0xF),
                    ((pu32Temp[u32Idx] >> 8) & 0xF)
                    );
            } else if ((7 == (u32Idx % 8)) || ((u32Cnt - 1) == u32Idx)) {
                sprintf(szTemp, ", 0x%01X(%d)\n",
                    ((pu32Temp[u32Idx] >> 8) & 0xF),
                    ((pu32Temp[u32Idx] >> 8) & 0xF)
                    );
            } else {
                sprintf(szTemp, ", 0x%01X(%d)",
                    ((pu32Temp[u32Idx] >> 8) & 0xF),
                    ((pu32Temp[u32Idx] >> 8) & 0xF)
                    );
            }
            strcat(szMessage, szTemp);
        }
        LOG_INFO("%s", szMessage);
        LOG_INFO("YLevel Diff = 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.W0_LD1.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W1_LD2.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W2_LD3.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W3_LD4.field.diff
            );
        LOG_INFO("YMean Diff = 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.W4_MD1.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W5_MD2.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W6_MD3.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W7_MD4.field.diff
            );
        LOG_INFO("YLevel Factor = 0x%02X, 0x%02X, 0x%02X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.LF1_LS1.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF2_LS2.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF3_LS3.field.f
            );
        LOG_INFO("YLevel Slope = 0x%06X, 0x%06X, 0x%06X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.LF1_LS1.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF2_LS2.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF3_LS3.field.s
            );
        LOG_INFO("YMean Factor = 0x%02X, 0x%02X, 0x%02X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.MF1_MS1.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF2_MS2.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF3_MS3.field.f
            );
        LOG_INFO("YMean Slope = 0x%06X, 0x%06X, 0x%06X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.MF1_MS1.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF2_MS2.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF3_MS3.field.s
            );
        LOG_INFO("WSum = 0x%06X, NegDirFactor = 0x%01X, "\
            "PosDirFactor = 0x%01X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.WSum_NDirF_PDirF.field.wsum,
            stIspShrpRdma.stShrpRdma.stShrpReg.WSum_NDirF_PDirF.field.ndirf,
            stIspShrpRdma.stShrpRdma.stShrpReg.WSum_NDirF_PDirF.field.pdirf
            );
        LOG_INFO("UV Smooth - Diff1 = 0x%04X, Diff2 = 0x%04X, "\
            "Slope = 0x%05X, Factor = 0x%02X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothDiff1_2.field.uvdiff1,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothDiff1_2.field.uvdiff2,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothSlope_Factor.field.uvslope,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothSlope_Factor.field.uvf
            );
        LOG_INFO("UV Color Kill - Diff1 = 0x%04X, Diff2 = 0x%04X, "\
            "Slope = 0x%05X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.UvCKDiff1_2.field.uvckdiff1,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvCKDiff1_2.field.uvckdiff2,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvCKSlope.field.uvckslope
            );
        LOG_INFO("Edge smooth - Threshold Enable = %s, Threshold = 0x%04X\n",
            ((stIspShrpRdma.stShrpRdma.stShrpReg.EdgeSmoothCfg.field.en)
                ? ("Enable") : ("Disable")),
            stIspShrpRdma.stShrpRdma.stShrpReg.EdgeSmoothCfg.field.th
            );
#if 1
        u32Cnt = ST_SHRP_REG_LEN;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("Sharpe Reg[%02d] = 0x%08X\n", u32Idx, pu32Temp[u32Idx]);
        }
#endif
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISHRP1_0;
    u32Length = ST_SHRP_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_SHRP)
//-----------------------------------------------------------------------------
STF_RESULT Test_SHRP_SetWeightAndYDifferent(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[SHRP_WT_Y_DIFF_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ISHRP1_0;
    u32Length = SHRP_WT_Y_DIFF_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SHRP_RDMA stIspShrpRdma;
        STF_CHAR szMessage[1024] = "";
        STF_CHAR szTemp[128] = "";

        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISHRP1_0;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.u32Param = ST_SHRP_REG_LEN;
        stIspShrpRdma.stShrpRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = ST_SHRP_REG_LEN;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            switch (u32Idx) {
                case 0 ... 7:
                    pu32Temp[u32Idx] = ((rand() & 0xF) << 8)
                        | ((rand() & 0x03FF) << 16);
                    break;

                case 8 ... 14:
                    pu32Temp[u32Idx] = ((rand() & 0xF) << 8);
                    break;
            }
        }
        stIspShrpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SHRP_SetWeightAndYDifferent(
            pstCiConnection,
            &stIspShrpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SHRP_SetWeightAndYDifferent()\n",
            stIspShrpRdma.u8IspIdx);
        szMessage[0] = '\0';
        u32Cnt = 15;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg.W0_LD1;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if (0 == u32Idx) {
                sprintf(szTemp, "Weighting = 0x%01X(%d)",
                    ((pu32Temp[u32Idx] >> 8) & 0xF),
                    ((pu32Temp[u32Idx] >> 8) & 0xF)
                    );
            } else if ((7 == (u32Idx % 8)) || ((u32Cnt - 1) == u32Idx)) {
                sprintf(szTemp, ", 0x%01X(%d)\n",
                    ((pu32Temp[u32Idx] >> 8) & 0xF),
                    ((pu32Temp[u32Idx] >> 8) & 0xF)
                    );
            } else {
                sprintf(szTemp, ", 0x%01X(%d)",
                    ((pu32Temp[u32Idx] >> 8) & 0xF),
                    ((pu32Temp[u32Idx] >> 8) & 0xF)
                    );
            }
            strcat(szMessage, szTemp);
        }
        LOG_INFO("%s", szMessage);
        LOG_INFO("YLevel Diff = 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.W0_LD1.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W1_LD2.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W2_LD3.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W3_LD4.field.diff
            );
        LOG_INFO("YMean Diff = 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.W4_MD1.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W5_MD2.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W6_MD3.field.diff,
            stIspShrpRdma.stShrpRdma.stShrpReg.W7_MD4.field.diff
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISHRP1_0;
    u32Length = SHRP_WT_Y_DIFF_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SHRP_SetFactorAndSlope(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[SHRP_FACTOR_SLOPE_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ISHRP2_0;
    u32Length = SHRP_FACTOR_SLOPE_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SHRP_RDMA stIspShrpRdma;

        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISHRP1_0;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.u32Param = ST_SHRP_REG_LEN;
        stIspShrpRdma.stShrpRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = SHRP_FACTOR_SLOPE_LEN;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg.LF1_LS1;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0x1FFFFF) | ((rand() & 0x7F) << 24);
        }
        stIspShrpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SHRP_SetFactorAndSlope(
            pstCiConnection,
            &stIspShrpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SHRP_SetFactorAndSlope()\n",
            stIspShrpRdma.u8IspIdx);
        LOG_INFO("YLevel Factor = 0x%02X, 0x%02X, 0x%02X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.LF1_LS1.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF2_LS2.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF3_LS3.field.f
            );
        LOG_INFO("YLevel Slope = 0x%06X, 0x%06X, 0x%06X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.LF1_LS1.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF2_LS2.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.LF3_LS3.field.s
            );
        LOG_INFO("YMean Factor = 0x%02X, 0x%02X, 0x%02X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.MF1_MS1.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF2_MS2.field.f,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF3_MS3.field.f
            );
        LOG_INFO("YMean Slope = 0x%06X, 0x%06X, 0x%06X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.MF1_MS1.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF2_MS2.field.s,
            stIspShrpRdma.stShrpRdma.stShrpReg.MF3_MS3.field.s
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISHRP2_0;
    u32Length = SHRP_FACTOR_SLOPE_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SHRP_SetWeightNormalization(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[SHRP_WT_N11N_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ISHRP3;
    u32Length = SHRP_WT_N11N_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SHRP_RDMA stIspShrpRdma;

        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISHRP1_0;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.u32Param = ST_SHRP_REG_LEN;
        stIspShrpRdma.stShrpRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = SHRP_FACTOR_SLOPE_LEN;
        pu32Temp =
            (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg.WSum_NDirF_PDirF;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0x3FFFFF)
                | ((rand() & 0xF) << 24) | ((rand() & 0xF) << 28);
        }
        stIspShrpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SHRP_SetWeightNormalization(
            pstCiConnection,
            &stIspShrpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SHRP_SetWeightNormalization()\n",
            stIspShrpRdma.u8IspIdx);
        LOG_INFO("WSum = 0x%06X, NegDirFactor = 0x%01X, "\
            "PosDirFactor = 0x%01X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.WSum_NDirF_PDirF.field.wsum,
            stIspShrpRdma.stShrpRdma.stShrpReg.WSum_NDirF_PDirF.field.ndirf,
            stIspShrpRdma.stShrpRdma.stShrpReg.WSum_NDirF_PDirF.field.pdirf
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISHRP3;
    u32Length = SHRP_WT_N11N_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SHRP_SetUvSmoothCurve(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[SHRP_UV_SMTH_CRV_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_IUVS1;
    u32Length = SHRP_UV_SMTH_CRV_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SHRP_RDMA stIspShrpRdma;

        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISHRP1_0;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.u32Param = ST_SHRP_REG_LEN;
        stIspShrpRdma.stShrpRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = SHRP_UV_SMTH_CRV_LEN;
        pu32Temp =
            (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothDiff1_2;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            switch (u32Idx) {
                case 0:
                    pu32Temp[u32Idx] = (rand() & 0x03FF)
                        | ((rand() & 0x03FF) << 16);
                    break;

                case 1:
                    pu32Temp[u32Idx] = (rand() & 0x3FFFF)
                        | ((rand() & 0xFF) << 24);
                    break;
            }
        }
        stIspShrpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SHRP_SetUvSmoothCurve(
            pstCiConnection,
            &stIspShrpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SHRP_SetUvSmoothCurve()\n",
            stIspShrpRdma.u8IspIdx);
        LOG_INFO("UV Smooth - Diff1 = 0x%04X, Diff2 = 0x%04X, "\
            "Slope = 0x%05X, Factor = 0x%02X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothDiff1_2.field.uvdiff1,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothDiff1_2.field.uvdiff2,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothSlope_Factor.field.uvslope,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvSmoothSlope_Factor.field.uvf
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IUVS1;
    u32Length = SHRP_UV_SMTH_CRV_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SHRP_SetUvColorKillCurve(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[SHRP_UV_COLR_KILL_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_IUVCKS1;
    u32Length = SHRP_UV_COLR_KILL_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SHRP_RDMA stIspShrpRdma;

        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISHRP1_0;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.u32Param = ST_SHRP_REG_LEN;
        stIspShrpRdma.stShrpRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = SHRP_UV_COLR_KILL_LEN;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg.UvCKDiff1_2;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            switch (u32Idx) {
                case 0:
                    pu32Temp[u32Idx] = (rand() & 0x03FF)
                        | ((rand() & 0x03FF) << 16);
                    break;

                case 1:
                    pu32Temp[u32Idx] = (rand() & 0x3FFFF);
                    break;
            }
        }
        stIspShrpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SHRP_SetUvColorKillCurve(
            pstCiConnection,
            &stIspShrpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SHRP_SetUvColorKillCurve()\n",
            stIspShrpRdma.u8IspIdx);
        LOG_INFO("UV Color Kill - Diff1 = 0x%04X, Diff2 = 0x%04X, "\
            "Slope = 0x%05X\n",
            stIspShrpRdma.stShrpRdma.stShrpReg.UvCKDiff1_2.field.uvckdiff1,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvCKDiff1_2.field.uvckdiff2,
            stIspShrpRdma.stShrpRdma.stShrpReg.UvCKSlope.field.uvckslope
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IUVCKS1;
    u32Length = SHRP_UV_COLR_KILL_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SHRP_SetEdgeParam(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[SHRP_EDGE_SMTH_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ISHRPET;
    u32Length = SHRP_EDGE_SMTH_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SHRP_RDMA stIspShrpRdma;

        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISHRP1_0;
        stIspShrpRdma.stShrpRdma.stRdmaStartCmd.u32Param = ST_SHRP_REG_LEN;
        stIspShrpRdma.stShrpRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = SHRP_EDGE_SMTH_LEN;
        pu32Temp = (STF_U32 *)&stIspShrpRdma.stShrpRdma.stShrpReg.EdgeSmoothCfg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            switch (u32Idx) {
                case 0:
                    pu32Temp[u32Idx] = (rand() & 0x1)
                        | ((rand() & 0xFFFF) << 8);
                    break;
            }
        }
        stIspShrpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SHRP_SetEdgeParam(
            pstCiConnection,
            &stIspShrpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SHRP_SetEdgeParam()\n",
            stIspShrpRdma.u8IspIdx);
        LOG_INFO("Edge smooth - Threshold Enable = %s, Threshold = 0x%04X\n",
            ((stIspShrpRdma.stShrpRdma.stShrpReg.EdgeSmoothCfg.field.en)
                ? ("Enable") : ("Disable")),
            stIspShrpRdma.stShrpRdma.stShrpReg.EdgeSmoothCfg.field.th
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISHRPET;
    u32Length = SHRP_EDGE_SMTH_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_SHRP)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Shrp_Test(
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
        ret = Test_SHRP_SetReg(
            );
        LOG_INFO("Test SHRP_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SHRP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_SHRP)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SHRP_SetWeightAndYDifferent(
            );
        LOG_INFO("Test SHRP_SetWeightAndYDifferent command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SHRP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SHRP_SetFactorAndSlope(
            );
        LOG_INFO("Test SHRP_SetFactorAndSlope command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SHRP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SHRP_SetWeightNormalization(
            );
        LOG_INFO("Test SHRP_SetWeightNormalization command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SHRP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SHRP_SetUvSmoothCurve(
            );
        LOG_INFO("Test SHRP_SetUvSmoothCurve command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SHRP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SHRP_SetUvColorKillCurve(
            );
        LOG_INFO("Test SHRP_SetUvColorKillCurve command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SHRP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SHRP_SetEdgeParam(
            );
        LOG_INFO("Test SHRP_SetEdgeParam command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SHRP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_SHRP)

    return ret;
}

//-----------------------------------------------------------------------------
