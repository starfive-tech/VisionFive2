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

#ifndef VPUAPI_H_INCLUDED
#define VPUAPI_H_INCLUDED

#include "vpuconfig.h" 
#include "vputypes.h"
#include "../vdi/vdi.h"
#include "../vdi/vdi_osal.h"
#include "vpuerror.h"

#ifdef SUPPORT_980_RC_LIB
#include "coda9/980_roi_rc_lib/util_float.h"
#include "coda9/980_roi_rc_lib/hevc_enc_rc.h"
#include "coda9/980_roi_rc_lib/frm_rc.h"
#endif


#define MAX_GDI_IDX                             31
#ifdef SUPPORT_SRC_BUF_CONTROL
#define MAX_REG_FRAME                           2000
#else
#define MAX_REG_FRAME                           MAX_GDI_IDX*2 // 2 for WTL
#endif

#define WAVE4_FBC_LUMA_TABLE_SIZE(_w, _h)       (((_h+15)/16)*((_w+255)/256)*128)
#define WAVE4_FBC_CHROMA_TABLE_SIZE(_w, _h)     (((_h+15)/16)*((_w/2+255)/256)*128)
#define WAVE4_DEC_HEVC_MVCOL_BUF_SIZE(_w, _h)   (((_w+63)/64)*((_h+63)/64)*256+ 64)     
#define WAVE4_ENC_HEVC_MVCOL_BUF_SIZE(_w, _h)   (((_w+63)/64)*((_h+63)/64)*128)          // encoder case = (((_w+63)/64)*((_h+63)/64)128).
#define WAVE4_DEC_VP9_MVCOL_BUF_SIZE(_w, _h)    (((VPU_ALIGN64(_w)*VPU_ALIGN64(_h))>>2))

#define WAVE5_ENC_HEVC_MVCOL_BUF_SIZE(_w, _h)   (VPU_ALIGN64(_w)/64*VPU_ALIGN64(_h)/64*128)
#define WAVE5_FBC_LUMA_TABLE_SIZE(_w, _h)       (VPU_ALIGN16(_h)*VPU_ALIGN256(_w)/32)
#define WAVE5_FBC_CHROMA_TABLE_SIZE(_w, _h)     (VPU_ALIGN16(_h)*VPU_ALIGN256(_w/2)/32)

//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------
/**
* @brief 
@verbatim 
This is an enumeration for declaring codec standard type variables. Currently,
VPU supports many different video standards such as H.265/HEVC, MPEG4 SP/ASP, H.263 Profile 3, H.264/AVC
BP/MP/HP, VC1 SP/MP/AP, Divx3, MPEG1, MPEG2, AVS, RealVideo 8/9/10, AVS Jizhun/Guangdian profile,
 Theora, VP3, VP8, and VP9.  

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
    OPT_CHANGE_PARAM    = 16 /**< SET_PARAM command option for parameters change (WAVE520 only)  */
} SET_PARAM_OPTION;

/**
* @brief 
@verbatim 
This is an enumeration for declaring SET_PARAM command options. (CODA7Q encoder only)
Depending on this, SET_PARAM command parameter registers have different settings.

@endverbatim
*/
typedef enum {
    C7_OPT_COMMON       = 16,
    C7_OPT_SET_PARAM    = 17,
    C7_OPT_PARAM_CHANGE = 18,
} C7_SET_PARAM_OPTION;

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
} DEC_PIC_OPTION;

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
/* VP9 */
#define VP9_PROFILE_0                       0
#define VP9_PROFILE_1                       1
#define VP9_PROFILE_2                       2
#define VP9_PROFILE_3                       3
/* Tier */
#define HEVC_TIER_MAIN                      0
#define HEVC_TIER_HIGH                      1
/* Level */
#define HEVC_LEVEL(_Major, _Minor)          (_Major*10+_Minor)

/* H.264 */
#define H264_PROFILE_BASELINE               66
#define H264_PROFILE_MAIN                   77
#define H264_PROFILE_EXTENDED               88
#define H264_PROFILE_HIGH                   100
#define H264_PROFILE_HIGH10                 110
#define H264_PROFILE_HIGH10_INTRA           120
#define H264_PROFILE_HIGH422                122
#define H264_PROFILE_HIGH444                244
#define H264_PROFILE_CAVLC_444_INTRA        44

/* H265 USER_DATA(SPS & SEI) ENABLE FLAG */
#define H265_USERDATA_FLAG_RESERVED_0           (0)
#define H265_USERDATA_FLAG_RESERVED_1           (1)
#define H265_USERDATA_FLAG_VUI                  (2)
#define H265_USERDATA_FLAG_RESERVED_3           (3)
#define H265_USERDATA_FLAG_PIC_TIMING           (4)
#define H265_USERDATA_FLAG_ITU_T_T35_PRE        (5)  /* SEI Prefix: user_data_registered_itu_t_t35 */
#define H265_USERDATA_FLAG_UNREGISTERED_PRE     (6)  /* SEI Prefix: user_data_unregistered */
#define H265_USERDATA_FLAG_ITU_T_T35_SUF        (7)  /* SEI Suffix: user_data_registered_itu_t_t35 */
#define H265_USERDATA_FLAG_UNREGISTERED_SUF     (8)  /* SEI Suffix: user_data_unregistered */
#define H265_USERDATA_FLAG_RESERVED_9           (9)  /* SEI RESERVED */
#define H265_USERDATA_FLAG_MASTERING_COLOR_VOL  (10) /* SEI Prefix: mastering_display_color_volume */
#define H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT  (11) /* SEI Prefix: chroma_resampling_filter_hint */
#define H265_USERDATA_FLAG_KNEE_FUNCTION_INFO   (12) /* SEI Prefix: knee_function_info */

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
    RETCODE_QUERY_FAILURE,              /**< This means that query command was not successful (WAVE5 only) */
    RETCODE_QUEUEING_FAILURE,           /**< This means that commands cannot be queued (WAVE5 only) */
    RETCODE_VPU_STILL_RUNNING,          /**< This means that VPU cannot be flushed or closed now. because VPU is running (WAVE5 only) */
    RETCODE_REPORT_NOT_READY,           /**< This means that report is not ready for Query(GET_RESULT) command (WAVE5 only) */
#ifdef AUTO_FRM_SKIP_DROP
    RETCODE_FRAME_DROP,                 /**< This means that frame is dropped. HOST application don't have to wait INT_BIT_PIC_RUN.  (CODA9 only) */
#endif
} RetCode;

/************************************************************************/
/* Utility macros                                                       */
/************************************************************************/
#define VPU_ALIGN4(_x)              (((_x)+0x03)&~0x03)
#define VPU_ALIGN8(_x)              (((_x)+0x07)&~0x07)
#define VPU_ALIGN16(_x)             (((_x)+0x0f)&~0x0f)
#define VPU_ALIGN32(_x)             (((_x)+0x1f)&~0x1f)
#define VPU_ALIGN64(_x)             (((_x)+0x3f)&~0x3f)
#define VPU_ALIGN128(_x)            (((_x)+0x7f)&~0x7f)
#define VPU_ALIGN256(_x)            (((_x)+0xff)&~0xff)
#define VPU_ALIGN512(_x)            (((_x)+0x1ff)&~0x1ff)
#define VPU_ALIGN4096(_x)           (((_x)+0xfff)&~0xfff)
#define VPU_ALIGN16384(_x)          (((_x)+0x3fff)&~0x3fff)

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define INTERRUPT_TIMEOUT_VALUE     (Uint32)-1

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

#define DISPLAY_IDX_FLAG_SEQ_END    -1
#define DISPLAY_IDX_FLAG_NO_FB      -3
#define DECODED_IDX_FLAG_NO_FB      -1
#define DECODED_IDX_FLAG_SKIP       -2

#define MAX_ROI_NUMBER  64
#define MAX_CODA980_ROI_NUMBER 10

/**
* @brief    This is a special enumeration type for some configuration commands 
* which can be issued to VPU by HOST application. Most of these commands can be called occasionally, 
* not periodically for changing the configuration of decoder or encoder operation running on VPU. 
* Details of these commands are presented in <<vpuapi_h_VPU_DecGiveCommand>>.
*/
typedef enum {
    ENABLE_ROTATION,
    DISABLE_ROTATION,
    ENABLE_MIRRORING,
    DISABLE_MIRRORING,
    SET_MIRROR_DIRECTION,
    SET_ROTATION_ANGLE,
    SET_ROTATOR_OUTPUT,
    SET_ROTATOR_STRIDE,
    DEC_GET_SEQ_INFO,
    DEC_SET_SPS_RBSP,
    DEC_SET_PPS_RBSP,
    DEC_SET_SEQ_CHANGE_MASK,
    ENABLE_DERING,
    DISABLE_DERING,
    SET_SEC_AXI,
    SET_DRAM_CONFIG,    //coda960 only
    GET_DRAM_CONFIG,    //coda960 only
    ENABLE_REP_USERDATA,
    DISABLE_REP_USERDATA,
    SET_ADDR_REP_USERDATA,
    SET_VIRT_ADDR_REP_USERDATA,
    SET_SIZE_REP_USERDATA,
    SET_USERDATA_REPORT_MODE,
    // WAVE4 CU REPORT INFTERFACE 
    ENABLE_REP_CUDATA,
    DISABLE_REP_CUDATA,
    SET_ADDR_REP_CUDATA,
    SET_SIZE_REP_CUDATA,
    SET_CACHE_CONFIG,
    GET_TILEDMAP_CONFIG,
    SET_LOW_DELAY_CONFIG,
    GET_LOW_DELAY_OUTPUT,
    SET_DECODE_FLUSH,
    DEC_SET_FRAME_DELAY,
    DEC_SET_WTL_FRAME_FORMAT,
    DEC_GET_FIELD_PIC_TYPE,
    DEC_GET_DISPLAY_OUTPUT_INFO,
    DEC_ENABLE_REORDER,
    DEC_DISABLE_REORDER,
    DEC_SET_AVC_ERROR_CONCEAL_MODE,
    DEC_FREE_FRAME_BUFFER,
    DEC_GET_FRAMEBUF_INFO,   
    DEC_RESET_FRAMEBUF_INFO, 
    ENABLE_DEC_THUMBNAIL_MODE,
    DEC_SET_DISPLAY_FLAG,
    DEC_SET_TARGET_TEMPORAL_ID,             //!<< H.265 temporal scalability
    DEC_SET_BWB_CUR_FRAME_IDX,
    DEC_SET_FBC_CUR_FRAME_IDX,
    DEC_SET_INTER_RES_INFO_ON,
    DEC_SET_INTER_RES_INFO_OFF,
    DEC_FREE_FBC_TABLE_BUFFER,
    DEC_FREE_MV_BUFFER,
    DEC_ALLOC_FBC_Y_TABLE_BUFFER,
    DEC_ALLOC_FBC_C_TABLE_BUFFER,
    DEC_ALLOC_MV_BUFFER,
    //vpu put header stream to bitstream buffer
    ENC_PUT_VIDEO_HEADER,
    ENC_PUT_MP4_HEADER,
    ENC_PUT_AVC_HEADER,
    //host generate header bitstream
    ENC_GET_VIDEO_HEADER,
    ENC_SET_INTRA_MB_REFRESH_NUMBER,
    ENC_ENABLE_HEC,
    ENC_DISABLE_HEC,
    ENC_SET_SLICE_INFO,
    ENC_SET_GOP_NUMBER,
    ENC_SET_INTRA_QP,
    ENC_SET_BITRATE,
    ENC_SET_FRAME_RATE,

    ENC_SET_PARA_CHANGE,
    ENABLE_LOGGING,
    DISABLE_LOGGING,
    ENC_SET_SEI_NAL_DATA,
    DEC_GET_QUEUE_STATUS,
    ENC_GET_BW_REPORT,  // wave520 only
    ENC_GET_FRAMEBUF_IDX,
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
    MIRDIR_NONE,
    MIRDIR_VER,
    MIRDIR_HOR,
    MIRDIR_HOR_VER
} MirrorDirection;

/**
 * @brief   This is an enumeration type for representing chroma formats of the frame buffer and pixel formats in packed mode.
 */ 
 typedef enum {
    FORMAT_420             = 0  ,      /* 8bit */
    FORMAT_422               ,         /* 8bit */
    FORMAT_224               ,         /* 8bit */
    FORMAT_444               ,         /* 8bit */
    FORMAT_400               ,         /* 8bit */

                                       /* Little Endian Perspective     */
                                       /*     | addr 0  | addr 1  |     */
    FORMAT_420_P10_16BIT_MSB = 5 ,     /* lsb | 00xxxxx |xxxxxxxx | msb */
    FORMAT_420_P10_16BIT_LSB ,         /* lsb | xxxxxxx |xxxxxx00 | msb */
    FORMAT_420_P10_32BIT_MSB ,         /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */
    FORMAT_420_P10_32BIT_LSB ,         /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */

                                       /* 4:2:2 packed format */
                                       /* Little Endian Perspective     */
                                       /*     | addr 0  | addr 1  |     */
    FORMAT_422_P10_16BIT_MSB ,         /* lsb | 00xxxxx |xxxxxxxx | msb */
    FORMAT_422_P10_16BIT_LSB ,         /* lsb | xxxxxxx |xxxxxx00 | msb */
    FORMAT_422_P10_32BIT_MSB ,         /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */
    FORMAT_422_P10_32BIT_LSB ,         /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */
    
    FORMAT_YUYV              ,         /**< 8bit packed format : Y0U0Y1V0 Y2U1Y3V1 ... */
    FORMAT_YUYV_P10_16BIT_MSB,         /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(YUYV) format(1Pixel=2Byte) */
    FORMAT_YUYV_P10_16BIT_LSB,         /* lsb | xxxxxxxxxx000000 | msb */ /**< 10bit packed(YUYV) format(1Pixel=2Byte) */
    FORMAT_YUYV_P10_32BIT_MSB,         /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(YUYV) format(3Pixel=4Byte) */
    FORMAT_YUYV_P10_32BIT_LSB,         /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(YUYV) format(3Pixel=4Byte) */
    
    FORMAT_YVYU              ,         /**< 8bit packed format : Y0V0Y1U0 Y2V1Y3U1 ... */
    FORMAT_YVYU_P10_16BIT_MSB,         /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(YVYU) format(1Pixel=2Byte) */
    FORMAT_YVYU_P10_16BIT_LSB,         /* lsb | xxxxxxxxxx000000 | msb */ /**< 10bit packed(YVYU) format(1Pixel=2Byte) */
    FORMAT_YVYU_P10_32BIT_MSB,         /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(YVYU) format(3Pixel=4Byte) */
    FORMAT_YVYU_P10_32BIT_LSB,         /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(YVYU) format(3Pixel=4Byte) */
    
    FORMAT_UYVY              ,         /**< 8bit packed format : U0Y0V0Y1 U1Y2V1Y3 ... */
    FORMAT_UYVY_P10_16BIT_MSB,         /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(UYVY) format(1Pixel=2Byte) */
    FORMAT_UYVY_P10_16BIT_LSB,         /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(UYVY) format(1Pixel=2Byte) */
    FORMAT_UYVY_P10_32BIT_MSB,         /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(UYVY) format(3Pixel=4Byte) */
    FORMAT_UYVY_P10_32BIT_LSB,         /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(UYVY) format(3Pixel=4Byte) */
    
    FORMAT_VYUY              ,         /**< 8bit packed format : V0Y0U0Y1 V1Y2U1Y3 ... */
    FORMAT_VYUY_P10_16BIT_MSB,         /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(VYUY) format(1Pixel=2Byte) */
    FORMAT_VYUY_P10_16BIT_LSB,         /* lsb | xxxxxxxxxx000000 | msb */ /**< 10bit packed(VYUY) format(1Pixel=2Byte) */
    FORMAT_VYUY_P10_32BIT_MSB,         /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(VYUY) format(3Pixel=4Byte) */
    FORMAT_VYUY_P10_32BIT_LSB,         /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(VYUY) format(3Pixel=4Byte) */

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

NOTE: This is only for WAVE4 IP.
*/
typedef enum {
    INT_WAVE_INIT           = 0,
    INT_WAVE_DEC_PIC_HDR    = 1,
    INT_WAVE_ENC_SETPARAM   = 2,
    INT_WAVE_FINI_SEQ       = 2,
    INT_WAVE_DEC_PIC        = 3,
    INT_WAVE_ENC_PIC        = 3,
    INT_WAVE_SET_FRAMEBUF   = 4,
    INT_WAVE_FLUSH_DECODER  = 5,
    INT_WAVE_GET_FW_VERSION = 8,
    INT_WAVE_QUERY_DECODER  = 9,
    INT_WAVE_SLEEP_VPU      = 10,
    INT_WAVE_WAKEUP_VPU     = 11,
    INT_WAVE_CHANGE_INST    = 12,
    INT_WAVE_CREATE_INSTANCE= 14,
    INT_WAVE_BIT_BUF_EMPTY  = 15,   /* Decoder */
    INT_WAVE_BIT_BUF_FULL   = 15,   /* Encoder */
} Wave4InterruptBit;

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
    INT_WAVE5_DEC_QUERY         = 14,
    INT_WAVE5_BSBUF_EMPTY       = 15,
    INT_WAVE5_BSBUF_FULL        = 15,
} Wave5InterruptBit;

/**
* @brief    This is an enumeration type for representing picture types.
*/
typedef enum {
    PIC_TYPE_I            = 0, /**< I picture */
    PIC_TYPE_P            = 1, /**< P picture */
    PIC_TYPE_B            = 2, /**< B picture (except VC1) */
    PIC_TYPE_REPEAT       = 2, /**< Repeat frame (VP9 only) */
    PIC_TYPE_VC1_BI       = 2, /**< VC1 BI picture (VC1 only) */
    PIC_TYPE_VC1_B        = 3, /**< VC1 B picture (VC1 only) */
    PIC_TYPE_D            = 3, /**< D picture in MPEG2 that is only composed of DC coefficients (MPEG2 only) */
    PIC_TYPE_S            = 3, /**< S picture in MPEG4 that is an acronym of Sprite and used for GMC (MPEG4 only)*/
    PIC_TYPE_VC1_P_SKIP   = 4, /**< VC1 P skip picture (VC1 only) */
    PIC_TYPE_MP4_P_SKIP_NOT_CODED = 4, /**< Not Coded P Picture in mpeg4 packed mode */ 
    PIC_TYPE_IDR          = 5, /**< H.264/H.265 IDR picture */  
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
    PRODUCT_ID_7503,
    PRODUCT_ID_320,
    PRODUCT_ID_410,
    PRODUCT_ID_4102,
    PRODUCT_ID_420,
    PRODUCT_ID_412,
    PRODUCT_ID_7Q,
    PRODUCT_ID_420L,
    PRODUCT_ID_510,
    PRODUCT_ID_512,
    PRODUCT_ID_515,
    PRODUCT_ID_520,
    PRODUCT_ID_NONE,
}ProductId;

#define PRODUCT_ID_W_SERIES(x)      (x == PRODUCT_ID_410 || x == PRODUCT_ID_4102 || x == PRODUCT_ID_420 || x == PRODUCT_ID_412 || x == PRODUCT_ID_7Q || x == PRODUCT_ID_420L || x == PRODUCT_ID_510 || x == PRODUCT_ID_512 || x == PRODUCT_ID_515 || x == PRODUCT_ID_520)
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
    LINEAR_FRAME_MAP            = 0,  /**< Linear frame map type */       
    TILED_FRAME_V_MAP           = 1,  /**< Tiled frame vertical map type (CODA9 only) */
    TILED_FRAME_H_MAP           = 2,  /**< Tiled frame horizontal map type (CODA9 only) */
    TILED_FIELD_V_MAP           = 3,  /**< Tiled field vertical map type (CODA9 only) */
    TILED_MIXED_V_MAP           = 4,  /**< Tiled mixed vertical map type (CODA9 only) */
    TILED_FRAME_MB_RASTER_MAP   = 5,  /**< Tiled frame MB raster map type (CODA9 only) */
    TILED_FIELD_MB_RASTER_MAP   = 6,  /**< Tiled field MB raster map type (CODA9 only) */
    TILED_FRAME_NO_BANK_MAP     = 7,  /**< Tiled frame no bank map. (CODA9 only) */ // coda980 only 
    TILED_FIELD_NO_BANK_MAP     = 8,  /**< Tiled field no bank map. (CODA9 only) */ // coda980 only  
    LINEAR_FIELD_MAP            = 9,  /**< Linear field map type. (CODA9 only) */ // coda980 only 
    CODA_TILED_MAP_TYPE_MAX     = 10,  
    COMPRESSED_FRAME_MAP        = 10, /**< Compressed frame map type (WAVE only) */ // WAVE4 only
    ARM_COMPRESSED_FRAME_MAP      = 12, /**< AFBC(ARM Frame Buffer Compression) compressed frame map type */ // AFBC enabled WAVE decoder
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
* @brief    This is data structure of product information. (WAVE only)
*/
typedef struct {
    Uint32 productId;  /**< The product id */    //W4_RET_PRODUCT_ID  
    Uint32 fwVersion;  /**< The firmware version */    //W4_RET_FW_VERSION  
    Uint32 productName;  /**< VPU hardware product name  */    //W4_RET_PRODUCT_NAME
    Uint32 productVersion;  /**< VPU hardware product version */    //W4_RET_PRODUCT_VERSION
    Uint32 customerId;  /**< The customer id */    //W4_RET_CUSTOMER_ID
    Uint32 stdDef0;  /**< The system configuration information  */    //W4_RET_STD_DEF0
    Uint32 stdDef1;  /**< The hardware configuration information  */    //W4_RET_STD_DEF1
    Uint32 confFeature;  /**< The supported codec standard */    //W4_RET_CONF_FEATURE
    Uint32 configDate;  /**< The date that the hardware has been configured in YYYYmmdd in digit */    //W4_RET_CONFIG_DATE
    Uint32 configRevision;  /**< The revision number when the hardware has been configured */    //W4_RET_CONFIG_REVISION
    Uint32 configType;  /**< The define value used in hardware configuration */    //W4_RET_CONFIG_TYPE
    Uint32 configVcore[4];  /**< VCORE Configuration Information */    //W4_RET_CONF_VCORE0
}ProductInfo;

