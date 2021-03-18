/*
 * Copyright (c) 2019, Chips&Media
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VPUAPI_H_INCLUDED
#define VPUAPI_H_INCLUDED

#include "vpuconfig.h"
#include "vputypes.h"
#include "../vdi/vdi.h"
#include "../vdi/vdi_osal.h"
#include "vpuerror.h"



#define MAX_GDI_IDX                             31
#define MAX_REG_FRAME                           MAX_GDI_IDX*2 // 2 for WTL

#define WAVE5_ENC_FBC50_LUMA_TABLE_SIZE(_w, _h)     (VPU_ALIGN2048(VPU_ALIGN32(_w))*VPU_ALIGN4(_h)/64)
#define WAVE5_ENC_FBC50_CHROMA_TABLE_SIZE(_w, _h)   (VPU_ALIGN2048(VPU_ALIGN32(_w)/2)*VPU_ALIGN4(_h)/128)

#define WAVE5_ENC_FBC50_LOSSLESS_LUMA_10BIT_FRAME_SIZE(_w, _h)    ((VPU_ALIGN32(_w)+127)/128*VPU_ALIGN4(_h)*160)
#define WAVE5_ENC_FBC50_LOSSLESS_LUMA_8BIT_FRAME_SIZE(_w, _h)     ((VPU_ALIGN32(_w)+127)/128*VPU_ALIGN4(_h)*128)
#define WAVE5_ENC_FBC50_LOSSLESS_CHROMA_10BIT_FRAME_SIZE(_w, _h)  ((VPU_ALIGN32(_w)/2+127)/128*VPU_ALIGN4(_h)/2*160)
#define WAVE5_ENC_FBC50_LOSSLESS_CHROMA_8BIT_FRAME_SIZE(_w, _h)   ((VPU_ALIGN32(_w)/2+127)/128*VPU_ALIGN4(_h)/2*128)
#define WAVE5_ENC_FBC50_LOSSY_LUMA_FRAME_SIZE(_w, _h, _tx)        ((VPU_ALIGN32(_w)+127)/128*VPU_ALIGN4(_h)*VPU_ALIGN32(_tx))
#define WAVE5_ENC_FBC50_LOSSY_CHROMA_FRAME_SIZE(_w, _h, _tx)      ((VPU_ALIGN32(_w)/2+127)/128*VPU_ALIGN4(_h)/2*VPU_ALIGN32(_tx))

/* YUV422 cframe */
#define WAVE5_ENC_FBC50_422_CHROMA_TABLE_SIZE(_w, _h)   (VPU_ALIGN2048(VPU_ALIGN32(_w)/2)*VPU_ALIGN4(_h)/128*2)
#define WAVE5_ENC_FBC50_LOSSLESS_422_CHROMA_10BIT_FRAME_SIZE(_w, _h)  ((VPU_ALIGN32(_w)/2+127)/128*VPU_ALIGN4(_h)*160)
#define WAVE5_ENC_FBC50_LOSSLESS_422_CHROMA_8BIT_FRAME_SIZE(_w, _h)   ((VPU_ALIGN32(_w)/2+127)/128*VPU_ALIGN4(_h)*128)
#define WAVE5_ENC_FBC50_LOSSY_422_CHROMA_FRAME_SIZE(_w, _h, _tx)      ((VPU_ALIGN32(_w)/2+127)/128*VPU_ALIGN4(_h)*VPU_ALIGN32(_tx))

#define WAVE5_DEC_HEVC_MVCOL_BUF_SIZE(_w, _h)       (((_w+63)/64)*((_h+63)/64)*256+ 64)
#define WAVE5_DEC_AVC_MVCOL_BUF_SIZE(_w, _h)        ((((VPU_ALIGN256(_w)/16)*(VPU_ALIGN16(_h)/16)) + 16)*80)
#define WAVE5_DEC_VP9_MVCOL_BUF_SIZE(_w, _h)        (((VPU_ALIGN64(_w)*VPU_ALIGN64(_h))>>2))
#define WAVE5_DEC_AVS2_MVCOL_BUF_SIZE(_w, _h)       (((VPU_ALIGN64(_w)*VPU_ALIGN64(_h))>>5))    // 1 CTU16 = 8bytes, VPU_ALIGN64(w)/16*VPU_ALIGN64(h)/16 * 8 = VPU_ALIGN64(w)*VPU_ALIGN64(h)/32
// AV1 MVCOL BUF SIZE : MFMV + Segment ID + CDF probs table + Film grain param Y + Film graim param C
#define WAVE5_DEC_AV1_MVCOL_BUF_SIZE(_w, _h)        (((VPU_ALIGN64(_w)/64)*256 + (VPU_ALIGN256(_w)/64)*128)*(VPU_ALIGN64(_h)/64) + ((VPU_ALIGN64(_w)/64)*(VPU_ALIGN64(_h)/64)*96) + 41984 + 8192 + 4864)
#define WAVE5_DEC_SVAC_MVCOL_BUF_SIZE(_w, _h)       (((VPU_ALIGN128(_w)*VPU_ALIGN128(_h))>>2))

#define WAVE5_FBC_LUMA_TABLE_SIZE(_w, _h)           (VPU_ALIGN64(_h)*VPU_ALIGN256(_w)/32)
#define WAVE5_FBC_CHROMA_TABLE_SIZE(_w, _h)         (VPU_ALIGN64(_h)*VPU_ALIGN256(_w/2)/32)
#define WAVE5_ENC_AVC_MVCOL_BUF_SIZE(_w, _h)        (VPU_ALIGN64(_w)*VPU_ALIGN64(_h)/32)
#define WAVE5_ENC_HEVC_MVCOL_BUF_SIZE(_w, _h)       (VPU_ALIGN64(_w)/64*VPU_ALIGN64(_h)/64*128)
#define WAVE5_ENC_SVAC_MVCOL_0_BUF_SIZE(_w, _h)     (VPU_ALIGN64(_w)/64*VPU_ALIGN64(_h)/64*128)   // same as hevc enc.
#define WAVE5_ENC_SVAC_MVCOL_1_BUF_SIZE(_w, _h)     (VPU_ALIGN64(_w)/64*VPU_ALIGN64(_h)/64*512)   // 2nd col-mv buffer (fixed size)

//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------
/**
* @brief
@verbatim
This is an enumeration for declaring codec standard type variables. Currently,
VPU supports many different video standards such as H.265/HEVC, MPEG4 SP/ASP, H.263 Profile 3, H.264/AVC
BP/MP/HP, VC1 SP/MP/AP, Divx3, MPEG1, MPEG2, RealVideo 8/9/10, AVS Jizhun/Guangdian profile, AVS2,
 Theora, VP3, VP8/VP9 and SVAC.

NOTE: MPEG-1 decoder operation is handled as a special case of MPEG2 decoder.
STD_THO must be always 9.
@endverbatim
*/
typedef enum {
    STD_AVC,
    STD_VC1,
    STD_MPEG2,
    STD_MPEG4,
    STD_H263,
    STD_DIV3,
    STD_RV,
    STD_AVS,
    STD_THO = 9,
    STD_VP3,
    STD_VP8,
    STD_HEVC,
    STD_VP9,
    STD_AVS2,
    STD_SVAC,
    STD_AV1,
    STD_MAX
} CodStd;

/**
* @brief
@verbatim
This is an enumeration for declaring SET_PARAM command options.
Depending on this, SET_PARAM command parameter registers have different settings.

NOTE: This is only for WAVE encoder IP.

@endverbatim
*/
typedef enum {
    OPT_COMMON          = 0, /**< SET_PARAM command option for encoding sequence */
    OPT_CUSTOM_GOP      = 1, /**< SET_PARAM command option for setting custom GOP */
    OPT_CUSTOM_HEADER   = 2, /**< SET_PARAM command option for setting custom VPS/SPS/PPS */
    OPT_VUI             = 3, /**< SET_PARAM command option for encoding VUI  */
    OPT_CHANGE_PARAM    = 0x10, /**< SET_PARAM command option for parameters change (WAVE Encoder only)  */
#ifdef SUPPORT_LOOK_AHEAD_RC
    OPT_LOOKAHEAD_PARAM_1 = 0x100, /**< SET_PARAM command option for Look Ahead RC parameters setting  */
    OPT_LOOKAHEAD_PARAM_2 = 0x101, /**< SET_PARAM command option for Look Ahead RC parameters setting  */
    OPT_LOOKAHEAD_PARAM_3 = 0x102, /**< SET_PARAM command option for Look Ahead RC parameters setting  */
    OPT_LOOKAHEAD_PARAM_4 = 0x103, /**< SET_PARAM command option for Look Ahead RC parameters setting  */
    OPT_LOOKAHEAD_PARAM_5 = 0x104, /**< SET_PARAM command option for Look Ahead RC parameters setting  */
#endif
} SET_PARAM_OPTION;

/**
* @brief
@verbatim
This is an enumeration for declaring the operation mode of DEC_PIC_HDR command. (WAVE decoder only)

@endverbatim
*/
typedef enum {
    INIT_SEQ_NORMAL     = 0x01,  /**< It initializes some parameters (i.e. buffer mode) required for decoding sequence, performs sequence header, and returns information on the sequence. */
    INIT_SEQ_W_THUMBNAIL = 0x11, /**< It decodes only the first I picture of sequence to get thumbnail. */
} DEC_PIC_HDR_OPTION;

/**
* @brief
@verbatim
This is an enumeration for declaring the running option of DEC_PIC command. (WAVE decoder only)

@endverbatim
*/
typedef enum {
    DEC_PIC_NORMAL      = 0x00, /**< It is normal mode of DEC_PIC command. */
    DEC_PIC_W_THUMBNAIL = 0x10, /**< It handles CRA picture as BLA picture not to use reference from the previously decoded pictures. */
    SKIP_NON_IRAP       = 0x11, /**< It is thumbnail mode (skip non-IRAP without reference reg.) */
    SKIP_NON_RECOVERY   = 0x12, /**< It skips to decode non-IRAP pictures. */
    SKIP_NON_REF_PIC    = 0x13, /**< It skips to decode non-reference pictures which correspond to sub-layer non-reference picture with MAX_DEC_TEMP_ID. (The sub-layer non-reference picture is the one whose nal_unit_type equal to TRAIL_N, TSA_N, STSA_N, RADL_N, RASL_N, RSV_VCL_N10, RSV_VCL_N12, or RSV_VCL_N14. )*/
    SKIP_TEMPORAL_LAYER = 0x14, /**< It decodes only frames whose temporal id is equal to or less than MAX_DEC_TEMP_ID. */
    SKIP_SVAC_BL        = 0x20, /**< It skips base layer pictures. */
    SKIP_SVAC_EL        = 0x40  /**< It skips enhance layer pictures. */
} DEC_PIC_OPTION;

/**
* @brief
@verbatim
This is an enumeration for declaring options of getting a write pointer of bitstream buffer. (WAVE encoder only)

@endverbatim
*/
typedef enum {
    GET_ENC_PIC_DONE_WRPTR      = 0x00, /**< It reads the write pointer of bitstream buffer after picture encoding is done. */
    GET_ENC_BSBUF_FULL_WRPTR    = 0x01, /**< It reads the write pointer of bitstream buffer when buffer full is occurred. */
    GET_ENC_LOW_LATENCY_WRPTR   = 0x02, /**< It reads the write pointer of bitstream buffer when low latency encoding is done.  */
} ENC_QUERY_WRPTR_SEL;
/************************************************************************/
/* Limitations                                                          */
/************************************************************************/
#define MAX_FRAMEBUFFER_COUNT               31  /* The 32nd framebuffer is reserved for WTL */

/************************************************************************/
/* PROFILE & LEVEL                                                      */
/************************************************************************/
/* HEVC */
#define HEVC_PROFILE_MAIN                   1
#define HEVC_PROFILE_MAIN10                 2
#define HEVC_PROFILE_STILLPICTURE           3
#define HEVC_PROFILE_MAIN10_STILLPICTURE    2

/* VP9 */
#define VP9_PROFILE_0                       0
#define VP9_PROFILE_1                       1
#define VP9_PROFILE_2                       2
#define VP9_PROFILE_3                       3

/* AVS2 */
#define AVS2_PROFILE_MAIN_PICTURE           0x12
#define AVS2_PROFILE_MAIN                   0x20
#define AVS2_PROFILE_MAIN10                 0x22

/* Tier */
#define HEVC_TIER_MAIN                      0
#define HEVC_TIER_HIGH                      1
/* Level */
#define HEVC_LEVEL(_Major, _Minor)          (_Major*10+_Minor)

/* H.264 */
#define H264_PROFILE_BP                     66
#define H264_PROFILE_MP                     77
#define H264_PROFILE_EXTENDED               88
#define H264_PROFILE_HP                     100
#define H264_PROFILE_HIGH10                 110
#define H264_PROFILE_HIGH10_INTRA           120
#define H264_PROFILE_HIGH422                122
#define H264_PROFILE_HIGH444                244
#define H264_PROFILE_CAVLC_444_INTRA        44

/* SAVC */
#define SVAC_PROFILE_BASE                   0x11
#define SVAC_PROFILE_ADVANCE                0x33

/* H265 USER_DATA(SPS & SEI) ENABLE FLAG */
#define H265_USERDATA_FLAG_RESERVED_0                       (0)
#define H265_USERDATA_FLAG_RESERVED_1                       (1)
#define H265_USERDATA_FLAG_VUI                              (2)
#define H265_USERDATA_FLAG_RESERVED_3                       (3)
#define H265_USERDATA_FLAG_PIC_TIMING                       (4)
#define H265_USERDATA_FLAG_ITU_T_T35_PRE                    (5)  /* SEI Prefix: user_data_registered_itu_t_t35 */
#define H265_USERDATA_FLAG_UNREGISTERED_PRE                 (6)  /* SEI Prefix: user_data_unregistered */
#define H265_USERDATA_FLAG_ITU_T_T35_SUF                    (7)  /* SEI Suffix: user_data_registered_itu_t_t35 */
#define H265_USERDATA_FLAG_UNREGISTERED_SUF                 (8)  /* SEI Suffix: user_data_unregistered */
#define H265_USERDATA_FLAG_RECOVERY_POINT                   (9)  /* SEI RESERVED */
#define H265_USERDATA_FLAG_MASTERING_COLOR_VOL              (10) /* SEI Prefix: mastering_display_color_volume */
#define H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT    (11) /* SEI Prefix: chroma_resampling_filter_hint */
#define H265_USERDATA_FLAG_KNEE_FUNCTION_INFO               (12) /* SEI Prefix: knee_function_info */
#define H265_USERDATA_FLAG_TONE_MAPPING_INFO                (13) /* SEI Prefix: tone_mapping_info */
#define H265_USERDATA_FLAG_FILM_GRAIN_CHARACTERISTICS_INFO  (14) /* SEI Prefix: film_grain_characteristics_info */
#define H265_USERDATA_FLAG_CONTENT_LIGHT_LEVEL_INFO         (15) /* SEI Prefix: content_light_level_info */
#define H265_USERDATA_FLAG_COLOUR_REMAPPING_INFO            (16) /* SEI Prefix: content_light_level_info */
#define H265_USERDATA_FLAG_ITU_T_T35_PRE_1                  (28) /* SEI Prefix: additional user_data_registered_itu_t_t35 */
#define H265_USERDATA_FLAG_ITU_T_T35_PRE_2                  (29) /* SEI Prefix: additional user_data_registered_itu_t_t35 */
#define H265_USERDATA_FLAG_ITU_T_T35_SUF_1                  (30) /* SEI Suffix: additional user_data_registered_itu_t_t35 */
#define H265_USERDATA_FLAG_ITU_T_T35_SUF_2                  (31) /* SEI Suffix: additional user_data_registered_itu_t_t35 */


/* H264 USER_DATA(SPS & SEI) ENABLE FLAG */
#define H264_USERDATA_FLAG_RESERVED_0                       (0)  /* SEI RESERVED */
#define H264_USERDATA_FLAG_RESERVED_1                       (1)  /* SEI RESERVED */
#define H264_USERDATA_FLAG_VUI                              (2)  /* SEI RESERVED */
#define H264_USERDATA_FLAG_RESERVED_3                       (3)  /* SEI RESERVED */
#define H264_USERDATA_FLAG_PIC_TIMING                       (4)  /* SEI : pic_timing */
#define H264_USERDATA_FLAG_ITU_T_T35                        (5)  /* SEI : user_data_registered_itu_t_t35 */
#define H264_USERDATA_FLAG_UNREGISTERED                     (6)  /* SEI Prefix: user_data_unregistered */
#define H264_USERDATA_FLAG_RESERVED_7                       (7)  /* SEI RESERVED */
#define H264_USERDATA_FLAG_RESERVED_8                       (8)  /* SEI RESERVED */
#define H264_USERDATA_FLAG_RESERVED_9                       (9)  /* SEI RESERVED */
#define H264_USERDATA_FLAG_RESERVED_10                      (10) /* SEI : mastering_display_color_volume */
#define H264_USERDATA_FLAG_RESERVED_11                      (11) /* SEI RESERVED */
#define H264_USERDATA_FLAG_RESERVED_12                      (12) /* SEI RESERVED */
#define H264_USERDATA_FLAG_TONE_MAPPING_INFO                (13) /* SEI : tone_mapping_info */
#define H264_USERDATA_FLAG_FILM_GRAIN_CHARACTERISTICS_INFO  (14) /* SEI : film_grain_characteristics_info */
#define H264_USERDATA_FLAG_RESERVED_15                      (15) /* SEI RESERVED */
#define H264_USERDATA_FLAG_COLOUR_REMAPPING_INFO            (16) /* SEI : colour_remapping_info */
#define H264_USERDATA_FLAG_ITU_T_T35_1                      (28) /* SEI : user_data_registered_itu_t_t35_1 */
#define H264_USERDATA_FLAG_ITU_T_T35_2                      (29) /* SEI : user_data_registered_itu_t_t35_2 */

/************************************************************************/
/* Error codes                                                          */
/************************************************************************/

/**
* @brief    This is an enumeration for declaring return codes from API function calls.
The meaning of each return code is the same for all of the API functions, but the reasons of
non-successful return might be different. Some details of those reasons are
briefly described in the API definition chapter. In this chapter, the basic meaning
of each return code is presented.
*/
typedef enum {
    RETCODE_SUCCESS,                    /**< This means that operation was done successfully.  */  /* 0  */
    RETCODE_FAILURE,                    /**< This means that operation was not done successfully. When un-recoverable decoder error happens such as header parsing errors, this value is returned from VPU API.  */
    RETCODE_INVALID_HANDLE,             /**< This means that the given handle for the current API function call was invalid (for example, not initialized yet, improper function call for the given handle, etc.).  */
    RETCODE_INVALID_PARAM,              /**< This means that the given argument parameters (for example, input data structure) was invalid (not initialized yet or not valid anymore). */
    RETCODE_INVALID_COMMAND,            /**< This means that the given command was invalid (for example, undefined, or not allowed in the given instances).  */
    RETCODE_ROTATOR_OUTPUT_NOT_SET,     /**< This means that rotator output buffer was not allocated even though postprocessor (rotation, mirroring, or deringing) is enabled. */  /* 5  */
    RETCODE_ROTATOR_STRIDE_NOT_SET,     /**< This means that rotator stride was not provided even though postprocessor (rotation, mirroring, or deringing) is enabled.  */
    RETCODE_FRAME_NOT_COMPLETE,         /**< This means that frame decoding operation was not completed yet, so the given API function call cannot be allowed.  */
    RETCODE_INVALID_FRAME_BUFFER,       /**< This means that the given source frame buffer pointers were invalid in encoder (not initialized yet or not valid anymore).  */
    RETCODE_INSUFFICIENT_FRAME_BUFFERS, /**< This means that the given numbers of frame buffers were not enough for the operations of the given handle. This return code is only received when calling VPU_DecRegisterFrameBuffer() or VPU_EncRegisterFrameBuffer() function. */
    RETCODE_INVALID_STRIDE,             /**< This means that the given stride was invalid (for example, 0, not a multiple of 8 or smaller than picture size). This return code is only allowed in API functions which set stride.  */   /* 10 */
    RETCODE_WRONG_CALL_SEQUENCE,        /**< This means that the current API function call was invalid considering the allowed sequences between API functions (for example, missing one crucial function call before this function call).  */
    RETCODE_CALLED_BEFORE,              /**< This means that multiple calls of the current API function for a given instance are invalid. */
    RETCODE_NOT_INITIALIZED,            /**< This means that VPU was not initialized yet. Before calling any API functions, the initialization API function, VPU_Init(), should be called at the beginning.  */
    RETCODE_USERDATA_BUF_NOT_SET,       /**< This means that there is no memory allocation for reporting userdata. Before setting user data enable, user data buffer address and size should be set with valid value. */
    RETCODE_MEMORY_ACCESS_VIOLATION,    /**< This means that access violation to the protected memory has been occurred. */   /* 15 */
    RETCODE_VPU_RESPONSE_TIMEOUT,       /**< This means that VPU response time is too long, time out. */
    RETCODE_INSUFFICIENT_RESOURCE,      /**< This means that VPU cannot allocate memory due to lack of memory. */
    RETCODE_NOT_FOUND_BITCODE_PATH,     /**< This means that BIT_CODE_FILE_PATH has a wrong firmware path or firmware size is 0 when calling VPU_InitWithBitcode() function.  */
    RETCODE_NOT_SUPPORTED_FEATURE,      /**< This means that HOST application uses an API option that is not supported in current hardware.  */
    RETCODE_NOT_FOUND_VPU_DEVICE,       /**< This means that HOST application uses the undefined product ID. */   /* 20 */
    RETCODE_CP0_EXCEPTION,              /**< This means that coprocessor exception has occurred. (WAVE only) */
    RETCODE_STREAM_BUF_FULL,            /**< This means that stream buffer is full in encoder. */
    RETCODE_ACCESS_VIOLATION_HW,        /**< This means that GDI access error has occurred. It might come from violation of write protection region or spec-out GDI read/write request. (WAVE only) */
    RETCODE_QUERY_FAILURE,              /**< This means that query command was not successful. (WAVE5 only) */
    RETCODE_QUEUEING_FAILURE,           /**< This means that commands cannot be queued. (WAVE5 only) */
    RETCODE_VPU_STILL_RUNNING,          /**< This means that VPU cannot be flushed or closed now, because VPU is running. (WAVE5 only) */
    RETCODE_REPORT_NOT_READY,           /**< This means that report is not ready for Query(GET_RESULT) command. (WAVE5 only) */
    RETCODE_VLC_BUF_FULL,               /**< This means that VLC buffer is full in encoder. (WAVE5 only) */
    RETCODE_INVALID_SFS_INSTANCE,       /**< This means that current instance can't run sub-framesync. (already an instance was running with sub-frame sync (WAVE5 only) */
    RETCODE_ERROR_FW_FATAL,             /**< This means that firmware detects a fatal error. (WAVE5 only) */
} RetCode;

/************************************************************************/
/* Utility macros                                                       */
/************************************************************************/
#define VPU_CEIL(_data, _align)     (((_data)+(_align-1))&~(_align-1))
#define VPU_ALIGN4(_x)              (((_x)+0x03)&~0x03)
#define VPU_ALIGN8(_x)              (((_x)+0x07)&~0x07)
#define VPU_ALIGN16(_x)             (((_x)+0x0f)&~0x0f)
#define VPU_ALIGN32(_x)             (((_x)+0x1f)&~0x1f)
#define VPU_ALIGN64(_x)             (((_x)+0x3f)&~0x3f)
#define VPU_ALIGN128(_x)            (((_x)+0x7f)&~0x7f)
#define VPU_ALIGN256(_x)            (((_x)+0xff)&~0xff)
#define VPU_ALIGN512(_x)            (((_x)+0x1ff)&~0x1ff)
#define VPU_ALIGN2048(_x)           (((_x)+0x7ff)&~0x7ff)
#define VPU_ALIGN4096(_x)           (((_x)+0xfff)&~0xfff)
#define VPU_ALIGN16384(_x)          (((_x)+0x3fff)&~0x3fff)

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define INTERRUPT_TIMEOUT_VALUE     (-1)

/**
 * \brief   parameters of DEC_SET_SEQ_CHANGE_MASK
 */
#define SEQ_CHANGE_ENABLE_PROFILE   (1<<5)
#define SEQ_CHANGE_ENABLE_SIZE      (1<<16)
#define SEQ_CHANGE_ENABLE_BITDEPTH  (1<<18)
#define SEQ_CHANGE_ENABLE_DPB_COUNT (1<<19)

#define SEQ_CHANGE_INTER_RES_CHANGE (1<<17)         /* VP9 */
#define SEQ_CHANGE_ENABLE_ALL_VP9      (SEQ_CHANGE_ENABLE_PROFILE    | \
                                        SEQ_CHANGE_ENABLE_SIZE       | \
                                        SEQ_CHANGE_INTER_RES_CHANGE | \
                                        SEQ_CHANGE_ENABLE_BITDEPTH   | \
                                        SEQ_CHANGE_ENABLE_DPB_COUNT)

#define SEQ_CHANGE_ENABLE_ALL_HEVC     (SEQ_CHANGE_ENABLE_PROFILE    | \
                                        SEQ_CHANGE_ENABLE_SIZE       | \
                                        SEQ_CHANGE_ENABLE_BITDEPTH   | \
                                        SEQ_CHANGE_ENABLE_DPB_COUNT)

#define SEQ_CHANGE_ENABLE_ALL_AVS2     (SEQ_CHANGE_ENABLE_PROFILE    | \
                                        SEQ_CHANGE_ENABLE_SIZE       | \
                                        SEQ_CHANGE_ENABLE_BITDEPTH   | \
                                        SEQ_CHANGE_ENABLE_DPB_COUNT)

#define SEQ_CHANGE_ENABLE_ALL_SVAC     (SEQ_CHANGE_ENABLE_PROFILE    | \
                                        SEQ_CHANGE_ENABLE_SIZE       | \
                                        SEQ_CHANGE_ENABLE_BITDEPTH   | \
                                        SEQ_CHANGE_ENABLE_DPB_COUNT)


#define SEQ_CHANGE_ENABLE_ALL_AVC      (SEQ_CHANGE_ENABLE_SIZE       | \
                                        SEQ_CHANGE_ENABLE_BITDEPTH   | \
                                        SEQ_CHANGE_ENABLE_DPB_COUNT)

#define SEQ_CHANGE_ENABLE_ALL_AV1      (SEQ_CHANGE_ENABLE_PROFILE    | \
                                        SEQ_CHANGE_ENABLE_SIZE       | \
                                        SEQ_CHANGE_ENABLE_BITDEPTH   | \
                                        SEQ_CHANGE_ENABLE_DPB_COUNT)

#define DISPLAY_IDX_FLAG_SEQ_END     -1
#define DISPLAY_IDX_FLAG_NO_FB       -3
#define DECODED_IDX_FLAG_NO_FB       -1
#define DECODED_IDX_FLAG_SKIP        -2

#define DECODED_IDX_FLAG_UNSUPPORT   -4     // avc decoder in WAVE5

#define RECON_IDX_FLAG_ENC_END       -1
#define RECON_IDX_FLAG_ENC_DELAY     -2
#define RECON_IDX_FLAG_HEADER_ONLY   -3
#define RECON_IDX_FLAG_CHANGE_PARAM  -4

#define MAX_ROI_NUMBER  10

