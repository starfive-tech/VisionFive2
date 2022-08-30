/**
  ******************************************************************************
  * @file  ISP_mod_car_test.c
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

#define LOG_TAG "Mod_Car_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_car_test.h"
#include "ISP_test.h"


/* ISPC CAR driver test structure */


/* ISPC CAR driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC CAR driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_CAR_SetReg(
    STF_VOID
    )
{
#if 0
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
#endif
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if 0
    u32Offset = ADDR_REG_ICFAM;
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
#endif
    {
        ST_ISP_CAR_RDMA stIspCarRdma;

        stIspCarRdma.stCarRdma.stRdmaStartCmd.Tag = MREG_WR;
        //stIspCarRdma.stCarRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ICFAM;
        stIspCarRdma.stCarRdma.stRdmaStartCmd.u32Param = ST_CAR_REG_LEN;
        stIspCarRdma.stCarRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspCarRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CAR_SetReg(
            pstCiConnection,
            &stIspCarRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CAR_SetReg()\n", stIspCarRdma.u8IspIdx);
#if 0
        LOG_INFO("hv_width = 0x%01X\n",
            stCarRdma.stCarReg.Mode.field.hv_width
            );
        LOG_INFO("cross_cov = 0x%01X\n",
            stCarRdma.stCarReg.Mode.field.cross_cov
            );
#endif
        LOG_INFO("---------------------------------\n");
    }
#if 0
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICFAM;
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
#endif
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_CAR)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_CAR)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Car_Test(
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
        ret = Test_CAR_SetReg(
            );
        LOG_INFO("Test CAR_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CAR module command test.\n");
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
    LOG_INFO("The CAR module doesn't has any registers needs to program and test.\n");
    LOG_INFO("---------------------------------\n");
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif
#if defined(EXTEND_IOCTL_COMMAND_CAR)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_CAR)

    return ret;
}

//-----------------------------------------------------------------------------