/** 
* @brief 
@verbatim
This is a data structure of tiled map information.

NOTE: WAVE4 does not support tiledmap type so this structure is not used in the product. 
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
* @brief    This is a data structure of DRAM information. (CODA960 and BODA950 only). 
VPUAPI sets default values for this structure. 
However, HOST application can configure if the default values are not associated with their DRAM
    or desirable to change. 
*/
typedef struct {
    int rasBit;     /**< This value is used for width of RAS bit. (13 on the CNM FPGA platform) */
    int casBit;     /**< This value is used for width of CAS bit. (9 on the CNM FPGA platform) */
    int bankBit;    /**< This value is used for width of BANK bit. (2 on the CNM FPGA platform) */
    int busBit;     /**< This value is used for width of system BUS bit. (3 on CNM FPGA platform) */  
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

* 0 : CbCr data is written in their separate frame memory (chroma separate mode).
* 1 : CbCr data is interleaved in chroma memory (chroma interleave mode).
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
    int myIndex;        /**< A frame buffer index to identify each frame buffer that is processed by VPU. */
    int mapType;        /**< A map type for GDI inferface or FBC (Frame Buffer Compression). NOTE: For detailed map types, please refer to <<vpuapi_h_TiledMapType>>. */
    int stride;         /**< A horizontal stride for given frame buffer */
    int width;          /**< A width for given frame buffer */
    int height;         /**< A height for given frame buffer */
    int size;           /**< A size for given frame buffer */
    int lumaBitDepth;   /**< Bit depth for luma component */
    int chromaBitDepth; /**< Bit depth for chroma component  */
    FrameBufferFormat   format;     /**< A YUV format of frame buffer */
/**
@verbatim
It enables source frame data with long burst length to be loaded for reducing DMA latency (CODA9 encoder only).

@* 0 : disable the long-burst mode.
@* 1 : enable the long-burst mode.
@endverbatim
*/
    int  sourceLBurstEn;
    int srcBufState;    /**< It indicates a status of each source buffer, whether the source buffer is used for encoding or not. */
    int sequenceNo;     /**< A sequence number that the frame belongs to. It increases by 1 every time a sequence changes in decoder.  */    
    BOOL updateFbInfo; /**< If this is TRUE, VPU updates API-internal framebuffer information when any of the information is changed. */
} FrameBuffer;

/**
* @brief    This is a data structure for representing framebuffer parameters.
It is used when framebuffer allocation using VPU_DecAllocateFrameBuffer() or VPU_EncAllocateFrameBuffer().
*/
typedef struct {
    int mapType;                /**< <<vpuapi_h_TiledMapType>> */
    int cbcrInterleave;         /**< CbCr interleave mode of frame buffer  */
    int nv21;                   /**< 1 : CrCb (NV21) , 0 : CbCr (NV12).  This is valid when cbcrInterleave is 1. */
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
    int picWidth;       /**< This value is used for width of the displayable picture region. */
    int picHeight;      /**< This value is used for height of the displayable picture region. */
    int picOffsetX;     /**< This value is located at the lower-left corner of the displayable picture region. */
    int picOffsetY;     /**< This value is located at the lower-left corner of the displayable picture region. */
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
    unsigned picWidth       : 14;   /**< Picture width in units of sample */
    unsigned picHeight      : 14;   /**< Picture height in units of sample */
} Vp8ScaleInfo;


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
            
            int useMeEnable;  /**<  This enables AXI secondary channel for motion estimation data.  */
            int useScalerEnable; /**< This enables AXI secondary channel for scaler temporal data.  */
        } coda9;
        struct {
            // for Decoder
            int useBitEnable;   /**<  This enables AXI secondary channel for prediction data of the BIT-processor. */
            int useIpEnable;    /**<  This enables AXI secondary channel for row pixel data of IP. */
            int useLfRowEnable; /**<  This enables AXI secondary channel for loopfilter.   */

            // for Encoder
            int useEncImdEnable; /**< This enables AXI secondary channel for intra mode decision. */
            int useEncLfEnable;  /**< This enables AXI secondary channel for loopfilter. */
            int useEncRdoEnable; /**< This enables AXI secondary channel for RDO. */ 

        } wave4;
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
 
@* 1 : Cache off 
@* 0 : Cache on
@endverbatim 
*/
        unsigned Bypass         : 1;
/**
@verbatim
It enables two frame caching mode.

@* 1 : Dual mode (caching for FrameIndex0 and FrameIndex1)
@* 0 : Single mode (caching for FrameIndex0)
@endverbatim 
*/
        unsigned DualConf       : 1;
/**
@verbatim
Mode for page merging

@* 0 : Disable
@* 1 : Horizontal
@* 2 : Vertical

We recommend you to set 1 (horizontal) in tiled map or to set 2 (vertical) in linear map.
@endverbatim 
*/       
        unsigned PageMerge      : 2;
    } type1;
    struct {
/**
@verbatim
CacheMode represents cache configuration. 

@* [10:9] : Cache line processing direction and merge mode
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
@** 0: Cache enable
@** 1: Cache disable (bypass) 
@* [0] CacheBypassMC
@** 0: Cache enable
@** 1: Cache disable (bypass)
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


struct CodecInst;

//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------

#define VPU_HANDLE_INSTANCE_NO(_handle)         (_handle->instIndex)
#define VPU_HANDLE_CORE_INDEX(_handle)          (((CodecInst*)_handle)->coreIdx)
#define VPU_HANDLE_PRODUCT_ID(_handle)          (((CodecInst*)_handle)->productId)
#define VPU_CONVERT_WTL_INDEX(_handle, _index)  ((((CodecInst*)_handle)->CodecInfo->decInfo).numFbsForDecoding+_index)

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

#define FBC_MODE_BEST_PREDICTION    0x00        //!<< Best for bandwidth, some performance overhead
#define FBC_MODE_NORMAL_PREDICTION  0x0c        //!<< Good for badnwidth and performance
#define FBC_MODE_BASIC_PREDICTION   0x3c        //!<< Best for performance

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
@* 0 : No extension of H.264/AVC
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
It enables WTL (Write Linear) function. If this field is enabled,
VPU writes a decoded frame to the frame buffer twice - first in linear map and second in tiled or compressed map. 
Therefore, HOST application should allocate one more frame buffer for saving both formats of frame buffers.

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
@* 0 : CbCr data is written in separate frame memories (chroma separate mode)
@* 1 : CbCr data is interleaved in chroma memory. (chroma interleave mode)           
@endverbatim
*/     
    int             cbcrInterleave; 
/**
@verbatim    
CrCb interleaved mode (NV21).

@* 0 : Decoded chroma data is written in CbCr (NV12) format. 
@* 1 : Decoded chroma data is written in CrCb (NV21) format. 

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
Frame buffer endianness

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
Bitstream buffer endianess

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
When read pointer reaches write pointer in the middle of decoding one picture,

@* 0 : VPU sends an interrupt to HOST application and waits for more bitstream to decode. (interrupt mode)
@* 1 : Reserved
@* 2 : VPU decodes bitstream from read pointer to write pointer. (PicEnd mode)
@endverbatim
*/     
    int             bitstreamMode; 
    Uint32          coreIdx;    /**< VPU core index number (0 ~ [number of VPU core] - 1) */ 
/** 
BIT processor work buffer SDRAM address/size information. In parallel decoding operation, work buffer is shared between VPU cores.
The work buffer address is set to this member variable when VPU_Decopen() is called. 
Unless HOST application sets the address and size of work buffer, VPU allocates automatically work buffer  
when VPU_DecOpen() is executed.  
*/    
    vpu_buffer_t    vbWork;
/**
@verbatim
It determines prediction mode of frame buffer compression.

@* 0x00 : Best Predection (best for bandwidth, but some performance overhead might exist)
@* 0x0C : Normal Prediction (good for bandwidth and performance)
@* 0x3C : Basic Predcition (best for performance)
@endverbatim    
*/
    int             fbc_mode;    
    Uint32          virtAxiID; /**< AXI_ID to distinguish guest OS. For virtualization only. Set this value in highest bit order.*/
    BOOL            bwOptimization; /**< Bandwidth optimization feature which allows WTL(Write to Linear)-enabled VPU to skip writing compressed format of non-reference pictures or linear format of non-display pictures to the frame buffer for BW saving reason. */
    Uint32          div3Width;    /**< The picture width of div3 stream  (CODA7Q only) */  
    Uint32          div3Height;   /**< The picture height of div3 stream (CODA7Q only) */  
} DecOpenParam;

/**
* @brief Data structure to get information necessary to start decoding from the decoder.
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
    Int32           maxSubLayers;           /**< Number of sub-layer for H.265/HEVC */
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
@** 3\'b10 : SpatiallyScalable
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
@* VP8 : 0 - 3
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
@* VC1 : level in struct B
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
    Int32           isExtSAR;                  
/**
@verbatim
This is one of the SPS syntax elements in H.264.

@* 0 : max_num_ref_frames is 0.
@* 1 : max_num_ref_frames is not 0.
@endverbatim
*/
    Int32           maxNumRefFrmFlag;       
    Int32           maxNumRefFrm;
/**
@verbatim
@* H.264/AVC : When avcIsExtSAR is 0, this indicates aspect_ratio_idc[7:0]. When avcIsExtSAR is 1, this indicates sar_width[31:16] and sar_height[15:0].
If aspect_ratio_info_present_flag = 0, the register returns -1 (0xffffffff).
@* VC1 : This reports ASPECT_HORIZ_SIZE[15:8] and ASPECT_VERT_SIZE[7:0].
@* MPEG2 : This value is index of Table 6-3 in ISO/IEC 13818-2.
@* MPEG4/H.263 : This value is index of Table 6-12 in ISO/IEC 14496-2.
@* RV : aspect_ratio_info
@* AVS : This value is the aspect_ratio_info[3:0] which is used as index of Table 7-5 in AVS Part2
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
    Int32           seqInitErrReason;
    Int32           warnInfo;
    PhysicalAddress rdPtr;         /**< A read pointer of bitstream buffer */
    PhysicalAddress wrPtr;         /**< A write pointer of bitstream buffer */
    AvcVuiInfo      avcVuiInfo;    /**< This is H.264/AVC VUI information. Refer to <<vpuapi_h_AvcVuiInfo>>. */
    MP2BarDataInfo  mp2BardataInfo;/**< This is bar information in MPEG2 user data. For details about this, please see the document 'ATSC Digital Television Standard: Part 4:2009'. */
    Uint32          sequenceNo;    /**< This is the number of sequence information. This variable is increased by 1 when VPU detects change of sequence. */
    Int32           numReorderFrames;   /**< This is num_reorder_frames in in H.264/AVC VUI syntax (CODA7Q only) */
} DecInitialInfo;

// Report Information

/**
* @brief    The data structure for options of picture decoding.
*/
#define WAVE_SKIPMODE_WAVE_NONE 0
#define WAVE_SKIPMODE_NON_IRAP  1
#define WAVE_SKIPMODE_NON_REF   2
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
@* 1 : skip frame enabled (skip frames but I (IDR) frame)
@* 2 : skip frame enabled (skip any frames),

If this option is enabled, decoder skips decoding as many as skipframeNum.

If skipframeMode is 1, decoder skips frames but I (IDR) frames and decodes I (IDR)
frames. If skipframeMode is 2, decoder skips any frames. After decoder skips
frames, decoder returns the decoded index -2 (0xFFFE) when decoder does not have any
frames displayed.

In case of WAVE4,

@* 0x0 : skip frame off
@* 0x1 : skip non-RAP pictures. That is, all pictures that are not IDR, CRA, or BLA are skipped. 
@* 0x2 : skip non-reference pictures. 
@* 0x3 : Reserved
@* 0x4~0xf : Reserved

NOTE: When decoder meets EOS (End Of Sequence) code during frame skip,
decoder returns -1(0xFFFF).
@endverbatim
*/
    Int32 skipframeMode;           
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
FSets a de-blocking filter mode for RV streams.

@* 0 : It enables de-blocking filter for all pictures.
@* 1 : It disables de-blocking filter for all pictures.
@* 2 : It disables de-blocking filter for P and B pictures.
@* 3 : It disables de-blocking filter only for B pictures.
@endverbatim
*/
        Int32 rvDbkMode;          
    } DecStdParam;

    BOOL    craAsBlaFlag;   /**< It handles CRA picture as BLA picture not to use reference from the previous decoded pictures (H.265/HEVC only) */
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
 #define H265_USERDATA_FLAG_RESERVED_9           (9)  
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
    int viewIdxDisplay; /**< This is view index order of display frame buffer corresponding to indexFrameDisplay of DecOutputInfo structure. */
    int viewIdxDecoded; /**< This is view index order of decoded frame buffer corresponding to indexFrameDecoded of DecOutputInfo structure. */
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
    int cpbMinus1;          /**< cpb_cnt_minus1 */
    int vclHrdParamFlag;    /**< vcl_hrd_parameters_present_flag */
    int nalHrdParamFlag;    /**< nal_hrd_parameters_present_flag */
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
    int recoveryFrameCnt;       /**< recovery_frame_cnt */
    int exactMatchFlag;         /**< exact_match_flag */
    int brokenLinkFlag;         /**< broken_link_flag */
    int changingSliceGroupIdc;  /**< changing_slice_group_idc */
} AvcRpSei;

// HEVC specific Recovery Point information 
/**
* @brief    This is a data structure for H.265/HEVC specific picture information. (H.265/HEVC decoder only)
VPU returns this structure after decoding a frame.
*/
typedef struct {
/**
@verbatim
This is a flag to indicate whether H.265/HEVC RP SEI exists or not.

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
* @brief    This is a data structure that H.265/HEVC decoder returns for reporting POC (Picture Order Count).
*/
typedef struct {
    int decodedPOC; /**< A POC value of picture that has currently been decoded and with decoded index. When indexFrameDecoded is -1, it returns -1. */
    int displayPOC; /**< A POC value of picture with display index. When indexFrameDisplay is -1, it returns -1. */
    int temporalId; /**< A temporal ID of the picture */
} H265Info;

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

@* -3(0xFFFD) : It is when decoder skip option is on. 
@* -2(0xFFFE) : It is when decoder have decoded sequence but cannot give a display output due to reordering.
@* -1(0xFFFF) : It is when there is no more output for display at the end of sequence decoding.

@endverbatim
*/
    int indexFrameDisplay;      
    int indexFrameDisplayForTiled;  /**< In case of WTL mode, this index indicates a display index of tiled or compressed framebuffer. */
/**
@verbatim
This is a frame buffer index of decoded picture among frame buffers which were
registered using VPU_DecRegisterFrameBuffer(). The currently decoded frame is stored into the frame buffer specified by
this index. 

* -2 : It indicates that no decoded output is generated because decoder meets EOS (End Of Sequence) or skip.
* -1 : It indicates that decoder fails to decode a picture because there is no available frame buffer.
@endverbatim
*/
    int indexFrameDecoded;      
    int indexInterFrameDecoded;     /**< In case of VP9 codec, this indicates an index of the frame buffer to reallocate for the next frame's decoding. VPU returns this information when detecting change of the inter-frame resolution. */
    int indexFrameDecodedForTiled;  /**< In case of WTL mode, this indicates a decoded index of tiled or compressed framebuffer. */
    int nalType;                    /**< This is nal Type of decoded picture. Please refer to nal_unit_type in Table 7-1 - NAL unit type codes and NAL unit type classes in H.265/HEVC specification. (WAVE only) */
    int picType;                    /**< This is the picture type of decoded picture. It reports the picture type of bottom field for interlaced stream. <<vpuapi_h_PicType>>. */
    int picTypeFirst;               /**< This is only valid in interlaced mode and indicates the picture type of the top field. */
    int numOfErrMBs;                /**< This is the number of error coded unit in a decoded picture. */
    int numOfTotMBs;                /**< This is the number of coded unit in a decoded picture. */
    int numOfErrMBsInDisplay;       /**< This is the number of error coded unit in a picture mapped to indexFrameDisplay. */
    int numOfTotMBsInDisplay;       /**< This is the number of coded unit in a picture mapped to indexFrameDisplay. */
    BOOL refMissingFrameFlag;       /**< This indicates that the current frame's references are missing in decoding. (WAVE only)  */
    int notSufficientSliceBuffer;   /**< This is a flag which represents whether slice save buffer is not sufficient to decode the current picture. VPU might not get the last part of the current picture stream due to buffer overflow, which leads to macroblock errors. HOST application can continue decoding the remaining pictures of the current bitstream without closing the current instance, even though several pictures could be error-corrupted. (H.264/AVC BP only) */
    int notSufficientPsBuffer;      /**< This is a flag which represents whether PS (SPS/PPS) save buffer is not sufficient to decode the current picture. VPU might not get the last part of the current picture stream due to buffer overflow. HOST application must close the current instance, since the following picture streams cannot be decoded properly for loss of SPS/PPS data. (H.264/AVC only) */
/** 
@verbatim
This variable indicates whether decoding process was finished completely or not. If stream
has error in the picture header syntax or has the first slice header syntax of H.264/AVC
stream, VPU returns 0 without proceeding MB decode routine. 

@* 0 : It indicates incomplete finish of decoding process
@* 1 : It indicates complete finish of decoding process

@endverbatim
*/
    int decodingSuccess;    
/** 
@verbatim
@* 0 : A progressive frame which consists of one picture
@* 1 : An interlaced frame which consists of two fields 
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
* No DPB available: It is when VPU is not able to consume chunk with no frame buffers available at the moment. Thus, the whole chunk should be provided again. 
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
    H265Info h265Info;              /**< This is H.265/HEVC picture information. Refer to <<vpuapi_h_H265Info>>. */
/**
@verbatim
This field is valid only for VC1 decoding. Field information of display frame index
is returned on `indexFrameDisplay`. 

@* 0 : Paired fields
@* 1 : Bottom (top-field missing)
@* 2 : Top (bottom-field missing)
@endverbatim
*/
    int vc1NpfFieldInfo;
    int mp2DispVerSize;             /**< This is display_vertical_size syntax of sequence display extension in MPEG2 specification. */
    int mp2DispHorSize;             /**< This is display_horizontal_size syntax of sequence display extension in MPEG2 specification. */
/**
@verbatim
This field is valid only for MPEG2 decoding. Field information of display frame index
is returned on `indexFrameDisplay`. 

@* 0 : Paired fields
@* 1 : Bottom (top-field missing)
@* 2 : Top (bottom-field missing)
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

@* 0 : Paired fields
@* 1 : Bottom (top-field missing)
@* 2 : Top (bottom-field missing)
@endverbatim
*/
    int avcNpfFieldInfo; 
    int avcPocPic;  /**< This field reports the POC value of frame picture in case of H.264/AVC decoding. */
    int avcPocTop;  /**< This field reports the POC value of top field picture in case of H.264/AVC decoding. */
    int avcPocBot;  /**< This field reports the POC value of bottom field picture in case of H.264/AVC decoding. */
    Uint32 avcTemporalId;  /**< This field reports the target temporal ID of current picture. This is valid in H.264/AVC decoder. */
    // Report Information
/** 
@verbatim
This variable indicates that the decoded picture is progressive or interlaced
picture. The value of pictureStructure is used as below.

@* H.264/AVC : MBAFF
@* VC1 : FCM
@** 0 : Progressive
@** 2 : Frame interlace
@** 3 : Field interlaced
@* MPEG2 : picture structure
@** 1 : TopField
@** 2 : BotField
@** 3 : Frame
@* MPEG4 : N/A
@* Real Video : N/A
@* H.265/HEVC : N/A
@endverbatim
*/
    int pictureStructure;    
/**
@verbatim
For decoded picture consisting of two fields, this variable reports

@ 0 : VPU decodes the bottom field and then top field. 
@ 1 : VPU decodes the top field and then bottom field.   

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
@** 0 : The decoded frame has paired fields.
@** 1 : The decoded frame has a top-field missing.
@** 2 : The decoded frame has a bottom-field missing.
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

@* sequenceChanged[5] : It indicates that the profile_idc has been changed.
@* sequenceChanged[16] : It indicates that the resolution has been changed.
@* sequenceChanged[19] : It indicates that the required number of frame buffer has been changed. 
@endverbatim
*/
    int sequenceChanged;     
    // CODA9: [0]   1 - sequence changed
    // WAVEX: [5]   1 - H.265 profile changed
    //        [16]  1 - resolution changed
    //        [19]  1 - number of DPB changed

    int streamEndFlag;  /**< This variable reports the status of `end of stream` flag. This information can be used for low delay decoding (CODA980 only). */
    int frameCycle;     /**< This variable reports the cycle number of decoding one frame. */
    int errorReason;    /**< This variable reports the error reason that occurs while decoding. For error description, please find the 'Appendix: Error Definition' in the Programmer's Guide. */
    int errorReasonExt; /**< This variable reports the specific reason of error. For error description, please find the 'Appendix: Error Definition' in the Programmer's Guide. (WAVE only) */
    int warnInfo;
    Uint32 sequenceNo;  /**< This variable increases by 1 whenever sequence changes. If it happens, HOST should call VPU_DecFrameBufferFlush() to get the decoded result that remains in the buffer in the form of DecOutputInfo array. HOST can recognize with this variable whether this frame is in the current sequence or in the previous sequence when it is displayed. (WAVE only) */ 
    int rvTr;           /**< This variable reports RV timestamp for Ref frame. */
    int rvTrB;          /**< This variable reports RV timestamp for B frame. */
    
/**
@verbatim
This variable reports the result of pre-scan which is the start of decoding routine for DEC_PIC command. (WAVE4 only)
In the prescan phase, VPU parses bitstream and pre-allocates frame buffers. 

@* -2 : It is when VPU prescanned bitstream(bitstream consumed), but a decode buffer was not allocated for the bitstream during pre-scan, since there was only header information.
@* -1 : It is when VPU detected full of framebuffer while pre-scannig (bitstream not consumed).
@* >= 0 : It indicates that prescan has been successfully done. This index is returned to a decoded index for the next decoding.
@endverbatim
*/ 
    int indexFramePrescan; 
#ifdef SUPPORT_REF_FLAG_REPORT
    int frameReferenceFlag[31]; 
#endif

    Int32   seekCycle;          /**< This variable reports the number of cycles in seeking phase on the command queue. (WAVE5 only) */
    Int32   parseCycle;         /**< This variable reports the number of cycles in prescan phase on the command queue. (WAVE5 only) */
    Int32   decodeCycle;        /**< This variable reports the number of cycles in decoding phase on the command queue. (WAVE5 only) */

/**
@verbatim
A CTU size (only for WAVE series)

@* 16 : CTU16x16
@* 32 : CTU32x32
@* 64 : CTU64x64
@endverbatim
*/
    Int32 ctuSize;
    Int32 outputFlag;          /**< This variable reports whether the current frame is bumped out or not. (WAVE5 only) */
} DecOutputInfo;