/**
* @brief    This is a special enumeration type for some configuration commands
* which can be issued to VPU by HOST application. Most of these commands can be called occasionally,
* not periodically for changing the configuration of decoder or encoder operation running on VPU.
*
*/
typedef enum {
    ENABLE_ROTATION,  /**< This command enables rotation. In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS. */
    DISABLE_ROTATION, /**< This command disables rotation. In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS. */
    ENABLE_MIRRORING, /**< This command enables mirroring. In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS. */
    DISABLE_MIRRORING,/**< This command disables mirroring. In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS. */
/**
@verbatim
This command sets mirror direction of the post-rotator, and `parameter` is
interpreted as a pointer to MirrorDirection. The `parameter` should be one of
MIRDIR_NONE, MIRDIR_VER, MIRDIR_HOR, and MIRDIR_HOR_VER.

@* MIRDIR_NONE: No mirroring
@* MIRDIR_VER: Vertical mirroring
@* MIRDIR_HOR: Horizontal mirroring
@* MIRDIR_HOR_VER: Both directions

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given mirroring direction is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given mirroring
direction is invalid.

@endverbatim
*/
    SET_MIRROR_DIRECTION,
/**
@verbatim
This command sets counter-clockwise angle for post-rotation, and `parameter` is
interpreted as a pointer to the integer which represents rotation angle in
degrees. Rotation angle should be one of 0, 90, 180, and 270.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given rotation angle is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given rotation
angle is invalid.
@endverbatim
*/
    SET_ROTATION_ANGLE,
/**
@verbatim
This command sets rotator output buffer address. (CODA decoder only) The `parameter` is interpreted as
the pointer of a structure representing physical addresses of YCbCr components
of output frame. For storing the rotated output for display, at least one more
frame buffer should be allocated. When multiple display buffers are required,
HOST application could change the buffer pointer of rotated output at every frame by
issuing this command.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given rotation angle is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given frame
buffer pointer is invalid.
@endverbatim
*/
    SET_ROTATOR_OUTPUT,
/**
@verbatim
This command sets the stride size of the frame buffer containing rotated output. (CODA decoder only)
The `parameter` is interpreted as the value of stride of the rotated output.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given rotation angle is valid.
* RETCODE_INVALID_STRIDE:
The given argument parameter, `parameter`, was invalid, which means given value of
stride is invalid. The value of stride must be greater than 0 and a multiple of 8.
@endverbatim
*/
    SET_ROTATOR_STRIDE,
    DEC_GET_SEQ_INFO, /**< This command returns the information of the current sequence with <<vpuapi_h_DecInitialInfo>>. This command is mainly used for getting new sequence information after change of sequence.  */
/**
@verbatim
This command applies SPS stream received from a certain
out-of-band reception scheme to the decoder.
The stream should be in RBSP format and in big Endian.
The argument `parameter` is interpreted as a pointer to DecParamSet structure.
In this case, `paraSet` is an array of 32 bits which contains SPS RBSP, and `size`
is the size of the stream in bytes.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means transferring an SPS RBSP to decoder
was done successfully.
* RETCODE_INVALID_COMMAND:
The given argument `cmd` was invalid, which means the given `cmd` was undefined,
or not allowed in the current instance. In this case, current instance might not
be an H.264/AVC decoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
@endverbatim
*/
    DEC_SET_SPS_RBSP,
/**
@verbatim
This command applies PPS stream received from a certain
out-of-band reception scheme to the decoder. The stream should be in RBSP format and in
big Endian. The argument `parameter` is interpreted as a pointer to a DecParamSet structure.
In this case, paraSet is an array of 32 bits which contains PPS RBSP, and `size`
is the size of the stream in bytes.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means transferring a PPS RBSP to decoder
was done successfully.
* RETCODE_INVALID_COMMAND:
The given argument `cmd` was invalid, which means the given cmd was undefined,
or not allowed in the current instance. In this case, current instance might not
be an H.264/AVC decoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
@endverbatim
*/
    DEC_SET_PPS_RBSP,
/**
@verbatim
This command sets DEC_SET_SEQ_CHANGE_MASK which allows VPU to notify change of sequence information
such as picture size, DPB count, profile, and bit-depth.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means transferring a PPS RBSP to decoder
was done successfully.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.

@endverbatim
*/
    DEC_SET_SEQ_CHANGE_MASK,
    ENABLE_DERING, /**< This command enables deringing filter of the post-rotator. (CODA decoder only) In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS. */
    DISABLE_DERING, /**< This command disables deringing filter of the post-rotator. (CODA decoder only) In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS. */
/**
@verbatim
This command sets the secondary channel of AXI for saving memory bandwidth to
dedicated memory. The argument `parameter` is interpreted as a pointer to <<vpuapi_h_SecAxiUse>> which
represents an enable flag and physical address which is related with the secondary
channel.

This command has one of the following return codes::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value for setting secondary
AXI is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given value is
invalid.
@endverbatim
*/
    SET_SEC_AXI,
    SET_DRAM_CONFIG, /**< This command sets the DRAM attributes to use tiled map. The argument `parameter` is interpreted as a pointer to <<vpuapi_h_DRAMConfig>>. It returns RETCODE_INVALID_PARAM when any value is not given to the argument parameter, `parameter`. (CODA960 only) */   //coda960 only
    GET_DRAM_CONFIG, /**< This command gets the DRAM attributes to use tiled map. The argument `parameter` is interpreted as a pointer to <<vpuapi_h_DRAMConfig>>. It returns RETCODE_INVALID_PARAM when any value is not given to the argument parameter, `parameter`. (CODA960 only) */   //coda960 only
/**
@verbatim
This command enables user data report. This command ignores `parameter`.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means enabling user data report is done
successfully.
* RETCODE_USERDATA_BUF_NOT_SET:
This means user data buffer address and size have not set yet.
@endverbatim
*/
    ENABLE_REP_USERDATA,
    DISABLE_REP_USERDATA,/**< This command disables user data report. This command ignores `parameter` and returns RETCODE_SUCCESS. */
/**
@verbatim

This command sets user data buffer address. `parameter` is interpreted as a pointer
to address. This command returns as follows.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value of address is valid and
setting is done successfully.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` was invalid, which means given value of
address is invalid. The value of address must be greater than 0 and a multiple
of 8.

@endverbatim
*/
    SET_ADDR_REP_USERDATA,
/**
@verbatim
This command sets user data buffer address (virtual address) as well as physical address by using SET_ADDR_REP_USERDATA
`parameter` is interpreted as a pointer to address. This command returns as follows.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value of address is valid and setting is done successfully.
* RETCODE_USERDATA_BUF_NOT_SET:
SET_ADDR_REP_USERDATA command was not been executed
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` was invalid, which means given value of address is invalid.
    The value of address must be greater than 0 and a multiple of 8.

@endverbatim
*/
    SET_VIRT_ADDR_REP_USERDATA,
/**
@verbatim
This command sets the size of user data buffer which is set with
SET_ADDR_REP_USERDATA command. `parameter` is interpreted as a pointer to the value
of size. This command returns RETCODE_SUCCESS.

According to codec standards, user data type means as below.

@* H.264/AVC
@** 4 : user_data_registered_itu_t_t35
@** 5 : user_data_unregistered

More details are in Annex D of H.264 specifications.

@* VC1
@** 31 : Sequence Level user data
@** 30 : Entry-point Level user data
@** 29 : Frame Level user data
@** 28 : Field Level user data
@** 27 : Slice Level user data

@* MPEG2
@** 0 : Sequence user data
@** 1 : GOP user data
@** 2 : Picture user data

@* MPEG4
@** 0 : VOS user data
@** 1 : VIS user data
@** 2 : VOL user data
@** 3 : GOV user data

The user data size 0 - 15 is used to make offset from userDataBuf Base + 8x17.
It specifies byte size of user data 0 to 15 excluding 0 padding byte,
which exists between user data. So HOST reads 1 user data from
userDataBuf Base + 8x17 + 0 User Data Size + 0 Padding.
Size of 0 padding is (8 - (User Data Size % 8))%8.

@endverbatim
*/
    SET_SIZE_REP_USERDATA,
/**
@verbatim
This command sets the interrupt flag of user data buffer overflow. (CODA9 only)

@* 0 : interrupt mode
@* 1 : interrupt disable mode

@endverbatim
*/
    SET_USERDATA_REPORT_MODE,
/**
@verbatim
This command sets the configuration of cache. The `parameter` is interpreted as a pointer to MaverickCacheConfig. (CODA9 only)

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value is valid and setting is done successfully.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid. The value of address must be not zero.

@endverbatim
*/
    SET_CACHE_CONFIG,
/**
@verbatim
This command gets tiled map configuration according to `TiledMapConfig` structure. (CODA9 only)

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value is valid and setting is done successfully.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means it has a null pointer,
 or given values for some member variables are improper values.

@endverbatim
*/
    GET_TILEDMAP_CONFIG,
/**
@verbatim
This command sets the low delay decoding options which enable low delay decoding and indicate the number of MB row. (CODA9 decoder only)
The argument `parameter` is interpreted as a pointer to LowDelayInfo which represents an enable flag and the number of MB row.
If low delay decoding is enabled, VPU sends an interrupt and indexFrameDisplay to HOST when the number of MB row decoding is done.
If the interrupt is issued, HOST should clear the interrupt and read indexFrameDisplay from the RET_DEC_PIC_FRAME_IDX register in order to display.

@endverbatim
*/
    SET_LOW_DELAY_CONFIG,
/**
@verbatim
This command gets the low delay decoding options which enable low delay decoding and indicate the number of MB row. (CODA decoder only)
The argument `parameter` is interpreted as a pointer to LowDelayInfo which represents an enable flag and the number of MB row.
If low delay decoding is enabled, VPU sends an interrupt and indexFrameDisplay to HOST when the number of MB row decoding is done.
If the interrupt is issued, HOST should clear the interrupt and read indexFrameDisplay from the RET_DEC_PIC_FRAME_IDX register in order to display.

@endverbatim
*/
    GET_LOW_DELAY_OUTPUT,

/**
@verbatim
HOST can set the frameBufDelay value of <<vpuapi_h_DecInitialInfo>>. (CODA9 H.264/AVC decoder only)
This command is useful when HOST is sure of display reorder delay of stream and wants to display soonner than
frameBufDelay value of <<vpuapi_h_DecInitialInfo>> which is calculated based on video specification by VPU.
However, if HOST set an invalid frameBufDelay value, it might lead to failure of display.

@endverbatim
*/
    DEC_SET_FRAME_DELAY,
    DEC_SET_WTL_FRAME_FORMAT, /**< This command sets FrameBufferFormat for WTL. */
    DEC_GET_FIELD_PIC_TYPE, /**< This command gets a field picture type of decoded picture after INT_BIT_DEC_FIELD interrupt is issued. */
/**
@verbatim
HOST can get decoder output information according to display index in <<vpuapi_h_DecOutputInfo>> structure.
HOST can set display index using member variable `indexFrameDisplay`.
This command returns RETCODE_SUCCESS.

* Example code
.........................................................................
DecOutputInfo decOutputInfo;
decOutputInfo. indexFrameDisplay = disp_index;
VPU_DecGiveCommand(handle, DEC_GET_DISPLAY_OUTPUT_INFO, & decOutputInfo);
.........................................................................
@endverbatim
*/
    DEC_GET_DISPLAY_OUTPUT_INFO,
/**
@verbatim
HOST can enable display buffer reordering when decoding H.264 streams. (CODA9 H.264 decoder and WAVE decoder only)
In H.264 case, output decoded picture may be re-ordered if pic_order_cnt_type is 0 or 1. In that case, decoder
must delay output display for re-ordering but some applications (ex. video telephony) do not
want such display delay.
@endverbatim
*/
    DEC_ENABLE_REORDER,
/**
@verbatim
HOST can disable output display buffer reorder-ing. Then BIT processor does not re-order output buffer when pic_order_cnt_type is 0 or 1. If
In H.264/AVC case. pic_order_cnt_type is 2 or the other standard case, this flag is ignored because output display
buffer reordering is not allowed.
@endverbatim
*/
    DEC_DISABLE_REORDER,
/**
@verbatim
This command sets error conceal mode for H.264 decoder.
This command must be issued through VPU_DecGiveCommand() before calling VPU_DecGetInitialInfo() or VPU_DecIssueSeqInit().
In other words, error conceal mode cannot be applied once a sequence initialized.

@* AVC_ERROR_CONCEAL_MODE_DEFAULT - VPU performs error concealment in default mode.
@* AVC_ERROR_CONCEAL_MODE_ENABLE_SELECTIVE_CONCEAL_MISSING_REFERENCE -
VPU performs error concealment using another framebuffer if the error comes from missing reference frame.
@* AVC_ERROR_CONCEAL_MODE_DISABLE_CONCEAL_MISSING_REFERENCE -
VPU does not perform error concealment if the error comes from missing reference frame.
@* AVC_ERROR_CONCEAL_MODE_DISABLE_CONCEAL_WRONG_FRAME_NUM -
VPU does not perform error concealment if the error comes from wrong frame_num syntax.
@endverbatim
*/
    DEC_SET_AVC_ERROR_CONCEAL_MODE,
/**
@verbatim
HOST can free all the frame buffers allocated by VPUAPI. (CODA9 only)
This command is useful when VPU detects sequence change.
For example, if HOST knows resolution change while decoding through `sequenceChanged` variable of <<vpuapi_h_DecOutputInfo>> structure,
HOST should change the size of frame buffer accordingly.
This command is used to release the frame buffers allocated for the previous sequence.
Then VPU_DecGetInitialInfo() and VPU_DecIsseuSeqInit() are called before frame buffer allocation for a new sequence.
@endverbatim
*/
    DEC_FREE_FRAME_BUFFER,
    DEC_GET_FRAMEBUF_INFO, /**< This command gives HOST the information of framebuffer in <<vpuapi_h_DecGetFramebufInfo>>. (WAVE only) */
    DEC_RESET_FRAMEBUF_INFO, /**< This command resets the information of framebuffer. Unlike DEC_FREE_FRAME_BUFFER, it does not release the assigned memory itself.  This command is used for sequence change along with DEC_GET_FRAMEBUF_INFO. */
/**
@verbatim
This command decodes only an I-frame of picture from bitstream for using it as a thumbnail.
It requires as little as size of frame buffer since I-picture does not need any reference picture.
If HOST issues this command and sets one frame buffer address to FrameBuffer array in VPU_DecRegisterFrameBuffer(),
only the frame buffer is used.
And please make sure that the number of frame buffer `num` should be registered as minFrameBufferCount.
@endverbatim
*/
    ENABLE_DEC_THUMBNAIL_MODE,
/**
@verbatim
Applications can set a display flag for each frame buffer by calling this function after creating
decoder instance. If a certain display flag of frame buffer is set, the frame buffer cannot be used in the
decoding process. Applications can control displaying a buffer with this command
to prevent VPU from using buffer in every decoding process.

This command is the opposite of what VPU_DecClrDispFlag() does.
@endverbatim
*/
    DEC_SET_DISPLAY_FLAG,
    DEC_GET_SCALER_INFO, /**< This command returns setting information to downscale an image such as enable, width, and height. */
    DEC_SET_SCALER_INFO, /**< This command sets information to downscale an image such as enable, width, and height. */
    DEC_SET_TARGET_TEMPORAL_ID, /**< This command decodes only a frame whose temporal id is equal to or less than the given target temporal id. (H.265/HEVC decoder only) */           //!<< H.265 temporal scalability
    DEC_SET_BWB_CUR_FRAME_IDX, /**< This command specifies the index of linear frame buffer which needs to be changed to due to change of inter-frame resolution while decoding. (VP9 decoder only) */
    DEC_SET_FBC_CUR_FRAME_IDX, /**< This command specifies the index of FBC frame buffer which needs to be changed to due to change of inter-frame resolution while decoding.  (VP9 decoder only) */
    DEC_SET_INTER_RES_INFO_ON, /**< This command informs inter-frame resolution has been changed while decoding. After this command issued, VPU reallocates one frame buffer for the change. (VP9 decoder only) */
    DEC_SET_INTER_RES_INFO_OFF, /**< This command releases the flag informing inter-frame resolution change. It should be issued after reallocation of one frame buffer is completed. (VP9 decoder only) */
    DEC_FREE_FBC_TABLE_BUFFER, /**< This command frees one FBC table to deal with inter-frame resolution change. (VP9 decoder only) */
    DEC_FREE_MV_BUFFER, /**< This command frees one MV buffer to deal with inter-frame resolution change. (VP9 decoder only) */
    DEC_ALLOC_FBC_Y_TABLE_BUFFER, /**< This command allocates one FBC luma table to deal with inter-frame resolution change. (VP9 decoder only) */
    DEC_ALLOC_FBC_C_TABLE_BUFFER, /**< This command allocates one FBC chroma table to deal with inter-frame resolution change. (VP9 decoder only)  */
    DEC_ALLOC_MV_BUFFER, /**< This command allocates one MV buffer to deal with inter-frame resolution change. (VP9 decoder only) */
    DEC_SET_TEMPORAL_ID_MODE, /**< This command specifies the target temporal layer id selection mode. (WAVE decoder only) */
/**
@verbatim
This command writes an RBSP format of the SPS/PPS to parameter buffer.(CODA9 encoder only)
This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header
syntax was successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument cmd was invalid
which means the given cmd was undefined, or not allowed in the current instance. In this
case, the current instance might not be an MPEG4 encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` was invalid, which means
it has a null pointer, or given values for some member variables are improper values.
@endverbatim
*/
    ENC_SET_PARAM,
    //vpu put header stream to bitstream buffer
/**
@verbatim
This command inserts an MPEG4 header syntax or SPS or PPS to the HEVC/AVC bitstream to the bitstream
during encoding. It is valid for all types of encoders. The argument `parameter` is interpreted as a pointer to <<vpuapi_h_EncHeaderParam>> holding

* buf is a physical address pointing the generated stream location
* size is the size of generated stream in bytes
* headerType is a type of header that HOST application wants to generate and have values as
<<vpuapi_h_Mp4HeaderType>>, <<vpuapi_h_AvcHeaderType>>, <<vpuapi_h_WaveEncHeaderType>>.

This command has one of the following return codes.::
+
--
* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header
syntax was successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument cmd was invalid
which means the given cmd was undefined, or not allowed in the current instance. In this
case, the current instance might not be an MPEG4 encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or headerType
was invalid, which means it has a null pointer, or given values for some member variables
are improper values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not recieved any response from VPU and has timed out.
--
@endverbatim
*/
    ENC_PUT_VIDEO_HEADER,
/**
@verbatim
This command changes intra MB refresh number of header syntax during encoding. (ChangeParam command for CODA9 encoder only)
The argument `parameter` is interpreted as a pointer to integer which
represents an intra refresh number. It should be between 0 and
macroblock number of encoded picture.

This command returns the following code.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
    ENC_SET_INTRA_MB_REFRESH_NUMBER,
/**
@verbatim
This command enables HEC(Header Extension Code) syntax of MPEG4.

This command ignores the argument `parameter` and returns one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument, cmd, was invalid which means the given cmd was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an MPEG4 encoder instance.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
    ENC_ENABLE_HEC,
/**
@verbatim
This command disables HEC(Header Extension Code) syntax of MPEG4.

This command ignores the argument `parameter` and returns one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument, cmd, was invalid which means the given cmd was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an MPEG4 encoder instance.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
    ENC_DISABLE_HEC,
/**
@verbatim
This command changes slice inforamtion of header syntax during encoding. (ChangeParam command for CODA9 encoder only)
The argument `parameter` is interpreted as a pointer to <<vpuapi_h_EncSliceMode>>
structure holding

* sliceMode is a mode which means enabling multi slice structure
* sliceSizeMode is the mode representing mode of calculating one slicesize
* sliceSize is the size of one slice.

This command has one of the following return codes.::
+
--
* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
--
@endverbatim
*/
    ENC_SET_SLICE_INFO,
/**
@verbatim
This command changes GOP number of header syntax during encoding. (ChangeParam command for CODA9 encoder only)
The argument `parameter` is interpreted as a pointer to the integer which
represents a GOP number.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
    ENC_SET_GOP_NUMBER,
/**
@verbatim
This command changes intra QP value of header syntax during encoding. (ChangeParam command for CODA9 encoder only)
The argument `parameter` is interpreted as a pointer to the integer which
represents a Constant I frame QP. The Constant I frame QP should be between 1 and 31
for MPEG4 and between 0 and 51 for H.264/AVC.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given cmd was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
    ENC_SET_INTRA_QP,
/**
@verbatim
This command changes bitrate inforamtion of header syntax during encoding. (ChangeParam command for CODA9 encoder only)
The argument `parameter` is interpreted as a pointer to the integer which
represents a bitrate. It should be between 0 and 32767.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given `cmd` was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
    ENC_SET_BITRATE,
/**
@verbatim
This command changes frame rate of header syntax during encoding. (ChangeParam command for CODA9 encoder only)
The argument `parameter` is interpreted as a pointer to the integer which
represents a frame rate value. The fraem rate should be greater than 0.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given `cmd` was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
    ENC_SET_FRAME_RATE,

/**
@verbatim
This command changes encoding parameter(s) during the encoding operation. (WAVE encoder only)
The argument `parameter` is interpreted as a pointer to <<vpuapi_h_EncChangeParam>> structure holding

* enable_option : Set an enum value that is associated with parameters to change (multiple option allowed).

For instance, if bitrate and framerate need to be changed in the middle of encoding, that requires some setting like below.

 EncChangeParam changeParam;
 changeParam.enable_option	= ENC_RC_TARGET_RATE_CHANGE | ENC_FRAME_RATE_CHANGE;

 changeParam.bitrate		= 14213000;
 changeParam.frameRate		= 15;
 VPU_EncGiveCommand(handle, ENC_SET_PARA_CHANGE, &changeParam);

This command has one of the following return codes.::
+
--
* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given `cmd` was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an H.264/AVC encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not received any response from VPU and has timed out.
--
@endverbatim
*/
    ENC_SET_PARA_CHANGE,
    ENABLE_LOGGING, /**< HOST can activate message logging once VPU_DecOpen() or VPU_EncOpen() is called. */
    DISABLE_LOGGING, /**< HOST can deactivate message logging which is off as default. */
    DEC_GET_QUEUE_STATUS, /**< This command returns the number of queued commands for the current decode instance and the number of queued commands for the total decode instances.  */
    ENC_GET_QUEUE_STATUS, /**< This command returns the number of queued commands for the current encode instance and the number of queued commands for the total encode instances.  */
    GET_BANDWIDTH_REPORT, /**< This command reports the amount of bytes which are transferred on AXI bus. */     /* WAVE52x products. */
    ENC_WRPTR_SEL, /**< This command sets <<vpuapi_h_ENC_QUERY_WRPTR_SEL>>.  */
/**
@verbatim
This commmand sets the count of cycles per tick which is mostly a constant value to internal timer. Cycle per tick is used to calculate the number of cycle from the number of ticks that firmware returns. (WAVE5 only)
This command returns RETCODE_SUCCESS.
@endverbatim
*/
    SET_CYCLE_PER_TICK,
/**
@verbatim
This command obtains the value of releaseSrcFlag when srcReleaseIntEnable is 1 and source release interrupt is occurred. (WAVE5 only)
This command has one of the following return codes.::

* RETCODE_SUCCESS:
Requested operation was done successfully.
* RETCODE_QUERY_FAILURE:
This means this query command was not successful. (WAVE5 only)
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given `cmd` was
undefined, or not allowed in the current instance.
@endverbatim
*/
    ENC_GET_SRC_BUF_FLAG,
/**
@verbatim
This command returns the debug information on error situations such as hang-up. (WAVE5 only)
This command has one of the following return codes.::

* RETCODE_SUCCESS:
Requested operation was done successfully.
* RETCODE_REPORT_NOT_READY:
This means that report is not ready for this command. (WAVE5 only)
* RETCODE_QUERY_FAILURE:
This means this query command was not successful. (WAVE5 only)
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given `cmd` was
undefined, or not allowed in the current instance.
@endverbatim
*/
    GET_DEBUG_INFORM,
#ifdef SUPPORT_LOOK_AHEAD_RC
    ENC_SET_LARC_DATA, /*  < This command sets Look Ahead RC data */
#endif
    CMD_END
} CodecCommand;

/**
 * @brief   This is an enumeration type for representing error conceal modes. (H.264/AVC decoder only)
 */
typedef enum {
    AVC_ERROR_CONCEAL_MODE_DEFAULT                                      = 0, /**< basic error concealment and error concealment for missing reference frame, wrong frame_num syntax (default)  */
    AVC_ERROR_CONCEAL_MODE_ENABLE_SELECTIVE_CONCEAL_MISSING_REFERENCE   = 1, /**< error concealment - selective error concealment for missing reference frame */
    AVC_ERROR_CONCEAL_MODE_DISABLE_CONCEAL_MISSING_REFERENCE            = 2, /**< error concealment - disable error concealment for missing reference frame */
    AVC_ERROR_CONCEAL_MODE_DISABLE_CONCEAL_WRONG_FRAME_NUM              = 4, /**< error concealment - disable error concealment for wrong frame_num syntax */
} AVCErrorConcealMode;

/**
 * @brief   This is an enumeration type for representing the way of writing chroma data in planar format of frame buffer.
 */
 typedef enum {
    CBCR_ORDER_NORMAL,  /**< Cb data are written in Cb buffer, and Cr data are written in Cr buffer. */
    CBCR_ORDER_REVERSED /**< Cr data are written in Cb buffer, and Cb data are written in Cr buffer. */
} CbCrOrder;

/**
* @brief    This is an enumeration type for representing the mirroring direction.
*/
 typedef enum {
    MIRDIR_NONE,   /**< No mirroring */
    MIRDIR_VER,    /**< Vertical mirroring */
    MIRDIR_HOR,    /**< Horizontal mirroring */
    MIRDIR_HOR_VER /**< Horizontal and vertical mirroring */
} MirrorDirection;

/**
 * @brief   This is an enumeration type for representing chroma formats of the frame buffer and pixel formats in packed mode.
 */
 typedef enum {
    FORMAT_ERR           = -1,
    FORMAT_420           = 0 ,    /* 8bit */
    FORMAT_422               ,    /* 8bit */
    FORMAT_224               ,    /* 8bit */
    FORMAT_444               ,    /* 8bit */
    FORMAT_400               ,    /* 8bit */

                                  /* Little Endian Perspective    */
                                  /*     | addr 0 | addr 1  |    */
    FORMAT_420_P10_16BIT_MSB = 5, /* lsb |000000xx|xxxxxxxx | msb */
    FORMAT_420_P10_16BIT_LSB ,    /* lsb |xxxxxxx |xx000000 | msb */
    FORMAT_420_P10_32BIT_MSB ,    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */
    FORMAT_420_P10_32BIT_LSB ,    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */

                                  /* 4:2:2 packed format */
                                  /* Little Endian Perspective     */
                                  /*     | addr 0  | addr 1  |     */
    FORMAT_422_P10_16BIT_MSB ,    /* lsb |000000xx |xxxxxxxx | msb */
    FORMAT_422_P10_16BIT_LSB ,    /* lsb |xxxxxxxx |xx000000 | msb */
    FORMAT_422_P10_32BIT_MSB ,    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */
    FORMAT_422_P10_32BIT_LSB ,    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */

    FORMAT_YUYV              ,    /**< 8bit packed format : Y0U0Y1V0 Y2U1Y3V1 ... */
    FORMAT_YUYV_P10_16BIT_MSB,    /* lsb |000000xxxxxxxxxx | msb */ /**< 10bit packed(YUYV) format(1Pixel=2Byte) */
    FORMAT_YUYV_P10_16BIT_LSB,    /* lsb |xxxxxxxxxx000000 | msb */ /**< 10bit packed(YUYV) format(1Pixel=2Byte) */
    FORMAT_YUYV_P10_32BIT_MSB,    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(YUYV) format(3Pixel=4Byte) */
    FORMAT_YUYV_P10_32BIT_LSB,    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(YUYV) format(3Pixel=4Byte) */

    FORMAT_YVYU              ,    /**< 8bit packed format : Y0V0Y1U0 Y2V1Y3U1 ... */
    FORMAT_YVYU_P10_16BIT_MSB,    /* lsb |000000xxxxxxxxxx | msb */ /**< 10bit packed(YVYU) format(1Pixel=2Byte) */
    FORMAT_YVYU_P10_16BIT_LSB,    /* lsb |xxxxxxxxxx000000 | msb */ /**< 10bit packed(YVYU) format(1Pixel=2Byte) */
    FORMAT_YVYU_P10_32BIT_MSB,    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(YVYU) format(3Pixel=4Byte) */
    FORMAT_YVYU_P10_32BIT_LSB,    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(YVYU) format(3Pixel=4Byte) */

    FORMAT_UYVY              ,    /**< 8bit packed format : U0Y0V0Y1 U1Y2V1Y3 ... */
    FORMAT_UYVY_P10_16BIT_MSB,    /* lsb |000000xxxxxxxxxx | msb */ /**< 10bit packed(UYVY) format(1Pixel=2Byte) */
    FORMAT_UYVY_P10_16BIT_LSB,    /* lsb |xxxxxxxxxx000000 | msb */ /**< 10bit packed(UYVY) format(1Pixel=2Byte) */
    FORMAT_UYVY_P10_32BIT_MSB,    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(UYVY) format(3Pixel=4Byte) */
    FORMAT_UYVY_P10_32BIT_LSB,    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(UYVY) format(3Pixel=4Byte) */

    FORMAT_VYUY              ,    /**< 8bit packed format : V0Y0U0Y1 V1Y2U1Y3 ... */
    FORMAT_VYUY_P10_16BIT_MSB,    /* lsb |000000xxxxxxxxxx | msb */ /**< 10bit packed(VYUY) format(1Pixel=2Byte) */
    FORMAT_VYUY_P10_16BIT_LSB,    /* lsb |xxxxxxxxxx000000 | msb */ /**< 10bit packed(VYUY) format(1Pixel=2Byte) */
    FORMAT_VYUY_P10_32BIT_MSB,    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(VYUY) format(3Pixel=4Byte) */
    FORMAT_VYUY_P10_32BIT_LSB,    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(VYUY) format(3Pixel=4Byte) */

    FORMAT_MAX,
} FrameBufferFormat;

/**
 * @brief   This is an enumeration type for representing output image formats of down scaler.
 */
typedef enum{
    YUV_FORMAT_I420,    /**< This format is a 420 planar format, which is described as forcc I420. */
    YUV_FORMAT_NV12,    /**< This format is a 420 semi-planar format with U and V interleaved, which is described as fourcc NV12. */
    YUV_FORMAT_NV21,    /**< This format is a 420 semi-planar format with V and U interleaved, which is described as fourcc NV21. */
    YUV_FORMAT_I422,    /**< This format is a 422 planar format, which is described as forcc I422. */
    YUV_FORMAT_NV16,    /**< This format is a 422 semi-planar format with U and V interleaved, which is described as fourcc NV16. */
    YUV_FORMAT_NV61,    /**< This format is a 422 semi-planar format with V and U interleaved, which is described as fourcc NV61. */
    YUV_FORMAT_UYVY,    /**< This format is a 422 packed mode with UYVY, which is described as fourcc UYVY. */
    YUV_FORMAT_YUYV,    /**< This format is a 422 packed mode with YUYV, which is described as fourcc YUYV. */
} ScalerImageFormat;

/**
 * @brief   This is an enumeration type for representing YUV packed format.
 */
typedef enum {
    NOT_PACKED = 0,
    PACKED_YUYV,
    PACKED_YVYU,
    PACKED_UYVY,
    PACKED_VYUY,
} PackedFormatNum;

/**
* @brief    This is an enumeration type for representing interrupt bit positions for CODA series.
*/
typedef enum {
    INT_BIT_INIT            = 0,
    INT_BIT_SEQ_INIT        = 1,
    INT_BIT_SEQ_END         = 2,
    INT_BIT_PIC_RUN         = 3,
    INT_BIT_FRAMEBUF_SET    = 4,
    INT_BIT_ENC_HEADER      = 5,
    INT_BIT_DEC_PARA_SET    = 7,
    INT_BIT_DEC_BUF_FLUSH   = 8,
    INT_BIT_USERDATA        = 9,
    INT_BIT_DEC_FIELD       = 10,
    INT_BIT_DEC_MB_ROWS     = 13,
    INT_BIT_BIT_BUF_EMPTY   = 14,
    INT_BIT_BIT_BUF_FULL    = 15
} InterruptBit;

/* For backward compatibility */
typedef InterruptBit Coda9InterruptBit;


/**
* @brief    This is an enumeration type for representing interrupt bit positions.

NOTE: This is only for WAVE5 IP.
*/
typedef enum {
    INT_WAVE5_INIT_VPU          = 0,
    INT_WAVE5_WAKEUP_VPU        = 1,
    INT_WAVE5_SLEEP_VPU         = 2,
    INT_WAVE5_CREATE_INSTANCE   = 3,
    INT_WAVE5_FLUSH_INSTANCE    = 4,
    INT_WAVE5_DESTORY_INSTANCE  = 5,
    INT_WAVE5_INIT_SEQ          = 6,
    INT_WAVE5_SET_FRAMEBUF      = 7,
    INT_WAVE5_DEC_PIC           = 8,
    INT_WAVE5_ENC_PIC           = 8,
    INT_WAVE5_ENC_SET_PARAM     = 9,
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
    INT_WAVE5_ENC_SRC_RELEASE   = 10,
#endif
    INT_WAVE5_ENC_LOW_LATENCY   = 13,
    INT_WAVE5_DEC_QUERY         = 14,
    INT_WAVE5_BSBUF_EMPTY       = 15,
    INT_WAVE5_BSBUF_FULL        = 15,
} Wave5InterruptBit;

/**
* @brief    This is an enumeration type for representing picture types.
*/
typedef enum {
    PIC_TYPE_I            = 0, /**< I picture */
    PIC_TYPE_KEY          = 0, /**< KEY frame for SVAC and AV1*/
    PIC_TYPE_P            = 1, /**< P picture */
    PIC_TYPE_INTER        = 1, /**< Inter frame for SVAC and AV1*/
    PIC_TYPE_B            = 2, /**< B picture (except VC1) */
    PIC_TYPE_REPEAT       = 2, /**< Repeat frame (VP9 only) */
    PIC_TYPE_AV1_INTRA    = 2, /**< Intra only frame (AV1 only) */
    PIC_TYPE_VC1_BI       = 2, /**< VC1 BI picture (VC1 only) */
    PIC_TYPE_VC1_B        = 3, /**< VC1 B picture (VC1 only) */
    PIC_TYPE_D            = 3, /**< D picture in MPEG2 that is only composed of DC coefficients (MPEG2 only) */
    PIC_TYPE_S            = 3, /**< S picture in MPEG4 that is an acronym of Sprite and used for GMC (MPEG4 only)*/
    PIC_TYPE_AVS2_F       = 3, /**< F picture in AVS2 */
    PIC_TYPE_AV1_SWITCH   = 3, /**< Switch frame (AV1 only) */
    PIC_TYPE_VC1_P_SKIP   = 4, /**< VC1 P skip picture (VC1 only) */
    PIC_TYPE_MP4_P_SKIP_NOT_CODED = 4, /**< Not Coded P Picture in MPEG4 packed mode */
    PIC_TYPE_AVS2_S       = 4, /**< S picture in AVS2 */
    PIC_TYPE_IDR          = 5, /**< H.264/H.265 IDR picture */
    PIC_TYPE_AVS2_G       = 5, /**< G picture in AVS2 */
    PIC_TYPE_AVS2_GB      = 6, /**< GB picture in AVS2 */
    PIC_TYPE_MAX               /**< No Meaning */
}PicType;

/**
* @brief    This is an enumeration type for H.264/AVC NPF (Non Paired Field) information.
*/
typedef enum {
    PAIRED_FIELD          = 0,
    TOP_FIELD_MISSING     = 1,
    BOT_FIELD_MISSING     = 2,
}AvcNpfFieldInfo;

/**
* @brief    This is an enumeration type for specifying frame buffer types when tiled2linear or wtlEnable is used.
*/
typedef enum {
    FF_NONE                 = 0, /**< Frame buffer type when tiled2linear or wtlEnable is disabled */
    FF_FRAME                = 1, /**< Frame buffer type to store one frame */
    FF_FIELD                = 2, /**< Frame buffer type to store top field or bottom field separately */
}FrameFlag;

/**
 * @brief   This is an enumeration type for representing bitstream handling modes in decoder.
 */
typedef enum {
    BS_MODE_INTERRUPT,  /**< VPU returns an interrupt when bitstream buffer is empty while decoding. VPU waits for more bitstream to be filled. */
    BS_MODE_RESERVED,   /**< Reserved for the future */
    BS_MODE_PIC_END,    /**< VPU tries to decode with very small amount of bitstream (not a complete 512-byte chunk). If it is not successful, VPU performs error concealment for the rest of the frame. */
}BitStreamMode;

/**
 * @brief  This is an enumeration type for representing software reset options.
 */
typedef enum {
    SW_RESET_SAFETY,    /**< It resets VPU in safe way. It waits until pending bus transaction is completed and then perform reset. */
    SW_RESET_FORCE,     /**< It forces to reset VPU without waiting pending bus transaction to be completed. It is used for immediate termination such as system off. */
    SW_RESET_ON_BOOT    /**< This is the default reset mode that is executed since system booting.  This mode is actually executed in VPU_Init(), so does not have to be used independently. */
}SWResetMode;

/**
 * @brief  This is an enumeration type for representing product IDs.
 */
typedef enum {
    PRODUCT_ID_980,
    PRODUCT_ID_960  = 1,
    PRODUCT_ID_950  = 1,    // same with CODA960
    PRODUCT_ID_512,
    PRODUCT_ID_515,
    PRODUCT_ID_521,
    PRODUCT_ID_511,
    PRODUCT_ID_517,
    PRODUCT_ID_NONE,
}ProductId;

#define PRODUCT_ID_W_SERIES(x)      (x == PRODUCT_ID_512 || x == PRODUCT_ID_515 || x == PRODUCT_ID_521 || x == PRODUCT_ID_511 || x == PRODUCT_ID_517)
#define PRODUCT_ID_NOT_W_SERIES(x)  !PRODUCT_ID_W_SERIES(x)

/**
* @brief This is an enumeration type for representing map types for frame buffer.

NOTE: Tiled maps are only for CODA9. Please find them in the CODA9 datasheet for detailed information.
*/
typedef enum {
/**
@verbatim
Linear frame map type

NOTE: Products earlier than CODA9 can only set this linear map type.
@endverbatim
*/
    LINEAR_FRAME_MAP                            = 0,  /**< Linear frame map type */
    TILED_FRAME_V_MAP                           = 1,  /**< Tiled frame vertical map type (CODA9 only) */
    TILED_FRAME_H_MAP                           = 2,  /**< Tiled frame horizontal map type (CODA9 only) */
    TILED_FIELD_V_MAP                           = 3,  /**< Tiled field vertical map type (CODA9 only) */
    TILED_MIXED_V_MAP                           = 4,  /**< Tiled mixed vertical map type (CODA9 only) */
    TILED_FRAME_MB_RASTER_MAP                   = 5,  /**< Tiled frame MB raster map type (CODA9 only) */
    TILED_FIELD_MB_RASTER_MAP                   = 6,  /**< Tiled field MB raster map type (CODA9 only) */
    TILED_FRAME_NO_BANK_MAP                     = 7,  /**< Tiled frame no bank map. (CODA9 only) */ // coda980 only
    TILED_FIELD_NO_BANK_MAP                     = 8,  /**< Tiled field no bank map. (CODA9 only) */ // coda980 only
    LINEAR_FIELD_MAP                            = 9,  /**< Linear field map type. (CODA9 only) */ // coda980 only
    CODA_TILED_MAP_TYPE_MAX                     = 10,
    ARM_COMPRESSED_FRAME_MAP                    = 10, /**< AFBC(ARM Frame Buffer Compression) compressed frame map type */ // AFBC enabled WAVE decoder
    COMPRESSED_FRAME_MAP_V50_LOSSLESS_8BIT      = 11, /**< CFRAME50(Chips&Media Frame Buffer Compression) compressed framebuffer type */
    COMPRESSED_FRAME_MAP_V50_LOSSLESS_10BIT     = 12, /**< CFRAME50(Chips&Media Frame Buffer Compression) compressed framebuffer type */
    COMPRESSED_FRAME_MAP_V50_LOSSY              = 13, /**< CFRAME50(Chips&Media Frame Buffer Compression) compressed framebuffer type */
    COMPRESSED_FRAME_MAP_V50_LOSSLESS_422_8BIT  = 14, /**< CFRAME50(Chips&Media Frame Buffer Compression) compressed 4:2:2 framebuffer type */
    COMPRESSED_FRAME_MAP_V50_LOSSLESS_422_10BIT = 15, /**< CFRAME50(Chips&Media Frame Buffer Compression) compressed 4:2:2 framebuffer type */
    COMPRESSED_FRAME_MAP_V50_LOSSY_422          = 16, /**< CFRAME50(Chips&Media Frame Buffer Compression) compressed 4:2:2 framebuffer type */
    COMPRESSED_FRAME_MAP                        = 17, /**< Compressed frame map type (WAVE only) */ // WAVE4 only
    COMPRESSED_FRAME_MAP_SVAC_SVC_BL            = 18, /**< Compressed frame map type for base layer in SVAC encoder */
    COMPRESSED_FRAME_MAP_DUAL_CORE_8BIT         = 19, /**< 8bit compressed frame map type for dual core */
    COMPRESSED_FRAME_MAP_DUAL_CORE_10BIT        = 20, /**< 10bit compressed frame map type for dual core */
    PVRIC_COMPRESSED_FRAME_LOSSLESS_MAP         = 21, /**< PVRIC compressed frame map type for lossless mode*/
    PVRIC_COMPRESSED_FRAME_LOSSY_MAP            = 22, /**< PVRIC compressed frame map type for lossy mode */
    TILED_MAP_TYPE_MAX
} TiledMapType;

