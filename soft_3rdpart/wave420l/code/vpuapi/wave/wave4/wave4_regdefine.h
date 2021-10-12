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

#ifndef __WAVE410_REGISTER_DEFINE_H__
#define __WAVE410_REGISTER_DEFINE_H__

#define W4_REG_BASE                     0x00000000
#define W4_CMD_REG_BASE                 0x00000100
#define W4_CMD_REG_END                  0x00000200

/************************************************************************/
/* DECODER - DEC_PIC_HDR/DEC_PIC                                        */
/************************************************************************/
#define W4_CMD_DEC_USER_MASK                (W4_REG_BASE + 0x0174)
#define W4_CMD_DEC_SEVERITY_LEVEL           (W4_REG_BASE + 0x0178)
#define W4_CMD_DEC_VCORE_LIMIT              (W4_REG_BASE + 0x017C)
#define W4_CMD_DEC_SKIP_OPTION              (W4_REG_BASE + 0x0184)
#define W4_CMD_DEC_FORCE_FB_LATENCY_PLUS1   (W4_REG_BASE + 0x0188)
#define W4_CMD_DEC_TEMPORAL_ID_PLUS1        (W4_REG_BASE + 0x018C)

/* Sequence change enable mask register
 * CMD_SEQ_CHANGE_ENABLE_FLAG [5]   profile_idc 
 *                            [16]  pic_width/height_in_luma_sample
 *                            [19]  sps_max_dec_pic_buffering, max_num_reorder, max_latency_increase
 */
#define W4_CMD_SEQ_CHANGE_ENABLE_FLAG       (W4_REG_BASE + 0x0194)

#define W4_RET_DEC_USER_MASK                (W4_REG_BASE + 0x0174)
/* Profile/Level/Tier/Max sublayers...
 * LEVEL_IDC                [7:0]       general_level_idc
 * FRAME_ONLY_CONSTRAINT_FLAG [8]       general_frame_only_constraint_flag 
 * NON_PACKED_CONSTRAINT_FLAG [9]       general_non_packed_constraint_flag
 * INTERLACE_SOUCE_FLAG       [10]      general_interlaced_source_flag
 * PROGRESSIVE_SOURCE_FLAG    [11]      general_progressive_source_flag
 * PROFILE_COMPATIBLE_FLAG[8] [19:12]   
 * SPS_MAX_SUB_LAYER          [23:21]   
 * PROFILE_IDC                [28:24]
 * TIER_FLAG                  [29]
 * PROFILE_SPACE              [31:30]
 */ 
#define W4_RET_DEC_SEQ_PARAM                (W4_REG_BASE + 0x0198)
#define W4_RET_DEC_DECODED_ID               (W4_REG_BASE + 0x019C)
#define W4_RET_DEC_COLOR_SAMPLE_INFO        (W4_REG_BASE + 0x01A0)
#define W4_RET_DEC_ASPECT_RATIO             (W4_REG_BASE + 0x01A4)
#define W4_RET_DEC_BIT_RATE                 (W4_REG_BASE + 0x01A8)
#define W4_RET_DEC_FRAME_RATE_NR            (W4_REG_BASE + 0x01AC)
#define W4_RET_DEC_FRAME_RATE_DR            (W4_REG_BASE + 0x01B0)
/* conformance_window:  top, bottom
 * bottom                   [15:0]      conformance_window_bottom_offset
 * top                      [31:16]     conformance_window_top_offset
 */
#define W4_RET_DEC_CROP_TOP_BOTTOM          (W4_REG_BASE + 0x01B4)
/* conformance_window: left, right
 * right                    [15:0]      conformance_window_left_offset
 * left                     [31:16]     conformance_window_right_offset
 */