/**
 * @brief   This is a data structure of frame buffer information. It is used for parameter when host issues DEC_GET_FRAMEBUF_INFO of <<vpuapi_h_VPU_DecGiveCommand>>.
 */
 typedef struct {
    vpu_buffer_t  vbFrame;          /**< The information of frame buffer where compressed frame is saved  */
    vpu_buffer_t  vbWTL;            /**< The information of frame buffer where decoded, uncompressed frame is saved with linear format if WTL is on   */
    vpu_buffer_t  vbFbcYTbl[MAX_REG_FRAME];        /**< The information of frame buffer to save luma offset table of compressed frame  */
    vpu_buffer_t  vbFbcCTbl[MAX_REG_FRAME];        /**< The information of frame buffer to save chroma offset table of compressed frame */
    vpu_buffer_t  vbMvCol[MAX_REG_FRAME];          /**< The information of frame buffer to save motion vector collocated buffer */
    FrameBuffer   framebufPool[64]; /**< This is an array of <<vpuapi_h_FrameBuffer>> which contains the information of each frame buffer. When WTL is enabled, the number of framebufPool would be [number of compressed frame buffer] x 2, and the starting index of frame buffer for WTL is framebufPool[number of compressed frame buffer].  */
} DecGetFramebufInfo;

/**
 * @brief   This is a data structure of queue command information. It is used for parameter when host issues DEC_GET_QUEUE_STATUS of <<vpuapi_h_VPU_DecGiveCommand>>. (WAVE5 only)
 */
typedef struct {
    Uint32  instanceQueueCount; /**< This variable indicates the number of queued commands of the instance.  */
    Uint32  totalQueueCount;    /**< This variable indicates the number of queued commands of all instances.  */
} DecQueueStatusInfo;

//------------------------------------------------------------------------------
// encode struct and definition
//------------------------------------------------------------------------------



#define MAX_NUM_TEMPORAL_LAYER          7
#define MAX_GOP_NUM                     8

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
    int numRefPicL0;
    int refPocL0;                       /**< A POC of reference L0 of Nth picture in the custom GOP */
    int refPocL1;                       /**< A POC of reference L1 of Nth picture in the custom GOP */
    int temporalId;                     /**< A temporal ID of Nth picture in the custom GOP */
} CustomGopPicParam;

/**
* @brief    This is a data structure for custom GOP parameters. (WAVE encoder only)
*/
typedef struct {
    int customGopSize;                  /**< The size of custom GOP (0~8) */
    int useDeriveLambdaWeight;           /**< It internally derives a lamda weight instead of using the given lamda weight. */
    CustomGopPicParam picParam[MAX_GOP_NUM];  /**< Picture parameters of Nth picture in custom GOP */
    int gopPicLambda[MAX_GOP_NUM];       /**< A lamda weight of Nth picture in custom GOP */
    int enTemporalLayerQp; /**< Enable QP setting of each temporal layers */
    int tidQp0;            /**< Specifies a QP value of temporal layer 0 */
    int tidQp1;            /**< Specifies a QP value of temporal layer 1 */
    int tidQp2;            /**< Specifies a QP value of temporal layer 2 */
    int tidPeriod0;        /**< Specifies the period of temporal layer 0 */
} CustomGopParam;


/**
* @brief    This is a data structure for setting CTU level options (ROI, CTU mode, CTU QP) in H.265/HEVC encoder (WAVE4 encoder only). 
*/
typedef struct {
    int roiEnable;                      /**< It enables ROI map. NOTE: It is valid when rcEnable is on. */
    int roiDeltaQp;                     /**< It specifies a delta QP that is used to calculate ROI QPs internally. */
    int mapEndian;                      /**< It specifies endianness of ROI CTU map. For the specific modes, refer to the EndianMode of <<vpuapi_h_DecOpenParam>>. */
/**
@verbatim
It specifies the stride of CTU-level ROI/mode/QP map. It should be set with the value as below.

 (Width  + CTB_SIZE - 1) / CTB_SIZE 
@endverbatim
*/
    int mapStride;                      
/**
@verbatim
The start buffer address of ROI map

ROI map holds importance levels for CTUs within a picture. The memory size is the number of CTUs of picture in bytes. 
For example, if there are 64 CTUs within a picture, the size of ROI map is 64 bytes.  
All CTUs have their ROI importance level (0 ~ 8 ; 1 byte)  in raster order. 
A CTU with a high ROI importance level is encoded with a lower QP for higher quality. 
It should be given when roiEnable is 1. 
@endverbatim
*/
    PhysicalAddress addrRoiCtuMap;

/**
@verbatim

The start buffer address of CTU qp map

The memory size is the number of CTUs of picture in bytes. 
For example if there are 64 CTUs within a picture, the size of CTU map is 64 bytes. 
It should be given when ctuQpEnable is 1. 

The content of ctuQpMap directly is mapped to the QP used to encode the CTU. I.e, 
if (ctuQpMap[k] is 5), it means that CTU(k) is encoded with QP 5.
@endverbatim
*/
    PhysicalAddress addrCtuQpMap; 
/**
@verbatim
It enables CTU QP map that allows CTUs to be encoded with the given QPs. 

NOTE: rcEnable should be turned off for this, encoding with the given CTU QPs.

@endverbatim
*/
    int ctuQpEnable;    
} HevcCtuOptParam;

/**
* @brief    This is a data structure for setting custom map options in H.265/HEVC encoder. (WAVE5 encoder only). 
*/
typedef struct {
    int roiAvgQp;  /**< It sets an average QP of ROI map. */
    int customRoiMapEnable; /**< It enables ROI map. */
    int customLambdaMapEnable; /**< It enables custom lambda map. */
    int customModeMapEnable;   /**< It enables to force CTU to be encoded with intra or to be skipped.  */
    int customCoefDropEnable;  /**< It enables to force all coefficients to be zero after TQ or not for each CTU (to be dropped).*/
    PhysicalAddress addrCustomMap;  /**< The address of custom map */
} HevcCustomMapOpt;

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
* @brief    This is a data structure for setting SEI NALs in H.265/HEVC encoder. 
*/
typedef struct {
    Uint32 prefixSeiNalEnable;  /**< It enables to encode the prefix SEI NAL which is given by HOST application. */
    Uint32 prefixSeiDataSize;   /**< The total byte size of the prefix SEI */
/**
@verbatim
A flag whether to encode PREFIX_SEI_DATA with a picture of this command or with a source picture of the buffer at the moment
 
@* 0 : encode PREFIX_SEI_DATA when the current input source picture is encoded.
@* 1 : encode PREFIX_SEI_DATA at this command.
@endverbatim
*/
    Uint32 prefixSeiDataEncOrder;
    PhysicalAddress prefixSeiNalAddr; /**< The start address of the total prefix SEI NALs to be encoded */

    Uint32 suffixSeiNalEnable;  /**< It enables to encode the suffix SEI NAL which is given by HOST application. */
    Uint32 suffixSeiDataSize;   /**< The total byte size of the suffix SEI */
/**
@verbatim
A flag whether to encode SUFFIX_SEI_DATA with a picture of this command or with a source picture of the buffer at the moment
 
@* 0 : encode SUFFIX_SEI_DATA when the current input source picture is encoded.
@* 1 : encode SUFFIX_SEI_DATA at this command.
@endverbatim
*/
    Uint32 suffixSeiDataEncOrder;  
    PhysicalAddress suffixSeiNalAddr; /**< The start address of the total suffix SEI NALs to be encoded */
} HevcSEIDataEnc;

/**
* @brief    This is a data structure for H.265/HEVC encoder parameters.
*/
typedef struct {
/**
@verbatim
A profile indicator

@* 1 : main
@* 2 : main10
@endverbatim
*/
    int profile;                        
    int level;                          /**< A level indicator (level * 10) */ 
/**
@verbatim
A tier indicator

@* 0 : Main
@* 1 : High
@endverbatim
*/
    int tier;                           
/**
@verbatim
A bit-depth (8bit or 10bit) which VPU internally uses for encoding
 
VPU encodes with internalBitDepth instead of InputBitDepth. 
For example, if InputBitDepth is 8 and InternalBitDepth is 10, 
VPU converts the 8-bit source frames into 10-bit ones and then encodes them.    

@endverbatim
*/
    int internalBitDepth;               
    int chromaFormatIdc;                /**< A chroma format indicator (0 for 4:2:0) */
    int losslessEnable;                 /**< It enables lossless coding. */
    int constIntraPredFlag;             /**< It enables constrained intra prediction. */
/**
@verbatim
A GOP structure preset option

@* 0 : Custom GOP
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

    int confWinTop;                     /**< A top offset of conformance window */
    int confWinBot;                     /**< A bottom offset of conformance window */
    int confWinLeft;                    /**< A left offset of conformance window */
    int confWinRight;                   /**< A right offset of conformance window */

/**
@verbatim
A slice mode for independent slice 

@* 0 : No multi-slice
@* 1 : Slice in CTU number  
@endverbatim
*/
    int independSliceMode;              
    int independSliceModeArg;           /**< The number of CTU for a slice when independSliceMode is set with 1  */

/**
@verbatim
A slice mode for dependent slice 

@* 0 : No multi-slice
@* 1 : Slice in CTU number  
@* 2 : Slice in number of byte
@endverbatim
*/
    int dependSliceMode;                
    int dependSliceModeArg;             /**< The number of CTU or bytes for a slice when dependSliceMode is set with 1 or 2  */

/**
@verbatim
An intra refresh mode

@* 0 : No intra refresh
@* 1 : Row
@* 2 : Column 
@* 3 : Step size in CTU
@endverbatim
*/
    int intraRefreshMode;               
    int intraRefreshArg;                /**< The number of CTU (only valid when intraRefreshMode is 3.) */
    
/**
@verbatim
It uses one of the recommended encoder parameter presets.
 
@* 0 : Custom
@* 1 : Recommend encoder parameters (slow encoding speed, highest picture quality)
@* 2 : Boost mode (normal encoding speed, moderate picture quality)
@* 3 : Fast mode (fast encoding speed, low picture quality)
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
    int maxNumMerge;                    /**< Maximum number of merge candidates (0~2) */
    int dynamicMerge8x8Enable;          /**< It enables dynamic merge 8x8 candidates. */
    int dynamicMerge16x16Enable;        /**< It enables dynamic merge 16x16 candidates. */
    int dynamicMerge32x32Enable;        /**< It enables dynamic merge 32x32 candidates. */
    int disableDeblk;                   /**< It disables in-loop deblocking filtering. */
    int lfCrossSliceBoundaryEnable;     /**< It enables filtering across slice boundaries for in-loop deblocking. */
    int betaOffsetDiv2;                 /**< It enables BetaOffsetDiv2 for deblocking filter. */
    int tcOffsetDiv2;                   /**< It enables TcOffsetDiv3 for deblocking filter. */
    int skipIntraTrans;                 /**< It enables transform skip for an intra CU. */
    int saoEnable;                      /**< It enables SAO (Sample Adaptive Offset). */
    int intraInInterSliceEnable;        /**< It enables to make intra CUs in an inter slice. */
    int intraNxNEnable;                 /**< It enables intra NxN PUs. */

    int intraQpOffset;                  /**< It specifies an intra QP offset relative to an inter QP. It is only valid when RateControl is enabled. */
/**
@verbatim
It specifies encoder initial delay. It is only valid when RateControl is enabled.

 encoder initial delay = InitialDelay * InitBufLevelx8 / 8

@endverbatim
*/
    int initBufLevelx8;                 
/**
@verbatim
It specifies picture bits allocation mode.
It is only valid when RateControl is enabled and GOP size is larger than 1.

@* 0 : More referenced pictures have better quality than less referenced pictures.
@* 1 : All pictures in a GOP have similar image quality.
@* 2 : Each picture bits in a GOP is allocated according to FixedRatioN.
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

    int hvsQpScaleEnable;               /**< It enable QP scaling factor for CU QP adjustment when hvsQPEnable is 1. */
    int hvsQpScale;                     /**< A QP scaling factor for CU QP adjustment when hvcQpenable is 1 */

    int hvsMaxDeltaQp;                     /**< A maximum delta QP for HVS */

    int minQp;                          /**< A minimum QP for rate control */
    int maxQp;                          /**< A maximum QP for rate control */
    int maxDeltaQp;                     /**< A maximum delta QP for rate control */

    int transRate;                      /**< A peak transmission bitrate in bps */

    // CUSTOM_GOP
    CustomGopParam gopParam;            /**< <<vpuapi_h_CustomGopParam>> */
    HevcCtuOptParam ctuOptParam;        /**< <<vpuapi_h_HevcCtuOptParam>> */ 
    HevcVuiParam vuiParam;              /**< <<vpuapi_h_HevcVuiParam>> */ 

    Uint32 numUnitsInTick;              /**< It specifies the number of time units of a clock operating at the frequency time_scale Hz. */ 
    Uint32 timeScale;                   /**< It specifies the number of time units that pass in one second. */ 
    Uint32 numTicksPocDiffOne;          /**< It specifies the number of clock ticks corresponding to a difference of picture order count values equal to 1. */ 

    int chromaCbQpOffset;               /**< The value of chroma(Cb) QP offset (only for WAVE420L) */
    int chromaCrQpOffset;               /**< The value of chroma(Cr) QP offset  (only for WAVE420L) */

    int initialRcQp;                    /**< The value of initial QP by HOST application. This value is meaningless if INITIAL_RC_QP is 63. (only for WAVE420L) */

    Uint32  nrYEnable;                  /**< It enables noise reduction algorithm to Y component.  */
    Uint32  nrCbEnable;                 /**< It enables noise reduction algorithm to Cb component. */
    Uint32  nrCrEnable;                 /**< It enables noise reduction algorithm to Cr component. */
    Uint32  nrNoiseEstEnable;           /**< It enables noise estimation for reduction. When this is disabled, noise estimation is carried out ouside VPU. */
    Uint32  nrNoiseSigmaY;              /**< It specifies Y noise standard deviation if no use of noise estimation (nrNoiseEstEnable is 0).  */
    Uint32  nrNoiseSigmaCb;             /**< It specifies Cb noise standard deviation if no use of noise estimation (nrNoiseEstEnable is 0). */
    Uint32  nrNoiseSigmaCr;             /**< It specifies Cr noise standard deviation if no use of noise estimation (nrNoiseEstEnable is 0). */

    // ENC_NR_WEIGHT
    Uint32  nrIntraWeightY;             /**< A weight to Y noise level for intra picture (0 ~ 31). nrIntraWeight/4 is multiplied to the noise level that has been estimated. This weight is put for intra frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrIntraWeightCb;            /**< A weight to Cb noise level for intra picture (0 ~ 31) */
    Uint32  nrIntraWeightCr;            /**< A weight to Cr noise level for intra picture (0 ~ 31) */
    Uint32  nrInterWeightY;             /**< A weight to Y noise level for inter picture (0 ~ 31). nrInterWeight/4 is multiplied to the noise level that has been estimated. This weight is put for inter frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrInterWeightCb;            /**< A weight to Cb noise level for inter picture (0 ~ 31) */
    Uint32  nrInterWeightCr;            /**< A weight to Cr noise level for inter picture (0 ~ 31) */

    // ENC_INTRA_MIN_MAX_QP
    Uint32 intraMinQp;                  /**< It specifies a minimum QP for intra picture (0 ~ 51). It is only valid when RateControl is 1. */
    Uint32 intraMaxQp;                  /**< It specifies a maximum QP for intra picture (0 ~ 51). It is only valid when RateControl is 1. */

    Uint32 enableAFBCD;                 /**< It enables AFBCD function that is able to receive AFBC(ARM Frame Buffer Compression) format stream as source input and decode them. */
    Uint32 useLongTerm;                 /**< It enables long-term reference function. */
    int    forcedIdrHeaderEnable;       /**< It enables every IDR frame to include VPS/SPS/PPS. */


    // newly added for WAVE520
//#ifdef WAVE520
    Uint32 monochromeEnable;
    Uint32 strongIntraSmoothEnable;
    
    Uint32 weightPredEnable;
    Uint32 bgDetectEnable;
    Uint32 bgThrDiff;
    Uint32 bgThrMeanDiff;
    Uint32 bgLambdaQp;
    int    bgDeltaQp;
    Uint32 tileNumCols;
    Uint32 tileNumRows;
    Uint32 tileUniformSpaceEnable;
    
    Uint32 customLambdaEnable;
    Uint32 customMDEnable;
    int    pu04DeltaRate;
    int    pu08DeltaRate;
    int    pu16DeltaRate;
    int    pu32DeltaRate;
    int    pu04IntraPlanarDeltaRate;
    int    pu04IntraDcDeltaRate;
    int    pu04IntraAngleDeltaRate;
    int    pu08IntraPlanarDeltaRate;
    int    pu08IntraDcDeltaRate;
    int    pu08IntraAngleDeltaRate;
    int    pu16IntraPlanarDeltaRate;
    int    pu16IntraDcDeltaRate;
    int    pu16IntraAngleDeltaRate;
    int    pu32IntraPlanarDeltaRate;
    int    pu32IntraDcDeltaRate;
    int    pu32IntraAngleDeltaRate;
    int    cu08IntraDeltaRate;
    int    cu08InterDeltaRate;
    int    cu08MergeDeltaRate;
    int    cu16IntraDeltaRate;
    int    cu16InterDeltaRate;
    int    cu16MergeDeltaRate;
    int    cu32IntraDeltaRate;
    int    cu32InterDeltaRate;
    int    cu32MergeDeltaRate;
    int    coefClearDisable;
    int    minQpI;                   /**< A minimum QP of I picture for rate control */
    int    maxQpI;                   /**< A maximum QP of I picture for rate control */
    int    minQpP;                   /**< A minimum QP of P picture for rate control */
    int    maxQpP;                   /**< A maximum QP of P picture for rate control */
    int    minQpB;                   /**< A minimum QP of B picture for rate control */
    int    maxQpB;                   /**< A maximum QP of B picture for rate control */
    PhysicalAddress customLambdaAddr;
    PhysicalAddress userScalingListAddr;
//#endif

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
}EncHevcParam;

/**
* @brief This is an enumeration for encoder parameter change. (WAVE4 encoder only)
*/
typedef enum {
    // COMMON parameters
    ENC_PIC_PARAM_CHANGE                = (1<<1),
    ENC_INTRA_PARAM_CHANGE              = (1<<3),
    ENC_CONF_WIN_TOP_BOT_CHANGE         = (1<<4),
    ENC_CONF_WIN_LEFT_RIGHT_CHANGE      = (1<<5),
    ENC_FRAME_RATE_CHANGE               = (1<<6),
    ENC_INDEPENDENT_SLICE_CHANGE        = (1<<7),
    ENC_DEPENDENT_SLICE_CHANGE          = (1<<8),
    ENC_INTRA_REFRESH_CHANGE            = (1<<9),
    ENC_PARAM_CHANGE                    = (1<<10),
    ENC_RC_INIT_QP                      = (1<<11),
    ENC_RC_PARAM_CHANGE                 = (1<<12),
    ENC_RC_MIN_MAX_QP_CHANGE            = (1<<13),
    ENC_RC_TARGET_RATE_LAYER_0_3_CHANGE = (1<<14),
    ENC_RC_TARGET_RATE_LAYER_4_7_CHANGE = (1<<15),
    ENC_RC_INTRA_MIN_MAX_QP_CHANGE      = (1<<16),
    ENC_NUM_UNITS_IN_TICK_CHANGE        = (1<<18),
    ENC_TIME_SCALE_CHANGE               = (1<<19),
    ENC_RC_TRANS_RATE_CHANGE            = (1<<21),
    ENC_RC_TARGET_RATE_CHANGE           = (1<<22),
    ENC_ROT_PARAM_CHANGE                = (1<<23),
    ENC_NR_PARAM_CHANGE                 = (1<<25),
    ENC_NR_WEIGHT_CHANGE                = (1<<26),
    ENC_CHANGE_SET_PARAM_ALL            = (0xFFFFFFFF),
} ChangeCommonParam;

