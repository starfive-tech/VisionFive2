/**
  ******************************************************************************
  * @file  ISP_reg_test.c
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

#define LOG_TAG "Reg_Test"
#include "stf_common/userlog.h"

#include "ISP_reg_test.h"
#include "ISP_test.h"


#if defined(V4L2_DRIVER)
#define IMAP_ISP_REG_SIZE               (1 * 0x00001000LL)
#else
#define IMAP_ISP_REG_SIZE               (1 * 0x00010000LL)
#endif //#if defined(V4L2_DRIVER)


/* ISPC Register driver test structure */


/* ISPC Register driver test variables */
const STF_U32 g_u32RegAddr[] = {
    0x000002E0, 0x000002E4, 0x000002E8, 0x000002EC,
    0x000002F0, 0x000002F4, 0x000002F8, 0x000002FC,
    0x000000C4, 0x000000C8, 0x000000CC, 0x000000D0,
    0x000000D4, 0x000004D0, 0x000004D8, 0x000004E0,
    0x000004E8, 0x000004F0, 0x000004F8, 0x00000500,
    0x00000508, 0x00000510, 0x00000518, 0x00000520,
    0x00000528, 0x00000530, 0x00000538, 0x0000053C,
    0x00000540, 0x00000544, 0x00000548, 0x0000054C,
    0x000005D0, 0x000005D4, 0x00000548, 0x0000054C,
};
#define REG_ADDR_RAND_SIZE      (sizeof(g_u32RegAddr) / sizeof(STF_U32))


