/**
  ******************************************************************************
  * @file  ISP_mem_test.c
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

#define LOG_TAG "Mem_Test"
#include "stf_common/userlog.h"

#include "ISP_mem_test.h"
#include "ISP_test.h"


//=== Godspeed === Add new memory/buffer type support here.
#define TEST_IOCTL_MEM_ALLOC_FREE_UO
#define TEST_IOCTL_MEM_ALLOC_FREE_SS0
#define TEST_IOCTL_MEM_ALLOC_FREE_SS1
#define TEST_IOCTL_MEM_ALLOC_FREE_DUMP
//#define TEST_IOCTL_MEM_ALLOC_FREE_TILING_1_RD
#define TEST_IOCTL_MEM_ALLOC_FREE_TILING_1_WR
#define TEST_IOCTL_MEM_ALLOC_FREE_SC_DUMP
#define TEST_IOCTL_MEM_ALLOC_FREE_Y_HIST
#define TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM
#define TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM_ALIGN_128


//#define TEST_USER_MODE_MEMORY_ACCESS


/* ISPC Memory driver test structure */


//=== Godspeed === Add new memory/buffer type support here.
/* ISPC Memory driver test variables */
static CI_MEM_PARAM *g_pstTestUoMem = NULL;
static CI_MEM_PARAM *g_pstTestSs0Mem = NULL;
static CI_MEM_PARAM *g_pstTestSs1Mem = NULL;
static CI_MEM_PARAM *g_pstTestDumpMem = NULL;
static CI_MEM_PARAM *g_pstTestTiling_1_WrMem = NULL;
static CI_MEM_PARAM *g_pstTestScDumpMem = NULL;
static CI_MEM_PARAM *g_pstTestYHistMem = NULL;
static CI_MEM_PARAM *g_pstTestGenMemMem = NULL;
static CI_MEM_PARAM *g_pstTestGenMemAlign128Mem = NULL;