/**
* @brief This is an enumeration for encoder parameter change. (WAVE5 encoder only)
*/
typedef enum {
    // COMMON parameters which are changed frame by frame.
    ENC_SET_PPS_PARAM_CHANGE                = (1<<1),
    ENC_SET_RC_FRAMERATE_CHANGE             = (1<<4),
    ENC_SET_INDEPEND_SLICE_CHANGE           = (1<<5),
    ENC_SET_DEPEND_SLICE_CHANGE             = (1<<6),
    ENC_SET_RDO_PARAM_CHANGE                = (1<<8),
    ENC_SET_RC_TARGET_RATE_CHANGE           = (1<<9),
    ENC_SET_RC_PARAM_CHANGE                 = (1<<10),
    ENC_SET_RC_MIN_MAX_QP_CHANGE            = (1<<11),
    ENC_SET_RC_BIT_RATIO_LAYER_CHANGE       = (1<<12),
    ENC_SET_BG_PARAM_CHANGE                 = (1<<18),
    ENC_SET_CUSTOM_MD_CHANGE                = (1<<19),
} Wave5ChangeParam;

/**
* @brief This is a data structure for encoding parameters that have changed.
*/
typedef struct {
/**
@verbatim

@* 0 : changes the COMMON parameters. 
@* 1 : Reserved
@endverbatim
*/
    int changeParaMode;                 
    int enable_option;                  /**< <<vpuapi_h_ChangeCommonParam>> */  

    // ENC_PIC_PARAM_CHANGE
    int losslessEnable;                 /**< It enables lossless coding. */
    int constIntraPredFlag;             /**< It enables constrained intra prediction. */
    int chromaCbQpOffset;               /**< The value of chroma(Cb) QP offset (only for WAVE420L) */
    int chromaCrQpOffset;               /**< The value of chroma(Cr) QP offset (only for WAVE420L) */


    // ENC_INTRA_PARAM_CHANGE
/**
@verbatim
The type of I picture to be inserted at every intraPeriod

@* 0 : Non-IRAP
@* 1 : CRA
@* 2 : IDR
@endverbatim
*/
    int decodingRefreshType;            
    int intraPeriod;                    /**< A period of intra picture in GOP size */ 
    int intraQP;                        /**< A quantization parameter of intra picture */

    // ENC_CONF_WIN_TOP_BOT_CHANGE
    int confWinTop;                     /**< A top offset of conformance window */
    int confWinBot;                     /**< A bottom offset of conformance window */

    // ENC_CONF_WIN_LEFT_RIGHT_CHANGE
    int confWinLeft;                    /**< A left offset of conformance window */
    int confWinRight;                   /**< A right offset of conformance window */

    // ENC_FRAME_RATE_CHANGE
    int frameRate;                      /**< A frame rate indicator ( x 1024) */

    // ENC_INDEPENDENT_SLICE_CHANGE
/**
@verbatim
A slice mode for independent slice 

@* 0 : no multi-slice
@* 1 : Slice in CTU number  
@endverbatim
*/
    int independSliceMode;              
    int independSliceModeArg;           /**< The number of CTU for a slice when independSliceMode is set with 1  */

    // ENC_DEPENDENT_SLICE_CHANGE
/**
@verbatim
A slice mode for dependent slice 

@* 0 : no multi-slice
@* 1 : Slice in CTU number  
@* 2 : Slice in number of byte
@endverbatim
*/
    int dependSliceMode;                
    int dependSliceModeArg;             /**< The number of CTU or bytes for a slice when dependSliceMode is set with 1 or 2  */

    // ENC_INTRA_REFRESH_CHANGE
/**
@verbatim
An intra refresh mode

@* 0 : No intra refresh
@* 1 : Row
@* 2 : Column 
@* 3 : Step size in CTU
@endverbatim
*/
    int intraRefreshMode;               
    int intraRefreshArg;                /**< The number of CTU (only valid when intraRefreshMode is 3.) */

    // ENC_PARAM_CHANGE
/**
@verbatim
It uses a recommended ENC_PARAM setting.

@* 0 : Custom 
@* 1 : Recommended ENC_PARAM
@* 2 ~ 3  : Reserved
@endverbatim
*/
    int useRecommendEncParam;           
    int scalingListEnable;              /**< It enables a scaling list. */
/**
@verbatim
It enables CU(Coding Unit) size to be used in encoding process. Host application can also select multiple CU sizes. 

@* 0 : 8x8
@* 1 : 16x16
@* 2 : 32x32 
@endverbatim
*/
    int cuSizeMode;                   
    int tmvpEnable;                     /**< It enables temporal motion vector prediction. */
    int wppEnable;                      /**< It enables WPP (Wave-front Parallel Processing). WPP is unsupported in ring buffer mode of bitstream buffer.  */
    int maxNumMerge;                    /**< Maximum number of merge candidates (0~2) */
    int dynamicMerge8x8Enable;          /**< It enables dynamic merge 8x8 candidates. */
    int dynamicMerge16x16Enable;        /**< It enables dynamic merge 16x16 candidates. */
    int dynamicMerge32x32Enable;        /**< It enables dynamic merge 32x32 candidates. */
    int disableDeblk;                   /**< It disables in-loop deblocking filtering. */
    int lfCrossSliceBoundaryEnable;     /**< It enables filtering across slice boundaries for in-loop deblocking. */
    int betaOffsetDiv2;                 /**< It enables BetaOffsetDiv2 for deblocking filter. */
    int tcOffsetDiv2;                   /**< It enables TcOffsetDiv3 for deblocking filter. */
    int skipIntraTrans;                 /**< It enables transform skip for an intra CU. */
    int saoEnable;                      /**< It enables SAO (Sample Adaptive Offset). */
    int intraInInterSliceEnable;        /**< It enables to make intra CUs in an inter slice. */
    int intraNxNEnable;                 /**< It enables intra NxN PUs. */

    // ENC_RC_PARAM_CHANGE
/**
@verbatim
@* WAVE420 
@** 0 : Rate control is off.
@** 1 : Rate control is on. 

@* CODA9
@** 0 : Constant QP (VBR, rate control off)
@** 1 : Constant Bit-Rate (CBR)
@** 2 : Average Bit-Rate (ABR)
@** 4 : Picture level rate control 
@endverbatim
*/	
    int rcEnable;                       
    int intraQpOffset;                  /**< It specifies an intra QP offset relative to an inter QP. It is only valid when RateControl is enabled. */
/**
@verbatim
It specifies encoder initial delay. It is only valid when RateControl is enabled.

 encoder initial delay = InitialDelay * InitBufLevelx8 / 8

@endverbatim
*/
	int initBufLevelx8;                 
/**
@verbatim
It specifies picture bits allocation mode.
It is only valid when RateControl is enabled and GOP size is larger than 1.

@* 0 : More referenced pictures have better quality than less referenced pictures
@* 1 : All pictures in a GOP have similar image quality
@* 2 : Each picture bits in a GOP is allocated according to FixedRatioN
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

    int cuLevelRCEnable;                /**< It enables CU level rate control. */
    int hvsQPEnable;                    /**< It enables CU QP adjustment for subjective quality enhancement. */
    int hvsQpScaleEnable;               /**< It enables QP scaling factor for CU QP adjustment when hvsQPEnable is 1. */
    int hvsQpScale;                     /**< QP scaling factor for CU QP adjustment when hvcQpenable is 1. */
    int initialDelay;                   /**< An initial cpb delay in msec */
    Uint32 initialRcQp;                 /**< The value of initial QP by HOST application. This value is meaningless if INITIAL_RC_QP is 63. (only for WAVE420L) */

    // ENC_RC_MIN_MAX_QP_CHANGE
    int minQp;                          /**< Minimum QP for rate control */
    int maxQp;                          /**< Maximum QP for rate control */
    int maxDeltaQp;                     /**< Maximum delta QP for rate control */

    // ENC_TARGET_RATE_CHANGE
    int bitRate;                        /**< A target bitrate when separateBitrateEnable is 0 */
    
    // ENC_TRANS_RATE_CHANGE
    int transRate;                      /**< A peak transmission bitrate in bps */

    // ENC_RC_INTRA_MIN_MAX_CHANGE
    int intraMaxQp;                     /**< It specifies a maximum QP for intra picture (0 ~ 51). It is only valid when RateControl is 1. */
    int intraMinQp;                     /**< It specifies a minimum QP for intra picture (0 ~ 51). It is only valid when RateControl is 1. */

    // ENC_ROT_PARAM_CHANGE
    int rotEnable;                      /**< It enables or disable rotation. */
/**
@verbatim
@* [1:0] 90 degree left rotate
@** 1=90'
@** 2=180'
@** 3=270'
@* [2] vertical mirror
@* [3] horizontal mirror 
@endverbatim
*/  
    int rotMode;                        

    // ENC_NR_PARAM_CHANGE
    Uint32  nrYEnable;                  /**< It enables noise reduction algorithm to Y component.  */
    Uint32  nrCbEnable;                 /**< It enables noise reduction algorithm to Cb component. */
    Uint32  nrCrEnable;                 /**< It enables noise reduction algorithm to Cr component. */
    Uint32  nrNoiseEstEnable;           /**< It enables noise estimation for reduction. When this is disabled, noise estimation is carried out ouside VPU. */
    Uint32  nrNoiseSigmaY;              /**< It specifies Y noise standard deviation if no use of noise estimation (nrNoiseEstEnable is 0). */
    Uint32  nrNoiseSigmaCb;             /**< It specifies Cb noise standard deviation if no use of noise estimation (nrNoiseEstEnable is 0). */
    Uint32  nrNoiseSigmaCr;             /**< It specifies Cr noise standard deviation if no use of noise estimation (nrNoiseEstEnable is 0). */

    // ENC_NR_WEIGHT_CHANGE
    Uint32  nrIntraWeightY;             /**< A weight to Y noise level for intra picture (0 ~ 31). nrIntraWeight/4 is multiplied to the noise level that has been estimated. This weight is put for intra frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrIntraWeightCb;            /**< A weight to Cb noise level for intra picture (0 ~ 31).*/
    Uint32  nrIntraWeightCr;            /**< A weight to Cr noise level for intra picture (0 ~ 31).*/
    Uint32  nrInterWeightY;             /**< A weight to Y noise level for inter picture (0 ~ 31). nrInterWeight/4 is multiplied to the noise level that has been estimated. This weight is put for inter frame to be filtered more strongly or more weakly than just with the estimated noise level. */
    Uint32  nrInterWeightCb;            /**< A weight to Cb noise level for inter picture (0 ~ 31).*/
    Uint32  nrInterWeightCr;            /**< A weight to Cr noise level for inter picture (0 ~ 31).*/

    // ENC_NUM_UNITS_IN_TICK_CHANGE
    Uint32  numUnitsInTick;             /**< It specifies the number of time units of a clock operating at the frequency time_scale Hz. */ 

    // ENC_TIME_SCALE_CHANGE
    Uint32 timeScale;                   /**< It specifies the number of time units that pass in one second. */ 

    // belows are only for WAVE5 encoder
    Uint32 numTicksPocDiffOne;
    Uint32 monochromeEnable;
    Uint32 strongIntraSmoothEnable;
    Uint32 weightPredEnable;
    Uint32 bgDetectEnable;
    Uint32 bgThrDiff;
    Uint32 bgThrMeanDiff;
    Uint32 bgLambdaQp;
    int    bgDeltaQp;
    Uint32 customLambdaEnable;
    Uint32 customMDEnable;
    int    pu04DeltaRate;
    int    pu08DeltaRate;
    int    pu16DeltaRate;
    int    pu32DeltaRate;
    int    pu04IntraPlanarDeltaRate;
    int    pu04IntraDcDeltaRate;
    int    pu04IntraAngleDeltaRate;
    int    pu08IntraPlanarDeltaRate;
    int    pu08IntraDcDeltaRate;
    int    pu08IntraAngleDeltaRate;
    int    pu16IntraPlanarDeltaRate;
    int    pu16IntraDcDeltaRate;
    int    pu16IntraAngleDeltaRate;
    int    pu32IntraPlanarDeltaRate;
    int    pu32IntraDcDeltaRate;
    int    pu32IntraAngleDeltaRate;
    int    cu08IntraDeltaRate;
    int    cu08InterDeltaRate;
    int    cu08MergeDeltaRate;
    int    cu16IntraDeltaRate;
    int    cu16InterDeltaRate;
    int    cu16MergeDeltaRate;
    int    cu32IntraDeltaRate;
    int    cu32InterDeltaRate;
    int    cu32MergeDeltaRate;
    int    coefClearDisable;
    int    seqRoiEnable;
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
* @brief    This is a data structure for configuring H.264/AVC-specific parameters in encoder applications.
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
    int deblkFilterOffsetAlpha; /**< deblk_filter_offset_alpha (-6 - 6) */  
    int deblkFilterOffsetBeta;  /**< deblk_filter_offset_beta (-6 - 6)  */      
    int chromaQpOffset;         /**< chroma_qp_offset (-12 - 12)  */ 
/**
@verbatim
@* 0 : disable
@* 1 : enable

If this is 1, the encoder generates AUD RBSP at the start of every picture.
@endverbatim
*/
    int audEnable;
/**
@verbatim
@* 0 : disable
@* 1 : enable

If this is 1, the encoder generates frame_cropping_flag syntax at the SPS header.
@endverbatim
*/
    int frameCroppingFlag;
    int frameCropLeft;      /**< The sample number of left cropping region in a line. See the frame_crop_left_offset syntax in AVC/H.264 SPS tabular form. The least significant bit of this parameter should be always zero. */
    int frameCropRight;     /**< The sample number of right cropping region in a line. See the frame_crop_right_offset syntax in AVC/H.264 SPS tabular form. The least significant bit of this parameter should be always zero. */
    int frameCropTop;       /**< The sample number of top cropping region in a picture column. See the frame_crop_top_offset syntax in AVC/H.264 SPS tabular form. The least significant bit of this parameter should be always zero. */
    int frameCropBottom;    /**< The sample number of bottom cropping region in a picture column. See the frame_crop_bottom_offset syntax in AVC/H.264 SPS tabular form. The least significant bit of this parameter should be always zero. */
    
    int svcExtension;       /**< SVC extension */

    int level;              /**< H.264/AVC level_idc in SPS */
/**
@verbatim
H.264 profile_idc parameter is derived from each coding tool usage.

@* 0 : Baseline profile
@* 1 : Main profile
@* 2 : High profile
@endverbatim
*/
    int profile; 
} EncAvcParam;