#define W4_RET_DEC_CROP_LEFT_RIGHT          (W4_REG_BASE + 0x01B8)
#define W4_RET_DEC_PIC_SIZE                 (W4_REG_BASE + 0x01BC)
#define W4_RET_DEC_FRAMEBUF_NEEDED          (W4_REG_BASE + 0x01C0)
#define W4_RET_DEC_NUM_REORDER_DELAY        (W4_REG_BASE + 0x0188)
#define W4_RET_DEC_INTER_RES_CHANGE_REALLOCATED_DPB_IDX         (W4_REG_BASE + 0x0188)
/* Sequence change result register
 * RET_SEQ_CHANGE_FLAG   [5]   profile_idc changed
 *                       [16]  pic_width/height_in_luma_sample changed
 *                       [19]  sps_max_dec_pic_buffering, max_num_reorder or max_latency_increase changed
 */
#define W4_RET_DEC_SEQ_CHANGE_FLAG          (W4_REG_BASE + 0x01C4)
#define W4_RET_DEC_RECOVERY_POINT           (W4_REG_BASE + 0x01C8)
#define W4_RET_DEC_VCORE_USED               (W4_REG_BASE + 0x01CC)
#define W4_RET_TEMP_SUB_LAYER_INFO          (W4_REG_BASE + 0x01D0)
#define W4_RET_FRAME_CYCLE                  (W4_REG_BASE + 0x01D4)
#define W4_RET_DEC_WARN_INFO                (W4_REG_BASE + 0x01D8)
#define W4_RET_DEC_PRESCAN_INDEX            (W4_REG_BASE + 0x01DC)
#define W4_RET_DEC_DECODED_INDEX            (W4_REG_BASE + 0x01E0)
#define W4_RET_DEC_DISPLAY_INDEX               (W4_REG_BASE + 0x01E4)
/* I_SLICE_DECODED          [0]
 * P_SLICE_DECODED          [1]
 * B_SLICE_DECODED          [2]
 * VCL_NAL_UNIT_TYPE        [9:4]       See Table 7-1 in H.265 specification.
 */
#define W4_RET_DEC_PIC_TYPE                 (W4_REG_BASE + 0x01E8)
#define W4_RET_DEC_PIC_POC                  (W4_REG_BASE + 0x01EC)
#define W4_RET_DEC_ERR_CTB_NUM              (W4_REG_BASE + 0x01F0)
#define W4_RET_DEC_ERR_INFO                 (W4_REG_BASE + 0x01F4)
#define W4_RET_DEC_AU_START_POS             (W4_REG_BASE + 0x01F8)
#define W4_RET_DEC_AU_END_POS               (W4_REG_BASE + 0x01FC)

#define W4_ENC_SUB_FRAME_SYNC_IF             (W4_REG_BASE + 0x00F0)
#define W4_CMD_ENC_SUB_FRAME_SYNC_CONFIG     (W4_REG_BASE + 0x019C)
/************************************************************************/
/* ENCODER - SET_PARAM (in common)                                      */
/************************************************************************/
#define W4_CMD_ENC_SET_PARAM_OPTION          (W4_REG_BASE + 0x010C)
#define W4_ROLLBACK_OPTION                   (W4_REG_BASE + 0x011C)

/* same as the WAVE410 host registers
 *#define W4_BS_START_ADDR                    (W4_REG_BASE + 0x0120)
 *#define W4_BS_SIZE                          (W4_REG_BASE + 0x0124)
 *#define W4_BS_PARAM                         (W4_REG_BASE + 0x0128)
 *#define W4_BS_OPTION                        (W4_REG_BASE + 0x012C)
 *#define W4_BS_RD_PTR                        (W4_REG_BASE + 0x0130)
 *#define W4_BS_WR_PTR                        (W4_REG_BASE + 0x0134)
 *#define W4_ADDR_WORK_BASE                   (W4_REG_BASE + 0x0138)
 *#define W4_WORK_SIZE                        (W4_REG_BASE + 0x013C)
 *#define W4_WORK_PARAM                       (W4_REG_BASE + 0x0140)
 *#define W4_ADDR_TEMP_BASE                   (W4_REG_BASE + 0x0144)
 *#define W4_TEMP_SIZE                        (W4_REG_BASE + 0x0148)
 *#define W4_TEMP_PARAM                       (W4_REG_BASE + 0x014C)
 *#define W4_ADDR_SEC_AXI                     (W4_REG_BASE + 0x0150)
 *#define W4_SEC_AXI_SIZE                     (W4_REG_BASE + 0x0154)
 *#define W4_USE_SEC_AXI                      (W4_REG_BASE + 0x0158)
 */