/**
* @brief    This is an enumeration for declaring a type of framebuffer that is allocated when VPU_DecAllocateFrameBuffer()
and VPU_EncAllocateFrameBuffer() function call.
*/
typedef enum {
    FB_TYPE_CODEC,  /**< A framebuffer type used for decoding or encoding  */
    FB_TYPE_PPU,    /**< A framebuffer type used for additional allocation of framebuffer for postprocessing(rotation/mirror) or display (tiled2linear) purpose */
} FramebufferAllocType;

/**
* @brief    This is a data structure of hardware attributes for VPU.
*/
typedef struct {
    Uint32  productId;                  /**< The product ID */
    Uint32  productNumber;              /**< The product number. ex) 512 */
    char    productName[8];             /**< The product name in ascii code */
    Uint32  productVersion;             /**< The product version number */
    Uint32  fwVersion;                  /**< The F/W version */
    Uint32  customerId;                 /**< Customer ID number */
    Uint32  supportDecoders;            /**< bitmask: See <<vpuapi_h_CodStd>> */
    Uint32  supportEncoders;            /**< bitmask: See <<vpuapi_h_CodStd>> */
    Uint32  numberOfMemProtectRgns;     /**< It is always 10. */
    Uint32  maxNumVcores;               /**< The number of core in the VPU */
    BOOL    supportWTL;                 /**< This indicates whether a product supports the WTL or not */
    BOOL    supportTiled2Linear;        /**< This indicates whether a product supports the Tile2Linear or not */
    BOOL    supportTiledMap;            /**< This indicates whether a product supports the TiledMap or not */
    BOOL    supportMapTypes;            /**< This indicates whether a product supports map types. */
    BOOL    supportLinear2Tiled;        /**< This indicates whether a product supports the Linear2Tiled. - Encoder feature */
    BOOL    support128bitBus;           /**< This indicates whether a product supports 128bit bus. */
    BOOL    supportThumbnailMode;       /**< This indicates whether a product supports the thumbnail mode. */
    BOOL    supportBitstreamMode;       /**< This indicates whether a product supports bitstream modes such as pic-end mode and ringbuffer mode. */
    BOOL    supportFBCBWOptimization;   /**< This indicates whether a product supports the FBC optimization. */
    BOOL    supportGDIHW;               /**< This indicates whether a product supports the GDI. */
    BOOL    supportCommandQueue;        /**< This indicates whether a product supports the Command Queue. */
    BOOL    supportBackbone;            /**< This indicates whether a product supports the Backbone. Higher version of the GDI. */
    BOOL    supportNewTimer;            /**< 0: timeeScale=32768, 1: timerScale=256 (tick = cycle/timerScale) */
    BOOL    support2AlignScaler;        /**< This indicates whether a product supports the 2-align scaler. */
    BOOL    supportAVC10bitEnc;         /**< This indicates whether a product supports AVC 10bit encoder */
    BOOL    supportHEVC10bitEnc;        /**< This indicates whether a product supports HEVC 10bit encoder */
    Uint32  supportEndianMask;          /**< A variable of supported endian mode in product */
    Uint32  framebufferCacheType;       /**< 0: None, 1: Maverick-I, 2: Maverick-II */
    Uint32  bitstreamBufferMargin;      /**< A variable to leave a margin in bistream buffer for GBU operation. This is valid when ringBufferEnable is 1. (CODA9 only) */
    Uint32  hwConfigDef0;               /**< System configuration information (internal use only) such as external memory interface, external model information, and output support */
    Uint32  hwConfigDef1;               /**< General hardware configuration information (internal use only) */
    Uint32  hwConfigFeature;            /**< supported codec standards */
    Uint32  hwConfigDate;               /**< Configuation Date Decimal, ex)20151130 */
    Uint32  hwConfigRev;                /**< SVN revision, ex) 83521 */
    Uint32  hwConfigType;               /**< Not defined yet */
    BOOL    supportDualCore;            /**< This indicates whether a product has two vcores. */
    BOOL    supportVcoreBackbone;       /**< This indicates whether a product supports the vcore backbone version.*/
} VpuAttr;


/**
* @brief
@verbatim
This is a data structure of tiled map information.

NOTE: WAVE does not support tiledmap type so this structure is not used in the product.
@endverbatim
*/
typedef struct {
    // gdi2.0
    int xy2axiLumMap[32];
    int xy2axiChrMap[32];
    int xy2axiConfig;

    // gdi1.0
    int xy2caMap[16];
    int xy2baMap[16];
    int xy2raMap[16];
    int rbc2axiMap[32];
    int xy2rbcConfig;
    unsigned long tiledBaseAddr;

    // common
    int mapType;
    int productId;
    int tbSeparateMap;
    int topBotSplit;
    int tiledMap;
    int convLinear;
} TiledMapConfig;

/**
* @brief    This is a data structure of DRAM information(CODA960 and BODA950 only) and CFRAME50 configuration(WAVE5 only)
VPUAPI sets default values for this structure.
However, HOST application can configure if the default values are not associated with their DRAM
    or desirable to change.
*/
typedef struct {
    int rasBit;     /**< This value is used for width of RAS bit. (13 on the CNM FPGA platform) */
    int casBit;     /**< This value is used for width of CAS bit. (9 on the CNM FPGA platform) */
    int bankBit;    /**< This value is used for width of BANK bit. (2 on the CNM FPGA platform) */
    int busBit;     /**< This value is used for width of system BUS bit. (3 on CNM FPGA platform) */
    int tx16y;      /**< This value is used for CFRAME50(Chips&Media Frame Buffer Compression) (WAVE5 only) */
    int tx16c;      /**< This value is used for CFRAME50(Chips&Media Frame Buffer Compression) (WAVE5 only) */
} DRAMConfig;

/**
* @brief
@verbatim
This is a data structure for representing frame buffer information such as pointer of each YUV
component, endian, map type, etc.

All of the 3 component addresses must be aligned to AXI bus width.
HOST application must allocate external SDRAM spaces for those components by using this data
structure. For example, YCbCr 4:2:0, one pixel value
of a component occupies one byte, so the frame data sizes of Cb and Cr buffer are 1/4 of Y buffer size.

In case of CbCr interleave mode, Cb and Cr frame data are written to memory area started from bufCb address.
Also, in case that the map type of frame buffer is a field type, the base addresses of frame buffer for bottom fields -
bufYBot, bufCbBot and bufCrBot should be set separately.
@endverbatim
*/
typedef struct {
    PhysicalAddress bufY;       /**< It indicates the base address for Y component in the physical address space when linear map is used. It is the RAS base address for Y component when tiled map is used (CODA9). It is also compressed Y buffer or ARM compressed framebuffer (WAVE). */
    PhysicalAddress bufCb;      /**< It indicates the base address for Cb component in the physical address space when linear map is used. It is the RAS base address for Cb component when tiled map is used (CODA9). It is also compressed CbCr buffer (WAVE) */
    PhysicalAddress bufCr;      /**< It indicates the base address for Cr component in the physical address space when linear map is used. It is the RAS base address for Cr component when tiled map is used (CODA9). */
    PhysicalAddress bufYBot;    /**< It indicates the base address for Y bottom field component in the physical address space when linear map is used. It is the RAS base address for Y bottom field component when tiled map is used (CODA980 only). */ // coda980 only
    PhysicalAddress bufCbBot;   /**< It indicates the base address for Cb bottom field component in the physical address space when linear map is used. It is the RAS base address for Cb bottom field component when tiled map is used (CODA980 only). */ // coda980 only
    PhysicalAddress bufCrBot;   /**< It indicates the base address for Cr bottom field component in the physical address space when linear map is used. It is the RAS base address for Cr bottom field component when tiled map is used (CODA980 only). */ // coda980 only
/**
@verbatim
It specifies a chroma interleave mode of frame buffer.

@* 0 : Cb data are written in Cb frame memory and Cr data are written in Cr frame memory. (chroma separate mode)
@* 1 : Cb and Cr data are written in the same chroma memory. (chroma interleave mode)
@endverbatim
*/
    int cbcrInterleave;
/**
@verbatim
It specifies the way chroma data is interleaved in the frame buffer, bufCb or bufCbBot.

@* 0 : CbCr data is interleaved in chroma memory (NV12).
@* 1 : CrCb data is interleaved in chroma memory (NV21).
@endverbatim
*/
    int nv21;
/**
@verbatim
It specifies endianess of frame buffer.

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE Datasheet'.
@endverbatim
*/
    int endian;
    int myIndex;           /**< A frame buffer index to identify each frame buffer that is processed by VPU. */
    TiledMapType mapType;  /**< A map type for GDI inferface or FBC (Frame Buffer Compression). NOTE: For detailed map types, please refer to <<vpuapi_h_TiledMapType>>. */
    int stride;            /**< A horizontal stride for given frame buffer */
    int width;             /**< A width for given frame buffer */
    int height;            /**< A height for given frame buffer */
    int size;              /**< A size for given frame buffer */
    int lumaBitDepth;      /**< Bit depth for luma component */
    int chromaBitDepth;    /**< Bit depth for chroma component  */
    FrameBufferFormat   format;     /**< A YUV format of frame buffer */
/**
@verbatim
It enables source frame data with long burst length to be loaded for reducing DMA latency (CODA9 encoder only).

@* 0 : disable the long-burst mode.
@* 1 : enable the long-burst mode.
@endverbatim
*/
    int sourceLBurstEn;
    int sequenceNo;     /**< A sequence number that the frame belongs to. It increases by 1 every time a sequence changes in decoder.  */

    BOOL updateFbInfo; /**< If this is TRUE, VPU updates API-internal framebuffer information when any of the information is changed. */
} FrameBuffer;

/**
* @brief    This is a data structure for representing framebuffer parameters.
It is used when framebuffer allocation using VPU_DecAllocateFrameBuffer() or VPU_EncAllocateFrameBuffer().
*/
typedef struct {
    int mapType;                /**< <<vpuapi_h_TiledMapType>> */
/**
@verbatim
@* 0 : Cb data are written in Cb frame memory and Cr data are written in Cr frame memory. (chroma separate mode)
@* 1 : Cb and Cr data are written in the same chroma memory. (chroma interleave mode)
@endverbatim
*/
    int cbcrInterleave;
    int nv21;                   /**< 1 : CrCb (NV21), 0 : CbCr (NV12).  This is valid when cbcrInterleave is 1. */
    FrameBufferFormat format;   /**< <<vpuapi_h_FrameBufferFormat>>  */
    int stride;                 /**< A stride value of frame buffer  */
    int height;                 /**< A height of frame buffer  */
    int size;                   /**< A size of frame buffer  */
    int lumaBitDepth;           /**< A bit-depth of luma sample */
    int chromaBitDepth;         /**< A bit-depth of chroma sample */
    int endian;                 /**< An endianess of frame buffer  */
    int num;                    /**< The number of frame buffer to allocate  */
    int type;                   /**< <<vpuapi_h_FramebufferAllocType>>  */
} FrameBufferAllocInfo;

/**
* @brief
@verbatim
This is a data structure for representing rectangular window information in a
frame.

In order to specify a display window (or display window after cropping), this structure is
provided to HOST application. Each value means an offset from the start point of
a frame and therefore, all variables have positive values.
@endverbatim
*/
typedef struct {
    Uint32 left;    /**< A horizontal pixel offset of top-left corner of rectangle from (0, 0) */
    Uint32 top;     /**< A vertical pixel offset of top-left corner of rectangle from (0, 0) */
    Uint32 right;   /**< A horizontal pixel offset of bottom-right corner of rectangle from (0, 0) */
    Uint32 bottom;  /**< A vertical pixel offset of bottom-right corner of rectangle from (0, 0) */
} VpuRect;

//Theora specific display information
/**
* @brief    This is a data structure of picture size information. This structure is valid only for Theora decoding case.
When HOST application allocates frame buffers and gets a displayable picture region, HOST application needs this information.
*/
typedef struct {
    int frameWidth;     /**< This value is used for width of frame buffer. */
    int frameHeight;    /**< This value is used for height of frame buffer. */
    int picWidth;       /**< This value is used for width of the picture region to be displayed. */
    int picHeight;      /**< This value is used for height of the picture region to be displayed. */
    int picOffsetX;     /**< This value is located at the lower-left corner of the picture region to be displayed. */
    int picOffsetY;     /**< This value is located at the lower-left corner of the picture region to be displayed. */
} ThoScaleInfo;

// VP8 specific display information
/**
* @brief    This is a data structure of picture upscaling information for post-processing out of decoding loop.
This structure is valid only for VP8 decoding case and can never be used by VPU itself.
If HOST application has an upsampling device, this information is useful for them.
When the HOST application allocates a frame buffer, HOST application needs upscaled resolution derived by this information
 to allocate enough (maximum) memory for variable resolution picture decoding.
*/
typedef struct {
/**
@verbatim
This is an upscaling factor for horizontal expansion.
The value could be 0 to 3, and meaning of each value is described in below table.

.Upsampling Ratio by Scale Factor
[separator="|", frame="all", grid="all"]
`50`50_70
h/vScaleFactor     | Upsampling Ratio
________________________________________________________________________________
0                  |1
1                  |5/4
2                  |5/3
3                  |2/1
________________________________________________________________________________

@endverbatim
*/
    unsigned hScaleFactor   : 2;
    unsigned vScaleFactor   : 2;    /**< This is an upscaling factor for vertical expansion. The value could be 0 to 3, meaning of each value is described in above table. */
    unsigned picWidth       : 14;   /**< Picture width in unit of sample */
    unsigned picHeight      : 14;   /**< Picture height in unit of sample */
} Vp8ScaleInfo;

typedef struct {
    BOOL        enScaler;
    Uint32      scaleWidth;
    Uint32      scaleHeight;
} ScalerInfo;

/**
* @brief    The data structure to enable low delay decoding.
*/
typedef struct {
/**
@verbatim
This enables low delay decoding. (CODA980 H.264/AVC decoder only)

If this flag is 1, VPU sends an interrupt to HOST application when numRows decoding is done.

* 0 : disable
* 1 : enable

When this field is enabled, reorderEnable, tiled2LinearEnable, and the post-rotator should be disabled.
@endverbatim
*/
    int lowDelayEn;
/**
@verbatim
This field indicates the number of mb rows (macroblock unit).

The value is from 1 to height/16 - 1.
If the value of this field is 0 or picture height/16, low delay decoding is disabled even though lowDelayEn is 1.
@endverbatim
*/
    int numRows;
} LowDelayInfo;

/**
* @brief    This is a data structure for representing use of secondary AXI for each hardware block.
*/
typedef struct {
    union {
        struct {
            int useBitEnable;   /**<  This enables AXI secondary channel for prediction data of the BIT-processor. */
            int useIpEnable;    /**<  This enables AXI secondary channel for row pixel data of IP.  */
            int useDbkYEnable;  /**<  This enables AXI secondary channel for temporal luminance data of the de-blocking filter. */
            int useDbkCEnable;  /**<  This enables AXI secondary channel for temporal chrominance data of the de-blocking filter.  */
            int useOvlEnable;   /**<   This enables AXI secondary channel for temporal data of the the overlap filter (VC1 only). */
            int useBtpEnable;   /**<  This enables AXI secondary channel for bit-plane data of the BIT-processor (VC1 only).  */
        } coda9;
        struct {
            // for Decoder
            int useSclEnable; /**< This enables AXI secondary channel for Scaler line buffer. */
            int useBitEnable;   /**<  This enables AXI secondary channel for prediction data of the BIT-processor. */
            int useIpEnable;    /**<  This enables AXI secondary channel for row pixel data of IP. */
            int useLfRowEnable; /**<  This enables AXI secondary channel for loopfilter.   */

            // for Encoder
            int useEncImdEnable; /**< This enables AXI secondary channel for intra mode decision. */
            int useEncLfEnable;  /**< This enables AXI secondary channel for loopfilter. */
            int useEncRdoEnable; /**< This enables AXI secondary channel for RDO. */
        } wave;
    } u;
} SecAxiUse;

// For MaverickCache1
/**
* @brief    This is a data structure for representing cache rectangle area for each component of MC reference frame. (CODA9 only)
*/
typedef struct {
    unsigned BufferSize     : 8; /**< This is the cache buffer size for each component and can be set with 0 to 255. The unit of this value is fixed with 256byte. */
    unsigned PageSizeX      : 4; /**< This is the cache page size and can be set as 0 to 4. With this value(n), 8*(2^n) byte is requested as the width of a page. */
    unsigned PageSizeY      : 4; /**< This is the cache page size and can be set as 0 to 7. With this value(m), a page width*(2^m) byte is requested as the rectangle of a page.*/
    unsigned CacheSizeX     : 4; /**< This is the component data cache size, and it can be set as 0 to 7 in a page unit. Then there can be 2^n pages in x(y)-direction. Make sure that for luma component the CacheSizeX + CacheSizeY must be less than 8. For chroma components, CacheSizeX + CacheSizeY must be less than 7. */
    unsigned CacheSizeY     : 4; /**< This is the component data cache size, and it can be set as 0 to 7 in a page unit. Then there can be 2^n pages in x(y)-direction. Make sure that for luma component the CacheSizeX + CacheSizeY must be less than 8. For chroma components, CacheSizeX + CacheSizeY must be less than 7. */
    unsigned Reserved       : 8;
} CacheSizeCfg;

/**
* @brief    This is a data structure for cache configuration. (CODA9 only)
*/
typedef struct {
    struct {
        union {
            Uint32 word;
            CacheSizeCfg cfg; /**< <<vpuapi_h_CacheSizeCfg>> */
        } luma;
        union {
            Uint32 word;
            CacheSizeCfg cfg; /**< <<vpuapi_h_CacheSizeCfg>> */
        } chroma;
/**
@verbatim
It disables cache function.

@* 1 : cache off
@* 0 : cache on
@endverbatim
*/
        unsigned Bypass         : 1;
/**
@verbatim
It enables two frame caching mode.

@* 1 : dual mode (caching for FrameIndex0 and FrameIndex1)
@* 0 : single mode (caching for FrameIndex0)
@endverbatim
*/
        unsigned DualConf       : 1;
/**
@verbatim
Mode for page merging

@* 0 : disable
@* 1 : horizontal
@* 2 : vertical

We recommend you to set 1 (horizontal) in tiled map or to set 2 (vertical) in linear map.
@endverbatim
*/
        unsigned PageMerge      : 2;
    } type1;
    struct {
/**
@verbatim
CacheMode represents cache configuration.

@* [10:9] : cache line processing direction and merge mode
@* [8:5] : CacheWayShape
@** [8:7] : CacheWayLuma
@** [6:5] : CacheWayChroma
@* [4] reserved
@* [3] CacheBurstMode
@** 0: burst 4
@** 1: bust 8
@* [2] CacheMapType
@** 0: linear
@** 1: tiled
@* [1] CacheBypassME
@** 0: cache enable
@** 1: cache disable (bypass)
@* [0] CacheBypassMC
@** 0: cache enable
@** 1: cache disable (bypass)
@endverbatim
*/
 unsigned int CacheMode;
    } type2;
} MaverickCacheConfig;

/**
* @brief    This structure is used when HOST application additionally wants to send SPS data
or PPS data from external way. The resulting SPS data or PPS data can be used in
real applications as a kind of out-of-band information.
*/
typedef struct {
    Uint32 * paraSet;   /**< The SPS/PPS rbsp data */
    int     size;       /**< The size of stream in byte */
} DecParamSet;

typedef enum {
    DEC_TASK, DEC_WORK, DEC_FBC, DEC_FBCY_TBL, DEC_FBCC_TBL, DEC_BS, DEC_FB_LINEAR, DEC_MV, DEC_ETC,
    ENC_TASK, ENC_WORK, ENC_FBC, ENC_FBCY_TBL, ENC_FBCC_TBL, ENC_BS, ENC_SRC,       ENC_MV, ENC_SUBSAMBUF, ENC_ETC
}MemTypes;



struct CodecInst;

//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------
#define VPU_HANDLE_INSTANCE_NO(_handle)         (_handle->instIndex)
#define VPU_HANDLE_CORE_INDEX(_handle)          (((CodecInst*)_handle)->coreIdx)
#define VPU_HANDLE_PRODUCT_ID(_handle)          (((CodecInst*)_handle)->productId)
#define VPU_CONVERT_WTL_INDEX(_handle, _index)  ((((CodecInst*)_handle)->CodecInfo->decInfo).numFbsForDecoding+_index)
#define VPU_HANDLE_TO_DECINFO(_handle)          (&(((CodecInst*)_handle)->CodecInfo->decInfo))
#define VPU_HANDLE_TO_ENCINFO(_handle)          (&(((CodecInst*)_handle)->CodecInfo->encInfo))
/**
* @brief
@verbatim
This is a dedicated type for handle returned when a decoder instance or a encoder instance is
opened.

@endverbatim
*/
typedef struct CodecInst* VpuHandle;

/**
* @brief
@verbatim
This is a dedicated type for decoder handle returned when a decoder instance is
opened. A decoder instance can be referred to by the corresponding handle. CodecInst
is a type managed internally by API. Application does not need to care about it.

NOTE: This type is vaild for decoder only.
@endverbatim
*/
typedef struct CodecInst* DecHandle;

/**
* @brief    This is a data structure for H.264/AVC specific picture information. Only H.264/AVC decoder
returns this structure after decoding a frame. For details about all these
flags, please find them in H.264/AVC VUI syntax.
*/
typedef struct {
/**
@verbatim
@* 1 : It indicates that the temporal distance between the decoder output times of any
two consecutive pictures in output order is constrained as fixed_frame_rate_flag
in H.264/AVC VUI syntax.
@* 0 : It indicates that no such constraints apply to the temporal distance between
the decoder output times of any two consecutive pictures in output order
@endverbatim
*/
    int fixedFrameRateFlag;
/**
@verbatim
timing_info_present_flag in H.264/AVC VUI syntax

@* 1 : FixedFrameRateFlag is valid.
@* 0 : FixedFrameRateFlag is not valid.
@endverbatim
*/
    int timingInfoPresent;
    int chromaLocBotField;      /**< chroma_sample_loc_type_bottom_field in H.264/AVC VUI syntax. It specifies the location of chroma samples for the bottom field. */
    int chromaLocTopField;      /**< chroma_sample_loc_type_top_field in H.264/AVC VUI syntax. It specifiesf the location of chroma samples for the top field. */
    int chromaLocInfoPresent;   /**< chroma_loc_info_present_flag in H.264/AVC VUI syntax. */
/**
@verbatim
chroma_loc_info_present_flag in H.264/AVC VUI syntax

@* 1 : ChromaSampleLocTypeTopField and ChromaSampleLoc TypeTopField are valid.
@* 0 : ChromaSampleLocTypeTopField and ChromaSampleLoc TypeTopField are not valid.
@endverbatim
*/
    int colorPrimaries;         /**< colour_primaries syntax in VUI parameter in H.264/AVC */
    int colorDescPresent;       /**< colour_description_present_flag in VUI parameter in H.264/AVC */
    int isExtSAR;               /**< This flag indicates whether aspectRateInfo represents 8bit aspect_ratio_idc or 32bit extended_SAR. If the aspect_ratio_idc is extended_SAR mode, this flag returns 1. */
    int vidFullRange;           /**< video_full_range in VUI parameter in H.264/AVC */
    int vidFormat;              /**< video_format in VUI parameter in H.264/AVC */
    int vidSigTypePresent;      /**< video_signal_type_present_flag in VUI parameter in H.264/AVC */
    int vuiParamPresent;        /**< vui_parameters_present_flag in VUI parameter in H.264/AVC */
    int vuiPicStructPresent;    /**< pic_struct_present_flag of VUI in H.264/AVC. This field is valid only for H.264/AVC decoding. */
    int vuiPicStruct;           /**< pic_struct in H.264/AVC VUI reporting (Table D-1 in H.264/AVC specification) */
} AvcVuiInfo;

/**
* @brief    This is a data structure for bar information of MPEG2 user data.
For more details on this, please refer to 'ATSC Digital Television Standard: Part 4:2009'.
*/

typedef struct {
/**
@verbatim
A 14-bit unsigned integer value representing the last horizontal
luminance sample of a vertical pillarbox bar area at the left side of the reconstructed frame.
Pixels shall be numbered from zero, starting with the leftmost pixel.

This variable is initialized to -1.
@endverbatim
*/
    int barLeft;
/**
@verbatim
A 14-bit unsigned integer value representing the first horizontal
luminance sample of a vertical pillarbox bar area at the right side of the reconstructed frame.
Pixels shall be numbered from zero, starting with the leftmost pixel.

This variable is initialized to -1.
@endverbatim
*/
    int barRight;
/**
@verbatim
A 14-bit unsigned integer value representing the first line of a
horizontal letterbox bar area at the top of the reconstructed frame. Designation of line
numbers shall be as defined per each applicable standard in Table 6.9.

This variable is initialized to -1.
@endverbatim
*/
    int barTop;
/**
@verbatim
A 14-bit unsigned integer value representing the first line of a
horizontal letterbox bar area at the bottom of the reconstructed frame. Designation of line
numbers shall be as defined per each applicable standard in Table 6.9.

This variable is initialized to -1.
@endverbatim
*/
    int barBottom;
} MP2BarDataInfo;

/**
* @brief
@verbatim
This is a data structure for MP2PicDispExtInfo.

NOTE: For detailed information on these fields,
please refer to the MPEG2 standard specification.
@endverbatim
*/
typedef struct {
    Uint32  offsetNum;          /**< This is number of frame_centre_offset with a range of 0 to 3, inclusive. */
    Int16   horizontalOffset1;  /**< A horizontal offset of display rectangle in units of 1/16th sample */
    Int16   horizontalOffset2;  /**< A horizontal offset of display rectangle in units of 1/16th sample */
    Int16   horizontalOffset3;  /**< A horizontal offset of display rectangle in units of 1/16th sample */

    Int16   verticalOffset1;    /**< A vertical offset of display rectangle in units of 1/16th sample */
    Int16   verticalOffset2;    /**< A vertical offset of display rectangle in units of 1/16th sample */
    Int16   verticalOffset3;    /**< A vertical offset of display rectangle in units of 1/16th sample */
} MP2PicDispExtInfo;

/**
* @brief    This data structure is a group of common decoder parameters to run VPU with a new decoding instance.
This is used when HOST application calls VPU_Decopen().
*/
typedef struct {
    CodStd          bitstreamFormat;    /**< A standard type of bitstream in decoder operation. It is one of codec standards defined in CodStd. */
    PhysicalAddress bitstreamBuffer;    /**< The start address of bitstream buffer from which the decoder can get the next bitstream. This address must be aligned to AXI bus width. */
    int             bitstreamBufferSize;/**< The size of the buffer pointed by bitstreamBuffer in byte. This value must be a multiple of 1024.  */
/**
@verbatim
@* 0 : disable
@* 1 : enable

When this field is set in case of MPEG4, H.263 (post-processing), DivX3 or MPEG2 decoding,
VPU generates MPEG4 deblocking filtered output.
@endverbatim
*/
    int             mp4DeblkEnable;
/**
@verbatim
@* 0 : no extension of H.264/AVC
@* 1 : MVC extension of H.264/AVC
@endverbatim
*/
    int             avcExtension;
/**
@verbatim

@* 0 : MPEG4
@* 1 : DivX 5.0 or higher
@* 2 : Xvid
@* 5 : DivX 4.0
@* 6 : old Xvid
@* 256 : Sorenson Spark

NOTE: This variable is only valid when decoding MPEG4 stream.
@endverbatim
*/
    int             mp4Class;
    int             tiled2LinearEnable; /**< It enables a tiled to linear map conversion feature for display. */
/**
@verbatim
It specifies which picture type is converted to. (CODA980 only)

@* 1 : conversion to linear frame map (when FrameFlag enum is FF_FRAME)
@* 2 : conversion to linear field map (when FrameFlag enum is FF_FIELD)

@endverbatim
*/
    int             tiled2LinearMode;
/**
@verbatim
It enables WTL(Write Linear) function. If this field is enabled,
VPU writes a non-linear and a linear format of the decoded frame to the frame buffer at the same time. - first in linear map and second in tiled or compressed map.
Basically, VPU only writes a non-linear format of frame.
If you want to get a linear format of frame for display, WTL should be enabled.
@endverbatim
*/
    int             wtlEnable;
/**
@verbatim
It specifies whether VPU writes in frame linear map or in field linear map when WTL is enabled. (CODA980 only)

@* 1 : write decoded frames in frame linear map (when FrameFlag enum is FF_FRAME)
@* 2 : write decoded frames in field linear map (when FrameFlag enum is FF_FIELD)

@endverbatim
*/
    int             wtlMode;
/**
@verbatim
@* 0 : Cb data are written in Cb frame memory and Cr data are written in Cr frame memory. (chroma separate mode)
@* 1 : Cb and Cr data are written in the same chroma memory. (chroma interleave mode)
@endverbatim
*/
    int             cbcrInterleave;
/**
@verbatim
CrCb interleaved mode (NV21).

@* 0 : decoded chroma data is written in CbCr (NV12) format.
@* 1 : decoded chroma data is written in CrCb (NV21) format.

This is only valid if cbcrInterleave is 1.
@endverbatim
*/
    int             nv21;
/**
@verbatim
CbCr order in planar mode (YV12 format)

@* 0 : Cb data are written first and then Cr written in their separate plane.
@* 1 : Cr data are written first and then Cb written in their separate plane.
@endverbatim
*/
    int             cbcrOrder;
/**
@verbatim
It writes output with 8 burst in linear map mode. (CODA9 only)

@* 0 : burst write back is disabled
@* 1 : burst write back is enabled.

@endverbatim
*/
    int             bwbEnable;
/**
@verbatim
The endianess of the frame buffer for reference and reconstruction

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE Datasheet'.
@endverbatim
*/
    EndianMode      frameEndian;
/**
@verbatim
The endianess of the bitstream buffer

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bits little endian format
@* 3 : 32 bits big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE Datasheet'.
@endverbatim
*/
    EndianMode      streamEndian;
/**
@verbatim
When a read pointer reaches a write pointer in the middle of decoding one picture,

@* 0 : VPU sends an interrupt to HOST application and waits for more bitstream to decode. (interrupt mode)
@* 1 : reserved
@* 2 : VPU decodes bitstream from read pointer to write pointer. (PicEnd mode)
@endverbatim
*/
    BitStreamMode   bitstreamMode;
    Uint32          coreIdx;    /**< VPU core index number (0 ~ [number of VPU core] - 1) */
/**
Work buffer SDRAM address/size information. In parallel decoding operation, work buffer is shared between VPU cores.
The work buffer address is set to this member variable when VPU_Decopen() is called.
Unless HOST application sets the address and size of work buffer, VPU allocates automatically work buffer
when VPU_DecOpen() is executed.
*/
    vpu_buffer_t    vbWork;
    Uint32          virtAxiID; /**< AXI_ID to distinguish guest OS. For virtualization only. Set this value in highest bit order.*/
    BOOL            bwOptimization; /**< Bandwidth optimization feature which allows WTL(Write to Linear)-enabled VPU to skip writing compressed format of non-reference pictures or linear format of non-display pictures to the frame buffer for BW saving reason. */
/**
@verbatim
It sets the selection mode of temporal ID.

@* 0: use the target temporal_id as absolute value.
@** TARGET_DEC_TEMP_ID = TARGET_DEC_TEMP_ID_PLUS1 -1
@** When using an absolute value, decoder can keep the decoding layer ID.
@** If the SPS_MAX_SUB_LAYER is changed in the bitstream, the temporal skip ratio can be changed.

@* 1: use the target temporal_id as relative value
@** TARGET_DEC_TEMP_ID = SPS_MAX_SUB_LAYER - REL_TARGET_TEMP_ID
@** SPS_MAX_SUB_LAYER is signalled from bitstream.
@** When using a relative value, decoder can keep the temporal skip ratio, regardless of the change of SPS_MAX_SUB_LAYER in the bitstream.
@endverbatim
*/
    BOOL            tempIdSelectMode;
/**
@verbatim

@* 0: AV1_STREAM_IVF  AV1 bistream contained by IVF format
@* 1: AV1_STREAM_OBU  OBU syntax bitstream
@* 2: AV1_STREAM_ANNEXB  Length delimited bitstream format

NOTE: This variable is only valid when decoding AV1 stream.
@endverbatim
*/
    int             av1Format;
} DecOpenParam;

