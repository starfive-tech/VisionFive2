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

#ifndef __CODA7Q_REGISTER_DEFINE_H__
#define __CODA7Q_REGISTER_DEFINE_H__

#define C7_REG_BASE                     0x00000000
#define C7_CMD_REG_BASE                 0x00000100
#define C7_CMD_REG_END                  0x00000200

//------------------------------------------------------------------------------
// [DEC SEQ INIT : DEC_PIC_HDR] COMMAND for CODA7Q
//------------------------------------------------------------------------------
#define C7_CMD_DEC_SEQ_USER_DATA_OPTION         (C7_REG_BASE + 0x174)

#define C7_CMD_DEC_USER_DATA_MASK               (C7_REG_BASE + 0x174)
#define C7_RET_DEC_USER_FLAG                    (C7_REG_BASE + 0x174)
#define C7_CMD_DEC_SEVERITY_LEVEL               (C7_REG_BASE + 0x178)
#define C7_CMD_DEC_SEQ_OPTION                   (C7_REG_BASE + 0x178)
#define C7_CMD_DEC_VCORE_LIMIT                  (C7_REG_BASE + 0x17C)
#define C7_RET_DEC_NUM_REORDER_DELAY            (C7_REG_BASE + 0x188)
#define C7_CMD_DEC_FORCE_FB_LATENCY_PLUS1       (C7_REG_BASE + 0x188)

#define C7_CMD_DEC_TEMPORAL_ID                  (C7_REG_BASE + 0x18C)
#define C7_CMD_SEQ_CHANGE_ENABLE_FLAG           (C7_REG_BASE + 0x194)
#define C7_RET_DEC_SEQ_PARAM                    (C7_REG_BASE + 0x198)
#define C7_RET_DEC_SEQ_HEADER_REPORT            (C7_REG_BASE + 0x198)
#define C7_RET_DEC_SEQ_VUI_INFO                 (C7_REG_BASE + 0x19C)
#define C7_RET_DEC_COLOR_SAMPLE_INFO            (C7_REG_BASE + 0x1A0)
#define C7_RET_DEC_ASPECT_RATIO                 (C7_REG_BASE + 0x1A4)
#define C7_RET_DEC_BIT_RATE                     (C7_REG_BASE + 0x1A8)
#define C7_RET_DEC_FRAME_RATE_NR                (C7_REG_BASE + 0x1AC)
#define C7_RET_DEC_FRAME_RATE_DR                (C7_REG_BASE + 0x1B0)

#define C7_RET_DEC_CROP_TOP_BOTTOM              (C7_REG_BASE + 0x1B4)
#define C7_RET_DEC_CROP_LEFT_RIGHT              (C7_REG_BASE + 0x1B8)

#define C7_CMD_DEC_SEQ_SRC_SIZE                 (C7_REG_BASE + 0x1BC)
#define C7_RET_DEC_PIC_SIZE                     (C7_REG_BASE + 0x1BC)
#define C7_RET_DEC_FRAMEBUF_NEEDED              (C7_REG_BASE + 0x1C0)
#define C7_FRAME_COUNT                          (C7_REG_BASE + 0x1D4)

#define C7_RET_DEC_SEQ_INFO                     (C7_REG_BASE + 0x1DC)
#define C7_RET_DEC_SEQ_VP8_SCALE_INFO           (C7_REG_BASE + 0x1DC)

#define C7_CMD_DEC_SEQ_MP4_ASP_CLASS            (C7_REG_BASE + 0x1E0)
#define C7_CMD_DEC_SEQ_X264_MV_EN               (C7_REG_BASE + 0x1E0)
#define C7_CMD_DEC_SEQ_VC1_STREAM_FMT           (C7_REG_BASE + 0x1E0)