/**
* @brief    This structure is used for declaring an encoder slice mode and its options. It is newly added for more flexible usage of slice mode control in encoder.
*/
typedef struct{
/**
@verbatim
@* 0 : one slice per picture
@* 1 : multiple slices per picture
@* 2 : multiple slice encoding mode 2 for H.264 only.

In normal MPEG4 mode, resync-marker and packet header are inserted between
slice boundaries. In short video header with Annex K of 0, GOB headers are inserted
at every GOB layer start. In short video header with Annex K of 1, multiple
slices are generated. In AVC mode, multiple slice layer RBSP is generated.
@endverbatim
*/
    int sliceMode;
/**
@verbatim
This parameter means the size of generated slice when sliceMode of 1.

@* 0 : sliceSize is defined by the amount of bits
@* 1 : sliceSize is defined by the number of MBs in a slice
@* 2 : sliceSize is defined by MBs run-length table (only for H.264)

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
    Uint32          bitstreamBufferSize;    /**< The size of the buffer in bytes pointed by bitstreamBuffer. This value must be a multiple of 1024. The maximum size is 16383 x 1024 bytes. */
    CodStd          bitstreamFormat;        /**< The standard type of bitstream in encoder operation. It is one of STD_MPEG4, STD_H263, STD_AVC and STD_MJPG. */
/**
@verbatim
@* 0 : disable
@* 1 : enable

This flag declares the streaming mode for the current encoder instance. Two
streaming modes, packet-based streaming with ring-buffer (buffer-reset mode) and
frame-based streaming with line buffer (buffer-flush mode), can be configured by
using this flag.

When this field is set, packet-based streaming with ring-buffer is used.
And when this field is not set, frame-based streaming with line-buffer is used.

@endverbatim
*/
    int             ringBufferEnable;
    int             picWidth;           /**< The width of a picture to be encoded in pixels. */
    int             picHeight;          /**< The height of a picture to be encoded in pixels. */
/**
@verbatim
It is a linear to tiled enable mode. 
The map type can be changed from linear to tiled format while reading source frame
in the PrP (Pre-Processing) unit.

@* 0 : disable linear to tiled-map conversion for getting source image
@* 1 : enable linear to tiled-map conversion for getting source image
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
The 16 LSB bits, [15:0], is a numerator and 16 MSB bits, [31:16], is a
denominator for calculating frame rate. The numerator means clock ticks per
second, and the denominator is clock ticks between frames minus 1.

So the frame rate can be defined by (numerator/(denominator + 1)),
which equals to (frameRateInfo & 0xffff) /((frameRateInfo >> 16) + 1).

For example, the value 30 of frameRateInfo represents 30 frames/sec, and the
value 0x3e87530 represents 29.97 frames/sec.
@endverbatim
*/
    int             frameRateInfo;
/**
@verbatim
The horizontal search range mode for Motion Estimation

@* 0 : Horizontal(-64 ~ 63)
@* 1 : Horizontal(-48 ~ 47)
@* 2 : Horizontal(-32 ~ 31)
@* 3 : Horizontal(-16 ~ 15)
@endverbatim
*/
    // [START] CODA980 & WAVE320
    int             MESearchRangeX;
/**
@verbatim
The vertical search range mode for Motion Estimation

@* 0 : Vertical(-48 ~ 47)
@* 1 : Vertical(-32 ~ 31)
@* 2 : Vertical(-16 ~ 15)
@endverbatim
*/
    int             MESearchRangeY;
/**
@verbatim
An initial QP offset for I picture in GOP.

This setting can lower an I picture QP value
down to a user-defined value ranging -4 to 4.
This value is valid for AvcEnc and ignored
when when RcEnable is 0 or RcGopIQpOff-setEn is 0.
@endverbatim
*/
    int             rcGopIQpOffsetEn;
/**
@verbatim
An enable flag for initial QP offset for I picture in GOP.

@* 0 : disable (default)
@* 1 : disable

This value is valid for AvcEnc and ignored when when RcEnable is 0.
@endverbatim
*/
    int             rcGopIQpOffset;
/**
@verbatim
The search range mode for Motion Estimation

@* 0 : Horizontal(-128 ~ 127), Vertical(-64 ~ 63)
@* 1 : Horizontal(-64 ~ 63), Vertical(-32 ~ 31)
@* 2 : Horizontal(-32 ~ 31), Vertical(-16 ~ 15)
@* 3 : Horizontal(-16 ~ 15), Vertical(-16 ~ 15)
@endverbatim
*/
    int             MESearchRange;
    // [END] CODA980 & WAVE320 
/**
@verbatim
vbv_buffer_size in bits

This value is ignored if rate control is disabled or
initialDelay is 0. The value 0 means the encoder does not check for reference
decoder buffer size constraints.
@endverbatim
*/
    // [START] CODA9xx & WAVE320
    int             vbvBufferSize;
/**
@verbatim
Frame skip indicates that encoder can skip frame encoding automatically when 
bitstream has been generated much so far considering the given target bitrate. This parameter is
ignored if rate control is not used (bitRate is 0).

@* 0 : enables frame skip function in encoder. 
@* 1 : disables frame skip function in encoder.
@endverbatim
*/
    int             frameSkipDisable;

/**
@verbatim
This is the GOP size.

@* 0 : only first picture is I
@* 1 : all I pictures
@* 2 : IPIP...
@* 3 : IPPIPP...

The maximum value is 32767, but in practice, a smaller value should be
chosen by HOST application for proper error concealment operations. This value is
ignored in case of STD_MJPG
@endverbatim
*/
    int             gopSize;
/**
An interval of adding an IDR picture
*/
    int idrInterval;
/**
@verbatim
A block mode enable flag for Motion Estimation. (H.264/AVC only).
A HOST can use some combination (bitwise or-ing) of each value under below.

@* 4'b0000 or 4'b1111 : Use all block mode
@* 4'b0001 : Enable 16x16 block mode
@* 4'b0010 : Enable 16x8 block mode
@* 4'b0100 : Enable 8x16 block mode
@* 4'b1000 : Enable 8x8 block mode
@endverbatim
*/
    int             meBlkMode;
    EncSliceMode    sliceMode;      /**< The parameter for slice mode */
/**
@verbatim
@* 0 : intra MB refresh is not used.
@* Otherwise : at least N MBs in every P-frame is encoded as intra MBs.

This value is ignored in case of STD_MJPG
@endverbatim
*/
    int             intraRefresh;
/**
@verbatim
Consecutive intra MB refresh mode

This option is valid only when IntraMbRefresh-Num[15:0] is not 0.

@* 0 - ConscIntraRefreshEn is disabled. IntraMbRefreshNumnumber of MBs are encoded
as an intra MB at the defined interval by picture
size.
@* 1 - IntraMbRefreshNumnumber of consec-utive MBs are encoded as an intra MB.
@endverbatim
*/    
    int             ConscIntraRefreshEnable;
/**
@verbatim
The maximum quantized step parameter for encoding process

In MPEG4/H.263 mode, the maximum value is 31.
In H.264 mode, allowed maximum value is 51.
@endverbatim
*/
    int             userQpMax;

    //h.264 only
    int             maxIntraSize;   /**< The maximum bit size for intra frame or encoding process. It works only in H.264/AVC encoder. */
    int             userMaxDeltaQp; /**< The maximum delta QP for encoding process. It works only in the H.264/AVC mode. */
    int             userMinDeltaQp; /**< The minimum delta QP for encoding process. It works only in the H.264/AVC mode. */
    int             userQpMin;    /**< The minimum quantized step parameter for encoding process. (CODA9 H.264/AVC only) */
/**
@verbatim
The PMV option for Motion Estimation.
If this field is 1, encoding quality could be worse than when it was zero.

@* 0 : Motion Estimation engine uses PMV that was derived from neighbor MV
@* 1 : Motion Estimation engine uses Zero PMV
@endverbatim
*/
    int             MEUseZeroPmv;       
/**
@verbatim
Additional weight of intra cost for mode decision to reduce intra MB density

By default, it could be zero.
If this register have some value W,
and the cost of best intra mode that was decided by Refine-Intra-Mode-Decision is ICOST,
the Final Intra Cost FIC is like below,

FIC = ICOST + W

So, if this field is not zero,
the Final Intra Cost have additional weight. Then the Intra/Inter mode decision logic tend to make more Inter-Macroblock.
@endverbatim
*/
    int             intraCostWeight;    
/**
@verbatim
The quantization parameter for I frame

When this value is -1, the quantization
parameter for I frames is automatically determined by VPU. This value is ignored
in case of STD_MJPG
@endverbatim
*/
    //mp4 only
    int             rcIntraQp;
/**
@verbatim
A gamma is a smoothing factor in motion estimation. A value for gamma is
factor * 32768, the factor value is selected from the range 0 &le; factor &ge; 1.

@* If the factor value getting close to 0, Qp changes slowly.
@* If the factor value getting close to 1, Qp changes quickly.

The default gamma value is 0.75 * 32768
@endverbatim
*/
    int             userGamma;
/**
@verbatim
Encoder Rate Control Mode setting

@* 0 : Normal mode rate control
@* 1 : FRAME_LEVEL rate control
@* 2 : SLICE_LEVEL rate control
@* 3 : USER DEFINED MB LEVEL rate control
@endverbatim
*/
    int             rcIntervalMode;     
/**
@verbatim
The user defined MB interval value

This value is used only when rcIntervalMode is 3.
@endverbatim
*/
    int             mbInterval;         
/**
@verbatim
Target bit rate in kbps

If 0, there is no rate control, and pictures are encoded with a quantization parameter equal to quantParam in EncParam.
@endverbatim
*/
    // [END] CODA9xx & WAVE320
    int             bitRate;
/**
@verbatim
Time delay (in mili-seconds)

It takes for the bitstream to reach initial occupancy of the vbv buffer from zero level.

This value is ignored if rate
control is disabled. The value 0 means the encoder does not check for reference
decoder buffer delay constraints.
@endverbatim
*/
    int             initialDelay;
/**
@verbatim
@* WAVE420 
@** 0 : Rate control is off.
@** 1 : Rate control is on. 

@* CODA9
@** 0 : Constant QP (VBR, rate control off)
@** 1 : Constant Bit-Rate (CBR)
@** 2 : Average Bit-Rate (ABR)
@** 4 : Picture level rate control 
@endverbatim
*/
    int             rcEnable;           

    union {
        EncMp4Param     mp4Param;       /**< The parameters for MPEG4 part 2 Visual  */
        EncH263Param    h263Param;      /**< The parameters for ITU-T H.263 */
        EncAvcParam     avcParam;       /**< The parameters for ITU-T H.263 */
        EncHevcParam    hevcParam;      /**< The parameters for ITU-T H.265/HEVC */
    } EncStdParam;

    // Maverick-II Cache Configuration
/**
@verbatim
Cache MC bypass

@* 0 : MC uses a cache.
@* 1 : MC does not use a cache.
@endverbatim
*/
    int             cacheBypass;
/**
@verbatim
It enables two-frame caching mode.

@* 1 : Dual mode (caching for two frame indexes)
@* 0 : Single mode (caching for one frame index)
@endverbatim
*/
    int             cacheDual;
/**
@verbatim
@* 0 : CbCr data is written in separate frame memories (chroma separate mode)
@* 1 : CbCr data is interleaved in chroma memory. (chroma interleave mode)
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
Frame buffer endianness

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format
    
NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE4 Datasheet'. 
@endverbatim
*/
    int             frameEndian;
/**
@verbatim
Bistream buffer endianness

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format
    
NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE4 Datasheet'. 
@endverbatim
*/
    int             streamEndian;
/**
@verbatim
Endianness of source YUV

@* 0 : Little endian format
@* 1 : Big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format
    
NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE4 Datasheet'. 
@endverbatim
*/    
    int             sourceEndian;
/**
@verbatim    
It writes output with 8 burst in linear map mode.

@* 0 : Burst write back is disabled
@* 1 : Burst write back is enabled.

NOTE: WAVE4 does not support it. 
@endverbatim
*/
    int             bwbEnable;
/**
@verbatim
@* 0 : Disable
@* 1 : Enable

This flag is used for AvcEnc frame-based streaming with line buffer.
If coding standard is not AvcEnc or ringBufferEnable is 1, this flag is ignored.

If this field is set, VPU sends a buffer full interrupt when line buffer is full, and waits until the interrupt is cleared.
HOST should read the bitstream in line buffer and clear the interrupt.
If this field is not set, VPU does not send a buffer full interrupt when line buffer is full.
In both cases, VPU sets RET_ENC_PIC_FLAG register to 1 if line buffer is full.
@endverbatim
*/
    int             lineBufIntEn;
/**
@verbatim
This flag indicates whether source images are in packed format.

@* 0 : Not packed mode
@* 1 : Packed mode
@endverbatim
*/    
    int             packedFormat;
/**
This flag indicates a color format of source image which is one among <<vpuapi_h_FrameBufferFormat>>.
*/    
    int             srcFormat;
    int             srcBitDepth;

/**
@verbatim
VPU core index number

* 0 ~ number of VPU core - 1
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



    Uint32 encodeHrdRbspInVPS;        /**< It encodes the HRD syntax into VPS. */
    Uint32 encodeHrdRbspInVUI;        /**< It encodes the HRD syntax into VUI. */
    Uint32 hrdRbspDataSize;           /**< The bit size of the HRD rbsp data */
    PhysicalAddress hrdRbspDataAddr;  /**< The address of the HRD rbsp data */

    Uint32 encodeVuiRbsp;     /**< A flag to encode the VUI syntax in rbsp which is given by HOST application */
    Uint32 vuiRbspDataSize;   /**< The bit size of the VUI rbsp data */
    PhysicalAddress vuiRbspDataAddr;   /**< The address of the VUI rbsp data */

    Uint32          virtAxiID;
    BOOL            enablePTS; /**< An enable flag to report PTS(Presentation Timestamp. */
#ifdef SUPPORT_980_RC_LIB
    int             coda9RoiEnable;  /**< It enables ROI for CODA980. */
    int             RoiPicAvgQp;     /**< It specifies an ROI average QP. */
    char            RoiFile[1024];   /**< It specifies the ROI file. */

    int             set_dqp_pic_num; /**< It is the number of cyclic pictures. */

    gop_entry_t     gopEntry[MAX_GOP_SIZE *2];  /**< The entire structure of cyclic pictures  */
    int             gopPreset;                  /**< GOP preset value (0~5) */

    int             LongTermPeriod;  /**< It specifies an interval value of longterm period. */
    int             LongTermDeltaQp; /**< It specifies a delta QP value between base(intra) picture and longterm inter picture.   */

    int             dqp[8];          /**< A delta QP value of frame# within the cyclic pictures  */
    int             HvsQpScaleDiv2;  /**< It specifies the scale for MB QP derivation. It is only avilable when EN_HVS_QP is 1. */
    int             EnHvsQp;         /**< It enables HVS QP. */
    int             EnRowLevelRc;    /**< It enables row-level rate control. */
    int             RcInitialQp;     /**< An initial QP for rate control */
    int             RcHvsMaxDeltaQp; /**< A maximum delta QP value for HVS considered rate control */
#ifdef ROI_MB_RC
    int             roi_max_delta_qp_minus;
    int             roi_max_delta_qp_plus;
#endif
#endif
#ifdef AUTO_FRM_SKIP_DROP
    int             enAutoFrmSkip; /**< It enables frame skip automatically according to threshold */
    int             enAutoFrmDrop; /**< It enables frame drop automatically according to threshold */
    int             vbvThreshold;  /**< Vbv buffer threshold value */
    int             qpThreshold;   /**< Qp buffer threshold value */
    int             maxContinuosFrameSkipNum;  /**< It specifies the maximum number of continusous frame skip. */
    int             maxContinuosFrameDropNum;  /**< It specifies the maximum number of continusous frame drop. */
#endif
    int             rcWeightFactor; /**< Adjusts the speed of updating parameters to the rate control model. If RcWeightFactor is set with a large value, the RC parameters are updated slowly. (Min: 1, Max: 32, default: 2) */

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
    The number of intra MB to be inserted in picture (CODA9 only)

    @* 0 : intra MB refresh is not used.
    @* Other value : intraRefreshNum of MBs are encoded as intra MBs in every P frame.

    @endverbatim
    */
    int             intraRefreshNum;
} EncOpenParam;

/**
* @brief    This is a data structure for parameters of VPU_EncGetInitialInfo() which
is required to get the minimum required buffer count in HOST application. This
returned value is used to allocate frame buffers in
VPU_EncRegisterFrameBuffer().
*/
typedef struct {
    int             minFrameBufferCount; /**< Minimum number of frame buffer */
    int             minSrcFrameCount;    /**< Minimum number of source buffer */
} EncInitialInfo;

/**
* @brief    This is a data structure for setting ROI. (CODA9 encoder only).
 */
typedef struct {
/**
@verbatim
@* 0 : It disables ROI.
@* 1 : User can set QP value for both ROI and non-ROI region
@* 2 : User can set QP value for only ROI region (Currently, not support)
@endverbatim
*/
    int mode;
/**
@verbatim
Total number of ROI

The maximum value is MAX_ROI_NUMBER. 
MAX_ROI_NUMBER can be either 10 or 50. In order to use MAX_ROI_NUMBER as 50, 
SUPPORT_ROI_50 define needs to be enabled in the reference SW. 
@endverbatim
*/
    int number;                     
    VpuRect region[MAX_ROI_NUMBER]; /**< Rectangle information for ROI */
    int qp[MAX_ROI_NUMBER];         /**< QP value for ROI region */
} AvcRoiParam;

/**
* @brief    This is a data structure for setting NAL unit coding options.
 */
typedef struct {
    int implicitHeaderEncode;       /**< Whether HOST application encodes a header implicitly or not. If this value is 1, below encode options are ignored. */
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

#ifdef ROI_MB_RC
/**
* @brief    This is a data structure for setting ROI.
 */

typedef struct {
    Uint32 left;    /**<  A horizontal pixel offset of top-left corner of rectangle from (0, 0), top-left corner of a frame. */
    Uint32 top;     /**< A vertical pixel offset of top-left corner of rectangle from (0, 0), top-left corner of a frame. */
    Uint32 right;   /**< A horizontal pixel offset of bottom-right corner of rectangle from (0, 0), bottom-right corner of a frame. */
    Uint32 bottom;  /**< A vertical pixel offset of bottom-right corner of rectangle from (0, 0), bottom-right corner of a frame. */
} Rect;

typedef struct {
/**
@verbatim
Total number of ROI

The maximum value is MAX_ROI_NUMBER.
MAX_ROI_NUMBER can be either 10 or 50. In order to use MAX_ROI_NUMBER as 50,
SUPPORT_ROI_50 define needs to be enabled in the reference SW.
@endverbatim
*/
    int number;
    Rect region[MAX_ROI_NUMBER];    /**< Rectangle information for ROI */
    int qp[MAX_ROI_NUMBER];         /**< QP value for ROI region */
    int roi_file;
} EncSetROI;
#endif

/**
* @brief This is a data structure for configuring one frame encoding operation.
 */
 typedef struct {
    FrameBuffer*    sourceFrame;    /**< This member must represent the frame buffer containing source image to be encoded. */
/**
@verbatim
If this value is 0, the picture type is determined by VPU
according to the various parameters such as encoded frame number and GOP size.

If this value is 1, the frame is encoded as an I-picture regardless of the
frame number or GOP size, and I-picture period calculation is reset to
initial state. In MPEG4 and H.263 case, I-picture is sufficient for decoder
refresh. In H.264/AVC case, the picture is encoded as an IDR (Instantaneous
Decoding Refresh) picture.

This value is ignored if skipPicture is 1.
@endverbatim
*/
    int             forceIPicture;
/**
@verbatim
If this value is 0, the encoder encodes a picture as normal.

If this value is 1, the encoder ignores sourceFrame and generates a skipped
picture. In this case, the reconstructed image at decoder side is a duplication
of the previous picture. The skipped picture is encoded as P-type regardless of
the GOP size.
@endverbatim
*/
    int             skipPicture;
    int             quantParam;         /**< This value is used for all quantization parameters in case of VBR (no rate control).  */
/**
@verbatim
The start address of a picture stream buffer under line-buffer mode and dynamic
buffer allocation. (CODA encoder only)

This variable represents the start of a picture stream for encoded output. In
buffer-reset mode, HOST might use multiple picture stream buffers for the
best performance. By using this variable, applications could re-register the
start position of the picture stream while issuing a picture encoding operation.
This start address of this buffer must be 4-byte aligned, and its size is
specified the following variable, picStreamBufferSize. In packet-based streaming
with ring-buffer, this variable is ignored.

NOTE: This variable is only meaningful when both line-buffer mode and dynamic
buffer allocation are enabled.
@endverbatim
*/
    PhysicalAddress picStreamBufferAddr;
/**
@verbatim
The byte size of a picture stream chunk

This variable represents the byte size of a picture stream buffer. This variable is
so crucial in line-buffer mode. That is because encoder output could be
corrupted if this size is smaller than any picture encoded output. So this value
should be big enough for storing multiple picture streams with average size. In
packet-based streaming with ring-buffer, this variable is ignored.
@endverbatim
*/
   int             picStreamBufferSize;
/**
@verbatim
@* 0 : progressive (frame) encoding mode
@* 1 : interlaced (field) encoding mode
@endverbatim
*/
    int             fieldRun;              



    int             coda9RoiEnable;  /**< It enables ROI for CODA980. */
    Uint32 coda9RoiPicAvgQp;        /**< A average value of ROI QP for a picture (CODA9 only) */
    PhysicalAddress roiQpMapAddr;   /**< The start address of ROI QP map (CODA9 only) */
    Uint32 nonRoiQp;                /**< A non-ROI QP for a picture */

#ifdef ROI_MB_RC
    AvcRoiParam     setROI;             /**< This value sets ROI. If setROI mode is 0, ROI does work and other member value of setROI is ignored. */
#endif

    HevcCtuOptParam ctuOptParam;        /**< <<vpuapi_h_HevcCtuOptParam>> for H.265/HEVC encoding */ 



    int             forcePicQpEnable;   /**< A flag to use a force picture quantization parameter */
    int             forcePicQpSrcOrderEnable; /**< A flag to use a force picture QP by source index order */
    int             forcePicQpI;        /**< A force picture quantization parameter for I picture */
    int             forcePicQpP;        /**< A force picture quantization parameter for P picture */
    int             forcePicQpB;        /**< A force picture quantization parameter for B picture */
    int             forcePicTypeEnable; /**< A flag to use a force picture type */
    int             forcePicTypeSrcOrderEnable; /**< A flag to use a force picture type by source index order */
    int             forcePicType;       /**< A force picture type (I, P, B, IDR, CRA) */
    int             srcIdx;             /**< A source frame buffer index */
    int             srcEndFlag;         /**< A flag indicating that there is no more source frame buffer to encode */

    EncCodeOpt      codeOption;             /**< <<vpuapi_h_EncCodeOpt>> */ 



    Uint32  useCurSrcAsLongtermPic; /**< A flag for the current picture to be used as a longterm reference picture later when other picture's encoding  */
    Uint32  useLongtermRef;         /**< A flag to use a longterm reference picture in DPB when encoding the current picture */
    Uint32  pts;                    /**< The presentation Timestamp (PTS) of input source */

    // belows are newly added for WAVE5 encoder
    HevcCustomMapOpt customMapOpt;  /**< <<vpuapi_h_HevcCustomMapOpt>> */
    Uint32 wpPixVarianceY;
    Uint32 wpPixVarianceCb;
    Uint32 wpPixVarianceCr;
    Uint32 wpPixMeanY;
    Uint32 wpPixMeanCb;
    Uint32 wpPixMeanCr;
    Uint32 forceAllCtuCoefDropEnable;

} EncParam;

/**
* @brief This structure is used for reporting encoder information.
*/
typedef struct {
/**
@verbatim
@* 0 : reporting disable
@* 1 : reporting enable
@endverbatim
*/
    int enable;
    int type;               /**< This value is used for picture type reporting in MVInfo. */ 
    int sz;                 /**< This value means size for each reporting data (MBinfo, MVinfo, Sliceinfo).    */ 
    PhysicalAddress addr;   /**< The start address of each reporting buffer into which encoder puts data.  */ 
} EncReportInfo;

#ifdef SUPPORT_W5ENC_BW_REPORT
/**
* @brief This structure is used for reporting bandwidth (only for WAVE5).
*/
typedef struct {
    osal_file_t fpBW;
    Uint32 prpBwRead;
    Uint32 prpBwWrite;
    Uint32 fbdYRead;
    Uint32 fbcYWrite;
    Uint32 fbdCRead;
    Uint32 fbcCWrite;
    Uint32 priBwRead;
    Uint32 priBwWrite;
    Uint32 secBwRead;
    Uint32 secBwWrite;
    Uint32 procBwRead;
    Uint32 procBwWrite;
}EncBwMonitor;
#endif

/**
* @brief    This is a data structure for reporting the results of picture encoding operations.
*/
typedef struct {
/**
@verbatim
The Physical address of the starting point of newly encoded picture stream

If dynamic buffer allocation is enabled in line-buffer mode, this value is
identical with the specified picture stream buffer address by HOST.
@endverbatim
*/
    PhysicalAddress bitstreamBuffer;
    Uint32 bitstreamSize;   /**< The byte size of encoded bitstream */
    int bitstreamWrapAround;/**< This is a flag to indicate whether bitstream buffer operates in ring buffer mode in which read and write pointer are wrapped arounded. If this flag is 1, HOST application needs a larger buffer. */
/**
@verbatim
Coded picture type

@* H.263 and MPEG4
@** 0 : I picture
@** 1 : P picture

@* H.264/AVC
@** 0 : IDR picture
@** 1 : Non-IDR picture
@endverbatim
*/
    int picType;
    int numOfSlices;        /**<  The number of slices of the currently being encoded Picture  */
    int reconFrameIndex;    /**<  A reconstructed frame index. The reconstructed frame is used for reference of future frame.  */
    FrameBuffer reconFrame; /**<  A reconstructed frame address and information. Please refer to <<vpuapi_h_FrameBuffer>>.  */
    int rdPtr;              /**<  A read pointer in bitstream buffer, which is where HOST has read encoded bitstream from the buffer */
    int wrPtr;              /**<  A write pointer in bitstream buffer, which is where VPU has written encoded bitstream into the buffer  */
#ifdef AUTO_FRM_SKIP_DROP
    int picDropped;         /**< A flag which represents whether the current encoding has been dropped or not.*/
#endif

    // for WAVE420
    int picSkipped;         /**< A flag which represents whether the current encoding has been skipped or not. */
    int numOfIntra;         /**< The number of intra coded block */
    int numOfMerge;         /**< The number of merge block in 8x8 */
    int numOfSkipBlock;     /**< The number of skip block in 8x8 */
    int avgCtuQp;           /**< The average value of CTU QPs */
    int encPicByte;         /**< The number of encoded picture bytes   */
    int encGopPicIdx;       /**< GOP index of the current picture */
    int encPicPoc;          /**< POC(picture order count) value of the current picture */
    int encSrcIdx;          /**< Source buffer index of the current encoded picture */
    int encVclNal;          /**< Encoded NAL unit type of VCL */
    int encPicCnt;          /**< Encoded picture number */

    // Report Information
    EncReportInfo mbInfo;   /**<  The parameter for reporting MB data . Please refer to <<vpuapi_h_EncReportInfo>> structure. */
    EncReportInfo mvInfo;   /**<  The parameter for reporting motion vector. Please refer to <<vpuapi_h_EncReportInfo>> structure. */
    EncReportInfo sliceInfo;/**<  The parameter for reporting slice information. Please refer to <<vpuapi_h_EncReportInfo>> structure. */
    int frameCycle;         /**<  The parameter for reporting the cycle number of decoding/encoding one frame.*/
    Uint64      pts;        /**< Presentation Timestamp of encoded picture. */
    Uint32      encInstIdx;     /**< An index of instance which have finished encoding with a picture at this command. This is only for Multi-Core encoder product. */
    Uint32  encPrepareCycle;
    Uint32  encProcessingCycle;
    Uint32  encEncodingCycle;
} EncOutputInfo;

/**
 * @brief   This structure is used when HOST processor additionally wants to get SPS data or
PPS data from encoder instance. The resulting SPS data or PPS data can be used
in real applications as a kind of out-of-band information.
*/
 typedef struct {
    PhysicalAddress paraSet;/**< An array of 32 bits which contains SPS RBSP */
    int size;               /**< The size of stream in byte */
} EncParamSet;

/**
 * @brief   This structure is used for adding a header syntax layer into the encoded bit
stream. The parameter headerType is the input parameter to VPU, and the
other two parameters are returned values from VPU after completing
requested operation.
*/
 typedef struct {
    PhysicalAddress buf;        /**< A physical address pointing the generated stream location  */
    BYTE *pBuf;                 /**< The virtual address according to buf. This address is needed when a HOST wants to access encoder header bitstream buffer. */
    size_t  size;               /**< The size of the generated stream in bytes */
    Int32   headerType;         /**< This is a type of header that HOST wants to generate and have values as VOL_HEADER, VOS_HEADER, VO_HEADER, SPS_RBSP or PPS_RBSP. */
    BOOL    zeroPaddingEnable;  /**< It enables header to be padded at the end with zero for byte alignment. */
    Int32   failReasonCode;     /**< Not defined yet */
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
    SVC_RBSP_SEI,   /**< SEI for SVC */

    END_OF_SEQUENCE,/**< End of sequence nal for H.264/AVC encoder (CODA7Q only) */
    END_OF_STREAM   /**< End of stream nal for H.264/AVC encoder (CODA7Q only) */
} AvcHeaderType;

/**
 * @brief   This is a special enumeration type for explicit encoding headers such as VPS, SPS, PPS. (WAVE encoder only)
*/
typedef enum {
    CODEOPT_ENC_VPS             = (1 << 2), /**< A flag to encode VPS nal unit explicitly */
    CODEOPT_ENC_SPS             = (1 << 3), /**< A flag to encode SPS nal unit explicitly */
    CODEOPT_ENC_PPS             = (1 << 4), /**< A flag to encode PPS nal unit explicitly */
} HevcHeaderType ;

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
    PRESET_IDX_T0S              = 16,   /**< Bitmain preset1 */
    PRESET_IDX_T1S              = 17,   /**< Bitmain preset2 */
    PRESET_IDX_T1L              = 18,   /**< Bitmain preset3 */
    PRESET_IDX_T2ST1S           = 19,   /**< Bitmain preset4 */
    PRESET_IDX_T2ST1L           = 20,   /**< Bitmain preset5 */
} GOP_PRESET_IDX;


