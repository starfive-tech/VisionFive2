/**
  ******************************************************************************
  * @file  ISP_mod_yhist_test.c
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

#define LOG_TAG "Mod_YHist_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_yhist_test.h"
#include "ISP_test.h"


/* ISPC YHIST driver test structure */


/* ISPC YHIST driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC YHIST driver test interface */
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

STF_RESULT Test_YHIST_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_YHIST_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_YHIST_CFG_0;
    u32Length = ST_YHIST_REG_LEN;
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
            ADDR_REG_OBA_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U16 *pu16Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_YHIST_RDMA stIspYHistRdma;

        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_YHIST_CFG_0;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.u32Param = ST_YHIST_REG_LEN;
        stIspYHistRdma.stYHistRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.value =
            (rand() & 0x1FFF) | ((rand() & 0x1FFF) << 16);
        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.value =
            (rand() & 0x1FFF) | ((rand() & 0x1FFF) << 16);
        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.value = (rand() & 0x7)
            | ((rand() & 0x7) << 16);
        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg3.value = (rand() & 0x1);
#if defined(V4L2_DRIVER)
#else
        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg4.value = (rand()
            & 0xFFFFFF80);
#endif //#if defined(V4L2_DRIVER)
        stIspYHistRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_YHIST_SetReg(
            pstCiConnection,
            &stIspYHistRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_SetReg()\n",
            stIspYHistRdma.u8IspIdx);
        LOG_INFO("Start Location HStart = 0x%04X(%d), VStart = 0x%04X(%d)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_hstart,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_hstart,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_vstart,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_vstart
            );
        LOG_INFO("Active Window size Width = 0x%04X(%d), Height = 0x%04X(%d)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_width,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_width + 1,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_height,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_height + 1
            );
        LOG_INFO("Decimation window Width = 0x%04X(%d), Height = 0x%04X(%d)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_etw,
            Power(2,
                stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_etw),
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_eth,
            Power(2,
                stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_eth)
            );
        LOG_INFO("Mux select = 0x%01X(%s)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg3.value,
            ((stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg3.value) ?
                ("After Y curve") : ("Before Y curve"))
            );
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("Dump buffer address = 0x%08X\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg4.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_0;
    u32Length = ST_YHIST_REG_LEN;
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
            ADDR_REG_OBA_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_YHIST)
