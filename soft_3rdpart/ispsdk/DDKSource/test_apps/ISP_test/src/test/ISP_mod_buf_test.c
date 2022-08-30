/**
  ******************************************************************************
  * @file  ISP_mod_buf_test.c
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

#define LOG_TAG "Mod_Buf_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_buf_test.h"
#include "ISP_test.h"


/* ISPC BUF driver test structure */


/* ISPC BUF driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC BUF driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_BUF_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_BUF_REG_LEN * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = ST_BUF_REG_LEN;
    //=== Godspeed === Add new memory/buffer type support here.
    i = 0;
    u32Buffer[i * 2] = ADDR_REG_UOAY;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_UOAUV;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS0AY;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS0AUV;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS1AY;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS1AUV;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_DUMP_CFG_0;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDRYSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDRUSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDWYSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDWUSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SCD_CFG_0;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_YHIST_CFG_4;
    u32Buffer[i * 2 + 1] = 0;
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
        ST_ISP_BUF_RDMA stIspBufRdma;

        stIspBufRdma.stBufRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaStartCmd.RegAddr = ADDR_REG_UOAY;
        stIspBufRdma.stBufRdma.stRdmaStartCmd.u32Param = ST_BUF_UO_REG_LEN;
        stIspBufRdma.stBufRdma.stRdmaBufSs0Cmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaBufSs0Cmd.RegAddr = ADDR_REG_SS0AY;
        stIspBufRdma.stBufRdma.stRdmaBufSs0Cmd.u32Param = ST_BUF_SS0_REG_LEN;
        stIspBufRdma.stBufRdma.stRdmaBufSs1Cmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaBufSs1Cmd.RegAddr = ADDR_REG_SS1AY;
        stIspBufRdma.stBufRdma.stRdmaBufSs1Cmd.u32Param = ST_BUF_SS1_REG_LEN;
#if defined(SUPPORT_DUMP_MODULE)
        stIspBufRdma.stBufRdma.stRdmaBufDumpCmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaBufDumpCmd.RegAddr = ADDR_REG_DUMP_CFG_0;
        stIspBufRdma.stBufRdma.stRdmaBufDumpCmd.u32Param = ST_BUF_DUMP_REG_LEN;
#endif //#if defined(SUPPORT_DUMP_MODULE)
        stIspBufRdma.stBufRdma.stRdmaBufTil_1_RdCmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaBufTil_1_RdCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspBufRdma.stBufRdma.stRdmaBufTil_1_RdCmd.u32Param =
            ST_BUF_TIL_1_RD_REG_LEN;
        stIspBufRdma.stBufRdma.stRdmaBufTil_1_WrCmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaBufTil_1_WrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspBufRdma.stBufRdma.stRdmaBufTil_1_WrCmd.u32Param =
            ST_BUF_TIL_1_WR_REG_LEN;
        stIspBufRdma.stBufRdma.stRdmaBufScDumpCmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaBufScDumpCmd.RegAddr = ADDR_REG_SCD_CFG_0;
        stIspBufRdma.stBufRdma.stRdmaBufScDumpCmd.u32Param =
            ST_BUF_SC_DUMP_REG_LEN;
        stIspBufRdma.stBufRdma.stRdmaBufYHistCmd.Tag = MREG_WR;
        stIspBufRdma.stBufRdma.stRdmaBufYHistCmd.RegAddr = ADDR_REG_YHIST_CFG_4;
        stIspBufRdma.stBufRdma.stRdmaBufYHistCmd.u32Param =
            ST_BUF_YHIST_REG_LEN;
        stIspBufRdma.stBufRdma.stRdmaEndCmd.Tag = DESP_END;

        //=== Godspeed === Add new memory/buffer type support here.
        stIspBufRdma.stBufRdma.stBufUoReg.UoYAddr.value = (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufUoReg.UoUvAddr.value = (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufSs0Reg.Ss0YAddr.value = (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufSs0Reg.Ss0UvAddr.value = (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufSs1Reg.Ss1YAddr.value = (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufSs1Reg.Ss1UvAddr.value = (rand() & 0xFFFFFFF8);
#if defined(SUPPORT_DUMP_MODULE)
        stIspBufRdma.stBufRdma.stBufDumpReg.DumpBufAddr.value =
            (rand() & 0xFFFFFF80);
#endif //#if defined(SUPPORT_DUMP_MODULE)
        stIspBufRdma.stBufRdma.stBufTil_1_RdReg.Til_1_ReadYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufTil_1_RdReg.Til_1_ReadUvAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufTil_1_WrReg.Til_1_WriteYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufTil_1_WrReg.Til_1_WriteUvAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspBufRdma.stBufRdma.stBufScDumpReg.ScDumpBufAddr.value =
            (rand() & 0xFFFFFF80);
        stIspBufRdma.stBufRdma.stBufYHistReg.YHistBufAddr.value =
            (rand() & 0xFFFFFF80);
        stIspBufRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_BUF_SetReg(
            pstCiConnection,
            &stIspBufRdma
            );
        if (ret) {
            return ret;
        }
        //=== Godspeed === Add new memory/buffer type support here.
        LOG_INFO("ISP_%d : STFDRV_ISP_BUF_SetReg()\n", stIspBufRdma.u8IspIdx);
        LOG_INFO("Uo - YAddr = 0x%08X, UvAddr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufUoReg.UoYAddr.value,
            stIspBufRdma.stBufRdma.stBufUoReg.UoUvAddr.value
            );
        LOG_INFO("--------------------\n");
        LOG_INFO("Ss0 - YAddr = 0x%08X, UvAddr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufSs0Reg.Ss0YAddr.value,
            stIspBufRdma.stBufRdma.stBufSs0Reg.Ss0UvAddr.value
            );
        LOG_INFO("--------------------\n");
        LOG_INFO("Ss1 - YAddr = 0x%08X, UvAddr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufSs1Reg.Ss1YAddr.value,
            stIspBufRdma.stBufRdma.stBufSs1Reg.Ss1UvAddr.value
            );
        LOG_INFO("--------------------\n");
#if defined(SUPPORT_DUMP_MODULE)
        LOG_INFO("Dump - Addr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufDumpReg.DumpBufAddr.value
            );
        LOG_INFO("--------------------\n");
#endif //#if defined(SUPPORT_DUMP_MODULE)
        LOG_INFO("Tiling 1 Read - YAddr = 0x%08X, UvAddr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufTil_1_RdReg.Til_1_ReadYAddr.value,
            stIspBufRdma.stBufRdma.stBufTil_1_RdReg.Til_1_ReadUvAddr.value
            );
        LOG_INFO("--------------------\n");
        LOG_INFO("Tiling 1 Write - YAddr = 0x%08X, UvAddr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufTil_1_WrReg.Til_1_WriteYAddr.value,
            stIspBufRdma.stBufRdma.stBufTil_1_WrReg.Til_1_WriteUvAddr.value
            );
        LOG_INFO("--------------------\n");
        LOG_INFO("ScDump - Addr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufScDumpReg.ScDumpBufAddr.value
            );
        LOG_INFO("--------------------\n");
        LOG_INFO("Y Histogram - Addr = 0x%08X\n",
            stIspBufRdma.stBufRdma.stBufYHistReg.YHistBufAddr.value
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = ST_BUF_REG_LEN;
    //=== Godspeed === Add new memory/buffer type support here.
    i = 0;
    u32Buffer[i * 2] = ADDR_REG_UOAY;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_UOAUV;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS0AY;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS0AUV;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS1AY;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SS1AUV;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_DUMP_CFG_0;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDRYSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDRUSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDWYSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_ITIDWUSAR;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_SCD_CFG_0;
    u32Buffer[i * 2 + 1] = 0;
    i++;
    u32Buffer[i * 2] = ADDR_REG_YHIST_CFG_4;
    u32Buffer[i * 2 + 1] = 0;
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

#if defined(EXTEND_IOCTL_COMMAND_BUF)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_BUF)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Buf_Test(
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
        ret = Test_BUF_SetReg(
            );
        LOG_INFO("Test BUF_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the BUF module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_BUF)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_BUF)

    return ret;
}

//-----------------------------------------------------------------------------
