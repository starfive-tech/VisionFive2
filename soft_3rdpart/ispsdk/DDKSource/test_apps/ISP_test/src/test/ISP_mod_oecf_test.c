/**
  ******************************************************************************
  * @file  ISP_mod_oecf_test.c
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

#define LOG_TAG "Mod_Oecf_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_oecf_test.h"
#include "ISP_test.h"


/* ISPC OECF driver test structure */


/* ISPC OECF driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC OECF driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_OECF_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[96];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OECF_X0_CFG_0;
    u32Length = 96;
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
            ADDR_REG_OECF_X0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OECF_RDMA stIspOecfRdma;

        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OECF_X0_CFG_0;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.u32Param = ST_OECF_REG_LEN;
        stIspOecfRdma.stOecfRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 96 * 2;
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspOecfRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OECF_SetReg(
            pstCiConnection,
            &stIspOecfRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OECF_SetReg()\n",
            stIspOecfRdma.u8IspIdx);
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stRInput;
        LOG_INFO("RInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGrInput;
        LOG_INFO("GrInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGbInput;
        LOG_INFO("GbInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stBInput;
        LOG_INFO("BInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stROutput;
        LOG_INFO("ROutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGrOutput;
        LOG_INFO("GrOutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGbOutput;
        LOG_INFO("GbOutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stBOutput;
        LOG_INFO("BOutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stRSlope;
        LOG_INFO("RSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGrSlope;
        LOG_INFO("GrSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGbSlope;
        LOG_INFO("GbSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stBSlope;
        LOG_INFO("BSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OECF_X0_CFG_0;
    u32Length = 96;
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
            ADDR_REG_OECF_X0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_OECF)
//-----------------------------------------------------------------------------
STF_RESULT Test_OECF_SetInput(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[32];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OECF_X0_CFG_0;
    u32Length = 32;
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
            ADDR_REG_OECF_X0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OECF_RDMA stIspOecfRdma;

        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OECF_X0_CFG_0;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.u32Param = ST_OECF_REG_LEN;
        stIspOecfRdma.stOecfRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 32 * 2;
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stRInput;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspOecfRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OECF_SetInput(
            pstCiConnection,
            &stIspOecfRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OECF_SetInput()\n",
            stIspOecfRdma.u8IspIdx);
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stRInput;
        LOG_INFO("RInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGrInput;
        LOG_INFO("GrInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGbInput;
        LOG_INFO("GbInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stBInput;
        LOG_INFO("BInput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OECF_X0_CFG_0;
    u32Length = 32;
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
            ADDR_REG_OECF_X0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_OECF_SetOutput(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[32];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OECF_Y0_CFG_0;
    u32Length = 32;
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
            ADDR_REG_OECF_Y0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OECF_RDMA stIspOecfRdma;

        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OECF_X0_CFG_0;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.u32Param = ST_OECF_REG_LEN;
        stIspOecfRdma.stOecfRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 32 * 2;
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stROutput;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspOecfRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OECF_SetOutput(
            pstCiConnection,
            &stIspOecfRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OECF_SetOutput()\n",
            stIspOecfRdma.u8IspIdx);
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stROutput;
        LOG_INFO("ROutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGrOutput;
        LOG_INFO("GrOutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGbOutput;
        LOG_INFO("GbOutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stBOutput;
        LOG_INFO("BOutput = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OECF_Y0_CFG_0;
    u32Length = 32;
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
            ADDR_REG_OECF_Y0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_OECF_SetSlope(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[32];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OECF_S0_CFG_0;
    u32Length = 32;
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
            ADDR_REG_OECF_S0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OECF_RDMA stIspOecfRdma;

        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OECF_X0_CFG_0;
        stIspOecfRdma.stOecfRdma.stRdmaStartCmd.u32Param = ST_OECF_REG_LEN;
        stIspOecfRdma.stOecfRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 32 * 2;
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stRSlope;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspOecfRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OECF_SetSlope(
            pstCiConnection,
            &stIspOecfRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OECF_SetSlope()\n",
            stIspOecfRdma.u8IspIdx);
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stRSlope;
        LOG_INFO("RSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGrSlope;
        LOG_INFO("GrSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stGbSlope;
        LOG_INFO("GbSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        pu16Temp = (STF_U16 *)&stIspOecfRdma.stOecfRdma.stOecfReg.stBSlope;
        LOG_INFO("BSlope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OECF_S0_CFG_0;
    u32Length = 32;
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
            ADDR_REG_OECF_S0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_OECF)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Oecf_Test(
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
        ret = Test_OECF_SetReg(
            );
        LOG_INFO("Test OECF_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OECF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OECF)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OECF_SetInput(
            );
        LOG_INFO("Test OECF_SetInput command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OECF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OECF_SetOutput(
            );
        LOG_INFO("Test OECF_SetOutput command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OECF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OECF_SetSlope(
            );
        LOG_INFO("Test OECF_SetSlope command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OECF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OECF)

    return ret;
}

//-----------------------------------------------------------------------------