#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief This function returns whether processing a frame by VPU is completed or not.
* @return
@verbatim
@* 0 : VPU hardware is idle.
@* Non-zero value : VPU hardware is busy with processing a frame.
@endverbatim
*/
Int32  VPU_IsBusy(
    Uint32 coreIdx  /**< [input] An index of VPU core */
    );

/**
* @brief 
@verbatim
This function makes HOST application wait until VPU finishes processing a frame,
or check a busy flag of VPU during the given timeout period.  
The behavior of this function depends on VDI layer\'s implementation.

NOTE: Timeout may not work according to implementation of VDI layer.
@endverbatim
* @param coreIdx [input] An index of VPU core
* @param timeout [output] See return value.
* @return
@verbatim
* 1 : Wait time out 
* Non -1 value : The value of InterruptBit
@endverbatim
*/
Int32 VPU_WaitInterrupt(
    Uint32 coreIdx,
    int timeout
    );

Int32 VPU_WaitInterruptEx(
    VpuHandle handle, 
    int timeout
    );

/**
 * @brief This function returns whether VPU is currently running or not.
 * @param coreIdx [input] An index of VPU core
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
* @brief 
@verbatim 
This function initializes VPU hardware and its data
structures/resources. HOST application must call this function only once before calling
VPU_DeInit().

NOTE: Before use, HOST application needs to define the header file path of BIT firmware to BIT_CODE_FILE_PATH.
@endverbatim
*/
 RetCode VPU_Init(
    Uint32 coreIdx  /**<[input] An index of VPU core. This value can be from 0 to (number of VPU core - 1).*/
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
* @brief    This function frees all the resources allocated by VPUAPI and releases the device driver.
VPU_Init() and VPU_DeInit() always work in pairs. 
* @return none
*/
RetCode VPU_DeInit(
    Uint32 coreIdx  /**< [Input] An index of VPU core */
    );

/**
* @brief This function returns the number of instances opened. 
* @return      The number of instances opened
*/
int VPU_GetOpenInstanceNum(
    Uint32 coreIdx          /**< [Input] An index of VPU core */
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
be completed by calling VPU_Dec Info(). Even though the result of the
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
* @brief This function clears VPU interrupts that are pending.
* @return      none
*/
void VPU_ClearInterrupt(
    Uint32 coreIdx  /**< [Input] An index of VPU core */
    );

void VPU_ClearInterruptEx(
    VpuHandle handle, 
    Int32 intrFlag  /**< An interrupt flag to be cleared */
    );

/**
* @brief
@verbatim 

This function stops operation of the current frame and initializes VPU hardware by sending reset signals. 
It can be used when VPU is having a longer delay or seems hang-up.
After VPU has completed initialization, the context is rolled back to the state 
before calling the previous VPU_DecStartOneFrame() or VPU_EncStartOneFrame(). 
HOST can resume decoding from the next picture, instead of decoding from the sequence header. 
It works only for the current instance, so this function does not affect other instance\'s running in multi-instance operation. 

This is some applicable scenario of using `VPU_SWReset()` when a series of hang-up happens.
For example, when VPU is hung up with frame 1, HOST application calls `VPU_SWReset()` to initialize VPU and then calls `VPU_DecStartOneFrame()` for frame 2
with specifying the start address, read pointer. 
If there is still problem with frame 2, we recommend calling `VPU_SWReset()` and `seq_init()` or calling `VPU_SWReset()` and enabling iframe search.
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* :: 
Operation was done successfully, which means required information of the stream data to be
decoded was received successfully.
@endverbatim
 */
 RetCode VPU_SWReset(
    Uint32      coreIdx,    /**<[Input] An index of VPU core */
/**
@verbatim
[Input] Way of reset

@* SW_RESET_SAFETY : It waits until AXI bus completes ongoing tasks. If remaining bus transactions are done, VPU enters the reset process. (recommended mode)
@* SW_RESET_FORCE : It forces to reset VPU no matter whether bus transactions are completed or not. It might affect what other blocks do with bus, so we do not recommend using this mode. 
@* SW_RESET_ON_BOOT : This is the default reset mode that is executed once system boots up. This mode is actually executed in VPU_Init(), so does not have to be used independently.
@endverbatim
*/
    SWResetMode resetMode,
    void*       pendingInst
    );

/**
* @brief 
@verbatim
This function resets VPU as VPU_SWReset() does, but 
it is done by the system reset signal and all the internal contexts are initialized.
Therefore after the `VPU_HWReset()`, HOST application needs to call `VPU_Init()`. 

`VPU_HWReset()` requires vdi_hw_reset part of VDI module to be implemented before use. 
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* :: 
Operation was done successfully, which means required information of the stream data to be decoded was received successfully.
@endverbatim
 */
 RetCode VPU_HWReset(
    Uint32  coreIdx     /**<  [Input] An index of VPU core */
    );

/**
* @brief 
@verbatim
This function saves or restores context when VPU powers on/off. 

NOTE: This is a tip for safe operation - 
    call this function to make VPU enter into a sleep state before power down, and 
    after the power off call this function again to return to a wake state.
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* :: 
Operation was done successfully, which means required information of the stream data to be decoded was received successfully.
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
* @brief    This function returns the size of motion vector co-located buffer that are needed to decode H.265/AVC stream. 
*           The mvcol buffer should be allocated along with frame buffers and given to VPU_DecRegisterFramebuffer() as an argument.
* @return   It returns the size of required mvcol buffer in byte unit. 
*/
int VPU_GetMvColBufSize(
    CodStd codStd,      /**< [Input] Video coding standards */
    int width,          /**< [Input] Width of framebuffer */
    int height,         /**< [Input] Height of framebuffer */
    int num             /**< [Input] Number of framebuffers. */
    );

/**
* @brief    This function returns the size of FBC (Frame Buffer Compression) offset table for luma and chroma.
*           The offset tables are to look up where compressed data is located.  
*           HOST should allocate the offset table buffers for luma and chroma as well as frame buffers
*           and give their base addresses to VPU_DecRegisterFramebuffer() as an argument.
*
* @return   
@verbatim
*RETCODE_SUCCESS* :: 
Operation was done successfully, which means given value is valid and setting is done successfully.
*RETCODE_INVALID_PARAM* :: 
The given argument parameter, ysize, csize, or handle was NULL.
@endverbatim
*/
RetCode VPU_GetFBCOffsetTableSize(
    CodStd  codStd,     /**< [Input] Video coding standards */
    int     width,      /**< [Input] Width of framebuffer */
    int     height,     /**< [Input] Height of framebuffer  */
    int*     ysize,     /**< [Output] Size of offset table for Luma in bytes */
    int*     csize      /**< [Output] Size of offset table for Chroma in bytes */
    );

/**
*  @brief   This function returns the size of frame buffer that is required for VPU to decode or encode one frame. 
*  
*  @return  The size of frame buffer to be allocated
*/
int VPU_GetFrameBufSize(
    int         coreIdx,    /**< [Input] VPU core index number */
    int         stride,     /**< [Input] The stride of image  */
    int         height,     /**< [Input] The height of image */
    int         mapType,    /**< [Input] The map type of framebuffer */
    int         format,     /**< [Input] The color format of framebuffer */
    int         interleave, /**< [Input] Whether to use CBCR interleave mode or not */
    DRAMConfig* pDramCfg    /**< [Input] Attributes of DRAM. It is only valid for CODA960. Set NULL for this variable in case of other products. */
    );

/**
 *  @brief  This function returns the product ID of VPU which is currently running.
 *  @return Product information. Please refer to the <<vpuapi_h_ProductId>> enumeration. 
 */
int VPU_GetProductId(
    int coreIdx     /**< [Input] VPU core index number */
    );

// function for decode
/**
*  @brief In order to decode, HOST application must open the decoder. By calling this function, HOST application can get a handle by which they can refer to a decoder instance. Because the VPU is multiple instance codec, HOST application needs this kind of handle. Once a HOST application gets a handle, the HOST application must pass this handle to all subsequent decoder-related functions.
*
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
The given argument parameter, pOpenParam, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
*RETCODE_NOT_INITIALIZED* :: 
This means VPU was not initialized yet before calling this function.
Applications should initialize VPU by calling VPU_Init() before calling this
function.
@endverbatim
 */
 RetCode VPU_DecOpen(
    DecHandle *pHandle,     /**< [Output] A pointer to a DecHandle type variable which specifies each instance for HOST application. */
    DecOpenParam *pop       /**< [Input] A pointer to a DecOpenParam type structure which describes required parameters for creating a new decoder instance. */
    );

/**
 *  @brief      When HOST application finished decoding a sequence and wanted to release this
instance for other processing, the applicaton should close this instance. After
completion of this function call, the instance referred to by handle gets
free. Once a HOST application closes an instance, the HOST application cannot call any further
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
function.
@endverbatim
 */
 RetCode VPU_DecClose(
    DecHandle handle    /**< [Output] A decoder handle obtained from VPU_DecOpen() */
    );

/**
 * @brief
@verbatim
Applications must pass the address of <<vpuapi_h_DecInitialInfo>> structure, where the
decoder stores information such as picture size, number of necessary frame
buffers, etc. For the details, see definition of <<vpuapi_h_DecInitialInfo>> data structure.
This function should be called once after creating a decoder instance and before starting frame decoding.

It is a HOST application\'s responsibility to provide sufficient amount of bitstream to
the decoder by calling VPU_DecUpdateBitstreamBuffer() so that bitstream buffer
does not get empty before this function returns.
If HOST application cannot ensure to feed stream enough, they can use the Forced
Escape option by using VPU_DecSetEscSeqInit().
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
The given argument parameter, pInfo, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
*RETCODE_FRAME_NOT_COMPLETE* :: 
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling  VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST should call
VPU_DecGetOutputInfo() to proceed this function call.
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
+
--
Operation has not received any response from VPU and has timed out.

NOTE: When this function returns RETCODE_SUCCESS, HOST should call VPU_ClearInterrupt() function to clear out the interrupt pending status.
--
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
except for VPU_IsBusy(), VPU_DecGetBitstreamBuffer(), and VPU_DecUpdateBitstreamBuffer().

A pair of this function and VPU_DecCompleteSeqInit() or VPU_DecGetInitialInfo() should be called
at least once after creating a decoder instance and before starting frame decoding.
@endverbatim
*
* @return
@verbatim
*RETCODE_SUCCESS* :: 
Operation was done successfully, which means required information of the stream data to be
decoded was received successfully
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
VPU_DecIssueSeqInit (). Even though the result of the current frame operation is not necessary,
HOST should call VPU_DecIssueSeqInit () to proceed this function call.
*RETCODE_WRONG_CALL_SEQUENCE* :: 
This means the current API function call was invalid considering the allowed sequences between
API functions. In this case, HOST application might call this function before successfully putting
bitstream data by calling VPU_DecUpdateBitstreamBuffer(). In order to perform this functions
call, bitstream data including sequence level header should be transferred into bitstream buffer
before calling VPU_ DecIssueSeqInit ().
@endverbatim
 */
RetCode VPU_DecIssueSeqInit(
    DecHandle handle        /**<  [Input] A decoder handle obtained from VPU_DecOpen() */
    );

/**
 * @brief Application can get the information about sequence header.
Applications must pass the address of DecInitialInfo structure, where the decoder stores
information such as picture size, number of necessary frame buffers, etc.
For more details, see definition of the section called the DecInitialInfo data structure.
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
    DecInitialInfo* info        /**< [Output] A pointer to DecInitialInfo data structure */
    );

/**
 *  @brief  This is a special function to provide a way of escaping VPU hanging during DEC_SEQ_INIT.
When this flag is set to 1 and stream buffer empty happens, VPU terminates automatically
DEC_SEQ_INIT operation. If target applications ensure that high layer header syntax is
periodically sent through the channel, they do not need to use this option. But if target applications
cannot ensure that such as file-play, it might be very useful to avoid VPU hanging without HOST timeout caused
by crucial errors in header syntax.
*
* @return 
@verbatim
*RETCODE_SUCCESS* :: 
Operation was done successfully, which means Force escape flag is successfully provided to
BIT Processor.
*RETCODE_INVALID_HANDLE* :: 
+
--
This means the given handle for the current API function call, `handle`, was invalid. This return
code might be caused if

* `handle` is not the handle which has been obtained by VPU_DecOpen().
* `handle` is the handle of an instance which has been closed already, etc.
* bitstreamMode of DecOpenParam structure is not BS_MODE_INTERRUPT.
--
@endverbatim
*/
RetCode VPU_DecSetEscSeqInit(
    DecHandle   handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    int         escape      /**< [Input] A flag to enable or disable forced escape from SEQ_INIT */
    );
    
/**
 * @brief This function is used for registering frame buffers with the acquired
information from VPU_DecGetInitialInfo(). The frame buffers pointed to by
bufArray are managed internally within VPU. These include reference
frames, reconstructed frame, etc. Applications must not change the contents of
the array of frame buffers during the life time of the instance, and `num` must
not be less than `minFrameBufferCount` obtained by VPU_DecGetInitialInfo().
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
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST should call
VPU_DecGetOutputInfo() to proceed this function call.
*RETCODE_WRONG_CALL_SEQUENCE* :: 
This means the current API function call was invalid considering the allowed
sequences between API functions. A HOST might call this
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
decoded picture width, or is not a multiple of 8 in this case.
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
    FrameBuffer* bufArray,  /**< [Input] Allocated frame buffer address and information. If this parameter is set to 0, VPU allocates frame buffers. */
    int          num,       /**< [Input] A number of frame buffers. VPU can allocate frame buffers as many as this given value. */
    int          stride,    /**< [Input] A stride value of the given frame buffers  */
    int          height,    /**< [Input] Frame height   */ 
/**
@verbatim 
[Input] 
Map type of frame buffer The distance between a pixel in a row and the corresponding pixel in the next
row is called a stride. It comes from 64-bit access unit of AXI. 

The stride for luminance frame buffer should be at least equal or greater than the width of picture
and a multiple of 8. It means the least significant 3-bit of the 13-bit stride should be always 0. 
The stride for chrominance frame buffers is the half of the luminance stride. 
So in case Cb/Cr non-interleave (separate Cb/Cr) map is used, make sure the stride for luminance frame buffer
 should be a multiple of 16 so that the stride for chrominance frame buffer can become a multiple of 8. 
@endverbatim
*/
    int          mapType    
    );

/**
 * @brief This function is used for registering frame buffers with the acquired
information from VPU_DecGetInitialInfo(). 
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
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST should call
VPU_DecGetOutputInfo() to proceed this function call.
*RETCODE_WRONG_CALL_SEQUENCE* :: 
This means the current API function call was invalid considering the allowed
sequences between API functions. A HOST might call this
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
decoded picture width, or is not a multiple of 8 in this case.
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
    FrameBuffer* bufArray,          /**< [Input] Allocated frame buffer address and information. If this parameter is set to 0, VPU allocates frame buffers. */
    int          numOfDecFbs,       /**< [Input] Number of compressed frame buffer  */
    int          numOfDisplayFbs,   /**< [Input] Number of linear frame buffer when WTL is enabled. In WAVE4, this should be equal to or larger than framebufDelay of <<vpuapi_h_DecInitialInfo>> + 2. */
    int          stride,            /**< [Input] A stride value of the given frame buffers  */
    int          height,            /**< [Input] Frame height   */ 
    int          mapType            /**< [Input] A Map type for GDI inferface or FBC (Frame Buffer Compression) For detailed map options, please refer to the <<vpuapi_h_TiledMapType>>.   */ 
    );    

RetCode VPU_DecUpdateFrameBuffer(
    DecHandle     handle,
    FrameBuffer*  fbcFb,
    FrameBuffer*  linearFb,
    Int32         mvColIndex,
    Int32         picWidth,
    Int32         picHeight
    );

RetCode VPU_DecSetBitstreamBuffer(
    DecHandle       handle, 
    PhysicalAddress rdPtr, 
    PhysicalAddress wrPtr
    );

/**
* @brief   
This is a special function that enables HOST to allocate directly the frame buffer 
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
Fail to allocate a framebuffer due to lack of memory
*RETCODE_INVALID_PARAM* :: 
The given argument parameter, index, was invalid, which means it has improper values.
@endverbatim

 */
 RetCode VPU_DecAllocateFrameBuffer(
    DecHandle               handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    FrameBufferAllocInfo    info,       /**< [Input] Information required for frame bufer allocation */
    FrameBuffer*            frameBuffer /**< [Output] Data structure that holds information of allocated frame buffers */
    );
    
/**
* @brief 
@verbatim 
This function returns the frame buffer information that was allocated by VPU_DecRegisterFrameBuffer() function. 

It does not affect actual decoding and simply does obtain the information of frame buffer. 
This function is more helpful especially when frame buffers are automatically assigned by 
setting 0 to `bufArray` of VPU_DecRegisterFrameBuffer() and HOST wants to know about the allocated frame buffer.  
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
    FrameBuffer* frameBuf   /**< [output] Allocated frame buffer address and information.  */
    );

/**
* @brief
@verbatim
Before decoding bitstream, HOST application must feed the decoder with bitstream. To
do that, HOST application must know where to put bitstream and the maximum size.
Applications can get the information by calling this function. This way is more
efficient than providing arbitrary bitstream buffer to the decoder as far as
VPU is concerned.

The given size is the total sum of free space in ring buffer. So when
a HOST application downloads this given size of bitstream, Wrptr could meet the end of
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
* @brief
@verbatim
Before decoding bitstream, HOST application must feed the decoder with bitstream. To
do that, HOST application must know where to put bitstream and the maximum size.
Applications can get the information by calling this function. This way is more
efficient than providing arbitrary bitstream buffer to the decoder as far as
VPU is concerned.

The given size is the total sum of free space in ring buffer. So when
a HOST application downloads this given size of bitstream, Wrptr could meet the end of
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

/**
* @brief   Applications must let decoder know how much bitstream has been transferred to the
address obtained from VPU_DecGetBitstreamBuffer(). By just giving the size as
an argument, API automatically handles pointer wrap-around and updates the write
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
@* -1: It allows VPU to continue to decode without VPU_DecClose(), 
even after remaining stream has completely been decoded by VPU_DecUpdateBitstreamBuffer(handle, 0).  
This is especially useful when a sequence changes in the middle of decoding.
@* -2 : It is for handling exception cases like error stream or
failure of finding frame boundaries in interrupt mode. If such cases happen, VPU is unable to continue decoding.   
If -2 is set for `size`, VPU decodes until the current write pointer in the bitstream buffer and then force to end decoding. (WAVE only)

@endverbatim
*/
    int         size        
    );

/**
* @brief   
This function specifies the location of read pointer in bitstream buffer. 
It can also set a write pointer with same value of read pointer (addr) when updateWrPtr is not a zero value,  
which allows to flush up the bitstream buffer at once.
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
VPU_ DecSetRdPtr ().
@endverbatim
 */
 RetCode VPU_DecSetRdPtr(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    PhysicalAddress addr,       /**< [Input] Updated read or write pointer */
    int             updateWrPtr /**< [Input] A flag whether to move the write pointer to where the read pointer is located */
    );    
    
