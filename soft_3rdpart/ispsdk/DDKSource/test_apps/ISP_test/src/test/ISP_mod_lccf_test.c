/**
  ******************************************************************************
  * @file  ISP_mod_lccf_test.c
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

#define LOG_TAG "Mod_Lccf_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_lccf_test.h"
#include "ISP_test.h"


/* ISPC LCCF driver test structure */


/* ISPC LCCF driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC LCCF driver test interface */
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

STF_RESULT Test_LCCF_SetReg(
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
    u32Length = 6;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCCF_CFG_0;
        } else if (1 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCCF_CFG_1;
        } else {
            u32Buffer[i * 2] = ADDR_REG_LCCF_CFG_2 + ((i - 2) * 4);
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
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_LCCF_RDMA stIspLccfRdma;

        stIspLccfRdma.stLccfRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspLccfRdma.stLccfRdma.stRdmaStartCmd.RegAddr = ADDR_REG_LCCF_CFG_0;
        stIspLccfRdma.stLccfRdma.stRdmaStartCmd.u32Param = ST_LCCF_CTR_REG_LEN;
        stIspLccfRdma.stLccfRdma.stRdmaRadCmd.Tag = MREG_WR;
        stIspLccfRdma.stLccfRdma.stRdmaRadCmd.RegAddr = ADDR_REG_LCCF_CFG_1;
        stIspLccfRdma.stLccfRdma.stRdmaRadCmd.u32Param = ST_LCCF_RAD_REG_LEN;
        stIspLccfRdma.stLccfRdma.stRdmaFactorCmd.Tag = MREG_WR;
        stIspLccfRdma.stLccfRdma.stRdmaFactorCmd.RegAddr = ADDR_REG_LCCF_CFG_2;
        stIspLccfRdma.stLccfRdma.stRdmaFactorCmd.u32Param =
            ST_LCCF_FACTOR_REG_LEN;
        stIspLccfRdma.stLccfRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspLccfRdma.stLccfRdma.stLccfCtrReg.Center.value = (rand() & 0x7FFF)
            | ((rand() & 0x7FFF) << 16);
        stIspLccfRdma.stLccfRdma.stLccfRadReg.Radius.value = (rand() & 0xF);
        u32Cnt = 4;
        pu32Temp = (STF_U32 *)&stIspLccfRdma.stLccfRdma.stLccfFactorReg.RFactor;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0x1FFF) | ((rand() & 0x0FFF) << 16);
        }
        stIspLccfRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_LCCF_SetReg(
            pstCiConnection,
            &stIspLccfRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCCF_SetReg()\n",
            stIspLccfRdma.u8IspIdx);
        LOG_INFO("Center = %02X(%d), %02X(%d)\n",
            stIspLccfRdma.stLccfRdma.stLccfCtrReg.Center.field.xd,
            stIspLccfRdma.stLccfRdma.stLccfCtrReg.Center.field.xd,
            stIspLccfRdma.stLccfRdma.stLccfCtrReg.Center.field.yd,
            stIspLccfRdma.stLccfRdma.stLccfCtrReg.Center.field.yd
            );
        LOG_INFO("Radius = %02X(%d)\n",
            stIspLccfRdma.stLccfRdma.stLccfRadReg.Radius.field.m,
            Power(2, stIspLccfRdma.stLccfRdma.stLccfRadReg.Radius.field.m)
            );
        LOG_INFO("R channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.RFactor.field.f1_s0,
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.RFactor.field.f2_s0
            );
        LOG_INFO("Gr channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.GrFactor.field.f1_s1,
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.GrFactor.field.f2_s1
            );
        LOG_INFO("Gb channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.GbFactor.field.f1_s2,
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.GbFactor.field.f2_s2
            );
        LOG_INFO("B channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.BFactor.field.f1_s3,
            stIspLccfRdma.stLccfRdma.stLccfFactorReg.BFactor.field.f2_s3
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 6;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCCF_CFG_0;
        }
        else if (1 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCCF_CFG_1;
        } else {
            u32Buffer[i * 2] = ADDR_REG_LCCF_CFG_2 + ((i - 2) * 4);
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

#if defined(EXTEND_IOCTL_COMMAND_LCCF)
//-----------------------------------------------------------------------------
STF_RESULT Test_LCCF_SetCenter(
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
    u32Offset = ADDR_REG_LCCF_CFG_0;
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
        ST_LCCF_CENTER_PARAM stLccfCenter;

        stLccfCenter.stPoint.u16X = (rand() & 0x7FFF);
        stLccfCenter.stPoint.u16Y = (rand() & 0x7FFF);
        stLccfCenter.u8IspIdx = 0;
        ret = STFDRV_ISP_LCCF_SetCenter(
            pstCiConnection,
            &stLccfCenter
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCCF_SetCenter()\n",
            stLccfCenter.u8IspIdx);
        LOG_INFO("Center = %02X(%d), %02X(%d)\n",
            stLccfCenter.stPoint.u16X,
            stLccfCenter.stPoint.u16X,
            stLccfCenter.stPoint.u16Y,
            stLccfCenter.stPoint.u16Y
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_LCCF_CFG_0;
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

STF_RESULT Test_LCCF_SetRadius(
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
    u32Offset = ADDR_REG_LCCF_CFG_1;
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
        STF_U8 u8Radius = (rand() & 0xF);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_LCCF_SetRadius(
            pstCiConnection,
            u8IspIdx,
            u8Radius
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCCF_SetRadius()\n", u8IspIdx);
        LOG_INFO("Radius = %02X(%d)\n",
            u8Radius,
            Power(2, u8Radius)
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_LCCF_CFG_1;
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

STF_RESULT Test_LCCF_SetFactor(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[4 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_LCCF_CFG_2;
    u32Length = 4;
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
            ADDR_REG_LCCF_CFG_2+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_LCCF_FACTOR_PARAM stIspLccfFactor;

        u32Cnt = 4;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            stIspLccfFactor.stLccfFactor.au16F1[u32Idx] = (rand() & 0x1FFF);
            stIspLccfFactor.stLccfFactor.au16F2[u32Idx] = (rand() & 0x0FFF);
        }
        stIspLccfFactor.u8IspIdx = 0;
        ret = STFDRV_ISP_LCCF_SetFactor(
            pstCiConnection,
            &stIspLccfFactor
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCCF_SetFactor()\n",
            stIspLccfFactor.u8IspIdx);
        LOG_INFO("R channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfFactor.stLccfFactor.au16F1[0],
            stIspLccfFactor.stLccfFactor.au16F2[0]
            );
        LOG_INFO("Gr channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfFactor.stLccfFactor.au16F1[1],
            stIspLccfFactor.stLccfFactor.au16F2[1]
            );
        LOG_INFO("Gb channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfFactor.stLccfFactor.au16F1[2],
            stIspLccfFactor.stLccfFactor.au16F2[2]
            );
        LOG_INFO("B channel - F1 = 0x%04X, F2 = 0x%04X\n",
            stIspLccfFactor.stLccfFactor.au16F1[3],
            stIspLccfFactor.stLccfFactor.au16F2[3]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_LCCF_CFG_2;
    u32Length = 4;
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
            ADDR_REG_LCCF_CFG_2+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_LCCF)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Lccf_Test(
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
        ret = Test_LCCF_SetReg(
            );
        LOG_INFO("Test LCCF_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCCF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_LCCF)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_LCCF_SetCenter(
            );
        LOG_INFO("Test LCCF_SetCenter command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCCF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_LCCF_SetRadius(
            );
        LOG_INFO("Test LCCF_SetRadius command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCCF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_LCCF_SetFactor(
            );
        LOG_INFO("Test LCCF_SetFactor command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCCF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_LCCF)

    return ret;
}

//-----------------------------------------------------------------------------
