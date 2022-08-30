/**
  ******************************************************************************
  * @file  ISP_mod_lcbq_test.c
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

#define LOG_TAG "Mod_Lcbq_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_lcbq_test.h"
#include "ISP_test.h"


/* ISPC LCBQ driver test structure */


/* ISPC LCBQ driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC LCBQ driver test interface */
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

STF_RESULT Test_LCBQ_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_LCBQ_REG_LEN * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = ST_LCBQ_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_CFG_0;
        } else if (1 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_CFG_1;
        } else if ((2 + (13 * 4)) > i) {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_GAIN0_CFG_0 + ((i - 2) * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_OFFSET0_CFG_0
                + ((i - (2 + (13 * 4))) * 4);
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
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_LCBQ_RDMA stIspLcbqRdma;

        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.u32Param =
            ST_LCBQ_WIN_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_1;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_COOR_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_LCBQ_GAIN0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.u32Param =
            ST_LCBQ_GAIN_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_OFFSET0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_OFST_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.value =
            ((rand() & 0xF) << 8) | ((rand() & 0xF) << 12);
        stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.value =
            (rand() & 0x0FFF) | ((rand() & 0x0FFF) << 16);
        u32Cnt = (sizeof(ST_LCBQ_GAIN_REG) / sizeof(STF_U32)) * 2;
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stRGain;
        for (u32Idx = 0; u32Idx < (u32Cnt * 4); u32Idx++) {
            if ((u32Cnt - 1) == (u32Idx % u32Cnt)) {
                pu16Temp[u32Idx] = 0;
            } else {
                pu16Temp[u32Idx] = (rand() & 0x01FF);
            }
        }
        u32Cnt = (sizeof(ST_LCBQ_OFST_REG) / sizeof(STF_U32)) * 2;
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stROffset;
        for (u32Idx = 0; u32Idx < (u32Cnt * 4); u32Idx++) {
            if ((u32Cnt - 1) == (u32Idx % u32Cnt)) {
                pu16Temp[u32Idx] = 0;
            } else {
                pu16Temp[u32Idx] = (rand() & 0x01FF);
            }
        }
        stIspLcbqRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_LCBQ_SetReg(
            pstCiConnection,
            &stIspLcbqRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCBQ_SetReg()\n",
            stIspLcbqRdma.u8IspIdx);
        LOG_INFO("Sub-Window size - Width = %02X(%d), Height = %02X(%d)\n",
            stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.width,
            Power(2,
                stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.width),
            stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.height,
            Power(2,
                stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.height)
            );
        LOG_INFO("Coordinate - X = %04X(%d), Y = %04X(%d)\n",
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.x,
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.x,
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.y,
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.y
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stRGain;
        LOG_INFO("R-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stGrGain;
        LOG_INFO("Gr-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stGbGain;
        LOG_INFO("Gb-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stBGain;
        LOG_INFO("B-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stROffset;
        LOG_INFO("R-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stGrOffset;
        LOG_INFO("Gr-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stGbOffset;
        LOG_INFO("Gb-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stBOffset;
        LOG_INFO("B-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = ST_LCBQ_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_CFG_0;
        } else if (1 == i) {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_CFG_1;
        } else if ((2 + (13 * 4)) > i) {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_GAIN0_CFG_0 + ((i - 2) * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_LCBQ_OFFSET0_CFG_0
                + ((i - (2 + (13 * 4))) * 4);
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

#if defined(EXTEND_IOCTL_COMMAND_LCBQ)
//-----------------------------------------------------------------------------
STF_RESULT Test_LCBQ_SetSubWindowSize(
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
    u32Offset = ADDR_REG_LCBQ_CFG_0;
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
        //STF_U16 *pu16Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_LCBQ_RDMA stIspLcbqRdma;

        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.u32Param =
            ST_LCBQ_WIN_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_1;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_COOR_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_LCBQ_GAIN0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.u32Param =
            ST_LCBQ_GAIN_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_OFFSET0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_OFST_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.value =
            ((rand() & 0xF) << 8) | ((rand() & 0xF) << 12);
        stIspLcbqRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_LCBQ_SetSubWindowSize(
            pstCiConnection,
            &stIspLcbqRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCBQ_SetSubWindowSize()\n",
            stIspLcbqRdma.u8IspIdx);
        LOG_INFO("Sub-Window size - Width = %02X(%d), Height = %02X(%d)\n",
            stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.width,
            Power(2,
                stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.width),
            stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.height,
            Power(2,
                stIspLcbqRdma.stLcbqRdma.stLcbqWinReg.SubWindowSize.field.height)
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_LCBQ_CFG_0;
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

STF_RESULT Test_LCBQ_SetCoordinate(
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
    u32Offset = ADDR_REG_LCBQ_CFG_1;
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
        //STF_U16 *pu16Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_LCBQ_RDMA stIspLcbqRdma;

        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.u32Param =
            ST_LCBQ_WIN_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_1;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_COOR_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_LCBQ_GAIN0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.u32Param =
            ST_LCBQ_GAIN_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_OFFSET0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_OFST_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.value =
            (rand() & 0x0FFF) | ((rand() & 0x0FFF) << 16);
        stIspLcbqRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_LCBQ_SetCoordinate(
            pstCiConnection,
            &stIspLcbqRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCBQ_SetCoordinate()\n",
            stIspLcbqRdma.u8IspIdx);
        LOG_INFO("Coordinate - X = %04X(%d), Y = %04X(%d)\n",
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.x,
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.x,
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.y,
            stIspLcbqRdma.stLcbqRdma.stLcbqCoorReg.Coordinate.field.y
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_LCBQ_CFG_1;
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

STF_RESULT Test_LCBQ_SetGain(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[13 * 4];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_LCBQ_GAIN0_CFG_0;
    u32Length = (sizeof(ST_LCBQ_GAIN_REG) / sizeof(STF_U32)) * 4;
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
            ADDR_REG_LCBQ_GAIN0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_LCBQ_RDMA stIspLcbqRdma;

        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.u32Param =
            ST_LCBQ_WIN_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_1;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_COOR_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_LCBQ_GAIN0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.u32Param =
            ST_LCBQ_GAIN_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_OFFSET0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_OFST_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = (sizeof(ST_LCBQ_GAIN_REG) / sizeof(STF_U32)) * 2;
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stRGain;
        for (u32Idx = 0; u32Idx < (u32Cnt * 4); u32Idx++) {
            if ((u32Cnt - 1) == (u32Idx % u32Cnt)) {
                pu16Temp[u32Idx] = 0;
            } else {

                pu16Temp[u32Idx] = (rand() & 0x01FF);
            }
        }
        stIspLcbqRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_LCBQ_SetGain(
            pstCiConnection,
            &stIspLcbqRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCBQ_SetGain()\n",
            stIspLcbqRdma.u8IspIdx);
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stRGain;
        LOG_INFO("R-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stGrGain;
        LOG_INFO("Gr-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stGbGain;
        LOG_INFO("Gb-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X,\n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqGainReg.stBGain;
        LOG_INFO("B-Gain = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_LCBQ_GAIN0_CFG_0;
    u32Length = (sizeof(ST_LCBQ_GAIN_REG) / sizeof(STF_U32)) * 4;
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
            ADDR_REG_LCBQ_GAIN0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_LCBQ_SetOffset(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[13 * 4];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_LCBQ_OFFSET0_CFG_0;
    u32Length = (sizeof(ST_LCBQ_OFST_REG) / sizeof(STF_U32)) * 4;
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
            ADDR_REG_LCBQ_OFFSET0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_LCBQ_RDMA stIspLcbqRdma;

        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaStartCmd.u32Param =
            ST_LCBQ_WIN_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_CFG_1;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_COOR_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.RegAddr =
            ADDR_REG_LCBQ_GAIN0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaGainCmd.u32Param =
            ST_LCBQ_GAIN_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.Tag = MREG_WR;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.RegAddr =
            ADDR_REG_LCBQ_OFFSET0_CFG_0;
        stIspLcbqRdma.stLcbqRdma.stRdmaOfstCmd.u32Param =
            ST_LCBQ_OFST_ALL_REG_LEN;
        stIspLcbqRdma.stLcbqRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = (sizeof(ST_LCBQ_OFST_REG) / sizeof(STF_U32)) * 2;
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stROffset;
        for (u32Idx = 0; u32Idx < (u32Cnt * 4); u32Idx++) {
            if ((u32Cnt - 1) == (u32Idx % u32Cnt)) {
                pu16Temp[u32Idx] = 0;
            } else {
                pu16Temp[u32Idx] = (rand() & 0x01FF);
            }
        }
        stIspLcbqRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_LCBQ_SetOffset(
            pstCiConnection,
            &stIspLcbqRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_LCBQ_SetOffset()\n",
            stIspLcbqRdma.u8IspIdx);
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stROffset;
        LOG_INFO("R-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stGrOffset;
        LOG_INFO("Gr-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stGbOffset;
        LOG_INFO("Gb-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        pu16Temp = (STF_U16 *)&stIspLcbqRdma.stLcbqRdma.stLcbqOfstReg.stBOffset;
        LOG_INFO("B-Offset = 0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, "\
            "0x%04X, 0x%04X, 0x%04X, 0x%04X, \n"\
            "0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4], pu16Temp[5], pu16Temp[6], pu16Temp[7],
            pu16Temp[8], pu16Temp[9], pu16Temp[10], pu16Temp[11],
            pu16Temp[12], pu16Temp[13], pu16Temp[14], pu16Temp[15],
            pu16Temp[16], pu16Temp[17], pu16Temp[18], pu16Temp[19],
            pu16Temp[20], pu16Temp[21], pu16Temp[22], pu16Temp[23],
            pu16Temp[24]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_LCBQ_OFFSET0_CFG_0;
    u32Length = (sizeof(ST_LCBQ_OFST_REG) / sizeof(STF_U32)) * 4;
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
            ADDR_REG_LCBQ_OFFSET0_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_LCBQ)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Lcbq_Test(
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
        ret = Test_LCBQ_SetReg(
            );
        LOG_INFO("Test LCBQ_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCBQ module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_LCBQ)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_LCBQ_SetSubWindowSize(
            );
        LOG_INFO("Test LCBQ_SetSubWindowSize command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCBQ module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_LCBQ_SetCoordinate(
            );
        LOG_INFO("Test LCBQ_SetCoordinate command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCBQ module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_LCBQ_SetGain(
            );
        LOG_INFO("Test LCBQ_SetGain command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCBQ module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_LCBQ_SetOffset(
            );
        LOG_INFO("Test LCBQ_SetOffset command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the LCBQ module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_LCBQ)

    return ret;
}

//-----------------------------------------------------------------------------
