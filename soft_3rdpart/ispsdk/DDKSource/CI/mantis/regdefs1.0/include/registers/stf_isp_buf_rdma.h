/**
  ******************************************************************************
  * @file  stf_isp_buf_rdma.h
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
#ifndef __STF_ISP_BUF_RDMA_H__
#define __STF_ISP_BUF_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add ISP video output, dump, tiling read/write and statistical collection buffer interface registers structure */
#if 0
typedef struct _ST_BUF_INFO {
    STF_BOOL8 bEnable;
    STF_U32 u32YAddr;
    STF_U32 u32UvAddr;
} ST_BUF_INFO, *PST_BUF_INFO;

//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ST_BUFS_INFO {
    ST_BUF_INFO stUoBufInfo;
    ST_BUF_INFO stSs0BufInfo;
    ST_BUF_INFO stSs1BufInfo;
    ST_BUF_INFO stDumpBufInfo;
    ST_BUF_INFO stTiling_1_RdBufInfo;
    ST_BUF_INFO stTiling_1_WrBufInfo;
    ST_BUF_INFO stScDumpBufInfo;
    ST_BUF_INFO stYHistBufInfo;
} ST_BUFS_INFO, *PST_BUFS_INFO;

#endif
//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ST_BUF_UO_REG {
    REG_UOAY UoYAddr;               // 0x0A80, Unscaled Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_UOAUV UoUvAddr;             // 0x0A84, Unscaled Output Image UV Plane Start Address Register, 8-byte alignment.
} ST_BUF_UO_REG, *PST_BUF_UO_REG;

#define ST_BUF_UO_REG_SIZE          (sizeof(ST_BUF_UO_REG))
#define ST_BUF_UO_REG_LEN           (ST_BUF_UO_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_SS0_REG {
    REG_SS0AY Ss0YAddr;             // 0x0A94, SS0 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS0AUV Ss0UvAddr;           // 0x0A98, SS0 Output Image UV Plane Start Address Register, 8-byte alignment.
} ST_BUF_SS0_REG, *PST_BUF_SS0_REG;

#define ST_BUF_SS0_REG_SIZE         (sizeof(ST_BUF_SS0_REG))
#define ST_BUF_SS0_REG_LEN          (ST_BUF_SS0_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_SS1_REG {
    REG_SS1AY Ss1YAddr;             // 0x0AAC, SS1 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS1AUV Ss1UvAddr;           // 0x0AB0, SS1 Output Image UV Plane Start Address Register, 8-byte alignment.
} ST_BUF_SS1_REG, *PST_BUF_SS1_REG;

#define ST_BUF_SS1_REG_SIZE         (sizeof(ST_BUF_SS1_REG))
#define ST_BUF_SS1_REG_LEN          (ST_BUF_SS1_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_DUMP_REG {
    REG_DUMP_CFG_0 DumpBufAddr;     // 0x0024, base address for captured image, 128-byte alignment.
} ST_BUF_DUMP_REG, *PST_BUF_DUMP_REG;

#define ST_BUF_DUMP_REG_SIZE        (sizeof(ST_BUF_DUMP_REG))
#define ST_BUF_DUMP_REG_LEN         (ST_BUF_DUMP_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_TIL_1_RD_REG {
    REG_ITIDRYSAR Til_1_ReadYAddr;  // 0x0B30, TIL 1 DRAM read Y start address register, 8-byte alignment.
    REG_ITIDRUSAR Til_1_ReadUvAddr; // 0x0B34, TIL 1 DRAM read UV start address register, 8-byte alignment.
} ST_BUF_TIL_1_RD_REG, *PST_BUF_TIL_1_RD_REG;

#define ST_BUF_TIL_1_RD_REG_SIZE    (sizeof(ST_BUF_TIL_1_RD_REG))
#define ST_BUF_TIL_1_RD_REG_LEN     (ST_BUF_TIL_1_RD_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_TIL_1_WR_REG {
    REG_ITIDWYSAR Til_1_WriteYAddr; // 0x0B28, TIL 1 DRAM write Y start address register, 8-byte alignment.
    REG_ITIDWUSAR Til_1_WriteUvAddr;// 0x0B2C, TIL 1 DRAM write UV start address register, 8-byte alignment.
} ST_BUF_TIL_1_WR_REG, *PST_BUF_TIL_1_WR_REG;

#define ST_BUF_TIL_1_WR_REG_SIZE    (sizeof(ST_BUF_TIL_1_WR_REG))
#define ST_BUF_TIL_1_WR_REG_LEN     (ST_BUF_TIL_1_WR_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_SC_DUMP_REG {
    REG_SCD_CFG_0 ScDumpBufAddr;    // 0x0098, SC dumping base address.
} ST_BUF_SC_DUMP_REG, *PST_BUF_SC_DUMP_REG;

#define ST_BUF_SC_DUMP_REG_SIZE     (sizeof(ST_BUF_SC_DUMP_REG))
#define ST_BUF_SC_DUMP_REG_LEN      (ST_BUF_SC_DUMP_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_YHIST_REG {
    REG_YHIST_CFG_4 YHistBufAddr;   // 0x0CD8, Y Histogram dumping base address.
} ST_BUF_YHIST_REG, *PST_BUF_YHIST_REG;

