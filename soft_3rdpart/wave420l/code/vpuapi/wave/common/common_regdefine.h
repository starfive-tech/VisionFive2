//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#ifndef __COMMON_REGISTER_DEFINE_H__
#define __COMMON_REGISTER_DEFINE_H__

#define W4_REG_BASE                     0x00000000
#define W4_CMD_REG_BASE                 0x00000100
#define W4_CMD_REG_END                  0x00000200

/*
 * Common
 */
/* Power On Configuration
 * PO_DEBUG_MODE    [0]     1 - Power On with debug mode
 * USE_PO_CONF      [3]     1 - Use Power-On-Configuration
 */
#define W4_PO_CONF                     (W4_REG_BASE + 0x0000)
#define W4_VCPU_CUR_PC                 (W4_REG_BASE + 0x0004)
#define W4_VPU_PDBG_CTRL               (W4_REG_BASE + 0x0010)         // vCPU debugger ctrl register
#define W4_VPU_PDBG_IDX_REG            (W4_REG_BASE + 0x0014)         // vCPU debugger index register
#define W4_VPU_PDBG_WDATA_REG          (W4_REG_BASE + 0x0018)         // vCPU debugger write data register
#define W4_VPU_PDBG_RDATA_REG          (W4_REG_BASE + 0x001C)         // vCPU debugger read data register
#define W4_VPU_FIO_CTRL_ADDR           (W4_REG_BASE + 0x0020)
#define W4_VPU_FIO_DATA                (W4_REG_BASE + 0x0024)
#define W4_VPU_VINT_REASON_USR         (W4_REG_BASE + 0x0030)
#define W4_VPU_VINT_REASON_CLR         (W4_REG_BASE + 0x0034)
#define W4_VPU_HOST_INT_REQ            (W4_REG_BASE + 0x0038)
#define W4_VPU_VINT_CLEAR              (W4_REG_BASE + 0x003C)
#define W4_VPU_HINT_CLEAR              (W4_REG_BASE + 0x0040)
#define W4_VPU_VPU_INT_STS             (W4_REG_BASE + 0x0044)
#define W4_VPU_VINT_ENABLE             (W4_REG_BASE + 0x0048)
/*  INTERRUPT
 */
enum {
    W4_INT_INIT_VPU          = 0,
    W4_INT_DEC_PIC_HDR       = 1,
    W4_INT_SET_PARAM         = 1,
    W4_INT_ENC_INIT_SEQ      = 1,
    W4_INT_FINI_SEQ          = 2,
    W4_INT_DEC_PIC           = 3,
    W4_INT_ENC_PIC           = 3,
    W4_INT_SET_FRAMEBUF      = 4,
    W4_INT_FLUSH_DEC         = 5,
    W4_INT_ENC_SLICE_INT     = 7,
    W4_INT_GET_FW_VERSION    = 8,
    W4_INT_QUERY_DEC         = 9,
    W4_INT_SLEEP_VPU         = 10,
    W4_INT_WAKEUP_VPU        = 11,
    W4_INT_CHANGE_INT        = 12,       
    W4_INT_CREATE_INSTANCE   = 14, 
    W4_INT_BSBUF_EMPTY       = 15,   /*!<< Bitstream buffer empty[dec]/full[enc] */
};

typedef enum {
    INIT_VPU        = 0x0001,
    DEC_PIC_HDR     = 0x0002,               /* INIT_SEQ or SLICE HEADER */
    SET_PARAM       = 0x0002,
    FINI_SEQ        = 0x0004,
    DEC_PIC         = 0x0008,
    ENC_PIC         = 0x0008,
    SET_FRAMEBUF    = 0x0010,
    FLUSH_DECODER   = 0x0020,
    GET_FW_VERSION  = 0x0100,
    QUERY_DECODER   = 0x0200,
    SLEEP_VPU       = 0x0400,
    WAKEUP_VPU      = 0x0800,
    CREATE_INSTANCE = 0x4000,
    RESET_VPU		= 0x10000,
    MAX_VPU_COMD	= 0x10000,
} VPU_COMMAND;

