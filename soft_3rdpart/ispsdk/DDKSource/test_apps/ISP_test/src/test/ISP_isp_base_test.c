/**
  ******************************************************************************
  * @file  ISP_isp_base_test.c
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

#define LOG_TAG "Isp_Base_Test"
#include "stf_common/userlog.h"

#include "ISP_isp_base_test.h"
#include "ISP_test.h"


/* ISPC ISP base driver test structure */


/* ISPC ISP base driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;

#if 0
static STF_U64 g_u64ModuleMask[] = {
    ISP_MOD_SUD_OFF,
    ISP_MOD_DC, ISP_MOD_DEC, ISP_MOD_OBC, ISP_MOD_LCBQ,
    ISP_MOD_OECF, ISP_MOD_OECFHM, ISP_MOD_LCCF, ISP_MOD_AWB,
    ISP_MOD_OBA, ISP_MOD_SC, ISP_MOD_DUMP, ISP_MOD_DBC,
    ISP_MOD_CTC, ISP_MOD_CFA, ISP_MOD_CAR, ISP_MOD_CCM,
    ISP_MOD_GMARGB, ISP_MOD_R2Y, ISP_MOD_YCRV, ISP_MOD_SHRP,
    ISP_MOD_DNYUV, ISP_MOD_SAT, ISP_MOD_YHIST, ISP_MOD_OUT,
    ISP_MOD_UO, ISP_MOD_SS0, ISP_MOD_SS1, ISP_MOD_TIL,
    ISP_MOD_TIL_1_RD, ISP_MOD_TIL_1_WR, ISP_MOD_SUD, ISP_MOD_SUD_CSI,
    ISP_MOD_SUD_ISP, ISP_MOD_CSI, ISP_MOD_ISP, ISP_MOD_CBAR,
    ISP_MOD_BUF,
};

static STF_CHAR g_szModuleMask[][20] = {
    "ISP_MOD_SUD_OFF",
    "ISP_MOD_DC", "ISP_MOD_DEC", "ISP_MOD_OBC", "ISP_MOD_LCBQ",
    "ISP_MOD_OECF", "ISP_MOD_OECFHM", "ISP_MOD_LCCF", "ISP_MOD_AWB",
    "ISP_MOD_OBA", "ISP_MOD_SC", "ISP_MOD_DUMP", "ISP_MOD_DBC",
    "ISP_MOD_CTC", "ISP_MOD_CFA", "ISP_MOD_CAR", "ISP_MOD_CCM",
    "ISP_MOD_GMARGB", "ISP_MOD_R2Y", "ISP_MOD_YCRV", "ISP_MOD_SHRP",
    "ISP_MOD_DNYUV", "ISP_MOD_SAT", "ISP_MOD_YHIST", "ISP_MOD_OUT",
    "ISP_MOD_UO", "ISP_MOD_SS0", "ISP_MOD_SS1", "ISP_MOD_TIL",
    "ISP_MOD_TIL_1_RD", "ISP_MOD_TIL_1_WR",  "ISP_MOD_SUD", "ISP_MOD_SUD_CSI",
    "ISP_MOD_SUD_ISP", "ISP_MOD_CSI", "ISP_MOD_ISP", "ISP_MOD_CBAR",
    "ISP_MOD_BUF"
};

#endif
static STF_CHAR g_szIspFirstPixel[][20] = {
    "EN_ISP_1ST_PXL_R",
    "EN_ISP_1ST_PXL_B",
    "EN_ISP_1ST_PXL_GR",
    "EN_ISP_1ST_PXL_GB",
};


/* ISPC ISP base driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_SetCSIEnable(
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= 0xFFFFFFFE;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bEnable = STF_ENABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetCsiEnable(
            pstCiConnection,
            u8IspIdx,
            bEnable
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiEnable(), bEnable = %s\n",
            u8IspIdx,
            ((bEnable) ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
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
    {
        STF_BOOL8 bEnable = STF_DISABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetCsiEnable(
            pstCiConnection,
            u8IspIdx,
            bEnable
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiEnable(), bEnable = %s\n",
            u8IspIdx,
            ((bEnable) ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
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

#if defined(EXTEND_IOCTL_COMMAND_BASE)
STF_RESULT Test_IsCSIBusy(
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= 0x00000002;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCSIBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCSIBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= 0xFFFFFFFD;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCSIBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCSIBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsScdDone(
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= 0x00000004;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsScdDone(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsScdDone(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Busy"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= 0xFFFFFFFB;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsScdDone(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsScdDone(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Busy"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsScdError(
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= 0x00000040;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsScdError(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsScdError(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= 0xFFFFFFBF;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsScdError(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsScdError(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsItu656Field(
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= 0x00000008;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bField = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bField = STFDRV_ISP_BASE_IsItu656Field(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsItu656Field(), bField = %s\n",
            u8IspIdx,
            ((bField) ? ("2") : ("1"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= 0xFFFFFFF7;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bField = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bField = STFDRV_ISP_BASE_IsItu656Field(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsItu656Field(), bField = %s\n",
            u8IspIdx,
            ((bField) ? ("2") : ("1"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsItu656Error(
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= 0x00000010;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsItu656Error(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsItu656Error(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= 0xFFFFFFEF;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsItu656Error(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsItu656Error(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
STF_RESULT Test_GetCsiAllStatus(
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= STS_CSI_BUSY;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= ~STS_CSI_BUSY;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= STS_SCD_DONE;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= ~STS_SCD_DONE;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= STS_ITU656_FIELD_2;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= ~STS_ITU656_FIELD_2;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= STS_ITU656_ERROR;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= ~STS_ITU656_ERROR;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= STS_SCD_ERROR;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= ~STS_SCD_ERROR;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] |= STS_CSI_ALL;
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0] &= (~STS_CSI_ALL);
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
    u32Offset = ADDR_REG_ENABLE_STATUS;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetCsiAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiAllStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("CSI Status=%s, SC Status=%s(%s) ITU656 Field=%s(%s)\n",
            ((u32Status & STS_CSI_BUSY) ? ("Busy") : ("Done")),
            ((u32Status & STS_SCD_DONE) ? ("Done") : ("Busy")),
            ((u32Status & STS_SCD_ERROR) ? ("Error") : ("Successful")),
            ((u32Status & STS_ITU656_FIELD_2) ? ("2") : ("1")),
            ((u32Status & STS_ITU656_ERROR) ? ("Error") : ("Successful"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT Test_GetCSIVCount(
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
    u32Offset = ADDR_REG_VCNT;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_VCNT;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFF0000) | (rand() & 0xFFFF);
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
    {
        STF_U16 u16Count = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u16Count = STFDRV_ISP_BASE_GetCSIVCount(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCSIVCount(), u16Count = 0x%04X\n",
            u8IspIdx,
            u16Count
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_GetCSIHCount(
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
    u32Offset = ADDR_REG_VCNT;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_VCNT;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFF) | ((rand() & 0xffff) << 16);
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
    {
        STF_U16 u16Count = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u16Count = STFDRV_ISP_BASE_GetCSIHCount(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCSIHCount(), u16Count = 0x%04X\n",
            u8IspIdx,
            u16Count
            );
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_SetCsiLineIntLine(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U16 u16Line = 1080;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetCsiLineIntLine(
            pstCiConnection,
            u8IspIdx,
            u16Line
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiLineIntLine(), "\
            "u16Line = %d(0x%04X)\n",
            u8IspIdx,
            u16Line,
            u16Line
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

STF_RESULT Test_GetCsiLineIntLine(
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
    {
        STF_U16 u16Line = 0;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u16Line = STFDRV_ISP_BASE_GetCsiLineIntLine(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetCsiLineIntLine(), "\
            "u16Line = %d(0x%04X)\n",
            u8IspIdx,
            u16Line,
            u16Line
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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
STF_RESULT Test_SetModuleEnable(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[7 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_U64 u64Module = EN_KRN_MOD_ID_DC;
        STF_U64 u64ModuleMask = 0x00000000;
        STF_BOOL8 bEnable = STF_ENABLE;

        if (EN_KRN_MOD_ID_MAX > u64Module) {
            u64ModuleMask = g_u64ModuleMask[u64Module];
            bFound = STF_TRUE;
        }
        if (bFound) {
            STF_U8 u8IspIdx = 0;

            u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModuleEnable(
                pstCiConnection,
                u8IspIdx,
                u64ModuleMask,
                bEnable
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u8IspIdx,
                u64Module,
                u64ModuleMask,
                g_szModuleMask[u64Module],
                ((bEnable) ? ("Enable") : ("Disable"))
                );
        } else {
            LOG_INFO("STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(Unknown), "\
                "bEnable = %s\n",
                u64Module,
                u64ModuleMask,
                ((bEnable) ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_U64 u64Module = EN_KRN_MOD_ID_DC;
        STF_U64 u64ModuleMask = 0x00000000;
        STF_BOOL8 bEnable = STF_DISABLE;

        if (EN_KRN_MOD_ID_MAX > u64Module) {
            u64ModuleMask = g_u64ModuleMask[u64Module];
            bFound = STF_TRUE;
        }
        if (bFound) {
            STF_U8 u8IspIdx = 0;

            u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModuleEnable(
                pstCiConnection,
                u8IspIdx,
                u64ModuleMask,
                bEnable
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u8IspIdx,
                u64Module,
                u64ModuleMask,
                g_szModuleMask[u64Module],
                ((bEnable) ? ("Enable") : ("Disable"))
                );
        } else {
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u64Module,
                u64ModuleMask,
                ((bEnable) ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_U64 u64Module = EN_KRN_MOD_ID_SUD;
        STF_U64 u64ModuleMask = 0x00000000;
        STF_BOOL8 bEnable = STF_ENABLE;

        if (EN_KRN_MOD_ID_MAX > u64Module) {
            u64ModuleMask = g_u64ModuleMask[u64Module];
            bFound = STF_TRUE;
        }
        if (bFound) {
            STF_U8 u8IspIdx = 0;

            u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModuleEnable(
                pstCiConnection,
                u8IspIdx,
                u64ModuleMask,
                bEnable
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u8IspIdx,
                u64Module,
                u64ModuleMask,
                g_szModuleMask[u64Module],
                ((bEnable) ? ("Enable") : ("Disable"))
                );
        } else {
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u64Module,
                u64ModuleMask,
                ((bEnable) ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_U64 u64Module = EN_KRN_MOD_ID_SUD;
        STF_U64 u64ModuleMask = 0x00000000;
        STF_BOOL8 bEnable = STF_DISABLE;

        if (EN_KRN_MOD_ID_MAX > u64Module) {
            u64ModuleMask = g_u64ModuleMask[u64Module];
            bFound = STF_TRUE;
        }
        if (bFound) {
            STF_U8 u8IspIdx = 0;

            u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModuleEnable(
                pstCiConnection,
                u8IspIdx,
                u64ModuleMask,
                bEnable
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u8IspIdx,
                u64Module,
                u64ModuleMask,
                g_szModuleMask[u64Module],
                ((bEnable) ? ("Enable") : ("Disable"))
                );
        } else {
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u64Module,
                u64ModuleMask,
                ((bEnable) ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_U64 u64Module = EN_KRN_MOD_ID_SUD;
        STF_U64 u64ModuleMask = 0x00000000;
        STF_BOOL8 bEnable = STF_ENABLE;

        if (EN_KRN_MOD_ID_MAX > u64Module) {
            u64ModuleMask = g_u64ModuleMask[u64Module];
            bFound = STF_TRUE;
        }
        if (bFound) {
            STF_U8 u8IspIdx = 0;

            u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModuleEnable(
                pstCiConnection,
                u8IspIdx,
                u64ModuleMask,
                bEnable
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u8IspIdx,
                u64Module,
                u64ModuleMask,
                g_szModuleMask[u64Module],
                ((bEnable) ? ("Enable") : ("Disable"))
                );
        } else {
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u64Module,
                u64ModuleMask,
                ((bEnable) ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_U64 u64Module = EN_KRN_MOD_ID_SUD_OFF;
        STF_U64 u64ModuleMask = 0x00000000;
        //STF_BOOL8 bEnable = STF_DISABLE;
        STF_BOOL8 bEnable = STF_ENABLE;

        if (EN_KRN_MOD_ID_MAX > u64Module) {
            u64ModuleMask = g_u64ModuleMask[u64Module];
            bFound = STF_TRUE;
        }
        if (bFound) {
            STF_U8 u8IspIdx = 0;

            u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModuleEnable(
                pstCiConnection,
                u8IspIdx,
                u64ModuleMask,
                bEnable
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u8IspIdx,
                u64Module,
                u64ModuleMask,
                g_szModuleMask[u64Module],
                ((bEnable) ? ("Enable") : ("Disable"))
                );
        } else {
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModuleEnable(), "\
                "u64Module = %d, u64ModuleMask = 0x%016lX(%s), bEnable = %s\n",
                u64Module,
                u64ModuleMask,
                ((bEnable) ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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

STF_RESULT Test_SetModulesEnable(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[7 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_BOOL8 bCount;
        STF_U32 u32Idx;
        ST_ISP_MODULES_EN_PARAM stIspModulesEnableParam;
        STF_CHAR szModules[1024];
        STF_CHAR szTemp[128];

#if !defined(RISCV)
        stIspModulesEnableParam.stModulesEnableParam.u64Modules = EN_KRN_MOD_DEC
            | EN_KRN_MOD_SC | EN_KRN_MOD_R2Y | EN_KRN_MOD_YHIST | EN_KRN_MOD_UO
            | EN_KRN_MOD_TIL_1_WR | EN_KRN_MOD_SUD_CSI | EN_KRN_MOD_SUD_ISP
            | EN_KRN_MOD_CSI | EN_KRN_MOD_ISP;
#else
        stIspModulesEnableParam.stModulesEnableParam.u64Modules = EN_KRN_MOD_DEC
            | EN_KRN_MOD_SC | EN_KRN_MOD_R2Y | EN_KRN_MOD_YHIST | EN_KRN_MOD_UO
            | EN_KRN_MOD_TIL_1_WR | EN_KRN_MOD_SUD_CSI | EN_KRN_MOD_SUD_ISP;
#endif //#if !defined(RISCV)
        stIspModulesEnableParam.stModulesEnableParam.bEnable = STF_ENABLE;
        STF_MEMSET(szModules, 0, sizeof(szModules));
        for (u32Idx = 0, bCount = 0; u32Idx < EN_KRN_MOD_ID_MAX; u32Idx++) {
            if (stIspModulesEnableParam.stModulesEnableParam.u64Modules
                & (1LL << u32Idx)) {
                bCount++;
                if (STF_FALSE == bFound) {
                    sprintf(szTemp, "0x%016lX(%s)",
                        g_u64ModuleMask[u32Idx], g_szModuleMask[u32Idx]);
                } else {
                    if (0 == (bCount % 2)) {
                        sprintf(szTemp, ", 0x%016lX(%s)\n",
                            g_u64ModuleMask[u32Idx], g_szModuleMask[u32Idx]);
                    } else {
                        sprintf(szTemp, ", 0x%016lX(%s)",
                            g_u64ModuleMask[u32Idx], g_szModuleMask[u32Idx]);
                    }
                }
                strcat(szModules, szTemp);
                bFound = STF_TRUE;
            }
        }
        if (bFound) {
            stIspModulesEnableParam.u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModulesEnable(
                pstCiConnection,
                &stIspModulesEnableParam
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModulesEnable(), "\
                "EN_KRN_MOD_ID = 0x%016lX, bEnable = %s\n",
                stIspModulesEnableParam.u8IspIdx,
                stIspModulesEnableParam.stModulesEnableParam.u64Modules,
                ((stIspModulesEnableParam.stModulesEnableParam.bEnable)
                    ? ("Enable") : ("Disable"))
                );
            LOG_INFO("u64ModuleMask = %s\n", szModules);
        } else {
            LOG_INFO("STFDRV_ISP_BASE_SetModuleEnable(), "\
                "EN_KRN_MOD_ID = 0x%016lX, u64ModuleMask = (Unknown), "\
                "bEnable = %s\n",
                stIspModulesEnableParam.stModulesEnableParam.u64Modules,
                ((stIspModulesEnableParam.stModulesEnableParam.bEnable)
                    ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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
        STF_BOOL8 bFound = STF_FALSE;
        STF_BOOL8 bCount;
        STF_U32 u32Idx;
        ST_ISP_MODULES_EN_PARAM stIspModulesEnableParam;
        STF_CHAR szModules[1024];
        STF_CHAR szTemp[128];

#if !defined(RISCV)
        stIspModulesEnableParam.stModulesEnableParam.u64Modules = EN_KRN_MOD_DEC
            | EN_KRN_MOD_SC | EN_KRN_MOD_R2Y | EN_KRN_MOD_YHIST | EN_KRN_MOD_UO
            | EN_KRN_MOD_TIL_1_WR | EN_KRN_MOD_SUD_CSI | EN_KRN_MOD_SUD_ISP
            | EN_KRN_MOD_CSI | EN_KRN_MOD_ISP;
#else
        stIspModulesEnableParam.stModulesEnableParam.u64Modules = EN_KRN_MOD_DEC
            | EN_KRN_MOD_SC | EN_KRN_MOD_R2Y | EN_KRN_MOD_YHIST | EN_KRN_MOD_UO
            | EN_KRN_MOD_TIL_1_WR | EN_KRN_MOD_SUD_CSI | EN_KRN_MOD_SUD_ISP;
#endif //#if !defined(RISCV)
        stIspModulesEnableParam.stModulesEnableParam.bEnable = STF_DISABLE;
        STF_MEMSET(szModules, 0, sizeof(szModules));
        szModules[0] = '0';
        for (u32Idx = 0, bCount = 0; u32Idx < EN_KRN_MOD_ID_MAX; u32Idx++) {
            if (stIspModulesEnableParam.stModulesEnableParam.u64Modules
                & (1LL << u32Idx)) {
                bCount++;
                if (STF_FALSE == bFound) {
                    sprintf(szTemp, "0x%016lX(%s)",
                        g_u64ModuleMask[u32Idx], g_szModuleMask[u32Idx]);
                } else {
                    if (0 == (bCount % 2)) {
                        sprintf(szTemp, ", 0x%016lX(%s)\n",
                            g_u64ModuleMask[u32Idx], g_szModuleMask[u32Idx]);
                    } else {
                        sprintf(szTemp, ", 0x%016lX(%s)",
                            g_u64ModuleMask[u32Idx], g_szModuleMask[u32Idx]);
                    }
                }
                strcat(szModules, szTemp);
                bFound = STF_TRUE;
            }
        }
        if (bFound) {
            stIspModulesEnableParam.u8IspIdx = 0;
            ret = STFDRV_ISP_BASE_SetModulesEnable(
                pstCiConnection,
                &stIspModulesEnableParam
                );
            if (ret) {
                return ret;
            }
            LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetModulesEnable(), "\
                "EN_KRN_MOD_ID = 0x%016lX, bEnable = %s\n",
                stIspModulesEnableParam.u8IspIdx,
                stIspModulesEnableParam.stModulesEnableParam.u64Modules,
                ((stIspModulesEnableParam.stModulesEnableParam.bEnable)
                    ? ("Enable") : ("Disable"))
                );
            LOG_INFO("u64ModuleMask = %s\n", szModules);
        } else {
            LOG_INFO("STFDRV_ISP_BASE_SetModuleEnable(), "\
                "EN_KRN_MOD_ID = 0x%016lX, u64ModuleMask = (Unknown), "\
                "bEnable = %s\n",
                stIspModulesEnableParam.stModulesEnableParam.u64Modules,
                ((stIspModulesEnableParam.stModulesEnableParam.bEnable)
                    ? ("Enable") : ("Disable"))
                );
            ret = STF_ERROR_INVALID_PARAMETERS;
            return ret;
        }
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_FUNCTION_MODE;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_ITIDPSR;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_IESHD;
    u32Buffer[6 * 2 + 1] = 0;
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

//-----------------------------------------------------------------------------
STF_RESULT Test_SetSensorParameter(
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
    u32Offset = ADDR_REG_SENSOR;
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
        EN_IMAGER_SEL enImagerSel = EN_IMAGER_SEL_DVP;
        EN_DVP_ITU656 enItu656 = EN_DVP_ITU656_DISABLE;
        EN_DVP_SYNC_POLARITY enPolarity = EN_DVP_SYNC_POL_H_POS_V_NEG;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetSensorParameter(
            pstCiConnection,
            u8IspIdx,
            enImagerSel,
            enItu656,
            enPolarity
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetSensorParameter(), "\
            "enImagerSel = %d, enItu656 = %d, enPolarity = %d\n",
            u8IspIdx,
            enImagerSel,
            enItu656,
            enPolarity
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_SENSOR;
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

STF_RESULT Test_GetSensorRawFormat(
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
    u32Offset = ADDR_REG_CFA;
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
        STF_U32 u32RawFormat;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32RawFormat = STFDRV_ISP_BASE_GetSensorRawFormat(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetSensorRawFormat(), "\
            "u32RawFormat = 0x%08X\n",
            u8IspIdx,
            u32RawFormat
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_SetSensorRawFormat(
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
    u32Offset = ADDR_REG_CFA;
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
        EN_CFA_RAW_FMT enRawFormat = EN_CFA_RAW_FMT_BGGR;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetSensorRawFormat(
            pstCiConnection,
            u8IspIdx,
            enRawFormat
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetSensorRawFormat(), "\
            "enRawFormat = 0x%08X\n",
            u8IspIdx,
            enRawFormat
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CFA;
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
STF_RESULT Test_GetIspCaptureSize(
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
    u32Buffer[0] = (rand() & 0x1FFF) | ((rand() & 0x1FFF) << 16);
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
    {
        ST_SIZE stSize = { 0, 0 };
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        stSize = STFDRV_ISP_BASE_GetIspCaptureSize(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspCaptureSize(), "\
            "u16Width = 0x%04X, u16Height = 0x%04X\n",
            u8IspIdx,
            stSize.u16Cx,
            stSize.u16Cy
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_SetCsiCaptureWindowAndIspCaptureSize(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[3 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 3;
    u32Buffer[0 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
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
    {
        ST_CAPTURE_WINDOW stCsiCaptureWindow = {
            0,
            {
                0,
                0,
                1919,
                1079,
            }
        };

        stCsiCaptureWindow.u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetCsiCaptureWindowAndIspCaptureSize(
            pstCiConnection,
            &stCsiCaptureWindow
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetCsiCaptureWindowAndIspCaptureSize(), "\
            "stRect = { %d, %d, %d, %d }\n",
            stCsiCaptureWindow.u8IspIdx,
            stCsiCaptureWindow.stRect.u16Left,
            stCsiCaptureWindow.stRect.u16Top,
            stCsiCaptureWindow.stRect.u16Right,
            stCsiCaptureWindow.stRect.u16Bottom
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 3;
    u32Buffer[0 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
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
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT Test_SetIspFirstFixel(
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
        ret = STFDRV_ISP_BASE_SetIspFirstFixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspFirstFixel(), "\
            "FirstPixel = %s\n",
            u8IspIdx,
            g_szIspFirstPixel[enFirstPixel]
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
        ret = STFDRV_ISP_BASE_SetIspFirstFixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspFirstFixel(), "\
            "FirstPixel = %s\n",
            u8IspIdx,
            g_szIspFirstPixel[enFirstPixel]
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
        EN_ISP_1ST_PXL enFirstPixel = EN_ISP_1ST_PXL_GB;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspFirstFixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspFirstFixel(), "\
            "FirstPixel = %s\n",
            u8IspIdx,
            g_szIspFirstPixel[enFirstPixel]
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
        ret = STFDRV_ISP_BASE_SetIspFirstFixel(
            pstCiConnection,
            u8IspIdx,
            enFirstPixel
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspFirstFixel(), "\
            "FirstPixel = %s\n",
            u8IspIdx,
            g_szIspFirstPixel[enFirstPixel]
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

STF_RESULT Test_SetIspMultiFrameMode(
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
        EN_ISP_MULTI_FRAME_MODE enIspMultiFrameMode =
            EN_ISP_MULTI_FRAME_MODE_DISABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspMultiFrameMode(
            pstCiConnection,
            u8IspIdx,
            enIspMultiFrameMode
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspMultiFrameMode(), "\
            "enMultiFrameMode = %s\n",
            u8IspIdx,
            ((enIspMultiFrameMode) ? ("Enable") : ("Disable"))
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
        EN_ISP_MULTI_FRAME_MODE enIspMultiFrameMode =
            EN_ISP_MULTI_FRAME_MODE_ENABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspMultiFrameMode(
            pstCiConnection,
            u8IspIdx,
            enIspMultiFrameMode
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspMultiFrameMode(), "\
            "enMultiFrameMode = %s\n",
            u8IspIdx,
            ((enIspMultiFrameMode) ? ("Enable") : ("Disable"))
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

STF_RESULT Test_SetIspAndCsiBaseParameters(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[7 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_SENSOR;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CFA;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_IFSZ;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[6 * 2 + 1] = 0;
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
        ST_ISP_CSI_BASE_PARAMETERS stIspCsiBaseParameters =
        {
            0,
            {
#if 0
                {
                    EN_IMAGER_SEL_DVP,
                    EN_DVP_ITU656_DISABLE,
                    EN_DVP_SYNC_POL_H_POS_V_NEG
                },
                EN_CFA_RAW_FMT_BGGR,
                { 0, 0, 1919, 1079 },
                EN_ISP_1ST_PXL_B,
                EN_ISP_MULTI_FRAME_MODE_ENABLE
#else
                {
                    EN_IMAGER_SEL_MIPI,
                    EN_DVP_ITU656_ENABLE,
                    EN_DVP_SYNC_POL_H_POS_V_POS
                },
                EN_CFA_RAW_FMT_RGGB,
                { 10, 10, 1909, 1069 },
                EN_ISP_1ST_PXL_R,
                EN_ISP_MULTI_FRAME_MODE_DISABLE
#endif
            }
        };

        stIspCsiBaseParameters.u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspAndCsiBaseParameters(
            pstCiConnection,
            &stIspCsiBaseParameters
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspAndCsiBaseParameters()\n",
            stIspCsiBaseParameters.u8IspIdx
            );
        LOG_INFO("enImagerSel = %d, enItu656 = %d, enPolarity = %d\n",
            stIspCsiBaseParameters.stIspCsiBaseParam.stSensorParam.enImagerSel,
            stIspCsiBaseParameters.stIspCsiBaseParam.stSensorParam.enItu656,
            stIspCsiBaseParameters.stIspCsiBaseParam.stSensorParam.enPolarity
            );
        LOG_INFO("enRawFormat = 0x%08X\n",
            stIspCsiBaseParameters.stIspCsiBaseParam.enRawFormat
            );
        LOG_INFO("stRect = { %d, %d, %d, %d }\n",
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Left,
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Top,
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Right,
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Bottom
            );
        LOG_INFO("FirstPixel = %s\n",
            g_szIspFirstPixel[stIspCsiBaseParameters.stIspCsiBaseParam.enFirstPixel]
                              );
        LOG_INFO("enMultiFrameMode = %s\n",
            ((stIspCsiBaseParameters.stIspCsiBaseParam.enMultiFrameMode)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_SENSOR;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CFA;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_IFSZ;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[6 * 2 + 1] = 0;
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
        ST_ISP_CSI_BASE_PARAMETERS stIspCsiBaseParameters =
        {
            0,
            {
#if 1
                {
                    EN_IMAGER_SEL_DVP,
                    EN_DVP_ITU656_DISABLE,
                    EN_DVP_SYNC_POL_H_POS_V_NEG
                },
                EN_CFA_RAW_FMT_BGGR,
                { 0, 0, 1919, 1079 },
                EN_ISP_1ST_PXL_B,
                EN_ISP_MULTI_FRAME_MODE_ENABLE
#else
                {
                    EN_IMAGER_SEL_MIPI,
                    EN_DVP_ITU656_ENABLE,
                    EN_DVP_SYNC_POL_H_POS_V_POS
                },
                EN_CFA_RAW_FMT_RGGB,
                { 10, 10, 1909, 1069 },
                EN_ISP_1ST_PXL_R,
                EN_ISP_MULTI_FRAME_MODE_DISABLE
#endif
            }
        };

        stIspCsiBaseParameters.u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspAndCsiBaseParameters(
            pstCiConnection,
            &stIspCsiBaseParameters
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspAndCsiBaseParameters()\n",
            stIspCsiBaseParameters.u8IspIdx
            );
        LOG_INFO("enImagerSel = %d, enItu656 = %d, enPolarity = %d\n",
            stIspCsiBaseParameters.stIspCsiBaseParam.stSensorParam.enImagerSel,
            stIspCsiBaseParameters.stIspCsiBaseParam.stSensorParam.enItu656,
            stIspCsiBaseParameters.stIspCsiBaseParam.stSensorParam.enPolarity
            );
        LOG_INFO("enRawFormat = 0x%08X\n",
            stIspCsiBaseParameters.stIspCsiBaseParam.enRawFormat
            );
        LOG_INFO("stRect = { %d, %d, %d, %d }\n",
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Left,
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Top,
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Right,
            stIspCsiBaseParameters.stIspCsiBaseParam.stRect.u16Bottom
            );
        LOG_INFO("FirstPixel = %s\n",
            g_szIspFirstPixel[stIspCsiBaseParameters.stIspCsiBaseParam.enFirstPixel]
                              );
        LOG_INFO("enMultiFrameMode = %s\n",
            ((stIspCsiBaseParameters.stIspCsiBaseParam.enMultiFrameMode)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0 * 2] = ADDR_REG_SENSOR;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CFA;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_CAPTURE_WINDOW_START;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_CAPTURE_WINDOW_END;
    u32Buffer[3 * 2 + 1] = 0;
    u32Buffer[4 * 2] = ADDR_REG_IFSZ;
    u32Buffer[4 * 2 + 1] = 0;
    u32Buffer[5 * 2] = ADDR_REG_ISPCTRL1;
    u32Buffer[5 * 2 + 1] = 0;
    u32Buffer[6 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[6 * 2 + 1] = 0;
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

STF_RESULT Test_SetIspEnable(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= 0xFFFFFFFE;
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
    {
        STF_BOOL8 bEnable = STF_ENABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspEnable(
            pstCiConnection,
            u8IspIdx,
            bEnable
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspEnable(), bEnable = %s\n",
            u8IspIdx,
            ((bEnable) ? ("Enable") : ("Disable"))
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
        STF_BOOL8 bEnable = STF_DISABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspEnable(
            pstCiConnection,
            u8IspIdx,
            bEnable
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspEnable(), bEnable = %s\n",
            u8IspIdx,
            ((bEnable) ? ("Enable") : ("Disable"))
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

STF_RESULT Test_SetIspReset(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= 0xFFFFFFFD;
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
    {
        STF_BOOL8 bReset = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspReset(
            pstCiConnection,
            u8IspIdx,
            bReset
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspReset(), bReset = %s\n",
            u8IspIdx,
            ((bReset) ? ("Reset") : ("Normal"))
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
        STF_BOOL8 bReset = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspReset(
            pstCiConnection,
            u8IspIdx,
            bReset
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspReset(), bReset = %s\n",
            u8IspIdx,
            ((bReset) ? ("Reset") : ("Normal"))
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

STF_RESULT Test_IspReset(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] |= 0x00000002;
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
    {
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_IspReset(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IspReset()\n", u8IspIdx);
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

STF_RESULT Test_IspResetAndEnable(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFFFFFE) | 0x00000002;
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
    {
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_IspResetAndEnable(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IspResetAndEnable()\n", u8IspIdx);
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

//-----------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_BASE)
STF_RESULT Test_IsIsp_Out_IspOrOutBusy(
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
    u32Offset = ADDR_REG_ISPSTAT0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFFFF0) | 0x00000001;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] &= 0xFFFFFFF0;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsIsp_Uo_Out_IspOrOutBusy(
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
    u32Offset = ADDR_REG_ISPSTAT0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFFFF0) | 0x00000002;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsUoBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsUoBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs0Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs0Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs1Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs1Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] &= 0xFFFFFFF0;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsUoBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsUoBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs0Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs0Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs1Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs1Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsIsp_Ss0_Out_IspOrOutBusy(
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
    u32Offset = ADDR_REG_ISPSTAT0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFFFF0) | 0x00000004;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsUoBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsUoBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs0Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs0Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs1Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs1Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] &= 0xFFFFFFF0;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsUoBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsUoBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs0Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs0Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs1Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs1Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsIsp_Ss1_Out_IspOrOutBusy(
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
    u32Offset = ADDR_REG_ISPSTAT0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & 0xFFFFFF0) | 0x00000008;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsUoBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsUoBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs0Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs0Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs1Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs1Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] &= 0xFFFFFFF0;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsUoBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsUoBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs0Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs0Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsSs1Busy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsSs1Busy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspOrOutBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspOrOutBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsCsiBufferOverrun(
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
    u32Offset = ADDR_REG_ISPSTAT0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] |= 0x00000200;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCsiBufferOverrun(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCsiBufferOverrun(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] &= 0xFFFFFDFF;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_BOOL8 bStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCsiBufferOverrun(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCsiBufferOverrun(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
STF_RESULT Test_GetIspAllStatus(
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
    u32Offset = ADDR_REG_ISPSTAT0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL) | STS_ISP_BUSY;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL) | STS_ISP_UO_BUSY;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL) | STS_ISP_SS0_BUSY;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL) | STS_ISP_SS1_BUSY;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL) | STS_OUT_ALL;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL) | STS_ISP_AND_OUT_ALL;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL) | STS_CSI_BUFFER_ERROR;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] = (u32Buffer[0] & ~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] |= STS_ISP_ALL;
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s,"\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPSTAT0;
    u32Buffer[0] &= (~STS_ISP_ALL);
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
    u32Offset = ADDR_REG_ISPSTAT0;
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
        STF_U32 u32IspAllStatus = STF_TRUE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32IspAllStatus = STFDRV_ISP_BASE_GetIspAllStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetIspAllStatus(), "\
            "u32IspAllStatus = 0x%08X\n",
            u8IspIdx,
            u32IspAllStatus
            );
        LOG_INFO("ISP_n_Out = %s, ISP = %s, Out = %s, UO = %s, SS0 = %s, "\
            "SS1 = %s, CSI_BUF = %s\n",
            ((u32IspAllStatus & STS_ISP_AND_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_OUT_ALL) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_UO_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS0_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_ISP_SS1_BUSY) ? ("Busy") : ("Done")),
            ((u32IspAllStatus & STS_CSI_BUFFER_ERROR) ? ("Overrun") : ("Empty"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_BASE)
STF_RESULT Test_IsIspDoneInterrupt(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] |= EN_INT_ISP_DONE;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspDoneInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspDoneInterrupt(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Processing"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= ~EN_INT_ISP_DONE;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsIspDoneInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsIspDoneInterrupt(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Processing"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsCsiDoneInterrupt(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] |= EN_INT_CSI_DONE;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCsiDoneInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCsiDoneInterrupt(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Processing"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= ~EN_INT_CSI_DONE;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCsiDoneInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCsiDoneInterrupt(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Processing"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsScDoneInterrupt(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] |= EN_INT_SC_DONE;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsScDoneInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsScDoneInterrupt(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Processing"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= ~EN_INT_SC_DONE;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsScDoneInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsScDoneInterrupt(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Done") : ("Processing"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_IsCsiLineInterrupt(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] |= EN_INT_LINE_INT;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCsiLineInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCsiLineInterrupt(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Interrupted") : ("No-Interrupt"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= ~EN_INT_LINE_INT;
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
        STF_BOOL8 bStatus = STF_FALSE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_BASE_IsCsiLineInterrupt(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_IsCsiLineInterrupt(), "\
            "bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Interrupted") : ("No-Interrupt"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_SetInterrupt(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= (~EN_INT_ALL);
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
        EN_ISP_INTERRUPT enInterrupt = EN_INT_ALL;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetInterrupt(
            pstCiConnection,
            u8IspIdx,
            enInterrupt
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetInterrupt(), "\
            "enInterrupt = 0x%08X\n",
            u8IspIdx,
            enInterrupt
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
STF_RESULT Test_SetInterruptClear(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
#if 0
    u32Buffer[0] |= EN_INT_ALL;
#else
    u32Buffer[0] &= ~(EN_INT_ALL);
#endif
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
        EN_ISP_INTERRUPT enInterrupt = EN_INT_ALL;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetInterruptClear(
            pstCiConnection,
            u8IspIdx,
            enInterrupt
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetInterruptClear(), "\
            "enInterrupt = 0x%08X\n",
            u8IspIdx,
            enInterrupt
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

STF_RESULT Test_GetInterruptStatus(
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
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] = EN_INT_ISP_DONE;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] = EN_INT_CSI_DONE;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] = EN_INT_SC_DONE;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] = EN_INT_LINE_INT;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] = EN_INT_ISP_DONE | EN_INT_CSI_DONE;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] = EN_INT_ISP_DONE | EN_INT_CSI_DONE | EN_INT_SC_DONE;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] |= EN_INT_ALL;
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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
    u32Offset = ADDR_REG_ISPCTRL0;
    u32Buffer[0] &= (~EN_INT_SC_DONE);
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
        STF_U32 u32Status = 0x00000000;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_BASE_GetInterruptStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_GetInterruptStatus(), "\
            "bStatus = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("ISP = %s, CSI = %s, SC = %s, CSI Line = %s\n",
            ((u32Status & EN_INT_ISP_DONE)
                ? ("Module Done") : ("Module Processing")),
            ((u32Status & EN_INT_CSI_DONE)
                ? ("Capture End") : ("Capture Processing")),
            ((u32Status & EN_INT_SC_DONE)
                ? ("Dump Done") : ("Dump Processing")),
            ((u32Status & EN_INT_LINE_INT)
                ? ("Line Interrupted") : ("No-Interrupt"))
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

#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SetCsiShadowMode(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        EN_SUD_MODE enShadowMode = EN_SUD_MODE_NEXT_VSYNC;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadowMode(
            pstCiConnection,
            u8IspIdx,
            enShadowMode
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiShadowMode(), "\
            "enShadowMode = %s\n",
            u8IspIdx,
            ((enShadowMode) ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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
        EN_SUD_MODE enShadowMode = EN_SUD_MODE_IMMEDIATELY;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadowMode(
            pstCiConnection,
            u8IspIdx,
            enShadowMode
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiShadowMode(), "\
            "enShadowMode = %s\n",
            u8IspIdx,
            ((enShadowMode) ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SetCsiShadow(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        ST_SUD_SHADOW_PARAM stSudShadowParam = {
            0,
            {
                EN_SUD_MODE_NEXT_VSYNC,
                EN_SUD_CTRL_ENABLE,
            },
        };

        //stSudShadowParam.u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadow(
            pstCiConnection,
            &stSudShadowParam
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiShadow(), "\
            "enShadowEnable = %s, enShadowMode = %s\n",
            stSudShadowParam.u8IspIdx,
            ((stSudShadowParam.stShadowParam.enShadowEnable)
                ? ("Enable") : ("Disable")),
            ((stSudShadowParam.stShadowParam.enShadowMode)
                ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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
        ST_SUD_SHADOW_PARAM stSudShadowParam = {
            0,
            {
                EN_SUD_MODE_IMMEDIATELY,
                EN_SUD_CTRL_ENABLE,
            },
        };

        //stSudShadowParam.u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadow(
            pstCiConnection,
            &stSudShadowParam
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiShadow(), "\
            "enShadowEnable = %s, enShadowMode = %s\n",
            stSudShadowParam.u8IspIdx,
            ((stSudShadowParam.stShadowParam.enShadowEnable)
                ? ("Enable") : ("Disable")),
            ((stSudShadowParam.stShadowParam.enShadowMode)
                ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

STF_RESULT Test_SetCsiShadowImmediatelyAndEnable(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadowImmediatelyAndEnable(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetCsiShadowImmediatelyAndEnable()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

STF_RESULT Test_SetCsiShadowNextVSyncAndEnable(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadowNextVSyncAndEnable(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetCsiShadowNextVSyncAndEnable()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

STF_RESULT Test_SetCsiShadowImmediatelyAndTrigger(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadowImmediatelyAndTrigger(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetCsiShadowImmediatelyAndTrigger()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

STF_RESULT Test_SetCsiShadowNextVSyncAndTrigger(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetCsiShadowNextVSyncAndTrigger(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetCsiShadowNextVSyncAndTrigger()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

STF_RESULT Test_TriggerCsiShadow(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_TriggerCsiShadow(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_TriggerCsiShadow()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SetIspShadowMode(
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
    u32Offset = ADDR_REG_IESHD;
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
        EN_SUD_MODE enShadowMode = EN_SUD_MODE_NEXT_VSYNC;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadowMode(
            pstCiConnection,
            u8IspIdx,
            enShadowMode
            );
        LOG_INFO("ISP_%d :"\
            "STFDRV_ISP_BASE_SetIspShadowMode(), enShadowMode = %s\n",
            u8IspIdx,
            ((enShadowMode) ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        EN_SUD_MODE enShadowMode = EN_SUD_MODE_IMMEDIATELY;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadowMode(
            pstCiConnection,
            u8IspIdx,
            enShadowMode
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetIspShadowMode(), enShadowMode = %s\n",
            u8IspIdx,
            ((enShadowMode) ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SetIspShadow(
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
    u32Offset = ADDR_REG_IESHD;
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
        ST_SUD_SHADOW_PARAM stSudShadowParam = {
            0,
            {
                EN_SUD_MODE_NEXT_VSYNC,
                EN_SUD_CTRL_ENABLE,
            },
        };

        //stSudShadowParam.u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadow(
            pstCiConnection,
            &stSudShadowParam
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspShadow(), "
                "enShadowEnable = %s, enShadowMode = %s\n",
            stSudShadowParam.u8IspIdx,
            ((stSudShadowParam.stShadowParam.enShadowEnable)
                ? ("Enable") : ("Disable")),
            ((stSudShadowParam.stShadowParam.enShadowMode)
                ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        ST_SUD_SHADOW_PARAM stSudShadowParam = {
            0,
            {
                EN_SUD_MODE_IMMEDIATELY,
                EN_SUD_CTRL_ENABLE,
            },
        };

        //stSudShadowParam.u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadow(
            pstCiConnection,
            &stSudShadowParam
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspShadow(),"
                " enShadowEnable = %s, enShadowMode = %s\n",
            stSudShadowParam.u8IspIdx,
            ((stSudShadowParam.stShadowParam.enShadowEnable)
                ? ("Enable") : ("Disable")),
            ((stSudShadowParam.stShadowParam.enShadowMode)
                ? ("Next Vsync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SetIspShadowImmediatelyAndEnable(
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
    u32Offset = ADDR_REG_IESHD;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadowImmediatelyAndEnable(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetIspShadowImmediatelyAndEnable()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SetIspShadowNextVSyncAndEnable(
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
    u32Offset = ADDR_REG_IESHD;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadowNextVSyncAndEnable(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetIspShadowNextVSyncAndEnable()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SetIspShadowImmediatelyAndTrigger(
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
    u32Offset = ADDR_REG_IESHD;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadowImmediatelyAndTrigger(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetIspShadowImmediatelyAndTrigger()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SetIspShadowNextVSyncAndTrigger(
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
    u32Offset = ADDR_REG_IESHD;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_SetIspShadowNextVSyncAndTrigger(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_BASE_SetIspShadowNextVSyncAndTrigger()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_TriggerIspShadow(
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
    u32Offset = ADDR_REG_IESHD;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_TriggerIspShadow(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_TriggerIspShadow()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
STF_RESULT Test_CsiAndIspStart(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[4 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    {
        STF_BOOL8 bEnable = STF_DISABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetCsiEnable(
            pstCiConnection,
            u8IspIdx,
            bEnable
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetCsiEnable(), bEnable = %s\n",
            u8IspIdx,
            ((bEnable) ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    {
        STF_BOOL8 bEnable = STF_DISABLE;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_BASE_SetIspEnable(
            pstCiConnection,
            u8IspIdx,
            bEnable
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_SetIspEnable(), bEnable = %s\n",
            u8IspIdx,
            ((bEnable) ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 4;
    u32Buffer[0 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_IESHD;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_CsiAndIspStart(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_CsiAndIspStart()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 4;
    u32Buffer[0 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_IESHD;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
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

STF_RESULT Test_CsiAndIspStop(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[4 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 4;
    u32Buffer[0 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_IESHD;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
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
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        STFDRV_ISP_BASE_CsiAndIspStop(
            pstCiConnection,
            &u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_BASE_CsiAndIspStop()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 4;
    u32Buffer[0 * 2] = ADDR_REG_CSIINTS;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_IESHD;
    u32Buffer[1 * 2 + 1] = 0;
    u32Buffer[2 * 2] = ADDR_REG_ENABLE_STATUS;
    u32Buffer[2 * 2 + 1] = 0;
    u32Buffer[3 * 2] = ADDR_REG_ISPCTRL0;
    u32Buffer[3 * 2 + 1] = 0;
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

//-----------------------------------------------------------------------------
STF_RESULT ISP_Isp_Base_Test(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    g_pstCiConnection = (CI_CONNECTION *)STFLIB_ISP_BASE_GetConnection(
        TEST_ISP_DEVICE
        );

    //=========================================================================
#if !defined(RISCV)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCSIEnable(
            );
        LOG_INFO("Test SetCSIEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if !defined(RISCV)
#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsCSIBusy(
            );
        LOG_INFO("Test IsCSIBusy command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsScdDone(
            );
        LOG_INFO("Test IsScdDone command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsScdError(
            );
        LOG_INFO("Test IsScdError command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsItu656Field(
            );
        LOG_INFO("Test IsItu656Field command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsItu656Error(
            );
        LOG_INFO("Test IsItu656Error command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_GetCsiAllStatus(
            );
        LOG_INFO("Test GetCsiAllStatus command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
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
        ret = Test_GetCSIVCount(
            );
        LOG_INFO("Test GetCSIVCount command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_GetCSIHCount(
            );
        LOG_INFO("Test GetCSIHCount command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiLineIntLine(
            );
        LOG_INFO("Test SetCsiLineIntLine command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_GetCsiLineIntLine(
            );
        LOG_INFO("Test GetCsiLineIntLine command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
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
        ret = Test_SetModuleEnable(
            );
        LOG_INFO("Test SetModuleEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetModulesEnable(
            );
        LOG_INFO("Test SetModulesEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
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
        ret = Test_SetSensorParameter(
            );
        LOG_INFO("Test SetSensorParameter command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_GetSensorRawFormat(
            );
        LOG_INFO("Test GetSensorRawFormat command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetSensorRawFormat(
            );
        LOG_INFO("Test SetSensorRawFormat command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
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
        ret = Test_GetIspCaptureSize(
            );
        LOG_INFO("Test GetIspCaptureSize command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiCaptureWindowAndIspCaptureSize(
            );
        LOG_INFO("Test SetCsiCaptureWindowAndIspCaptureSize command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
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
        ret = Test_SetIspFirstFixel(
            );
        LOG_INFO("Test SetIspFirstFixel command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspMultiFrameMode(
            );
        LOG_INFO("Test SetIspMultiFrameMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspAndCsiBaseParameters(
            );
        LOG_INFO("Test SetIspAndCsiBaseParameters command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#if !defined(RISCV)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspEnable(
            );
        LOG_INFO("Test SetIspEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if !defined(RISCV)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspReset(
            );
        LOG_INFO("Test SetIspReset command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IspReset(
            );
        LOG_INFO("Test IspReset command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
#if !defined(RISCV)

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IspResetAndEnable(
            );
        LOG_INFO("Test IspResetAndEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
#endif //#if !defined(RISCV)
    //=========================================================================

    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsIsp_Out_IspOrOutBusy(
            );
        LOG_INFO("Test IsIsp_Out_IspOrOutBusy command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsIsp_Uo_Out_IspOrOutBusy(
            );
        LOG_INFO("Test IsIsp_Uo_Out_IspOrOutBusy command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsIsp_Ss0_Out_IspOrOutBusy(
            );
        LOG_INFO("Test IsIsp_Ss0_Out_IspOrOutBusy command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsIsp_Ss1_Out_IspOrOutBusy(
            );
        LOG_INFO("Test IsIsp_Ss1_Out_IspOrOutBusy command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsCsiBufferOverrun(
            );
        LOG_INFO("Test IsCsiBufferOverrun command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_GetIspAllStatus(
            );
        LOG_INFO("Test GetIspAllStatus command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsIspDoneInterrupt(
            );
        LOG_INFO("Test IsIspDoneInterrupt command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsCsiDoneInterrupt(
            );
        LOG_INFO("Test IsCsiDoneInterrupt command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsScDoneInterrupt(
            );
        LOG_INFO("Test IsScDoneInterrupt command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_IsCsiLineInterrupt(
            );
        LOG_INFO("Test IsCsiLineInterrupt command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetInterrupt(
            );
        LOG_INFO("Test SetInterrupt command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetInterruptClear(
            );
        LOG_INFO("Test SetInterruptClear command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_GetInterruptStatus(
            );
        LOG_INFO("Test GetInterruptStatus command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //=========================================================================
    // Since the shadow update enable bit cannot be read back,
    // therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiShadowMode(
            );
        LOG_INFO("Test SetCsiShadowMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
   //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiShadow(
            );
        LOG_INFO("Test SetCsiShadow command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiShadowImmediatelyAndEnable(
            );
        LOG_INFO("Test SetCsiShadowImmediatelyAndEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiShadowNextVSyncAndEnable(
            );
        LOG_INFO("Test SetCsiShadowNextVSyncAndEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiShadowImmediatelyAndTrigger(
            );
        LOG_INFO("Test SetCsiShadowImmediatelyAndTrigger command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetCsiShadowNextVSyncAndTrigger(
            );
        LOG_INFO("Test SetCsiShadowNextVSyncAndTrigger command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TriggerCsiShadow(
            );
        LOG_INFO("Test TriggerCsiShadow command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
#if defined(EXTEND_IOCTL_COMMAND_BASE)
    //=========================================================================
    // Since the shadow update enable bit cannot be read back,
    // therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspShadowMode(
            );
        LOG_INFO("Test SetIspShadowMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspShadow(
            );
        LOG_INFO("Test SetIspShadow command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspShadowImmediatelyAndEnable(
            );
        LOG_INFO("Test SetIspShadowImmediatelyAndEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspShadowNextVSyncAndEnable(
            );
        LOG_INFO("Test SetIspShadowNextVSyncAndEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspShadowImmediatelyAndTrigger(
            );
        LOG_INFO("Test SetIspShadowImmediatelyAndTrigger command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SetIspShadowNextVSyncAndTrigger(
            );
        LOG_INFO("Test SetIspShadowNextVSyncAndTrigger command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TriggerIspShadow(
            );
        LOG_INFO("Test TriggerIspShadow command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
#if !defined(RISCV)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_CsiAndIspStart(
            );
        LOG_INFO("Test CsiAndIspStart command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_CsiAndIspStop(
            );
        LOG_INFO("Test CsiAndIspStop command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP base command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if !defined(RISCV)

    return ret;
}

//-----------------------------------------------------------------------------
