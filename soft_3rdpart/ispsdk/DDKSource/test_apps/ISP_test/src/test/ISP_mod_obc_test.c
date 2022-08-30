/**
  ******************************************************************************
  * @file  ISP_mod_obc_test.c
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

#define LOG_TAG "Mod_Obc_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_obc_test.h"
#include "ISP_test.h"


/* ISPC OBC driver test structure */


/* ISPC OBC driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC OBC driver test interface */
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

STF_RESULT Test_OBC_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[9 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 9;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_OBC_CFG + (i * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_OBCG_CFG_0 + ((i - 1) * 4);
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
        ST_ISP_OBC_RDMA stIspObcRdma;

        stIspObcRdma.stObcRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OBC_CFG;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.u32Param =
            ST_OBC_CFG_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_OBCG_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.u32Param =
            ST_OBC_GAIN_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_OBCO_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.u32Param =
            ST_OBC_OFST_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspObcRdma.stObcRdma.stObcCfgReg.Region.value = (rand() & 0xF)
            | ((rand() & 0xF) << 4);
        u32Cnt = 4;
        pu32Temp = (STF_U32 *)&stIspObcRdma.stObcRdma.stObcGainReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] =
                (rand() & 0xFF) |
                ((rand() & 0xFF) << 8) |
                ((rand() & 0xFF) << 16) |
                ((rand() & 0xFF) << 24);
        }
        u32Cnt = 4;
        pu32Temp = (STF_U32 *)&stIspObcRdma.stObcRdma.stObcOfstReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] =
                (rand() & 0xFF) |
                ((rand() & 0xFF) << 8) |
                ((rand() & 0xFF) << 16) |
                ((rand() & 0xFF) << 24);
        }
        stIspObcRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OBC_SetReg(
            pstCiConnection,
            &stIspObcRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OBC_SetReg()\n", stIspObcRdma.u8IspIdx);
        LOG_INFO("Region - Width = 0x%02X(%d), Height = 0x%02X(%d)\n",
            stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.width,
            Power(2, stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.width),
            stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.height,
            Power(2, stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.height)
            );
        LOG_INFO("RGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.d_s0
            );
        LOG_INFO("GrGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.d_s0
            );
        LOG_INFO("GbGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.d_s0
            );
        LOG_INFO("BGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.d_s0
            );
        LOG_INFO("ROffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.d_s0
            );
        LOG_INFO("GrOffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.d_s0
            );
        LOG_INFO("GbOffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.d_s0
            );
        LOG_INFO("BOffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.d_s0
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 9;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_OBC_CFG + (i * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_OBCG_CFG_0 + ((i - 1) * 4);
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

#if defined(EXTEND_IOCTL_COMMAND_OBC)
//-----------------------------------------------------------------------------
STF_RESULT Test_OBC_SetRegion(
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
    u32Offset = ADDR_REG_OBC_CFG;
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
        ST_ISP_OBC_RDMA stIspObcRdma;

        stIspObcRdma.stObcRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OBC_CFG;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.u32Param =
            ST_OBC_CFG_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_OBCG_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.u32Param =
            ST_OBC_GAIN_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_OBCO_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.u32Param =
            ST_OBC_OFST_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspObcRdma.stObcRdma.stObcCfgReg.Region.value = (rand() & 0xF)
            | ((rand() & 0xF) << 4);
        stIspObcRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OBC_SetRegion(
            pstCiConnection,
            &stIspObcRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OBC_SetRegion()\n",
            stIspObcRdma.u8IspIdx);
        LOG_INFO("Region - Width = 0x%02X(%d), Height = 0x%02X(%d)\n",
            stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.width,
            Power(2, stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.width),
            stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.height,
            Power(2, stIspObcRdma.stObcRdma.stObcCfgReg.Region.field.height)
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OBC_CFG;
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

STF_RESULT Test_OBC_SetGain(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[4 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 4;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i * 2] = ADDR_REG_OBCG_CFG_0 + (i * 4);
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
        ST_ISP_OBC_RDMA stIspObcRdma;

        stIspObcRdma.stObcRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OBC_CFG;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.u32Param =
            ST_OBC_CFG_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_OBCG_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.u32Param =
            ST_OBC_GAIN_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_OBCO_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.u32Param =
            ST_OBC_OFST_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 4;
        pu32Temp = (STF_U32 *)&stIspObcRdma.stObcRdma.stObcGainReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] =
                (rand() & 0xFF) |
                ((rand() & 0xFF) << 8) |
                ((rand() & 0xFF) << 16) |
                ((rand() & 0xFF) << 24);
        }
        stIspObcRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OBC_SetGain(
            pstCiConnection,
            &stIspObcRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OBC_SetGain()\n", stIspObcRdma.u8IspIdx);
        LOG_INFO("RGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.RGain.field.d_s0
            );
        LOG_INFO("GrGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GrGain.field.d_s0
            );
        LOG_INFO("GbGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.GbGain.field.d_s0
            );
        LOG_INFO("BGain = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.a_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.b_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.c_s0,
            stIspObcRdma.stObcRdma.stObcGainReg.BGain.field.d_s0
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 4;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i * 2] = ADDR_REG_OBCG_CFG_0 + (i * 4);
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

STF_RESULT Test_OBC_SetOffset(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[4 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 4;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i * 2] = ADDR_REG_OBCO_CFG_0 + (i * 4);
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
        ST_ISP_OBC_RDMA stIspObcRdma;

        stIspObcRdma.stObcRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OBC_CFG;
        stIspObcRdma.stObcRdma.stRdmaStartCmd.u32Param =
            ST_OBC_CFG_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_OBCG_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaGainCmd.u32Param =
            ST_OBC_GAIN_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_OBCO_CFG_0;
        stIspObcRdma.stObcRdma.stRdmaOfstCmd.u32Param =
            ST_OBC_OFST_REG_LEN;
        stIspObcRdma.stObcRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 4;
        pu32Temp = (STF_U32 *)&stIspObcRdma.stObcRdma.stObcOfstReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] =
                (rand() & 0xFF) |
                ((rand() & 0xFF) << 8) |
                ((rand() & 0xFF) << 16) |
                ((rand() & 0xFF) << 24);
        }
        stIspObcRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OBC_SetOffset(
            pstCiConnection,
            &stIspObcRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OBC_SetOffset()\n", stIspObcRdma.u8IspIdx);
        LOG_INFO("ROffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.ROffset.field.d_s0
            );
        LOG_INFO("GrOffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GrOffset.field.d_s0
            );
        LOG_INFO("GbOffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.GbOffset.field.d_s0
            );
        LOG_INFO("BOffset = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.a_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.b_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.c_s0,
            stIspObcRdma.stObcRdma.stObcOfstReg.BOffset.field.d_s0
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 4;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i * 2] = ADDR_REG_OBCO_CFG_0 + (i * 4);
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_OBC)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Obc_Test(
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
        ret = Test_OBC_SetReg(
            );
        LOG_INFO("Test Test_OBC_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OBC)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OBC_SetRegion(
            );
        LOG_INFO("Test OBC_SetRegion command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OBC_SetGain(
            );
        LOG_INFO("Test OBC_SetGain command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OBC_SetOffset(
            );
        LOG_INFO("Test OBC_SetOffset command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OBC)

    return ret;
}

//-----------------------------------------------------------------------------