#define W4_VPU_VINT_REASON                   (W4_REG_BASE + 0x004C)
/*  RESET_REQ
 *  CCLK    [7:0]           - for each vCORE
 *  BCLK    [15:8]          - for each vCORE
 *  ACLK    [23:16]         - for each vCORE
 *  ACLK    [24]            - for vCPU
 *  VCLK    [25]            - 
 *  MCLK    [26]            - Multi-media AXI
 */
#define W4_VPU_RESET_REQ                (W4_REG_BASE + 0x0050)
#define W4_RST_BLOCK_CCLK(_core)        (1<<_core)
#define W4_RST_BLOCK_CCLK_ALL           (0xff)
#define W4_RST_BLOCK_BCLK(_core)        (0x100<<_core)
#define W4_RST_BLOCK_BCLK_ALL           (0xff00)
#define W4_RST_BLOCK_ACLK(_core)        (0x10000<<_core)
#define W4_RST_BLOCK_ACLK_ALL           (0xff0000)
#define W4_RST_BLOCK_VCPU               (0x1000000)
#define W4_RST_BLOCK_VCLK               (0x2000000)
#define W4_RST_BLOCK_MCLK               (0x4000000)
#define W4_RST_BLOCK_ALL                (0xfffffff)
#define W4_VPU_RESET_STATUS             (W4_REG_BASE + 0x0054)
enum {
    RESET_VCE     = 0x01,
    RESET_BPU     = 0x02,
    RESET_MAXI    = 0x04,
    RESET_VCPU    = 0x08,
    RESET_PAXI    = 0x10,
    RESET_ALL     = 0x1f,
};

#define W4_VCPU_RESTART                      (W4_REG_BASE + 0x0058)
#define W4_VPU_CLK_MASK                     (W4_REG_BASE + 0x005C)
/* REMAP_CTRL
 * PAGE SIZE:   [8:0]   0x001 - 4K          
 *                      0x002 - 8K
 *                      0x004 - 16K
 *                      ...
 *                      0x100 - 1M
 * REGION ATTR1 [10]    0     - Normal
 *                      1     - Make Bus error for the region
 * REGION ATTR2 [11]    0     - Normal
 *                      1     - Bypass region
 * REMAP INDEX  [15:12]       - 0 ~ 3 , 0 - Code, 1-Stack
 * ENDIAN       [19:16]       - See EndianMode in vdi.h
 * AXI-ID       [23:20]       - Upper AXI-ID
 * BUS_ERROR    [29]    0     - bypass
 *                      1     - Make BUS_ERROR for unmapped region
 * BYPASS_ALL   [30]    1     - Bypass all
 * ENABLE       [31]    1     - Update control register[30:16]
 */
enum {
    W4_REMAP_CODE_INDEX=0,
};
#define W4_VPU_REMAP_CTRL               (W4_REG_BASE + 0x0060)
#define W4_VPU_REMAP_VADDR              (W4_REG_BASE + 0x0064)
#define W4_VPU_REMAP_PADDR              (W4_REG_BASE + 0x0068)
#define W4_VPU_REMAP_CORE_START         (W4_REG_BASE + 0x006C)
#define W4_VPU_BUSY_STATUS              (W4_REG_BASE + 0x0070)
#define W4_VPU_HALT_STATUS              (W4_REG_BASE + 0x0074)
#define W4_CMD_VCPU_DDR_CH_SELECT       (W4_REG_BASE + 0x00F8)

/************************************************************************/
/* Decoder Common                                                       */
/************************************************************************/
#define W4_COMMAND                      (W4_REG_BASE + 0x0100)
#define W4_CORE_INDEX                   (W4_REG_BASE + 0x0104)
/* INST_INDEX   [15:0]  : 0 - HEVC
 *              [31:16] : Instance index
 */
#define W4_INST_INDEX                   (W4_REG_BASE + 0x0108)
/*   COMMAND                    Description
 * ------------------------------------------------------------------
 * DEC_PIC          0x100       Reset
 */
#define DEC_PIC_SEQ_INIT                (0x1)
#define DEC_PIC_OPT_DISCARD_PICTURE     (0x100)
#define DEC_PIC_OPT_EMPTY_INTR_INTERVAL (0x10000)
#define W4_COMMAND_OPTION               (W4_REG_BASE + 0x010C)
#define W4_RET_SUCCESS                  (W4_REG_BASE + 0x0110)
/*
 * See WAVE4_ERRORS
 */