/**
* @brief   This function is provided to let HOST have a certain level of freedom for
re-configuring decoder operation after creating a decoder instance. Some options
which can be changed dynamically during decoding as the video sequence has been
included. Some command-specific return codes are also presented.
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
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST application should call
VPU_DecGetOutputInfo() to proceed this function call.
values.
@endverbatim
 * @note
@verbatim
The list of commands can be summarized as follows:

@* ENABLE_ROTATION
@* DIABLE_ROTATION
@* ENABLE_MIRRORING
@* DISABLE_MIRRORING
@* ENABLE_DERING
@* DISABLE_DERING
@* SET_MIRROR_DIRECTION
@* SET_ROTATION_ANGLE
@* SET_ROTATOR_OUTPUT
@* SET_ROTATOR_STRIDE
@* ENABLE_DEC_THUMBNAIL_MODE,
@* DEC_SET_SPS_RBSP
@* DEC_SET_PPS_RBSP
@* ENABLE_REP_USERDATA
@* DISABLE_REP_USERDATA
@* SET_ADDR_REP_USERDATA
@* SET_VIRT_ADDR_REP_USERDATA
@* SET_SIZE_REP_USERDATA
@* SET_USERDATA_REPORT_MODE
@* SET_SEC_AXI
@* SET_DRAM_CONFIG
@* GET_DRAM_CONFIG
@* ENABLE_REP_BUFSTAT
@* DISABLE_REP_BUFSTAT
@* ENABLE_REP_MBPARAM
@* DISABLE_REP_MBPARAM
@* ENABLE_REP_MBMV
@* DISABLE_REP_MBMV
@* SET_ADDR_REP_PICPARAM
@* SET_ADDR_REP_BUF_STATE
@* SET_ADDR_REP_MBMV_DATA
@* SET_CACHE_CONFIG
@* GET_TILEDMAP_CONFIG
@* SET_LOW_DELAY_CONFIG
@* DEC_GET_DISPLAY_OUTPUT_INFO 
@* SET_DECODE_FLUSH
@* DEC_SET_FRAME_DELAY
@* DEC_FREE_FRAME_BUFFER
@* DEC_GET_FIELD_PIC_TYPE
@* DEC_ENABLE_REORDER
@* DEC_DISABLE_REORDER
@* DEC_GET_FRAMEBUF_INFO
@* DEC_RESET_FRAMEBUF_INFO
@* DEC_SET_DISPLAY_FLAG 
@* DEC_GET_SEQ_INFO
@* ENABLE_LOGGING
@* DISABLE_LOGGING
@* DEC_SET_2ND_FIELD_INFO 
@* DEC_ENABLE_AVC_MC_INTERPOL
@* DEC_DISABLE_AVC_MC_INTERPOL

====== ENABLE_ROTATION
This command enables rotation of the post-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== DISABLE_ROTATION
This command disables rotation of the post-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== ENABLE_MIRRORING
This command enables mirroring of the post-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== DISABLE_MIRRORING
This command disables mirroring of the post-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== ENABLE_DERING
This command enables deringring filter of the post-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== DISABLE_DERING
This command disables deringing filter of the post-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== SET_MIRROR_DIRECTION
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

====== SET_ROTATION_ANGLE
This command sets counter-clockwise angle for post-rotation, and `parameter` is
interpreted as a pointer to the integer which represents rotation angle in
degrees. Rotation angle should be one of 0, 90, 180, and 270.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given rotation angle is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given rotation
angle is invalid.

====== SET_ROTATOR_OUTPUT
This command sets rotator output buffer address, and `parameter` is interpreted as
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

====== SET_ROTATOR_STRIDE
This command sets the stride size of the frame buffer containing rotated output,
and `parameter` is interpreted as the value of stride of the rotated output.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given rotation angle is valid.
* RETCODE_INVALID_STRIDE:
The given argument parameter, `parameter`, was invalid, which means given value of
stride is invalid. The value of stride must be greater than 0 and a multiple of 8.

====== ENABLE_DEC_THUMBNAIL_MODE
This command decodes only an I-frame of picture from bitstream for using it as a thumbnail. 
It requires as little as size of frame buffer since I-picture does not need any reference picture.
If HOST issues this command and sets one frame buffer address to FrameBuffer array in VPU_DecRegisterFrameBuffer(),
only the frame buffer is used.
And please make sure that the number of frame buffer `num` should be registered as minFrameBufferCount.
    
====== DEC_SET_SPS_RBSP
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

====== DEC_SET_PPS_RBSP
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

====== ENABLE_REP_USERDATA
This command enables user data report. This command ignores `parameter`.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means enabling user data report is done
successfully.
* RETCODE_USERDATA_BUF_NOT_SET:
This means user data buffer address and size have not set yet.

====== DISABLE_REP_USERDATA
This command disables user data report. This command ignores `parameter` and returns
RETCODE_SUCCESS.

====== SET_ADDR_REP_USERDATA
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

====== SET_VIRT_ADDR_REP_USERDATA
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

====== SET_SIZE_REP_USERDATA
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

NOTE: This command is available soon.

The user data size 0 - 15 is used to make offset from userDataBuf Base + 8x17.
It specifies byte size of user data 0 to 15 excluding 0 padding byte,
which exists between user data. So HOST reads 1 user data from
userDataBuf Base + 8x17 + 0 User Data Size + 0 Padding.
Size of 0 padding is (8 - (User Data Size % 8))%8.

====== SET_USERDATA_REPORT_MODE
TBD

====== SET_SEC_AXI
This command sets the secondary channel of AXI for saving memory bandwidth to
dedicated memory. The argument `parameter` is interpreted as a pointer to SecAxiUse which
represents an enable flag and physical address which is related with the secondary
channel for BIT processor, IP/AC-DC predictor, de-blocking filter, overlap
filter respectively.

This command has one of the following return codes::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value for setting secondary
AXI is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given value is
invalid.

====== SET_DRAM_CONFIG
TBD

====== GET_DRAM_CONFIG
TBD

====== ENABLE_REP_BUFSTAT
This command enables frame buffer status report. This command ignores `parameter` and
returns RETCODE_SUCCESS.

If this option is enabled, frame buffer status is reported.
Each frame buffers can be used for display, reference or not used. Decoder
reports the status of each frame buffer by using 4 bits [3:0]

@* [3] Not used
@* [2] USE_DIS
@* [1] USE_REF
@* [0] Not used

For example, if the value of frame buffer status is 6, then the frame buffer is
used for reference and display. If 4, the frame buffer is used for display and
is not used for reference.

In H.264/AVC, bit field definition is as bellows:

@* [3] Not used
@* [2] USE_DIS
@* [1] USE_OUT
@* [0] USE_REF

If USE_OUT is 1, it means that the Frame is in DPB buffer.

====== DISABLE_REP_BUFSTAT
This command disables frame buffer status report. This command ignores `parameter`
and returns RETCODE_SUCCESS.

====== ENABLE_REP_MBPARAM
This command enables MB Parameter report. This command ignores `parameter` and
returns RETCODE_SUCCESS.

If this option is enabled, error flag, Slice Boundary and QP are reported using
8 bits.

@* [7] : Error Map. If error is detected in macroblock decoding, this bit field is set to 1.
@* [6] : Slice Boundary. Whenever new slice header is decoded, this bit field is toggled.
@* [5:0] : An macroblock QP value

====== DISABLE_REP_MBPARAM
This command disables MB Parameter report. This command ignores `parameter` and
returns RETCODE_SUCCESS.

====== ENABLE_REP_MV
This command enables MV report. This command ignores `parameter` and returns
RETCODE_SUCCESS.

If this option is enabled, decoder reports 1 motion vector for P picture and 2
motion vectors for B picture.

====== DISABLE_REP_MV
This command disables MV report. This command ignores `parameter` and returns
RETCODE_SUCCESS.

====== SET_ADDR_REP_PICPARAM
This command sets the address of picture parameter base. `parameter` is interpreted
as a pointer to a address.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value of address is valid and
setting is done successfully.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given value of
address is invalid. The value of address must be greater than 0 and a multiple
of 8.

To report frame buffer status, MB parameter and Motion vector, VPU reads the base
addresses of external memory, which are specified by PicParaBaseAddr.

[[dec_picparam_address_struct]]
.Decoder Picture parameter base address structure
ifdef::pdf[]
image::../figure/dec_picparam_address_struct.svg["Decoder Picture parameter base address structure", width=300]
endif::pdf[]
ifdef::html[]
image::../figure/dec_picparam_address_struct.png[Decoder Picture parameter base address structure]
endif::html[]

When `picUserDataEnable`, `mvReportEnable`, `mbParamEnable` or `frmBufStaEnable` in
CMD_DEC_PIC_OPTION register are enabled, HOST application should specify buffer addresses in
<<dec_picparam_address_struct>>.
VPU reports each data and fills info, Data Size and MV num fields to
 these buffer addresses of external memory. For VPU to report data
properly, HOST application needs to specify these 3 buffer addresses preserving 8 byte alignment
and buffer sizes need to be multiples of 256.

====== SET_ADDR_REP_BUF_STATE

This command sets the buffer address of frame buffer status. The `parameter` is interpreted as a pointer to the address.

This command has one of the following return codes::

* RETCODE_SUCCESS: Operation was done successfully, which means given value of address
is valid and setting is done successfully.
* RETCODE_INVALID_PARAM: The given argument parameter, `parameter`, was invalid,
which means given value of address is invalid. The value of address must be greater than 0
and a multiple of 8.

====== SET_ADDR_REP_MBMV_DATA

This command sets the buffer address of motion vector information reporting array. The `parameter` is interpreted as a pointer to the address.

This command has one of the following return codes.::

* RETCODE_SUCCESS: Operation was done successfully, which means given value of address
is valid and setting is done successfully.
* RETCODE_INVALID_PARAM: The given argument parameter, `parameter`, was invalid,
which means given value of address is invalid. The value of address must be greater than 0
and a multiple of 8.

The motion vector information reporting array consists of below fields for each macroblock.
The array element has 32 bit data per a macroblock.

.Description of SET_ADDR_REP_MBMV_DATA
[separator="|", frame="all", grid="all"]
`15`20`65_80
Bit range  | Field name      | Description
________________________________________________________________________________
31         | Inter MB        | A flag to indicate whether the macroblock is inter macroblock or not
29:16      | Mv X            | Signed motion vector value for X axis
13:00      | Mv Y            | Signed motion vector value for Y axis
________________________________________________________________________________

====== SET_CACHE_CONFIG

This command sets the configuration of cache. The `parameter` is interpreted as a pointer to MaverickCacheConfig.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value is valid and setting is done successfully.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid. The value of address must be not zero.

====== GET_TILEDMAP_CONFIG
This command gets tiled map configuration according to `TiledMapConfig` structure.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value is valid and setting is done successfully.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means it has a null pointer,
 or given values for some member variables are improper values.

====== SET_LOW_DELAY_CONFIG
This command sets the low delay decoding options which enable low delay decoding and indicate the number of MB row.
The argument `parameter` is interpreted as a pointer to LowDelayInfo which represents an enable flag and the number of MB row.
If low delay decoding is enabled, VPU sends an interrupt and indexFrameDisplay to HOST when the number of MB row decoding is done.
If the interrupt is issued, HOST should clear the interrupt and read indexFrameDisplay from the RET_DEC_PIC_FRAME_IDX register in order to display.

====== DEC_GET_DISPLAY_OUTPUT_INFO
HOST can get decoder output information according to display index in DecOutputInfo structure. 
HOST can set display index using member variable `indexFrameDisplay`.
This command returns RETCODE_SUCCESS.

* Example code
.........................................................................
DecOutputInfo decOutputInfo;
decOutputInfo. indexFrameDisplay = disp_index;
VPU_DecGiveCommand(handle, DEC_GET_DISPLAY_OUTPUT_INFO, & decOutputInfo);
.........................................................................

====== SET_DECODE_FLUSH
This command is used to change bitstream buffer mode from Interrupt mode to Picture End mode.
When HOST receives an interrupt about bistream buffer empty in Interrupt Mode, but there is no more bitstream for the current frame in bitstream buffer,
this command completes decoding only with remaining bitstream.

This command returns RETCODE_SUCCESS.

====== DEC_SET_FRAME_DELAY 
HOST can set the number of frame to be delayed before display (H.264/AVC only) by using this command. 
This command is useful when display frame buffer delay is supposed to happen for buffering decoded picture reorder and 
HOST is sure of that. 
Unless this command is executed, VPU has display frame buffer delay as frameBufDelay value of DecInitialInfo structure. 

====== DEC_FREE_FRAME_BUFFER
HOST can free all the frame buffers allocated by VPUAPI.
This command is useful when VPU detects sequence change. 
For example, if HOST knows resolution change while decoding through `sequenceChanged` variable of DecOutputInfo structure, 
HOST should change the size of frame buffer accordingly.
This command is used to release the frame buffers allocated for the previous sequence.
Then VPU_DecGetInitialInfo() and VPU_DecIsseuSeqInit() are called before frame buffer allocation for a new sequence. 

====== DEC_SET_DISPLAY_FLAG
Applications can set a display flag for each frame buffer by calling this function after creating
decoder instance. If a certain display flag of frame buffer is set, the frame buffer cannot be used in the
decoding process. Applications can control displaying a buffer with this command
to prevent VPU from using buffer in every decoding process.

This command is the opposite of what VPU_DecClrDispFlag() does.
All of buffer flags are initialized with 0x00000000 which means being able to decode.
Unless it is called, VPU starts decoding with available frame buffer that has been cleared 
in round robin order. 

====== DEC_GET_SEQ_INFO
This command returns the information of the current sequence in the form of <<vpuapi_h_DecInitialInfo>> structure. 
HOST can get this information with more accuracy after VPU_DecIssueSeqInit() or VPU_DecGetOutputInfo() is called.

====== ENABLE_LOGGING
HOST can activate message logging once VPU_DecOpen() or VPU_EncOpen() is called.

====== DISABLE_LOGGING
HOST can deactivate message logging which is off as default.

====== DEC_ENABLE_AVC_MC_INTERPOL
This is option for enable fast MC interpolation. this command is only used for H.264/AVC decoder.

====== DEC_DISABLE_AVC_MC_INTERPOL
This is option for disable fast MC interpolation. this command is only used for H.264/AVC decoder.

====== DEC_SET_2ND_FIELD_INFO 
This command sets bistream buffer information for a second field after a first field has been decoded. 
In case of H.264/AVC, MPEG2, or VC1 decoder, they issue an INT_BIT_DEC_FIELD interrupt after first field decoding.
This command then can give VPU the address and size of bitstream buffer for a second field. 
This commands is used when first field and second field are in separate bistreams - not together in a bitstream. 
If HOST gives this command and clears the INT_BIT_DEC_FIELD interrupt,
VPU starts decoding from the given base address until the size of bitstream buffer.

====== DEC_GET_FIELD_PIC_TYPE 
This command gets a field picture type of decoded picture after INT_BIT_DEC_FIELD interrupt is issued.

====== DEC_ENABLE_REORDER
HOST can enable display buffer reordering when decoding H.264 streams. In H.264 case
output decoded picture may be re-ordered if pic_order_cnt_type is 0 or 1. In that case, decoder
must delay output display for re-ordering but some applications (ex. video telephony) do not
want such display delay. 

====== DEC_DISABLE_REORDER
HOST can disable output display buffer reorder-ing. Then BIT processor does not re-order output buffer when pic_order_cnt_type is 0 or 1. If
In H.264/AVC case. pic_order_cnt_type is 2 or the other standard case, this flag is ignored because output display
buffer reordering is not allowed.

====== DEC_GET_FRAMEBUF_INFO
This command gives HOST the information of framebuffer in the form of DecGetFramebufInfo structure. 

====== DEC_RESET_FRAMEBUF_INFO
This command resets the information of framebuffer. 
Unlike DEC_FREE_FRAME_BUFFER, it does not release the assigned memory itself. 
This command is used for sequence change along with DEC_GET_FRAMEBUF_INFO. 

====== ENABLE_REP_CUDATA
This command enables to report CU data.

====== DISABLE_REP_CUDATA
This command disables to report CU data. 

====== SET_ADDR_REP_CUDATA
This command sets the address of buffer where CU data is written.

====== SET_SIZE_REP_CUDATA
This command sets the size of buffer where CU data is written. 

====== DEC_SET_WTL_FRAME_FORMAT
This command sets FrameBufferFormat for WTL.

====== DEC_SET_AVC_ERROR_CONCEAL_MODE
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

====== DEC_GET_SCALER_INFO
This command returns setting information to downscale an image such as enable, width, and height.

====== DEC_SET_SCALER_INFO
This command sets information to downscale an image such as enable, width, and height.

====== DEC_SET_TARGET_TEMPORAL_ID 
This command decodes only a frame whose temporal id is equal to or less than the given target temporal id. 

NOTE: It is only for H.265/HEVC decoder.

====== DEC_SET_BWB_CUR_FRAME_IDX
This command specifies the index of linear frame buffer which needs to be changed to due to change of inter-frame resolution while decoding.

NOTE: It is only for VP9 decoder.

====== DEC_SET_FBC_CUR_FRAME_IDX
This command specifies the index of FBC frame buffer which needs to be changed to due to change of inter-frame resolution while decoding.

NOTE: It is only for VP9 decoder.

====== DEC_SET_INTER_RES_INFO_ON
This command informs inter-frame resolution has been changed while decoding. After this command issued, 
VPU reallocates one frame buffer for the change. 

NOTE: It is only for VP9 decoder.

====== DEC_SET_INTER_RES_INFO_OFF
This command releases the flag informing inter-frame resolution change.
It should be issued after reallocation of one frame buffer is completed. 

NOTE: It is only for VP9 decoder.

====== DEC_FREE_FBC_TABLE_BUFFER 
This command frees one FBC table to deal with inter-frame resolution change. 

NOTE: It is only for VP9 decoder.

====== DEC_FREE_MV_BUFFER
This command frees one MV buffer to deal with inter-frame resolution change. 

NOTE: It is only for VP9 decoder.

====== DEC_ALLOC_FBC_Y_TABLE_BUFFER
This command allocates one FBC luma table to deal with inter-frame resolution change. 

NOTE: It is only for VP9 decoder.

====== DEC_ALLOC_FBC_C_TABLE_BUFFER
This command allocates one FBC chroma table to deal with inter-frame resolution change. 

NOTE: It is only for VP9 decoder.

====== DEC_ALLOC_MV_BUFFER
This command allocates one MV buffer to deal with inter-frame resolution change. 

NOTE: It is only for VP9 decoder.

@endverbatim
*/
 RetCode VPU_DecGiveCommand(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    CodecCommand    cmd,        /**< [Input] A variable specifying the given command of CodecComand type */
    void*           parameter   /**< [Input/Output] A pointer to command-specific data structure which describes picture I/O parameters for the current decoder instance */
    );    

/**
* @brief   
This function sets the secondary channel of AXI for saving memory bandwidth to dedicated
memory. 
It works same as SET_SEC_AXI command which is only able to be called before VPU_DecRegisterFramebuffer().
This function can be called before VPU_DecStartOneFrame() to change information about the secondary channel of AXI.
* @return
@verbatim
*RETCODE_SUCCESS* :: 
Operation was done successfully, which means given value for setting secondary AXI is valid.
*RETCODE_FRAME_NOT_COMPLETE* :: 
This means frame decoding operation was not completed yet, so the given API function call
cannot be allowed.
*RETCODE_INSUFFICIENT_RESOURCE* :: 
This means that VPU cannot allocate memory due to lack of memory.
*RETCODE_VPU_RESPONSE_TIMEOUT* :: 
This means that VPU response time is too long, time out.
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
VPU_ DecSetRdPtr ().
@endverbatim
 */
 RetCode VPU_DecConfigSecondAxi(
    DecHandle handle,   /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    int stride,         /**< [Input] Width of framebuffer */
    int height          /**< [Input] Height of framebuffer */
    );
    
/**
 * @brief   This function starts decoding one frame. Returning from this function does not
mean the completion of decoding one frame, and it is just that decoding one
frame was initiated. Every call of this function should be matched with VPU_DecGetOutputInfo()
with the same handle. Without calling a pair of these funtions, HOST cannot call any other
API functions except for VPU_IsBusy(), VPU_DecGetBitstreamBuffer(), and
VPU_DecUpdateBitstreamBuffer().
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
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling VPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, HOST should call
VPU_DecGetOutputInfo() to proceed this function call.
*RETCODE_WRONG_CALL_SEQUENCE* :: 
This means the current API function call was invalid considering the allowed
sequences between API functions. A HOST might call this
function before successfully calling VPU_DecRegisterFrameBuffer(). This function
should be called after calling VPU_ DecRegisterFrameBuffer() successfully.
@endverbatim
 */
RetCode VPU_DecStartOneFrame(
    DecHandle handle,   /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    DecParam *param     /**< [Input] A pointer to a DecParam type structure which describes picture decoding parameters for the given decoder instance */
    );

/**
 * @brief   VPU returns the result of decoding which 
includes information on decoded picture, syntax value, frame buffer, other report values, and etc. 
HOST should call this function after frame decoding is finished and
before starting the further processing.
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
instance already closed, or `handle` is a handle to a decoder instance. Also,
this value is returned when VPU_DecStartOneFrame() is matched with
VPU_DecGetOutputInfo() with different handles.
*RETCODE_INVALID_PARAM* :: 
The given argument parameter, pInfo, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
@endverbatim
 */
 RetCode VPU_DecGetOutputInfo(
    DecHandle       handle,     /**< [Input] A decoder handle obtained from VPU_DecOpen() */
    DecOutputInfo*  info        /**< [Output] A pointer to a DecOutputInfo type structure which describes picture decoding results for the current decoder instance. */
    );

/**
* @brief   
@verbatim
This function flushes all of the decoded framebuffer contexts that remain in decoder firmware. 
It is used to start seamless decoding operation without randome access to the buffer or calling VPU_DecClose(). 

NOTE: In WAVE4, this function returns all of the decoded framebuffer contexts that remain. 
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
    DecOutputInfo*  pRemainingInfo, /**<  [Output] All of the decoded framebuffer contexts are stored in display order as array of DecOutputInfo. 
                                            If this is NULL, the remaining information is not returned.  */
    Uint32*         pRetNum         /**<  [Output] The number of the decoded frame buffer contexts. It this is null, the information is not returned.  */
    );