STF_RESULT Test_YHIST_SetStartLocation(
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
    u32Offset = ADDR_REG_YHIST_CFG_0;
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
        ST_ISP_YHIST_RDMA stIspYHistRdma;

        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_YHIST_CFG_0;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.u32Param = ST_YHIST_REG_LEN;
        stIspYHistRdma.stYHistRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.value =
            (rand() & 0x1FFF) | ((rand() & 0x1FFF) << 16);
        stIspYHistRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_YHIST_SetStartLocation(
            pstCiConnection,
            &stIspYHistRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_SetStartLocation()\n",
            stIspYHistRdma.u8IspIdx);
        LOG_INFO("Start Location HStart = 0x%04X(%d), VStart = 0x%04X(%d)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_hstart,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_hstart,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_vstart,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg0.field.yh_vstart
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_0;
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

STF_RESULT Test_YHIST_SetWindowSize(
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
    u32Offset = ADDR_REG_YHIST_CFG_1;
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
        ST_ISP_YHIST_RDMA stIspYHistRdma;

        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_YHIST_CFG_0;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.u32Param = ST_YHIST_REG_LEN;
        stIspYHistRdma.stYHistRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.value =
            (rand() & 0x1FFF) | ((rand() & 0x1FFF) << 16);
        stIspYHistRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_YHIST_SetWindowSize(
            pstCiConnection,
            &stIspYHistRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_SetWindowSize()\n",
            stIspYHistRdma.u8IspIdx);
        LOG_INFO("Active Window size Width = 0x%04X(%d), Height = 0x%04X(%d)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_width,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_width + 1,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_height,
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg1.field.yh_height + 1
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_1;
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

STF_RESULT Test_YHIST_SetDecimation(
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
    u32Offset = ADDR_REG_YHIST_CFG_2;
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
        ST_ISP_YHIST_RDMA stIspYHistRdma;

        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_YHIST_CFG_0;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.u32Param = ST_YHIST_REG_LEN;
        stIspYHistRdma.stYHistRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.value = (rand() & 0x7)
            | ((rand() & 0x7) << 16);
        stIspYHistRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_YHIST_SetDecimation(
            pstCiConnection,
            &stIspYHistRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_SetDecimation()\n",
            stIspYHistRdma.u8IspIdx);
        LOG_INFO("Decimation window Width = 0x%04X(%d), Height = 0x%04X(%d)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_etw,
            Power(2,
                stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_etw),
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_eth,
            Power(2,
                stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg2.field.yh_dec_eth)
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_2;
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

STF_RESULT Test_YHIST_SetMuxSel(
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
    u32Offset = ADDR_REG_YHIST_CFG_3;
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
        ST_ISP_YHIST_RDMA stIspYHistRdma;

        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_YHIST_CFG_0;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.u32Param = ST_YHIST_REG_LEN;
        stIspYHistRdma.stYHistRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg3.value = (rand() & 0x1);
        stIspYHistRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_YHIST_SetMuxSel(
            pstCiConnection,
            &stIspYHistRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_SetMuxSel()\n",
            stIspYHistRdma.u8IspIdx);
        LOG_INFO("Mux select = 0x%01X(%s)\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg3.value,
            ((stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg3.value) ?
                ("After Y curve") : ("Before Y curve"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_3;
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

#if defined(V4L2_DRIVER)
#else
STF_RESULT Test_YHIST_SetDumpBufAddr(
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
    u32Offset = ADDR_REG_YHIST_CFG_4;
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
        ST_ISP_YHIST_RDMA stIspYHistRdma;

        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_YHIST_CFG_0;
        stIspYHistRdma.stYHistRdma.stRdmaStartCmd.u32Param = ST_YHIST_REG_LEN;
        stIspYHistRdma.stYHistRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg4.value =
            (rand() & 0xFFFFFF80);
        stIspYHistRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_YHIST_SetDumpBufAddr(
            pstCiConnection,
            &stIspYHistRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_SetDumpBufAddr()\n",
            stIspYHistRdma.u8IspIdx);
        LOG_INFO("Dump buffer address = 0x%08X\n",
            stIspYHistRdma.stYHistRdma.stYHistReg.YHistCfg4.value
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_4;
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

#endif //#if defined(V4L2_DRIVER)
STF_RESULT Test_YHIST_GetDumpStatus(
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
    u32Offset = ADDR_REG_YHIST_CFG_3;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_3;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFFFFFD) | ((rand() & 0x01) << 1);
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_3;
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
        STF_BOOL8 bDumpStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bDumpStatus = STFDRV_ISP_YHIST_GetDumpStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_GetDumpStatus()\n", u8IspIdx);
        LOG_INFO("bDumpStatus = 0x%01X(%s)\n",
            bDumpStatus,
            ((bDumpStatus) ? ("Done") : ("Busy"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

STF_RESULT Test_YHIST_GetAccValidStatus(
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
    u32Offset = ADDR_REG_YHIST_CFG_3;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_3;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFFFFFB) | ((rand() & 0x01) << 2);
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_CFG_3;
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
        STF_BOOL8 bAccValidStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bAccValidStatus = STFDRV_ISP_YHIST_GetAccValidStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_GetAccValidStatus()\n", u8IspIdx);
        LOG_INFO("bAccValidStatus = 0x%01X(%s)\n",
            bAccValidStatus,
            ((bAccValidStatus) ? ("Vaild") : ("Not Vaild"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_YHIST)
STF_RESULT Test_YHIST_GetAccResult(
    STF_VOID
    )
{
#if 0
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[Y_HIST_ITEM_SIZE];
#endif
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if 0
    u32Offset = ADDR_REG_YHIST_ACC_0;
    u32Length = Y_HIST_ITEM_SIZE;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //for (i = 0; i < u32Length; i++) {
    //    LOG_INFO("Reg[0x%08X] = 0x%08X\n",
    //        ADDR_REG_YHIST_ACC_0+i*4, u32Buffer[i]);
    //}
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_ACC_0;
    u32Length = Y_HIST_ITEM_SIZE;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i] = (rand() & 0x0003FFFF);
    }
    ret = STFLIB_ISP_RegWriteSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] <- 0x%08X\n",
            ADDR_REG_YHIST_ACC_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YHIST_ACC_0;
    u32Length = Y_HIST_ITEM_SIZE;
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
            ADDR_REG_YHIST_ACC_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
#endif
    {
        //STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_YHIST_RESULT_PARAM stIspYHistResult;

        stIspYHistResult.u8IspIdx = 0;
        ret = STFDRV_ISP_YHIST_GetAccResult(
            pstCiConnection,
            &stIspYHistResult
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YHIST_GetAccResult()\n",
            stIspYHistResult.u8IspIdx);
        u32Cnt = Y_HIST_ITEM_SIZE;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("Y_Hist[%d] = 0x%08X\n",
                u32Idx,
                stIspYHistResult.stYHistResult.u32Accumulate[u32Idx]
                );
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_YHist_Test(
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
        ret = Test_YHIST_SetReg(
            );
        LOG_INFO("Test YHIST_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_YHIST)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_SetStartLocation(
            );
        LOG_INFO("Test YHIST_SetStartLocation command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_SetWindowSize(
            );
        LOG_INFO("Test YHIST_SetWindowSize command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_SetDecimation(
            );
        LOG_INFO("Test YHIST_SetDecimation command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_SetMuxSel(
            );
        LOG_INFO("Test YHIST_SetMuxSel command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#if defined(V4L2_DRIVER)
#else
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_SetDumpBufAddr(
            );
        LOG_INFO("Test YHIST_SetDumpBufAddr command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(V4L2_DRIVER)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_GetDumpStatus(
            );
        LOG_INFO("Test YHIST_GetDumpStatus command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_GetAccValidStatus(
            );
        LOG_INFO("Test YHIST_GetAccValidStatus command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_YHIST)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_YHIST_GetAccResult(
            );
        LOG_INFO("Test YHIST_GetAccResult command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YHIST module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