#define W4_RET_FAIL_REASON              (W4_REG_BASE + 0x0114)
#define W4_FRAME_CYCLE                  (W4_REG_BASE + 0x01d4)

/************************************************************************/
/* DECODER - INIT_VPU                                                   */
/************************************************************************/
/* Note: W4_INIT_CODE_BASE_ADDR should be aligned to 4KB */
#define W4_ADDR_CODE_BASE               (W4_REG_BASE + 0x0118)
#define W4_CODE_SIZE                    (W4_REG_BASE + 0x011C)
#define W4_CODE_PARAM                   (W4_REG_BASE + 0x0120)
#define W4_HW_OPTION                    (W4_REG_BASE + 0x0124)
/* Note: W4_INIT_STACK_BASE_ADDR should be aligned to 4KB */
#define W4_ADDR_STACK_BASE              (W4_REG_BASE + 0x012C)
#define W4_STACK_SIZE                   (W4_REG_BASE + 0x0130)
#define W4_TIMEOUT_CNT                  (W4_REG_BASE + 0x0134)

/************************************************************************/
/* DECODER - SET_FRAMEBUF                                               */
/************************************************************************/
/* SFB_OPTION       [25:20]     fbc mode
 *                  [19:16]     endian
 *                  [4]         Done
 *                  [3:0]       0: SET_FRAMEBUFFER(upto 8 framebuffers at once)
 */
#define W4_SFB_OPTION                   (W4_REG_BASE + 0x010C)
/* COMMON_PIC_INFO  [29]        NV21_ENABLE for linear framebuffer
 *                  [28]        BWB_ENABLE_FLAG
 *                  [27:24]     AXI_ID
 *                  [23]        Reserved
 *                  [22]        0: Right justified, 1: Left justified
 *                  [21:20]     MapType 0: 8bit, 1: 16bit, 2: 3 pixels in 32bit 
 *                  [19:17]     Color format
 *                  [16]        Cb/Cr interleaved
 *                  [15:0]      stride
 */

#define W4_COMMON_PIC_INFO               (W4_REG_BASE + 0x0120)
/* PIC_SIZE         [31:16]     picture width
 *                  [15:0]      picture height
 */
#define W4_PIC_SIZE                     (W4_REG_BASE + 0x0124)
/* SET_FB_NUM       [12:8]      FB_NUM_START
 *                  [4:0]       FB_NUM_END
 *                  [31:0]      TARGET_FB_FLAG
 */
#define W4_SET_FB_NUM                   (W4_REG_BASE + 0x0128)     // ADD/REMOVE NUM

#define C7_SET_FRAME_SLICE_BB_START     (W4_REG_BASE + 0x0150)
#define C7_SET_FRAME_SLICE_BB_SIZE      (W4_REG_BASE + 0x0154)