/**
* @brief   This function clears a display flag of frame buffer. 
If display flag of frame buffer is cleared, the
frame buffer can be reused in the decoding process. With VPU_DecClrDispFlag(), HOST application can control
display of frame buffer marked with the display index which is given by VPU. 
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
* @brief    In order to start a new encoder operation, HOST application must open a new instance
for this encoder operation. By calling this function, HOST application can get a
handle specifying a new encoder instance. Because VPU supports multiple
instances of codec operations, HOST application needs this kind of handles for the
all codec instances now on running. Once a HOST application gets a handle, the HOST application
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
    EncHandle*      handle,     /**< [Output] A pointer to a EncHandle type variable which specifies each instance for HOST application. If no instance is available, null handle is returned. */
    EncOpenParam*   encOpParam  /**< [Input] A pointer to a EncOpenParam type structure which describes required parameters for creating a new encoder instance. */
    );

/**
* @brief    When HOST application finished encoding operations and wanted to release this
instance for other processing, the HOST application should close this instance by calling
this function. After completion of this function call, the instance referred to
by pHandle gets free. Once HOST application closes an instance, the HOST application
cannot call any further encoder-specific function with the current handle before
re-opening a new instance with the same handle.
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
* @brief    Before starting encoder operation, HOST application must allocate frame buffers
according to the information obtained from this function. This function returns
the required parameters for VPU_EncRegisterFrameBuffer(), which follows right after
this function call.
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
    EncInitialInfo* encInitInfo /**< [Output] Minimum number of frame buffer for this encoder instance  */
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
for the luminance frame buffer becomes a multiple of 8.
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
the value of  minFrameBufferCount obtained from VPU_EncGetInitialInfo().

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
    FrameBuffer*    bufArray,   /**< [Input] Allocated frame buffer address and information. If this parameter is set to -1, VPU allocates frame buffers. */
    int             num,        /**< [Input] A number of frame buffers. VPU can allocate frame buffers as many as this given value. */
    int             stride,     /**< [Input] A stride value of the given frame buffers */
    int             height,     /**< [Input] Frame height */
    int             mapType     /**< [Input] Map type of frame buffer */
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
Fail to allocate a framebuffer due to lack of memory

*RETCODE_INVALID_PARAM* ::
The given argument parameter, index, was invalid, which means it has improper values
@endverbatim
*/
RetCode VPU_EncAllocateFrameBuffer(
    EncHandle handle,           /**< [Input] An encoder handle obtained from VPU_EncOpen()  */
    FrameBufferAllocInfo info,  /**< [Input] Information required for frame bufer allocation  */
    FrameBuffer *frameBuffer    /**< [Output]  Data structure that holds information of allocated frame buffers  */
    );

/**
* @brief    
@verbatim
This function gets the frame buffer information that was allocated by VPU_EncRegisterFrameBuffer() function. 

It does not affect actual encoding and simply does obtain the information of frame buffer. 
This function is more helpful especially when frame buffers are automatically assigned by 
setting 0 to `bufArray` of VPU_EncRegisterFrameBuffer() and HOST wants to know about the allocated frame buffer.  
@endverbatim
* @return
@verbatim
*RETCODE_SUCCESS* ::
Operation was done successfully, which means registering frame buffer
information was done successfully.

*RETCODE_INVALID_HANDLE* ::
This means the given handle for the current API function call, `handle`, was invalid.
This return code might be caused if
+
--
* `handle` is not a handle which has been obtained by VPU_DecOpen().
* `handle` is a handle of an instance which has been closed already, etc.
--

*RETCODE_INVALID_PARAM* ::
The given argument parameter, frameIdx, was invalid, which means frameIdx is larger than allocated framebuffer. 
@endverbatim
*/
RetCode VPU_EncGetFrameBuffer(
    EncHandle       handle,     /**< [Input] An encoder handle obtained from VPU_EncOpen()   */
    int             frameIdx,   /**< [Input] An index of frame buffer  */
    FrameBuffer*    frameBuf    /**< [output] Allocated frame buffer address and information   */
    );

/**
* @brief    This function is provided to let HOST application have a certain level of freedom for
re-configuring encoder operation after creating an encoder instance. Some
options which can be changed dynamically during encoding as the video sequence has
been included. Some command-specific return codes are also presented.
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
@endverbatim
* @note
@verbatim
The list of commands can be summarized as follows:

@* ENABLE_ROTATION
@* DIABLE_ROTATION
@* ENABLE_MIRRORING
@* DISABLE_MIRRORING
@* SET_MIRROR_DIRECTION
@* SET_ROTATION_ANGLE
@* ENC_ADD_PPS
@* ENC_SET_ACTIVE_PPS
@* ENC_GET_ACTIVE_PPS
@* ENC_PUT_VIDEO_HEADER
@* ENC_PUT_MP4_HEADER
@* ENC_PUT_AVC_HEADER
@* ENC_GET_VIDEO_HEADER
@* ENC_SET_INTRA_MB_REFRESH_NUMBER
@* ENC_ENABLE_HEC
@* ENC_DISABLE_HEC
@* ENC_SET_SLICE_INFO
@* ENC_SET_GOP_NUMBER
@* ENC_SET_INTRA_QP
@* ENC_SET_BITRATE
@* ENC_SET_FRAMERATE
@* ENC_SET_SEARCHRAM_PARAM
@* ENC_SET_REPORT_MBINFO
@* ENC_SET_REPORT_MVINFO
@* ENC_SET_REPORT_SLICEINFO
@* ENC_SET_PIC_PARA_ADDR
@* SET_SEC_AXI
@* ENABLE_LOGGING
@* DISABLE_LOGGING
@* ENC_CONFIG_SUB_FRAME_SYNC
@* ENC_SET_SUB_FRAME_SYNC

====== ENABLE_ROTATION
This command enables rotation of the pre-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== DIABLE_ROTATION
This command disables rotation of the pre-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== ENABLE_MIRRORING
This command enables mirroring of the pre-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== DISABLE_MIRRORING
This command disables mirroring of the pre-rotator.
In this case, `parameter` is ignored. This command returns RETCODE_SUCCESS.

====== SET_MIRROR_DIRECTION
This command sets mirror direction of the pre-rotator, and `parameter` is
interpreted as a pointer to MirrorDirection.
The `parameter` should be one of MIRDIR_NONE, MIRDIR_VER, MIRDIR_HOR, and MIRDIR_HOR_VER.

@* MIRDIR_NONE: No mirroring
@* MIRDIR_VER: Vertical mirroring
@* MIRDIR_HOR: Horizontal mirroring
@* MIRDIR_HOR_VER: Both directions

This command has one of the following return codes::

* RETCODE_SUCCESS:
Operation was done successfully, which means given mirroring direction is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given mirroring
direction is invalid.

====== SET_ROTATION_ANGLE
This command sets counter-clockwise angle for post-rotation, and `parameter` is
interpreted as a pointer to the integer which represents
rotation angle in degrees. Rotation angle should be one of 0, 90, 180, and 270.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given rotation angle is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given rotation
angle is invalid.

NOTE: Rotation angle could not be changed after sequence initialization, because
it might cause problems in handling frame buffers.

====== ENC_ADD_PPS
This command adds PPS header syntax of AVC/H.264 bitstream

This command has one of the following return codes.::

* RETCODE_SUCCESS: 
Operation was done successfully, which means the requested header
syntax was successfully inserted.
* RETCODE_INVALID_COMMAND: 
This means the given argument cmd was invalid
which means the given cmd was undefined, or not allowed in the current instance. In this
case, the current instance might not be an H.264/AVC encoder instance.
* RETCODE_INVALID_PARAM: 
The given argument parameter `parameter` was invalid, which means
it has a null pointer, or given values for some member variables are improper values.

====== ENC_SET_ACTIVE_PPS
This command sets active PPS header syntax of AVC/H.264 bitstream, and `parameter` is interpreted ]
as an active PPS index.

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
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not recieved any response from VPU and has timed out.

====== ENC_GET_ACTIVE_PPS
This command gets active PPS header syntax of AVC/H.264 bitstream, and `parameter` is interpreted as an active PPS index.

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

====== ENC_PUT_VIDEO_HEADER
This command inserts an MPEG4 header syntax or SPS or PPS to the AVC/H.264 bitstream to the bitstream
during encoding. The argument `parameter` is interpreted as a pointer to EncHeaderParam structure holding

* buf is a physical address pointing the generated stream location
* size is the size of generated stream in bytes
* headerType is a type of header that HOST application wants to generate and have values as
VOL_HEADER, VOS_HEADER,, VO_HEADER, SPS_RBSP or PPS_RBSP.

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

====== ENC_PUT_MP4_HEADER
This command inserts an MPEG4 header syntax to the bitstream during encoding.
The argument `parameter` is interpreted as a pointer to EncHeaderParam
structure holding

* `buf` is a physical address pointing the generated stream
location
* `size` is the size of generated stream in bytes
* `headerType` is a type of header that HOST application wants to generate and have values as VOL_HEADER,
VOS_HEADER, or VO_HEADER.

This command has one of the following return codes.::
+
--
* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given cmd was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an MPEG4 encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not recieved any response from VPU and has timed out.
--

====== ENC_PUT_AVC_HEADER
This command inserts an SPS or PPS to the AVC/H.264 bitstream during encoding.

The argument `parameter` is interpreted as a pointer to EncHeaderParam
structure holding

* buf is a physical address pointing the generated stream location
* size is the size of generated stream in bytes
* headerType is type of header that HOST application wants to generate and have values as SPS_RBSP or
PPS_RBSP.

This command has one of the following return codes.::
+
--
* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument `cmd` was invalid which means the given `cmd` was
undefined, or not allowed in the current instance. In this case, the current
instance might not be an AVC/H.264 encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not recieved any response from VPU and has timed out.
--

====== ENC_GET_VIDEO_HEADER
This command gets a SPS to the AVC/H.264 bitstream to the bitstream during encoding.
Because VPU does not generate bitstream but HOST application generate bitstream in this command,
HOST application has to set pBuf value to access bitstream buffer.
The argument `parameter` is interpreted as a pointer to EncHeaderParam structure holding

* buf is a physical address pointing the generated stream location
* pBuf is a virtual address pointing the generated stream location
* size is the size of generated stream in bytes
* headerType is a type of header that HOST application wants to generate and have values as SPS_RBSP.

This command has one of the following return codes.::
+
--
* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header
syntax was successfully inserted.
* RETCODE_INVALID_COMMAND:
This means the given argument cmd was invalid
which means the given cmd was undefined, or not allowed in the current instance. In this
case, the current instance might not be an AVC/H.264 encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or headerType
was invalid, which means it has a null pointer, or given values for some member variables
are improper values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not recieved any response from VPU and has timed out.
--

====== ENC_SET_INTRA_MB_REFRESH_NUMBER
This command sets intra MB refresh number of header syntax.
The argument `parameter` is interpreted as a pointer to integer which
represents an intra refresh number. It should be between 0 and
macroblock number of encoded picture. 

This command returns the following code.::

* RETCODE_SUCCESS:
Operation was done successfully, which means the requested header syntax was
successfully inserted.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not recieved any response from VPU and has timed out.

====== ENC_ENABLE_HEC
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
Operation has not recieved any response from VPU and has timed out.

====== ENC_DISABLE_HEC
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
Operation has not recieved any response from VPU and has timed out.

====== ENC_SET_SLICE_INFO
This command sets slice inforamtion of header syntax.
The argument `parameter` is interpreted as a pointer to EncSliceMode
structure holding

* sliceModeuf is a mode which means enabling multi slice structure
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
Operation has not recieved any response from VPU and has timed out.
--

====== ENC_SET_GOP_NUMBER
This command sets GOP number of header syntax.
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
Operation has not recieved any response from VPU and has timed out.

====== ENC_SET_INTRA_QP
This command sets intra QP value of header syntax.
The argument `parameter` is interpreted as a pointer to the integer which
represents a Constant I frame QP. The Constant I frame QP should be between 1 and 31
for MPEG4 and between 0 and 51 for AVC/H.264.

This command has one of the following return codes::

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
Operation has not recieved any response from VPU and has timed out.

====== ENC_SET_BITRATE
This command sets bitrate inforamtion of header syntax.
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
Operation has not recieved any response from VPU and has timed out.

====== ENC_SET_FRAMERATE
This command sets frame rate of header syntax.
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
Operation has not recieved any response from VPU and has timed out.

====== ENC_SET_SEARCHRAM_PARAM
This command sets the parameter of search sram used in encoder. 
The `parameter` is interpreted as a pointer to SearchRamParam.

This command has one of the following return codes.::

* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `SearchRamSize` was invalid, 
which means it has a null pointer, or given values for some member variables are improper values.

====== ENC_SET_REPORT_MBINFO
This command sets MB data report structure. The argument `parameter` is interpreted as a pointer
to address and returns RETCODE_SUCCESS.

If this option is set, the slice boundary and QP are reported using 1 bytes.

* [7] : Reserved
* [6] : Slice Boundary. Whenever a new slice header is decoded, this bit field is
toggled.
* [5:0] : An macroblock QP value

====== ENC_SET_REPORT_MVINFO
This command sets motion vector report structure. The argument `parameter` is interpreted as a
pointer to address and returns RETCODE_SUCCESS.

If this option is set, the motion vector information are reported using 4 bytes.

* [31] : Intra Flag (1: intra, 0: inter)
* [30:16] : X value of motion vector
* [16:0] : Y value of motion vector

====== ENC_SET_REPORT_SLICEINFO
This command sets slice information report structure.
The argument `parameter` is interpreted as a pointer to address and returns RETCODE_SUCCESS.

If this option is set, the slice information are reported using 8 bytes.

* [63:48] : Reserved.
* [47:32] :The last macroblock indeed of a slice (zero based-index)
* [31:0] : Total of bits used for encoding a slice.

====== ENC_SET_PIC_PARA_ADDR
This command sets the address of picture parameter base. The argument `parameter` is interpreted
as a pointer to `address` and returns RETCODE_SUCCESS.

To report MB data, Motion Vector, and Slice information, VPU should reads the base
address of external memory, which all are specified by picParaBaseAddr.

[[enc_pic_param_baseaddr_struct]]
.Encoder Picture parameter base address structure
ifdef::pdf[]
image::../figure/enc_pic_param_baseaddr_struct.svg["Encoder Picture parameter base address structure", width=300]
endif::pdf[]
ifdef::html[]
image::../figure/enc_pic_param_baseaddr_struct.png[Encoder Picture parameter base address structure]
endif::html[]

When `enReportMBInfo`, `enReportMVInfo`, and `enReportSliceinfo` in CMD_ENC_PIC_OPTION
register are enabled, HOST application should specify the buffer addresses in <<enc_pic_param_baseaddr_struct>>.
VPU reports each data and fills info, Data Size and other fields
to these buffer addresses of external memory. For VPU to report data properly,
HOST application needs to specify these 3 buffer addresses preserving 8 byte alignment and
buffer sizes need to be multiples of 256.

====== SET_SEC_AXI
This command sets the secondary channel of AXI for saving memory bandwidth to
dedicated memory. The argument `parameter` is interpreted as a pointer to SecAxiUse which
represents an enable flag and physical address which is related with the secondary
channel for BIT processor, IP/AC-DC predictor, de-blocking filter, overlap
filter respectively.

This command has one of the following return codes.::

* RETCODE_SUCCESS:
Operation was done successfully, which means given value for setting secondary
AXI is valid.
* RETCODE_INVALID_PARAM:
The given argument parameter, `parameter`, was invalid, which means given value is
invalid.

====== ENABLE_LOGGING
HOST can activate message logging once VPU_DecOpen() or VPU_EncOpen() is called.

====== DISABLE_LOGGING
HOST can deactivate message logging which is off as default.

====== ENC_CONFIG_SUB_FRAME_SYNC
This command sets the configuration of sub frame sync mode. 
The `parameter` is interpreted as a pointer to EncSubFrameSyncConfig. This command returns RETCODE_SUCCESS.

====== ENC_SET_SUB_FRAME_SYNC
This command sets the status of sub frame sync signal. 
The `parameter` is interpreted as a pointer to EncSubFrameSyncState. This command returns RETCODE_SUCCESS.

====== ENC_SET_PARA_CHANGE
This command changes encoding parameter(s) during the encoding operation in H.265/AVC encoder 
The argument `parameter` is interpreted as a pointer to <<vpuapi_h_EncChangeParam>> structure holding 

* changeParaMode : OPT_COMMON (only valid currently)
* enable_option : Set an enum value that is associated with parameters to change, <<vpuapi_h_ChangeCommonParam>> (multiple option allowed).

For instance, if bitrate and framerate need to be changed in the middle of encoding, that requires some setting like below.

 EncChangeParam changeParam;
 changeParam.changeParaMode    = OPT_COMMON;
 changeParam.enable_option     = ENC_RC_TARGET_RATE_CHANGE | ENC_FRAME_RATE_CHANGE;
            
 changeParam.bitrate           = 14213000;
 changeParam.frameRate         = 15;
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
instance might not be an AVC/H.264 encoder instance.
* RETCODE_INVALID_PARAM:
The given argument parameter `parameter` or `headerType` was invalid, which means it
has a null pointer, or given values for some member variables are improper
values.
* RETCODE_VPU_RESPONSE_TIMEOUT:
Operation has not recieved any response from VPU and has timed out.
--

====== ENC_SET_PROFILE

This command encodes videos into H.264/AVC stream with HOST application-given profile in header syntax. (CODA9 only)

====== ENC_SET_SEI_NAL_DATA
This command sets variables in HevcSEIDataEnc structure for SEI encoding.

====== SET_ENC_WTL_INFO
This command sets information of frame buffer such as Y/Cb/Cr addr, cbcrinterleave, nv21, and number of linear framebuffer for WTL-enabled encoder. 

====== SET_ADDR_ENC_RC_REPORT_BASE
This command sets the base address of report buffer. 
It should be specified when any of these bitInfoReportEnable, frameDistortionReportEnable,
 or qpHistogramReportEnable in EncParam is on. 
In total 144bytes of data are reported to the report buffer from w4EncReportBaseAddr in the following order. 
If a certain ReportEnable flag is diabled i.e frameDistortionReportEnable, dummy data are written into the frameDistortion region of the buffer. 

image::../figure/rc_report.png["Report Buffer", width=150]

--
* bit information

@* Header bits : total slice header bits of the current frame except start code
@* Residual bits : total residual bits of the current frame

header bits are in the beginning 32bits of w4EncReportBaseAddr,
 and header bits are in the following 32bits of the report buffer.

* frame distortion

With this command, 
frame distortion data - frame distortion sum, y, cb, and cr for each 8bytes, totally 32bytes) are 
reported 8bytes away from w4EncReportBaseAddr (w4EncReportBaseAddr + 8) when picture encoding is completed. 

* QP histogram

With this command, 
QP histogram for every subCTU(CU32) taking 16bits are reported 40bytes away from w4EncReportBaseAddr
(w4EncReportBaseAddr + 40) when picture encoding is completed. 
--

====== SET_SIZE_ENC_RC_REPORT
This command sets the size of report buffer. 
It should be specified when any of these bitInfoReportEnable, frameDistortionReportEnable,
 or qpHistogramReportEnable in EncParam is on. 
 
@endverbatim
*/
RetCode VPU_EncGiveCommand(
    EncHandle handle,   /**< [Input] An encoder handle obtained from VPU_EncOpen() */
    CodecCommand cmd,   /**< [Input] A variable specifying the given command of CodecComand type */
    void * parameter    /**< [Input/Output] A pointer to command-specific data structure which describes picture I/O parameters for the current encoder instance */
    );
    
/**
* @brief    
@verbatim
This function starts encoding one frame. Returning from this function does not
mean the completion of encoding one frame, and it is just that encoding one
frame was initiated.

Every call of this function should be matched with VPU_EncGetOutputInfo()
with the same handle. Without calling a pair of these funtions,
HOST application cannot call any other API functions
except for VPU_IsBusy(), VPU_EncGetBitstreamBuffer(), and
VPU_EncUpdateBitstreamBuffer().
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
@endverbatim
*/
RetCode VPU_EncStartOneFrame(
    EncHandle handle,   /**< [Input] An encoder handle obtained from VPU_EncOpen()  */
    EncParam * param    /**< [Input] A pointer to EncParam type structure which describes picture encoding parameters for the current encoder instance.  */
    );

/**
* @brief    This function gets information of the output of encoding. Application can know
about picture type, the address and size of the generated bitstream, the number
of generated slices, the end addresses of the slices, and macroblock bit
position information. HOST application should call this function after frame
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
@endverbatim
*/
RetCode VPU_EncGetOutputInfo(
    EncHandle handle,       /**<  [Input] An encoder handle obtained from VPU_EncOpen().  */
    EncOutputInfo * info    /**<  [Output] A pointer to a EncOutputInfo type structure which describes picture encoding results for the current encoder instance. */
    );

/**
* @brief    After encoding frame, HOST application must get bitstream from the encoder. To do
that, they must know where to get bitstream and the maximum size.
Applications can get the information by calling this function.
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
    int * size                  /**<  [Output] A variable specifying the available space in bitstream buffer for the current encoder instance */
    );

/**
* @brief    Applications must let encoder know how much bitstream has been transferred from
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
    int size            /**< [Input] A variable specifying the amount of bits being filled from bitstream buffer for the current encoder instance */
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
    PhysicalAddress addr,   /**< [Input] Updated write or read pointer */
    int updateRdPtr         /**< [Input] A flag whether to move the read pointer to where the write pointer is located */
    );    

/** 
Not used
*/  
void VPU_EncSetHostParaAddr(
    PhysicalAddress baseAddr,
    PhysicalAddress paraAddr
    );

RetCode VPU_EncIssueSeqInit(
    EncHandle handle           /**<  [Input] A encoder handle obtained from VPU_EncOpen() */
    );


#ifdef __cplusplus
}
#endif

#endif

