/**
  ******************************************************************************
  * @file  ISP_mod_awb_test.c
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

#define LOG_TAG "Mod_Awb_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_awb_test.h"
#include "ISP_test.h"


/* ISPC AWB driver test structure */


/* ISPC AWB driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC AWB driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_AWB_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[24 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 24;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i * 2] = ADDR_REG_AWB_X0_CFG_0 + (i * 4);
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
        ST_ISP_AWB_RDMA stIspAwbRdma;

        stIspAwbRdma.stAwbRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspAwbRdma.stAwbRdma.stRdmaStartCmd.RegAddr = ADDR_REG_AWB_X0_CFG_0;
        stIspAwbRdma.stAwbRdma.stRdmaStartCmd.u32Param = ST_AWB_REG_LEN;
        stIspAwbRdma.stAwbRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Length = 24;
        pu32Temp = (STF_U32 *)&stIspAwbRdma.stAwbRdma.stAwbReg;
        for (i = 0; i < u32Length; i++) {
            pu32Temp[i] = (rand() & 0x03FF) | ((rand() & 0x03FF) << 16);
        }
        stIspAwbRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_AWB_SetReg(
            pstCiConnection,
            &stIspAwbRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_AWB_SetReg()\n", stIspAwbRdma.u8IspIdx);
        LOG_INFO("stAwbReg.RIn0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.RIn0_1.value
            );
        LOG_INFO("stAwbReg.RIn2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.RIn2_3.value
            );
        LOG_INFO("stAwbReg.GrIn0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GrIn0_1.value
            );
        LOG_INFO("stAwbReg.GrIn2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GrIn2_3.value
            );
        LOG_INFO("stAwbReg.GbIn0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GbIn0_1.value
            );
        LOG_INFO("stAwbReg.GbIn2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GbIn2_3.value
            );
        LOG_INFO("stAwbReg.BIn0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.BIn0_1.value
            );
        LOG_INFO("stAwbReg.BIn2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.BIn2_3.value
            );
        LOG_INFO("---\n");
        LOG_INFO("stAwbReg.ROut0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.ROut0_1.value
            );
        LOG_INFO("stAwbReg.ROut2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.ROut2_3.value
            );
        LOG_INFO("stAwbReg.GrOut0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GrOut0_1.value
            );
        LOG_INFO("stAwbReg.GrOut2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GrOut2_3.value
            );
        LOG_INFO("stAwbReg.GbOut0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GbOut0_1.value
            );
        LOG_INFO("stAwbReg.GbOut2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GbOut2_3.value
            );
        LOG_INFO("stAwbReg.BOut0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.BOut0_1.value
            );
        LOG_INFO("stAwbReg.BOut2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.BOut2_3.value
            );
        LOG_INFO("---\n");
        LOG_INFO("stAwbReg.RSlope0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.RSlope0_1.value
            );
        LOG_INFO("stAwbReg.RSlope2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.RSlope2_3.value
            );
        LOG_INFO("stAwbReg.GrSlope0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GrSlope0_1.value
            );
        LOG_INFO("stAwbReg.GrSlope2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GrSlope2_3.value
            );
        LOG_INFO("stAwbReg.GbSlope0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GbSlope0_1.value
            );
        LOG_INFO("stAwbReg.GbSlope2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.GbSlope2_3.value
            );
        LOG_INFO("stAwbReg.BSlope0_1 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.BSlope0_1.value
            );
        LOG_INFO("stAwbReg.BSlope2_3 = 0x%08X\n",
            stIspAwbRdma.stAwbRdma.stAwbReg.BSlope2_3.value
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 24;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i * 2] = ADDR_REG_AWB_X0_CFG_0 + (i * 4);
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

#if defined(EXTEND_IOCTL_COMMAND_AWB)
//-----------------------------------------------------------------------------
STF_RESULT Test_AWB_SetChnCrv(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[6 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    {
        STF_U8 u8Chn = EN_AWB_CHN_B;

        u32Length = 6;
        for (i = 0; i < (u32Length / 3); i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_X0_CFG_0 + u8Chn * 2 * 4
                + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
            u32Buffer[4 + i * 2] = ADDR_REG_AWB_Y0_CFG_0 + u8Chn * 2 * 4
                + (i * 4);
            u32Buffer[4 + i * 2 + 1] = 0;
            u32Buffer[8 + i * 2] = ADDR_REG_AWB_S0_CFG_0 + u8Chn * 2 * 4
                + (i * 4);
            u32Buffer[8 + i * 2 + 1] = 0;
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
    }
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_AWB_CHN_CRV_PARAM stIspAwbChnCrv;

        stIspAwbChnCrv.stAwbChnCrv.u8Chn = EN_AWB_CHN_B;
        u32Cnt = 12;
        pu16Temp = (STF_U16 *)&stIspAwbChnCrv.stAwbChnCrv.au16Pi;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspAwbChnCrv.u8IspIdx = 0;
        ret = STFDRV_ISP_AWB_SetChnCrv(
            pstCiConnection,
            &stIspAwbChnCrv
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_AWB_SetChnCrv()\n",
            stIspAwbChnCrv.u8IspIdx);
        LOG_INFO("stAwbChnCrv.u8Chn = %d\n", stIspAwbChnCrv.stAwbChnCrv.u8Chn);
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbChnCrv.au16Pi[%d] = 0x%04X\n",
                u32Idx, stIspAwbChnCrv.stAwbChnCrv.au16Pi[u32Idx]);
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbChnCrv.au16Po[%d] = 0x%04X\n",
                u32Idx, stIspAwbChnCrv.stAwbChnCrv.au16Po[u32Idx]);
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbChnCrv.au16Ps[%d] = 0x%04X\n",
                u32Idx, stIspAwbChnCrv.stAwbChnCrv.au16Ps[u32Idx]);
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_U8 u8Chn = EN_AWB_CHN_B;

        u32Length = 6;
        for (i = 0; i < (u32Length / 3); i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_X0_CFG_0 + u8Chn * 2 * 4
                + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
            u32Buffer[4 + i * 2] = ADDR_REG_AWB_Y0_CFG_0 + u8Chn * 2 * 4
                + (i * 4);
            u32Buffer[4 + i * 2 + 1] = 0;
            u32Buffer[8 + i * 2] = ADDR_REG_AWB_S0_CFG_0 + u8Chn * 2 * 4
                + (i * 4);
            u32Buffer[8 + i * 2 + 1] = 0;
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
    }
   //=========================================================================

    return ret;
}

STF_RESULT Test_AWB_SetPi(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[8 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    {
        u32Length = 8;
        for (i = 0; i < u32Length; i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_X0_CFG_0 + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
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
    }
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_AWB_INPUT_PARAM stIspAwbInput;

        u32Cnt = 16;
        pu16Temp = (STF_U16 *)&stIspAwbInput.stAwbInput.au16RPi;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspAwbInput.u8IspIdx = 0;
        ret = STFDRV_ISP_AWB_SetPi(
            pstCiConnection,
            &stIspAwbInput
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_AWB_SetPi()\n", stIspAwbInput.u8IspIdx);
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbInput.au16RPi[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbInput.stAwbInput.au16RPi[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbInput.au16GrPi[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbInput.stAwbInput.au16GrPi[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbInput.au16GbPi[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbInput.stAwbInput.au16GbPi[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbInput.au16BPi[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbInput.stAwbInput.au16BPi[u32Idx]
                );
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        u32Length = 8;
        for (i = 0; i < u32Length; i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_X0_CFG_0 + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
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
    }
   //=========================================================================

    return ret;
}

STF_RESULT Test_AWB_SetPo(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[8 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    {
        u32Length = 8;
        for (i = 0; i < u32Length; i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_Y0_CFG_0 + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
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
    }
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_AWB_OUTPUT_PARAM stIspAwbOutput;

        u32Cnt = 16;
        pu16Temp = (STF_U16 *)&stIspAwbOutput.stAwbOutput.au16RPo;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspAwbOutput.u8IspIdx = 0;
        ret = STFDRV_ISP_AWB_SetPo(
            pstCiConnection,
            &stIspAwbOutput
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_AWB_SetPo()\n", stIspAwbOutput.u8IspIdx);
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbOutput.au16RPo[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbOutput.stAwbOutput.au16RPo[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbOutput.au16GrPo[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbOutput.stAwbOutput.au16GrPo[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbOutput.au16GbPo[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbOutput.stAwbOutput.au16GbPo[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbOutput.au16BPo[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbOutput.stAwbOutput.au16BPo[u32Idx]
                );
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        u32Length = 8;
        for (i = 0; i < u32Length; i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_Y0_CFG_0 + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
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
    }
   //=========================================================================

    return ret;
}

STF_RESULT Test_AWB_SetPs(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[8 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    {
        u32Length = 8;
        for (i = 0; i < u32Length; i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_S0_CFG_0 + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
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
    }
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_AWB_SLOPE_PARAM stIspAwbSlope;

        u32Cnt = 16;
        pu16Temp = (STF_U16 *)&stIspAwbSlope.stAwbSlope.au16RPs;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspAwbSlope.u8IspIdx = 0;
        ret = STFDRV_ISP_AWB_SetPs(
            pstCiConnection,
            &stIspAwbSlope
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_AWB_SetPs()\n", stIspAwbSlope.u8IspIdx);
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbSlope.au16RPs[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbSlope.stAwbSlope.au16RPs[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbSlope.au16GrPs[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbSlope.stAwbSlope.au16GrPs[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbSlope.au16GbPs[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbSlope.stAwbSlope.au16GbPs[u32Idx]
                );
        }
        LOG_INFO("---\n");
        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("stAwbSlope.au16BPs[%d] = 0x%04X\n",
                u32Idx,
                stIspAwbSlope.stAwbSlope.au16BPs[u32Idx]
                );
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        u32Length = 8;
        for (i = 0; i < u32Length; i++) {
            u32Buffer[0 + i * 2] = ADDR_REG_AWB_S0_CFG_0 + (i * 4);
            u32Buffer[0 + i * 2 + 1] = 0;
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
    }
   //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_AWB)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Awb_Test(
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
        ret = Test_AWB_SetReg(
            );
        LOG_INFO("Test AWB_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the AWB module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_AWB)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_AWB_SetChnCrv(
            );
        LOG_INFO("Test AWB_SetChnCrv command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the AWB module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_AWB_SetPi(
            );
        LOG_INFO("Test AWB_SetPi command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the AWB module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_AWB_SetPo(
            );
        LOG_INFO("Test AWB_SetPo command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the AWB module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_AWB_SetPs(
            );
        LOG_INFO("Test AWB_SetPs command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the AWB module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_AWB)

    return ret;
}

//-----------------------------------------------------------------------------