#define W4_ADDR_LUMA_BASE0              (W4_REG_BASE + 0x0160)
#define W4_ADDR_CB_BASE0                (W4_REG_BASE + 0x0164)
#define W4_ADDR_CR_BASE0                (W4_REG_BASE + 0x0168)
#define W4_ADDR_FBC_Y_OFFSET0           (W4_REG_BASE + 0x0168)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET0           (W4_REG_BASE + 0x016C)       // Compression offset table for Chroma
#define W4_ADDR_LUMA_BASE1              (W4_REG_BASE + 0x0170)
#define W4_ADDR_CB_ADDR1                (W4_REG_BASE + 0x0174)
#define W4_ADDR_CR_ADDR1                (W4_REG_BASE + 0x0178)
#define W4_ADDR_FBC_Y_OFFSET1           (W4_REG_BASE + 0x0178)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET1           (W4_REG_BASE + 0x017C)       // Compression offset table for Chroma
#define W4_ADDR_LUMA_BASE2              (W4_REG_BASE + 0x0180)
#define W4_ADDR_CB_ADDR2                (W4_REG_BASE + 0x0184)
#define W4_ADDR_CR_ADDR2                (W4_REG_BASE + 0x0188)
#define W4_ADDR_FBC_Y_OFFSET2           (W4_REG_BASE + 0x0188)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET2           (W4_REG_BASE + 0x018C)       // Compression offset table for Chroma
#define W4_ADDR_LUMA_BASE3              (W4_REG_BASE + 0x0190)
#define W4_ADDR_CB_ADDR3                (W4_REG_BASE + 0x0194)
#define W4_ADDR_CR_ADDR3                (W4_REG_BASE + 0x0198)
#define W4_ADDR_FBC_Y_OFFSET3           (W4_REG_BASE + 0x0198)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET3           (W4_REG_BASE + 0x019C)       // Compression offset table for Chroma
#define W4_ADDR_LUMA_BASE4              (W4_REG_BASE + 0x01A0)
#define W4_ADDR_CB_ADDR4                (W4_REG_BASE + 0x01A4)
#define W4_ADDR_CR_ADDR4                (W4_REG_BASE + 0x01A8)
#define W4_ADDR_FBC_Y_OFFSET4           (W4_REG_BASE + 0x01A8)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET4           (W4_REG_BASE + 0x01AC)       // Compression offset table for Chroma
#define W4_ADDR_LUMA_BASE5              (W4_REG_BASE + 0x01B0)
#define W4_ADDR_CB_ADDR5                (W4_REG_BASE + 0x01B4)
#define W4_ADDR_CR_ADDR5                (W4_REG_BASE + 0x01B8)
#define W4_ADDR_FBC_Y_OFFSET5           (W4_REG_BASE + 0x01B8)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET5           (W4_REG_BASE + 0x01BC)       // Compression offset table for Chroma
#define W4_ADDR_LUMA_BASE6              (W4_REG_BASE + 0x01C0)
#define W4_ADDR_CB_ADDR6                (W4_REG_BASE + 0x01C4)
#define W4_ADDR_CR_ADDR6                (W4_REG_BASE + 0x01C8)
#define W4_ADDR_FBC_Y_OFFSET6           (W4_REG_BASE + 0x01C8)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET6           (W4_REG_BASE + 0x01CC)       // Compression offset table for Chroma
#define W4_ADDR_LUMA_BASE7              (W4_REG_BASE + 0x01D0)
#define W4_ADDR_CB_ADDR7                (W4_REG_BASE + 0x01D4)
#define W4_ADDR_CR_ADDR7                (W4_REG_BASE + 0x01D8)
#define W4_ADDR_FBC_Y_OFFSET7           (W4_REG_BASE + 0x01D8)       // Compression offset table for Luma
#define W4_ADDR_FBC_C_OFFSET7           (W4_REG_BASE + 0x01DC)       // Compression offset table for Chroma
#define W4_ADDR_MV_COL0                 (W4_REG_BASE + 0x01E0)
#define W4_ADDR_MV_COL1                 (W4_REG_BASE + 0x01E4)
#define W4_ADDR_MV_COL2                 (W4_REG_BASE + 0x01E8)
#define W4_ADDR_MV_COL3                 (W4_REG_BASE + 0x01EC)
#define W4_ADDR_MV_COL4                 (W4_REG_BASE + 0x01F0)
#define W4_ADDR_MV_COL5                 (W4_REG_BASE + 0x01F4)
#define W4_ADDR_MV_COL6                 (W4_REG_BASE + 0x01F8)
#define W4_ADDR_MV_COL7                 (W4_REG_BASE + 0x01FC)

/************************************************************************/
/* DECODER - FLUSH_DECODER                                              */
/************************************************************************/
/* W4_FLUSH_DECODER_OPTION[0] 0 - flush DPB
 */