/* ISPC Memory driver test interface */
//-----------------------------------------------------------------------------
//=== Godspeed === Add new memory/buffer type support here.
STF_RESULT Test_UoMemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestUoMem,
        1920,
        1080,
        CI_TYPE_UO,
        TYPE_YUV,
        YUV_420_PL12_8,
        MOSAIC_NONE,
        0,
        "Uo"
        );

    if (STF_SUCCESS == ret) {
        STF_U8 *pBuffer = (STF_U8*) g_pstTestUoMem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestUoMem = %p, "\
                "g_pstTestUoMem->pBUffer = %p (%p)\n",
                g_pstTestUoMem,
                g_pstTestUoMem->pBuffer, pBuffer
                );
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestUoMem->au32Offset[0],
                pBuffer[g_pstTestUoMem->au32Offset[0]],
                g_pstTestUoMem->au32Offset[1] - 1,
                pBuffer[g_pstTestUoMem->au32Offset[1] - 1],
                g_pstTestUoMem->au32Offset[1] + 0,
                pBuffer[g_pstTestUoMem->au32Offset[1] + 0],
                g_pstTestUoMem->au32Offset[1]
                    + (g_pstTestUoMem->au32CbCrSize[0]
                        * g_pstTestUoMem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestUoMem->au32Offset[1]
                    + (g_pstTestUoMem->au32CbCrSize[0]
                        * g_pstTestUoMem->au32CbCrSize[1]) - 1]
                );

            pBuffer[g_pstTestUoMem->au32Offset[1] - 1] = 0x12;
            pBuffer[g_pstTestUoMem->au32Offset[1]
                + (g_pstTestUoMem->au32CbCrSize[0]
                    * g_pstTestUoMem->au32CbCrSize[1]) - 1] = 0x34;
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestUoMem->au32Offset[0],
                pBuffer[g_pstTestUoMem->au32Offset[0]],
                g_pstTestUoMem->au32Offset[1] - 1,
                pBuffer[g_pstTestUoMem->au32Offset[1] - 1],
                g_pstTestUoMem->au32Offset[1] + 0,
                pBuffer[g_pstTestUoMem->au32Offset[1] + 0],
                g_pstTestUoMem->au32Offset[1]
                    + (g_pstTestUoMem->au32CbCrSize[0]
                        * g_pstTestUoMem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestUoMem->au32Offset[1]
                    + (g_pstTestUoMem->au32CbCrSize[0]
                        * g_pstTestUoMem->au32CbCrSize[1]) - 1]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestUoMem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestUoMem memory allocate and mapping is %s, "\
        "ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_UoMemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestUoMem
        );

    LOG_INFO("g_pstTestUoMem = %p\n", g_pstTestUoMem);
    LOG_INFO("Test g_pstTestUoMem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_Ss0MemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestSs0Mem,
        960,
        540,
        CI_TYPE_SS0,
        TYPE_YUV,
        YUV_420_PL12_8,
        MOSAIC_NONE,
        0,
        "Ss0"
        );

    if (STF_SUCCESS == ret) {
        STF_U8 *pBuffer = (STF_U8*) g_pstTestSs0Mem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestSs0Mem = %p, "\
                "g_pstTestSs0Mem->pBUffer = %p (%p)\n",
                g_pstTestSs0Mem,
                g_pstTestSs0Mem->pBuffer, pBuffer
                );
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestSs0Mem->au32Offset[0],
                pBuffer[g_pstTestSs0Mem->au32Offset[0]],
                g_pstTestSs0Mem->au32Offset[1] - 1,
                pBuffer[g_pstTestSs0Mem->au32Offset[1] - 1],
                g_pstTestSs0Mem->au32Offset[1] + 0,
                pBuffer[g_pstTestSs0Mem->au32Offset[1] + 0],
                g_pstTestSs0Mem->au32Offset[1]
                    + (g_pstTestSs0Mem->au32CbCrSize[0]
                        * g_pstTestSs0Mem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestSs0Mem->au32Offset[1]
                    + (g_pstTestSs0Mem->au32CbCrSize[0]
                        * g_pstTestSs0Mem->au32CbCrSize[1]) - 1]
                );

            pBuffer[g_pstTestSs0Mem->au32Offset[1] - 1] = 0x56;
            pBuffer[g_pstTestSs0Mem->au32Offset[1]
                + (g_pstTestSs0Mem->au32CbCrSize[0]
                    * g_pstTestSs0Mem->au32CbCrSize[1]) - 1] = 0x78;
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestSs0Mem->au32Offset[0],
                pBuffer[g_pstTestSs0Mem->au32Offset[0]],
                g_pstTestSs0Mem->au32Offset[1] - 1,
                pBuffer[g_pstTestSs0Mem->au32Offset[1] - 1],
                g_pstTestSs0Mem->au32Offset[1] + 0,
                pBuffer[g_pstTestSs0Mem->au32Offset[1] + 0],
                g_pstTestSs0Mem->au32Offset[1]
                    + (g_pstTestSs0Mem->au32CbCrSize[0]
                        * g_pstTestSs0Mem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestSs0Mem->au32Offset[1]
                    + (g_pstTestSs0Mem->au32CbCrSize[0]
                        * g_pstTestSs0Mem->au32CbCrSize[1]) - 1]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestSs0Mem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestSs0Mem memory allocate and mapping is %s, "\
        "ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_Ss0MemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestSs0Mem
        );

    LOG_INFO("g_pstTestSs0Mem = %p\n", g_pstTestSs0Mem);
    LOG_INFO("Test g_pstTestSs0Mem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_Ss1MemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestSs1Mem,
        480,
        270,
        CI_TYPE_SS1,
        TYPE_YUV,
        YUV_420_PL12_8,
        MOSAIC_NONE,
        0,
        "Ss1"
        );

    if (STF_SUCCESS == ret) {
        STF_U8 *pBuffer = (STF_U8*) g_pstTestSs1Mem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestSs1Mem = %p, "\
                "g_pstTestSs1Mem->pBUffer = %p (%p)\n",
                g_pstTestSs1Mem,
                g_pstTestSs1Mem->pBuffer, pBuffer
                );
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestSs1Mem->au32Offset[0],
                pBuffer[g_pstTestSs1Mem->au32Offset[0]],
                g_pstTestSs1Mem->au32Offset[1] - 1,
                pBuffer[g_pstTestSs1Mem->au32Offset[1] - 1],
                g_pstTestSs1Mem->au32Offset[1] + 0,
                pBuffer[g_pstTestSs1Mem->au32Offset[1] + 0],
                g_pstTestSs1Mem->au32Offset[1]
                    + (g_pstTestSs1Mem->au32CbCrSize[0]
                        * g_pstTestSs1Mem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestSs1Mem->au32Offset[1]
                    + (g_pstTestSs1Mem->au32CbCrSize[0]
                        * g_pstTestSs1Mem->au32CbCrSize[1]) - 1]
                );

            pBuffer[g_pstTestSs1Mem->au32Offset[1] - 1] = 0x90;
            pBuffer[g_pstTestSs1Mem->au32Offset[1]
                + (g_pstTestSs1Mem->au32CbCrSize[0]
                    * g_pstTestSs1Mem->au32CbCrSize[1]) - 1] = 0x12;
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestSs1Mem->au32Offset[0],
                pBuffer[g_pstTestSs1Mem->au32Offset[0]],
                g_pstTestSs1Mem->au32Offset[1] - 1,
                pBuffer[g_pstTestSs1Mem->au32Offset[1] - 1],
                g_pstTestSs1Mem->au32Offset[1] + 0,
                pBuffer[g_pstTestSs1Mem->au32Offset[1] + 0],
                g_pstTestSs1Mem->au32Offset[1]
                    + (g_pstTestSs1Mem->au32CbCrSize[0]
                        * g_pstTestSs1Mem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestSs1Mem->au32Offset[1]
                    + (g_pstTestSs1Mem->au32CbCrSize[0]
                        * g_pstTestSs1Mem->au32CbCrSize[1]) - 1]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestSs1Mem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestSs1Mem-> memory allocate and mapping is %s, "\
        "ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_Ss1MemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestSs1Mem
        );

    LOG_INFO("g_pstTestSs1Mem = %p\n", g_pstTestSs1Mem);
    LOG_INFO("Test g_pstTestSs1Mem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_DumpMemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestDumpMem,
        1920,
        1080,
        CI_TYPE_DUMP,
        TYPE_BAYER,
        BAYER_RGGB_NO_GAP_PACK_12,
        MOSAIC_RGGB,
        0,
        "Dump"
        );

    if (STF_SUCCESS == ret) {
        STF_U8 *pBuffer = (STF_U8*) g_pstTestDumpMem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestDumpMem = %p, "\
                "g_pstTestDumpMem->pBUffer = %p (%p)\n",
                g_pstTestDumpMem,
                g_pstTestDumpMem->pBuffer, pBuffer
                );
            LOG_INFO("Dump[%d] = 0x%02X, Dump[%d] = 0x%02X\n",
                g_pstTestDumpMem->au32Offset[0],
                pBuffer[g_pstTestDumpMem->au32Offset[0]],
                (g_pstTestDumpMem->au32YSize[0]
                    * g_pstTestDumpMem->au32YSize[1]) - 1,
                pBuffer[(g_pstTestDumpMem->au32YSize[0]
                    * g_pstTestDumpMem->au32YSize[1]) - 1]
                );

            pBuffer[g_pstTestDumpMem->au32Offset[0]] = 0x19;
            pBuffer[(g_pstTestDumpMem->au32YSize[0]
                * g_pstTestDumpMem->au32YSize[1]) - 1] = 0x20;
            LOG_INFO("Dump[%d] = 0x%02X, Dump[%d] = 0x%02X\n",
                g_pstTestDumpMem->au32Offset[0],
                pBuffer[g_pstTestDumpMem->au32Offset[0]],
                (g_pstTestDumpMem->au32YSize[0]
                    * g_pstTestDumpMem->au32YSize[1]) - 1,
                pBuffer[(g_pstTestDumpMem->au32YSize[0]
                    * g_pstTestDumpMem->au32YSize[1]) - 1]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestDumpMem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestDumpMem memory allocate and mapping is %s, "\
        "ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_DumpMemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestDumpMem
        );

    LOG_INFO("g_pstTestDumpMem = %p\n", g_pstTestDumpMem);
    LOG_INFO("Test g_pstTestDumpMem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_Tiling_1_WrMemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestTiling_1_WrMem,
        480,
        270,
        CI_TYPE_TILING_1_WR,
        TYPE_YUV,
        YUV_420_PL12_8,
        MOSAIC_NONE,
        0,
        "Tiling_1_Wr"
        );

    if (STF_SUCCESS == ret) {
        STF_U8 *pBuffer = (STF_U8*) g_pstTestTiling_1_WrMem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestTiling_1_WrMem = %p, "\
                "g_pstTestTiling_1_WrMem->pBUffer = %p (%p)\n",
                g_pstTestTiling_1_WrMem,
                g_pstTestTiling_1_WrMem->pBuffer, pBuffer
                );
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestTiling_1_WrMem->au32Offset[0],
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[0]],
                g_pstTestTiling_1_WrMem->au32Offset[1] - 1,
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1] - 1],
                g_pstTestTiling_1_WrMem->au32Offset[1] + 0,
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1] + 0],
                g_pstTestTiling_1_WrMem->au32Offset[1]
                    + (g_pstTestTiling_1_WrMem->au32CbCrSize[0]
                        * g_pstTestTiling_1_WrMem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1]
                    + (g_pstTestTiling_1_WrMem->au32CbCrSize[0]
                        * g_pstTestTiling_1_WrMem->au32CbCrSize[1]) - 1]
                );

            pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1] - 1] = 0x90;
            pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1]
                + (g_pstTestTiling_1_WrMem->au32CbCrSize[0]
                    * g_pstTestTiling_1_WrMem->au32CbCrSize[1]) - 1] = 0x12;
            LOG_INFO("Y[%d] = 0x%02X, Y[%d] = 0x%02X, CbCr[%d] = 0x%02X, "\
                "CbCr[%d] = 0x%02X\n",
                g_pstTestTiling_1_WrMem->au32Offset[0],
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[0]],
                g_pstTestTiling_1_WrMem->au32Offset[1] - 1,
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1] - 1],
                g_pstTestTiling_1_WrMem->au32Offset[1] + 0,
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1] + 0],
                g_pstTestTiling_1_WrMem->au32Offset[1]
                    + (g_pstTestTiling_1_WrMem->au32CbCrSize[0]
                        * g_pstTestTiling_1_WrMem->au32CbCrSize[1]) - 1,
                pBuffer[g_pstTestTiling_1_WrMem->au32Offset[1]
                    + (g_pstTestTiling_1_WrMem->au32CbCrSize[0]
                        * g_pstTestTiling_1_WrMem->au32CbCrSize[1]) - 1]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestTiling_1_WrMem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestTiling_1_WrMem-> memory allocate and "\
        "mapping is %s, ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_Tiling_1_WrMemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestTiling_1_WrMem
        );

    LOG_INFO("g_pstTestTiling_1_WrMem = %p\n", g_pstTestTiling_1_WrMem);
    LOG_INFO("Test g_pstTestTiling_1_WrMem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_ScDumpMemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestScDumpMem,
        0,
        0,
        CI_TYPE_SC_DUMP,
        TYPE_NONE,
        PXL_NONE,
        MOSAIC_NONE,
        0,
        "ScDump"
        );

    if (STF_SUCCESS == ret) {
        STF_U32 *pBuffer = (STF_U32*) g_pstTestScDumpMem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestScDumpMem = %p, "\
                "g_pstTestScDumpMem->pBUffer = %p (%p)\n",
                g_pstTestScDumpMem,
                g_pstTestScDumpMem->pBuffer, pBuffer
                );
            LOG_INFO("ScDump[%d] = 0x%08X, ScDump[%d] = 0x%08X, "\
                "ScDump[%d] = 0x%08X, ScDump[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestScDumpMem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestScDumpMem->u32BufSize - 8) / 4)],
                ((g_pstTestScDumpMem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestScDumpMem->u32BufSize - 4) / 4)]
                );

            pBuffer[4 / 4] = 0x12345678;
            pBuffer[((g_pstTestScDumpMem->u32BufSize - 4) / 4)] = 0x87654321;
            LOG_INFO("ScDump[%d] = 0x%08X, ScDump[%d] = 0x%08X, "\
                "ScDump[%d] = 0x%08X, ScDump[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestScDumpMem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestScDumpMem->u32BufSize - 8) / 4)],
                ((g_pstTestScDumpMem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestScDumpMem->u32BufSize - 4) / 4)]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestScDumpMem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestScDumpMem memory allocate and mapping is %s, "\
        "ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_ScDumpMemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestScDumpMem
        );

    LOG_INFO("g_pstTestScDumpMem = %p\n", g_pstTestScDumpMem);
    LOG_INFO("Test g_pstTestScDumpMem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_YHistMemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestYHistMem,
        0,
        0,
        CI_TYPE_Y_HIST,
        TYPE_NONE,
        PXL_NONE,
        MOSAIC_NONE,
        0,
        "YHist"
        );

    if (STF_SUCCESS == ret) {
        STF_U32 *pBuffer = (STF_U32*) g_pstTestYHistMem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestYHistMem = %p, g_pstTestYHistMem->pBUffer = "\
                "%p (%p)\n",
                g_pstTestYHistMem,
                g_pstTestYHistMem->pBuffer, pBuffer
                );
            LOG_INFO("YHist[%d] = 0x%08X, YHist[%d] = 0x%08X, "\
                "YHist[%d] = 0x%08X, YHist[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestYHistMem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestYHistMem->u32BufSize - 8) / 4)],
                ((g_pstTestYHistMem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestYHistMem->u32BufSize - 4) / 4)]
                );

            pBuffer[4 / 4] = 0x43218765;
            pBuffer[((g_pstTestYHistMem->u32BufSize - 4) / 4)] = 0x56781234;
            LOG_INFO("YHist[%d] = 0x%08X, YHist[%d] = 0x%08X, "\
                "YHist[%d] = 0x%08X, YHist[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestYHistMem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestYHistMem->u32BufSize - 8) / 4)],
                ((g_pstTestYHistMem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestYHistMem->u32BufSize - 4) / 4)]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestYHistMem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestYHistMem memory allocate and mapping is %s, "\
        "ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_YHistMemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestYHistMem
        );

    LOG_INFO("g_pstTestYHistMem = %p\n", g_pstTestYHistMem);
    LOG_INFO("Test g_pstTestYHistMem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_GenMemMemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestGenMemMem,
        0,
        0,
        CI_TYPE_GEN_MEM,
        TYPE_NONE,
        PXL_NONE,
        MOSAIC_NONE,
        (1920),
        "GenMem"
        );

    if (STF_SUCCESS == ret) {
        STF_U8 *pBuffer = (STF_U8*) g_pstTestGenMemMem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestGenMemMem = %p, "\
                "g_pstTestGenMemMem->pBUffer = %p (%p)\n",
                g_pstTestGenMemMem,
                g_pstTestGenMemMem->pBuffer, pBuffer
                );
            LOG_INFO("GenMem[%d] = 0x%08X, GenMem[%d] = 0x%08X, "\
                "GenMem[%d] = 0x%08X, GenMem[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestGenMemMem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestGenMemMem->u32BufSize - 8) / 4)],
                ((g_pstTestGenMemMem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestGenMemMem->u32BufSize - 4) / 4)]
                );

            pBuffer[4 / 4] = 0x38;
            pBuffer[((g_pstTestGenMemMem->u32BufSize - 4) / 4)] = 0x83;
            LOG_INFO("GenMem[%d] = 0x%08X, GenMem[%d] = 0x%08X, "\
                "GenMem[%d] = 0x%08X, GenMem[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestGenMemMem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestGenMemMem->u32BufSize - 8) / 4)],
                ((g_pstTestGenMemMem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestGenMemMem->u32BufSize - 4) / 4)]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestGenMemMem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestGenMemMem-> memory allocate and mapping is %s, "\
        "ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_GenMemMemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestGenMemMem
        );

    LOG_INFO("g_pstTestGenMemMem = %p\n", g_pstTestGenMemMem);
    LOG_INFO("Test g_pstTestGenMemMem memory free and unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

STF_RESULT Test_GenMemAlign128MemAlloc(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

#if 1
    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestGenMemAlign128Mem,
        0,
        0,
        CI_TYPE_GEN_MEM_ALIGN_128,
        TYPE_NONE,
        PXL_NONE,
        MOSAIC_NONE,
        (32 * 32 * 64 * sizeof(STF_U32)),
        "GenMemAlign128"
        );
#else
    ret = STFLIB_ISP_MemAlloc2(
        TEST_ISP_DEVICE,
        &g_pstTestGenMemAlign128Mem,
        128,
        128,
        CI_TYPE_GEN_MEM_ALIGN_128,
        TYPE_YUV,
        PXL_ISP_444IL3YCrCb16,
        MOSAIC_NONE,
        0,
        "GenMemAlign128"
        );
#endif

    if (STF_SUCCESS == ret) {
        STF_U8 *pBuffer = (STF_U8*) g_pstTestGenMemAlign128Mem->pBuffer;

        if (pBuffer) {
#ifdef TEST_USER_MODE_MEMORY_ACCESS
            LOG_INFO("g_pstTestGenMemAlign128Mem = %p, "\
                "g_pstTestGenMemAlign128Mem->pBUffer = %p (%p)\n",
                g_pstTestGenMemAlign128Mem,
                g_pstTestGenMemAlign128Mem->pBuffer, pBuffer
                );
            LOG_INFO("GenMemAlign128[%d] = 0x%08X, "\
                "GenMemAlign128[%d] = 0x%08X, GenMemAlign128[%d] = 0x%08X, "\
                "GenMemAlign128[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestGenMemAlign128Mem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestGenMemAlign128Mem->u32BufSize - 8) / 4)],
                ((g_pstTestGenMemAlign128Mem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestGenMemAlign128Mem->u32BufSize - 4) / 4)]
                );

            pBuffer[4 / 4] = 0x38;
            pBuffer[((g_pstTestGenMemAlign128Mem->u32BufSize - 4) / 4)] = 0x83;
            LOG_INFO("GenMemAlign128[%d] = 0x%08X, "\
                "GenMemAlign128[%d] = 0x%08X, GenMemAlign128[%d] = 0x%08X, "\
                "GenMemAlign128[%d] = 0x%08X\n",
                (0 / 4),
                pBuffer[(0 / 4)],
                (4 / 4),
                pBuffer[4 / 4],
                ((g_pstTestGenMemAlign128Mem->u32BufSize - 8) / 4),
                pBuffer[((g_pstTestGenMemAlign128Mem->u32BufSize - 8) / 4)],
                ((g_pstTestGenMemAlign128Mem->u32BufSize - 4) / 4),
                pBuffer[((g_pstTestGenMemAlign128Mem->u32BufSize - 4) / 4)]
                );
#endif //TEST_USER_MODE_MEMORY_ACCESS
        } else {
            LOG_ERROR("Allocate or mapped \"%s\" memory to user space "\
                "failed!\n",
                g_szAllocBufferType[g_pstTestGenMemAlign128Mem->enAllocBufferType]);
        }
    }

    LOG_INFO("Test g_pstTestGenMemAlign128Mem-> memory allocate and "\
        "mapping is %s, ret = %d(%s)\n",
        ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
        ret,
        STF_STR_ERROR(ret)
        );

    return ret;
}

