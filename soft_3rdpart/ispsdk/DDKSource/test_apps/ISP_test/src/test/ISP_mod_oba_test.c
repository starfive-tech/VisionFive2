/**
  ******************************************************************************
  * @file  ISP_mod_oba_test.c
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

#define LOG_TAG "Mod_Oba_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_oba_test.h"
#include "ISP_test.h"


/* ISPC OBA driver test structure */


/* ISPC OBA driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC OBA driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_OBA_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OBA_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OBA_CFG_0;
    u32Length = ST_OBA_REG_LEN;
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
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OBA_RDMA stIspObaRdma;

        stIspObaRdma.stObaRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspObaRdma.stObaRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_OBA_CFG_0;
        stIspObaRdma.stObaRdma.stRdmaStartCmd.u32Param =
            ST_OBA_REG_LEN;
        stIspObaRdma.stObaRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspObaRdma.stObaRdma.stObaReg.TopLeft.value = (rand() & 0xFFFF)
            | ((rand() & 0xFFFF) << 16);
        stIspObaRdma.stObaRdma.stObaReg.BottomRight.value = (rand() & 0xFFFF)
            | ((rand() & 0xFFFF) << 16);
        stIspObaRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OBA_SetReg(
            pstCiConnection,
            &stIspObaRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OBA_SetReg()\n", stIspObaRdma.u8IspIdx);
        LOG_INFO("HStart = 0x%04X(%d), VStart = 0x%04X(%d), "\
            "HEnd = 0x%04X(%d), VEnd = 0x%04X(%d)\n",
            stIspObaRdma.stObaRdma.stObaReg.TopLeft.field.hstart,
            stIspObaRdma.stObaRdma.stObaReg.TopLeft.field.hstart,
            stIspObaRdma.stObaRdma.stObaReg.TopLeft.field.vstart,
            stIspObaRdma.stObaRdma.stObaReg.TopLeft.field.vstart,
            stIspObaRdma.stObaRdma.stObaReg.BottomRight.field.hend,
            stIspObaRdma.stObaRdma.stObaReg.BottomRight.field.hend,
            stIspObaRdma.stObaRdma.stObaReg.BottomRight.field.vend,
            stIspObaRdma.stObaRdma.stObaReg.BottomRight.field.vend
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OBA_CFG_0;
    u32Length = ST_OBA_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_OBA_CFG_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_OBA_GetAcc(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OBA_ACC_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OBA_ACC_0;
    u32Length = ST_OBA_ACC_REG_LEN;
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
        LOG_INFO("Read Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_OBA_ACC_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OBA_ACC_0;
    u32Length = ST_OBA_ACC_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i] = (rand() & 0xFFFFFFFF);
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
        LOG_INFO("Write Reg[0x%08X] <- 0x%08X\n",
            ADDR_REG_OBA_ACC_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U16 *pu16Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_OBA_ACC_PARAM stIspObaAcc = {
            0,
            {
                0,
                0,
                0,
                0,
            },
        };

        stIspObaAcc.u8IspIdx = 0;
        stIspObaAcc.stObaAcc = STFDRV_ISP_OBA_GetAcc(
            pstCiConnection,
            stIspObaAcc.u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_OBA_GetAcc()\n", stIspObaAcc.u8IspIdx);
        LOG_INFO("R_Acc = 0x%08X, Gr_Acc = 0x%08X, "\
            "Gb_Acc = 0x%08X, B_Acc = 0x%08X\n",
            stIspObaAcc.stObaAcc.u32RAcc,
            stIspObaAcc.stObaAcc.u32GrAcc,
            stIspObaAcc.stObaAcc.u32GbAcc,
            stIspObaAcc.stObaAcc.u32BAcc
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_OBA)
//-----------------------------------------------------------------------------
STF_RESULT Test_OBA_SetBlackCropping(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OBA_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OBA_CFG_0;
    u32Length = ST_OBA_REG_LEN;
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
        ST_ISP_OBA_CROP_PARAM stIspObaCrop;

        stIspObaCrop.stCrop.u16HStart = (rand() & 0xFFFF);
        stIspObaCrop.stCrop.u16VStart = (rand() & 0xFFFF);
        stIspObaCrop.stCrop.u16HEnd = (rand() & 0xFFFF);
        stIspObaCrop.stCrop.u16VEnd = (rand() & 0xFFFF);
        stIspObaCrop.u8IspIdx = 0;
        ret = STFDRV_ISP_OBA_SetBlackCropping(
            pstCiConnection,
            &stIspObaCrop
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OBA_SetBlackCropping()\n",
            stIspObaCrop.u8IspIdx);
        LOG_INFO("HStart = 0x%04X(%d), VStart = 0x%04X(%d), "\
            "HEnd = 0x%04X(%d), VEnd = 0x%04X(%d)\n",
            stIspObaCrop.stCrop.u16HStart,
            stIspObaCrop.stCrop.u16HStart,
            stIspObaCrop.stCrop.u16VStart,
            stIspObaCrop.stCrop.u16VStart,
            stIspObaCrop.stCrop.u16HEnd,
            stIspObaCrop.stCrop.u16HEnd,
            stIspObaCrop.stCrop.u16VEnd,
            stIspObaCrop.stCrop.u16VEnd
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OBA_CFG_0;
    u32Length = ST_OBA_REG_LEN;
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

STF_RESULT Test_OBA_GetAccumulation(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OBA_ACC_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_OBA_ACC_0;
    u32Length = ST_OBA_ACC_REG_LEN;
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
        LOG_INFO("Read Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_OBA_ACC_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_OBA_ACC_0;
    u32Length = ST_OBA_ACC_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[i] = (rand() & 0xFFFFFFFF);
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
        LOG_INFO("Write Reg[0x%08X] <- 0x%08X\n",
            ADDR_REG_OBA_ACC_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U16 *pu16Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_OBA_ACCUMULATION_PARAM stIspObaAccumulation = {
            0,
            {
                6,
                7,
                8,
                9,
            },
        };

        stIspObaAccumulation.u8IspIdx = 0;
        ret = STFDRV_ISP_OBA_GetAccumulation(
            pstCiConnection,
            &stIspObaAccumulation
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OBA_GetAccumulation()\n",
            stIspObaAccumulation.u8IspIdx);
        LOG_INFO("R_Acc = 0x%08X, Gr_Acc = 0x%08X, "\
            "Gb_Acc = 0x%08X, B_Acc = 0x%08X\n",
            stIspObaAccumulation.au32Acc[0],
            stIspObaAccumulation.au32Acc[1],
            stIspObaAccumulation.au32Acc[2],
            stIspObaAccumulation.au32Acc[3]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_OBA)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Oba_Test(
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
        ret = Test_OBA_SetReg(
            );
        LOG_INFO("Test OBA_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBA module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OBA_GetAcc(
            );
        LOG_INFO("Test OBA_GetAcc command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBA module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OBA)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OBA_SetBlackCropping(
            );
        LOG_INFO("Test OBA_SetBlackCropping command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBA module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OBA_GetAccumulation(
            );
        LOG_INFO("Test OBA_GetAccumulation command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OBA module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OBA)

    return ret;
}

//-----------------------------------------------------------------------------