#define FLUSH_DPB                       0
#define FLUSH_CPB                       1
#define W4_FLUSH_DECODER_OPTION        (W4_REG_BASE + 0x010C)
#define W4_RET_DEC_DISPLAY_INDEX_0     (W4_REG_BASE + 0x0198)
#define W4_RET_DEC_DISPLAY_INDEX_1     (W4_REG_BASE + 0x019C)
#define W4_RET_DEC_DISPLAY_INDEX_2     (W4_REG_BASE + 0x01A0)
#define W4_RET_DEC_DISPLAY_INDEX_3     (W4_REG_BASE + 0x01A4)
#define W4_RET_DEC_DISPLAY_INDEX_4     (W4_REG_BASE + 0x01A8)
#define W4_RET_DEC_DISPLAY_INDEX_5     (W4_REG_BASE + 0x01AC)
#define W4_RET_DEC_DISPLAY_INDEX_6     (W4_REG_BASE + 0x01B0)
#define W4_RET_DEC_DISPLAY_INDEX_7     (W4_REG_BASE + 0x01B4)
#define W4_RET_DEC_DISPLAY_INDEX_8     (W4_REG_BASE + 0x01B8)
#define W4_RET_DEC_DISPLAY_INDEX_9     (W4_REG_BASE + 0x01BC)
#define W4_RET_DEC_DISPLAY_INDEX_10    (W4_REG_BASE + 0x01C0)
#define W4_RET_DEC_DISPLAY_INDEX_11    (W4_REG_BASE + 0x01C4)
#define W4_RET_DEC_DISPLAY_INDEX_12    (W4_REG_BASE + 0x01C8)
#define W4_RET_DEC_DISPLAY_INDEX_13    (W4_REG_BASE + 0x01CC)
#define W4_RET_DEC_DISPLAY_INDEX_14    (W4_REG_BASE + 0x01D0)
#define W4_RET_DEC_DISPLAY_INDEX_15    (W4_REG_BASE + 0x01D4)
#define W4_RET_DEC_DISPLAY_SIZE        (W4_REG_BASE + 0x01D8)

/************************************************************************/
/* DECODER - DEC_PIC_HDR/DEC_PIC                                        */
/************************************************************************/
#define W4_BS_START_ADDR                    (W4_REG_BASE + 0x0120)
#define W4_BS_SIZE                          (W4_REG_BASE + 0x0124)
/* BS_PARAM             [9]         Ring-buffer enable
 *                      [3:0]       endianess
 */
#define BSPARAM_ENABLE_RINGBUFFER           (1<<9)
#define BSPARAM_ENABLE_NALPUMPING           (1<<8)
#define BSPARAM_EMODE_SEND_IMM              (0<<4)
#define BSPARAM_EMODE_CONCEAL               (2<<4)
#define W4_BS_PARAM                         (W4_REG_BASE + 0x0128)
/* BS_OPTION            [0] :       1 - explict_end      
 *                      [1] :       1 - stream end flag
 */
#define W4_BS_OPTION                        (W4_REG_BASE + 0x012C)
/* Bit Index */

#define W4_BS_RD_PTR                        (W4_REG_BASE + 0x0130)
#define W4_BS_WR_PTR                        (W4_REG_BASE + 0x0134)
#define W4_ADDR_WORK_BASE                   (W4_REG_BASE + 0x0138)
#define W4_WORK_SIZE                        (W4_REG_BASE + 0x013C)
/* WORK_PARAM           [3:0]       Endianess
 */
#define W4_WORK_PARAM                       (W4_REG_BASE + 0x0140)
#define W4_ADDR_TEMP_BASE                   (W4_REG_BASE + 0x0144)
#define W4_TEMP_SIZE                        (W4_REG_BASE + 0x0148)
/* TEMP_PARAM           [3:0]       Endianess
 */
#define W4_TEMP_PARAM                       (W4_REG_BASE + 0x014C)

#define W4_ADDR_SEC_AXI                     (W4_REG_BASE + 0x0150)
#define W4_SEC_AXI_SIZE                     (W4_REG_BASE + 0x0154)
#define W4_USE_SEC_AXI                      (W4_REG_BASE + 0x0158)
/************************************************************************/
/* DECODER - DEC_PIC_HDR/DEC_PIC                                        */
/************************************************************************/
#define W4_CMD_DEC_ADDR_REPORT_BASE         (W4_REG_BASE + 0x015C)
#define W4_CMD_DEC_REPORT_SIZE              (W4_REG_BASE + 0x0160)
/* USER_PARAM           [3:0]       Endianess
 */
#define W4_CMD_DEC_REPORT_PARAM             (W4_REG_BASE + 0x0164)
#define W4_CMD_DEC_ADDR_USER_BASE           (W4_REG_BASE + 0x0168)
#define W4_CMD_DEC_USER_SIZE                (W4_REG_BASE + 0x016C)
#define W4_CMD_DEC_USER_PARAM               (W4_REG_BASE + 0x0170)