//------------------------------------------------------------------------------
// [DEC PIC] COMMAND for CODA7Q
//------------------------------------------------------------------------------
#define C7_RET_DEC_PIC_HRD_INFO                 (C7_REG_BASE + 0x17C)
#define C7_RET_DEC_PIC_AVC_FPA_SEI0             (C7_REG_BASE + 0x180)
#define C7_RET_DEC_PIC_AVC_FPA_SEI1             (C7_REG_BASE + 0x184)
#define C7_RET_DEC_PIC_AVC_FPA_SEI2             (C7_REG_BASE + 0x18C)
#define C7_BIT_FRM_DIS_FLG                      (C7_REG_BASE + 0x190)
#define C7_RET_DEC_PIC_VUI_INFO                 (C7_REG_BASE + 0x19C)
#define C7_RET_DEC_PIC_VUI_PIC_STRUCT           (C7_REG_BASE + 0x1A0) 
#define C7_RET_DEC_PIC_ASPECT                   (C7_REG_BASE + 0x1A4)

#define C7_RET_DEC_PIC_FRAME_NR                 (C7_REG_BASE + 0x1AC)
#define C7_RET_DEC_PIC_FRAME_DR                 (C7_REG_BASE + 0x1B0)
#define C7_RET_DEC_PIC_RV_TR                    (C7_REG_BASE + 0x1AC)
#define C7_RET_DEC_PIC_RV_TR_BFRAME             (C7_REG_BASE + 0x1B0)
#define C7_RET_DEC_SEQ_CHANGE_FLAG              (C7_REG_BASE + 0x1C4)
#define C7_RET_DEC_PIC_VP8_PIC_REPORT           (C7_REG_BASE + 0x1C8)
#define C7_RET_DEC_PIC_POST                     (C7_REG_BASE + 0x1D0)
#define C7_RET_DEC_PIC_MVC_REPORT               (C7_REG_BASE + 0x1D0)
#define C7_RET_DEC_WARN_INFO                    (C7_REG_BASE + 0x1D8)
#define C7_RET_DEC_PIC_VP8_SCALE_INFO           (C7_REG_BASE + 0x1DC)
#define C7_RET_DEC_PRESCAN_INDEX                (C7_REG_BASE + 0x1DC)
#define C7_RET_DEC_PIC_DECODED_IDX              (C7_REG_BASE + 0x1E0)
#define C7_RET_DEC_PIC_DISPLAY_IDX              (C7_REG_BASE + 0x1E4)
#define C7_RET_DEC_PIC_TYPE                     (C7_REG_BASE + 0x1E8)
#define C7_RET_DEC_PIC_POC                      (C7_REG_BASE + 0x1EC)

#define C7_RET_DEC_PIC_ERR_MB                   (C7_REG_BASE + 0x1F0)
#define C7_RET_DEC_ERR_INFO                     (C7_REG_BASE + 0x1F4)

#define C7_RET_DEC_ERR_CTB_NUM                  (C7_REG_BASE + 0x1F0)
#define C7_RET_DEC_ERR_INFO                     (C7_REG_BASE + 0x1F4)
#define C7_RET_DEC_AU_START_POS                 (C7_REG_BASE + 0x1F8)
#define C7_RET_DEC_AU_END_POS                   (C7_REG_BASE + 0x1FC)
#endif /* __CODA7Q_REGISTER_DEFINE_H__ */

//------------------------------------------------------------------------------
// [ENC SEQ INIT] COMMAND
//------------------------------------------------------------------------------
#define C7_CMD_ENC_SET_PARAM_OPTION            (C7_REG_BASE + 0x10C)
#define C7_CMD_ENC_SEQ_SRC_SIZE                (C7_REG_BASE + 0x160)
#define C7_CMD_ENC_SEQ_PARAM                   (C7_REG_BASE + 0x16C)
#define C7_CMD_ENC_SEQ_GOP_NUM                 (C7_REG_BASE + 0x170)
#define C7_CMD_ENC_SEQ_INTRA_PARAM             (C7_REG_BASE + 0x174)

