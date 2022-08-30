/**
  ******************************************************************************
  * @file  ISP_mod_crop_test.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/23/2019
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

#define LOG_TAG "Mod_Crop_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_crop_test.h"
#include "ISP_test.h"


/* ISPC CROP driver test structure */


/* ISPC CROP driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC CROP driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_CROP_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[1 * 2 + 1] = 0;
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] <- 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        ST_ISP_CROP_RDMA stIspCropRdma = {
            0,
            {
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
                0x00000000,
                0x043707FF,
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
            }
        };
        STF_U16 u16Temp;

        stIspCropRdma.stCropRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspCropRdma.stCropRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_CAPTURE_WINDOW_START;
        stIspCropRdma.stCropRdma.stRdmaStartCmd.u32Param =
            ST_CROP_REG_LEN;
        stIspCropRdma.stCropRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.hstart =
            (rand() & 0xFFFF);
        stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.vstart =
            (rand() & 0xFFFF);
        stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.hend =
            (rand() & 0xFFFF);
        stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.vend =
            (rand() & 0xFFFF);
        if (stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.hstart
            > stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.hend) {
            u16Temp =
                stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.hstart;
            stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.hstart =
                stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.hend;
            stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.hend = u16Temp;
        }
        if (stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.vstart
            > stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.vend) {
            u16Temp =
                stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.vstart;
            stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.vstart =
                stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.vend;
            stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.vend = u16Temp;
        }
        stIspCropRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CROP_SetReg(
            pstCiConnection,
            &stIspCropRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CTC_SetReg()\n", stIspCropRdma.u8IspIdx);
        LOG_INFO("CaptureWindow: hstart = %d(0x%04X), vstart = %d(0x%04X), "\
            "hend = %d(0x%04X), vend = %d(0x%04X) }\n",
            stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.hstart,
            stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.hstart,
            stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.vstart,
            stIspCropRdma.stCropRdma.stCropReg.CapWinStart.field.vstart,
            stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.hend,
            stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.hend,
            stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.vend,
            stIspCropRdma.stCropRdma.stCropReg.CapWinEnd.field.vend
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[1 * 2 + 1] = 0;
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

#if defined(EXTEND_IOCTL_COMMAND_CROP)
//-----------------------------------------------------------------------------
STF_RESULT Test_CROP_SetCaptureWindow(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[1 * 2 + 1] = 0;
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
        ST_CAPTURE_WINDOW stCaptureWindow = {
            0,
            {
                0,
                0,
                1919,
                1079,
            }
        };
        STF_U16 u16Temp;

        stCaptureWindow.u8IspIdx = 0;
        stCaptureWindow.stRect.u16Left = (rand() & 0xFFFF);
        stCaptureWindow.stRect.u16Top = (rand() & 0xFFFF);
        stCaptureWindow.stRect.u16Right = (rand() & 0xFFFF);
        stCaptureWindow.stRect.u16Bottom = (rand() & 0xFFFF);
        if (stCaptureWindow.stRect.u16Left > stCaptureWindow.stRect.u16Right) {
            u16Temp = stCaptureWindow.stRect.u16Left;
            stCaptureWindow.stRect.u16Left = stCaptureWindow.stRect.u16Right;
            stCaptureWindow.stRect.u16Right = u16Temp;
        }
        if (stCaptureWindow.stRect.u16Top > stCaptureWindow.stRect.u16Bottom) {
            u16Temp = stCaptureWindow.stRect.u16Top;
            stCaptureWindow.stRect.u16Top = stCaptureWindow.stRect.u16Bottom;
            stCaptureWindow.stRect.u16Bottom = u16Temp;
        }
        ret = STFDRV_ISP_CROP_SetCaptureWindow(
            pstCiConnection,
            &stCaptureWindow
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CROP_SetCaptureWindow() stRect = { "\
            "%d(0x%04X), %d(0x%04X), %d(0x%04X), %d(0x%04X) }\n",
            stCaptureWindow.u8IspIdx,
            stCaptureWindow.stRect.u16Left,
            stCaptureWindow.stRect.u16Left,
            stCaptureWindow.stRect.u16Top,
            stCaptureWindow.stRect.u16Top,
            stCaptureWindow.stRect.u16Right,
            stCaptureWindow.stRect.u16Right,
            stCaptureWindow.stRect.u16Bottom,
            stCaptureWindow.stRect.u16Bottom
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[1 * 2 + 1] = 0;
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
        ST_CAPTURE_WINDOW stCaptureWindow = {
            0,
            {
                0,
                0,
                1919,
                1079,
            }
        };

        stCaptureWindow.u8IspIdx = 0;
        ret = STFDRV_ISP_CROP_SetCaptureWindow(
            pstCiConnection,
            &stCaptureWindow
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CROP_SetCaptureWindow() stRect = { "\
            "%d(0x%04X), %d(0x%04X), %d(0x%04X), %d(0x%04X) }\n",
            stCaptureWindow.u8IspIdx,
            stCaptureWindow.stRect.u16Left,
            stCaptureWindow.stRect.u16Left,
            stCaptureWindow.stRect.u16Top,
            stCaptureWindow.stRect.u16Top,
            stCaptureWindow.stRect.u16Right,
            stCaptureWindow.stRect.u16Right,
            stCaptureWindow.stRect.u16Bottom,
            stCaptureWindow.stRect.u16Bottom
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[1 * 2 + 1] = 0;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_CROP)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Crop_Test(
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
        ret = Test_CROP_SetReg(
            );
        LOG_INFO("Test Test_CROP_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CROP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_CROP)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_CROP_SetCaptureWindow(
            );
        LOG_INFO("Test CROP_SetCaptureWindow command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CROP module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_CROP)

    return ret;
}

//-----------------------------------------------------------------------------