#define W4_RET_DEC_DISP_FLAG                (W4_REG_BASE + 0x0190)
#define W4_CMD_DEC_DISP_FLAG                (W4_REG_BASE + 0x0190)
/************************************************************************/
/* DECODER - GET_FW_VERSION                                             */
/************************************************************************/
#define W4_RET_FW_VERSION                   (W4_REG_BASE + 0x0118)
#define W4_RET_PRODUCT_NAME                 (W4_REG_BASE + 0x011C)
#define W4_RET_PRODUCT_VERSION              (W4_REG_BASE + 0x0120)
#define W4_RET_STD_DEF0                     (W4_REG_BASE + 0x0124)
#define W4_RET_STD_DEF1                     (W4_REG_BASE + 0x0128)
#define W4_RET_CONF_FEATURE                 (W4_REG_BASE + 0x012C)
#define W4_RET_CONFIG_DATE                  (W4_REG_BASE + 0x0130)
#define W4_RET_CONFIG_REVISION              (W4_REG_BASE + 0x0134)
#define W4_RET_CONFIG_TYPE                  (W4_REG_BASE + 0x0138)

#define W4_RET_CONF_VCORE0                  (W4_REG_BASE + 0x0150)

/************************************************************************/
/* SLEEP_VPU                                                            */
/************************************************************************/
#define W4_RET_CUR_SP                       (W4_REG_BASE + 0x0124)

/************************************************************************/
/* WAKEUP_VPU                                                           */
/************************************************************************/
#define W4_SET_CUR_SP                       (W4_REG_BASE + 0x0120)


/************************************************************************/
/* PRODUCT INFORMATION                                                  */
/************************************************************************/
#define W4_PRODUCT_NAME                     (W4_REG_BASE + 0x1040)
#define W4_PRODUCT_NUMBER                   (W4_REG_BASE + 0x1044)

/************************************************************************/
/* GDI register for Debugging                                           */
/************************************************************************/
#define W4_GDI_BASE                         0x8800
#define W4_GDI_CORE1_BASE                   0x9800

#define W4_GDI_PRI_RD_PRIO_L                (W4_GDI_BASE + 0x000)
#define W4_GDI_PRI_RD_PRIO_H                (W4_GDI_BASE + 0x004)
#define W4_GDI_PRI_WR_PRIO_L                (W4_GDI_BASE + 0x008)
#define W4_GDI_PRI_WR_PRIO_H                (W4_GDI_BASE + 0x00c)
#define W4_GDI_PRI_RD_LOCK_CNT              (W4_GDI_BASE + 0x010)
#define W4_GDI_PRI_WR_LOCK_CNT              (W4_GDI_BASE + 0x014)
#define W4_GDI_SEC_RD_PRIO_L                (W4_GDI_BASE + 0x018)
#define W4_GDI_SEC_RD_PRIO_H                (W4_GDI_BASE + 0x01c)
#define W4_GDI_SEC_WR_PRIO_L                (W4_GDI_BASE + 0x020)
#define W4_GDI_SEC_WR_PRIO_H                (W4_GDI_BASE + 0x024)
#define W4_GDI_SEC_RD_LOCK_CNT              (W4_GDI_BASE + 0x028)
#define W4_GDI_SEC_WR_LOCK_CNT              (W4_GDI_BASE + 0x02c)
#define W4_GDI_SEC_CLIENT_EN                (W4_GDI_BASE + 0x030)
#define W4_GDI_CONTROL                      (W4_GDI_BASE + 0x034)
#define W4_GDI_PIC_INIT_HOST                (W4_GDI_BASE + 0x038)
#define W4_GDI_HW_VERINFO                   (W4_GDI_BASE + 0x050)
#define W4_GDI_PINFO_REQ                    (W4_GDI_BASE + 0x060)
#define W4_GDI_PINFO_ACK                    (W4_GDI_BASE + 0x064)
#define W4_GDI_PINFO_ADDR                   (W4_GDI_BASE + 0x068)
#define W4_GDI_PINFO_DATA                   (W4_GDI_BASE + 0x06c)
#define W4_GDI_BWB_ENABLE                   (W4_GDI_BASE + 0x070)
#define W4_GDI_BWB_SIZE                     (W4_GDI_BASE + 0x074)
#define W4_GDI_BWB_STD_STRUCT               (W4_GDI_BASE + 0x078)
#define W4_GDI_BWB_STATUS                   (W4_GDI_BASE + 0x07c)
#define W4_GDI_STATUS                       (W4_GDI_BASE + 0x080)
#define W4_GDI_DEBUG_0                      (W4_GDI_BASE + 0x084)
#define W4_GDI_DEBUG_1                      (W4_GDI_BASE + 0x088)
#define W4_GDI_DEBUG_2                      (W4_GDI_BASE + 0x08c)
#define W4_GDI_DEBUG_3                      (W4_GDI_BASE + 0x090)
#define W4_GDI_DEBUG_PROBE_ADDR             (W4_GDI_BASE + 0x094)
#define W4_GDI_DEBUG_PROBE_DATA             (W4_GDI_BASE + 0x098)

