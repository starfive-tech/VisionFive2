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

#ifndef __WAVE510_REGISTER_DEFINE_H__
#define __WAVE510_REGISTER_DEFINE_H__

enum {
    W5_INT_INIT_VPU          = 0,
    W5_INT_WAKEUP_VPU        = 1,
    W5_INT_SLEEP_VPU         = 2,
    W5_INT_CREATE_INSTANCE   = 3, 
    W5_INT_FLUSH_INSTANCE    = 4,
    W5_INT_DESTORY_INSTANCE  = 5,
    W5_INT_INIT_SEQ          = 6,
    W5_INT_SET_FRAMEBUF      = 7,
    W5_INT_DEC_PIC           = 8,
    W5_INT_ENC_PIC           = 8,
    W5_INT_ENC_SET_PARAM     = 9,
    W5_INT_DEC_QUERY         = 14,
    W5_INT_BSBUF_EMPTY       = 15,
};

typedef enum {
    W5_INIT_VPU        = 0x0001,
    W5_WAKEUP_VPU      = 0x0002,
    W5_SLEEP_VPU       = 0x0004,
    W5_CREATE_INSTANCE = 0x0008,            /* queuing command */
    W5_FLUSH_INSTANCE  = 0x0010,
    W5_DESTROY_INSTANCE= 0x0020,            /* queuing command */
    W5_INIT_SEQ        = 0x0040,            /* queuing command */
    W5_SET_FB          = 0x0080,
    W5_DEC_PIC         = 0x0100,            /* queuing command */
    W5_ENC_PIC         = 0x0100,            /* queuing command */
    W5_ENC_SET_PARAM   = 0x0200,            /* queuing command */
    W5_QUERY           = 0x4000,
    W5_UPDATE_BS       = 0x8000,
    W5_RESET_VPU	   = 0x10000,
    W5_MAX_VPU_COMD	= 0x10000,
} W5_VPU_COMMAND;

typedef enum {
    GET_VPU_INFO        = 0,
    SET_WRITE_PROT      = 1,
    GET_RESULT          = 2,
    UPDATE_DISP_FLAG    = 3,
    GET_BW_REPORT       = 4
} QUERY_OPT;

#define W5_REG_BASE                     0x00000000
#define W5_CMD_REG_BASE                 0x00000100
#define W5_CMD_REG_END                  0x00000200

#define WAVE5_AXI_ID                    0x0
/*
 * Common
 */
/* Power On Configuration
 * PO_DEBUG_MODE    [0]     1 - Power On with debug mode
 * USE_PO_CONF      [3]     1 - Use Power-On-Configuration
 */
#define W5_PO_CONF                     (W5_REG_BASE + 0x0000)
#define W5_VCPU_CUR_PC                 (W5_REG_BASE + 0x0004)
#define W5_VPU_PDBG_CTRL               (W5_REG_BASE + 0x0010)         // vCPU debugger ctrl register
#define W5_VPU_PDBG_IDX_REG            (W5_REG_BASE + 0x0014)         // vCPU debugger index register
#define W5_VPU_PDBG_WDATA_REG          (W5_REG_BASE + 0x0018)         // vCPU debugger write data register
#define W5_VPU_PDBG_RDATA_REG          (W5_REG_BASE + 0x001C)         // vCPU debugger read data register
#define W5_VPU_FIO_CTRL_ADDR           (W5_REG_BASE + 0x0020)
#define W5_VPU_FIO_DATA                (W5_REG_BASE + 0x0024)
#define W5_VPU_VINT_REASON_USR         (W5_REG_BASE + 0x0030)
#define W5_VPU_VINT_REASON_CLR         (W5_REG_BASE + 0x0034)
#define W5_VPU_HOST_INT_REQ            (W5_REG_BASE + 0x0038)
#define W5_VPU_VINT_CLEAR              (W5_REG_BASE + 0x003C)
#define W5_VPU_HINT_CLEAR              (W5_REG_BASE + 0x0040)
#define W5_VPU_VPU_INT_STS             (W5_REG_BASE + 0x0044)
#define W5_VPU_VINT_ENABLE             (W5_REG_BASE + 0x0048)
#define W5_VPU_VINT_REASON             (W5_REG_BASE + 0x004C)
#define W5_VPU_RESET_REQ               (W5_REG_BASE + 0x0050)
#define W5_RST_BLOCK_CCLK(_core)       (1<<_core)
#define W5_RST_BLOCK_CCLK_ALL          (0xff)
#define W5_RST_BLOCK_BCLK(_core)       (0x100<<_core)
#define W5_RST_BLOCK_BCLK_ALL          (0xff00)
#define W5_RST_BLOCK_ACLK(_core)       (0x10000<<_core)
#define W5_RST_BLOCK_ACLK_ALL          (0xff0000)
#define W5_RST_BLOCK_VCPU              (0x1000000)
#define W5_RST_BLOCK_VCLK              (0x2000000)
#define W5_RST_BLOCK_MCLK              (0x4000000)
#define W5_RST_BLOCK_ALL               (0xfffffff)
#define W5_VPU_RESET_STATUS            (W5_REG_BASE + 0x0054)