/************************************************************************/
/* ENCODER - SET_PARAM + COMMON                                         */
/************************************************************************/
#define W4_CMD_ENC_SET_PARAM_ENABLE           (W4_REG_BASE + 0x015C)
#define W4_CMD_ENC_SEQ_SRC_SIZE               (W4_REG_BASE + 0x0160)  
#define W4_CMD_ENC_SEQ_PIC_PARAM              (W4_REG_BASE + 0x016C)
#define W4_CMD_ENC_SEQ_GOP_PARAM              (W4_REG_BASE + 0x0170)
#define W4_CMD_ENC_SEQ_INTRA_PARAM            (W4_REG_BASE + 0x0174)
#define W4_CMD_ENC_SEQ_CONF_WIN_TOP_BOT       (W4_REG_BASE + 0x0178)
#define W4_CMD_ENC_SEQ_CONF_WIN_LEFT_RIGHT    (W4_REG_BASE + 0x017C)
#define W4_CMD_ENC_SEQ_FRAME_RATE             (W4_REG_BASE + 0x0180)
#define W4_CMD_ENC_SEQ_INDEPENDENT_SLICE      (W4_REG_BASE + 0x0184)
#define W4_CMD_ENC_SEQ_DEPENDENT_SLICE        (W4_REG_BASE + 0x0188)
#define W4_CMD_ENC_SEQ_INTRA_REFRESH          (W4_REG_BASE + 0x018C)
#define W4_CMD_ENC_PARAM                      (W4_REG_BASE + 0x0190)
#define W4_CMD_ENC_SEQ_LAYER_PERIOD           (W4_REG_BASE + 0x0194)
#define W4_CMD_ENC_RC_INTRA_MIN_MAX_QP        (W4_REG_BASE + 0x0194)
#define W4_CMD_ENC_RC_PARAM                   (W4_REG_BASE + 0x0198)
#define W4_CMD_ENC_RC_MIN_MAX_QP              (W4_REG_BASE + 0x019C)
#define W4_CMD_ENC_RC_BIT_RATIO_LAYER_0_3     (W4_REG_BASE + 0x01A0)
#define W4_CMD_ENC_RC_BIT_RATIO_LAYER_4_7     (W4_REG_BASE + 0x01A4)
#define W4_CMD_ENC_NR_PARAM                   (W4_REG_BASE + 0x01A8)
#define W4_CMD_ENC_NR_WEIGHT                  (W4_REG_BASE + 0x01AC)
#define W4_CMD_ENC_NUM_UNITS_IN_TICK          (W4_REG_BASE + 0x01B0)
#define W4_CMD_ENC_TIME_SCALE                 (W4_REG_BASE + 0x01B4)
#define W4_CMD_ENC_NUM_TICKS_POC_DIFF_ONE     (W4_REG_BASE + 0x01B8)
#define W4_CMD_ENC_RC_TRANS_RATE              (W4_REG_BASE + 0x01BC)
#define W4_CMD_ENC_RC_TARGET_RATE             (W4_REG_BASE + 0x01C0)
#define W4_CMD_ENC_ROT_PARAM                  (W4_REG_BASE + 0x01C4)
#define W4_CMD_ENC_ROT_RESERVED               (W4_REG_BASE + 0x01C8)
#define W4_RET_ENC_MIN_FB_NUM                 (W4_REG_BASE + 0x01CC)
#define W4_RET_ENC_NAL_INFO_TO_BE_ENCODED     (W4_REG_BASE + 0x01D0)
#define W4_RET_ENC_MIN_SRC_BUF_NUM            (W4_REG_BASE + 0x01D8)