#define C7_CMD_ENC_SEQ_SRC_F_RATE              (C7_REG_BASE + 0x180)
#define C7_CMD_ENC_SEQ_SLICE_MODE              (C7_REG_BASE + 0x184)
#define C7_CMD_ENC_SEQ_OPTION                  (C7_REG_BASE + 0x188)            // HecEnable,ConstIntraQp, FMO, QPREP, AUD, SLICE, MB BIT
#define C7_CMD_ENC_PARAM                       (C7_REG_BASE + 0x190)
#define C7_CMD_ENC_SEQ_RC_PARAM                (C7_REG_BASE + 0x198)
#define C7_CMD_ENC_SEQ_RC_MAX_QP               (C7_REG_BASE + 0x19C) 
#define C7_CMD_ENC_RC_TARGET_RATE              (C7_REG_BASE + 0x1C0) 
#define C7_CMD_ENC_ROT_PARAM                   (C7_REG_BASE + 0x1C4)
#define C7_CMD_ENC_SEQ_STD_PARA                (C7_REG_BASE + 0x1DC)
#define C7_CMD_ENC_SEQ_AUX_PARA                (C7_REG_BASE + 0x1E0)

#define C7_CMD_ENC_SEARCH_BASE                 (C7_REG_BASE + 0x1E8)
#define C7_CMD_ENC_SEARCH_SIZE                 (C7_REG_BASE + 0x1EC)
#define C7_CMD_ENC_SEQ_RC_GAMMA                (C7_REG_BASE + 0x1F0)      
#define C7_CMD_ENC_SEQ_RC_INTERVAL_MODE        (C7_REG_BASE + 0x1F4)      // mbInterval[32:2], rcIntervalMode[1:0]
#define C7_CMD_ENC_SEQ_INTRA_WEIGHT            (C7_REG_BASE + 0x1F8)
#define C7_CMD_ENC_SEQ_RC_BUF_SIZE             (C7_REG_BASE + 0x1FC)

//------------------------------------------------------------------------------
// [ENC SEQ END] COMMAND
//------------------------------------------------------------------------------
#define C7_RET_ENC_AXI_ALLOC_BUF_FAIL           (C7_REG_BASE + 0x1C8)
#define C7_RET_ENC_MIN_FB_NUM                   (C7_REG_BASE + 0x1CC)
#define C7_RET_ENC_NAL_INFO_TO_BE_ENCODED       (C7_REG_BASE + 0x1D0)
#define C7_RET_ENC_MIN_SRC_BUF_NUM              (C7_REG_BASE + 0x1D8)

//------------------------------------------------------------------------------
// [ENC PIC] COMMAND for CODA7Q
//------------------------------------------------------------------------------
#define C7_CMD_ENC_PIC_OPTION                 (C7_REG_BASE + 0x10C)
#define C7_CMD_ENC_ADDR_REPORT_BASE           (C7_REG_BASE + 0x15C)
#define C7_CMD_ENC_REPORT_SIZE                (C7_REG_BASE + 0x160)
#define C7_CMD_ENC_REPORT_PARAM               (C7_REG_BASE + 0x164)
#define C7_CMD_ENC_CODE_OPTION                (C7_REG_BASE + 0x168)
#define C7_CMD_ENC_PIC_PARAM                  (C7_REG_BASE + 0x16C)
#define C7_CMD_ENC_SRC_ADDR_Y                 (C7_REG_BASE + 0x174)
#define C7_CMD_ENC_SRC_ADDR_U                 (C7_REG_BASE + 0x178)
#define C7_CMD_ENC_SRC_ADDR_V                 (C7_REG_BASE + 0x17C)
#define C7_CMD_ENC_SRC_STRIDE                 (C7_REG_BASE + 0x180)
#define C7_CMD_ENC_SRC_FORMAT                 (C7_REG_BASE + 0x184)
#define C7_CMD_ENC_CONF_WIN_TOP_BOT           (C7_REG_BASE + 0x188)
#define C7_CMD_ENC_CONF_WIN_LEFT_RIGHT        (C7_REG_BASE + 0x18C)
#define C7_CMD_ENC_PIC_QS                     (C7_REG_BASE + 0x198)

#define C7_RET_ENC_PIC_FRAME_IDX              (C7_REG_BASE + 0x1A0)     // reconstructed frame index
#define C7_RET_ENC_PIC_SLICE_NUM              (C7_REG_BASE + 0x1A4)
#define C7_RET_ENC_PIC_NUM                    (C7_REG_BASE + 0x1D8)
#define C7_RET_ENC_PIC_TYPE                   (C7_REG_BASE + 0x1DC)
#define C7_RET_ENC_PIC_FLAG                   (C7_REG_BASE + 0x1E0)