#define W5_VCPU_RESTART                 (W5_REG_BASE + 0x0058)
#define W5_VPU_CLK_MASK                 (W5_REG_BASE + 0x005C)
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
    W5_REMAP_CODE_INDEX=0,
};
#define W5_VPU_REMAP_CTRL                       (W5_REG_BASE + 0x0060)
#define W5_VPU_REMAP_VADDR                      (W5_REG_BASE + 0x0064)
#define W5_VPU_REMAP_PADDR                      (W5_REG_BASE + 0x0068)
#define W5_VPU_REMAP_CORE_START                 (W5_REG_BASE + 0x006C)
#define W5_VPU_BUSY_STATUS                      (W5_REG_BASE + 0x0070)
#define W5_VPU_HALT_STATUS                      (W5_REG_BASE + 0x0074)


/************************************************************************/
/* DECODER/ENCODER COMMON                                               */
/************************************************************************/
#define W5_COMMAND                              (W5_REG_BASE + 0x0100)
#define W5_COMMAND_OPTION                       (W5_REG_BASE + 0x0104)
#define W5_QUERY_OPTION                         (W5_REG_BASE + 0x0104)
#define W5_RET_SUCCESS                          (W5_REG_BASE + 0x0108)
#define W5_RET_FAIL_REASON                      (W5_REG_BASE + 0x010C)
#define W5_CMD_INSTANCE_INFO                    (W5_REG_BASE + 0x0110)

#define W5_RET_QUEUE_STATUS                     (W5_REG_BASE + 0x01E0)
#define W5_RET_BS_EMPTY_INST                    (W5_REG_BASE + 0x01E4)
#define W5_RET_QUEUE_CMD_DONE_INST              (W5_REG_BASE + 0x01E8)
#define W5_RET_STAGE0_INSTANCE_INFO             (W5_REG_BASE + 0x01EC)
#define W5_RET_STAGE1_INSTANCE_INFO             (W5_REG_BASE + 0x01F0)
#define W5_RET_STAGE2_INSTANCE_INFO             (W5_REG_BASE + 0x01F4)

#define W5_RET_DONE_INSTANCE_INFO               (W5_REG_BASE + 0x01FC)

#define W5_BS_RD_PTR                            (W5_REG_BASE + 0x0118)
#define W5_BS_WR_PTR                            (W5_REG_BASE + 0x011C)
#define W5_BS_OPTION                            (W5_REG_BASE + 0x0120)

/************************************************************************/
/* DECODER - INIT_VPU                                                   */
/************************************************************************/
/* Note: W5_ADDR_CODE_BASE should be aligned to 4KB */
#define W5_ADDR_CODE_BASE                       (W5_REG_BASE + 0x0110)
#define W5_CODE_SIZE                            (W5_REG_BASE + 0x0114)
#define W5_CODE_PARAM                           (W5_REG_BASE + 0x0118)
#define W5_ADDR_TEMP_BASE                       (W5_REG_BASE + 0x011C)
#define W5_TEMP_SIZE                            (W5_REG_BASE + 0x0120)
#define W5_ADDR_SEC_AXI                         (W5_REG_BASE + 0x0124)
#define W5_SEC_AXI_SIZE                         (W5_REG_BASE + 0x0128)
#define W5_HW_OPTION                            (W5_REG_BASE + 0x012C)
#define W5_TIMEOUT_CNT                          (W5_REG_BASE + 0x0130)
#define W5_CMD_INIT_NUM_TASK_BUF                (W5_REG_BASE + 0x0134)
#define W5_CMD_INIT_ADDR_TASK_BUF0              (W5_REG_BASE + 0x0138)
#define W5_CMD_INIT_ADDR_TASK_BUF1              (W5_REG_BASE + 0x013C)
#define W5_CMD_INIT_ADDR_TASK_BUF2              (W5_REG_BASE + 0x0140)
#define W5_CMD_INIT_ADDR_TASK_BUF3              (W5_REG_BASE + 0x0144)
#define W5_CMD_INIT_ADDR_TASK_BUF4              (W5_REG_BASE + 0x0148)
#define W5_CMD_INIT_ADDR_TASK_BUF5              (W5_REG_BASE + 0x014C)
#define W5_CMD_INIT_ADDR_TASK_BUF6              (W5_REG_BASE + 0x0150)
#define W5_CMD_INIT_ADDR_TASK_BUF7              (W5_REG_BASE + 0x0154)
#define W5_CMD_INIT_ADDR_TASK_BUF8              (W5_REG_BASE + 0x0158)
#define W5_CMD_INIT_ADDR_TASK_BUF9              (W5_REG_BASE + 0x015C)
#define W5_CMD_INIT_ADDR_TASK_BUFA              (W5_REG_BASE + 0x0160)
#define W5_CMD_INIT_ADDR_TASK_BUFB              (W5_REG_BASE + 0x0164)
#define W5_CMD_INIT_ADDR_TASK_BUFC              (W5_REG_BASE + 0x0168)
#define W5_CMD_INIT_ADDR_TASK_BUFD              (W5_REG_BASE + 0x016C)
#define W5_CMD_INIT_ADDR_TASK_BUFE              (W5_REG_BASE + 0x0170)
#define W5_CMD_INIT_ADDR_TASK_BUFF              (W5_REG_BASE + 0x0174)
#define W5_CMD_INIT_TASK_BUF_SIZE               (W5_REG_BASE + 0x0178)