/************************************************************************/
/* ENCODER - SET_PARAM + CUSTOM_GOP                                     */
/************************************************************************/
#define W4_CMD_ENC_SET_CUSTOM_GOP_ENABLE      (W4_REG_BASE + 0x015C)
#define W4_CMD_ENC_CUSTOM_GOP_PARAM           (W4_REG_BASE + 0x0160)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_0     (W4_REG_BASE + 0x0164)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_1     (W4_REG_BASE + 0x0168)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_2     (W4_REG_BASE + 0x016C)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_3     (W4_REG_BASE + 0x0170)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_4     (W4_REG_BASE + 0x0174)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_5     (W4_REG_BASE + 0x0178)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_6     (W4_REG_BASE + 0x017C)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_PARAM_7     (W4_REG_BASE + 0x0180)
#define W4_CMD_ENC_CUSTOM_GOP_RESERVED        (W4_REG_BASE + 0x0184)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_0    (W4_REG_BASE + 0x0188)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_1    (W4_REG_BASE + 0x018C)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_2    (W4_REG_BASE + 0x0190)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_3    (W4_REG_BASE + 0x0194)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_4    (W4_REG_BASE + 0x0198)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_5    (W4_REG_BASE + 0x019C)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_6    (W4_REG_BASE + 0x01A0)
#define W4_CMD_ENC_CUSTOM_GOP_PIC_LAMBDA_7    (W4_REG_BASE + 0x01A4)

/************************************************************************/
/* ENCODER - SET_PARAM + CUSTOM_HEADER                                  */
/************************************************************************/
#define W4_CMD_ENC_CUSTOM_VPS_COMMON                    (W4_REG_BASE + 0x015C)
#define W4_CMD_ENC_CUSTOM_VPS_DEC_PIC_BUFFERING_MINUS1  (W4_REG_BASE + 0x0160)
#define W4_CMD_ENC_CUSTOM_VPS_NUM_REORDERING            (W4_REG_BASE + 0x0164)
#define W4_CMD_ENC_CUSTOM_VPS_MAX_LATENCY_PLUS1_0       (W4_REG_BASE + 0x0168)
#define W4_CMD_ENC_CUSTOM_VPS_MAX_LATENCY_PLUS1_1       (W4_REG_BASE + 0x016C)
#define W4_CMD_ENC_CUSTOM_VPS_MAX_LATENCY_PLUS1_2       (W4_REG_BASE + 0x0170)
#define W4_CMD_ENC_CUSTOM_VPS_MAX_LATENCY_PLUS1_3       (W4_REG_BASE + 0x0174)
#define W4_CMD_ENC_CUSTOM_VPS_MAX_LATENCY_PLUS1_4       (W4_REG_BASE + 0x0178)
#define W4_CMD_ENC_CUSTOM_VPS_MAX_LATENCY_PLUS1_5       (W4_REG_BASE + 0x017C)
#define W4_CMD_ENC_CUSTOM_VPS_MAX_LATENCY_PLUS1_6       (W4_REG_BASE + 0x0180)
#define W4_CMD_ENC_CUSTOM_VPS_LAYER_ID_0                (W4_REG_BASE + 0x0184)
#define W4_CMD_ENC_CUSTOM_VPS_LAYER_ID_1                (W4_REG_BASE + 0x0188)

#define W4_CMD_ENC_CUSTOM_SPS_COMMON                    (W4_REG_BASE + 0x018C)
#define W4_CMD_ENC_CUSTOM_SPS_DEC_PIC_BUFFERING_MINUS1  (W4_REG_BASE + 0x0190)
#define W4_CMD_ENC_CUSTOM_SPS_NUM_REORDERING            (W4_REG_BASE + 0x0194)
#define W4_CMD_ENC_CUSTOM_SPS_MAX_LATENCY_PLUS1_0       (W4_REG_BASE + 0x0198)
#define W4_CMD_ENC_CUSTOM_SPS_MAX_LATENCY_PLUS1_1       (W4_REG_BASE + 0x019C)
#define W4_CMD_ENC_CUSTOM_SPS_MAX_LATENCY_PLUS1_2       (W4_REG_BASE + 0x01A0)
#define W4_CMD_ENC_CUSTOM_SPS_MAX_LATENCY_PLUS1_3       (W4_REG_BASE + 0x01A4)
#define W4_CMD_ENC_CUSTOM_SPS_MAX_LATENCY_PLUS1_4       (W4_REG_BASE + 0x01A8)
#define W4_CMD_ENC_CUSTOM_SPS_MAX_LATENCY_PLUS1_5       (W4_REG_BASE + 0x01AC)
#define W4_CMD_ENC_CUSTOM_SPS_MAX_LATENCY_PLUS1_6       (W4_REG_BASE + 0x01B0)