/**
* @brief This is a data structure to get information necessary to start decoding from the decoder.
*/

typedef struct {
/**
@verbatim
Horizontal picture size in pixel

This width value is used while allocating decoder frame buffers. In some
cases, this returned value, the display picture width declared on stream header,
should be aligned to a specific value depending on product and video standard before allocating frame buffers.
@endverbatim
*/
    Int32           picWidth;
/**
@verbatim
Vertical picture size in pixel

This height value is used while allocating decoder frame buffers.
In some cases, this returned value, the display picture height declared on stream header,
should be aligned to a specific value depending on product and video standard before allocating frame buffers.
@endverbatim
*/
    Int32           picHeight;

/**
@verbatim
The numerator part of frame rate fraction

NOTE: The meaning of this flag can vary by codec standards.
For details about this,
please refer to 'Appendix: FRAME RATE NUMERATORS in programmer\'s guide'.
@endverbatim
*/
    Int32           fRateNumerator;
/**
@verbatim
The denominator part of frame rate fraction

NOTE: The meaning of this flag can vary by codec standards.
For details about this,
please refer to 'Appendix: FRAME RATE DENOMINATORS in programmer\'s guide'.
@endverbatim
*/
    Int32           fRateDenominator;
/**
@verbatim
Picture cropping rectangle information (H.264/H.265/AVS decoder only)

This structure specifies the cropping rectangle information.
The size and position of cropping window in full frame buffer is presented
by using this structure.
@endverbatim
*/
    VpuRect         picCropRect;
    Int32           mp4DataPartitionEnable; /**< data_partitioned syntax value in MPEG4 VOL header */
    Int32           mp4ReversibleVlcEnable; /**< reversible_vlc syntax value in MPEG4 VOL header */
/**
@verbatim
@* 0 : not h.263 stream
@* 1 : h.263 stream(mpeg4 short video header)
@endverbatim
*/
    Int32           mp4ShortVideoHeader;
/**
@verbatim
@* 0 : Annex J disabled
@* 1 : Annex J (optional deblocking filter mode) enabled
@endverbatim
*/
    Int32           h263AnnexJEnable;
    Int32           minFrameBufferCount;    /**< This is the minimum number of frame buffers required for decoding. Applications must allocate at least as many as this number of frame buffers and register the number of buffers to VPU using VPU_DecRegisterFrameBuffer() before decoding pictures. */
    Int32           frameBufDelay;          /**< This is the maximum display frame buffer delay for buffering decoded picture reorder. VPU may delay decoded picture display for display reordering when H.264/H.265, pic_order_cnt_type 0 or 1 case and for B-frame handling in VC1 decoder. */
    Int32           normalSliceSize;        /**< This is the recommended size of buffer used to save slice in normal case. This value is determined by quarter of the memory size for one raw YUV image in KB unit. This is only for H.264. */
    Int32           worstSliceSize;         /**< This is the recommended size of buffer used to save slice in worst case. This value is determined by half of the memory size for one raw YUV image in KB unit. This is only for H.264. */

    // Report Information
    Int32           maxSubLayers;           /**< Number of temporal sub-layers. */
/**
@verbatim
@* H.265/H.264 : profile_idc
@* VC1
@** 0 : Simple profile
@** 1 : Main profile
@** 2 : Advanced profile
@* MPEG2
@** 3\'b101 : Simple
@** 3\'b100 : Main
@** 3\'b011 : SNR Scalable
@** 3\'b10 : Spatially Scalable
@** 3\'b001 : High
@* MPEG4
@** 8\'b00000000 : SP
@** 8\'b00001111 : ASP
@* Real Video
@** 8 (version 8)
@** 9 (version 9)
@** 10 (version 10)
@* AVS
@** 8\'b0010 0000 : Jizhun profile
@** 8\'b0100 1000 : Guangdian profile
@* AVS2
@** 8\'b0001 0010 : Main picture profile
@** 8\'b0010 0000 : Main profile
@** 8\'b0010 0010 : Main10 profile
@* VC1 : level in struct B
@* VP8 : Profile 0 - 3
@* VP9 : Profile 0 - 3
@endverbatim
*/
    Int32           profile;
/**
@verbatim
@* H.265/H.264 : level_idc
@* VC1 : level
@* MPEG2 :
@** 4\'b1010 : Low
@** 4\'b1000 : Main
@** 4\'b0110 : High 1440,
@** 4\'b0100 : High
@* MPEG4 :
@** SP
@*** 4\'b1000 : L0
@*** 4\'b0001 : L1
@*** 4\'b0010 : L2
@*** 4\'b0011 : L3
@** ASP
@*** 4\'b0000 : L0
@*** 4\'b0001 : L1
@*** 4\'b0010 : L2
@*** 4\'b0011 : L3
@*** 4\'b0100 : L4
@*** 4\'b0101 : L5
@* Real Video : N/A (real video does not have any level info).
@* AVS :
@** 4\'b0000 : L2.0
@** 4\'b0001 : L4.0
@** 4\'b0010 : L4.2
@** 4\'b0011 : L6.0
@** 4\'b0100 : L6.2
@endverbatim
*/
    Int32           level;
/**
@verbatim
A tier indicator

@* 0 : Main
@* 1 : High
@endverbatim
*/
    Int32           tier;
    Int32           interlace;              /**< When this value is 1, decoded stream may be decoded into progressive or interlace frame. Otherwise, decoded stream is progressive frame. */
    Int32           constraint_set_flag[4]; /**< constraint_set0_flag ~ constraint_set3_flag in H.264/AVC SPS */
    Int32           direct8x8Flag;          /**< direct_8x8_inference_flag in H.264/AVC SPS */
    Int32           vc1Psf;                 /**< Progressive Segmented Frame(PSF) in VC1 sequence layer */
    Int32           isExtSAR;               /**< This flag indicates whether aspectRateInfo represents 8bit aspect_ratio_idc or 32bit extended_SAR. If the aspect_ratio_idc is extended_SAR mode, this flag returns 1. */
/**
@verbatim
This is one of the SPS syntax elements in H.264.

@* 0 : max_num_ref_frames is 0.
@* 1 : max_num_ref_frames is not 0.
@endverbatim
*/
    Int32           maxNumRefFrmFlag;
    Int32           maxNumRefFrm;  /**< num_ref_frames syntax value of SPS (CODA9 H.264 decoder only) */
/**
@verbatim
@* H.264/AVC : When avcIsExtSAR is 0, this indicates aspect_ratio_idc[7:0]. When avcIsExtSAR is 1, this indicates sar_width[31:16] and sar_height[15:0].
If aspect_ratio_info_present_flag = 0, the register returns -1 (0xffffffff).
@* VC1 : this reports ASPECT_HORIZ_SIZE[15:8] and ASPECT_VERT_SIZE[7:0].
@* MPEG2 : this value is index of Table 6-3 in ISO/IEC 13818-2.
@* MPEG4/H.263 : this value is index of Table 6-12 in ISO/IEC 14496-2.
@* RV : aspect_ratio_info
@* AVS : this value is the aspect_ratio_info[3:0] which is used as index of Table 7-5 in AVS Part2
@endverbatim
*/
    Int32           aspectRateInfo;
    Int32           bitRate;        /**< The bitrate value written in bitstream syntax. If there is no bitRate, this reports -1. */
    ThoScaleInfo    thoScaleInfo;   /**< This is the Theora picture size information. Refer to <<vpuapi_h_ThoScaleInfo>>. */
    Vp8ScaleInfo    vp8ScaleInfo;   /**< This is VP8 upsampling information. Refer to <<vpuapi_h_Vp8ScaleInfo>>. */
    Int32           mp2LowDelay;    /**< This is low_delay syntax of sequence extension in MPEG2 specification. */
    Int32           mp2DispVerSize; /**< This is display_vertical_size syntax of sequence display extension in MPEG2 specification. */
    Int32           mp2DispHorSize; /**< This is display_horizontal_size syntax of sequence display extension in MPEG2 specification. */
    Uint32          userDataHeader; /**< Refer to userDataHeader in <<vpuapi_h_DecOutputExtData>>. */
    Int32           userDataNum;    /**< Refer to userDataNum in <<vpuapi_h_DecOutputExtData>>. */
    Int32           userDataSize;   /**< Refer to userDataSize in <<vpuapi_h_DecOutputExtData>>. */
    Int32           userDataBufFull;/**< Refer to userDataBufFull in <<vpuapi_h_DecOutputExtData>>. */
    //VUI information
    Int32           chromaFormatIDC;/**< A chroma format indicator */
    Int32           lumaBitdepth;   /**< A bit-depth of luma sample */
    Int32           chromaBitdepth; /**< A bit-depth of chroma sample */
/**
@verbatim
This is an error reason of sequence header decoding.
For detailed meaning of returned value,
please refer to the 'Appendix: ERROR DEFINITION in programmer\'s guide'.
@endverbatim
*/
    Uint32           seqInitErrReason;
/**
@verbatim
This is warning information of sequence header decoding.
For detailed meaning of returned value,
please refer to the 'Appendix: ERROR DEFINITION in programmer\'s guide'.
@endverbatim
*/
    Int32           warnInfo;
    PhysicalAddress rdPtr;         /**< A read pointer of bitstream buffer */
    PhysicalAddress wrPtr;         /**< A write pointer of bitstream buffer */
    AvcVuiInfo      avcVuiInfo;    /**< This is H.264/AVC VUI information. Refer to <<vpuapi_h_AvcVuiInfo>>. */
    MP2BarDataInfo  mp2BardataInfo;/**< This is bar information in MPEG2 user data. For details about this, please see the document 'ATSC Digital Television Standard: Part 4:2009'. */
    Uint32          sequenceNo;    /**< This is the number of sequence information. This variable is increased by 1 when VPU detects change of sequence. */
/**
@verbatim
This is an spatial SVC flag.

@* 0 : Non-SVC stream
@* 1 : SVC stream
@endverbatim
*/
    Int32           spatialSvcEnable;
/**
@verbatim
This is an spatial SVC mode.

@* 0 : disable inter-layer prediction (simulcast).
@* 1 : enable inter-layer prediction in which EL picture is predicted with motion vector information from the base layer picture.
@endverbatim
*/
    Int32           spatialSvcMode;
/**
@verbatim
This is an output bit-depth. This is only for AVS2

@* 8  : output bit-depth is 8.
@* 10 : output bit-depth is 10.
@endverbatim
*/
    Uint32          outputBitDepth;
    Uint32          vlcBufSize; /**< The size of task buffer */
    Uint32          paramBufSize; /**< The size of task buffer */
} DecInitialInfo;

// Report Information

#define WAVE_SKIPMODE_WAVE_NONE 0
#define WAVE_SKIPMODE_NON_IRAP  1
#define WAVE_SKIPMODE_NON_REF   2


#define SEL_SVAC_ALL_LAYER      0
#define SEL_SVAC_BL             1
#define SEL_SVAC_EL             2

/**
* @brief    The data structure for options of picture decoding.
*/
typedef struct {
/**
@verbatim
@* 0 : disable
@* 1 : enable
@* 2 : I frame search enable (H.264/AVC only)

If this option is enabled, then decoder performs skipping frame decoding until
decoder meets an I (IDR) frame. If there is no I frame in given stream, decoder
waits for I (IDR) frame.
Especially in H.264/AVC stream decoding, they might have I-frame and IDR frame.
Therefore HOST application should set iframeSearchEnable value according to frame type.
If HOST application wants to search IDR frame, this flag should be set to 1 like other standards.
Otherwise if HOST application wants to search I frame, this flag should be set to 2.

Note that when decoder meets EOS (End Of Sequence) code during I-Search, decoder
returns -1 (0xFFFF). And if this option is enabled,
skipframeMode options are ignored.

NOTE: CODA9 only supports it.
@endverbatim
*/
    Int32 iframeSearchEnable;
/**
@verbatim
Skip frame function enable and operation mode

In case of CODA9,

@* 0 : skip frame disable
@* 1 : skip frames except I (IDR) frames
@* 2 : skip non-reference frames

After the skip, decoder returns -2 (0xFFFE) of the decoded index when decoder does not have any
frames displayed.

In case of WAVE5,

@* 0x0 : skip frame off
@* 0x1 : skip non-RAP pictures (skip any picture which is neither IDR, CRA, nor BLA).
@* 0x2 : skip non-reference pictures
@* 0x3 : reserved
@* 0x4~0xf : reserved

NOTE: When decoder meets EOS (End Of Sequence) code during frame skip,
decoder returns -1(0xFFFF).
@endverbatim
*/
    Int32 skipframeMode;
/**
@verbatim
Selects which layer of SVAC spatial SVC stream is decoded.

@* 0: decodes both BL(Base Layer) and EL(Enhanced Layer) picture.
@* 1: decodes BL picture.
@* 2: decodes EL picture.
@endverbatim
*/
    Int32 selSvacLayer;
    union {
/**
@verbatim
Forces to flush a display index of the frame buffer that
delayed without decoding of the current picture.

@* 0 : disable
@* 1 : enable flushing
@endverbatim
*/
        Int32 mp2PicFlush;
/**
@verbatim
Sets a de-blocking filter mode for RV streams.

@* 0 : enable de-blocking filter for all pictures.
@* 1 : disable de-blocking filter for all pictures.
@* 2 : disable de-blocking filter for P and B pictures.
@* 3 : disable de-blocking filter only for B pictures.
@endverbatim
*/
        Int32 rvDbkMode;
    } DecStdParam;

    BOOL    craAsBlaFlag;   /**< It handles CRA picture as BLA picture not to use reference from the previous decoded pictures (H.265/HEVC only) */

    Uint32  qosEnable;      /**< It updates QoS control information. To update QoS values, qosEnable must be 1. */
    Uint32  qosConfig;      /**< It enables register based QoS control. If register based QoS control feature is enabled, QoS for AXI is set by given qosValVcore0 and qosValVcore1 respectively. */
    Uint32  qosValVcore0;   /**< Qos value for VCORE0 */
    Uint32  qosValVcore1;   /**< Qos value for VCORE1 */
} DecParam;

// Report Information
/**
* @brief    The data structure to get result information from decoding a frame.
*/

typedef struct {
/**
@verbatim
This variable indicates which userdata is reported by VPU. (WAVE only)
When this variable is not zero, each bit corresponds to the `H265_USERDATA_FLAG_XXX`.

 // H265 USER_DATA(SPS & SEI) ENABLE FLAG
 #define H265_USERDATA_FLAG_RESERVED_0           (0)
 #define H265_USERDATA_FLAG_RESERVED_1           (1)
 #define H265_USERDATA_FLAG_VUI                  (2)
 #define H265_USERDATA_FLAG_RESERVED_3           (3)
 #define H265_USERDATA_FLAG_PIC_TIMING           (4)
 #define H265_USERDATA_FLAG_ITU_T_T35_PRE        (5)
 #define H265_USERDATA_FLAG_UNREGISTERED_PRE     (6)
 #define H265_USERDATA_FLAG_ITU_T_T35_SUF        (7)
 #define H265_USERDATA_FLAG_UNREGISTERED_SUF     (8)
 #define H265_USERDATA_FLAG_RECOVERY_POINT       (9)
 #define H265_USERDATA_FLAG_MASTERING_COLOR_VOL  (10)
 #define H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT  (11)
 #define H265_USERDATA_FLAG_KNEE_FUNCTION_INFO   (12)

Userdata are written from the memory address specified to SET_ADDR_REP_USERDATA,
and userdata consists of two parts, header (offset and size) and userdata as shown below.

 -------------------------------------
 | offset_00(32bit) | size_00(32bit) |
 | offset_01(32bit) | size_01(32bit) |
 |                  ...              | header
 |                  ...              |
 | offset_31(32bit) | size_31(32bit) |
 -------------------------------------
 |                                   | data
 |                                   |

@endverbatim
*/
    Uint32          userDataHeader;
    Uint32          userDataNum;        /**< This is the number of user data. */
    Uint32          userDataSize;       /**< This is the size of user data. */
    Uint32          userDataBufFull;    /**< When userDataEnable is enabled, decoder reports frame buffer status into the userDataBufAddr and userDataSize in byte size. When user data report mode is 1 and the user data size is bigger than the user data buffer size, VPU reports user data as much as buffer size, skips the remainings and sets userDataBufFull. */
    Uint32          activeFormat;       /**< active_format (4bit syntax value) in AFD user data. The default value is 0000b. This is valid only for H.264/AVC and MPEG2 stream. */
} DecOutputExtData;

// VP8 specific header information
/**
* @brief    This is a data structure for VP8 specific hearder information and reference frame indices.
Only VP8 decoder returns this structure after decoding a frame.
*/
typedef struct {
    unsigned showFrame      : 1;    /**< This flag is the frame header syntax, meaning whether the current decoded frame is displayable or not. It is 0 when the current frame is not for display, and 1 when the current frame is for display. */
    unsigned versionNumber  : 3;    /**< This is the VP8 profile version number information in the frame header. The version number enables or disables certain features in bitstream. It can be defined with one of the four different profiles, 0 to 3 and each of them indicates different decoding complexity. */
    unsigned refIdxLast     : 8;    /**< This is the frame buffer index for the Last reference frame. This field is valid only for next inter frame decoding. */
    unsigned refIdxAltr     : 8;    /**< This is the frame buffer index for the altref(Alternative Reference) reference frame. This field is valid only for next inter frame decoding. */
    unsigned refIdxGold     : 8;    /**< This is the frame buffer index for the Golden reference frame. This field is valid only for next inter frame decoding. */
} Vp8PicInfo;

// MVC specific picture information
/**
* @brief    This is a data structure for MVC specific picture information. Only MVC decoder returns this structure after decoding a frame.
*/
typedef struct {
    int viewIdxDisplay; /**< This is view index order of display frame buffer corresponding to indexFrameDisplay of <<vpuapi_h_DecOutputInfo>> structure. */
    int viewIdxDecoded; /**< This is view index order of decoded frame buffer corresponding to indexFrameDecoded of <<vpuapi_h_DecOutputInfo>> structure. */
} MvcPicInfo;

// AVC specific SEI information (frame packing arrangement SEI)
/**
* @brief    This is a data structure for H.264/AVC FPA(Frame Packing Arrangement) SEI.
For detailed information, refer to 'ISO/IEC 14496-10 D.2.25 Frame packing arrangement SEI message semantics'.
*/
typedef struct {
/**
@verbatim
This is a flag to indicate whether H.264/AVC FPA SEI exists or not.

@* 0 : H.264/AVC FPA SEI does not exist.
@* 1 : H.264/AVC FPA SEI exists.
@endverbatim
*/
    unsigned exist;
    unsigned framePackingArrangementId;         /**< 0 ~ 2^32-1 : An identifying number that may be used to identify the usage of the frame packing arrangement SEI message. */
    unsigned framePackingArrangementCancelFlag; /**< 1 indicates that the frame packing arrangement SEI message cancels the persistence of any previous frame packing arrangement SEI message in output order. */
    unsigned quincunxSamplingFlag;              /**< It indicates whether each color component plane of each constituent frame is quincunx sampled.  */
    unsigned spatialFlippingFlag;               /**< It indicates that one of the two constituent frames is spatially flipped.  */
    unsigned frame0FlippedFlag;                 /**< It indicates which one of the two constituent frames is flipped. */
    unsigned fieldViewsFlag;                    /**< 1 indicates that all pictures in the current coded video sequence are coded as complementary field pairs.  */
    unsigned currentFrameIsFrame0Flag;          /**< It indicates the current decoded frame and the next decoded frame in output order.  */
    unsigned frame0SelfContainedFlag;           /**< It indicates whether inter prediction operations within the decoding process for the samples of constituent frame 0  of the coded video sequence refer to samples of any constituent frame 1.  */
    unsigned frame1SelfContainedFlag;           /**< It indicates whether inter prediction operations within the decoding process for the samples of constituent frame 1 of the coded video sequence refer to samples of any constituent frame 0.  */
    unsigned framePackingArrangementExtensionFlag;  /**< 0 indicates that no additional data follows within the frame packing arrangement SEI message.  */
    unsigned framePackingArrangementType;       /**< The type of packing arrangement of the frames */
    unsigned contentInterpretationType;         /**< It indicates the intended interpretation of the constituent frames. */
    unsigned frame0GridPositionX;               /**< It specifies the horizontal location of the upper left sample of constituent frame 0 to the right of the spatial reference point.  */
    unsigned frame0GridPositionY;               /**< It specifies the vertical location of the upper left sample of constituent frame 0 below the spatial reference point.  */
    unsigned frame1GridPositionX;               /**< It specifies the horizontal location of the upper left sample of constituent frame 1 to the right of the spatial reference point.  */
    unsigned frame1GridPositionY;               /**< It specifies the vertical location of the upper left sample of constituent frame 1 below the spatial reference point.  */
    unsigned framePackingArrangementRepetitionPeriod;   /**< It indicates persistence of the frame packing arrangement SEI message.  */
} AvcFpaSei;

// H.264/AVC specific HRD information
/**
* @brief    This is a data structure for H.264/AVC specific picture information. (H.264/AVC decoder only)
VPU returns this structure after decoding a frame. For detailed information, refer to 'ISO/IEC 14496-10 E.1 VUI syntax'.
*/
typedef struct {
    int cpbMinus1;          /**< It indicates cpb_cnt_minus1 syntax. */
    int vclHrdParamFlag;    /**< It indicates vcl_hrd_parameters_present_flag syntax. */
    int nalHrdParamFlag;    /**< It indicates nal_hrd_parameters_present_flag syntax. */
} AvcHrdInfo;

// H.264/AVC specific Recovery Point information
/**
* @brief    This is a data structure for H.264/AVC specific picture information. (H.264/AVC decoder only)
VPU returns this structure after decoding a frame. For detailed information, refer to 'ISO/IEC 14496-10 D.1.7 Recovery point SEI message syntax'.
*/
typedef struct {
/**
@verbatim
This is a flag to indicate whether H.264/AVC RP SEI exists or not.

@* 0 : H.264/AVC RP SEI does not exist.
@* 1 : H.264/AVC RP SEI exists.
@endverbatim
*/
    unsigned exist;
    int recoveryFrameCnt;       /**< It indicates recovery_frame_cnt syntax. */
    int exactMatchFlag;         /**< It indicates exact_match_flag syntax. */
    int brokenLinkFlag;         /**< It indicates broken_link_flag syntax. */
    int changingSliceGroupIdc;  /**< It indicates changing_slice_group_idc syntax. */
} AvcRpSei;

// HEVC specific Recovery Point information
/**
* @brief    This is a data structure for H.265/HEVC specific picture information. (H.265/HEVC decoder only)
VPU returns this structure after decoding a frame.
*/
typedef struct {
/**
@verbatim
This is a flag to indicate whether H.265/HEVC Recovery Point SEI exists or not.

@* 0 : H.265/HEVC RP SEI does not exist.
@* 1 : H.265/HEVC RP SEI exists.
@endverbatim
*/
    unsigned exist;
    int recoveryPocCnt;         /**< recovery_poc_cnt */
    int exactMatchFlag;         /**< exact_match_flag */
    int brokenLinkFlag;         /**< broken_link_flag */
} H265RpSei;


/**
* @brief    This is a data structure of spatial scalable video coding information such as SVC layer and SVC mode (SVAC only)
*/
typedef struct {
/**
@verbatim
This is the spatial SVC mode.

@* 0 : disable inter-layer prediction. (simulcast)
@* 1 : enable inter-layer prediction in which EL picture is predicted with motion vector information from the base layer picture.
@endverbatim
*/
    int spatialSvcMode;
/**
@verbatim
This is the layer information of SVC stream.

@* 0 : base layer picture
@* 1 : enhance layer picture
@endverbatim
*/
    int spatialSvcLayer;
    int spatialSvcFlag; /**< An spatial SVC flag */
} SvacInfo;

/**
* @brief    This is a data structure for AVS2 specific picture information.
*/
typedef struct {
    int decodedPOI;  /**< A POI value of picture that has currently been decoded and with decoded index. When indexFrameDecoded is -1, it returns -1. */
    int displayPOI;  /**< A POI value of picture with display index. When indexFrameDisplay is -1, it returns -1. */
} Avs2Info;

/**
* @brief    This is a data structure for AV1 specific picture information.
*/
typedef struct {
    int enableScreenContents; /**< It indicates whether screen content tool is enabled. */
    int enableIntraBlockCopy; /**< It indicates whether intra block copy is enabled. */
} Av1Info;

/**
* @brief    The data structure to get result information from decoding a frame.
*/
typedef struct {
/**
@verbatim
This is a frame buffer index for the picture to be displayed at the moment among
frame buffers which are registered using VPU_DecRegisterFrameBuffer(). Frame
data to be displayed are stored into the frame buffer with this index.
When there is no display delay, this index is always
the same with indexFrameDecoded. However, if display delay does exist for display reordering in AVC
or B-frames in VC1), this index might be different with indexFrameDecoded.
By checking this index, HOST application can easily know whether sequence decoding has been finished or not.

@* -3(0xFFFD) or -2(0xFFFE) : it is when a display output cannot be given due to picture reordering or skip option.
@* -1(0xFFFF) : it is when there is no more output for display at the end of sequence decoding.

@endverbatim
*/
    int indexFrameDisplay;
    int indexFrameDisplayForTiled;  /**< In case of WTL mode, this index indicates a display index of tiled or compressed framebuffer. */
/**
@verbatim
This is a frame buffer index of decoded picture among frame buffers which were
registered using VPU_DecRegisterFrameBuffer(). The currently decoded frame is stored into the frame buffer specified by
this index.

* -2 : it indicates that no decoded output is generated because decoder meets EOS (End Of Sequence) or skip.
* -1 : it indicates that decoder fails to decode a picture because there is no available frame buffer.
@endverbatim
*/
    int indexFrameDecoded;
    int indexInterFrameDecoded;     /**< In case of VP9 codec, this indicates an index of the frame buffer to reallocate for the next frame's decoding. VPU returns this information when detecting change of the inter-frame resolution. */
    int indexFrameDecodedForTiled;  /**< In case of WTL mode, this indicates a decoded index of tiled or compressed framebuffer. */
    int nalType;                    /**< This is nal Type of decoded picture. Please refer to nal_unit_type in Table 7-1 - NAL unit type codes and NAL unit type classes in H.265/HEVC specification. (WAVE only) */
    int picType;                    /**< This is the picture type of decoded picture. It reports the picture type of bottom field for interlaced stream. <<vpuapi_h_PicType>>. */
    int picTypeFirst;               /**< This is only valid in interlaced mode and indicates the picture type of the top field. */
    int numOfErrMBs;                /**< This is the total number of error coded unit(MB/CTU) in the decoded picture. */
    int numOfTotMBs;                /**< This is the total number of coded unit(MB/CTU) in the decoded picture. */
    int numOfErrMBsInDisplay;       /**< This is the total number of error coded unit(MB/CTU) in the display picture of indexFrameDisplay. */
    int numOfTotMBsInDisplay;       /**< This is the total number of coded unit(MB/CTU) in the display picture of indexFrameDisplay. In normal stream, numOfTotMBs and numOfTotMBsInDisplay always have the same value. However, in case of sequence change stream(resolution change), they might be different. */
    BOOL refMissingFrameFlag;       /**< This indicates that the current frame's references are missing in decoding.*/
    int notSufficientSliceBuffer;   /**< This is a flag which represents whether slice save buffer is not sufficient to decode the current picture. VPU might not get the last part of the current picture stream due to buffer overflow, which leads to macroblock errors. HOST application can continue decoding the remaining pictures of the current bitstream without closing the current instance, even though several pictures could be error-corrupted. (H.264/AVC BP only) */
    int notSufficientPsBuffer;      /**< This is a flag which represents whether PS (SPS/PPS) save buffer is not sufficient to decode the current picture. VPU might not get the last part of the current picture stream due to buffer overflow. HOST application must close the current instance, since the following picture streams cannot be decoded properly for loss of SPS/PPS data. (H.264/AVC only) */
/**
@verbatim
This variable indicates whether decoding process was finished completely or not. If stream
has error in the picture header syntax or has the first slice header syntax of H.264/AVC
stream, VPU returns 0 without proceeding MB decode routine.

@* 0 : it indicates incomplete finish of decoding process.
@* 1 : it indicates complete finish of decoding process.

@endverbatim
*/
    Uint32 decodingSuccess;
/**
@verbatim
@* 0 : progressive frame which consists of one picture
@* 1 : interlaced frame which consists of two fields
@endverbatim
*/
    int interlacedFrame;
/**
@verbatim
This is a flag which represents whether chunk in bitstream buffer should be reused or not, even after VPU_DecStartOneFrame() is executed.
This flag is meaningful when bitstream buffer operates in PicEnd mode. In that mode, VPU consumes all the bitstream in bitstream buffer for the current VPU_DecStartOneFrame()
in assumption that one chunk is one frame.
However, there might be a few cases that chunk needs to be reused such as the following:

* DivX or XivD stream : One chunk can contain P frame and B frame to reduce display delay.
In that case after decoding P frame, this flag is set to 1. HOST application should try decoding with the rest of chunk data to get B frame.
* H.264/AVC NPF stream : After the first field has been decoded, this flag is set to 1. HOST application should check if the next field is NPF or not.
* No DPB available: VPU is not able to consume chunk with no frame buffers available at the moment. Thus, the whole chunk should be provided again.
@endverbatim
*/
    int chunkReuseRequired;
    VpuRect rcDisplay;              /**< This field reports the rectangular region in pixel unit after decoding one frame - the region of `indexFrameDisplay` frame buffer. */
    int dispPicWidth;               /**< This field reports the width of a picture to be displayed in pixel unit after decoding one frame - width of `indexFrameDisplay` frame bufffer.  */
    int dispPicHeight;              /**< This field reports the height of a picture to be displayed in pixel unit after decoding one frame - height of `indexFrameDisplay` frame bufffer.  */
    VpuRect rcDecoded;              /**< This field reports the rectangular region in pixel unit after decoding one frame - the region of `indexFrameDecoded` frame buffer. */
    int decPicWidth;                /**< This field reports the width of a decoded picture in pixel unit after decoding one frame - width of `indexFrameDecoded` frame bufffer. */
    int decPicHeight;               /**< This field reports the height of a decoded picture in pixel unit after decoding one frame - height of `indexFrameDecoded` frame bufffer.  */
    int aspectRateInfo;             /**< This is aspect ratio information for each standard. Refer to aspectRateInfo of <<vpuapi_h_DecInitialInfo>>. */
    int fRateNumerator;             /**< The numerator part of frame rate fraction. Note that the meaning of this flag can vary by codec standards. For details about this, please refer to 'Appendix: FRAME RATE NUMERATORS in programmer\'s guide'.  */
    int fRateDenominator;           /**< The denominator part of frame rate fraction. Note that the meaning of this flag can vary by codec standards. For details about this, please refer to 'Appendix: FRAME RATE DENOMINATORS in programmer\'s guide'.  */
    Vp8ScaleInfo vp8ScaleInfo;      /**< This is VP8 upsampling information. Refer to <<vpuapi_h_Vp8ScaleInfo>>. */
    Vp8PicInfo vp8PicInfo;          /**< This is VP8 frame header information. Refer to <<vpuapi_h_Vp8PicInfo>>. */
    MvcPicInfo mvcPicInfo;          /**< This is MVC related picture information. Refer to <<vpuapi_h_MvcPicInfo>>. */
    AvcFpaSei avcFpaSei;            /**< This is H.264/AVC frame packing arrangement SEI information. Refer to <<vpuapi_h_AvcFpaSei>>. */
    AvcHrdInfo avcHrdInfo;          /**< This is H.264/AVC HRD information. Refer to <<vpuapi_h_AvcHrdInfo>>. */
    AvcVuiInfo avcVuiInfo;          /**< This is H.264/AVC VUI information. Refer to <<vpuapi_h_AvcVuiInfo>>. */
    SvacInfo svacInfo;              /**< This field reports the information of SVC stream. Refer to <<vpuapi_h_SvacInfo>>. */
    Avs2Info avs2Info;              /**< This is AVS2 picture information. Refer to <<vpuapi_h_Avs2Info>>. */
    Av1Info  av1Info;               /**< This is AVS1 picture information. Refer to <<vpuapi_h_Av1Info>>. */
    int decodedPOC;                 /**< A POC value of picture that has currently been decoded and with decoded index. When indexFrameDecoded is -1, it returns -1. */
    int displayPOC;                 /**< A POC value of picture with display index. When indexFrameDisplay is -1, it returns -1. */
    int temporalId;                 /**< A temporal ID of the picture */
/**
@verbatim
This field is valid only for VC1 decoding. Field information of display frame index
is returned on `indexFrameDisplay`.

@* 0 : paired fields
@* 1 : bottom (top-field missing)
@* 2 : top (bottom-field missing)
@endverbatim
*/
    int vc1NpfFieldInfo;
    int mp2DispVerSize;             /**< This is display_vertical_size syntax of sequence display extension in MPEG2 specification. */
    int mp2DispHorSize;             /**< This is display_horizontal_size syntax of sequence display extension in MPEG2 specification. */
/**
@verbatim
This field is valid only for MPEG2 decoding. Field information of display frame index
is returned on `indexFrameDisplay`.

@* 0 : paired fields
@* 1 : bottom (top-field missing)
@* 2 : top (bottom-field missing)
@endverbatim
*/
    int mp2NpfFieldInfo;
    MP2BarDataInfo mp2BardataInfo;  /**< This is bar information in MPEG2 user data. For details about this, please see the document 'ATSC Digital Television Standard: Part 4:2009'. */
    MP2PicDispExtInfo mp2PicDispExtInfo; /**< For meaning of each field, please see <<vpuapi_h_MP2PicDispExtInfo>>. */
    AvcRpSei avcRpSei;              /**< This is H.264/AVC recovery point SEI information. Refer to <<vpuapi_h_AvcRpSei>>. */
    H265RpSei h265RpSei;            /**< This is H.265/HEVC recovery point SEI information. Refer to <<vpuapi_h_H265RpSei>>. */
/**
@verbatim
This field is valid only for H.264/AVC decoding.
Field information of display frame index is returned on `indexFrameDisplay`.
Refer to the <<vpuapi_h_AvcNpfFieldInfo>>.

@* 0 : paired fields
@* 1 : bottom (top-field missing)
@* 2 : top (bottom-field missing)
@endverbatim
*/
    int avcNpfFieldInfo;
    int avcPocPic;  /**< This field reports the POC value of frame picture in case of H.264/AVC decoding. */
    int avcPocTop;  /**< This field reports the POC value of top field picture in case of H.264/AVC decoding. */
    int avcPocBot;  /**< This field reports the POC value of bottom field picture in case of H.264/AVC decoding. */
    // Report Information
/**
@verbatim
This variable indicates that the decoded picture is progressive or interlaced
picture. The value of pictureStructure is used as below.

@* H.264/AVC : MBAFF
@* VC1 : FCM
@** 0 : progressive
@** 2 : frame interlace
@** 3 : field interlaced
@* MPEG2 : picture structure
@** 1 : top field
@** 2 : bottom field
@** 3 : frame
@* MPEG4 : N/A
@* Real Video : N/A
@* H.265/HEVC : N/A
@endverbatim
*/
    int pictureStructure;
/**
@verbatim
For decoded picture consisting of two fields, this variable reports

@* 0 : VPU decodes the bottom field and then top field.
@* 1 : VPU decodes the top field and then bottom field.

Regardless of this variable, VPU writes the decoded image of top field picture at each odd line and the decoded image of bottom field picture at each even line in frame buffer.
@endverbatim
*/
    int topFieldFirst;
    int repeatFirstField;   /**< This variable indicates Repeat First Field that repeats to display the first field. This flag is valid for VC1, AVS, and MPEG2. */
    int progressiveFrame;   /**< This variable indicates progressive_frame in MPEG2 picture coding extention or in AVS picture header. In the case of VC1, this variable means RPTFRM  (Repeat Frame Count), which is used during display process. */
    int fieldSequence;      /**< This variable indicates field_sequence in picture coding extention in MPEG2. */
    int frameDct;           /**< This variable indicates frame_pred_frame_dct in sequence extension of MPEG2. */
    int nalRefIdc;          /**< This variable indicates if the currently decoded frame is a reference frame or not. This flag is valid for H.264/AVC only.   */
 /**
@verbatim
@* H.264/AVC, MPEG2, and VC1
@** 0 : the decoded frame has paired fields.
@** 1 : the decoded frame has a top-field missing.
@** 2 : the decoded frame has a bottom-field missing.
@endverbatim
*/
    int decFrameInfo;
    int picStrPresent;      /**< It indicates pic_struct_present_flag in H.264/AVC pic_timing SEI. */
    int picTimingStruct;    /**< It indicates pic_struct in H.264/AVC pic_timing SEI reporting. (Table D-1 in H.264/AVC specification.) If pic_timing SEI is not presented, pic_struct is inferred by the D.2.1. pic_struct part in H.264/AVC specification. This field is valid only for H.264/AVC decoding. */
    int progressiveSequence;/**< It indicates progressive_sequence in sequence extension of MPEG2. */
    int mp4TimeIncrement;   /**< It indicates vop_time_increment_resolution in MPEG4 VOP syntax. */
    int mp4ModuloTimeBase;  /**< It indicates modulo_time_base in MPEG4 VOP syntax. */
    DecOutputExtData decOutputExtData;  /**< The data structure to get additional information about a decoded frame. Refer to <<vpuapi_h_DecOutputExtData>>. */
    int consumedByte;       /**< The number of bytes that are consumed by VPU. */
    int rdPtr;              /**< A stream buffer read pointer for the current decoder instance */
    int wrPtr;              /**< A stream buffer write pointer for the current decoder instance */
/**
@verbatim
The start byte position of the current frame after decoding the frame for audio-to-video synchronization

H.265/HEVC or H.264/AVC decoder seeks only 3-byte start code
(0x000001) while other decoders seek 4-byte start code(0x00000001).
@endverbatim
*/
    PhysicalAddress bytePosFrameStart;
    PhysicalAddress bytePosFrameEnd;    /**< It indicates the end byte position of the current frame after decoding. This information helps audio-to-video synchronization. */
    FrameBuffer dispFrame;              /**< It indicates the display frame buffer address and information. Refer to <<vpuapi_h_FrameBuffer>>. */
    int frameDisplayFlag;               /**< It reports a frame buffer flag to be displayed.  */
/**
@verbatim
This variable reports that sequence has been changed while H.264/AVC stream decoding.
If it is 1, HOST application can get the new sequence information by calling VPU_DecGetInitialInfo() or VPU_DecIssueSeqInit().

For H.265/HEVC decoder, each bit has a different meaning as follows.

@* sequenceChanged[5] : it indicates that the profile_idc has been changed.
@* sequenceChanged[16] : it indicates that the resolution has been changed.
@* sequenceChanged[19] : it indicates that the required number of frame buffer has been changed.
@endverbatim
*/
    int sequenceChanged;
    // CODA9: [0]   1 - sequence changed
    // WAVEX: [5]   1 - H.265 profile changed
    //        [16]  1 - resolution changed
    //        [19]  1 - number of DPB changed

    int streamEndFlag;  /**< This variable reports the status of `end of stream` flag. This information can be used for low delay decoding (CODA980 only). */
    int frameCycle;     /**< This variable reports the number of cycles for processing a frame. */
    int errorReason;    /**< This variable reports the error reason that occurs while decoding. For error description, please find the 'Appendix: Error Definition' in the Programmer's Guide. */
    Uint32 errorReasonExt; /**< This variable reports the specific reason of error. For error description, please find the 'Appendix: Error Definition' in the Programmer's Guide. (WAVE only) */
    int warnInfo;       /**< This variable reports the warning information that occurs while decoding. For warning description, please find the 'Appendix: Error Definition' in the Programmer's Guide. */
    Uint32 sequenceNo;  /**< This variable increases by 1 whenever sequence changes. If it happens, HOST should call VPU_DecFrameBufferFlush() to get the decoded result that remains in the buffer in the form of DecOutputInfo array. HOST can recognize with this variable whether this frame is in the current sequence or in the previous sequence when it is displayed. (WAVE only) */
    int rvTr;           /**< This variable reports RV timestamp for Ref frame. */
    int rvTrB;          /**< This variable reports RV timestamp for B frame. */

/**
@verbatim
This variable reports the result of pre-scan which is the start of decoding routine for DEC_PIC command. (WAVE only)
In the prescan phase, VPU parses bitstream and pre-allocates frame buffers.

@* -2 : it is when VPU prescanned bitstream(bitstream consumed), but a decode buffer was not allocated for the bitstream during pre-scan, since there was only header information.
@* -1 : it is when VPU detected full of framebuffer while pre-scannig (bitstream not consumed).
@* >= 0 : it indicates that prescan has been successfully done. This index is returned to a decoded index for the next decoding.
@endverbatim
*/
    int indexFramePrescan;
    Uint32  decHostCmdTick;     /**< Tick of DEC_PIC command for the picture */
    Uint32  decSeekStartTick;   /**< Start tick of seeking slices of the picture */
    Uint32  decSeekEndTick;     /**< End tick of seeking slices of the picture */
    Uint32  decParseStartTick;  /**< Start tick of parsing slices of the picture */
    Uint32  decParseEndTick;    /**< End tick of parsing slices of the picture */
    Uint32  decDecodeStartTick; /**< Start tick of decoding slices of the picture */
    Uint32  decDecodeEndTick;   /**< End tick of decoding slices of the picture */

    Uint32  seekCycle;    /**< The number of cycles for seeking slices of a picture in the internal pipeline. (Theoretically, framecycle = seekCycle + parseCycle + decodedCycle) */
    Uint32  parseCycle;   /**< The number of cycles for parsing slices of a picture in the internal pipeline. (Theoretically, framecycle = seekCycle + parseCycle + decodedCycle) */
    Uint32  DecodedCycle; /**< The number of cycles for decoding slices of a picture in the internal pipeline. (Theoretically, framecycle = seekCycle + parseCycle + decodedCycle) */

/**
@verbatim
A CTU size (only for WAVE series)

@* 16 : CTU16x16
@* 32 : CTU32x32
@* 64 : CTU64x64
@endverbatim
*/
    Int32 ctuSize;
    Int32 outputFlag;          /**< This variable reports the value of pic_output_flag syntax in HEVC slice_segment_header. (WAVE5 only) */
} DecOutputInfo;