/************************************************************************/
/* CREATE_INSTANCE - COMMON                                             */
/************************************************************************/
#define W5_ADDR_WORK_BASE                       (W5_REG_BASE + 0x0114)
#define W5_WORK_SIZE                            (W5_REG_BASE + 0x0118)
#define W5_CMD_DEC_BS_START_ADDR                (W5_REG_BASE + 0x011C)
#define W5_CMD_DEC_BS_SIZE                      (W5_REG_BASE + 0x0120)
#define W5_CMD_BS_PARAM                         (W5_REG_BASE + 0x0124)

/************************************************************************/
/* DECODER - INIT_SEQ                                                   */
/************************************************************************/

/************************************************************************/
/* SET_FRAME_BUF                                                        */
/************************************************************************/
/* SET_FB_OPTION 0x00       REGISTER FRAMEBUFFERS
                 0x01       UPDATE FRAMEBUFFER, just one framebuffer(linear, fbc and mvcol) 
 */
#define W5_SFB_OPTION                           (W5_REG_BASE + 0x0104)
#define W5_COMMON_PIC_INFO                      (W5_REG_BASE + 0x0118)
#define W5_PIC_SIZE                             (W5_REG_BASE + 0x011C)
#define W5_SET_FB_NUM                           (W5_REG_BASE + 0x0120)