#define W4_CMD_ENC_CUSTOM_PPS_COMMON                    (W4_REG_BASE + 0x01B4)
/************************************************************************/
/* ENCODER - SET_PARAM + VUI                                            */
/************************************************************************/
#define W4_CMD_ENC_VUI_PARAM_FLAGS            (W4_REG_BASE + 0x015C)
#define W4_CMD_ENC_VUI_ASPECT_RATIO_IDC       (W4_REG_BASE + 0x0160)
#define W4_CMD_ENC_VUI_SAR_SIZE               (W4_REG_BASE + 0x0164)
#define W4_CMD_ENC_VUI_OVERSCAN_APPROPRIATE   (W4_REG_BASE + 0x0168)
#define W4_CMD_ENC_VUI_VIDEO_SIGNAL           (W4_REG_BASE + 0x016C)
#define W4_CMD_ENC_VUI_CHROMA_SAMPLE_LOC      (W4_REG_BASE + 0x0170)
#define W4_CMD_ENC_VUI_DISP_WIN_LEFT_RIGHT    (W4_REG_BASE + 0x0174)
#define W4_CMD_ENC_VUI_DISP_WIN_TOP_BOT       (W4_REG_BASE + 0x0178)

#define W4_CMD_ENC_VUI_HRD_RBSP_PARAM_FLAG    (W4_REG_BASE + 0x017C)
#define W4_CMD_ENC_VUI_RBSP_ADDR              (W4_REG_BASE + 0x0180)
#define W4_CMD_ENC_VUI_RBSP_SIZE              (W4_REG_BASE + 0x0184)
#define W4_CMD_ENC_HRD_RBSP_ADDR              (W4_REG_BASE + 0x0188)
#define W4_CMD_ENC_HRD_RBSP_SIZE              (W4_REG_BASE + 0x018C)

/************************************************************************/
/* ENCODER - ENC_PIC                                                    */
/************************************************************************/
#define W4_CMD_ENC_ADDR_REPORT_BASE           (W4_REG_BASE + 0x015C)
#define W4_CMD_ENC_REPORT_SIZE                (W4_REG_BASE + 0x0160)
#define W4_CMD_ENC_REPORT_PARAM               (W4_REG_BASE + 0x0164)
#define W4_CMD_ENC_CODE_OPTION                (W4_REG_BASE + 0x0168)
#define W4_CMD_ENC_PIC_PARAM                  (W4_REG_BASE + 0x016C)
#define W4_CMD_ENC_SRC_PIC_IDX                (W4_REG_BASE + 0x0170)
#define W4_CMD_ENC_SRC_ADDR_Y                 (W4_REG_BASE + 0x0174)
#define W4_CMD_ENC_SRC_ADDR_U                 (W4_REG_BASE + 0x0178)
#define W4_CMD_ENC_SRC_ADDR_V                 (W4_REG_BASE + 0x017C)
#define W4_CMD_ENC_SRC_STRIDE                 (W4_REG_BASE + 0x0180)
#define W4_CMD_ENC_SRC_FORMAT                 (W4_REG_BASE + 0x0184)

#define W4_CMD_ENC_PREFIX_SEI_NAL_ADDR        (W4_REG_BASE + 0x0188)
#define W4_CMD_ENC_PREFIX_SEI_INFO            (W4_REG_BASE + 0x018C)
#define W4_CMD_ENC_SUFFIX_SEI_NAL_ADDR        (W4_REG_BASE + 0x0190)
#define W4_CMD_ENC_SUFFIX_SEI_INFO            (W4_REG_BASE + 0x0194)