STF_RESULT Test_GenMemAlign128MemFree(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    ret = STFLIB_ISP_MemFree2(
        TEST_ISP_DEVICE,
        &g_pstTestGenMemAlign128Mem
        );

    LOG_INFO("g_pstTestGenMemAlign128Mem = %p\n", g_pstTestGenMemAlign128Mem);
    LOG_INFO("Test g_pstTestGenMemAlign128Mem memory free and "\
        "unmapping is %s\n",
        ((STF_SUCCESS == ret) ? ("Successful") : ("Failed"))
        );

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT ISP_Mem_Test(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    //STFLIB_ISP_MemDebugInfoEnable(STF_ENABLE);

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("======== Allocate Memory ========\n");

#ifdef TEST_IOCTL_MEM_ALLOC_FREE_UO
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate UO memory start =====
    LOG_INFO("======== Test Uo memory ========\n");
    ret = Test_UoMemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate UO memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_UO
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_SS0
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate SS0 memory start =====
    LOG_INFO("======== Test Ss0 memory ========\n");
    ret = Test_Ss0MemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate SS0 memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_SS0
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_SS1
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate SS1 memory start =====
    LOG_INFO("======== Test Ss1 memory ========\n");
    ret = Test_Ss1MemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate SS1 memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_SS1
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_DUMP
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate CSI dump memory start =====
    LOG_INFO("======== Test Dump memory ========\n");
    ret = Test_DumpMemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate CSI dump memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_DUMP
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_TILING_1_WR
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate TIL_1_WR memory start =====
    LOG_INFO("======== Test TIL_1_WR memory ========\n");
    ret = Test_Tiling_1_WrMemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate TIL_1_WR memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_TILING_1_WR
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_SC_DUMP
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate ScDump memory start =====
    LOG_INFO("======== Test ScDump memory ========\n");
    ret = Test_ScDumpMemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate ScDump memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_SC_DUMP
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_Y_HIST
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate YHist memory start =====
    LOG_INFO("======== Test YHist memory ========\n");
    ret = Test_YHistMemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate YHist memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_Y_HIST
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate GEN_MEM memory start =====
    LOG_INFO("======== Test GenMem memory ========\n");
    ret = Test_GenMemMemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate GEN_MEM memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM_ALIGN_128
    //=========================================================================
    //-------------------------------------------------------------------------
    //=== Allocate GEN_MEM_ALIGN_128 memory start =====
    LOG_INFO("======== Test GenMemAlignt128 memory ========\n");
    ret = Test_GenMemAlign128MemAlloc();
    LOG_INFO("---------------------------------\n");
    if (ret) {
        LOG_ERROR("Stop the memory allocate/free test.\n");
        LOG_INFO("---------------------------------\n");
        //return ret;
        goto free_memory;
    }
    //=== Allocate GEN_MEM_ALIGN_128 memory end =====
    //LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM_ALIGN_128

free_memory:
    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("========== Free Memory ==========\n");

#ifdef TEST_IOCTL_MEM_ALLOC_FREE_UO
    ret = Test_UoMemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_UO
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_SS0
    ret = Test_Ss0MemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_SS0
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_SS1
    ret = Test_Ss1MemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_SS1
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_DUMP
    ret = Test_DumpMemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_DUMP
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_TILING_1_WR
    ret = Test_Tiling_1_WrMemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_TILING_1_WR
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_SC_DUMP
    ret = Test_ScDumpMemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_SC_DUMP
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_Y_HIST
    ret = Test_YHistMemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_Y_HIST
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM
    ret = Test_GenMemMemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_GEM_MEM
#ifdef TEST_IOCTL_MEM_ALLOC_FREE_GEN_MEM_ALIGN_128
    ret = Test_GenMemAlign128MemFree();
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

#endif //TEST_IOCTL_MEM_ALLOC_FREE_GEM_MEM_ALIGN_128
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

    //STFLIB_ISP_MemDebugInfoEnable(STF_DISABLE);

    return ret;
}

//-----------------------------------------------------------------------------