/**
 * @brief   This is a data structure of frame buffer information. It is used for parameter when host issues DEC_GET_FRAMEBUF_INFO of <<vpuapi_h_VPU_DecGiveCommand>>.
 */
 typedef struct {
    vpu_buffer_t  vbFrame;          /**< The information of frame buffer where compressed frame is saved  */
    vpu_buffer_t  vbWTL;            /**< The information of frame buffer where decoded, uncompressed frame is saved with linear format if WTL is on   */
    vpu_buffer_t  vbFbcYTbl[MAX_REG_FRAME];        /**< The information of frame buffer to save luma offset table of compressed frame  */
    vpu_buffer_t  vbFbcCTbl[MAX_REG_FRAME];        /**< The information of frame buffer to save chroma offset table of compressed frame */
    vpu_buffer_t  vbMvCol[MAX_REG_FRAME];          /**< The information of frame buffer to save co-located motion vector buffer */
    vpu_buffer_t  vbTask;   /**< The information of task buffer used for command queue */
    FrameBuffer   framebufPool[64]; /**< This is an array of <<vpuapi_h_FrameBuffer>> which contains the information of each frame buffer. When WTL is enabled, the number of framebufPool would be [number of compressed frame buffer] x 2, and the starting index of frame buffer for WTL is framebufPool[number of compressed frame buffer].  */
} DecGetFramebufInfo;

/**
 * @brief   This is a data structure of queue command information. It is used for parameter when host issues DEC_GET_QUEUE_STATUS of <<vpuapi_h_VPU_DecGiveCommand>>. (WAVE5 only)
 */
typedef struct {
    Uint32  instanceQueueCount; /**< This variable indicates the number of queued commands of the instance.  */
    Uint32  reportQueueCount;    /**< This variable indicates the number of report queued commands of all instances.  */
} QueueStatusInfo;

//------------------------------------------------------------------------------
// encode struct and definition
//------------------------------------------------------------------------------



#define MAX_NUM_TEMPORAL_LAYER          7
#define MAX_GOP_NUM                     8

/**
* @brief
@verbatim
This is a dedicated type for encoder handle returned when an encoder instance is
opened. An encoder instance can be referred to by the corresponding handle. CodecInst
is a type managed internally by API. Application does not need to care about it.

NOTE: This type is vaild for encoder only.
@endverbatim
*/
typedef struct CodecInst EncInst;

/**
* @brief
@verbatim
This is a dedicated type for encoder handle returned when an encoder instance is
opened. An encoder instance can be referred by the corresponding handle. EncInst
is a type managed internally by API. Application does not need to care about it.

NOTE: This type is vaild for encoder only.
@endverbatim
*/
typedef EncInst * EncHandle;

/**
* @brief    This is a data structure for configuring MPEG4-specific parameters in encoder applications. (CODA9 encoder only)
*/
typedef struct {
    int mp4DataPartitionEnable;  /**< It encodes with MPEG4 data_partitioned coding tool.   */
    int mp4ReversibleVlcEnable;  /**< It encodes with MPEG4 reversible_vlc coding tool. */
    int mp4IntraDcVlcThr;   /**< It encodes with MPEG4 intra_dc_vlc_thr coding tool. The valid range is 0 - 7. */
    int mp4HecEnable;       /**< It encodes with MPEG4 HEC (Header Extension Code) coding tool.  */
    int mp4Verid;           /**< It encodes with value of MPEG4 part 2 standard version ID. Version 1 and version 2 are allowed.  */
} EncMp4Param;

/**
* @brief    This is a data structure for configuring H.263-specific parameters in encoder applications. (CODA9 encoder only)
*/
typedef struct {
    int h263AnnexIEnable;  /**< It encodes with H.263 Annex I - Advanced INTRA Coding mode. */
    int h263AnnexJEnable;  /**< It encodes with H.263 Annex J - Deblocking Filter mode. */
    int h263AnnexKEnable;  /**< It encodes with H.263 Annex K - Slice Structured mode. */
    int h263AnnexTEnable;  /**< It encodes with H.263 Annex T - Modified Quantization mode. */
} EncH263Param;


/**
* @brief    This is a data structure for custom GOP parameters of the given picture. (WAVE encoder only)
*/
typedef struct {
    int picType;                        /**< A picture type of Nth picture in the custom GOP */
    int pocOffset;                      /**< A POC of Nth picture in the custom GOP */
    int picQp;                          /**< A quantization parameter of Nth picture in the custom GOP */
    int numRefPicL0;                    /**< The number of reference L0 of Nth picture in the custom GOP */
    int refPocL0;                       /**< A POC of reference L0 of Nth picture in the custom GOP */
    int refPocL1;                       /**< A POC of reference L1 of Nth picture in the custom GOP */
    int temporalId;                     /**< A temporal ID of Nth picture in the custom GOP */
} CustomGopPicParam;

/**
* @brief    This is a data structure for custom GOP parameters. (WAVE encoder only)
*/
typedef struct {
    int customGopSize;                  /**< The size of custom GOP (0~8) */
    CustomGopPicParam picParam[MAX_GOP_NUM];  /**< Picture parameters of Nth picture in custom GOP */
} CustomGopParam;

/**
* @brief    This is a data structure for setting custom map options in H.265/HEVC encoder. (WAVE5 encoder only).
*/
typedef struct {
    int roiAvgQp;  /**< It sets an average QP of ROI map. */
    int customRoiMapEnable; /**< It enables ROI map. */
    int customLambdaMapEnable; /**< It enables custom lambda map. */
    int customModeMapEnable;   /**< It enables to force CTU to be encoded with intra or to be skipped.  */
    int customCoefDropEnable;  /**< It enables to force all coefficients to be zero after TQ or not for each CTU (to be dropped).*/
/**
@verbatim
It indicates the start buffer address of custom map.
Each custom CTU map takes 8 bytes and holds mode, coefficient drop flag, QPs, and lambdas like the below illustration.

image::../figure/wave520_ctumap.svg["Format of custom Map", width=300]

@endverbatim
*/
    PhysicalAddress addrCustomMap;
} WaveCustomMapOpt;

/**
 * @brief    This is a data structure for setting VUI parameters in H.265/HEVC encoder. (WAVE only)
 */
 typedef struct {
     Uint32 vuiParamFlags;               /**< It specifies vui_parameters_present_flag.  */
     Uint32 vuiAspectRatioIdc;           /**< It specifies aspect_ratio_idc. */
     Uint32 vuiSarSize;                  /**< It specifies sar_width and sar_height (only valid when aspect_ratio_idc is equal to 255). */
     Uint32 vuiOverScanAppropriate;      /**< It specifies overscan_appropriate_flag. */
     Uint32 videoSignal;                 /**< It specifies video_signal_type_present_flag. */
     Uint32 vuiChromaSampleLoc;          /**< It specifies chroma_sample_loc_type_top_field and chroma_sample_loc_type_bottom_field. */
     Uint32 vuiDispWinLeftRight;         /**< It specifies def_disp_win_left_offset and def_disp_win_right_offset. */
     Uint32 vuiDispWinTopBottom;         /**< It specifies def_disp_win_top_offset and def_disp_win_bottom_offset. */
} HevcVuiParam;

/**
* @brief    This is a data structure of HEVC/AVC encoder parameters for WAVE5.
*/
typedef struct {
/**
@verbatim
A profile indicator (HEVC only)

@* 0 : The firmware determines a profile according to internalbitdepth.
@* 1 : Main profile
@* 2 : Main10 profile
@* 3 : Main still picture profile

NOTE: In AVC encoder, a profile cannot be set by host application. The firmware decides it based on internalbitdepth. It is HIGH profile for bitdepth of 8 and HIGH10 profile for bitdepth of 10.
@endverbatim
*/
    int profile;
    int enStillPicture;                 /**< Still picture profile */
    int level;                          /**< A level indicator (level * 10) */
/**
@verbatim
A tier indicator

@* 0 : Main tier
@* 1 : High tier
@endverbatim
*/
    int tier;
/**
@verbatim
An internal bit-depth which is used for actual encoding

For example, if you set internalBitDepth as 8 for 10bit source picture,
VPU encodes the 10bit source picture into 8bit picture stream.
If nothing is given to internalBitDepth, VPU encodes source YUV based on srcBitDepth.
@endverbatim
*/
    int internalBitDepth;
    int losslessEnable;                 /**< It enables lossless coding. */
    int constIntraPredFlag;             /**< It enables constrained intra prediction. */
/**
@verbatim
A GOP structure preset option

@* 0 : custom GOP
@* Other values : <<vpuapi_h_GOP_PRESET_IDX>>
@endverbatim
*/
    int gopPresetIdx;
/**
@verbatim
The type of I picture to be inserted at every intraPeriod

@* 0 : Non-IRAP
@* 1 : CRA
@* 2 : IDR
@endverbatim
*/
    int decodingRefreshType;
    int intraQP;                        /**< A quantization parameter of intra picture */
    int intraPeriod;                    /**< A period of intra picture in GOP size */
/**
@verbatim
t enables every IDR frame to include VPS/SPS/PPS

@* 0 : No froced Header(VPS/SPS/PPS)
@* 1 : Forced Header before IDR frame
@* 2 : Forced Header before key frame
@endverbatim
*/
    int forcedIdrHeaderEnable;
    int confWinTop;                     /**< A top offset of conformance window */
    int confWinBot;                     /**< A bottom offset of conformance window */
    int confWinLeft;                    /**< A left offset of conformance window */
    int confWinRight;                   /**< A right offset of conformance window */

/**
@verbatim
A slice mode for independent slice

@* 0 : no multi-slice
@* 1 : slice in CTU number
@endverbatim
*/
    int independSliceMode;
    int independSliceModeArg;           /**< The number of CTU for a slice when independSliceMode is set with 1  */

/**
@verbatim
A slice mode for dependent slice

@* 0 : no multi-slice
@* 1 : slice in CTU number
@* 2 : slice in number of byte
@endverbatim
*/
    int dependSliceMode;
    int dependSliceModeArg;             /**< The number of CTU or bytes for a slice when dependSliceMode is set with 1 or 2  */

/**
@verbatim
An intra refresh mode

@* 0 : no intra refresh
@* 1 : row
@* 2 : column
@* 3 : step size in CTU
@* 4 : adaptive intra refresh
@endverbatim
*/
    int intraRefreshMode;
/**
@verbatim
It Specifies an intra CTU refresh interval. Depending on intraRefreshMode,
it can mean one of the followings.

@* The number of consecutive CTU rows for IntraCtuRefreshMode of 1
@* The number of consecutive CTU columns for IntraCtuRefreshMode of 2
@* A step size in CTU for IntraCtuRefreshMode of 3
@* The number of Intra CTUs to be encoded in a picture for IntraCtuRefreshMode of 4

@endverbatim
*/
    int intraRefreshArg;
/**
@verbatim
It uses one of the recommended encoder parameter presets.

@* 0 : custom setting
@* 1 : recommended encoder parameters (slow encoding speed, highest picture quality)
@* 2 : boost mode (normal encoding speed, moderate picture quality)
@* 3 : fast mode (fast encoding speed, low picture quality)
@endverbatim
*/
    int useRecommendEncParam;
    int scalingListEnable;              /**< It enables a scaling list. */
/**
@verbatim
It enables CU(Coding Unit) size to be used in encoding process. Host application can also select multiple CU sizes.

@* 3'b001 : 8x8
@* 3'b010 : 16x16
@* 3'b100 : 32x32
@endverbatim
*/
    int cuSizeMode;
    int tmvpEnable;                     /**< It enables temporal motion vector prediction. */
    int wppEnable;                      /**< It enables WPP (Wave-front Parallel Processing). WPP is unsupported in ring buffer mode of bitstream buffer. */
    int maxNumMerge;                    /**< It specifies the number of merge candidates in RDO (1 or 2). 2 of maxNumMerge (default) offers better quality of encoded picture, while 1 of maxNumMerge improves encoding performance.  */
    int disableDeblk;                   /**< It disables in-loop deblocking filtering. */
    int lfCrossSliceBoundaryEnable;     /**< It enables filtering across slice boundaries for in-loop deblocking. */
    int betaOffsetDiv2;                 /**< It sets BetaOffsetDiv2 for deblocking filter. */
    int tcOffsetDiv2;                   /**< It sets TcOffsetDiv3 for deblocking filter. */
    int skipIntraTrans;                 /**< It enables the transform stage to be skipped for an intra CU. This value is set to transform_skip_enabled_flag in PPS. */
    int saoEnable;                      /**< It enables SAO (Sample Adaptive Offset). */
    int intraNxNEnable;                 /**< It enables intra NxN PUs. */

/**
@verbatim
It specifies picture bits allocation mode.
It is only valid when RateControl is enabled and GOP size is larger than 1.

@* 0 : More referenced pictures have more bits than less referenced pictures do.
@* 1 : All pictures in GOP have similar amount of bits.
@* 2 : Each picture in GOP is allocated a portion (fixedBitRatio) of total bit budget.
@endverbatim
*/
    int bitAllocMode;
/**
@verbatim
A fixed bit ratio (1 ~ 255) for each picture of GOP's bit
allocation

@* N = 0 ~ (MAX_GOP_SIZE - 1)
@* MAX_GOP_SIZE = 8

For instance when MAX_GOP_SIZE is 3, FixedBitRatio0, FixedBitRatio1, and FixedBitRatio2 can be set as 2, 1, and 1 repsectively for
the fixed bit ratio 2:1:1. This is only valid when BitAllocMode is 2.

@endverbatim
*/
    int fixedBitRatio[MAX_GOP_NUM];
    int cuLevelRCEnable;                /**< It enable CU level rate control. */

    int hvsQPEnable;                    /**< It enable CU QP adjustment for subjective quality enhancement. */

    int hvsQpScale;                     /**< A QP scaling factor for CU QP adjustment when hvsQpScaleEnable is 1 */

    int hvsMaxDeltaQp;                     /**< A maximum delta QP for HVS */

    // CUSTOM_GOP
    CustomGopParam gopParam;            /**< <<vpuapi_h_CustomGopParam>> */
    int roiEnable;                      /**< It enables ROI map. NOTE: It is valid when rate control is on. */

    Uint32 numUnitsInTick;              /**< It specifies the number of time units of a clock operating at the frequency time_scale Hz. This is used to to calculate frameRate syntax.  */
    Uint32 timeScale;                   /**< It specifies the number of time units that pass in one second. This is used to to calculate frameRate syntax.  */
    Uint32 numTicksPocDiffOne;          /**< It specifies the number of clock ticks corresponding to a difference of picture order count values equal to 1. This is used to calculate frameRate syntax. */

    int chromaCbQpOffset;               /**< The value of chroma(Cb) QP offset */
    int chromaCrQpOffset;               /**< The value of chroma(Cr) QP offset */

    int initialRcQp;                    /**< The value of initial QP by HOST application. This value is meaningless if INITIAL_RC_QP is 63.*/

    Uint32  nrYEnable;                  /**< It enables noise reduction algorithm to Y component.  */
    Uint32  nrCbEnable;                 /**< It enables noise reduction algorithm to Cb component. */
    Uint32  nrCrEnable;                 /**< It enables noise reduction algorithm to Cr component. */

    // ENC_NR_WEIGHT
    Uint32  nrIntraWeightY;             /**< A weight to Y noise level for intra picture (0 ~ 31). nrIntraWeight/4 is multiplied to the noise level that has been estimated. This weight is put for intra frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrIntraWeightCb;            /**< A weight to Cb noise level for intra picture (0 ~ 31) */
    Uint32  nrIntraWeightCr;            /**< A weight to Cr noise level for intra picture (0 ~ 31) */
    Uint32  nrInterWeightY;             /**< A weight to Y noise level for inter picture (0 ~ 31). nrInterWeight/4 is multiplied to the noise level that has been estimated. This weight is put for inter frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrInterWeightCb;            /**< A weight to Cb noise level for inter picture (0 ~ 31) */
    Uint32  nrInterWeightCr;            /**< A weight to Cr noise level for inter picture (0 ~ 31) */

    Uint32  nrNoiseEstEnable;           /**< It enables noise estimation for noise reduction. When this is disabled, host carries out noise estimation with nrNoiseSigmaY/Cb/Cr. */
    Uint32  nrNoiseSigmaY;              /**< It specifies Y noise standard deviation when nrNoiseEstEnable is 0.  */
    Uint32  nrNoiseSigmaCb;             /**< It specifies Cb noise standard deviation when nrNoiseEstEnable is 0. */
    Uint32  nrNoiseSigmaCr;             /**< It specifies Cr noise standard deviation when nrNoiseEstEnable is 0. */

    Uint32 useLongTerm;                 /**< It enables long-term reference function. */

    // newly added for WAVE Encoder
    Uint32 monochromeEnable;           /**< It enables monochrom encoding mode. */
    Uint32 strongIntraSmoothEnable;    /**< It enables strong intra smoothing. */

    Uint32 weightPredEnable;   /**< It enables to use weighted prediction.*/
    Uint32 bgDetectEnable;     /**< It enables background detection. */
    Uint32 bgThrDiff;          /**< It specifies the threshold of max difference that is used in s2me block. It is valid when background detection is on. */
    Uint32 bgThrMeanDiff;      /**< It specifies the threshold of mean difference that is used in s2me block. It is valid  when background detection is on. */
    Uint32 bgLambdaQp;         /**< It specifies the minimum lambda QP value to be used in the background area. */
    int    bgDeltaQp;          /**< It specifies the difference between the lambda QP value of background and the lambda QP value of foreground. */

    Uint32 customLambdaEnable;  /**< It enables custom lambda table. */
    Uint32 customMDEnable;      /**< It enables custom mode decision. */
    int    pu04DeltaRate;       /**< A value which is added to the total cost of 4x4 blocks */
    int    pu08DeltaRate;       /**< A value which is added to the total cost of 8x8 blocks */
    int    pu16DeltaRate;       /**< A value which is added to the total cost of 16x16 blocks */
    int    pu32DeltaRate;       /**< A value which is added to the total cost of 32x32 blocks */
    int    pu04IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 4x4 Planar intra prediction mode. */
    int    pu04IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost (=distortion + rate) in 4x4 DC intra prediction mode. */
    int    pu04IntraAngleDeltaRate;  /**< A value which is added to rate when calculating cost (=distortion + rate) in 4x4 Angular intra prediction mode.  */
    int    pu08IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 8x8 Planar intra prediction mode.*/
    int    pu08IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost(=distortion + rate) in 8x8 DC intra prediction mode.*/
    int    pu08IntraAngleDeltaRate;  /**< A value which is added to  rate when calculating cost(=distortion + rate) in 8x8 Angular intra prediction mode. */
    int    pu16IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 16x16 Planar intra prediction mode. */
    int    pu16IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost(=distortion + rate) in 16x16 DC intra prediction mode */
    int    pu16IntraAngleDeltaRate;  /**< A value which is added to rate when calculating cost(=distortion + rate) in 16x16 Angular intra prediction mode */
    int    pu32IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 32x32 Planar intra prediction mode */
    int    pu32IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost(=distortion + rate) in 32x32 DC intra prediction mode */
    int    pu32IntraAngleDeltaRate;  /**< A value which is added to rate when calculating cost(=distortion + rate) in 32x32 Angular intra prediction mode */
    int    cu08IntraDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU8x8 */
    int    cu08InterDeltaRate;       /**< A value which is added to rate when calculating cost for inter CU8x8 */
    int    cu08MergeDeltaRate;       /**< A value which is added to rate when calculating cost for merge CU8x8 */
    int    cu16IntraDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU16x16 */
    int    cu16InterDeltaRate;       /**< A value which is added to rate when calculating cost for inter CU16x16 */
    int    cu16MergeDeltaRate;       /**< A value which is added to rate when calculating cost for merge CU16x16 */
    int    cu32IntraDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU32x32 */
    int    cu32InterDeltaRate;       /**< A value which is added to rate when calculating cost for inter CU32x32 */
    int    cu32MergeDeltaRate;       /**< A value which is added to rate when calculating cost for merge CU32x32 */
    int    coefClearDisable;         /**< It disables the transform coefficient clearing algorithm for P or B picture. If this is 1, all-zero coefficient block is not evaluated in RDO. */
    int    minQpI;                   /**< A minimum QP of I picture for rate control */
    int    maxQpI;                   /**< A maximum QP of I picture for rate control */
    int    minQpP;                   /**< A minimum QP of P picture for rate control */
    int    maxQpP;                   /**< A maximum QP of P picture for rate control */
    int    minQpB;                   /**< A minimum QP of B picture for rate control */
    int    maxQpB;                   /**< A maximum QP of B picture for rate control */
    PhysicalAddress customLambdaAddr; /**< It specifies the address of custom lambda map.  */
    PhysicalAddress userScalingListAddr; /**< It specifies the address of user scaling list file. */

    // SVAC encoder only
    int svcEnable;                  /**< It enables to encode with SVC spatial (picture size) scalability. (SVAC encoder only) */
    int svcMode;                    /**< It specifies an SVC mode. (SVAC encoder only) */
    int lumaDcQpOffset;             /**< A delta quantization parameter for luma DC coefficients (SVAC encoder only) */
    int chromaDcQpOffset;           /**< A delta quantization parameter for chroma DC coefficients (SVAC encoder only) */
    int chromaAcQpOffset;           /**< A delta quantization parameter for chroma AC coefficients (SVAC encoder only) */



    // for H.264 on WAVE
    int avcIdrPeriod;				/**< A period of IDR picture (0 ~ 1024). 0 - implies an infinite period */
    int rdoSkip;					/**< It skips RDO(rate distortion optimization). */
    int lambdaScalingEnable;		/**< It enables lambda scaling using custom GOP. */
    int transform8x8Enable;			/**< It enables 8x8 intra prediction and 8x8 transform. */
/**
@verbatim
A slice mode for independent slice

@* 0 : no multi-slice
@* 1 : slice in MB number
@endverbatim
*/
    int avcSliceMode;
    int avcSliceArg;				/**< The number of MB for a slice when avcSliceMode is set with 1  */
/**
@verbatim
An intra refresh mode

@* 0 : no intra refresh
@* 1 : row
@* 2 : column
@* 3 : step size in CTU

@endverbatim
*/
    int intraMbRefreshMode;
/**
@verbatim
It Specifies an intra MB refresh interval. Depending on intraMbRefreshMode,
it can mean one of the followings.

@* The number of consecutive MB rows for intraMbRefreshMode of 1
@* The number of consecutive MB columns for intraMbRefreshMode of 2
@* A step size in MB for intraMbRefreshMode of 3

@endverbatim
*/
    int intraMbRefreshArg;
    int mbLevelRcEnable;			/**< It enables MB-level rate control. */
/**
@verbatim
It selects the entropy coding mode used in encoding process.

0 : CAVLC
1 : CABAC

@endverbatim
*/
    int entropyCodingMode;

    int s2fmeDisable;               /**< It disables s2me_fme (only for AVC encoder). */

    Uint32 rcWeightParam;           /**< Adjusts the speed of updating parameters to the rate control model. If RcWeightFactor is set with a large value, the RC parameters are updated slowly. (Min: 1, Max: 31)*/
    Uint32 rcWeightBuf;             /**< It is the smoothing factor in the estimation. (Min: 1, Max: 255) This parameter indicates the speed of adjusting bitrate toward fullness of buffer as a reaction parameter. As it becomess larger, the bit-rate error promptly affects the target bit allocation of the following picture. */
    HevcVuiParam vuiParam;          /**< <<vpuapi_h_HevcVuiParam>> */
#ifdef SUPPORT_LOOK_AHEAD_RC
    int larcEnable;
    int larcPass;
    int larcSize;
    int larcWeight;
#endif
}EncWaveParam;

/**
* @brief This is an enumeration for encoder parameter change. (WAVE5 encoder only)
*/
typedef enum {
    // COMMON parameters which can be changed frame by frame.
    ENC_SET_CHANGE_PARAM_PPS                 = (1<<0),
    ENC_SET_CHANGE_PARAM_INTRA_PARAM         = (1<<1),
    ENC_SET_CHANGE_PARAM_RC_TARGET_RATE      = (1<<8),
    ENC_SET_CHANGE_PARAM_RC                  = (1<<9),
    ENC_SET_CHANGE_PARAM_RC_MIN_MAX_QP       = (1<<10),
    ENC_SET_CHANGE_PARAM_RC_BIT_RATIO_LAYER  = (1<<11),
    ENC_SET_CHANGE_PARAM_RC_INTER_MIN_MAX_QP = (1<<12),
    ENC_SET_CHANGE_PARAM_RC_WEIGHT           = (1<<13),
    ENC_SET_CHANGE_PARAM_INDEPEND_SLICE      = (1<<16),
    ENC_SET_CHANGE_PARAM_DEPEND_SLICE        = (1<<17),
    ENC_SET_CHANGE_PARAM_RDO                 = (1<<18),
    ENC_SET_CHANGE_PARAM_NR                  = (1<<19),
    ENC_SET_CHANGE_PARAM_BG                  = (1<<20),
    ENC_SET_CHANGE_PARAM_CUSTOM_MD           = (1<<21),
    ENC_SET_CHANGE_PARAM_CUSTOM_LAMBDA       = (1<<22),
    ENC_SET_CHANGE_PARAM_VUI_HRD_PARAM       = (1<<23),
} Wave5ChangeParam;