#define W5_ADDR_LUMA_BASE0                      (W5_REG_BASE + 0x0134)
#define W5_ADDR_CB_BASE0                        (W5_REG_BASE + 0x0138)
#define W5_ADDR_CR_BASE0                        (W5_REG_BASE + 0x013C)
#define W5_ADDR_FBC_Y_OFFSET0                   (W5_REG_BASE + 0x013C)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET0                   (W5_REG_BASE + 0x0140)       // Compression offset table for Chroma
#define W5_ADDR_LUMA_BASE1                      (W5_REG_BASE + 0x0144)
#define W5_ADDR_CB_ADDR1                        (W5_REG_BASE + 0x0148)
#define W5_ADDR_CR_ADDR1                        (W5_REG_BASE + 0x014C)
#define W5_ADDR_FBC_Y_OFFSET1                   (W5_REG_BASE + 0x014C)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET1                   (W5_REG_BASE + 0x0150)       // Compression offset table for Chroma
#define W5_ADDR_LUMA_BASE2                      (W5_REG_BASE + 0x0154)
#define W5_ADDR_CB_ADDR2                        (W5_REG_BASE + 0x0158)
#define W5_ADDR_CR_ADDR2                        (W5_REG_BASE + 0x015C)
#define W5_ADDR_FBC_Y_OFFSET2                   (W5_REG_BASE + 0x015C)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET2                   (W5_REG_BASE + 0x0160)       // Compression offset table for Chroma
#define W5_ADDR_LUMA_BASE3                      (W5_REG_BASE + 0x0164)
#define W5_ADDR_CB_ADDR3                        (W5_REG_BASE + 0x0168)
#define W5_ADDR_CR_ADDR3                        (W5_REG_BASE + 0x016C)
#define W5_ADDR_FBC_Y_OFFSET3                   (W5_REG_BASE + 0x016C)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET3                   (W5_REG_BASE + 0x0170)       // Compression offset table for Chroma
#define W5_ADDR_LUMA_BASE4                      (W5_REG_BASE + 0x0174)
#define W5_ADDR_CB_ADDR4                        (W5_REG_BASE + 0x0178)
#define W5_ADDR_CR_ADDR4                        (W5_REG_BASE + 0x017C)
#define W5_ADDR_FBC_Y_OFFSET4                   (W5_REG_BASE + 0x017C)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET4                   (W5_REG_BASE + 0x0180)       // Compression offset table for Chroma
#define W5_ADDR_LUMA_BASE5                      (W5_REG_BASE + 0x0184)
#define W5_ADDR_CB_ADDR5                        (W5_REG_BASE + 0x0188)
#define W5_ADDR_CR_ADDR5                        (W5_REG_BASE + 0x018C)
#define W5_ADDR_FBC_Y_OFFSET5                   (W5_REG_BASE + 0x018C)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET5                   (W5_REG_BASE + 0x0190)       // Compression offset table for Chroma
#define W5_ADDR_LUMA_BASE6                      (W5_REG_BASE + 0x0194)
#define W5_ADDR_CB_ADDR6                        (W5_REG_BASE + 0x0198)
#define W5_ADDR_CR_ADDR6                        (W5_REG_BASE + 0x019C)
#define W5_ADDR_FBC_Y_OFFSET6                   (W5_REG_BASE + 0x019C)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET6                   (W5_REG_BASE + 0x01A0)       // Compression offset table for Chroma
#define W5_ADDR_LUMA_BASE7                      (W5_REG_BASE + 0x01A4)
#define W5_ADDR_CB_ADDR7                        (W5_REG_BASE + 0x01A8)
#define W5_ADDR_CR_ADDR7                        (W5_REG_BASE + 0x01AC)
#define W5_ADDR_FBC_Y_OFFSET7                   (W5_REG_BASE + 0x01AC)       // Compression offset table for Luma
#define W5_ADDR_FBC_C_OFFSET7                   (W5_REG_BASE + 0x01B0)       // Compression offset table for Chroma
#define W5_ADDR_MV_COL0                         (W5_REG_BASE + 0x01B4)
#define W5_ADDR_MV_COL1                         (W5_REG_BASE + 0x01B8)
#define W5_ADDR_MV_COL2                         (W5_REG_BASE + 0x01BC)
#define W5_ADDR_MV_COL3                         (W5_REG_BASE + 0x01C0)
#define W5_ADDR_MV_COL4                         (W5_REG_BASE + 0x01C4)
#define W5_ADDR_MV_COL5                         (W5_REG_BASE + 0x01C8)
#define W5_ADDR_MV_COL6                         (W5_REG_BASE + 0x01CC)
#define W5_ADDR_MV_COL7                         (W5_REG_BASE + 0x01D0)

/* UPDATE_FB */
/* CMD_SET_FB_STRIDE [15:0]     - FBC framebuffer stride
                     [31:15]    - Linear framebuffer stride
 */
#define W5_CMD_SET_FB_STRIDE                    (W5_REG_BASE + 0x0118)
#define W5_CMD_SET_FB_INDEX                     (W5_REG_BASE + 0x0120)
#define W5_ADDR_LUMA_BASE                       (W5_REG_BASE + 0x0134)
#define W5_ADDR_CB_BASE                         (W5_REG_BASE + 0x0138)
#define W5_ADDR_CR_BASE                         (W5_REG_BASE + 0x013C)
#define W5_ADDR_MV_COL                          (W5_REG_BASE + 0x0140)
#define W5_ADDR_FBC_Y_BASE                      (W5_REG_BASE + 0x0144)
#define W5_ADDR_FBC_C_BASE                      (W5_REG_BASE + 0x0148)
#define W5_ADDR_FBC_Y_OFFSET                    (W5_REG_BASE + 0x014C)
#define W5_ADDR_FBC_C_OFFSET                    (W5_REG_BASE + 0x0150)

/************************************************************************/
/* DECODER - DEC_PIC                                                    */
/************************************************************************/
#define W5_CMD_DEC_VCORE_LIMIT                  (W5_REG_BASE + 0x0124)
/* Sequence change enable mask register
 * CMD_SEQ_CHANGE_ENABLE_FLAG [5]   profile_idc 
 *                            [16]  pic_width/height_in_luma_sample
 *                            [19]  sps_max_dec_pic_buffering, max_num_reorder, max_latency_increase
 */
#define W5_CMD_SEQ_CHANGE_ENABLE_FLAG           (W5_REG_BASE + 0x0128)
#define W5_CMD_DEC_USER_MASK                    (W5_REG_BASE + 0x012C)
#define W5_CMD_DEC_TEMPORAL_ID_PLUS1            (W5_REG_BASE + 0x0130)
#define W5_CMD_DEC_FORCE_FB_LATENCY_PLUS1       (W5_REG_BASE + 0x0134)
#define W5_USE_SEC_AXI                          (W5_REG_BASE + 0x0138)

