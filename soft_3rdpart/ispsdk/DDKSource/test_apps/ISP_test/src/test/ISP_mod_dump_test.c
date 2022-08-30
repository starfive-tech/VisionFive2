/**
  ******************************************************************************
  * @file  ISP_mod_dump_test.c
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

#define LOG_TAG "Mod_Dump_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_dump_test.h"
#include "ISP_test.h"


/* ISPC DUMP driver test structure */


/* ISPC DUMP driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC DUMP driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_DUMP_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_DUMP_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_DUMP_CFG_1;
#else
    u32Offset = ADDR_REG_DUMP_CFG_0;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_DUMP_REG_LEN;
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
            u32Offset+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_DUMP_RDMA stIspDumpRdma;

        stIspDumpRdma.stDumpRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspDumpRdma.stDumpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_DUMP_CFG_1;
#else
        stIspDumpRdma.stDumpRdma.stRdmaStartCmd.RegAddr = ADDR_REG_DUMP_CFG_0;
#endif //#if defined(V4L2_DRIVER)
        stIspDumpRdma.stDumpRdma.stRdmaStartCmd.u32Param = ST_DUMP_REG_LEN;
        stIspDumpRdma.stDumpRdma.stRdmaEndCmd.Tag = DESP_END;

#if defined(V4L2_DRIVER)
#else
        stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg0.value =
            (rand() & 0xFFFFFF80);
#endif //#if defined(V4L2_DRIVER)
        stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.value = 0;
        stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.sd =
            ((rand() & 0xFF80) >> 7);
        stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.burst_len = (rand()
            & 0x3);
        stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.sht = (rand() & 0x7);
        stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.id = (rand() & 0xFF);
        stIspDumpRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_DUMP_SetReg(
            pstCiConnection,
            &stIspDumpRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DUMP_SetReg()\n", stIspDumpRdma.u8IspIdx);
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("Buffer Addr = 0x%08X\n",
            stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg0.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Stride = 0x%04X\n",
            (stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.sd << 7)
            );
        LOG_INFO("Burst Len = 0x%01X\n",
            stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.burst_len
            );
        LOG_INFO("Pixel right right number = 0x%01X(%d)\n",
            stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.sht,
            stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.sht
            );
        LOG_INFO("AxiId = 0x%02X\n",
            stIspDumpRdma.stDumpRdma.stDumpReg.DumpCfg1.field.id
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_DUMP_CFG_1;
#else
    u32Offset = ADDR_REG_DUMP_CFG_0;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_DUMP_REG_LEN;
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
            u32Offset+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_DUMP)
//-----------------------------------------------------------------------------
STF_RESULT Test_DUMP_SetBufAddr(
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
    u32Offset = ADDR_REG_DUMP_CFG_0;
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
        STF_U32 u32BufAddr = (rand() & 0xFFFFFF80);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_DUMP_SetBufAddr(
            pstCiConnection,
            u8IspIdx,
            u32BufAddr
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DUMP_SetBufAddr()\n", u8IspIdx);
        LOG_INFO("Buffer Addr = 0x%08X\n",
            u32BufAddr
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DUMP_CFG_0;
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

STF_RESULT Test_DUMP_SetStride(
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
    u32Offset = ADDR_REG_DUMP_CFG_1;
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
        STF_U16 u16Stride = (rand() & 0xFF80);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_DUMP_SetStride(
            pstCiConnection,
            u8IspIdx,
            u16Stride
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DUMP_SetStride()\n", u8IspIdx);
        LOG_INFO("Stride = 0x%04X\n",
            u16Stride
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DUMP_CFG_1;
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

STF_RESULT Test_DUMP_SetBurstLen(
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
    u32Offset = ADDR_REG_DUMP_CFG_1;
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
        STF_U8 u8BurstLen = (rand() & 0x3);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_DUMP_SetBurstLen(
            pstCiConnection,
            u8IspIdx,
            u8BurstLen
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DUMP_SetBurstLen()\n", u8IspIdx);
        LOG_INFO("Burst Len = 0x%01X\n",
            u8BurstLen
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DUMP_CFG_1;
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

STF_RESULT Test_DUMP_SetPixelRightShift(
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
    u32Offset = ADDR_REG_DUMP_CFG_1;
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
        STF_U8 u8Shift = (rand() & 0x7);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_DUMP_SetPixelRightShift(
            pstCiConnection,
            u8IspIdx,
            u8Shift
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DUMP_SetPixelRightShift()\n", u8IspIdx);
        LOG_INFO("Pixel right right number = 0x%01X(%d)\n",
            u8Shift,
            u8Shift
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DUMP_CFG_1;
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

STF_RESULT Test_DUMP_SetAxiId(
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
    u32Offset = ADDR_REG_DUMP_CFG_1;
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
        STF_U8 u8AxiId = (rand() & 0xFF);
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_DUMP_SetAxiId(
            pstCiConnection,
            u8IspIdx,
            u8AxiId
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DUMP_SetAxiId()\n", u8IspIdx);
        LOG_INFO("AxiId = 0x%02X\n",
            u8AxiId
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DUMP_CFG_1;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_DUMP)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Dump_Test(
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
        ret = Test_DUMP_SetReg(
            );
        LOG_INFO("Test DUMP_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DUMP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_DUMP)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_DUMP_SetBufAddr(
            );
        LOG_INFO("Test DUMP_SetBufAddr command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DUMP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_DUMP_SetStride(
            );
        LOG_INFO("Test DUMP_SetStride command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DUMP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_DUMP_SetBurstLen(
            );
        LOG_INFO("Test DUMP_SetBurstLen command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DUMP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_DUMP_SetPixelRightShift(
            );
        LOG_INFO("Test DUMP_SetPixelRightShift command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DUMP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_DUMP_SetAxiId(
            );
        LOG_INFO("Test DUMP_SetAxiId command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DUMP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_DUMP)

    return ret;
}

//-----------------------------------------------------------------------------