/**
* @brief This is a data structure for encoding parameters that have changed.
*/
typedef struct {
    int enable_option;                  /**< A flag to decide which parameter will change, <<vpuapi_h_Wave5ChangeParam>> */

    // ENC_SET_CHANGE_PARAM_PPS (lossless, WPP can't be changed while encoding)
    int constIntraPredFlag;             /**< It enables constrained intra prediction. */
    int lfCrossSliceBoundaryEnable;     /**< It enables filtering across slice boundaries for in-loop deblocking. */
    int weightPredEnable;               /**< It enables to use weighted prediction.*/
    int disableDeblk;                   /**< It disables in-loop deblocking filtering. */
    int betaOffsetDiv2;                 /**< It sets BetaOffsetDiv2 for deblocking filter. */
    int tcOffsetDiv2;                   /**< It sets TcOffsetDiv3 for deblocking filter. */
    int chromaCbQpOffset;               /**< The value of chroma(Cb) QP offset */
    int chromaCrQpOffset;               /**< The value of chroma(Cr) QP offset */
    int lumaDcQpOffset;                 /**< The value of DC QP offset for Y component (for SVAC encoder) */
    int chromaDcQpOffset;               /**< The value of DC QP offset for Cb/Cr component (for SVAC encoder) */
    int chromaAcQpOffset;               /**< The value of AC QP offset for Cb/Cr component (for SVAC encoder) */
    int transform8x8Enable;             /**< (for H.264 encoder) */
    int entropyCodingMode;              /**< (for H.264 encoder) */



    // ENC_SET_CHANGE_PARAM_INDEPEND_SLICE
    /**
    @verbatim
    A slice mode for independent slice

    @* 0 : no multi-slice
    @* 1 : slice in CTU number
    @endverbatim
    */
    int independSliceMode;
    int independSliceModeArg;           /**< The number of CTU for a slice when independSliceMode is set with 1  */

    // ENC_SET_CHANGE_PARAM_DEPEND_SLICE
    /**
    @verbatim
    A slice mode for dependent slice

    @* 0 : no multi-slice
    @* 1 : slice in CTU number
    @* 2 : slice in number of byte
    @endverbatim
    */
    int dependSliceMode;
    int dependSliceModeArg;             /**< The number of CTU or bytes for a slice when dependSliceMode is set with 1 or 2  */

/**
@verbatim
A slice mode for independent slice

@* 0 : no multi-slice
@* 1 : slice in MB number
@endverbatim
*/
    int avcSliceArg;
    int avcSliceMode;                   /**< The number of MB for a slice when avcSliceMode is set with 1  */

    // ENC_SET_CHANGE_PARAM_RDO (cuSizeMode, MonoChrom, and RecommendEncParam
    //  can't be changed while encoding)
    int coefClearDisable;               /**< It disables the transform coefficient clearing algorithm for P or B picture. If this is 1, all-zero coefficient block is not evaluated in RDO. */
    int intraNxNEnable;                 /**< It enables intra NxN PUs. */
    int maxNumMerge;                    /**< It specifies the number of merge candidates in RDO (1 or 2). 2 of maxNumMerge (default) offers better quality of encoded picture, while 1 of maxNumMerge improves encoding performance.  */
    int customLambdaEnable;             /**< It enables custom lambda table. */
    int customMDEnable;                 /**< It enables custom mode decision. */
    int rdoSkip;                        /**< It skips RDO(rate distortion optimization) in H.264 encoder. */
    int lambdaScalingEnable;            /**< It enables to use custom lambda scaling list. */

    // ENC_SET_CHANGE_PARAM_RC_TARGET_RATE
    int bitRate;                        /**< A target bitrate when separateBitrateEnable is 0 */


    // ENC_SET_CHANGE_PARAM_RC
    // (rcEnable, cuLevelRc, bitAllocMode, RoiEnable, RcInitQp can't be changed while encoding)
    int hvsQPEnable;                    /**< It enables CU QP adjustment for subjective quality enhancement. */
    int hvsQpScale;                     /**< QP scaling factor for CU QP adjustment when hvcQpenable is 1. */
    int vbvBufferSize;                  /**< It specifies the size of the VBV buffer in msec (10 ~ 3000). For example, 3000 should be set for 3 seconds. This value is valid when rcEnable is 1. VBV buffer size in bits is EncBitrate * VbvBufferSize / 1000. */
    int mbLevelRcEnable;                /**< It enables MB level rate control. (for H.264 encoder) */

    // ENC_SET_CHANGE_PARAM_RC_MIN_MAX_QP
    int minQpI;                         /**< A minimum QP of I picture for rate control */
    int maxQpI;                         /**< A maximum QP of I picture for rate control */
    int hvsMaxDeltaQp;                  /**< A maximum delta QP for HVS */

    // ENC_SET_CHANGE_PARAM_RC_INTER_MIN_MAX_QP
    int minQpP;                         /**< A minimum QP of P picture for rate control */
    int minQpB;                         /**< A minimum QP of B picture for rate control */
    int maxQpP;                         /**< A maximum QP of P picture for rate control */
    int maxQpB;                         /**< A maximum QP of B picture for rate control */

    // ENC_SET_CHANGE_PARAM_RC_BIT_RATIO_LAYER
    /**
    @verbatim
    A fixed bit ratio (1 ~ 255) for each picture of GOP's bit
    allocation

    @* N = 0 ~ (MAX_GOP_SIZE - 1)
    @* MAX_GOP_SIZE = 8

    For instance when MAX_GOP_SIZE is 3, FixedBitRatio0, FixedBitRatio1, and FixedBitRatio2 can be set as 2, 1, and 1 repsectively for
    the fixed bit ratio 2:1:1. This is only valid when BitAllocMode is 2.
    @endverbatim
    */
    int fixedBitRatio[MAX_GOP_NUM];

    // ENC_SET_CHANGE_PARAM_BG (bgDetectEnable can't be changed while encoding)
    int    s2fmeDisable;   /**< It disables s2me_fme (only for AVC encoder). */
    Uint32 bgThrDiff;      /**< It specifies the threshold of max difference that is used in s2me block. It is valid when background detection is on. */
    Uint32 bgThrMeanDiff;  /**< It specifies the threshold of mean difference that is used in s2me block. It is valid  when background detection is on. */
    Uint32 bgLambdaQp;     /**< It specifies the minimum lambda QP value to be used in the background area. */
    int    bgDeltaQp;      /**< It specifies the difference between the lambda QP value of background and the lambda QP value of foreground. */

    // ENC_SET_CHANGE_PARAM_NR
    Uint32  nrYEnable;     /**< It enables noise reduction algorithm to Y component.  */
    Uint32  nrCbEnable;    /**< It enables noise reduction algorithm to Cb component. */
    Uint32  nrCrEnable;    /**< It enables noise reduction algorithm to Cr component. */
    Uint32  nrNoiseEstEnable;  /**<  It enables noise estimation for noise reduction. When this is disabled, host carries out noise estimation with nrNoiseSigmaY/Cb/Cr. */
    Uint32  nrNoiseSigmaY;  /**< It specifies Y noise standard deviation when nrNoiseEstEnable is 0.  */
    Uint32  nrNoiseSigmaCb; /**< It specifies Cb noise standard deviation when nrNoiseEstEnable is 0. */
    Uint32  nrNoiseSigmaCr; /**< It specifies Cr noise standard deviation when nrNoiseEstEnable is 0. */

    Uint32  nrIntraWeightY;  /**< A weight to Y noise level for intra picture (0 ~ 31). nrIntraWeight/4 is multiplied to the noise level that has been estimated. This weight is put for intra frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrIntraWeightCb; /**< A weight to Cb noise level for intra picture (0 ~ 31) */
    Uint32  nrIntraWeightCr; /**< A weight to Cr noise level for intra picture (0 ~ 31) */
    Uint32  nrInterWeightY;  /**< A weight to Y noise level for inter picture (0 ~ 31). nrInterWeight/4 is multiplied to the noise level that has been estimated. This weight is put for inter frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrInterWeightCb; /**< A weight to Cb noise level for inter picture (0 ~ 31) */
    Uint32  nrInterWeightCr; /**< A weight to Cr noise level for inter picture (0 ~ 31) */


    // ENC_SET_CHANGE_PARAM_CUSTOM_MD
    int    pu04DeltaRate;       /**< A value which is added to the total cost of 4x4 blocks */
    int    pu08DeltaRate;       /**< A value which is added to the total cost of 8x8 blocks */
    int    pu16DeltaRate;       /**< A value which is added to the total cost of 16x16 blocks */
    int    pu32DeltaRate;       /**< A value which is added to the total cost of 32x32 blocks */
    int    pu04IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 4x4 Planar intra prediction mode. */
    int    pu04IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost (=distortion + rate) in 4x4 DC intra prediction mode. */
    int    pu04IntraAngleDeltaRate;  /**< A value which is added to rate when calculating cost (=distortion + rate) in 4x4 Angular intra prediction mode.  */
    int    pu08IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 8x8 Planar intra prediction mode.*/
    int    pu08IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost(=distortion + rate) in 8x8 DC intra prediction mode.*/
    int    pu08IntraAngleDeltaRate;  /**< A value which is added to  rate when calculating cost(=distortion + rate) in 8x8 Angular intra prediction mode. */
    int    pu16IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 16x16 Planar intra prediction mode. */
    int    pu16IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost(=distortion + rate) in 16x16 DC intra prediction mode */
    int    pu16IntraAngleDeltaRate;  /**< A value which is added to rate when calculating cost(=distortion + rate) in 16x16 Angular intra prediction mode */
    int    pu32IntraPlanarDeltaRate; /**< A value which is added to rate when calculating cost(=distortion + rate) in 32x32 Planar intra prediction mode */
    int    pu32IntraDcDeltaRate;     /**< A value which is added to rate when calculating cost(=distortion + rate) in 32x32 DC intra prediction mode */
    int    pu32IntraAngleDeltaRate;  /**< A value which is added to rate when calculating cost(=distortion + rate) in 32x32 Angular intra prediction mode */
    int    cu08IntraDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU8x8 */
    int    cu08InterDeltaRate;       /**< A value which is added to rate when calculating cost for inter CU8x8 */
    int    cu08MergeDeltaRate;       /**< A value which is added to rate when calculating cost for merge CU8x8 */
    int    cu16IntraDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU16x16 */
    int    cu16InterDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU16x16 */
    int    cu16MergeDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU16x16 */
    int    cu32IntraDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU32x32 */
    int    cu32InterDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU32x32 */
    int    cu32MergeDeltaRate;       /**< A value which is added to rate when calculating cost for intra CU32x32 */

    // ENC_SET_CHANGE_PARAM_CUSTOM_LAMBDA
    PhysicalAddress customLambdaAddr; /**< It specifies the address of custom lambda map.  */

    // ENC_SET_CHANGE_PARAM_INTRA_PARAM
    int intraQP;                        /**< A quantization parameter of intra picture */
    int intraPeriod;                    /**< A period of intra picture in GOP size */
    int avcIdrPeriod;                   /**< A period of IDR picture (0 ~ 1024). 0 - implies an infinite period(for H.264 encoder) */
    /**
    @verbatim
    t enables every IDR frame to include VPS/SPS/PPS

    @* 0 : No froced Header(VPS/SPS/PPS)
    @* 1 : Forced Header before IDR frame
    @* 2 : Forced Header before key frame
    @endverbatim
    */
    int forcedIdrHeaderEnable;

    // ENC_SET_CHANGE_PARAM_RC_WEIGHT
    Uint32 rcWeightBuf;   /**< It is the smoothing factor in the estimation. (Min: 1, Max: 255) This parameter indicates the speed of adjusting bitrate toward fullness of buffer as a reaction parameter. As it becomess larger, the bit-rate error promptly affects the target bit allocation of the following picture. */
    Uint32 rcWeightParam; /**< Adjusts the speed of updating parameters to the rate control model. If RcWeightFactor is set with a large value, the RC parameters are updated slowly. (Min: 1, Max: 31)*/

}EncChangeParam;

/**
* @brief    This is a data structure for configuring PPS information at H.264/AVC.
*/
typedef struct {
    int ppsId;          /**< H.264 picture_parameter_set_id in PPS. This shall be in the range of 0 to 255, inclusive. */
/**
@verbatim
It selects the entropy coding method used in the encoding process.

@* 0 : CAVLC
@* 1 : CABAC
@* 2 : CAVLC/CABAC select according to PicType
@endverbatim
*/
    int entropyCodingMode;
    int cabacInitIdc;   /**< It specifies the index for determining the initialization table used in the initialisation process for CABAC. The value of cabac_init_idc shall be in the range of 0 ~ 2. */
/**
@verbatim
It specifies whether to enable 8x8 intra prediction and 8x8 transform or not.

@* 0 : disable 8x8 intra and 8x8 transform (BP)
@* 1 : enable 8x8 intra and 8x8 transform (HP)
@endverbatim
*/
    int transform8x8Mode;
} AvcPpsParam;

/**
* @brief    This is a data structure for configuring H.264/AVC-specific parameters in encoder applications. (CODA9 only)
*/
typedef struct {
/**
@verbatim
@* 0 : disable
@* 1 : enable
@endverbatim
*/
    int constrainedIntraPredFlag;
/**
@verbatim
@* 0 : enable
@* 1 : disable
@* 2 : disable deblocking filter at slice boundaries
@endverbatim
*/
    int disableDeblk;
    int deblkFilterOffsetAlpha; /**< It sets deblk_filter_offset_alpha (-6 to 6). */
    int deblkFilterOffsetBeta;  /**< It sets deblk_filter_offset_beta (-6 to 6).  */
    int chromaQpOffset;         /**< It sets chroma_qp_offset (-12 to 12).  */
/**
@verbatim
@* 0 : disable
@* 1 : enable

If this is 1, VPU generates AUD RBSP at the start of every picture.
@endverbatim
*/
    int audEnable;
/**
@verbatim
@* 0 : disable
@* 1 : enable

If this is 1, VPU generates frame_cropping_flag syntax in the SPS header.
@endverbatim
*/
    int frameCroppingFlag;
    int frameCropLeft;      /**< The sample number of left cropping region in a picture line. See the frame_crop_left_offset syntax in H.264/AVC SPS tabular form. The least significant bit of this parameter should be always zero. */
    int frameCropRight;     /**< The sample number of right cropping region in a picture line. See the frame_crop_right_offset syntax in H.264/AVC SPS tabular form. The least significant bit of this parameter should be always zero. */
    int frameCropTop;       /**< The sample number of top cropping region in a picture column. See the frame_crop_top_offset syntax in H.264/AVC SPS tabular form. The least significant bit of this parameter should be always zero. */
    int frameCropBottom;    /**< The sample number of bottom cropping region in a picture column. See the frame_crop_bottom_offset syntax in H.264/AVC SPS tabular form. The least significant bit of this parameter should be always zero. */


    int level;              /**< H.264/AVC level_idc in SPS */
/**
@verbatim
@* 0 : Baseline profile
@* 1 : Main profile
@* 2 : High profile
@endverbatim
*/
    int profile;
} EncAvcParam;

/**
* @brief    This structure is used for declaring an encoder slice mode and its options. It is newly added for more flexible usage of slice mode control in encoder. (CODA9 only)
*/
typedef struct{
/**
@verbatim
@* 0 : one slice per picture
@* 1 : multiple slices per picture
@* 2 : multiple slice encoding mode 2 for H.264 only.

Slice separation::
In MPEG4, resync-marker and packet header are inserted between
slice boundaries. In short video header (H.263) with Annex K of 0, GOB headers are inserted
at every GOB layer start. In short video header (H.263) with Annex K of 1, multiple
slices are generated. In AVC, multiple slice layer RBSP is generated.
@endverbatim
*/
    int sliceMode;
/**
@verbatim
This parameter means the size of generated slice.

@* 0 : sliceSize is defined by the amount of bits when sliceMode is 1.
@* 1 : sliceSize is defined by the number of MBs in a slice when sliceMode is 1.
@* 2 : sliceSize is defined by MBs run-length table (only for H.264) when sliceMode is 2.

This parameter is ignored when sliceMode of 0 or
in short video header mode with Annex K of 0.
@endverbatim
*/

    int sliceSizeMode;
    int sliceSize;  /**< The size of a slice in bits or in MB numbers included in a slice, which is specified by the variable, sliceSizeMode. This parameter is ignored when sliceMode is 0 or in short video header mode with Annex K of 0. */
} EncSliceMode;



/**
* @brief    This data structure is used when HOST wants to open a new encoder instance.
*/
typedef struct {
    PhysicalAddress bitstreamBuffer;        /**< The start address of bitstream buffer into which encoder puts bitstream. This address must be aligned to AXI bus width. */
    Uint32          bitstreamBufferSize;    /**< The size of the buffer in bytes pointed by bitstreamBuffer. This value must be a multiple of 1024.  */
    CodStd          bitstreamFormat;        /**< The standard type of bitstream in encoder operation. It is one of STD_MPEG4, STD_H263, STD_AVC and STD_HEVC. */
/**
@verbatim
@* 0 : line-buffer mode
@* 1 : ring-buffer mode

This flag sets the streaming mode for the current encoder instance. There are two
streaming modes: packet-based streaming with ring-buffer (buffer-reset mode) and
frame-based streaming with line buffer (buffer-flush mode).

@endverbatim
*/
    int             ringBufferEnable;
    int             picWidth;           /**< The width of a picture to be encoded in unit of sample. */
    int             picHeight;          /**< The height of a picture to be encoded in unit of sample. */
/**
@verbatim
It is a linear-to-tiled enable mode.  (CODA9 only)
The source frame can be converted from linear format to tiled format in PrP (Pre-Processing) block.

@* 0 : disable linear-to-tiled-map conversion
@* 1 : enable linear-to-tiled-map conversion
@endverbatim
*/
    int             linear2TiledEnable;
/**
@verbatim
It can specify the map type of source frame buffer when linear2TiledEnable is enabled. (CODA980 only)

@* 1 : source frame buffer is in linear frame map.
@* 2 : source frame buffer is in linear field map.

@endverbatim
*/
    int             linear2TiledMode;
/**
@verbatim
(CODA9)
The 16 LSB bits, [15:0], is a numerator and 16 MSB bits, [31:16], is a
denominator for calculating frame rate. The numerator means clock ticks per
second, and the denominator is clock ticks between frames minus 1.

So the frame rate can be defined by (numerator/(denominator + 1)),
which equals to (frameRateInfo & 0xffff) /((frameRateInfo >> 16) + 1).

For example, the value 30 of frameRateInfo represents 30 frames/sec, and the
value 0x3e87530 represents 29.97 frames/sec.

(WAVE)
A frame rate indicator  for rate control
For example, the value 30 of frameRateInfo represents 30 frames/sec.
@endverbatim
*/
    int             frameRateInfo;
/**
@verbatim
The horizontal search range for Motion Estimation (CODA980 only)

@* 0 : horizontal search range (-64 ~ 63)
@* 1 : horizontal search range (-48 ~ 47)
@* 2 : horizontal search range (-32 ~ 31)
@* 3 : horizontal search range (-16 ~ 15)
@endverbatim
*/
    int             MESearchRangeX;
/**
@verbatim
The vertical search range for Motion Estimation (CODA980 only)

@* 0 : vertical search range (-48 ~ 47)
@* 1 : vertical search range(-32 ~ 31)
@* 2 : vertical search range(-16 ~ 15)
@endverbatim
*/
    int             MESearchRangeY;
/**
@verbatim
An enable flag for initial QP offset for I picture in GOP. (CODA980 only)

@* 0 : disable (default)
@* 1 : enable

This value is valid for H.264/AVC encoder and ignored when RcEnable is 0.
@endverbatim
*/
    int             rcGopIQpOffsetEn;
/**
@verbatim
An initial QP offset for I picture in GOP (CODA980 only)

rcGopIQpOffset (-4 to 4) is added to an I picture QP value.
This value is valid for H.264/AVC encoder and ignored when RcEnable is 0 or RcGopIQpOffsetEn is 0.
@endverbatim
*/
    int             rcGopIQpOffset;
/**
@verbatim
The search range for Motion Estimation (CODA960 only)

@* 0 : horizontal(-128 ~ 127) and vertical(-64 ~ 63)
@* 1 : horizontal(-64 ~ 63) and vertical(-32 ~ 31)
@* 2 : horizontal(-32 ~ 31) and vertical(-16 ~ 15)
@* 3 : horizontal(-16 ~ 15) and vertical(-16 ~ 15)
@endverbatim
*/
    int             MESearchRange;
/**
@verbatim
vbv_buffer_size in bits

This value is ignored if rate control is disabled.
The value 0 means that encoder does not check reference decoder buffer size constraints.
@endverbatim
*/
    int             vbvBufferSize;
/**
@verbatim
Frame skip indicates that encoder can skip frame encoding automatically when
bitstream has been generated much so far considering the given target bitrate. (CODA9 only)
This parameter is ignored if rate control is disabled.

@* 0 : enable frame skip function.
@* 1 : disable frame skip function.

NOTE: This variable is for CODA9. For WAVE, host can use EncParam.skipPicture.
@endverbatim
*/
    int             frameSkipDisable;

/**
@verbatim
This variable defines the interval of I picture. (CODA9 only)

@* 0 : only first I picture
@* 1 : all I pictures
@* 2 : IPIP ...
@* 3 : IPPIPP ...

The maximum value is 32767, but in practice, a smaller value should be
chosen by HOST application for error resilience.
@endverbatim
*/
    int             gopSize;
/**
An interval of adding an IDR picture (CODA9 only)
*/
    int idrInterval;
/**
@verbatim
A block mode enable flag for Motion Estimation. (H.264/AVC only).
HOST can use some combination (bitwise or-ing) of each value under below.

@* 4'b0000 or 4'b1111 : use all block mode
@* 4'b0001 : enable 16x16 block mode
@* 4'b0010 : enable 16x8 block mode
@* 4'b0100 : enable 8x16 block mode
@* 4'b1000 : enable 8x8 block mode
@endverbatim
*/
    int             meBlkMode;
    EncSliceMode    sliceMode;      /**< <<vpuapi_h_EncSliceMode>> */
/**
@verbatim
The number of intra MB to be inserted in picture (CODA9 only)

@* 0 : intra MB refresh is not used.
@* Other value : intraRefreshNum of MBs are encoded as intra MBs in every P frame.

@endverbatim
*/
    int             intraRefreshNum;
/**
@verbatim
Consecutive intra MB refresh mode (CODA9 only)

This option is valid only when IntraMbRefresh-Num[15:0] is not 0.

@* 0 : Consecutive intra MB refresh mode is disabled. IntraMbRefreshNum of MBs are encoded
as intra MB at the predefined interval
size.
@* 1 : IntraMbRefreshNum of consecutive MBs are encoded as intra MB.
@endverbatim
*/
    int             ConscIntraRefreshEnable;
/**
@verbatim
The maximum quantized step parameter for encoding process

In MPEG4/H.263 mode, the maximum value is 31.
In H.264 mode, allowed maximum value is 51.

NOTE: This variable is only for CODA9. For WAVE, host can use waveParam.maxQpI, maxQpP and maxQpB in EncOpenParam.
@endverbatim
*/
    int             userQpMax;

    //h.264 only
    int             maxIntraSize;   /**< The maximum bit size for intra frame. (CODA9 H.264/AVC only) */
    int             userMaxDeltaQp; /**< The maximum delta QP for encoding process. (CODA9 H.264/AVC only) */
    int             userMinDeltaQp; /**< The minimum delta QP for encoding process. (CODA9 H.264/AVC only) */
    int             userQpMin;    /**< The minimum quantized step parameter for encoding process. (CODA9 H.264/AVC only) */

/**
@verbatim
The PMV option for Motion Estimation. (CODA9 only)
If this field is 1, encoding quality can be worse than when it is 0.

@* 0 : Motion Estimation engine uses PMV that was derived from neighbor MV.
@* 1 : Motion Estimation engine uses Zero PMV.
@endverbatim
*/
    int             MEUseZeroPmv;
/**
@verbatim
Additional weight of intra cost for mode decision to reduce intra MB density (CODA9 only)

By default, it could be zero.
If this variable have some value W,
and the cost of best intra mode that was decided by Refine-Intra-Mode-Decision is ICOST,
the Final Intra Cost FIC is like the below,

FIC = ICOST + W

So, if this field is not zero,
the Final Intra Cost have additional weight. Then the mode decision logic is likely to decide inter mode rather than intra mode for MBs.
@endverbatim
*/
    int             intraCostWeight;
/**
@verbatim
The quantization parameter for I frame (CODA9 only)

When this value is -1, the quantization
parameter for I frame is automatically determined by VPU.
@endverbatim
*/
    //mp4 only
    int             rcIntraQp;
/**
@verbatim
A gamma is a smoothing factor in motion estimation. A value for gamma is
factor * 32768, the factor value is selected from the range 0 &le; factor &le; 1. (CODA9 only)

@* If the factor value is close to 0, QP changes slowly.
@* If the factor value is close to 1, QP changes quickly.

The default gamma value is 0.75 * 32768
@endverbatim
*/
    int             userGamma;
/**
@verbatim
Encoder rate control mode setting (CODA9 only)

@* 0 : normal mode rate control - QP changes for every MB
@* 1 : FRAME_LEVEL rate control - QP changes for every frame
@* 2 : SLICE_LEVEL rate control - QP changes for every slice
@* 3 : USER DEFINED MB LEVEL rate control - QP changes for every number of mbInterval
@endverbatim
*/
    int             rcIntervalMode;
/**
@verbatim
The user defined MB interval value (CODA9 only)

This value is used only when rcIntervalMode is 3.
@endverbatim
*/
    int             mbInterval;
/**
@verbatim
@* CODA9
@** Target bit rate in kbps. If it is 0, it means that rate control is disabled.

@* WAVE series
@** Target bit rate in bps
@endverbatim
*/
    int             bitRate;
    int             bitRateBL; /**< The bitrate value of base layer (SVAC encoder only) */
/**
@verbatim
Time delay in mili-seconds (CODA9 only)

It is the amount of time in ms taken for bitstream to reach initial occupancy of the vbv buffer from zero level.

This value is ignored if rate
control is disabled. The value 0 means VPU does not check for reference
decoder buffer delay constraints.
@endverbatim
*/
    int             rcInitDelay;
/**
@verbatim
@* WAVE series
@** 0 : rate control is off.
@** 1 : rate control is on.

@* CODA9
@** 0 : constant QP (VBR, rate control off)
@** 1 : constant bitrate (CBR)
@** 2 : average bitrate (ABR)
@** 4 : picture level rate control
@endverbatim
*/
    int             rcEnable;

    union {
        EncMp4Param     mp4Param;       /**< <<vpuapi_h_EncMp4Param>>  */
        EncH263Param    h263Param;      /**< <<vpuapi_h_EncH263Param>> */
        EncAvcParam     avcParam;       /**< <<vpuapi_h_EncAvcParam>> */
        EncWaveParam    waveParam;      /**< <<vpuapi_h_EncWaveParam>> */
    } EncStdParam;

    // Maverick-II Cache Configuration
/**
@verbatim
Cache MC bypass (CODA9 only)

@* 0 : MC uses a cache.
@* 1 : MC does not use a cache.
@endverbatim
*/
    int             cacheBypass;
/**
@verbatim
@* 0 : Cb data are written in Cb frame memory and Cr data are written in Cr frame memory. (chroma separate mode)
@* 1 : Cb and Cr data are written in the same chroma memory. (chroma interleave mode)
@endverbatim
*/
    int             cbcrInterleave;
/**
@verbatim
CbCr order in planar mode (YV12 format)

@* 0 : Cb data are written first and then Cr written in their separate plane.
@* 1 : Cr data are written first and then Cb written in their separate plane.
@endverbatim
*/
    int             cbcrOrder;
/**
@verbatim
The endianess of the frame buffer for reference and reconstruction

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE Datasheet'.
@endverbatim
*/
    int             frameEndian;
/**
@verbatim
The endianess of the bitstream buffer

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE Datasheet'.
@endverbatim
*/
    int             streamEndian;
/**
@verbatim
The endianess of the frame buffer for source YUV

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE Datasheet'.
@endverbatim
*/
    int             sourceEndian;
/**
@verbatim
It writes output with 8 burst in linear map mode. (CODA9 only)

@* 0 : burst write back is disabled
@* 1 : burst write back is enabled.

@endverbatim
*/
    int             bwbEnable;
/**
@verbatim
@* 0 : Disable
@* 1 : Enable

This flag is used to encode frame-based streaming video with line buffer.
If this field is set, VPU sends a buffer full interrupt when line buffer is full and waits until the interrupt is cleared.
HOST should read the bitstream in line buffer and clear the interrupt.
If this field is not set, VPU does not send a buffer full interrupt even if line buffer is full.

@endverbatim
*/
    int               lineBufIntEn;
    int               packedFormat;  /**< <<vpuapi_h_PackedFormatNum>> */
    FrameBufferFormat srcFormat;     /**< A color format of source image defined in <<vpuapi_h_FrameBufferFormat>>. */
    FrameBufferFormat outputFormat;  /**< A color format of output image defined in <<vpuapi_h_FrameBufferFormat>>. */
    int               srcBitDepth;   /**< A bit-depth of source image */

/**
@verbatim
VPU core index number

* 0 to (number of VPU core - 1)
@endverbatim
*/
    Uint32          coreIdx;
/**
@verbatim
@* 0 : CbCr data is interleaved in chroma source frame memory. (NV12)
@* 1 : CrCb data is interleaved in chroma source frame memory. (NV21)
@endverbatim
*/
    int             nv21;

    Uint32          virtAxiID; /**< AXI-ID for the V-CPU part (for virtualization) */

    BOOL            enablePTS; /**< An enable flag to report PTS(Presentation Timestamp) */
    int             lowLatencyMode; /**< 2bits low latency mode setting. bit[1]: low latency interrupt enable, bit[0]: fast bitstream-packing enable (only for WAVE5) */

    BOOL            enableNonRefFbcWrite;   /**< If it is TRUE, FBC data of non-reference picture are written into framebuffer. */
    int             picWidthBL;             /**< The width of the BL(base layer) picture in SVC (SVAC encoder only) */
    int             picHeightBL;            /**< The height of the BL(base layer) picture in SVC (SVAC encoder only) */
    int             sourceBufCount;         /**< The number of source frame buffer in encoder */
    int             streamBufCount;         /**< The number of stream buffer in encoder */
    int             streamBufSize;          /**< The size of stream buffer in encoder  */


    int             rcWeightFactor; /**< Adjusts the speed of updating parameters to the rate control model. If RcWeightFactor is set with a large value, the RC parameters are updated slowly. (Min: 1, Max: 32, default: 2) */
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
    int             srcReleaseIntEnable; /**< It enables released source buffer interrupt. */
#endif
    int             ringBufferWrapEnable; /**< It enables read and write pointers to be wrapped around under buffer fullness in ring buffer mode.  */

} EncOpenParam;

/**
* @brief    This is a data structure which contains the number of source frame buffer and reconstructed frame buffer required for running an encoder instance. This is returned after calling GetInitialInfo().
*/
typedef struct {
    int             minFrameBufferCount; /**< Minimum number of frame buffer */
    int             minSrcFrameCount;    /**< Minimum number of source buffer */
    int             maxLatencyPictures;  /**< Maximum number of picture latency */
    int             seqInitErrReason;    /**< Error information */
    int             warnInfo;            /**< Warn information */
    Uint32          vlcBufSize; /**< The size of task buffer */
    Uint32          paramBufSize; /**< The size of task buffer */
} EncInitialInfo;

#ifdef SUPPORT_LOOK_AHEAD_RC
#define LOOK_AHEAD_RC_MIN_SIZE 1
#define LOOK_AHEAD_RC_MAX_SIZE 80
#define LOOK_AHEAD_RC_DATA_SIZE (LOOK_AHEAD_RC_MAX_SIZE+(MAX_SRC_BUFFER_NUM*2)+COMMAND_QUEUE_DEPTH)

typedef struct {
    int             larcData[LOOK_AHEAD_RC_DATA_SIZE][3];
    int             Pass1encPicCnt;
    int             Pass2encPicCnt;
    int             larcPass1Finish;
    int             larcPass;
} EncLarcInfo;
#endif

/**
* @brief    This is a data structure for setting NAL unit coding options.
 */
typedef struct {
    int implicitHeaderEncode;       /**< Whether HOST application encodes a header implicitly or not. If this value is 1, three encode options encodeVPS, encodeSPS, and encodePPS are ignored. */
    int encodeVCL;                  /**< A flag to encode VCL nal unit explicitly */
    int encodeVPS;                  /**< A flag to encode VPS nal unit explicitly */
    int encodeSPS;                  /**< A flag to encode SPS nal unit explicitly */
    int encodePPS;                  /**< A flag to encode PPS nal unit explicitly */
    int encodeAUD;                  /**< A flag to encode AUD nal unit explicitly */
    int encodeEOS;                  /**< A flag to encode EOS nal unit explicitly. This should be set when to encode the last source picture of sequence. */
    int encodeEOB;                  /**< A flag to encode EOB nal unit explicitly. This should be set when to encode the last source picture of sequence. */
    int encodeVUI;                  /**< A flag to encode VUI nal unit explicitly */
    int encodeFiller;               /**< A flag to encode Filler nal unit explicitly (WAVE5 only) */
} EncCodeOpt;