/************************************************************************/
/* DECODER - QUERY : GET_VPU_INFO                                       */
/************************************************************************/
#define W5_RET_FW_VERSION                       (W5_REG_BASE + 0x0118)
#define W5_RET_PRODUCT_NAME                     (W5_REG_BASE + 0x011C)
#define W5_RET_PRODUCT_VERSION                  (W5_REG_BASE + 0x0120)
#define W5_RET_STD_DEF0                         (W5_REG_BASE + 0x0124)
#define W5_RET_STD_DEF1                         (W5_REG_BASE + 0x0128)
#define W5_RET_CONF_FEATURE                     (W5_REG_BASE + 0x012C)
#define W5_RET_CONF_DATE                        (W5_REG_BASE + 0x0130)
#define W5_RET_CONF_REVISION                    (W5_REG_BASE + 0x0134)
#define W5_RET_CONF_TYPE                        (W5_REG_BASE + 0x0138)
#define W5_RET_PRODUCT_ID                       (W5_REG_BASE + 0x013C)
#define W5_RET_CUSTOMER_ID                      (W5_REG_BASE + 0x0140)


/************************************************************************/
/* DECODER - QUERY : GET_RESULT                                         */
/************************************************************************/
#define W5_CMD_DEC_ADDR_REPORT_BASE         (W5_REG_BASE + 0x0114)
#define W5_CMD_DEC_REPORT_SIZE              (W5_REG_BASE + 0x0118)
#define W5_CMD_DEC_REPORT_PARAM             (W5_REG_BASE + 0x011C)

#define W5_RET_DEC_BS_RD_PTR                (W5_REG_BASE + 0x011C)
#define W5_RET_DEC_SEQ_PARAM                (W5_REG_BASE + 0x0120)
#define W5_RET_DEC_COLOR_SAMPLE_INFO        (W5_REG_BASE + 0x0124)
#define W5_RET_DEC_ASPECT_RATIO             (W5_REG_BASE + 0x0128)
#define W5_RET_DEC_BIT_RATE                 (W5_REG_BASE + 0x012C)
#define W5_RET_DEC_FRAME_RATE_NR            (W5_REG_BASE + 0x0130)
#define W5_RET_DEC_FRAME_RATE_DR            (W5_REG_BASE + 0x0134)
#define W5_RET_DEC_FRAMEBUF_NEEDED          (W5_REG_BASE + 0x0138)
#define W5_RET_DEC_NUM_REORDER_DELAY        (W5_REG_BASE + 0x013C)
#define W5_RET_DEC_SUB_LAYER_INFO           (W5_REG_BASE + 0x0140)
#define W5_RET_DEC_NOTIFICATION             (W5_REG_BASE + 0x0144)
#define W5_RET_DEC_USERDATA_IDC             (W5_REG_BASE + 0x0148)
#define W5_RET_DEC_PIC_SIZE                 (W5_REG_BASE + 0x014C)
#define W5_RET_DEC_CROP_TOP_BOTTOM          (W5_REG_BASE + 0x0150)
#define W5_RET_DEC_CROP_LEFT_RIGHT          (W5_REG_BASE + 0x0154)
#define W5_RET_DEC_AU_START_POS             (W5_REG_BASE + 0x0158)
#define W5_RET_DEC_AU_END_POS               (W5_REG_BASE + 0x015C)
#define W5_RET_DEC_PIC_TYPE                 (W5_REG_BASE + 0x0160)
#define W5_RET_DEC_PIC_POC                  (W5_REG_BASE + 0x0164)
#define W5_RET_DEC_RECOVERY_POINT           (W5_REG_BASE + 0x0168)
#define W5_RET_DEC_DEBUG_INDEX              (W5_REG_BASE + 0x016C)
#define W5_RET_DEC_DECODED_INDEX            (W5_REG_BASE + 0x0170)
#define W5_RET_DEC_DISPLAY_INDEX            (W5_REG_BASE + 0x0174)
#define W5_RET_DEC_REALLOC_INDEX            (W5_REG_BASE + 0x0178)
#define W5_RET_DEC_DISP_FLAG                (W5_REG_BASE + 0x017C)
#define W5_RET_DEC_ERR_CTB_NUM              (W5_REG_BASE + 0x0180)

#define W5_RET_DEC_SEEK_CYCLE               (W5_REG_BASE + 0x01C0)
#define W5_RET_DEC_PARSING_CYCLE            (W5_REG_BASE + 0x01C4)
#define W5_RET_DEC_DECODING_CYCLE           (W5_REG_BASE + 0x01C8)

#define W5_FRAME_CYCLE                      (W5_REG_BASE + 0x01D0)
#define W5_RET_DEC_WARN_INFO                (W5_REG_BASE + 0x01D4)
#define W5_RET_DEC_ERR_INFO                 (W5_REG_BASE + 0x01D8)
#define W5_RET_DEC_DECODING_SUCCESS         (W5_REG_BASE + 0x01DC)

