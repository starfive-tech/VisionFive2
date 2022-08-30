/**
  ******************************************************************************
  * @file  ISP_mod_isp_in_test.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/24/2021
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

#define LOG_TAG "Mod_Isp_In_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_isp_in_test.h"
#include "ISP_test.h"


/* ISPC ISP_IN driver test structure */


/* ISPC ISP_IN driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC ISP_IN driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_ISP_IN_SetReg(
    STF_VOID
    )
{
#if 0
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[1];
#endif
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if 0
    u32Length = 3;
    u32Buffer[0 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_IFSZ;
    u32Buffer[2 * 2 + 1] = 0;
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
#endif
    {
#if 0
        const STF_CHAR szFirstPixel[][4] = {
            "R",
            "B",
            "Gr",
            "Gb",
        };
#endif
        ST_ISP_ISP_IN_RDMA stIspIspInRdma;

        LOG_INFO("stIspInRdma Size = %d\n", sizeof(ST_ISP_IN_RDMA));
        stIspIspInRdma.stIspInRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspIspInRdma.stIspInRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_ISPCTRL0;
        stIspIspInRdma.stIspInRdma.stRdmaStartCmd.u32Param =
            ST_ISP_IN_REG_LEN;
        stIspIspInRdma.stIspInRdma.stRdmaEndCmd.Tag = DESP_END;

#if 0
        stIspIspInRdma.stIspInRdma.stIspInReg.IspCtrl1.field.sat =
            (rand() & 0x03);
        stIspIspInRdma.stIspInRdma.stIspInReg.IspCtrl0.field.enls =
            (rand() & 0x01);
        stIspIspInRdma.stIspInRdma.stIspInReg.IspCaptureSize.field.w =
            (rand() & 0x1FFF);
        stIspIspInRdma.stIspInRdma.stIspInReg.IspCaptureSize.field.h =
            (rand() & 0x1FFF);
#endif
        stIspIspInRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetReg(
            pstCiConnection,
            &stIspIspInRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetReg()\n",
            stIspIspInRdma.u8IspIdx);
#if 0
        LOG_INFO("First Pixel = %d(%s)\n",
            stIspIspInRdma.stIspInRdma.stIspInReg.IspCtrl1.field.sat,
            szFirstPixel[stIspIspInRdma.stIspInRdma.stIspInReg.IspCtrl1.field.sat]
            );
        LOG_INFO("Multi-Frame-Mode = %s\n",
            (stIspIspInRdma.stIspInRdma.stIspInReg.IspCtrl0.field.enls)
                ? ("Enable") : ("Disable")
            );
        LOG_INFO("Capture size = %d(0x%04X) * %d(0x%04X)\n",
            stIspIspInRdma.stIspInRdma.stIspInReg.IspCaptureSize.field.w,
            stIspIspInRdma.stIspInRdma.stIspInReg.IspCaptureSize.field.w,
            stIspIspInRdma.stIspInRdma.stIspInReg.IspCaptureSize.field.h,
            stIspIspInRdma.stIspInRdma.stIspInReg.IspCaptureSize.field.h
            );
#endif
        LOG_INFO("---------------------------------\n");
    }
#if 0
    //-------------------------------------------------------------------------
    u32Length = 3;
    u32Buffer[0 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_IFSZ;
    u32Buffer[2 * 2 + 1] = 0;
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
#endif
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT Test_ISP_IN_SetMultipleFrame(
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
    u32Offset = ADDR_REG_ISPCTRL0;
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
        EN_ISP_MULTI_FRAME_MODE enIspMultiFrameMode = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetMultipleFrame(
            pstCiConnection,
            u8IspIdx,
            enIspMultiFrameMode
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetMultipleFrame()\n", u8IspIdx);
        LOG_INFO("Multiple frame mode = %s\n",
            (enIspMultiFrameMode) ? ("Enable") : ("Disable")
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
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
        EN_ISP_MULTI_FRAME_MODE enIspMultiFrameMode = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetMultipleFrame(
            pstCiConnection,
            u8IspIdx,
            enIspMultiFrameMode
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetMultipleFrame()\n", u8IspIdx);
        LOG_INFO("Multiple frame mode = %s\n",
            (enIspMultiFrameMode) ? ("Enable") : ("Disable")
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
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

STF_RESULT Test_ISP_IN_SetFirstPixel(
    STF_VOID
    )
{
    static STF_CHAR g_szIspFirstPixel[][20] = {
        "EN_ISP_1ST_PXL_R",
        "EN_ISP_1ST_PXL_B",
        "EN_ISP_1ST_PXL_GR",
        "EN_ISP_1ST_PXL_GB",
    };
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ISPCTRL1;
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
        EN_ISP_1ST_PXL enFirstPixel = EN_ISP_1ST_PXL_R;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetFirstPixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetFirstPixel()\n", u8IspIdx);
        LOG_INFO("FirstPixel = %s(%d)\n",
            g_szIspFirstPixel[enFirstPixel],
            enFirstPixel
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL1;
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
        EN_ISP_1ST_PXL enFirstPixel = EN_ISP_1ST_PXL_GR;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetFirstPixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetFirstPixel()\n", u8IspIdx);
        LOG_INFO("FirstPixel = %s(%d)\n",
            g_szIspFirstPixel[enFirstPixel],
            enFirstPixel
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL1;
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
    {
        EN_ISP_1ST_PXL enFirstPixel = EN_ISP_1ST_PXL_GB;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetFirstPixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetFirstPixel()\n", u8IspIdx);
        LOG_INFO("FirstPixel = %s(%d)\n",
            g_szIspFirstPixel[enFirstPixel],
            enFirstPixel
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL1;
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
        EN_ISP_1ST_PXL enFirstPixel = EN_ISP_1ST_PXL_B;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetFirstPixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetFirstPixel()\n", u8IspIdx);
        LOG_INFO("FirstPixel = %s(%d)\n",
            g_szIspFirstPixel[enFirstPixel],
            enFirstPixel
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL1;
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

STF_RESULT Test_ISP_IN_SetCaptureSize(
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
    u32Offset = ADDR_REG_IFSZ;
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
        ST_SIZE stCaptureSize = { 0, 0 };
        STF_U8 u8IspIdx = 0;

        stCaptureSize.u16Cx = (rand() & 0x1FFF);
        stCaptureSize.u16Cy = (rand() & 0x1FFF);

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetCaptureSize(
            pstCiConnection,
            u8IspIdx,
            stCaptureSize
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetCaptureSize()\n", u8IspIdx);
        LOG_INFO("Capture size = %d(0x%04X) * %d(0x%04X)\n",
            stCaptureSize.u16Cx,
            stCaptureSize.u16Cx,
            stCaptureSize.u16Cy,
            stCaptureSize.u16Cy
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IFSZ;
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
        ST_SIZE stCaptureSize = { 0, 0 };
        STF_U8 u8IspIdx = 0;

        stCaptureSize.u16Cx = (rand() & 0x1FFF);
        stCaptureSize.u16Cy = (rand() & 0x1FFF);

        u8IspIdx = 0;
        ret = STFDRV_ISP_ISP_IN_SetCaptureSize(
            pstCiConnection,
            u8IspIdx,
            stCaptureSize
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_ISP_IN_SetCaptureSize()\n", u8IspIdx);
        LOG_INFO("Capture size = %d(0x%04X) * %d(0x%04X)\n",
            stCaptureSize.u16Cx,
            stCaptureSize.u16Cx,
            stCaptureSize.u16Cy,
            stCaptureSize.u16Cy
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IFSZ;
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

//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Isp_In_Test(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    g_pstCiConnection = (CI_CONNECTION *)STFLIB_ISP_BASE_GetConnection(
        TEST_ISP_DEVICE
        );

#if 1
    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_ISP_IN_SetReg(
            );
        LOG_INFO("Test ISP_IN_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP_IN module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_ISP_IN_SetMultipleFrame(
            );
        LOG_INFO("Test ISP_IN_SetMultipleFrame command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP_IN module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_ISP_IN_SetFirstPixel(
            );
        LOG_INFO("Test ISP_IN_SetFirstPixel command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP_IN module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_ISP_IN_SetCaptureSize(
            );
        LOG_INFO("Test ISP_IN_SetCaptureSize command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP_IN module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#else
    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    LOG_INFO("The ISP_IN module doesn't has any registers needs to "\
        "program and test.\n");
    LOG_INFO("---------------------------------\n");
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif

    return ret;
}

//-----------------------------------------------------------------------------