#define W4_CMD_ENC_LONGTERM_PIC               (W4_REG_BASE + 0x0198)
                                             /*(W4_REG_BASE + 0x019C) = used for SUB_FRAME_SYNC */
#define W4_CMD_ENC_CTU_OPT_PARAM              (W4_REG_BASE + 0x01A0)
#define W4_CMD_ENC_ROI_ADDR_CTU_MAP           (W4_REG_BASE + 0x01A4)
#define W4_CMD_ENC_CTU_QP_MAP_ADDR            (W4_REG_BASE + 0x01AC)
#define W4_CMD_ENC_SRC_TIMESTAMP_LOW          (W4_REG_BASE + 0x01B0)
#define W4_CMD_ENC_SRC_TIMESTAMP_HIGH         (W4_REG_BASE + 0x01B4)


#define W4_CMD_ENC_FC_PARAM                   (W4_REG_BASE + 0x01E8)
#define W4_CMD_ENC_FC_TABLE_ADDR_Y            (W4_REG_BASE + 0x01EC)
#define W4_CMD_ENC_FC_TABLE_ADDR_C            (W4_REG_BASE + 0x01F0)

#define W4_RET_ENC_PIC_IDX                    (W4_REG_BASE + 0x01A8)
#define W4_RET_ENC_PIC_SLICE_NUM              (W4_REG_BASE + 0x01AC)
#define W4_RET_ENC_PIC_SKIP                   (W4_REG_BASE + 0x01B0)
#define W4_RET_ENC_PIC_NUM_INTRA              (W4_REG_BASE + 0x01B4)
#define W4_RET_ENC_PIC_NUM_MERGE              (W4_REG_BASE + 0x01B8)
#define W4_RET_ENC_PIC_FLAG                   (W4_REG_BASE + 0x01BC)
#define W4_RET_ENC_PIC_NUM_SKIP               (W4_REG_BASE + 0x01C0)
#define W4_RET_ENC_PIC_AVG_CU_QP              (W4_REG_BASE + 0x01C4)
#define W4_RET_ENC_PIC_BYTE                   (W4_REG_BASE + 0x01C8)
#define W4_RET_ENC_GOP_PIC_IDX                (W4_REG_BASE + 0x01CC)
#define W4_RET_ENC_PIC_POC                    (W4_REG_BASE + 0x01D0)
#define W4_RET_ENC_USED_SRC_IDX               (W4_REG_BASE + 0x01D8)
#define W4_RET_ENC_PIC_NUM                    (W4_REG_BASE + 0x01DC)
#define W4_RET_ENC_PIC_TYPE                   (W4_REG_BASE + 0x01E0)
#define W4_RET_ENC_VCL_NUT                    (W4_REG_BASE + 0x01E4)

/************************************************************************/
/* ENCODER - SET_FRAMEBUF                                               */
/************************************************************************/
#define W4_CMD_FBC_STRIDE_Y                   (W4_REG_BASE + 0x150)
#define W4_CMD_FBC_STRIDE_C                   (W4_REG_BASE + 0x154)
/* 1/4 sub-sampled buffer (for S2 ME) 
 *      SUB_SAMPLED_ONE_FB_SIZE = ALIGN16(width/4) * ALIGN8(height/4)                      
 *      total size for sub-sampled buffer = SUB_SAMPLED_ONE_FB_SIZE * SET_FB_NUM                 
 */
#define W4_ADDR_SUB_SAMPLED_FB_BASE           (W4_REG_BASE + 0x0158)
#define W4_SUB_SAMPLED_ONE_FB_SIZE            (W4_REG_BASE + 0x015C)
/* 0x160 ~ 0x17F : same as the WAVE410 registers for SET_FRAMEBUF */


#endif /* __WAVE410_REGISTER_DEFINE_H__ */