/************************************************************************/
/* DECODER - FLUSH_INSTANCE                                             */
/************************************************************************/
#define W5_CMD_FLUSH_INST_OPT               (W5_REG_BASE + 0x104)

/************************************************************************/
/* DECODER - QUERY : UPDATE_DISP_FLAG                                   */
/************************************************************************/
#define W5_CMD_DEC_SET_DISP_IDC             (W5_REG_BASE + 0x0118)
#define W5_CMD_DEC_CLR_DISP_IDC             (W5_REG_BASE + 0x011C)
#define W5_RET_DEC_DISP_IDC                 (W5_REG_BASE + 0x017C)

/************************************************************************/
/* GDI register for Debugging                                           */
/************************************************************************/
#define W5_GDI_BASE                         0x8800                  // FIX ME
#define W5_GDI_BUS_CTRL                     (W5_GDI_BASE + 0x0F0)
#define W5_GDI_BUS_STATUS                   (W5_GDI_BASE + 0x0F4)

#define W5_ENC_GDI_BASE                     0xFE00
#define W5_ENC_GDI_BUS_CTRL                 (W5_ENC_GDI_BASE + 0x010)
#define W5_ENC_GDI_BUS_STATUS               (W5_ENC_GDI_BASE + 0x014)

/************************************************************************/
/*                                                                      */
/*               For  ENCODER                                           */             
/*                                                                      */
/************************************************************************/
#define W5_RET_STAGE3_INSTANCE_INFO             (W5_REG_BASE + 0x1F8)
/************************************************************************/
/* ENCODER - CREATE_INSTANCE                                            */
/************************************************************************/
// 0x114 ~ 0x124 : defined above (CREATE_INSTANCE COMMON)
#define W5_CMD_ENC_VCORE_LIMIT                  (W5_REG_BASE + 0x128)

/************************************************************************/
/* ENCODER - SET_FB                                                     */
/************************************************************************/
#define W5_FBC_STRIDE                           (W5_REG_BASE + 0x128)
#define W5_ADDR_SUB_SAMPLED_FB_BASE             (W5_REG_BASE + 0x12C)
#define W5_SUB_SAMPLED_ONE_FB_SIZE              (W5_REG_BASE + 0x130)

/************************************************************************/
/* ENCODER - ENC_SET_PARAM (COMMON & CHANGE_PARAM)                      */
/************************************************************************/
#define W5_CMD_ENC_SEQ_SET_PARAM_OPTION         (W5_REG_BASE + 0x104)
#define W5_CMD_ENC_SEQ_SET_PARAM_ENABLE         (W5_REG_BASE + 0x118)
#define W5_CMD_ENC_SEQ_SRC_SIZE                 (W5_REG_BASE + 0x11C)
#define W5_CMD_ENC_SEQ_CUSTOM_MAP_ENDIAN        (W5_REG_BASE + 0x120)
#define W5_CMD_ENC_SEQ_SYNTAX_SPS_PARAM         (W5_REG_BASE + 0x124)
#define W5_CMD_ENC_SEQ_SYNTAX_PPS_PARAM         (W5_REG_BASE + 0x128)
#define W5_CMD_ENC_SEQ_GOP_PARAM                (W5_REG_BASE + 0x12C)
#define W5_CMD_ENC_SEQ_INTRA_PARAM              (W5_REG_BASE + 0x130)
#define W5_CMD_ENC_SEQ_CONF_WIN_TOP_BOT         (W5_REG_BASE + 0x134)
#define W5_CMD_ENC_SEQ_CONF_WIN_LEFT_RIGHT      (W5_REG_BASE + 0x138)
#define W5_CMD_ENC_SEQ_RDO_PARAM                (W5_REG_BASE + 0x13C)
#define W5_CMD_ENC_SEQ_INDEPENDENT_SLICE        (W5_REG_BASE + 0x140)
#define W5_CMD_ENC_SEQ_DEPENDENT_SLICE          (W5_REG_BASE + 0x144)
#define W5_CMD_ENC_SEQ_INTRA_REFRESH            (W5_REG_BASE + 0x148)

#define W5_CMD_ENC_SEQ_RC_FRAME_RATE            (W5_REG_BASE + 0x150)
#define W5_CMD_ENC_SEQ_RC_TARGET_RATE           (W5_REG_BASE + 0x154)
#define W5_CMD_ENC_SEQ_RC_PARAM                 (W5_REG_BASE + 0x158)
#define W5_CMD_ENC_SEQ_RC_MIN_MAX_QP            (W5_REG_BASE + 0x15C)
#define W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_0_3   (W5_REG_BASE + 0x160)
#define W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_4_7   (W5_REG_BASE + 0x164)

