/**
  ******************************************************************************
  * @file  ISP_mod_gmargb_test.c
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

#include <string.h>

#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"

#include "ISPC/stflib_isp_base.h"

#define LOG_TAG "Mod_GmaRgb_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_gmargb_test.h"
#include "ISP_test.h"


/* ISPC GMARGB driver test structure */


/* ISPC GMARGB driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC GMARGB driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_GMARGB_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[GMARGB_PNT];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_IGRVAL_0;
    u32Length = GMARGB_PNT;
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
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_GMARGB_RDMA stIspGmaRgbRdma;
        STF_CHAR szMessage[512] = "";
        STF_CHAR szTemp[128] = "";

        stIspGmaRgbRdma.stGmaRgbRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspGmaRgbRdma.stGmaRgbRdma.stRdmaStartCmd.RegAddr = ADDR_REG_IGRVAL_0;
        stIspGmaRgbRdma.stGmaRgbRdma.stRdmaStartCmd.u32Param =
            ST_GMARGB_REG_LEN;
        stIspGmaRgbRdma.stGmaRgbRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = GMARGB_PNT;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            stIspGmaRgbRdma.stGmaRgbRdma.stGmaRgbReg.Gamma[u32Idx].value =
                (rand() & 0x03FF) | ((rand() & 0xFFFF) << 16);
        }
        stIspGmaRgbRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_GMARGB_SetReg(
            pstCiConnection,
            &stIspGmaRgbRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_GMARGB_SetReg()\n",
            stIspGmaRgbRdma.u8IspIdx);
        szMessage[0] = '\0';
        u32Cnt = GMARGB_PNT;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if (0 == u32Idx) {
                sprintf(szTemp, "RGB Gamma value = 0x%04X",
                    stIspGmaRgbRdma.stGmaRgbRdma.stGmaRgbReg.Gamma[u32Idx].field.gval
                    );
            } else if ((7 == (u32Idx % 8)) || ((u32Cnt - 1) == u32Idx)) {
                sprintf(szTemp, ", 0x%04X\n",
                    stIspGmaRgbRdma.stGmaRgbRdma.stGmaRgbReg.Gamma[u32Idx].field.gval
                    );
            } else {
                sprintf(szTemp, ", 0x%04X",
                    stIspGmaRgbRdma.stGmaRgbRdma.stGmaRgbReg.Gamma[u32Idx].field.gval
                    );
            }
            strcat(szMessage, szTemp);
        }
        LOG_INFO("%s", szMessage);
        szMessage[0] = '\0';
        u32Cnt = GMARGB_PNT;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if (0 == u32Idx) {
                sprintf(szTemp, "RGB Gamma Slope = 0x%04X",
                    stIspGmaRgbRdma.stGmaRgbRdma.stGmaRgbReg.Gamma[u32Idx].field.sgval
                    );
            } else if ((7 == (u32Idx % 8)) || ((u32Cnt - 1) == u32Idx)) {
                sprintf(szTemp, ", 0x%04X\n",
                    stIspGmaRgbRdma.stGmaRgbRdma.stGmaRgbReg.Gamma[u32Idx].field.sgval
                    );
            } else {
                sprintf(szTemp, ", 0x%04X",
                    stIspGmaRgbRdma.stGmaRgbRdma.stGmaRgbReg.Gamma[u32Idx].field.sgval
                    );
            }
            strcat(szMessage, szTemp);
        }
        LOG_INFO("%s", szMessage);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IGRVAL_0;
    u32Length = GMARGB_PNT;
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

#if defined(EXTEND_IOCTL_COMMAND_GMARGB)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_GMARGB)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_GmaRgb_Test(
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
        ret = Test_GMARGB_SetReg(
            );
        LOG_INFO("Test GMARGB_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the GMARGB module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_GMARGB)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_GMARGB)

    return ret;
}

//-----------------------------------------------------------------------------