// write protect
#define W4_GDI_WPROT_ERR_CLR                (W4_GDI_BASE + 0x0A0)
#define W4_GDI_WPROT_ERR_RSN                (W4_GDI_BASE + 0x0A4)
#define W4_GDI_WPROT_ERR_ADR                (W4_GDI_BASE + 0x0A8)
#define W4_GDI_WPROT_RGN_EN                 (W4_GDI_BASE + 0x0AC)
#define W4_GDI_WPROT_RGN0_STA               (W4_GDI_BASE + 0x0B0)
#define W4_GDI_WPROT_RGN0_END               (W4_GDI_BASE + 0x0B4)
#define W4_GDI_WPROT_RGN1_STA               (W4_GDI_BASE + 0x0B8)
#define W4_GDI_WPROT_RGN1_END               (W4_GDI_BASE + 0x0BC)
#define W4_GDI_WPROT_RGN2_STA               (W4_GDI_BASE + 0x0C0)
#define W4_GDI_WPROT_RGN2_END               (W4_GDI_BASE + 0x0C4)
#define W4_GDI_WPROT_RGN3_STA               (W4_GDI_BASE + 0x0C8)
#define W4_GDI_WPROT_RGN3_END               (W4_GDI_BASE + 0x0CC) 
#define W4_GDI_WPROT_RGN4_STA               (W4_GDI_BASE + 0x0D0)
#define W4_GDI_WPROT_RGN4_END               (W4_GDI_BASE + 0x0D4)
#define W4_GDI_WPROT_RGN5_STA               (W4_GDI_BASE + 0x0D8)
#define W4_GDI_WPROT_RGN5_END               (W4_GDI_BASE + 0x0DC)
#define W4_GDI_VCORE0_BUS_CTRL              (W4_GDI_BASE + 0x0F0)
#define W4_GDI_VCORE0_BUS_STATUS            (W4_GDI_BASE + 0x0F4)
#define W4_GDI_VCORE1_BUS_CTRL              (W4_GDI_CORE1_BASE + 0x0F0)
#define W4_GDI_VCORE1_BUS_STATUS            (W4_GDI_CORE1_BASE + 0x0F4)

#define W4_RET_GDI_WPROT_ERR_RSN            (W4_REG_BASE + 0x0118)
#define W4_RET_GDI_WPROT_ERR_ADR            (W4_REG_BASE + 0x011C)
#define W4_RET_GDI_SIZE_ERR_FLAG            (W4_REG_BASE + 0x0120)

#define W4_GDI_SIZE_ERR_FLAG                (W4_GDI_BASE + 0x0e0)
#define W4_GDI_ADR_RQ_SIZE_ERR_PRI0         (W4_GDI_BASE + 0x100)
#define W4_GDI_ADR_RQ_SIZE_ERR_PRI1         (W4_GDI_BASE + 0x104)
#define W4_GDI_ADR_RQ_SIZE_ERR_PRI2         (W4_GDI_BASE + 0x108)
#define W4_GDI_ADR_WQ_SIZE_ERR_PRI0         (W4_GDI_BASE + 0x10c)
#define W4_GDI_ADR_WQ_SIZE_ERR_PRI1         (W4_GDI_BASE + 0x110)
#define W4_GDI_ADR_WQ_SIZE_ERR_PRI2         (W4_GDI_BASE + 0x114)