#define W5_CMD_ENC_SEQ_ROT_PARAM                (W5_REG_BASE + 0x170)
#define W5_CMD_ENC_SEQ_NUM_UNITS_IN_TICK        (W5_REG_BASE + 0x174)
#define W5_CMD_ENC_SEQ_TIME_SCALE               (W5_REG_BASE + 0x178)
#define W5_CMD_ENC_SEQ_NUM_TICKS_POC_DIFF_ONE   (W5_REG_BASE + 0x17C)

#define W5_CMD_ENC_SEQ_CUSTOM_MD_PU04           (W5_REG_BASE + 0x184)
#define W5_CMD_ENC_SEQ_CUSTOM_MD_PU08           (W5_REG_BASE + 0x188)
#define W5_CMD_ENC_SEQ_CUSTOM_MD_PU16           (W5_REG_BASE + 0x18C)
#define W5_CMD_ENC_SEQ_CUSTOM_MD_PU32           (W5_REG_BASE + 0x190)
#define W5_CMD_ENC_SEQ_CUSTOM_MD_CU08           (W5_REG_BASE + 0x194)
#define W5_CMD_ENC_SEQ_CUSTOM_MD_CU16           (W5_REG_BASE + 0x198)
#define W5_CMD_ENC_SEQ_CUSTOM_MD_CU32           (W5_REG_BASE + 0x19C)
#define W5_CMD_ENC_SEQ_NR_PARAM                 (W5_REG_BASE + 0x1A0)
#define W5_CMD_ENC_SEQ_NR_WEIGHT                (W5_REG_BASE + 0x1A4)
#define W5_CMD_ENC_SEQ_BG_PARAM                 (W5_REG_BASE + 0x1A8)
#define W5_CMD_ENC_SEQ_CUSTOM_LAMBDA_ADDR       (W5_REG_BASE + 0x1AC)
#define W5_CMD_ENC_SEQ_USER_SCALING_LIST_ADDR   (W5_REG_BASE + 0x1B0)

/************************************************************************/
/* ENCODER - ENC_SET_PARAM (CUSTOM_GOP)                                 */
/************************************************************************/
#define W5_CMD_ENC_CUSTOM_GOP_PARAM             (W5_REG_BASE + 0x11C)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_0       (W5_REG_BASE + 0x120)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_1       (W5_REG_BASE + 0x124)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_2       (W5_REG_BASE + 0x128)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_3       (W5_REG_BASE + 0x12C)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_4       (W5_REG_BASE + 0x130)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_5       (W5_REG_BASE + 0x134)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_6       (W5_REG_BASE + 0x138)
#define W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_7       (W5_REG_BASE + 0x13C)

/************************************************************************/
/* ENCODER - ENC_PIC                                                    */
/************************************************************************/
#define W5_CMD_ENC_BS_START_ADDR                (W5_REG_BASE + 0x118)
#define W5_CMD_ENC_BS_SIZE                      (W5_REG_BASE + 0x11C)
#define W5_CMD_ENC_PIC_USE_SEC_AXI              (W5_REG_BASE + 0x124)
#define W5_CMD_ENC_PIC_REPORT_PARAM             (W5_REG_BASE + 0x128)
#define W5_CMD_ENC_PIC_REPORT_ENDIAN            (W5_REG_BASE + 0x12C)

#define W5_CMD_ENC_PIC_CUSTOM_MAP_OPTION_PARAM  (W5_REG_BASE + 0x138)
#define W5_CMD_ENC_PIC_CUSTOM_MAP_OPTION_ADDR   (W5_REG_BASE + 0x13C)

#define W5_CMD_ENC_PIC_SRC_PIC_IDX              (W5_REG_BASE + 0x144)
#define W5_CMD_ENC_PIC_SRC_ADDR_Y               (W5_REG_BASE + 0x148)
#define W5_CMD_ENC_PIC_SRC_ADDR_U               (W5_REG_BASE + 0x14C)
#define W5_CMD_ENC_PIC_SRC_ADDR_V               (W5_REG_BASE + 0x150)
#define W5_CMD_ENC_PIC_SRC_STRIDE               (W5_REG_BASE + 0x154)
#define W5_CMD_ENC_PIC_SRC_FORMAT               (W5_REG_BASE + 0x158)

