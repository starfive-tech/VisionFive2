/**
  ******************************************************************************
  * @file  ISP_mod_oecfhm_test.c
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

#define LOG_TAG "Mod_OecfHm_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_oecfhm_test.h"
#include "ISP_test.h"


/* ISPC OECFHM driver test structure */


/* ISPC OECFHM driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC OECFHM driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_OECFHM_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OECFHM_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OECFHM_Y_CFG_0;
    u32Length = ST_OECFHM_REG_LEN;
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
            ADDR_REG_OECFHM_Y_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OECFHM_RDMA stIspOecfHmRdma;

        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OECFHM_Y_CFG_0;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.u32Param =
            ST_OECFHM_REG_LEN;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = (ST_OECFHM_REG_LEN / 2) * 2;
        pu16Temp = (STF_U16 *)&stIspOecfHmRdma.stOecfHmRdma.stOecfHmReg;
        for (u32Idx = 0; u32Idx < (u32Cnt * 2); u32Idx++) {
            if ((u32Cnt - 1) == (u32Idx % u32Cnt)) {
                pu16Temp[u32Idx] = 0;
            } else {
                pu16Temp[u32Idx] = (rand() & 0x0FFF);
            }
        }
        stIspOecfHmRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OECFHM_SetReg(
            pstCiConnection,
            &stIspOecfHmRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OECFHM_SetReg()\n",
            stIspOecfHmRdma.u8IspIdx);
        pu16Temp =
            (STF_U16 *)&stIspOecfHmRdma.stOecfHmRdma.stOecfHmReg.YInput0_1;
        LOG_INFO("Input = 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4]
            );
        pu16Temp =
            (STF_U16 *)&stIspOecfHmRdma.stOecfHmRdma.stOecfHmReg.Slope0_1;
        LOG_INFO("Slope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OECFHM_Y_CFG_0;
    u32Length = ST_OECFHM_REG_LEN;
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
            ADDR_REG_OECFHM_Y_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_OECFHM)
//-----------------------------------------------------------------------------
STF_RESULT Test_OECFHM_SetInput(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OECFHM_REG_LEN / 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OECFHM_Y_CFG_0;
    u32Length = ST_OECFHM_REG_LEN / 2;
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
            ADDR_REG_OECFHM_Y_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OECFHM_RDMA stIspOecfHmRdma;

        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OECFHM_Y_CFG_0;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.u32Param =
            ST_OECFHM_REG_LEN;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = (ST_OECFHM_REG_LEN / 2) * 2;
        pu16Temp =
            (STF_U16 *)&stIspOecfHmRdma.stOecfHmRdma.stOecfHmReg.YInput0_1;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if ((u32Cnt - 1) == (u32Idx % u32Cnt)) {
                pu16Temp[u32Idx] = 0;
            } else {
                pu16Temp[u32Idx] = (rand() & 0x0FFF);
            }
        }
        stIspOecfHmRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OECFHM_SetInput(
            pstCiConnection,
            &stIspOecfHmRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OECFHM_SetInput()\n",
            stIspOecfHmRdma.u8IspIdx);
        pu16Temp =
            (STF_U16 *)&stIspOecfHmRdma.stOecfHmRdma.stOecfHmReg.YInput0_1;
        LOG_INFO("Input = 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OECFHM_Y_CFG_0;
    u32Length = ST_OECFHM_REG_LEN / 2;
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
            ADDR_REG_OECFHM_Y_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_OECFHM_SetSlope(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OECFHM_REG_LEN / 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OECFHM_S_CFG_0;
    u32Length = ST_OECFHM_REG_LEN / 2;
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
            ADDR_REG_OECFHM_S_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OECFHM_RDMA stIspOecfHmRdma;

        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OECFHM_Y_CFG_0;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaStartCmd.u32Param =
            ST_OECFHM_REG_LEN;
        stIspOecfHmRdma.stOecfHmRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = (ST_OECFHM_REG_LEN / 2) * 2;
        pu16Temp =
            (STF_U16 *)&stIspOecfHmRdma.stOecfHmRdma.stOecfHmReg.Slope0_1;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if ((u32Cnt - 1) == (u32Idx % u32Cnt)) {
                pu16Temp[u32Idx] = 0;
            } else {
                pu16Temp[u32Idx] = (rand() & 0x0FFF);
            }
        }
        stIspOecfHmRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OECFHM_SetSlope(
            pstCiConnection,
            &stIspOecfHmRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OECFHM_SetSlope()\n",
            stIspOecfHmRdma.u8IspIdx);
        pu16Temp =
            (STF_U16 *)&stIspOecfHmRdma.stOecfHmRdma.stOecfHmReg.Slope0_1;
        LOG_INFO("Slope = 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X\n",
            pu16Temp[0], pu16Temp[1], pu16Temp[2], pu16Temp[3],
            pu16Temp[4]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OECFHM_S_CFG_0;
    u32Length = ST_OECFHM_REG_LEN / 2;
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
            ADDR_REG_OECFHM_S_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_OECFHM)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_OecfHm_Test(
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
        ret = Test_OECFHM_SetReg(
            );
        LOG_INFO("Test OECFHM_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OECFHM module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OECFHM)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OECFHM_SetInput(
            );
        LOG_INFO("Test OECFHM_SetInput command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OECFHM module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OECFHM_SetSlope(
            );
        LOG_INFO("Test OECFHM_SetSlope command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OECFHM module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OECFHM)

    return ret;
}

//-----------------------------------------------------------------------------