#define W4_GDI_ADR_RQ_SIZE_ERR_SEC0         (W4_GDI_BASE + 0x118)
#define W4_GDI_ADR_RQ_SIZE_ERR_SEC1         (W4_GDI_BASE + 0x11c)
#define W4_GDI_ADR_RQ_SIZE_ERR_SEC2         (W4_GDI_BASE + 0x120)
#define W4_GDI_ADR_WQ_SIZE_ERR_SEC0         (W4_GDI_BASE + 0x124)
#define W4_GDI_ADR_WQ_SIZE_ERR_SEC1         (W4_GDI_BASE + 0x128)
#define W4_GDI_ADR_WQ_SIZE_ERR_SEC2         (W4_GDI_BASE + 0x12c)

#define W4_GDI_ADR_RQ_SIZE_ERR_PRI0_2D      (W4_GDI_BASE + 0x130)
#define W4_GDI_ADR_RQ_SIZE_ERR_PRI1_2D      (W4_GDI_BASE + 0x134)
#define W4_GDI_ADR_RQ_SIZE_ERR_PRI2_2D      (W4_GDI_BASE + 0x138)
#define W4_GDI_ADR_WQ_SIZE_ERR_PRI0_2D      (W4_GDI_BASE + 0x13c)
#define W4_GDI_ADR_WQ_SIZE_ERR_PRI1_2D      (W4_GDI_BASE + 0x140)
#define W4_GDI_ADR_WQ_SIZE_ERR_PRI2_2D      (W4_GDI_BASE + 0x144)

#define W4_GDI_INFO_CONTROL                 (W4_GDI_BASE + 0x400)
#define W4_GDI_INFO_PIC_SIZE                (W4_GDI_BASE + 0x404)
// W4_GDI 2.0 register
#define W4_GDI_INFO_BASE_Y_TOP              (W4_GDI_BASE + 0x408)
#define W4_GDI_INFO_BASE_CB_TOP             (W4_GDI_BASE + 0x40C)
#define W4_GDI_INFO_BASE_CR_TOP             (W4_GDI_BASE + 0x410)
#define W4_GDI_INFO_BASE_Y_BOT              (W4_GDI_BASE + 0x414)
#define W4_GDI_INFO_BASE_CB_BOT             (W4_GDI_BASE + 0x418)
#define W4_GDI_INFO_BASE_CR_BOT             (W4_GDI_BASE + 0x41C)
#define W4_GDI_XY2AXI_LUM_BIT00             (W4_GDI_BASE + 0x800)
#define W4_GDI_XY2AXI_LUM_BIT1F             (W4_GDI_BASE + 0x87C)
#define W4_GDI_XY2AXI_CHR_BIT00             (W4_GDI_BASE + 0x880)
#define W4_GDI_XY2AXI_CHR_BIT1F             (W4_GDI_BASE + 0x8FC)
#define W4_GDI_XY2AXI_CONFIG                (W4_GDI_BASE + 0x900)

//W4_GDI 1.0 register
#define W4_GDI_INFO_BASE_Y                  (W4_GDI_BASE + 0x408)
#define W4_GDI_INFO_BASE_CB                 (W4_GDI_BASE + 0x40C)
#define W4_GDI_INFO_BASE_CR                 (W4_GDI_BASE + 0x410)

#define W4_GDI_XY2_CAS_0                    (W4_GDI_BASE + 0x800)
#define W4_GDI_XY2_CAS_F                    (W4_GDI_BASE + 0x83C)

#define W4_GDI_XY2_BA_0                     (W4_GDI_BASE + 0x840)
#define W4_GDI_XY2_BA_1                     (W4_GDI_BASE + 0x844)
#define W4_GDI_XY2_BA_2                     (W4_GDI_BASE + 0x848)
#define W4_GDI_XY2_BA_3                     (W4_GDI_BASE + 0x84C)

#define W4_GDI_XY2_RAS_0                    (W4_GDI_BASE + 0x850)
#define W4_GDI_XY2_RAS_F                    (W4_GDI_BASE + 0x88C)

#define W4_GDI_XY2_RBC_CONFIG               (W4_GDI_BASE + 0x890)
#define W4_GDI_RBC2_AXI_0                   (W4_GDI_BASE + 0x8A0)
#define W4_GDI_RBC2_AXI_1F                  (W4_GDI_BASE + 0x91C)
#define W4_GDI_TILEDBUF_BASE                (W4_GDI_BASE + 0x920)

#define W4_FBC_STRIDE                       (W4_REG_BASE + 0x0154)

#endif /* __COMMON_REGISTER_DEFINE_H__ */