/**
* @brief This is a data structure for configuring picture encode operation. The variables can change every time one picture is encoded.
 */
 typedef struct {
    FrameBuffer*    sourceFrame;    /**< This member must represent the frame buffer containing source image to be encoded. */
/**
@verbatim
If this value is 0, the picture type is determined by the encoder
according to the various parameters such as encoded frame number and GOP size.

If this value is 1, the frame is encoded as an I-picture regardless of the
frame number or GOP size, and I-picture period calculation is reset to
initial state. In MPEG4 and H.263 case, I-picture is sufficient for decoder
refresh. In H.264/AVC case, the picture is encoded as an IDR (Instantaneous
Decoding Refresh) picture.

This value is ignored if skipPicture is 1. (CODA9 only)
@endverbatim
*/
    int             forceIPicture;
/**
@verbatim
If this value is 0, the encoder encodes a picture as normal.

If this value is 1, the encoder ignores sourceFrame and generates a skipped
picture. (WAVE only) In this case, the reconstructed image at decoder side is a duplication
of the previous picture. The skipped picture is encoded as P-type regardless of
the GOP size.
@endverbatim
*/
    int             skipPicture;
    int             quantParam;         /**< This value is used for all quantization parameters in case of VBR - no rate control. (CODA9 only) */
/**
@verbatim
The start address of picture stream buffer under line-buffer mode.

This variable represents the start of picture stream for encoded output. In
buffer-reset mode, HOST might use multiple picture stream buffers for the
best performance. By using this variable, HOST application could re-register the
start position of the picture stream while issuing a picture encoding operation.
The buffer size is specified by the following variable, picStreamBufferSize. In packet-based streaming
with ring-buffer, this variable is ignored.

NOTE: This variable is only meaningful when line-buffer mode is enabled.
@endverbatim
*/
    PhysicalAddress picStreamBufferAddr;
/**
@verbatim
This variable represents the byte size of picture stream buffer. This variable is
so crucial in line-buffer mode. That is because encoder output could be
corrupted if this size is smaller than any picture encoded output. So this value
should be big enough for storing multiple picture streams with average size. In
packet-based streaming with ring-buffer, this variable is ignored.
@endverbatim
*/
   int             picStreamBufferSize;
/**
@verbatim
@* 0 : progressive (frame) encoding
@* 1 : interlaced (field) encoding

This is only for CODA9.
@endverbatim
*/
    int             fieldRun;


    int             forcePicQpEnable;   /**< A flag to use a force picture quantization parameter (WAVE only) */
    int             forcePicQpI;        /**< A force picture quantization parameter for I picture. It is valid when forcePicQpEnable is 1. (WAVE only) */
    int             forcePicQpP;        /**< A force picture quantization parameter for P picture. It is valid when forcePicQpEnable is 1. (WAVE only) */
    int             forcePicQpB;        /**< A force picture quantization parameter for B picture. It is valid when forcePicQpEnable is 1. (WAVE only) */
    int             forcePicTypeEnable; /**< A flag to use a force picture type (WAVE only) */
    int             forcePicType;       /**< A force picture type (I, P, B, IDR, CRA). It is valid when forcePicTypeEnable is 1. (WAVE only) */
    int             srcIdx;             /**< A source frame buffer index (WAVE only) */
    int             srcEndFlag;         /**< A flag indicating that there is no more source frame buffer to encode  (WAVE only) */

    EncCodeOpt      codeOption;         /**< <<vpuapi_h_EncCodeOpt>> (WAVE only) */

    Uint32  useCurSrcAsLongtermPic; /**< A flag for the current picture to be used as a longterm reference picture later when other picture's encoding (WAVE only) */
    Uint32  useLongtermRef;         /**< A flag to use a longterm reference picture in DPB when encoding the current picture (WAVE only) */
    Uint64  pts;                    /**< The presentation Timestamp (PTS) of input source */

    Uint32 coda9RoiEnable;          /**< A flag to use ROI (CODA9 only) */
    Uint32 coda9RoiPicAvgQp;        /**< A average value of ROI QP for a picture (CODA9 only) */
    PhysicalAddress roiQpMapAddr;   /**< The start address of ROI QP map (CODA9 only) */
    Uint32 nonRoiQp;                /**< A non-ROI QP for a picture */
    // belows are newly added for WAVE5 encoder
    WaveCustomMapOpt customMapOpt;  /**< <<vpuapi_h_WaveCustomMapOpt>> */
    Uint32 wpPixSigmaY;  /**< Pixel variance of Y component for weighted prediction  */
    Uint32 wpPixSigmaCb; /**< Pixel variance of Cb component for weighted prediction  */
    Uint32 wpPixSigmaCr; /**< Pixel variance of Cr component for weighted prediction */
    Uint32 wpPixMeanY;   /**< Pixel mean value of Y component for weighted prediction */
    Uint32 wpPixMeanCb;  /**< Pixel mean value of Cb component for weighted prediction */
    Uint32 wpPixMeanCr;  /**< Pixel mean value of Cr component for weighted prediction */
    Uint32 forceAllCtuCoefDropEnable; /**< It forces all coefficients to be zero after TQ . */

    // only for SVAC encoder
    Int32 userFilterLevelEnable;    /**< It enables to set the user filter level. (SVAC encoder only) */
    Int32 lfFilterLevel;            /**< It specifies the loop filter level. (0 ~ 63) The userFilterLevelEnable must be 1. (SVAC encoder only) */
    Int32 sharpLevel;               /**< It specifies the sharpness level of filter. (0 ~ 7)  (SVAC encoder only) */
    Int32 lfRefDeltaIntra;          /**< It specifies a delta value of filter level for key frame. (-63 ~ 63) (SVAC encoder only) */
    Int32 lfRefDeltaRef0;           /**< It specifies a delta value of filter level for Ref0 (Dynamic Ref) frame. (-63 ~ 63) (SVAC encoder only) */
    Int32 lfRefDeltaRef1;           /**< It specifies a delta value of filter level for Ref1 (Optional Ref) frame. (-63 ~ 63) (SVAC encoder only) */
    Int32 lfModeDelta;              /**< It specifies a delta value of filter level according to inter/intra mode. (-63 ~ 63) (SVAC encoder only) */
/**
@verbatim
It specifies an spatial SVC layer.

@* 0 : base layer picture
@* 1 : enhanced layer picture
@endverbatim
*/
    Int32 svcLayerFlag;
    FrameBuffer*    OffsetTblBuffer;    /**< A offset table buffer address for Cframe50  */


    Uint32  qosEnable;  /**< It updates QoS control information. To update QoS values, qosEnable must be 1. */
    Uint32  qosConfig;  /**< It enables register based QoS control. If register based QoS control feature is enabled, QoS for AXI is set by given qosValVcore0 and qosValVcore1 respectively. */
    Uint32  qosValVcore0; /**< Qos value for VCORE0 */
    Uint32  qosValVcore1; /**< Qos value for VCORE1 */
#ifdef SUPPORT_LOOK_AHEAD_RC
    int  larcData[3];
#endif
} EncParam;

/**
* @brief This structure is used for reporting encoder information. (CODA9 only)
*/
typedef struct {
/**
@verbatim
@* 0 : reporting disable
@* 1 : reporting enable
@endverbatim
*/
    int enable;
    int type;               /**< This value is used for picture type reporting in MVInfo and Sliceinfo. */
    int sz;                 /**< This value means the size for each reporting data (MBinfo, MVinfo, Sliceinfo).    */
    PhysicalAddress addr;   /**< The start address of each reporting buffer into which encoder puts data.  */
} EncReportInfo;

/**
* @brief    This is a data structure for reporting the results of picture encoding operations.
*/
typedef struct {
/**
@verbatim
The physical address of the starting point of newly encoded picture stream

If dynamic buffer allocation is enabled in line-buffer mode, this value is
identical with the specified picture stream buffer address by HOST.
@endverbatim
*/
    PhysicalAddress bitstreamBuffer;
    Uint32 bitstreamSize;   /**< The byte size of encoded bitstream */
    int bitstreamWrapAround;/**< This is a flag to indicate that the write point is wrapped around in bitsteam buffer in case of ring buffer mode. If this flag is 1 in line buffer mode, it indicates fullness of bitstream buffer. Then HOST application needs a larger bitstream buffer. */
    int picType;            /**< <<vpuapi_h_PicType>> */
    int numOfSlices;        /**<  The number of slices of the currently being encoded Picture  */
    int reconFrameIndex;    /**<  A reconstructed frame index. The reconstructed frame can be used for reference of future frame.  */
    FrameBuffer reconFrame; /**<  A reconstructed frame address and information. Please refer to <<vpuapi_h_FrameBuffer>>.  */
    int rdPtr;              /**<  A read pointer in bitstream buffer, which is where HOST has read encoded bitstream from the buffer */
    int wrPtr;              /**<  A write pointer in bitstream buffer, which is where VPU has written encoded bitstream into the buffer  */
    int picSkipped;         /**< A flag which represents whether the current encoding has been skipped or not. (WAVE5 only) */
    int numOfIntra;         /**< The number of intra coded block (WAVE5 HEVC only) */
    int numOfMerge;         /**< The number of merge block in 8x8 (WAVE5 HEVC only) */
    int numOfSkipBlock;     /**< The number of skip block in 8x8 (WAVE5 HEVC only) */
    int avgCtuQp;           /**< The average value of CTU QPs (WAVE5 only) */
    int encPicByte;         /**< The number of encoded picture bytes (WAVE5 only)  */
    int encGopPicIdx;       /**< The GOP index of the currently encoded picture (WAVE5 only) */
    int encPicPoc;          /**< The POC(picture order count) value of the currently encoded picture (WAVE5 only) */
    int releaseSrcFlag;     /**< The source buffer indicator of the encoded pictures(WAVE5 only) */
    int encSrcIdx;          /**< The source buffer index of the currently encoded picture (WAVE5 only) */
    int encNumNut;          /**< The number of nal_unit_type of the currently encoded picture (WAVE5 only) */
    int encVclNut;          /**< The VCL NAL unit type of the currently encoded picture(WAVE5 only). For more information, refer to the RET_QUERY_ENC_VCL_NUT register in programmer's guide. */
    int encPicCnt;          /**< The encoded picture number (WAVE5 only) */
    int errorReason;        /**< The error reason of the currently encoded picture (WAVE5 only) */
    int warnInfo;           /**< The warning information of the currently encoded picture (WAVE5 only) */
    // Report Information
    EncReportInfo mbInfo;   /**<  The parameter for reporting MB data(CODA9 only) . Please refer to <<vpuapi_h_EncReportInfo>> structure. */
    EncReportInfo mvInfo;   /**<  The parameter for reporting motion vector(CODA9 only). Please refer to <<vpuapi_h_EncReportInfo>> structure. */
    EncReportInfo sliceInfo;/**<  The parameter for reporting slice information(CODA9 only). Please refer to <<vpuapi_h_EncReportInfo>> structure. */
    int frameCycle;         /**<  The parameter for reporting the cycle number of encoding one frame.*/

    Uint64 pts;             /**< The PTS(Presentation Timestamp) of the encoded picture which is retrieved and managed from VPU API */
    Uint32   cyclePerTick; /**< The number of cycles per tick */

    Uint32   encHostCmdTick;       /**< Tick of ENC_PIC command for the picture */
    Uint32   encPrepareStartTick;  /**< Start tick of preparing slices of the picture */
    Uint32   encPrepareEndTick;    /**< End tick of preparing slices of the picture  */
    Uint32   encProcessingStartTick; /**< Start tick of processing slices of the picture */
    Uint32   encProcessingEndTick;   /**< End tick of processing slices of the picture */
    Uint32   encEncodeStartTick;     /**< Start tick of encoding slices of the picture */
    Uint32   encEncodeEndTick;       /**< End tick of encoding slices of the picture */

    Uint32   prepareCycle;   /**< The number of cycles for preparing slices of a picture */
    Uint32   processing;     /**< The number of cycles for processing slices of a picture */
    Uint32   EncodedCycle;   /**< The number of cycles for encoding slices of a picture */

    Uint32  picDistortionLow;  /**< The 32bit lowest difference value(SSD) between an original block and a reconstructed block in the encoded picture. It can be an indicator for image quality. Host cannot tune quality by using this value, because this value is not an input parameter, but a reporting value. */
    Uint32  picDistortionHigh; /**< The 32bit highest difference value(SSD) between an original block and a reconstructed block in the encoded picture. It can be an indicator for image quality. Host cannot tune quality by using this value, because this value is not an input parameter, but a reporting value. */
/**
@verbatim
SVC layer type of the encoded picture

@* 0 : BL picture
@* 1 : EL picture

@endverbatim
*/
    Uint32  isSvcLayerEL;
#ifdef SUPPORT_LOOK_AHEAD_RC
    int  larcData[3];
#endif
    RetCode result;  /**< The return value of VPU_EncGetOutputInfo() */
} EncOutputInfo;

/**
 * @brief   This structure is used for adding a header syntax layer into the encoded bitstream.
 The headerType, buf, and zeropaddingEnable are input parameters to VPU. The size
is a returned value from VPU after completing requested operation.
*/
 typedef struct {
    PhysicalAddress buf;        /**< A physical address pointing the generated stream location  */
    size_t  size;               /**< The size of the generated stream in bytes */
    Int32   headerType;         /**< This is a type of header that HOST wants to generate such as <<vpuapi_h_Mp4HeaderType>>, <<vpuapi_h_AvcHeaderType>> or <<vpuapi_h_WaveEncHeaderType>>.   */
    BOOL    zeroPaddingEnable;  /**< It enables header to be padded at the end with zero for byte alignment. (CODA9 only) */
} EncHeaderParam;

/**
 * @brief   This is a special enumeration type for MPEG4 top-level header classes such as
visual sequence header, visual object header and video object layer header.
It is for MPEG4 encoder only.
*/
 typedef enum {
    VOL_HEADER,         /**< Video object layer header */
    VOS_HEADER,         /**< Visual object sequence header */
    VIS_HEADER          /**< Video object header */
} Mp4HeaderType;

/**
 * @brief   This is a special enumeration type for AVC parameter sets such as sequence
parameter set and picture parameter set. It is for AVC encoder only.
*/
typedef enum {
    SPS_RBSP,       /**< Sequence parameter set */
    PPS_RBSP,       /**< Picture parameter set  */
    SPS_RBSP_MVC,   /**< Subset sequence parameter set */
    PPS_RBSP_MVC,   /**< Picture parameter set for dependent view */
} AvcHeaderType;

/**
 * @brief   This is a special enumeration type for explicit encoding headers such as VPS, SPS, PPS. (WAVE encoder only)
*/
typedef enum {
    CODEOPT_ENC_VPS             = (1 << 2), /**< A flag to encode VPS nal unit explicitly */
    CODEOPT_ENC_SPS             = (1 << 3), /**< A flag to encode SPS nal unit explicitly */
    CODEOPT_ENC_PPS             = (1 << 4), /**< A flag to encode PPS nal unit explicitly */
} WaveEncHeaderType;

/**
 * @brief   This is a special enumeration type for NAL unit coding options
*/
typedef enum {
    CODEOPT_ENC_HEADER_IMPLICIT = (1 << 0), /**< A flag to encode (a) headers (VPS, SPS, PPS) implicitly for generating bitstreams conforming to spec. */
    CODEOPT_ENC_VCL             = (1 << 1), /**< A flag to encode VCL nal unit explicitly */
} ENC_PIC_CODE_OPTION;

/**
 * @brief   This is a special enumeration type for defining GOP structure presets.
*/
typedef enum {
    PRESET_IDX_CUSTOM_GOP       = 0,    /**< User defined GOP structure */
    PRESET_IDX_ALL_I            = 1,    /**< All Intra, gopsize = 1 */
    PRESET_IDX_IPP              = 2,    /**< Consecutive P, cyclic gopsize = 1  */
    PRESET_IDX_IBBB             = 3,    /**< Consecutive B, cyclic gopsize = 1  */
    PRESET_IDX_IBPBP            = 4,    /**< gopsize = 2  */
    PRESET_IDX_IBBBP            = 5,    /**< gopsize = 4  */
    PRESET_IDX_IPPPP            = 6,    /**< Consecutive P, cyclic gopsize = 4 */
    PRESET_IDX_IBBBB            = 7,    /**< Consecutive B, cyclic gopsize = 4 */
    PRESET_IDX_RA_IB            = 8,    /**< Random Access, cyclic gopsize = 8 */
} GOP_PRESET_IDX;


