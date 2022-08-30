/**
  ******************************************************************************
  * @file  ISP_mod_sat_test.c
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

#define LOG_TAG "Mod_Sat_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_sat_test.h"
#include "ISP_test.h"


/* ISPC SAT driver test structure */


/* ISPC SAT driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC SAT driver test interface */
//-----------------------------------------------------------------------------
static STF_U32 Power(STF_U32 u32Base, STF_U32 u32Exponent)
{
    STF_U32 u32Result = 1;
    STF_U32 u32Idx;


    for (u32Idx = 0; u32Idx < u32Exponent; u32Idx++) {
        u32Result *= u32Base;
    }

    return u32Result;
}

STF_RESULT Test_SAT_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_SAT_REG_LEN * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = ST_SAT_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        if (5 > i) {
            u32Buffer[i * 2] = ADDR_REG_ISAT0 + (i * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_IYADJ0 + ((i - 5) * 4);
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
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_SAT_RDMA stIspSatRdma;

        stIspSatRdma.stSatRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspSatRdma.stSatRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISAT0;
        stIspSatRdma.stSatRdma.stRdmaStartCmd.u32Param = ST_SAT_CHROMA_REG_LEN;
        stIspSatRdma.stSatRdma.stRdmaYCmd.Tag = MREG_WR;
        stIspSatRdma.stSatRdma.stRdmaYCmd.RegAddr = ADDR_REG_IYADJ0;
        stIspSatRdma.stSatRdma.stRdmaYCmd.u32Param = ST_SAT_Y_REG_LEN;
        stIspSatRdma.stSatRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspSatRdma.stSatRdma.stSatChromaReg.Gain.value = (rand() & 0x7FF)
            | ((rand() & 0xFFF) << 16);
        stIspSatRdma.stSatRdma.stSatChromaReg.Threshold.value = (rand() & 0x7FF)
            | ((rand() & 0xFFF) << 16);
        stIspSatRdma.stSatRdma.stSatChromaReg.UvOffset.value = (rand() & 0x7FF)
            | ((rand() & 0x7FF) << 16);
        stIspSatRdma.stSatRdma.stSatChromaReg.HueFactor.value = (rand() & 0x1FF)
            | ((rand() & 0x3FF) << 16);
        stIspSatRdma.stSatRdma.stSatChromaReg.ScalingCoefficient.value =
            (rand() & 0xF);
        stIspSatRdma.stSatRdma.stSatYReg.YAdjustment0.value = (rand() & 0x3FF)
            | ((rand() & 0x3FFF) << 16);
        stIspSatRdma.stSatRdma.stSatYReg.YAdjustment1.value = (rand() & 0x3FF)
            | ((rand() & 0x3FF) << 16);
        stIspSatRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SAT_SetReg(
            pstCiConnection,
            &stIspSatRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SAT_SetReg()\n", stIspSatRdma.u8IspIdx);
        LOG_INFO("Gain - Base = 0x%04X, Delta = 0x%04X\n",
            stIspSatRdma.stSatRdma.stSatChromaReg.Gain.field.cmab,
            stIspSatRdma.stSatRdma.stSatChromaReg.Gain.field.cmmd
            );
        LOG_INFO("Threshold - Base = 0x%04X, Delta = 0x%04X\n",
            stIspSatRdma.stSatRdma.stSatChromaReg.Threshold.field.cmb,
            stIspSatRdma.stSatRdma.stSatChromaReg.Threshold.field.cmd
            );
        LOG_INFO("U-Offset = 0x%04X, v-Offset = 0x%04X\n",
            stIspSatRdma.stSatRdma.stSatChromaReg.UvOffset.field.uoff,
            stIspSatRdma.stSatRdma.stSatChromaReg.UvOffset.field.voff
            );
        LOG_INFO("Hue factor - Cos = 0x%04X, Sin = 0x%04X\n",
            stIspSatRdma.stSatRdma.stSatChromaReg.HueFactor.field.cos,
            stIspSatRdma.stSatRdma.stSatChromaReg.HueFactor.field.sin
            );
        LOG_INFO("Scaling Coefficient = 0x%02X(%d)\n",
            stIspSatRdma.stSatRdma.stSatChromaReg.ScalingCoefficient.field.cmsf,
            Power(2,
                stIspSatRdma.stSatRdma.stSatChromaReg.ScalingCoefficient.field.cmsf)
            );
        LOG_INFO("Y Adjustment - Y_Input_Min = 0x%04X, Y_Slope = 0x%04X\n",
            stIspSatRdma.stSatRdma.stSatYReg.YAdjustment0.field.yimin,
            stIspSatRdma.stSatRdma.stSatYReg.YAdjustment0.field.yoir
            );
        LOG_INFO("Y Adjustment - Y_Output_Min = 0x%04X, "\
            "Y_Output_Max = 0x%04X\n",
            stIspSatRdma.stSatRdma.stSatYReg.YAdjustment1.field.yomin,
            stIspSatRdma.stSatRdma.stSatYReg.YAdjustment1.field.yomax
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = ST_SAT_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        if (5 > i) {
            u32Buffer[i * 2] = ADDR_REG_ISAT0 + (i * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_IYADJ0 + ((i - 5) * 4);
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

#if defined(EXTEND_IOCTL_COMMAND_SAT)
//-----------------------------------------------------------------------------
STF_RESULT Test_SAT_SetSaturationGain(
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
    u32Offset = ADDR_REG_ISAT0;
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
        ST_SAT_GAIN_PARAM stSatGain;

        stSatGain.stSatGain.u16AmplificationBase = (rand() & 0x7FF);
        stSatGain.stSatGain.u16AmplificationDelta = (rand() & 0xFFF);
        stSatGain.u8IspIdx = 0;
        ret = STFDRV_ISP_SAT_SetSaturationGain(
            pstCiConnection,
            &stSatGain
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SAT_SetSaturationGain()\n",
            stSatGain.u8IspIdx);
        LOG_INFO("Gain - Base = 0x%04X, Delta = 0x%04X\n",
            stSatGain.stSatGain.u16AmplificationBase,
            stSatGain.stSatGain.u16AmplificationDelta
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISAT0;
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

STF_RESULT Test_SAT_SetSaturationThreshold(
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
    u32Offset = ADDR_REG_ISAT1;
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
        ST_SAT_THRESHOLD_PARAM stSatThreshold;

        stSatThreshold.stSatThreshold.u16Base = (rand() & 0x7FF);
        stSatThreshold.stSatThreshold.u16Delta = (rand() & 0xFFF);
        stSatThreshold.u8IspIdx = 0;
        ret = STFDRV_ISP_SAT_SetSaturationThreshold(
            pstCiConnection,
            &stSatThreshold
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SAT_SetSaturationThreshold()\n",
            stSatThreshold.u8IspIdx);
        LOG_INFO("Threshold - Base = 0x%04X, Delta = 0x%04X\n",
            stSatThreshold.stSatThreshold.u16Base,
            stSatThreshold.stSatThreshold.u16Delta
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISAT1;
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

STF_RESULT Test_SAT_SetSaturationUVOffset(
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
    u32Offset = ADDR_REG_ISAT2;
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
        ST_SAT_UV_OFFSET_PARAM stSatUvOffset;

        stSatUvOffset.stSatUvOffset.u16UOffset = (rand() & 0x7FF);
        stSatUvOffset.stSatUvOffset.u16VOffset = (rand() & 0x7FF);
        stSatUvOffset.u8IspIdx = 0;
        ret = STFDRV_ISP_SAT_SetSaturationUVOffset(
            pstCiConnection,
            &stSatUvOffset
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SAT_SetSaturationUVOffset()\n",
            stSatUvOffset.u8IspIdx);
        LOG_INFO("U-Offset = 0x%04X, v-Offset = 0x%04X\n",
            stSatUvOffset.stSatUvOffset.u16UOffset,
            stSatUvOffset.stSatUvOffset.u16VOffset
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISAT2;
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

STF_RESULT Test_SAT_SetSaturationHueFactor(
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
    u32Offset = ADDR_REG_ISAT3;
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
        ST_SAT_HUE_FACTOR_PARAM stSatHueFactor;

        stSatHueFactor.stSatHueFactor.u16Cos = (rand() & 0x1FF);
        stSatHueFactor.stSatHueFactor.u16Sin = (rand() & 0x3FF);
        stSatHueFactor.u8IspIdx = 0;
        ret = STFDRV_ISP_SAT_SetSaturationHueFactor(
            pstCiConnection,
            &stSatHueFactor
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SAT_SetSaturationHueFactor()\n",
            stSatHueFactor.u8IspIdx);
        LOG_INFO("Hue factor - Cos = 0x%04X, Sin = 0x%04X\n",
            stSatHueFactor.stSatHueFactor.u16Cos,
            stSatHueFactor.stSatHueFactor.u16Sin
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISAT3;
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

STF_RESULT Test_SAT_SetSaturationScalingCoefficient(
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
    u32Offset = ADDR_REG_ISAT4;
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
        STF_U8 u8ScalingFactor = (rand() & 0xF);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SAT_SetSaturationScalingCoefficient(
            pstCiConnection,
            u8IspIdx,
            u8ScalingFactor
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SAT_SetSaturationScalingCoefficient()\n",
            u8IspIdx);
        LOG_INFO("Scaling Coefficient = 0x%02X(%d)\n",
            u8ScalingFactor,
            Power(2, u8ScalingFactor)
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISAT4;
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

STF_RESULT Test_SAT_SetSaturationYAdjustment(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[4];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_IYADJ0;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_IYADJ0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        ST_SAT_Y_ADJ_PARAM stIspSatYAdj;

        stIspSatYAdj.stSatYAdj.u16YiMin = (rand() & 0x3FF);
        stIspSatYAdj.stSatYAdj.u16Yoir = (rand() & 0x3FFF);
        stIspSatYAdj.stSatYAdj.u16YoMin = (rand() & 0x3FF);
        stIspSatYAdj.stSatYAdj.u16YoMax = (rand() & 0x3FF);
        stIspSatYAdj.u8IspIdx = 0;
        ret = STFDRV_ISP_SAT_SetSaturationYAdjustment(
            pstCiConnection,
            &stIspSatYAdj
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SAT_SetSaturationYAdjustment()\n",
            stIspSatYAdj.u8IspIdx);
        LOG_INFO("Y Adjustment - Y_Input_Min = 0x%04X, Y_Slope = 0x%04X\n",
            stIspSatYAdj.stSatYAdj.u16YiMin,
            stIspSatYAdj.stSatYAdj.u16Yoir
            );
        LOG_INFO("Y Adjustment - Y_Output_Min = 0x%04X, "\
            "Y_Output_Max = 0x%04X\n",
            stIspSatYAdj.stSatYAdj.u16YoMin,
            stIspSatYAdj.stSatYAdj.u16YoMax
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IYADJ0;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_IYADJ0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_SAT)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Sat_Test(
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
        ret = Test_SAT_SetReg(
            );
        LOG_INFO("Test SAT_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SAT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_SAT)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SAT_SetSaturationGain(
            );
        LOG_INFO("Test SAT_SetSaturationGain command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SAT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SAT_SetSaturationThreshold(
            );
        LOG_INFO("Test SAT_SetSaturationThreshold command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SAT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SAT_SetSaturationUVOffset(
            );
        LOG_INFO("Test SAT_SetSaturationUVOffset command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SAT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SAT_SetSaturationHueFactor(
            );
        LOG_INFO("Test SAT_SetSaturationHueFactor command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SAT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SAT_SetSaturationScalingCoefficient(
            );
        LOG_INFO("Test SAT_SetSaturationScalingCoefficient command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SAT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SAT_SetSaturationYAdjustment(
            );
        LOG_INFO("Test SAT_SetSaturationYAdjustment command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SAT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_SAT)

    return ret;
}

//-----------------------------------------------------------------------------