#define W5_CMD_ENC_PIC_CODE_OPTION              (W5_REG_BASE + 0x164)
#define W5_CMD_ENC_PIC_PIC_PARAM                (W5_REG_BASE + 0x168)
#define W5_CMD_ENC_PIC_LONGTERM_PIC             (W5_REG_BASE + 0x16C)
#define W5_CMD_ENC_PIC_WP_PIXEL_VAR_Y           (W5_REG_BASE + 0x170)
#define W5_CMD_ENC_PIC_WP_PIXEL_VAR_CB          (W5_REG_BASE + 0x174)
#define W5_CMD_ENC_PIC_WP_PIXEL_VAR_CR          (W5_REG_BASE + 0x178)
#define W5_CMD_ENC_PIC_WP_PIXEL_MEAN            (W5_REG_BASE + 0x17C)

/************************************************************************/
/* ENCODER - QUERY (GET_RESULT)                                         */
/************************************************************************/
#define W5_RET_ENC_RD_PTR                       (W5_REG_BASE + 0x114)
#define W5_RET_ENC_WR_PTR                       (W5_REG_BASE + 0x118)
#define W5_RET_ENC_MIN_FB_NUM                   (W5_REG_BASE + 0x11C)
#define W5_RET_ENC_MIN_SRC_BUF_NUM              (W5_REG_BASE + 0x120)
#define W5_RET_ENC_PIC_TYPE                     (W5_REG_BASE + 0x124)
#define W5_RET_ENC_PIC_POC                      (W5_REG_BASE + 0x128)
#define W5_RET_ENC_PIC_IDX                      (W5_REG_BASE + 0x12C)
#define W5_RET_ENC_PIC_SLICE_NUM                (W5_REG_BASE + 0x130)
#define W5_RET_ENC_PIC_SKIP                     (W5_REG_BASE + 0x134)
#define W5_RET_ENC_PIC_NUM_INTRA                (W5_REG_BASE + 0x138)
#define W5_RET_ENC_PIC_NUM_MERGE                (W5_REG_BASE + 0x13C)

#define W5_RET_ENC_PIC_NUM_SKIP                 (W5_REG_BASE + 0x144)
#define W5_RET_ENC_PIC_AVG_CTU_QP               (W5_REG_BASE + 0x148)
#define W5_RET_ENC_PIC_BYTE                     (W5_REG_BASE + 0x14C)
#define W5_RET_ENC_GOP_PIC_IDX                  (W5_REG_BASE + 0x150)
#define W5_RET_ENC_USED_SRC_IDX                 (W5_REG_BASE + 0x154)
#define W5_RET_ENC_PIC_NUM                      (W5_REG_BASE + 0x158)
#define W5_RET_ENC_VCL_NUT                      (W5_REG_BASE + 0x15C)


#define W5_RET_ENC_PREPARE_CYCLE                (W5_REG_BASE + 0x1C0)
#define W5_RET_ENC_PROCESSING_CYCLE             (W5_REG_BASE + 0x1C4)
#define W5_RET_ENC_ENCODING_CYCLE               (W5_REG_BASE + 0x1C8)

#define W5_RET_ENC_WARN_INFO                    (W5_REG_BASE + 0x1D4)
#define W5_RET_ENC_ERR_INFO                     (W5_REG_BASE + 0x1D8)
#define W5_RET_ENC_ENCODING_SUCCESS             (W5_REG_BASE + 0x1DC)

#define W5_CMD_QUERY_REPORT_BASE                (W5_REG_BASE + 0x17C)   // FIX ME

/************************************************************************/
/* ENCODER - QUERY (GET_BW_REPORT)                                      */
/************************************************************************/
#define RET_QUERY_BW_PRP_AXI_READ               (W5_REG_BASE + 0x118)
#define RET_QUERY_BW_PRP_AXI_WRITE              (W5_REG_BASE + 0x11C)
#define RET_QUERY_BW_FBD_Y_AXI_READ             (W5_REG_BASE + 0x120)
#define RET_QUERY_BW_FBC_Y_AXI_WRITE            (W5_REG_BASE + 0x124)
#define RET_QUERY_BW_FBD_C_AXI_READ             (W5_REG_BASE + 0x128)
#define RET_QUERY_BW_FBC_C_AXI_WRITE            (W5_REG_BASE + 0x12C)
#define RET_QUERY_BW_PRI_AXI_READ               (W5_REG_BASE + 0x130)
#define RET_QUERY_BW_PRI_AXI_WRITE              (W5_REG_BASE + 0x134)
#define RET_QUERY_BW_SEC_AXI_READ               (W5_REG_BASE + 0x138)
#define RET_QUERY_BW_SEC_AXI_WRITE              (W5_REG_BASE + 0x13C)
#define RET_QUERY_BW_PROC_AXI_READ              (W5_REG_BASE + 0x140)
#define RET_QUERY_BW_PROC_AXI_WRITE             (W5_REG_BASE + 0x144)

#endif /* __WAVE510_REGISTER_DEFINE_H__ */