#ifdef __cplusplus
extern "C" {
#endif


/**
* @brief
@verbatim
This function initializes VPU hardware and its data
structures/resources. HOST application must call this function only once before calling
VPU_DeInit().

NOTE: Before use, HOST application needs to define the header file path of BIT firmware to BIT_CODE_FILE_PATH.
@endverbatim
*/
 RetCode VPU_Init(
    Uint32 coreIdx  /**<[Input] An index of VPU core. This value can be from 0 to (number of VPU core - 1).*/
    );

/**
* @brief
@verbatim
This function initializes VPU hardware and its data structures/resources.
HOST application must call this function only once before calling VPU_DeInit().

VPU_InitWithBitcodec() is basically same as VPU_Init() except that it takes additional arguments, a buffer pointer where
BIT firmware binary is located and the size.
HOST application can use this function when they wish to load a binary format of BIT firmware,
instead of it including the header file of BIT firmware.
Particularly in multi core running environment with different VPU products,
this function must be used because each core needs to load different firmware.
@endverbatim
*
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means VPU has been initialized successfully.
*RETCODE_CALLED_BEFORE* ::
This function call is invalid which means multiple calls of the current API function
for a given instance are not allowed. In this case, VPU has been already
initialized, so that this function call is meaningless and not allowed anymore.
*RETCODE_NOT_FOUND_BITCODE_PATH* ::
The header file path of BIT firmware has not been defined.
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not received any response from VPU and has timed out.
*RETCODE_FAILURE* ::
Operation was failed.
@endverbatim
*/
RetCode VPU_InitWithBitcode(
    Uint32 coreIdx,         /**< [Input] An index of VPU core */
    const Uint16 *bitcode,  /**< [Input] Buffer where binary format of BIT firmware is located */
    Uint32 sizeInWord       /**< [Input] Size of binary BIT firmware in short integer */
    );

/**
 * @brief This function returns whether VPU is currently running or not.
 * @param coreIdx [Input] An index of VPU core
* @return
@verbatim
@* 0 : VPU is not running.
@* 1 or more : VPU is running.
@endverbatim
 */
Int32  VPU_IsInit(
    Uint32 coreIdx
    );

/**
* @brief    This function frees all the resources allocated by VPUAPI and releases the device driver.
VPU_Init() and VPU_DeInit() always work in pairs.
* @return none
*/
RetCode VPU_DeInit(
    Uint32 coreIdx  /**< [Input] An index of VPU core */
    );


/**
* @brief
@verbatim
This function waits for interrupts to be issued from VPU during the given timeout period. VPU sends an interrupt when
it completes a command or meets an exceptional case. (CODA9 only)

The behavior of this function depends on VDI layer\'s implementation.
Timeout may not work according to implementation of VDI layer.
@endverbatim
* @param coreIdx [Input] An index of VPU core
* @param timeout [Input] Time to wait
* @return
@verbatim
* -1 : timeout
* Non -1 value : The value of InterruptBit
@endverbatim
*/
Int32 VPU_WaitInterrupt(
    Uint32 coreIdx,
    int timeout
    );

/**
* @brief
@verbatim
This function waits for interrupts to be issued from VPU during the given timeout period. VPU sends an interrupt when
it completes a command or meets an exceptional case. (WAVE only)

The behavior of this function depends on VDI layer\'s implementation.
Timeout may not work according to implementation of VDI layer.
@endverbatim
* @param handle [Input] A decoder/encoder handle obtained from VPU_DecOpen()/VPU_EncOpen()
* @param timeout [Input] Time to wait
* @return
@verbatim
* -1 : timeout
* Non -1 value : The value of InterruptBit
@endverbatim
*/
Int32 VPU_WaitInterruptEx(
    VpuHandle handle,
    int timeout
    );

/**
* @brief This function clears VPU interrupts that are pending.
* @return      None
*/
void VPU_ClearInterrupt(
    Uint32 coreIdx  /**< [Input] An index of VPU core */
    );

/**
* @brief This function clears VPU interrupts for a specific instance.
* @return      None
*/
void VPU_ClearInterruptEx(
    VpuHandle handle,  /**< [Input] A decoder/encoder handle obtained from VPU_DecOpen()/VPU_EncOpen() */
    Int32 intrFlag     /**< [Input] An interrupt flag to be cleared */
    );

/**
* @brief
@verbatim

This function stops the current decode or encode operation and resets the internal blocks - BPU_CORE, BPU_BUS, VCE_CORE, and VCE_BUS.
It can be used when VPU is having a longer delay(timeout) or seems hang-up.

*SW_RESET_SAFETY*

SW_RESET_SAFETY moves the context back to the state before calling the current VPU_DecStartOneFrame()/VPU_EncStartOneFrame().
After calling `VPU_SWReset()` with SW_RESET_SAFETY, HOST can resume decoding/encoding from the next picture by calling VPU_DecStartOneFrame()/VPU_EncStartOneFrame().
It works only for the current instance, so this function does not affect other instance\'s running in multi-instance operation.

This is some applicable scenario of using SW_RESET_SAFETY especially for occurrance of hang-up.
For example, when VPU is hung up with frame 1, HOST application calls `VPU_SWReset()` and calls `VPU_DecStartOneFrame()` for frame 2
with specifying the start address and read pointer.
If there is still problem with frame 2, we recommend as below.

@* calling `VPU_SWReset()` with SW_RESET_SAFETY and `seq_init()`
@* calling `VPU_SWReset()` with SW_RESET_SAFETY and enabling iframe search.

*SW_RESET_FORCE*

Unlike the SW_RESET_SAFETY, SW_RESET_FORCE requires to restart the whole process from initialization of VPU
, setting parameters, and registering frame buffers.

@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully.
@endverbatim
 */
 RetCode VPU_SWReset(
    Uint32      coreIdx,    /**<[Input] An index of VPU core */
/**
@verbatim
[Input] Way of reset

@* SW_RESET_SAFETY : It waits for completion of ongoing bus transactions. If remaining bus transactions are done, VPU goes into the reset process. (recommended mode)
@* SW_RESET_FORCE : It forces to do SW_RESET immediately no matter whether bus transactions are completed or not. It might affect what other blocks do with bus. We do not recommend using this mode.
@* SW_RESET_ON_BOOT : This is the default reset mode which is only executed while system boots up. In fact, SW_RESET_ON_BOOT is executed in VPU_Init() and there is no separate use case.
@endverbatim
*/
    SWResetMode resetMode,
    void*       pendingInst /**< [Input] An instance handle */
    );

/**
* @brief
@verbatim
This function resets VPU as VPU_SWReset() does, but
it is done by the system reset signal and all the internal contexts are initialized.
Therefore, HOST application needs to call `VPU_Init()` after `VPU_HWReset()`.

`VPU_HWReset()` requires vdi_hw_reset part of VDI module to be implemented before use.
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully.
@endverbatim
 */
 RetCode VPU_HWReset(
    Uint32  coreIdx     /**<  [Input] An index of VPU core */
    );

/**
* @brief
@verbatim
This function saves or restores context when VPU is powered on or off.

NOTE: This is a tip for safe operation -
    call this function to make VPU enter into a sleep state before power down, and
    after the power off call this function again to return to a wake state.
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully.
@endverbatim
 */
RetCode VPU_SleepWake(
    Uint32  coreIdx,    /**< [Input] An index of VPU core */
/**
@verbatim
[Input]

@* 1 : saves all of the VPU contexts and converts into a sleep state.
@* 0 : restores all of the VPU contexts and converts back to a wake state.
@endverbatim
*/
                int     iSleepWake
    );

/**
 *  @brief  This function returns the product ID of VPU which is currently running.
 *  @return Product information. Please refer to the <<vpuapi_h_ProductId>> enumeration.
 */
int VPU_GetProductId(
    int coreIdx     /**< [Input] VPU core index number */
    );

/**
* @brief This function returns the product information of VPU which is currently running on the system.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means version information is acquired
successfully.
*RETCODE_FAILURE* ::
Operation was failed, which means the current firmware does not contain any version
information.
*RETCODE_NOT_INITIALIZED* ::
VPU was not initialized at all before calling this function. Application should
initialize VPU by calling VPU_Init() before calling this function.
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame-decoding operation should
be completed by calling VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST application should call
VPU_DecGetOutputInfo() to proceed this function call.
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not received any response from VPU and has timed out.
@endverbatim
*/
RetCode VPU_GetVersionInfo(
    Uint32 coreIdx,     /**< [Input] An index of VPU core */
/**
@verbatim
[Output]

@* Version_number[15:12] - Major revision
@* Version_number[11:8] - Hardware minor revision
@* Version_number[7:0] - Software minor revision
@endverbatim
*/
    Uint32* versionInfo,
    Uint32* revision,   /**< [Output] Revision information  */
    Uint32* productId   /**< [Output] Product information. Refer to the <<vpuapi_h_ProductId>> enumeration */
    );

/**
* @brief This function returns the hardware attributes for VPU. <<vpuapi_h_VpuAttr>>
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means version information is acquired
successfully.
@endverbatim
*/
RetCode VPU_GetProductInfo(
    Uint32 coreIdx,       /**< [Input] An index of VPU core */
    VpuAttr* productInfo  /**< [Output] <<vpuapi_h_VpuAttr>> */
    );

/**
* @brief This function returns the number of instances opened.
* @return      The number of instances opened
*/
int VPU_GetOpenInstanceNum(
    Uint32 coreIdx          /**< [Input] An index of VPU core */
    );



/**
*  @brief   This function returns the size of one frame buffer that is required for VPU to decode or encode a frame.
*
*  @return  The size of frame buffer to be allocated
*/
int VPU_GetFrameBufSize(
    VpuHandle   handle,     /**< [Input] A decoder/encoder handle obtained from VPU_DecOpen()/VPU_EncOpen() */
    int         coreIdx,    /**< [Input] VPU core index number */
    int         stride,     /**< [Input] The stride of image  */
    int         height,     /**< [Input] The height of image */
    int         mapType,    /**< [Input] The map type of framebuffer */
    int         format,     /**< [Input] The color format of framebuffer */
    int         interleave, /**< [Input] Whether to use CBCR interleave mode or not */
    DRAMConfig* pDramCfg    /**< [Input] Attributes of DRAM. It is only valid for CODA960. Set NULL for this variable in case of other products. */
    );




// function for decode
/**
*  @brief
This function opens a decoder instance in order to start a new decoder operation.
By calling this function, HOST application can get a handle by which they can refer to a decoder instance.
HOST application needs this kind of handle under multiple instances running codec.
Once HOST application gets a handle, the HOST application must pass this handle to all subsequent decoder-related functions.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means a new decoder instance was created
successfully.
*RETCODE_FAILURE* ::
Operation was failed, which means getting a new decoder instance was not done
successfully. If there is no free instance anymore, this value is returned
in this function call.
*RETCODE_INVALID_PARAM* ::
The given argument parameter, `pop`, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
*RETCODE_NOT_INITIALIZED* ::
This means VPU was not initialized yet before calling this function.
Applications should initialize VPU by calling VPU_Init() before calling this
function.
@endverbatim
 */
 RetCode VPU_DecOpen(
    DecHandle *pHandle,     /**< [Output] A pointer to DecHandle type variable which specifies each instance for HOST application. */
    DecOpenParam *pop       /**< [Input] A pointer to <<vpuapi_h_DecOpenParam>> which describes required parameters for creating a new decoder instance. */
    );

/**
 *  @brief      This function closes the given decoder instance.
 By calling this function, HOST application can end decoding of the sequence and release the instance.
After completion of this function call, relevant resources of the instance get
free. Once HOST application closes an instance, the HOST application cannot call any further
decoder-specific function with the current handle before re-opening a new
decoder instance with the same handle.
*  @return
 @verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means the current decoder instance was closed
successfully.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, handle, was invalid.
This return code might be caused if

* `handle` is not the handle which has been obtained by VPU_DecOpen()
* `handle` is the handle of an instance which has been closed already, etc.
--
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not received any response from VPU and has timed out.
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST application should call
VPU_DecGetOutputInfo() to proceed this function call.

@endverbatim
 */
 RetCode VPU_DecClose(
    DecHandle handle    /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    );

/**
 * @brief
@verbatim
This function decodes the sequence header in the bitstream buffer and returns crucial information for running decode operation such as
the required number of frame buffers.
Applications must pass the address of <<vpuapi_h_DecInitialInfo>> structure, where the
decoder stores information such as picture size, number of necessary frame
buffers, etc. For the details, see definition of <<vpuapi_h_DecInitialInfo>> data structure.
This function should be called once after creating a decoder instance and before starting frame decoding.

It is HOST application\'s responsibility to provide sufficient amount of bitstream to
the decoder so that bitstream buffer does not get empty before this function returns.
If HOST application cannot ensure to feed stream enough, they can use the Forced
Escape option by using VPU_DecSetEscSeqInit().

This function call plays the same role of calling DecIssueSeqInit() and DecCompleteSeqInit().

@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means required information of the stream
data to be decoded was received successfully.
*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for
configuring the decoder.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
The given argument parameter, `info`, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
*RETCODE_FRAME_NOT_COMPLETE* ::
This means that frame decoding operation was not completed yet, so the given API function call cannot be allowed.
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions. In this case, HOST might call this
function before successfully putting bitstream data by calling
VPU_DecUpdateBitstreamBuffer(). In order to perform this functions call,
bitstream data including sequence level header should be transferred into
bitstream buffer before calling VPU_DecGetInitialInfo().
*RETCODE_CALLED_BEFORE* ::
This function call might be invalid, which means multiple calls of the current API
function for a given instance are not allowed. In this case, decoder initial
information has been already received, so that this function call is meaningless
and not allowed anymore.
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not received any response from VPU and has timed out.

@endverbatim
 */
RetCode VPU_DecGetInitialInfo(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    DecInitialInfo* info       /**< [Output] A pointer to <<vpuapi_h_DecInitialInfo>> data structure */
    );

/**
 * @brief
@verbatim
This function starts decoding sequence header. Returning from this function does not mean
the completion of decoding sequence header, and it is just that decoding sequence header was initiated.
Every call of this function should be matched with VPU_DecCompleteSeqInit() with the same handle.
Without calling a pair of these funtions, HOST can not call any other API functions
except for VPU_DecGetBitstreamBuffer(), and VPU_DecUpdateBitstreamBuffer().

A pair of VPU_DecIssueSeqInit() and VPU_DecCompleteSeqInit() or just VPU_DecGetInitialInfo() should be called
at least once after creating a decoder instance and before starting frame decoding.
@endverbatim
*
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means the request for information of the stream data to be
decoded was sent successfully
*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for configuring
the decoder.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid. This return
code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding operation was not completed yet, so the given API function call
cannot be performed this time. A frame decoding operation should be completed by calling
VPU_DecIssueSeqInit(). Even though the result of the current frame operation is not necessary,
HOST should call VPU_DecIssueSeqInit() to proceed this function call.
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed sequences between
API functions. In this case, HOST application might call this function before successfully putting
bitstream data by calling VPU_DecUpdateBitstreamBuffer(). In order to perform this functions
call, bitstream data including sequence level header should be transferred into bitstream buffer
before calling VPU_ DecIssueSeqInit().
*RETCODE_QUEUEING_FAILURE* ::
This means that the current API function call cannot be queued because queue buffers are full at the moment.
@endverbatim
 */
RetCode VPU_DecIssueSeqInit(
    DecHandle handle        /**<  [Input] A decoder handle obtained from VPU_DecOpen() */
    );

/**
 * @brief This function returns the <<vpuapi_h_DecInitialInfo>> structure which holds crucial sequence information for decoder
 such as picture size, number of necessary frame buffers, etc.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means required information of the stream
data to be decoded was received successfully.
*RETCODE_FAILURE* ::
Operation was failed, which means there was a failure in getting sequence information for some reason - syntax errror, unableness to find sequence header, or
missing complete sequence header.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid. This return
code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
The given argument parameter, pInfo, was invalid, which means it has a null pointer, or given
values for some member variables are improper values.
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed sequences between
API functions. It might happen because VPU_DecIssueSeqInit () with the same handle was
not called before calling this function
*RETCODE_CALLED_BEFORE* ::
This function call might be invalid, which means multiple calls of the current API function for a given
instance are not allowed. In this case, decoder initial information has been already received,
so that this function call is meaningless and not allowed anymore.
@endverbatim
 */
RetCode VPU_DecCompleteSeqInit(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    DecInitialInfo* info        /**< [Output] A pointer to <<vpuapi_h_DecInitialInfo>> data structure */
    );

/**
 *  @brief
@verbatim
This is a special function to allow HOST to escape from waiting state while VPU_DecIssueSeqInit()/VPU_DecCompleteSeqInit() are executed.
When escape flag is set to 1 and stream buffer empty happens, VPU terminates VPU_DecIssueSeqInit()/VPU_DecCompleteSeqInit() operation
without issuing empty interrupt.

This function only works in ring buffer mode of bitstream buffer.
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means Force escape flag is successfully set.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid. This return
code might be caused if

* `handle` is not the handle which has been obtained by VPU_DecOpen().
* `handle` is the handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
BitstreamMode of DecOpenParam structure is not BS_MODE_INTERRUPT (ring buffer mode).
@endverbatim
*/
RetCode VPU_DecSetEscSeqInit(
    DecHandle   handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    int         escape      /**< [Input] A flag to enable or disable forced escape from SEQ_INIT */
    );

/**
 * @brief This function is used for registering frame buffers with the acquired
information from VPU_DecGetInitialInfo() or VPU_DecCompleteSeqInit(). The frame buffers pointed to by
bufArray are managed internally within VPU. These include reference
frames, reconstructed frame, etc. `num` must
not be less than `minFrameBufferCount` obtained by VPU_DecGetInitialInfo() or VPU_DecCompleteSeqInit().
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means registering frame buffer
information was done successfully.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means VPU operation was not completed yet, so the given API
function call cannot be performed this time.
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions. HOST might call this
function before calling VPU_DecGetInitialInfo() successfully. This function
should be called after successful calling VPU_DecGetInitialInfo().
*RETCODE_INVALID_FRAME_BUFFER* ::
This happens when pBuffer was invalid, which means pBuffer was not initialized
yet or not valid anymore.
*RETCODE_INSUFFICIENT_FRAME_BUFFERS* ::
This means the given number of frame buffers, num, was not enough for the
decoder operations of the given handle. It should be greater than or equal to
the value   requested by VPU_DecGetInitialInfo().
*RETCODE_INVALID_STRIDE* ::
The given argument stride was invalid, which means it is smaller than the
decoded picture width, or is not a multiple of AXI bus width in this case.
*RETCODE_CALLED_BEFORE* ::
This function call is invalid which means multiple calls of the current API function
for a given instance are not allowed. In this case, registering decoder frame
buffers has been already done, so that this function call is meaningless and not
allowed anymore.
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not recieved any response from VPU and has timed out.
@endverbatim
 */
 RetCode VPU_DecRegisterFrameBuffer(
    DecHandle    handle,    /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    FrameBuffer* bufArray,  /**< [Input] The allocated frame buffer address and information in <<vpuapi_h_FrameBuffer>>. If this parameter is NULL, this function (not HOST application) allocates frame buffers. */
    int          num,       /**< [Input] A number of frame buffers. VPU can allocate frame buffers as many as this given value. */
    int          stride,    /**< [Input] A stride value of the given frame buffers  */
    int          height,    /**< [Input] A frame height   */
    int          mapType    /**< [Input] A Map type for GDI inferface or FBC (Frame Buffer Compression) in <<vpuapi_h_TiledMapType>> */
    );

/**
 * @brief This function is used for registering frame buffers with the acquired
information from VPU_DecGetInitialInfo() or VPU_DecCompleteSeqInit().
This function is functionally same as VPU_DecRegisterFrameBuffer(), but
it can give linear (display) frame buffers and compressed buffers separately with different numbers
unlike the way VPU_DecRegisterFrameBuffer() does.
VPU_DecRegisterFrameBuffer() assigns only the same number of frame buffers for linear buffer and for compressed buffer,
which can take up huge memory space.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means registering frame buffer
information was done successfully.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means VPU operation was not completed yet, so the given API
function call cannot be performed this time.
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions. HOST might call this
function before calling VPU_DecGetInitialInfo() successfully. This function
should be called after successful calling VPU_DecGetInitialInfo().
*RETCODE_INVALID_FRAME_BUFFER* ::
This happens when pBuffer was invalid, which means pBuffer was not initialized
yet or not valid anymore.
*RETCODE_INSUFFICIENT_FRAME_BUFFERS* ::
This means the given number of frame buffers, num, was not enough for the
decoder operations of the given handle. It should be greater than or equal to
the value   requested by VPU_DecGetInitialInfo().
*RETCODE_INVALID_STRIDE* ::
The given argument stride was invalid, which means it is smaller than the
decoded picture width, or is not a multiple of AXI bus width in this case.
*RETCODE_CALLED_BEFORE* ::
This function call is invalid which means multiple calls of the current API function
for a given instance are not allowed. In this case, registering decoder frame
buffers has been already done, so that this function call is meaningless and not
allowed anymore.
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not recieved any response from VPU and has timed out.
@endverbatim
 */
RetCode VPU_DecRegisterFrameBufferEx(
    DecHandle    handle,            /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    FrameBuffer* bufArray,          /**< [Input] The allocated frame buffer address and information in <<vpuapi_h_FrameBuffer>>. If this parameter is NULL, this function (not HOST application) allocates frame buffers.  */
    int          numOfDecFbs,       /**< [Input] The number of compressed frame buffer  */
    int          numOfDisplayFbs,   /**< [Input] The number of linear frame buffer when WTL is enabled. In WAVE, this should be equal to or larger than framebufDelay of <<vpuapi_h_DecInitialInfo>> + 2. */
    int          stride,            /**< [Input] A stride value of the given frame buffers  */
    int          height,            /**< [Input] A frame height   */
    int          mapType            /**< [Input] A Map type for GDI inferface or FBC (Frame Buffer Compression) in <<vpuapi_h_TiledMapType>>   */
    );

/**
 * @brief
This is a special function for VP9 decoder that allows HOST application to replace one of the registered array of frame buffers
with a single new set of frame buffers - linear frame buffer, FBC frame buffer, and ColMv buffer.
This is the dedicated function only for the case that a new inter-frame is coded using a different resolution
than the previous frame.

* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means registering frame buffer
information was done successfully.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_NOT_SUPPORTED_FEATURE* ::
This means that HOST application uses this API call in other than VP9 decoder.
*RETCODE_INSUFFICIENT_RESOURCE* ::
This means failure to allocate a framebuffer due to lack of memory.
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not recieved any response from VPU and has timed out.
@endverbatim
 */
RetCode VPU_DecUpdateFrameBuffer(
    DecHandle     handle, /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    FrameBuffer*  fbcFb,  /**< [Input] The new FBC frame buffer index */
    FrameBuffer*  linearFb, /**< [Input] The new linear frame buffer index */
    Int32         mvColIndex, /**< [Input] The new co-located motion vector buffer index  */
    Int32         picWidth,   /**< [Input] The new frame width */
    Int32         picHeight   /**< [Input] The new frame height */
    );

/**
* @brief
This is a special function that allows HOST to allocate directly the frame buffer
for decoding (Recon) or for display or post-processor unit (PPU) such as Rotator
or Tiled2Linear.
In normal operation, VPU API allocates frame buffers when the argument `bufArray` in VPU_DecRegisterFrameBuffer() is set to 0.
However, for any other reason HOST can use this function to allocate frame buffers by themselves.
 * @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means the framebuffer is allocated successfully.
*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, handle, was invalid. This return code might be caused if
+
--
* `handle` is not the handle which has been obtained by VPU_DecOpen().
* `handle` is the handle of an instance which has been closed already, etc.
--
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed sequences between API functions.
It might happen because VPU_DecRegisterFrameBuffer() for (FramebufferAllocType.FB_TYPE_CODEC) has not been called,
before this function call for allocating frame buffer for PPU (FramebufferAllocType.FB_TYPE_PPU).
*RETCODE_INSUFFICIENT_RESOURCE* ::
This means failure to allocate a framebuffer due to lack of memory
*RETCODE_INVALID_PARAM* ::
The given argument parameter, index, was invalid, which means it has improper values.
@endverbatim

*/
 RetCode VPU_DecAllocateFrameBuffer(
    DecHandle               handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    FrameBufferAllocInfo    info,       /**< [Input] <<vpuapi_h_FrameBufferAllocInfo>> */
    FrameBuffer*            frameBuffer /**< [Output] <<vpuapi_h_FrameBuffer>> structure that holds information of allocated frame buffers */
    );

/**
* @brief
@verbatim
This function returns frame buffer information of the given frame buffer index.

It does not affect actual decoding and simply does obtain the information of frame buffer.
This function is more helpful especially when frame buffers are automatically assigned in VPU_DecRegisterFrameBuffer() and HOST wants to know about the allocated frame buffer.
@endverbatim
*
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means registering frame buffer
information was done successfully.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
The given argument parameter, frameIdx, was invalid, which means frameIdx is larger than allocated framebuffer.
@endverbatim
 */
 RetCode VPU_DecGetFrameBuffer(
    DecHandle   handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    int         frameIdx,   /**< [Input] An index of frame buffer */
    FrameBuffer* frameBuf   /**< [Output] Allocated frame buffer address and information in <<vpuapi_h_FrameBuffer>>.  */
    );

/**
 * @brief   This function starts decoding one frame.
 For the completion of decoding one frame, VPU_DecGetOutputInfo() should be called
with the same handle.
*
* @return
@verbatim
*RETCODE_SUCCESS* ::
+
--
Operation was done successfully, which means decoding a new frame was started
successfully.

NOTE: This return value does not mean that decoding a frame was completed
successfully.
--
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means VPU operation was not completed yet, so the given API
function call cannot be performed this time.
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions. HOST might call this
function before successfully calling VPU_DecRegisterFrameBuffer(). This function
should be called after calling VPU_ DecRegisterFrameBuffer() successfully.
*RETCODE_QUEUEING_FAILURE* ::
This means that the current API function call cannot be queued because queue buffers are full at the moment.
@endverbatim
 */
RetCode VPU_DecStartOneFrame(
    DecHandle handle,   /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    DecParam *param     /**< [Input] <<vpuapi_h_DecParam>> which describes picture decoding parameters for the given decoder instance */
    );

/**
 * @brief   VPU returns the result of frame decoding which
includes information on decoded picture, syntax value, frame buffer, other report values, and etc.
HOST should call this function after frame decoding is finished.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means receiving the output information of the
current frame was done successfully.
*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for
configuring the decoder.
*RETCODE_INVALID_HANDLE* ::
This means argument handle is invalid. This includes cases where `handle` is not
a handle which has been obtained by VPU_DecOpen(), `handle` is a handle to an
instance already closed, or `handle` is a handle to a decoder instance.
*RETCODE_INVALID_PARAM* ::
The given argument parameter, pInfo, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
*RETCODE_QUERY_FAILURE* ::
This means this query command was not successful. (WAVE5 only)
*RETCODE_REPORT_NOT_READY* ::
This means that report is not ready for this query(GET_RESULT) command. (WAVE5 only)

@endverbatim
 */
 RetCode VPU_DecGetOutputInfo(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    DecOutputInfo*  info        /**< [Output] A pointer to <<vpuapi_h_DecOutputInfo>> which describes picture decoding results for the current decoder instance. */
    );

/**
* @brief   This function executes an additional command such to set Secondary AXI or to report user data which is given by HOST application.
It allows HOST application to set directly the variables that can be set only through the API layer.
Some command-specific return codes are also presented.
* @return
@verbatim
*RETCODE_INVALID_COMMAND* ::
The given argument, cmd, was invalid, which means the given cmd was undefined,
or not allowed in the current instance.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means VPU operation was not completed yet, so the given API
function call cannot be performed this time.

*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not received any response from VPU and has timed out.

*RETCODE_QUEUEING_FAILURE* ::
This means that the current API function call cannot be queued because queue buffers are full at the moment.

@endverbatim
*/
 RetCode VPU_DecGiveCommand(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    CodecCommand    cmd,        /**< [Input] A variable specifying the given command of <<vpuapi_h_CodecCommand>> */
    void*           parameter   /**< [Input/Output] A pointer to command-specific data structure which describes picture I/O parameters for the current decoder instance */
    );

/**
* @brief
@verbatim
This function returns the read pointer, write pointer, available space of the bitstream in ringbuffer mode.
Before decoding bitstream, HOST application must feed the decoder with bitstream. To
do that, HOST application must know where to put bitstream and the maximum size.
Applications can get the information by calling this function. This way is more
efficient than providing arbitrary bitstream buffer to the decoder as far as
VPU is concerned.

The given size is the total sum of free space in ring buffer. So when
HOST application downloads this given size of bitstream, Wrptr could meet the end of
stream buffer. In this case, the HOST application should wrap-around the Wrptr back to the
beginning of stream buffer, and download the remaining bits. If not, decoder
operation could be crashed.
@endverbatim
*
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means required information for decoder
stream buffer was received successfully.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not the handle which has been obtained by VPU_DecOpen().
* `handle` is the handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
The given argument parameter, pRdptr, pWrptr or size, was invalid, which means
it has a null pointer, or given values for some member variables are improper
values.
@endverbatim
*/
RetCode VPU_DecGetBitstreamBuffer(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen()   */
    PhysicalAddress *prdPrt,    /**< [Output] A stream buffer read pointer for the current decoder instance  */
    PhysicalAddress *pwrPtr,    /**< [Output] A stream buffer write pointer for the current decoder instance   */
    Uint32          *size       /**< [Output] A variable specifying the available space in bitstream buffer for the current decoder instance   */
    );


/**
* @brief   This function notifies VPU of how much bitstream has been transferred to the bitstream buffer.
By just giving the size as an argument, API automatically handles pointer wrap-around and updates the write
pointer.
*
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means putting new stream data was done successfully.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if

* `handle` is not the handle which has been obtained by VPU_DecOpen().
* `handle` is the handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
The given argument parameter, size, was invalid, which means size is larger than
the value obtained from VPU_DecGetBitstreamBuffer(), or than the available space
in the bitstream buffer.
@endverbatim
*/
RetCode VPU_DecUpdateBitstreamBuffer(
    DecHandle   handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */

/**
@verbatim
[Input]
A variable specifying the amount of bits transferred into bitstream buffer for the current decoder instance.

@* 0 : It means that no more bitstream exists to feed (end of stream).
If 0 is set for `size`, VPU decodes just remaing bitstream and returns -1 to `indexFrameDisplay`.
@* -1: It enables to resume decoding without calling VPU_DecClose()
after remaining stream has completely been decoded to the end of stream by VPU_DecUpdateBitstreamBuffer(handle, 0).
@* -2 : It enables to decode until the current write pointer and force to end decoding.
It is for an exceptional case such as failure of finding sequence header in interrupt mode.
If that happens, VPU is in a state seeking sequence header, while HOST keeps feeding to the end of bistream, but never gets
the command done signal for a long time.

@endverbatim
*/
    int         size
    );

/**
* @brief
This function specifies the location of read pointer in bitstream buffer.
It can also set a write pointer with a same value of read pointer (addr) when updateWrPtr is not a zero value.
This function is used to operate bitstream buffer in PicEnd mode.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means required information of the stream data to be
decoded was received successfully.
*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for configuring
the decoder.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid. This return
code might be caused if

* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding operation was not completed yet, so the given API function call
cannot be performed this time. A frame decoding operation should be completed by calling
VPU_ DecSetRdPtr().
@endverbatim
 */
 RetCode VPU_DecSetRdPtr(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    PhysicalAddress addr,       /**< [Input] Updated read or write pointer */
    int             updateWrPtr /**< [Input] A flag whether to move the write pointer to where the read pointer is located */
    );


/**
* @brief
@verbatim
This function flushes all of the decoded framebuffer contexts that remain in decoder firmware.
It can be used to do random access (like skip picture) or to continue seamless decode operation without calling VPU_DecClose()
after change of sequence.

NOTE: In WAVE, this function returns all of the decoded framebuffer contexts that remain.
     pRetNum always has 0 in CODA9.
@endverbatim
*
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means receiving the output information of the
current frame was done successfully.
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST should call
VPU_DecGetOutputInfo() to proceed this function call.
*RETCODE_INVALID_HANDLE* ::
This means argument handle is invalid. This includes cases where `handle` is not
a handle which has been obtained by VPU_DecOpen(), `handle` is a handle to an
instance already closed, or `handle` is a handle to an decoder instance.
Also,this value is returned when VPU_DecStartOneFrame() is matched with
VPU_DecGetOutputInfo() with different handles.
*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not recieved any response from VPU and has timed out.
@endverbatim
 */
 RetCode VPU_DecFrameBufferFlush(
    DecHandle       handle,         /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    DecOutputInfo*  pRemainingInfo, /**< [Output] All of the decoded framebuffer contexts are stored in display order as array of <<vpuapi_h_DecOutputInfo>>.
                                            If this is NULL, the remaining information is not returned.  */
    Uint32*         pRetNum         /**< [Output] The number of the decoded frame buffer contexts. It this is null, the information is not returned.  */
    );

/**
* @brief   This function clears a display flag of the given index of frame buffer.
If the display flag of frame buffer is cleared, the
frame buffer can be reused in the decoding process.
VPU API keeps the display index of frame buffer remained until VPU_DecClrDispFlag() is called.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means receiving the output information of the
current frame was done successfully.
*RETCODE_INVALID_HANDLE* ::
This means argument handle is invalid. This includes cases where `handle` is not
a handle which has been obtained by VPU_DecOpen(), `handle` is a handle to an
instance already closed, or `handle` is a handle to an decoder instance.
Also,this value is returned when VPU_DecStartOneFrame() is matched with
VPU_DecGetOutputInfo() with different handles.
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions.
It might happen because VPU_DecRegisterFrameBuffer() with the
same handle was not called before calling this function.
*RETCODE_INVALID_PARAM* ::
The given argument parameter, index, was invalid, which means it has improper
values.
@endverbatim
 */
 RetCode VPU_DecClrDispFlag(
    DecHandle   handle, /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    int         index   /**< [Input] A frame buffer index to be cleared */
    );

// function for encode
/**
* @brief    This function opens an encoder instance in order to start a new encoder operation.
By calling this function, HOST application can get a
handle specifying a new encoder instance. Because VPU supports multiple
instances of codec operations, HOST application needs this kind of handles for the
all codec instances now on running. Once HOST application gets a handle, the HOST application
must use this handle to represent the target instances for all subsequent
encoder-related functions.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means a new encoder instance was opened
successfully.

*RETCODE_FAILURE* ::
Operation was failed, which means getting a new encoder instance was not done
successfully. If there is no free instance anymore, this value is returned
in this function call.

*RETCODE_INVALID_PARAM* ::
The given argument parameter, pOpenParam, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.

*RETCODE_NOT_INITIALIZED* ::
VPU was not initialized at all before calling this function. Application should
initialize VPU by calling VPU_Init() before calling this function.
@endverbatim
*/

RetCode VPU_EncOpen(
    EncHandle*      handle,     /**< [Output] A pointer to EncHandle type variable which specifies each instance for HOST application. If no instance is available, null handle is returned. */
    EncOpenParam*   encOpParam  /**< [Input] A pointer to <<vpuapi_h_EncOpenParam>> structure which describes required parameters for creating a new encoder instance. */
    );

/**
* @brief      This function closes the given encoder instance.
 By calling this function, HOST application can end encoding of the sequence and release the instance.
After completion of this function call, relevant resources of the instance get
free. Once HOST application closes an instance, the HOST application cannot call any further
encoder-specific function with the current handle before re-opening a new
encoder instance with the same handle.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully. That means the current encoder instance was closed
successfully.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if
+
--
* pHandle is not a handle which has been obtained by VPU_EncOpen().
* pHandle is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, HOST application should call VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo() to proceed this function call.

*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not recieved any response from VPU and has timed out.
@endverbatim
*/
RetCode VPU_EncClose(
    EncHandle handle    /**< [Input] An encoder handle obtained from VPU_EncOpen() */
    );

/**
* @brief    This function sets sequence information including source width and height
 and many other parameters such as coding tools, GOP preset, rate control, etc.
It also returns the required parameters such as minFrameBufferCount. (CODA9 only)
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means receiving the initial parameters
was done successfully.

*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for
configuring the encoder.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if

* pHandle is not a handle which has been obtained by VPU_EncOpen().
* pHandle is a handle of an instance which has been closed already, etc.

*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, HOST application should call VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo() to proceed this function call.

*RETCODE_INVALID_PARAM* ::
The given argument parameter, pInitialInfo, was invalid, which means it has a
null pointer, or given values for some member variables are improper values.

*RETCODE_CALLED_BEFORE* ::
This function call is invalid which means multiple calls of the current API function
for a given instance are not allowed. In this case, encoder initial information
has been received already, so that this function call is meaningless and not
allowed anymore.

*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not recieved any response from VPU and has timed out.
@endverbatim
*/
RetCode VPU_EncGetInitialInfo(
    EncHandle       handle,     /**< [Input] An encoder handle obtained from VPU_EncOpen() */
    EncInitialInfo* encInitInfo /**< [Output] A pointer to <<vpuapi_h_EncInitialInfo>> structure which describes required sequence information for the current encoder instance.  */
    );

/**
* @brief    Before starting encoder operation, HOST application should set sequence information including source width and height
 and many other parameters such as coding tools, GOP preset, rate control, etc. (WAVE only)
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means receiving the initial parameters
was done successfully.

*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for
configuring the encoder.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if

* pHandle is not a handle which has been obtained by VPU_EncOpen().
* pHandle is a handle of an instance which has been closed already, etc.

*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, HOST application should call VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo() to proceed this function call.

*RETCODE_INVALID_PARAM* ::
The given argument parameter, pInitialInfo, was invalid, which means it has a
null pointer, or given values for some member variables are improper values.

*RETCODE_CALLED_BEFORE* ::
This function call is invalid which means multiple calls of the current API function
for a given instance are not allowed. In this case, encoder initial information
has been received already, so that this function call is meaningless and not
allowed anymore.

*RETCODE_QUEUEING_FAILURE* ::
This means that the current API function call cannot be queued because queue buffers are full at the moment.
@endverbatim
*/
RetCode VPU_EncIssueSeqInit(
    EncHandle handle           /**<  [Input] A encoder handle obtained from VPU_EncOpen() */
    );

/**
* @brief    Before starting encoder operation, HOST application must allocate frame buffers
according to the information obtained from this function. This function returns the required parameters such as minFrameBufferCount. (WAVE only)
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means receiving the initial parameters
was done successfully.

*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for
configuring the encoder.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if

* pHandle is not a handle which has been obtained by VPU_EncOpen().
* pHandle is a handle of an instance which has been closed already, etc.

*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, HOST application should call VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo() to proceed this function call.

*RETCODE_INVALID_PARAM* ::
The given argument parameter, pInitialInfo, was invalid, which means it has a
null pointer, or given values for some member variables are improper values.

*RETCODE_CALLED_BEFORE* ::
This function call is invalid which means multiple calls of the current API function
for a given instance are not allowed. In this case, encoder initial information
has been received already, so that this function call is meaningless and not
allowed anymore.

@endverbatim
*/
RetCode VPU_EncCompleteSeqInit(
    EncHandle handle,           /**<  [Input] A encoder handle obtained from VPU_EncOpen() */
    EncInitialInfo * info       /**<  [Output] A pointer to <<vpuapi_h_EncInitialInfo>> structure which describes required sequence information for the current encoder instance. */
    );

/**
* @brief
@verbatim
This function registers frame buffers requested by VPU_EncGetInitialInfo(). The
frame buffers pointed to by pBuffer are managed internally within VPU.
These include reference frames, reconstructed frames, etc. Applications must not
change the contents of the array of frame buffers during the life time of the
instance, and `num` must not be less than minFrameBufferCount obtained by
VPU_EncGetInitialInfo().

The distance between a pixel in a row and the corresponding pixel in the next
row is called a stride. The value of stride must be a multiple of 8. The
address of the first pixel in the second row does not necessarily coincide with
the value next to the last pixel in the first row. In other words, a stride can
be greater than the picture width in pixels.

Applications should not set a stride value smaller than the picture width.
So, for Y component, HOST application must allocate at least a space of size
(frame height \* stride), and Cb or Cr component,
(frame height/2 \* stride/2), respectively.
But make sure that in Cb/Cr non-interleave (separate Cb/Cr) map,
a stride for the luminance frame buffer should be multiple of 16 so that a stride
for the chrominance frame buffer becomes a multiple of 8.
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means registering frame buffers were done
successfully.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if

* handle is not a handle which has been obtained by VPU_EncOpen().
* handle is a handle of an instance which has been closed already, etc.

*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, HOST application should call VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo() to proceed this function call.

*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions. HOST application might call this
function before calling VPU_EncGetInitialInfo() successfully. This function
should be called after successful calling of VPU_EncGetInitialInfo().

*RETCODE_INVALID_FRAME_BUFFER* ::
This means argument pBuffer were invalid, which means it was not initialized
yet or not valid anymore.

*RETCODE_INSUFFICIENT_FRAME_BUFFERS* ::
This means the given number of frame buffers, num, was not enough for the
encoder operations of the given handle. It should be greater than or equal to
the value of minFrameBufferCount obtained from VPU_EncGetInitialInfo().

*RETCODE_INVALID_STRIDE* ::
This means the given argument stride was invalid, which means it is 0, or is not
a multiple of 8 in this case.

*RETCODE_CALLED_BEFORE* ::
This function call is invalid which means multiple calls of the current API function
for a given instance are not allowed. It might happen when registering frame buffer for
this instance has been done already so that this function call is meaningless
and not allowed anymore.

*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not recieved any response from VPU and has timed out.
@endverbatim
*/
RetCode VPU_EncRegisterFrameBuffer(
    EncHandle       handle,     /**< [Input] An encoder handle obtained from VPU_EncOpen() */
    FrameBuffer*    bufArray,   /**< [Input] Allocated frame buffer address and information in <<vpuapi_h_FrameBuffer>>. If this parameter is set to -1, VPU allocates frame buffers. */
    int             num,        /**< [Input] A number of frame buffers. VPU can allocate frame buffers as many as this given value. */
    int             stride,     /**< [Input] A stride value of the given frame buffers */
    int             height,     /**< [Input] Frame height */
    TiledMapType    mapType     /**< [Input] Map type of frame buffer */
    );

/**
* @brief    This is a special function that enables HOST application to allocate directly the frame buffer
for encoding or for Pre-processor (PRP) such as Rotator.
In normal operation, VPU API allocates frame buffers when the argument `bufArray` in VPU_EncRegisterFrameBuffer() is set to 0.
However, for any other reason HOST application can use this function to allocate frame buffers by themselves.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means the framebuffer is allocated successfully.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid. This return code might be caused if
+
--
* handle is not a handle which has been obtained by VPU_EncOpen().
* handle is a handle of an instance which has been closed already, etc.
--
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed sequences between API functions.
It might happen because VPU_EncRegisterFrameBuffer() for (FramebufferAllocType.FB_TYPE_CODEC) has not been called,
before this function call for allocating frame buffer for PRP (FramebufferAllocType.FB_TYPE_PPU).

*RETCODE_INSUFFICIENT_RESOURCE* ::
This means failure to allocate a framebuffer due to lack of memory

*RETCODE_INVALID_PARAM* ::
The given argument parameter, index, was invalid, which means it has improper values
@endverbatim
*/
RetCode VPU_EncAllocateFrameBuffer(
    EncHandle handle,           /**< [Input] An encoder handle obtained from VPU_EncOpen()  */
    FrameBufferAllocInfo info,  /**< [Input] <<vpuapi_h_FrameBufferAllocInfo>>  */
    FrameBuffer *frameBuffer    /**< [Output] <<vpuapi_h_FrameBuffer>> that holds information of allocated frame buffers  */
    );


/**
* @brief
@verbatim
This function starts encoding one frame. Returning from this function does not
mean the completion of encoding one frame, and it is just that encoding one
frame was initiated.

Every call of this function should be matched with VPU_EncGetOutputInfo()
with the same handle. In other words, HOST application should call VPU_EncGetOutputInfo() once to get the result of VPU_EncStartOneFrame() call.

For CODA9, without "sequential" calling a pair of VPU_EncStartOneFrame() and VPU_EncGetOutputInfo(),
HOST application cannot call any other API functions except VPU_EncGetBitstreamBuffer(), and VPU_EncUpdateBitstreamBuffer().
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means encoding a new frame was started
successfully.
+
--
NOTE: This return value does not mean that encoding a frame was completed
successfully.
--
*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if
+
--
* handle is not a handle which has been obtained by VPU_EncOpen().
* handle is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, HOST application should call VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo() to proceed this function call.

*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions. In this case, HOST application might call this
function before successfully calling VPU_EncRegisterFrameBuffer(). This function
should be called after successfully calling VPU_EncRegisterFrameBuffer().

*RETCODE_INVALID_PARAM* ::
The given argument parameter, `parameter`, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.

*RETCODE_INVALID_FRAME_BUFFER* ::
This means sourceFrame in input structure EncParam was invalid, which means
sourceFrame was not valid even though picture-skip is disabled.

*RETCODE_QUEUEING_FAILURE* ::
This means that the current API function call cannot be queued because queue buffers are full at the moment.

@endverbatim
*/
RetCode VPU_EncStartOneFrame(
    EncHandle handle,   /**< [Input] An encoder handle obtained from VPU_EncOpen()  */
    EncParam * param    /**< [Input] A pointer to <<vpuapi_h_EncParam>> structure which describes picture encoding parameters for the current encoder instance.  */
    );

/**
* @brief    This function gets information of the output of encoding. Application can obtain
the picture type, the address and size of the generated bitstream, and the number
of generated slices. HOST application should call this function after frame
encoding is finished, and before starting the further processing.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means the output information of the current
frame encoding was received successfully.

*RETCODE_INVALID_HANDLE* ::
The given handle for the current API function call, pHandle, was invalid. This
return code might be caused if
+
--
* handle is not a handle which has been obtained by VPU_EncOpen(), for example
a decoder handle,
* handle is a handle of an instance which has been closed already,
* handle is not the same handle as the last VPU_EncStartOneFrame() has, etc.
--
*RETCODE_WRONG_CALL_SEQUENCE* ::
This means the current API function call was invalid considering the allowed
sequences between API functions. HOST application might call this
function before calling VPU_EncStartOneFrame() successfully. This function
should be called after successful calling of VPU_EncStartOneFrame().

*RETCODE_INVALID_PARAM* ::
The given argument parameter, pInfo, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.

*RETCODE_QUERY_FAILURE* ::
This means this query command was not successful. (WAVE5 only)

*RETCODE_REPORT_NOT_READY* ::
This means that report is not ready for this query(GET_RESULT) command. (WAVE5 only)

@endverbatim
*/
RetCode VPU_EncGetOutputInfo(
    EncHandle handle,       /**<  [Input] An encoder handle obtained from VPU_EncOpen().  */
    EncOutputInfo * info    /**<  [Output] A pointer to <<vpuapi_h_EncOutputInfo>> structure which describes picture encoding results for the current encoder instance. */
    );

/**
* @brief    This function executes an additional function such rotator or changeParam which is given by HOST application.
It allows HOST application to set directly the variables that can be set only through the API layer.
Some command-specific return codes are also presented.
* @return
@verbatim
*RETCODE_INVALID_COMMAND* ::
This means the given argument, cmd, was invalid which means the given cmd was
undefined, or not allowed in the current instance.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if
+
--
* pHandle is not a handle which has been obtained by VPU_EncOpen().
* pHandle is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, HOST application should call VPU_EncGetOutputInfo() or
VPU_DecGetOutputInfo() to proceed this function call.

*RETCODE_VPU_RESPONSE_TIMEOUT* ::
Operation has not received any response from VPU and has timed out.

*RETCODE_QUEUEING_FAILURE* ::
This means that the current API function call cannot be queued because queue buffers are full at the moment.

@endverbatim
*/
RetCode VPU_EncGiveCommand(
    EncHandle handle,   /**< [Input] An encoder handle obtained from VPU_EncOpen() */
    CodecCommand cmd,   /**< [Input] A variable specifying the given command of <<vpuapi_h_CodecCommand>> */
    void * parameter    /**< [Input/Output] A pointer to command-specific data structure which describes picture I/O parameters for the current encoder instance */
    );

/**
* @brief    After encoding a frame, HOST application must get the bitstream from encoder.
This function returns the location of encoded stream and the maximum size in bitstream buffer.

* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully. That means the current encoder instance was closed
successfully.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if
+
--
* pHandle is not a handle which has been obtained by VPU_EncOpen().
* pHandle is a handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
The given argument parameter, pRdptr, pWrptr or size, was invalid, which means
it has a null pointer, or given values for some member variables are improper
values.
@endverbatim
*/
RetCode VPU_EncGetBitstreamBuffer(
    EncHandle handle,           /**<  [Input] An encoder handle obtained from VPU_EncOpen()  */
    PhysicalAddress *prdPrt,    /**<  [Output] A stream buffer read pointer for the current encoder instance */
    PhysicalAddress *pwrPtr,    /**<  [Output] A stream buffer write pointer for the current encoder instance */
    int * size                  /**<  [Output] A variable indicating the written stream size in bitstream buffer for the current encoder instance */
    );

/**
* @brief    This function informs VPU API of how much bitstream has been transferred by HOST application from
the address obtained from VPU_EncGetBitstreamBuffer(). By just giving the size
as an argument, API automatically handles pointer wrap-around and updates the read
pointer.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully. That means the current encoder instance was closed
successfully.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, pHandle, was invalid.
This return code might be caused if
+
--
* pHandle is not a handle which has been obtained by VPU_EncOpen().
* pHandle is a handle of an instance which has been closed already, etc.
--
*RETCODE_INVALID_PARAM* ::
The given argument parameter, size, was invalid, which means size is larger than
the value obtained from VPU_EncGetBitstreamBuffer().
@endverbatim
*/
RetCode VPU_EncUpdateBitstreamBuffer(
    EncHandle handle,   /**< [Input] An encoder handle obtained from VPU_EncOpen() */
    int size            /**< [Input] A variable indicating the stream size read by HOST application from the bitstream buffer  */
    );

/**
* @brief    This function specifies the location of write pointer in bitstream buffer. It can also set a read
pointer with the same value of write pointer (addr) when updateRdPtr is not a zero value.
This function can be used regardless of bitstream buffer mode.
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means required information of the stream data to be
encoded was received successfully.
*RETCODE_FAILURE* ::
Operation was failed, which means there was an error in getting information for configuring
the encoder.
*RETCODE_INVALID_HANDLE* ::
+
--
This means the given handle for the current API function call, `handle`, was invalid. This return
code might be caused if

* `handle` is not a handle which has been obtained by VPU_EncOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--
*RETCODE_FRAME_NOT_COMPLETE* ::
This means frame encoding operation was not completed yet, so the given API function call
cannot be performed this time. A frame encoding operation should be completed by calling
VPU_ EncSetRdPtr ().
@endverbatim
*/
RetCode VPU_EncSetWrPtr(
    EncHandle handle,       /**< [Input] An encoder handle obtained from VPU_EncOpen()  */
    PhysicalAddress addr,   /**< [Input] Updated write pointer */
    int updateRdPtr         /**< [Input] A flag whether to move the read pointer to where the write pointer is located */
    );


#ifdef __cplusplus
}
#endif

#endif