#define ST_BUF_YHIST_REG_SIZE       (sizeof(ST_BUF_YHIST_REG))
#define ST_BUF_YHIST_REG_LEN        (ST_BUF_YHIST_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_BUF_REG {
    REG_UOAY UoYAddr;               // 0x0A80, Unscaled Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_UOAUV UoUvAddr;             // 0x0A84, Unscaled Output Image UV Plane Start Address Register, 8-byte alignment.

    REG_SS0AY Ss0YAddr;             // 0x0A94, SS0 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS0AUV Ss0UvAddr;           // 0x0A98, SS0 Output Image UV Plane Start Address Register, 8-byte alignment.

    REG_SS1AY Ss1YAddr;             // 0x0AAC, SS1 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS1AUV Ss1UvAddr;           // 0x0AB0, SS1 Output Image UV Plane Start Address Register, 8-byte alignment.

    REG_DUMP_CFG_0 DumpBufAddr;     // 0x0024, base address for captured image, 128-byte alignment.

    REG_ITIDRYSAR Til_1_ReadYAddr;  // 0x0B30, TIL 1 DRAM read Y start address register, 8-byte alignment.
    REG_ITIDRUSAR Til_1_ReadUvAddr; // 0x0B34, TIL 1 DRAM read UV start address register, 8-byte alignment.

    REG_ITIDWYSAR Til_1_WriteYAddr; // 0x0B28, TIL 1 DRAM write Y start address register, 8-byte alignment.
    REG_ITIDWUSAR Til_1_WriteUvAddr;// 0x0B2C, TIL 1 DRAM write UV start address register, 8-byte alignment.

    REG_SCD_CFG_0 ScDumpBufAddr;    // 0x0098, SC dumping base address.

    REG_YHIST_CFG_4 YHistBufAddr;   // 0x0CD8, Y Histogram dumping base address.
} ST_BUF_REG, *PST_BUF_REG;

#define ST_BUF_REG_SIZE             (sizeof(ST_BUF_REG))
#define ST_BUF_REG_LEN              (ST_BUF_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_BUF_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_BUF_UO_REG stBufUoReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaBufSs0Cmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_BUF_SS0_REG stBufSs0Reg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaBufSs1Cmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_BUF_SS1_REG stBufSs1Reg __attribute__ ((aligned (8)));
#if defined(SUPPORT_DUMP_MODULE)
  #if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaBufDumpCmd __attribute__ ((aligned (8)));
  #endif //SUPPORT_RDMA_FEATURE
    ST_BUF_DUMP_REG stBufDumpReg __attribute__ ((aligned (8)));
#endif //#if defined(SUPPORT_DUMP_MODULE)
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaBufTil_1_RdCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_BUF_TIL_1_RD_REG stBufTil_1_RdReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaBufTil_1_WrCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_BUF_TIL_1_WR_REG stBufTil_1_WrReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaBufScDumpCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_BUF_SC_DUMP_REG stBufScDumpReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaBufYHistCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_BUF_YHIST_REG stBufYHistReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_BUF_RDMA, *PST_BUF_RDMA;

#define ST_BUF_RDMA_SIZE            (sizeof(ST_BUF_RDMA))
#define ST_BUF_RDMA_LEN             (ST_BUF_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_UO_RDMA_SIZE         (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_UO_REG) + 7) / 8) * 8))
#define ST_BUF_UO_RDMA_LEN          (ST_BUF_UO_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_SS0_RDMA_SIZE        (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_SS0_REG) + 7) / 8) * 8))
#define ST_BUF_SS0_RDMA_LEN         (ST_BUF_SS0_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_SS1_RDMA_SIZE        (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_SS1_REG) + 7) / 8) * 8))
#define ST_BUF_SS1_RDMA_LEN         (ST_BUF_SS1_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_DUMP_RDMA_SIZE       (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_DUMP_REG) + 7) / 8) * 8))
#define ST_BUF_DUMP_RDMA_LEN        (ST_BUF_DUMP_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_TIL_1_RD_RDMA_SIZE   (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_TIL_1_RD_REG) + 7) / 8) * 8))
#define ST_BUF_TIL_1_RD_RDMA_LEN    (ST_BUF_TIL_1_RD_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_TIL_1_WR_RDMA_SIZE   (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_TIL_1_WR_REG) + 7) / 8) * 8))
#define ST_BUF_TIL_1_WR_RDMA_LEN    (ST_BUF_TIL_1_WR_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_SC_DUMP_RDMA_SIZE    (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_SC_DUMP_REG) + 7) / 8) * 8))
#define ST_BUF_SC_DUMP_RDMA_LEN     (ST_BUF_SC_DUMP_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_BUF_YHIST_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_BUF_YHIST_REG) + 7) / 8) * 8))
#define ST_BUF_YHIST_RDMA_LEN       (ST_BUF_YHIST_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_BUF_RDMA {
    STF_U8 u8IspIdx;
    ST_BUF_RDMA stBufRdma __attribute__ ((aligned (8)));
} ST_ISP_BUF_RDMA, *PST_ISP_BUF_RDMA;
#pragma pack(pop)


///* ISP video output, dump, tiling read/write and statistical collection buffer interface registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_BUF_SetReg(
//    STF_U8 u8IspIdx,
//    ST_ISP_BUF_RDMA *pstIspBufRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_BUF_RDMA_H__