/* ISPC Register driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_ReadOne(
    STF_VOID
    )
{
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    STF_RESULT ret = STF_SUCCESS;

#if defined(ENABLE_USING_MEM_TO_SIM_REG)
    u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
#else
    u32Offset = ((rand() & (IMAP_ISP_REG_SIZE - 1)) & 0xFFFFFFFC);
#endif //ENABLE_USING_MEM_TO_SIM_REG
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );

    return ret;
}

STF_RESULT Test_ReadSeries(
    STF_VOID
    )
{
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[10];
    STF_RESULT ret = STF_SUCCESS;

#if defined(ENABLE_USING_MEM_TO_SIM_REG)
    u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
    u32Length = 10;
    if (PESUDO_REG_SIZE < (u32Offset + (u32Length * 4))) {
        u32Offset = PESUDO_REG_SIZE - (u32Length * 4);
    }
#else
    u32Offset = ((rand() & (IMAP_ISP_REG_SIZE - 1)) & 0xFFFFFFFC);
    u32Length = 10;
    if (IMAP_ISP_REG_SIZE < (u32Offset + (u32Length * 4))) {
        u32Offset = IMAP_ISP_REG_SIZE - (u32Length * 4);
    }
#endif //ENABLE_USING_MEM_TO_SIM_REG
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );

    return ret;
}

STF_RESULT Test_ReadModule(
    STF_VOID
    )
{
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[10];
    STF_RESULT ret = STF_SUCCESS;

#if defined(ENABLE_USING_MEM_TO_SIM_REG)
    u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
    u32Length = 10;
    if (PESUDO_REG_SIZE < (u32Offset + (u32Length * 4))) {
        u32Offset = PESUDO_REG_SIZE - (u32Length * 4);
    }
#else
    u32Offset = ((rand() & (IMAP_ISP_REG_SIZE - 1)) & 0xFFFFFFFC);
    u32Length = 10;
    if (IMAP_ISP_REG_SIZE < (u32Offset + (u32Length * 4))) {
        u32Offset = IMAP_ISP_REG_SIZE - (u32Length * 4);
    }
#endif //ENABLE_USING_MEM_TO_SIM_REG
    ret = STFLIB_ISP_RegReadModule(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );

    return ret;
}

STF_RESULT Test_ReadByTable(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[10 * 2];
    STF_RESULT ret = STF_SUCCESS;

    u32Length = 10;
    for (i = 0; i < u32Length; i++) {
#if defined(ENABLE_USING_MEM_TO_SIM_REG)
        do {
            u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
        } while (PESUDO_REG_SIZE < u32Offset);
#else
        do {
            u32Offset = ((rand() & (IMAP_ISP_REG_SIZE - 1)) & 0xFFFFFFFC);
        } while (IMAP_ISP_REG_SIZE < u32Offset);
#endif //ENABLE_USING_MEM_TO_SIM_REG
        u32Buffer[i * 2] = u32Offset;
        u32Buffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );

    return ret;
}

STF_RESULT Test_ReadByTable2(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[10 * 3];
    STF_RESULT ret = STF_SUCCESS;

    u32Length = 10;
    for (i = 0; i < u32Length; i++) {
#if defined(ENABLE_USING_MEM_TO_SIM_REG)
        do {
            u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
        } while (PESUDO_REG_SIZE < u32Offset);
#else
        do {
            u32Offset = ((rand() & (IMAP_ISP_REG_SIZE - 1)) & 0xFFFFFFFC);
        } while (IMAP_ISP_REG_SIZE < u32Offset);
#endif //ENABLE_USING_MEM_TO_SIM_REG
        u32Buffer[i * 3] = u32Offset;
        u32Buffer[i * 3 + 1] = 0;
        u32Buffer[i * 3 + 2] = 0xFFFFFFFF;
    }
    ret = STFLIB_ISP_RegReadByTable2(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL_2 *)&u32Buffer[0]
        );

    return ret;
}

STF_RESULT Test_ReadByTable3(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[10 * 4];
    STF_RESULT ret = STF_SUCCESS;

    u32Length = 10;
    for (i = 0; i < u32Length; i++) {
#if defined(ENABLE_USING_MEM_TO_SIM_REG)
        do {
            u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
        } while (PESUDO_REG_SIZE < u32Offset);
#else
        do {
            u32Offset = ((rand() & (IMAP_ISP_REG_SIZE - 1)) & 0xFFFFFFFC);
        } while (IMAP_ISP_REG_SIZE < u32Offset);
#endif //ENABLE_USING_MEM_TO_SIM_REG
        u32Buffer[i * 4] = u32Offset;
        u32Buffer[i * 4 + 1] = 0;
        u32Buffer[i * 4 + 2] = 0xFFFFFFFF;
        u32Buffer[i * 4 + 2] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable3(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL_3 *)&u32Buffer[0]
        );

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT Test_WriteOne(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32ReadBuffer[1];
    STF_U32 u32WriteBuffer[1];
    STF_RESULT ret = STF_SUCCESS;

#if defined(ENABLE_USING_MEM_TO_SIM_REG)
    u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
#else
    u32Offset = g_u32RegAddr[rand() % REG_ADDR_RAND_SIZE];
#endif //ENABLE_USING_MEM_TO_SIM_REG
    u32Length = 1;
    for (i = 0; i < u32Length; i++) {
        u32WriteBuffer[i] = rand();
    }
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32WriteBuffer[0]
        );
    if (ret) {
        return ret;
    }
    u32ReadBuffer[0] = 0;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32ReadBuffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[i] != u32WriteBuffer[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]==0x%08X\n",
                u32Offset,
                i,
                u32ReadBuffer[i],
                i,
                u32WriteBuffer[i]
                );
            break;
        }
    }

    return ret;
}

STF_RESULT Test_WriteSeries(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32ReadBuffer[10];
    STF_U32 u32WriteBuffer[10];
    STF_RESULT ret = STF_SUCCESS;

#if defined(ENABLE_USING_MEM_TO_SIM_REG)
    u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
    u32Length = 10;
    if (PESUDO_REG_SIZE < (u32Offset + (u32Length * 4))) {
        u32Offset = PESUDO_REG_SIZE - (u32Length * 4);
    }
#else
    i = rand() % 3;
    if (0 == i) {
        u32Offset = 0x000002E0;
        u32Length = 8;
    } else if (1 == i) {
        u32Offset = 0x000000C4;
        u32Length = 4;
    } else {
        u32Offset = 0x00000538;
        u32Length = 6;
    }
#endif //ENABLE_USING_MEM_TO_SIM_REG
    for (i = 0; i < u32Length; i++) {
        u32WriteBuffer[i] = rand();
    }
    ret = STFLIB_ISP_RegWriteSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32WriteBuffer
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        u32ReadBuffer[i] = 0;
    }
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32ReadBuffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[i] != u32WriteBuffer[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]==0x%08X\n",
                u32Offset + (i * 4),
                i,
                u32ReadBuffer[i],
                i,
                u32WriteBuffer[i]
                );
            break;
        }
    }

    return ret;
}

STF_RESULT Test_WriteModule(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32ReadBuffer[10];
    STF_U32 u32WriteBuffer[10];
    STF_RESULT ret = STF_SUCCESS;

#if defined(ENABLE_USING_MEM_TO_SIM_REG)
    u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
    u32Length = 10;
    if (PESUDO_REG_SIZE < (u32Offset + (u32Length * 4))) {
        u32Offset = PESUDO_REG_SIZE - (u32Length * 4);
    }
#else
  #if defined(V4L2_DRIVER)
    i = rand() % 3;
    if (0 == i) {
        u32Offset = 0x000002E0;
        u32Length = 8;
    } else if (1 == i) {
        u32Offset = 0x000000C4;
        u32Length = 4;
    } else {
        u32Offset = 0x00000538;
        u32Length = 6;
    }
  #else
    u32Offset = ((rand() & (IMAP_ISP_REG_SIZE - 1)) & 0xFFFFFFFC);
    u32Length = 10;
    if (IMAP_ISP_REG_SIZE < (u32Offset + (u32Length * 4))) {
        u32Offset = IMAP_ISP_REG_SIZE - (u32Length * 4);
    }
  #endif //#if defined(V4L2_DRIVER)
#endif //ENABLE_USING_MEM_TO_SIM_REG
    for (i = 0; i < u32Length; i++) {
        u32WriteBuffer[i] = rand();
    }
    ret = STFLIB_ISP_RegWriteModule(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32WriteBuffer
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        u32ReadBuffer[i] = 0;
    }
    ret = STFLIB_ISP_RegReadModule(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32ReadBuffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[i] != u32WriteBuffer[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]==0x%08X\n",
                u32Offset + (i * 4),
                i,
                u32ReadBuffer[i],
                i,
                u32WriteBuffer[i]
                );
            break;
        }
    }

    return ret;
}

STF_RESULT Test_WriteByTable(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32ReadBuffer[10 * 2];
    STF_U32 u32WriteBuffer[10 * 2];
    STF_U32 u32Idx;
    STF_BOOL8 bLoop;
    STF_U32 u32Addr[20] = { 0xFFFFFFFF };
    STF_RESULT ret = STF_SUCCESS;

    u32Length = 10;
#if defined(ENABLE_USING_MEM_TO_SIM_REG)
    for (i = 0; i < u32Length; i++) {
        do {
            u32Offset = ((rand() & (PESUDO_REG_SIZE - 1)) & 0xFFFFFFFC);
        } while (PESUDO_REG_SIZE < u32Offset);
        u32WriteBuffer[i * 2] = u32Offset;
        u32WriteBuffer[i * 2 + 1] = rand();
    }
#else
    for (i = 0; i < u32Length; i++) {
        bLoop = STF_TRUE;
        do {
             u32Offset = g_u32RegAddr[rand() % REG_ADDR_RAND_SIZE];
             for (u32Idx = 0; u32Idx < i; u32Idx++) {
                 if (u32Offset == u32WriteBuffer[u32Idx * 2]) {
                     break;
                 }
             }
             if (u32Idx >= i) {
                 bLoop = STF_FALSE;
             }
        } while (bLoop);
        u32WriteBuffer[i * 2] = u32Offset;
        u32WriteBuffer[i * 2 + 1] = rand();
    }
#endif //ENABLE_USING_MEM_TO_SIM_REG
    ret = STFLIB_ISP_RegWriteByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32WriteBuffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        u32ReadBuffer[i * 2] = u32WriteBuffer[i * 2];
        u32ReadBuffer[i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32ReadBuffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[i * 2 + 1] != u32WriteBuffer[i * 2 + 1]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]=0x%08X\n",
                u32ReadBuffer[i * 2],
                i,
                u32ReadBuffer[i * 2 + 1],
                i,
                u32WriteBuffer[i * 2 + 1]
                );
            break;
        }
    }

    return ret;
}

STF_RESULT Test_WriteByTable2(
    STF_VOID
    )
{
    const STF_U32 u32RegValueTable[ST_CCM_REG_LEN][2] = {
        { 0x00060000, 0x000707FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x000000A6, 0x000007FF },
        { 0x000007CD, 0x000007FF },
        { 0x0000000D, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x0000009A, 0x000007FF },
        { 0x000007E6, 0x000007FF },
        { 0x00000026, 0x000007FF },
        { 0x0000074D, 0x000007FF },
        { 0x0000010D, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000000, 0x000007FF },
        { 0x00000796, 0x000007FF },
        { 0x0000023B, 0x000007FF },
    };
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    ST_REG_TBL_2 stReadBuffer[ST_CCM_REG_LEN];
    ST_REG_TBL_2 stWriteBuffer[ST_CCM_REG_LEN];
    STF_RESULT ret = STF_SUCCESS;

    u32Offset = ADDR_REG_ICAMD_0;
    u32Length = ST_CCM_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        stWriteBuffer[i].u32Offset = u32Offset + (i * 4);
        stWriteBuffer[i].u32Value = u32RegValueTable[i][0];
        stWriteBuffer[i].u32Mask = u32RegValueTable[i][1];
    }
    ret = STFLIB_ISP_RegWriteByTable2(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL_2 *)&stWriteBuffer
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        stReadBuffer[i].u32Offset = stWriteBuffer[i].u32Offset;
        stReadBuffer[i].u32Value = 0;
        stReadBuffer[i].u32Mask = stWriteBuffer[i].u32Mask;
    }
    ret = STFLIB_ISP_RegReadByTable2(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL_2 *)&stReadBuffer
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        if (stReadBuffer[i].u32Value != stWriteBuffer[i].u32Value) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]=0x%08X (Mask=0x%08X)\n",
                stReadBuffer[i].u32Offset,
                i,
                stReadBuffer[i].u32Value,
                i,
                stWriteBuffer[i].u32Value,
                stWriteBuffer[i].u32Mask
                );
            break;
        }
    }

    return ret;
}

STF_RESULT Test_WriteByTable3(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    ST_REG_TBL_3 stReadBuffer[ST_GMARGB_REG_LEN];
    ST_REG_TBL_3 stWriteBuffer[ST_GMARGB_REG_LEN];
    STF_RESULT ret = STF_SUCCESS;

    u32Offset = ADDR_REG_IGRVAL_0;
    u32Length = ST_GMARGB_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        stWriteBuffer[i].u32Offset = u32Offset + (i * 4);
        stWriteBuffer[i].u32Value = (rand() & 0x03FF)
            | ((rand() & 0xFFFF) << 16);;
        stWriteBuffer[i].u32Mask = 0xFFFF03FF;
        stWriteBuffer[i].u32DelayMs = 1;
    }
    ret = STFLIB_ISP_RegWriteByTable3(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL_3 *)&stWriteBuffer
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        stReadBuffer[i].u32Offset = stWriteBuffer[i].u32Offset;
        stReadBuffer[i].u32Value = 0;
        stReadBuffer[i].u32Mask = stWriteBuffer[i].u32Mask;
        stReadBuffer[i].u32DelayMs = stWriteBuffer[i].u32DelayMs;
    }
    ret = STFLIB_ISP_RegReadByTable3(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL_3 *)&stReadBuffer
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        if (stReadBuffer[i].u32Value != stWriteBuffer[i].u32Value) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]=0x%08X (Mask=0x%08X, Delay=%dms)\n",
                stReadBuffer[i].u32Offset,
                i,
                stReadBuffer[i].u32Value,
                i,
                stWriteBuffer[i].u32Value,
                stWriteBuffer[i].u32Mask,
                stWriteBuffer[i].u32DelayMs
                );
            break;
        }
    }

    return ret;
}

STF_RESULT Test_WriteBySimplePack(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    ST_SAT_RDMA stSatRdma;
    STF_U32 u32ReadBuffer[256];
    STF_U32 *pu32Value = NULL;
    STF_RESULT ret = STF_SUCCESS;

    stSatRdma.stRdmaStartCmd.Tag = MREG_WR;
    stSatRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ISAT0;
    stSatRdma.stRdmaStartCmd.u32Param = ST_SAT_CHROMA_REG_LEN;
    stSatRdma.stRdmaYCmd.Tag = MREG_WR;
    stSatRdma.stRdmaYCmd.RegAddr = ADDR_REG_IYADJ0;
    stSatRdma.stRdmaYCmd.u32Param = ST_SAT_Y_REG_LEN;
    stSatRdma.stRdmaEndCmd.Tag = DESP_END;
    stSatRdma.stRdmaEndCmd.u32Param = 0;

    stSatRdma.stSatChromaReg.Gain.value =
        (rand() & 0x7FF) | ((rand() & 0xFFF) << 16);
    stSatRdma.stSatChromaReg.Threshold.value =
        (rand() & 0x7FF) | ((rand() & 0xFFF) << 16);
    stSatRdma.stSatChromaReg.UvOffset.value =
        (rand() & 0x7FF) | ((rand() & 0x7FF) << 16);
    stSatRdma.stSatChromaReg.HueFactor.value =
        (rand() & 0x1FF) | ((rand() & 0x3FF) << 16);
    stSatRdma.stSatChromaReg.ScalingCoefficient.value =
        (rand() & 0xF);
    stSatRdma.stSatYReg.YAdjustment0.value =
        (rand() & 0x3FF) | ((rand() & 0x3FFF) << 16);
    stSatRdma.stSatYReg.YAdjustment1.value =
        (rand() & 0x3FF) | ((rand() & 0x3FF) << 16);

    u32Offset = 0;
    u32Length = ST_SAT_RDMA_LEN - 1;
    ret = STFLIB_ISP_RegWriteBySimplePack(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_RDMA_CMD *)&stSatRdma
        );
    if (ret) {
        return ret;
    }
    u32Length = ST_SAT_REG_LEN + 4;
    for (i = 0; i < u32Length; i++) {
        u32ReadBuffer[i] = 0;
    }
    u32Offset = ADDR_REG_ISAT0;
    u32Length = ST_SAT_REG_LEN + 4;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32ReadBuffer[0]
        );
    if (ret) {
        return ret;
    }
    u32Offset = ADDR_REG_ISAT0;
    u32Length = ST_SAT_CHROMA_REG_LEN;
    pu32Value = (STF_U32 *)&stSatRdma.stSatChromaReg;
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[i] != pu32Value[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("stSatChromaReg - Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]=0x%08X\n",
                u32Offset + (i * 4),
                i,
                u32ReadBuffer[i],
                i,
                pu32Value[i]
                );
            //break;
        }
    }
    u32Offset = ADDR_REG_IYADJ0;
    u32Length = ST_SAT_Y_REG_LEN;
    pu32Value = (STF_U32 *)&stSatRdma.stSatYReg.YAdjustment0.value;
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[ST_SAT_CHROMA_REG_LEN + 4 + i] != pu32Value[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("stSatYReg - Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "WriteBuffer[%d]=0x%08X\n",
                u32Offset + (i * 4),
                i,
                u32ReadBuffer[ST_SAT_CHROMA_REG_LEN + 4 + i],
                i,
                pu32Value[i]
                );
            //break;
        }
    }

    return ret;
}

STF_RESULT Test_WriteBySoftRdma(
    STF_VOID
    )
{
    const STF_U32 u32GmaRgb[ST_GMARGB_REG_LEN] = {
        0x24000000, 0x159500A5, 0x0F9900EE, 0x0CE40127,
        0x0B410157, 0x0A210181, 0x094B01A8, 0x08A401CC,
        0x081D01EE, 0x06B20263, 0x05D802C7, 0x05420320,
        0x04D30370, 0x047C03BB, 0x043703FF,
    };
    const STF_U32 u32Sat[ST_SAT_REG_LEN] = {
        0x00000133, 0x001F0001, 0x00000000, 0x00000100,
        0x00000008, 0x04020001, 0x03FF0000,
    };
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 *pu32RdmaData = NULL;
    ST_GMARGB_RDMA *pstGmaRgbRdma = NULL;
    ST_SAT_RDMA *pstSatRdma = NULL;
    CI_MEM_PARAM stGmaRgbDmaBuf = { 0 };
    CI_MEM_PARAM stSatDmaBuf = { 0 };
    STF_U32 u32ReadBuffer[256];
    STF_RESULT ret = STF_SUCCESS;

    //----------------------------------------------
    ret = STFLIB_ISP_MemAlloc(
        TEST_ISP_DEVICE,
        &stGmaRgbDmaBuf,
        0,
        0,
        CI_TYPE_GEN_MEM,
        TYPE_NONE,
        PXL_NONE,
        MOSAIC_NONE,
        ST_GMARGB_RDMA_SIZE,
        "GmaRgbRdma"
        );
    if (0 > ret) {
        LOG_ERROR("Failed to allocate ST_GMARGB_RDMA, ret = %d\n", ret);
        goto Test_WriteBySoftRdma_Exit_GmaRgb_Free;
    }
    pstGmaRgbRdma = stGmaRgbDmaBuf.pBuffer;
    //----------------------------------------------
    ret = STFLIB_ISP_MemAlloc(
        TEST_ISP_DEVICE,
        &stSatDmaBuf,
        0,
        0,
        CI_TYPE_GEN_MEM,
        TYPE_NONE,
        PXL_NONE,
        MOSAIC_NONE,
        ST_SAT_RDMA_SIZE,
        "SatRdma"
        );
    if (0 > ret) {
        LOG_ERROR("Failed to allocate ST_SAT_RDMA, ret = %d\n", ret);
        goto Test_WriteBySoftRdma_Exit_Sat_Free;
    }
    pstSatRdma = stSatDmaBuf.pBuffer;
    //----------------------------------------------
    // Initial pstGmaRgbRdma
    pstGmaRgbRdma->stRdmaStartCmd.Tag = MREG_WR;
    pstGmaRgbRdma->stRdmaStartCmd.RegAddr = ADDR_REG_IGRVAL_0;
    pstGmaRgbRdma->stRdmaStartCmd.u32Param = ST_GMARGB_REG_LEN;
    u32Length = ST_GMARGB_REG_LEN;
    pu32RdmaData = (STF_U32 *)(&pstGmaRgbRdma->stGmaRgbReg);
    for (i = 0; i < u32Length; i++) {
        pu32RdmaData[i] = u32GmaRgb[i];
    }
    pstGmaRgbRdma->stRdmaEndCmd.Tag = DESP_LNK;
    pstGmaRgbRdma->stRdmaEndCmd.u32Param =
        stSatDmaBuf.stSysMem.pAlloc->u32PhysAddress;
    //----------------------------------------------
    // Initial pstSatRdma
    pstSatRdma->stRdmaStartCmd.Tag = MREG_WR;
    pstSatRdma->stRdmaStartCmd.RegAddr = ADDR_REG_ISAT0;
    pstSatRdma->stRdmaStartCmd.u32Param = ST_SAT_CHROMA_REG_LEN;
    pstSatRdma->stRdmaYCmd.Tag = MREG_WR;
    pstSatRdma->stRdmaYCmd.RegAddr = ADDR_REG_IYADJ0;
    pstSatRdma->stRdmaYCmd.u32Param = ST_SAT_Y_REG_LEN;
    u32Length = ST_SAT_CHROMA_REG_LEN;
    pu32RdmaData = (STF_U32 *)(&pstSatRdma->stSatChromaReg);
    for (i = 0; i < u32Length; i++) {
        pu32RdmaData[i] = u32Sat[i];
    }
    u32Length = ST_SAT_Y_REG_LEN;
    pu32RdmaData = (STF_U32 *)(&pstSatRdma->stSatYReg);
    for (i = 0; i < u32Length; i++) {
        pu32RdmaData[i] = u32Sat[ST_SAT_CHROMA_REG_LEN + i];
    }
    pstSatRdma->stRdmaEndCmd.Tag = DESP_END;
    pstSatRdma->stRdmaEndCmd.u32Param = 0;
    //----------------------------------------------
    // Test SOFT-RDMA method
    u32Offset = stGmaRgbDmaBuf.stSysMem.pAlloc->u32PhysAddress;
    ret = STFLIB_ISP_RegWriteBySoftRdma(
        TEST_ISP_DEVICE,
        u32Offset
        );
    if (ret) {
        goto Test_WriteBySoftRdma_Exit_Sat_Free;
    }
    //----------------------------------------------
    // Check GmaRgb value
    u32Length = ST_GMARGB_REG_LEN;
    for (i = 0; i < u32Length; i++) {
        u32ReadBuffer[i] = 0;
    }
    u32Offset = ADDR_REG_IGRVAL_0;
    u32Length = ST_GMARGB_REG_LEN;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32ReadBuffer[0]
        );
    if (ret) {
        goto Test_WriteBySoftRdma_Exit_Sat_Free;
    }
    u32Offset = ADDR_REG_IGRVAL_0;
    u32Length = ST_GMARGB_REG_LEN;
    pu32RdmaData = (STF_U32 *)(&pstGmaRgbRdma->stGmaRgbReg);
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[i] != pu32RdmaData[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "stGmaRgbReg[%d]==0x%08X\n",
                u32Offset + (i * 4),
                i,
                u32ReadBuffer[i],
                i,
                pu32RdmaData[i]
                );
            break;
        }
    }
    //----------------------------------------------
    // Check Sat value
    u32Length = ST_SAT_REG_LEN + 4;
    for (i = 0; i < u32Length; i++) {
        u32ReadBuffer[i] = 0;
    }
    u32Offset = ADDR_REG_ISAT0;
    u32Length = ST_SAT_REG_LEN + 4;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32ReadBuffer[0]
        );
    if (ret) {
        goto Test_WriteBySoftRdma_Exit_Sat_Free;
    }
    u32Offset = ADDR_REG_ISAT0;
    u32Length = ST_SAT_CHROMA_REG_LEN;
    pu32RdmaData = (STF_U32 *)(&pstSatRdma->stSatChromaReg);
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[i] != pu32RdmaData[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "stSatReg[%d]==0x%08X\n",
                u32Offset + (i * 4),
                i,
                u32ReadBuffer[i],
                i,
                pu32RdmaData[i]
                );
            break;
        }
    }
    u32Offset = ADDR_REG_IYADJ0;
    u32Length = ST_SAT_Y_REG_LEN;
    pu32RdmaData = (STF_U32 *)(&pstSatRdma->stSatYReg);
    for (i = 0; i < u32Length; i++) {
        if (u32ReadBuffer[ST_SAT_CHROMA_REG_LEN + 4 + i] != pu32RdmaData[i]) {
            ret = STF_ERROR_FAILURE;
            LOG_ERROR("Addr=0x%08X, ReadBuffer[%d]=0x%08X != "\
                "stSatReg[%d]==0x%08X\n",
                u32Offset + (i * 4),
                i,
                u32ReadBuffer[ST_SAT_CHROMA_REG_LEN + 4 + i],
                i,
                pu32RdmaData[i]
                );
            break;
        }
    }
    //----------------------------------------------

Test_WriteBySoftRdma_Exit_Sat_Free:
    ret = STFLIB_ISP_MemFree(
        TEST_ISP_DEVICE,
        &stSatDmaBuf
        );

Test_WriteBySoftRdma_Exit_GmaRgb_Free:
    ret = STFLIB_ISP_MemFree(
        TEST_ISP_DEVICE,
        &stGmaRgbDmaBuf
        );


    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT ISP_Reg_Test(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    //STFLIB_ISP_RegDebugInfoEnable(STF_ENABLE);
    //STFLIB_ISP_MemDebugInfoEnable(STF_ENABLE);

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_ReadOne(
            );
        LOG_INFO("Test register read one method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_ReadSeries(
            );
        LOG_INFO("Test register read series method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_ReadModule(
            );
        LOG_INFO("Test register read module method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
//        if (ret) {
//        LOG_ERROR("Stop the registers access test.\n");
//            return ret;
//        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_ReadByTable(
            );
        LOG_INFO("Test register read by table method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_ReadByTable2(
            );
        LOG_INFO("Test register read by table 2 method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_ReadByTable3(
            );
        LOG_INFO("Test register read by table method 3 is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_WriteOne(
            );
        LOG_INFO("Test register write one method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_WriteSeries(
            );
        LOG_INFO("Test register write series method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_WriteModule(
            );
        LOG_INFO("Test register write module method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
//        if (ret) {
//        LOG_ERROR("Stop the registers access test.\n");
//            return ret;
//        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_WriteByTable(
            );
        LOG_INFO("Test register write by table method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_WriteByTable2(
            );
        LOG_INFO("Test register write by table 2 method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_WriteByTable3(
            );
        LOG_INFO("Test register write by table method 3 is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_WriteBySimplePack(
            );
        LOG_INFO("Test register write by simple pack method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    {
        ret = Test_WriteBySoftRdma(
            );
        LOG_INFO("Test register write by Soft Rdma method is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the registers access test.\n");
            return ret;
        }
    }
    //-------------------------------------------------------------------------
    //LOG_INFO("=================================\n");
    //=========================================================================

    //STFLIB_ISP_MemDebugInfoEnable(STF_DISABLE);
    //STFLIB_ISP_RegDebugInfoEnable(STF_DISABLE);

    return ret;
}

//-----------------------------------------------------------------------------
