/**
  ******************************************************************************
  * @file  ISP_mod_r2y_test.c
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

#define LOG_TAG "Mod_R2y_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_r2y_test.h"
#include "ISP_test.h"


/* ISPC R2Y driver test structure */


/* ISPC R2Y driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC R2Y driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_R2Y_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[9];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ICCONV_0;
    u32Length = 9;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_IGRVAL_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_R2Y_RDMA stIspR2yRdma;

        stIspR2yRdma.stR2yRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspR2yRdma.stR2yRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ICCONV_0;
        stIspR2yRdma.stR2yRdma.stRdmaStartCmd.u32Param = ST_R2Y_REG_LEN;
        stIspR2yRdma.stR2yRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 9;
        pu32Temp = (STF_U32 *)&stIspR2yRdma.stR2yRdma.stR2yReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0x1FF);
        }
        stIspR2yRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_R2Y_SetReg(
            pstCiConnection,
            &stIspR2yRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_R2Y_SetReg()\n", stIspR2yRdma.u8IspIdx);
        LOG_INFO("Matrix[0][] = [ 0x%04X, 0x%04X, 0x%04X]\n",
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[0][0],
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[0][1],
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[0][2]
            );
        LOG_INFO("Matrix[1][] = [ 0x%04X, 0x%04X, 0x%04X]\n",
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[1][0],
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[1][1],
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[1][2]
            );
        LOG_INFO("Matrix[2][] = [ 0x%04X, 0x%04X, 0x%04X]\n",
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[2][0],
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[2][1],
            stIspR2yRdma.stR2yRdma.stR2yReg.Matrix[2][2]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICCONV_0;
    u32Length = 9;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_IGRVAL_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_R2Y)
//-----------------------------------------------------------------------------
STF_RESULT Test_R2Y_SetColorMatrix(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[9];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ICCONV_0;
    u32Length = 9;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_IGRVAL_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_R2Y_MATRIX_PARAM stR2yMatrix;

        u32Cnt = 9;
        pu16Temp = (STF_U16 *)&stR2yMatrix.stR2yMatrix;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x1FF);
        }
        stR2yMatrix.u8IspIdx = 0;
        ret = STFDRV_ISP_R2Y_SetColorMatrix(
            pstCiConnection,
            &stR2yMatrix
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_R2Y_SetColorMatrix()\n",
            stR2yMatrix.u8IspIdx);
        LOG_INFO("Matrix[0][] = [ 0x%04X, 0x%04X, 0x%04X]\n",
            stR2yMatrix.stR2yMatrix.au16Matrix[0][0],
            stR2yMatrix.stR2yMatrix.au16Matrix[0][1],
            stR2yMatrix.stR2yMatrix.au16Matrix[0][2]
            );
        LOG_INFO("Matrix[1][] = [ 0x%04X, 0x%04X, 0x%04X]\n",
            stR2yMatrix.stR2yMatrix.au16Matrix[1][0],
            stR2yMatrix.stR2yMatrix.au16Matrix[1][1],
            stR2yMatrix.stR2yMatrix.au16Matrix[1][2]
            );
        LOG_INFO("Matrix[2][] = [ 0x%04X, 0x%04X, 0x%04X]\n",
            stR2yMatrix.stR2yMatrix.au16Matrix[2][0],
            stR2yMatrix.stR2yMatrix.au16Matrix[2][1],
            stR2yMatrix.stR2yMatrix.au16Matrix[2][2]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICCONV_0;
    u32Length = 9;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_IGRVAL_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_R2Y)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_R2y_Test(
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
        ret = Test_R2Y_SetReg(
            );
        LOG_INFO("Test R2Y_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the R2Y module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_R2Y)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_R2Y_SetColorMatrix(
            );
        LOG_INFO("Test R2Y_SetColorMatrix command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the R2Y module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_R2Y)

    return ret;
}

//-----------------------------------------------------------------------------
