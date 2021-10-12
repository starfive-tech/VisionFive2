//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2011  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#ifndef VPUAPI_UTIL_H_INCLUDED
#define VPUAPI_UTIL_H_INCLUDED

#include "vpuapi.h"

// COD_STD
enum {
    HEVC_DEC = 0,
    AVC_DEC  = 0,
    VC1_DEC  = 1,
    HEVC_ENC = 1,
    MP2_DEC  = 2,
    MP4_DEC  = 3,
    DV3_DEC  = 3,
    RV_DEC   = 4,
    AVS_DEC  = 5,
    VPX_DEC  = 7,
    MAX_DEC  = 7,
    AVC_ENC  = 8,
    MP4_ENC  = 11,
    MAX_CODECS,
};

// new COD_STD since Coda7Q, WAVE412
enum {
    C7_HEVC_DEC                 = 0x00,
    C7_HEVC_ENC                 = 0x01,

    C7_AVC_DEC                  = 0x10,
    C7_VC1_DEC                  = 0x11,
    C7_MP2_DEC                  = 0x12,
    C7_MP4_DEC                  = 0x13,
    C7_DV3_DEC                  = 0x13,
    C7_RVX_DEC                  = 0x14,
    C7_AVS_DEC                  = 0x15,
    C7_VP9_DEC                  = 0x16,
    C7_VP8_DEC                  = 0x17,
    C7_AVS2_DEC                 = 0x18,

    C7_VP9_ENC                  = 0x19,
    C7_SVAC_DEC                 = 0x20,
    C7_SVAC_ENC                 = 0x21,

    C7_AVC_ENC                  = 0x88, /* The original value is 0x18 */
    C7_MP4_ENC                  = 0x89, /* The original value is 0x19 */

    STD_UNKNOWN                 = 0xFF
};

// AUX_COD_STD
enum {
    MP4_AUX_MPEG4 = 0,
    MP4_AUX_DIVX3 = 1
};

enum {
    VPX_AUX_THO = 0,
    VPX_AUX_VP6 = 1,
    VPX_AUX_VP8 = 2,
    VPX_AUX_NUM
};

enum {
    AVC_AUX_AVC = 0,
    AVC_AUX_MVC = 1,
    AVC_AUX_SVC = 2
};

// BIT_RUN command
enum {
    DEC_SEQ_INIT = 1,
    ENC_SEQ_INIT = 1,
    DEC_SEQ_END = 2,
    ENC_SEQ_END = 2,
    PIC_RUN = 3,
    SET_FRAME_BUF = 4,
    ENCODE_HEADER = 5,
    ENC_PARA_SET = 6,
    DEC_PARA_SET = 7,
    DEC_BUF_FLUSH = 8,
    RC_CHANGE_PARAMETER    = 9,
    VPU_SLEEP = 10,
    VPU_WAKE = 11,
    ENC_ROI_INIT = 12,
    FIRMWARE_GET = 0xf
};

enum {
    SRC_BUFFER_EMPTY        = 0,    //!< source buffer doesn't allocated.
    SRC_BUFFER_ALLOCATED    = 1,    //!< source buffer has been allocated.
    SRC_BUFFER_SRC_LOADED   = 2,    //!< source buffer has been allocated.
    SRC_BUFFER_USE_ENCODE   = 3     //!< source buffer was sent to VPU. but it was not used for encoding.
};

#define HEVC_MAX_SUB_LAYER_ID   6
#define AVC_MAX_SUB_LAYER_ID    15

//#define API_DEBUG
#ifdef API_DEBUG
#ifdef _MSC_VER
#define APIDPRINT(_fmt, ...)            printf(_fmt, __VA_ARGS__)
#else
#define APIDPRINT(_fmt, ...)            printf(_fmt, ##__VA_ARGS__)
#endif
#else
#define APIDPRINT(_fmt, ...)            
#endif

extern Uint32 __VPU_BUSY_TIMEOUT;
/**
 * PRODUCT: CODA960/CODA980/WAVE320
 */
typedef struct {
    union {
        struct {
            int             useBitEnable;
            int             useIpEnable;
            int             useDbkYEnable;
            int             useDbkCEnable;
            int             useOvlEnable;
            int             useBtpEnable;
            int             useMeEnable;        // used only for coda7q encoder
            PhysicalAddress bufBitUse;
            PhysicalAddress bufIpAcDcUse;
            PhysicalAddress bufDbkYUse;
            PhysicalAddress bufDbkCUse;
            PhysicalAddress bufOvlUse;
            PhysicalAddress bufBtpUse;
            PhysicalAddress bufMeUse;           // used only for coda7q encoder
        } coda9;
        struct {
            int             useIpEnable;
            int             useLfRowEnable;
            int             useBitEnable;
            PhysicalAddress bufIp;
            PhysicalAddress bufLfRow;
            PhysicalAddress bufBit;
            int             useEncImdEnable;
            int             useEncRdoEnable;
            int             useEncLfEnable;
            PhysicalAddress bufImd;
            PhysicalAddress bufRdo;
            PhysicalAddress bufLf;
        } wave4;
    } u;
    int             bufSize;
    PhysicalAddress bufBase;
} SecAxiInfo;

typedef struct {
    DecOpenParam    openParam;
    DecInitialInfo  initialInfo;
    DecInitialInfo  newSeqInfo;     /* Temporal new sequence information */
    PhysicalAddress streamWrPtr;
    PhysicalAddress streamRdPtr;
    Int32           streamBufFull;
    int             streamEndflag;
    int             frameDisplayFlag;
    int             clearDisplayIndexes;
    int             setDisplayIndexes;
    PhysicalAddress streamRdPtrRegAddr;
    PhysicalAddress streamWrPtrRegAddr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
    PhysicalAddress frameDisplayFlagRegAddr;
    PhysicalAddress currentPC;
    PhysicalAddress busyFlagAddr;
    int             streamBufSize;
    FrameBuffer     frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t    vbFrame;
    vpu_buffer_t    vbWTL;
    vpu_buffer_t    vbPPU;
    vpu_buffer_t    vbMV[MAX_REG_FRAME];
    vpu_buffer_t    vbFbcYTbl[MAX_REG_FRAME];
    vpu_buffer_t    vbFbcCTbl[MAX_REG_FRAME];
    int             chBwbFrameIdx;
    int             chFbcFrameIdx;
    BOOL            interResChange;
    int             frameAllocExt;
    int             ppuAllocExt;    
    int             numFrameBuffers;
    int             numFbsForDecoding;                  /*!<< number of framebuffers used in decoding */
    int             numFbsForWTL;                       /*!<< number of linear framebuffer for displaying when DecInfo::wtlEnable is set to 1 */
    int             stride;
    int             frameBufferHeight;
    int             rotationEnable;
    int             mirrorEnable;
    int             deringEnable;
    MirrorDirection mirrorDirection;
    int             rotationAngle;
    FrameBuffer     rotatorOutput;
    int             rotatorStride;
    int             rotatorOutputValid;    
    int             initialInfoObtained;
    int             vc1BframeDisplayValid;
    int             mapType;
    int             tiled2LinearEnable;
    int             tiled2LinearMode;       
    int             wtlEnable;              
    int             wtlMode;                
    FrameBufferFormat   wtlFormat;                      /*!<< default value: FORMAT_420 8bit */
    SecAxiInfo          secAxiInfo;
    MaverickCacheConfig cacheConfig;
    int chunkSize;
    int seqInitEscape;

    // Report Information
    PhysicalAddress userDataBufAddr;    
    vpu_buffer_t    vbUserData;    

    BOOL            userDataEnable;                    /* User Data Enable Flag
                                                          CODA9xx: TRUE or FALSE
                                                          WAVE4xx: Refer to H265_USERDATA_FLAG_xxx values in vpuapi.h */
    int             userDataBufSize;
    int             userDataReportMode;                // User Data report mode (0 : interrupt mode, 1 interrupt disable mode)

    BOOL            cuDataEnable;                       //!<< Enable reporting CU data
    Int32           cuDataBufSize;                      //!<< Size of CU buffer in bytes
    PhysicalAddress cuDataBufAddr;                      //!<< Physical address 


    LowDelayInfo    lowDelayInfo;
    int             frameStartPos;
    int             frameEndPos;
    int             frameDelay;
    vpu_buffer_t    vbSlice;            // AVC, VP8 only
    vpu_buffer_t    vbWork;
    vpu_buffer_t    vbTemp;
    vpu_buffer_t    vbReport;
    int             workBufferAllocExt;
    DecOutputInfo   decOutInfo[MAX_GDI_IDX];
    TiledMapConfig  mapCfg;
    int             reorderEnable;
    Uint32          avcErrorConcealMode;
    DRAMConfig      dramCfg;            //coda960 only
    int             thumbnailMode;
    int             seqChangeMask;      // WAVE410
    Uint32          prevFrameEndPos;      //!<< WAVE410v2: end position of previous frame
    Int32           targetSubLayerId;       //!<< H.265 temporal scalability 


    Uint32          div3Width;      // for CODA7Q
    Uint32          div3Height;     // for CODA7Q
    int             rdPtrValidFlag;
    
    Int32           instanceQueueCount;
    Int32           totalQueueCount;
} DecInfo;

typedef struct {
    EncOpenParam        openParam;
    EncInitialInfo      initialInfo;
    PhysicalAddress     streamRdPtr;
    PhysicalAddress     streamWrPtr;
    int                 streamEndflag;
    PhysicalAddress     streamRdPtrRegAddr;
    PhysicalAddress     streamWrPtrRegAddr;
    PhysicalAddress     streamBufStartAddr;
    PhysicalAddress     streamBufEndAddr;    
    PhysicalAddress     currentPC;
    PhysicalAddress     busyFlagAddr;
    int                 streamBufSize;
    int                 linear2TiledEnable;
    int                 linear2TiledMode;    // coda980 only
    int                 mapType;
    int                 userMapEnable;
    FrameBuffer         frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t        vbFrame;
    vpu_buffer_t        vbPPU;    
    int                 frameAllocExt;
    int                 ppuAllocExt;    
    vpu_buffer_t        vbSubSampFrame;         /*!<< CODA960 only */
    vpu_buffer_t        vbMvcSubSampFrame;      /*!<< CODA960 only */
    int                 numFrameBuffers;
    int                 stride;
    int                 frameBufferHeight;
    int                 rotationEnable;
    int                 mirrorEnable;
    MirrorDirection     mirrorDirection;
    int                 rotationAngle;
    int                 initialInfoObtained;
    int                 ringBufferEnable;
    SecAxiInfo          secAxiInfo;
    MaverickCacheConfig cacheConfig;

    int                 sliceIntEnable;       /*!<< WAVE420 only */

    int                 ActivePPSIdx;           /*!<< CODA980 */
    int                 frameIdx;               /*!<< CODA980 */
    int                 fieldDone;              /*!<< CODA980 */
    int                 lineBufIntEn;
    vpu_buffer_t        vbWork;
    vpu_buffer_t        vbScratch;

    vpu_buffer_t        vbTemp;                 //!< Temp buffer (WAVE420)
    vpu_buffer_t        vbMV;                   //!< colMV buffer (WAVE420)
    vpu_buffer_t        vbFbcYTbl;              //!< FBC Luma table buffer (WAVE420)
    vpu_buffer_t        vbFbcCTbl;              //!< FBC Chroma table buffer (WAVE420)
    vpu_buffer_t        vbSubSamBuf;            //!< Sub-sampled buffer for ME (WAVE420)

    TiledMapConfig      mapCfg;    
    DRAMConfig          dramCfg;                /*!<< CODA960 */


    Uint32 prefixSeiNalEnable;
    Uint32 prefixSeiDataSize;
    Uint32 prefixSeiDataEncOrder;
    PhysicalAddress prefixSeiNalAddr;
    Uint32 suffixSeiNalEnable;
    Uint32 suffixSeiDataSize;
    Uint32 suffixSeiDataEncOrder;
    PhysicalAddress suffixSeiNalAddr;

    Int32   errorReasonCode;
    Uint64          curPTS;             /**! Current timestamp in 90KHz */
    Uint64          ptsMap[32];         /**! PTS mapped with source frame index */
    Uint32          instanceQueueCount;
    Uint32          totalQueueCount;

} EncInfo;


typedef struct CodecInst {
    Int32   inUse;
    Int32   instIndex;
    Int32   coreIdx;
    Int32   codecMode;
    Int32   codecModeAux;    
    Int32   productId;
    Int32   loggingEnable;
    Uint32  isDecoder;
    union {
        EncInfo encInfo;
        DecInfo decInfo;
    }* CodecInfo;
} CodecInst;

/*******************************************************************************
 * H.265 USER DATA(VUI and SEI)                                           *
 *******************************************************************************/
#define H265_MAX_DPB_SIZE               17
#define H265_MAX_NUM_SUB_LAYER          8
#define H265_MAX_NUM_ST_RPS             64
#define H265_MAX_CPB_CNT                32
#define H265_MAX_NUM_VERTICAL_FILTERS   5
#define H265_MAX_NUM_HORIZONTAL_FILTERS 3
#define H265_MAX_TAP_LENGTH             32
#define H265_MAX_NUM_KNEE_POINT         999

typedef struct
{
    Uint32   offset;
    Uint32   size;
} user_data_entry_t;

typedef struct
{
    Int16   left;
    Int16   right;
    Int16   top;
    Int16   bottom;
} win_t;

typedef struct  
{
    Uint8    nal_hrd_param_present_flag;
    Uint8    vcl_hrd_param_present_flag;
    Uint8    sub_pic_hrd_params_present_flag;
    Uint8    tick_divisor_minus2;
    Int8     du_cpb_removal_delay_inc_length_minus1;
    Int8     sub_pic_cpb_params_in_pic_timing_sei_flag;
    Int8     dpb_output_delay_du_length_minus1;
    Int8     bit_rate_scale;
    Int8     cpb_size_scale;
    Int8     initial_cpb_removal_delay_length_minus1;
    Int8     cpb_removal_delay_length_minus1;
    Int8     dpb_output_delay_length_minus1;

    Uint8    fixed_pic_rate_gen_flag[H265_MAX_NUM_SUB_LAYER];
    Uint8    fixed_pic_rate_within_cvs_flag[H265_MAX_NUM_SUB_LAYER];
    Uint8    low_delay_hrd_flag[H265_MAX_NUM_SUB_LAYER];
    Int8     cpb_cnt_minus1[H265_MAX_NUM_SUB_LAYER];
    Int16    elemental_duration_in_tc_minus1[H265_MAX_NUM_SUB_LAYER];

    Uint32   nal_bit_rate_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32   nal_cpb_size_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32   nal_cpb_size_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint32   nal_bit_rate_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint8    nal_cbr_flag[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];

    Uint32   vcl_bit_rate_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32   vcl_cpb_size_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    Uint32   vcl_cpb_size_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint32   vcl_bit_rate_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    Uint8    vcl_cbr_flag[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
} h265_hrd_param_t;

typedef struct
{
    Uint8    aspect_ratio_info_present_flag;
    Uint8    aspect_ratio_idc;
    Uint8    overscan_info_present_flag;
    Uint8    overscan_appropriate_flag;

    Uint8    video_signal_type_present_flag;
    Int8     video_format;

    Uint8    video_full_range_flag;
    Uint8    colour_description_present_flag;

    Uint16   sar_width;
    Uint16   sar_height;

    Uint8    colour_primaries;
    Uint8    transfer_characteristics;
    Uint8    matrix_coefficients;

    Uint8    chroma_loc_info_present_flag;
    Int8     chroma_sample_loc_type_top_field;
    Int8     chroma_sample_loc_type_bottom_field;

    Uint8    neutral_chroma_indication_flag;

    Uint8    field_seq_flag;

    Uint8    frame_field_info_present_flag;
    Uint8    default_display_window_flag;
    Uint8    vui_timing_info_present_flag;
    Uint8    vui_poc_proportional_to_timing_flag;

    Uint32   vui_num_units_in_tick;
    Uint32   vui_time_scale;

    Uint8    vui_hrd_parameters_present_flag;
    Uint8    bitstream_restriction_flag;

    Uint8    tiles_fixed_structure_flag;
    Uint8    motion_vectors_over_pic_boundaries_flag;
    Uint8    restricted_ref_pic_lists_flag;
    Int8     min_spatial_segmentation_idc;
    Int8     max_bytes_per_pic_denom;
    Int8     max_bits_per_mincu_denom;

    Int16    vui_num_ticks_poc_diff_one_minus1;
    Int8     log2_max_mv_length_horizontal;
    Int8     log2_max_mv_length_vertical;

    win_t    def_disp_win;
    h265_hrd_param_t hrd_param;
} h265_vui_param_t;

typedef struct
{
    Uint32   display_primaries_x[3];
    Uint32   display_primaries_y[3];
    Uint32   white_point_x                   : 16;
    Uint32   white_point_y                   : 16;
    Uint32   max_display_mastering_luminance : 32;
    Uint32   min_display_mastering_luminance : 32;
} h265_mastering_display_colour_volume_t;

typedef struct
{
    Uint32   ver_chroma_filter_idc               : 8;
    Uint32   hor_chroma_filter_idc               : 8;
    Uint32   ver_filtering_field_processing_flag : 1;
    Uint32   target_format_idc                   : 2; 
    Uint32   num_vertical_filters                : 3;
    Uint32   num_horizontal_filters              : 3;
    Uint8    ver_tap_length_minus1[ H265_MAX_NUM_VERTICAL_FILTERS ]; 
    Uint8    hor_tap_length_minus1[ H265_MAX_NUM_HORIZONTAL_FILTERS ]; 
    Int32    ver_filter_coeff[ H265_MAX_NUM_VERTICAL_FILTERS ][ H265_MAX_TAP_LENGTH ];
    Int32    hor_filter_coeff[ H265_MAX_NUM_HORIZONTAL_FILTERS ][ H265_MAX_TAP_LENGTH ];
} h265_chroma_resampling_filter_hint_t;

typedef struct
{
    Uint32   knee_function_id;
    Uint8    knee_function_cancel_flag;

    Uint8    knee_function_persistence_flag;
    Uint32   input_disp_luminance;
    Uint32   input_d_range;
    Uint32   output_d_range;
    Uint32   output_disp_luminance;
    Uint16   num_knee_points_minus1;
    Uint16   input_knee_point[ H265_MAX_NUM_KNEE_POINT ];
    Uint16   output_knee_point[ H265_MAX_NUM_KNEE_POINT ];
} h265_knee_function_info_t;

typedef struct
{
    Int8   status;           // 0 : empty, 1 : occupied
    Int8   pic_struct;
    Int8   source_scan_type;
    Int8   duplicate_flag;
} h265_sei_pic_timing_t;

#ifdef __cplusplus
extern "C" {
#endif

RetCode InitCodecInstancePool(Uint32 coreIdx);
RetCode GetCodecInstance(Uint32 coreIdx, CodecInst ** ppInst);
void    FreeCodecInstance(CodecInst * pCodecInst);

RetCode CheckDecOpenParam(DecOpenParam * pop);
int     DecBitstreamBufEmpty(DecInfo * pDecInfo);
#ifdef DRAM_TEST
RetCode ExecDRAMReadWriteTest(Uint32 coreIdx, Uint32* dram_source_addr, Uint32* dram_destination_addr, Uint32* dram_data_size);
#endif
RetCode SetParaSet(DecHandle handle, int paraSetType, DecParamSet * para);
void    DecSetHostParaAddr(Uint32 coreIdx, PhysicalAddress baseAddr, PhysicalAddress paraAddr);


Int32 ConfigSecAXICoda9(
    Uint32      coreIdx, 
    Int32       codecMode, 
    SecAxiInfo* sa, 
    Uint32      width, 
    Uint32      height, 
    Uint32      profile
    );

Int32 ConfigSecAXIWave(
    Uint32      coreIdx, 
    Int32       codecMode,
    SecAxiInfo* sa, 
    Uint32      width, 
    Uint32      height, 
    Uint32      profile,
    Uint32      level
    );

Int32 ConfigSecAXICoda7(
    Uint32      coreIdx, 
    CodStd      codStd, 
    SecAxiInfo* sa, 
    Uint32      width, 
    Uint32      height, 
    Uint32      profile
    );

RetCode AllocateLinearFrameBuffer(
    TiledMapType            mapType,
    FrameBuffer*            fbArr,
    Uint32                  numOfFrameBuffers,
    Uint32                  sizeLuma,
    Uint32                  sizeChroma
    );

RetCode AllocateTiledFrameBufferGdiV1(
    TiledMapType            mapType,
    PhysicalAddress         tiledBaseAddr,
    FrameBuffer*            fbArr,
    Uint32                  numOfFrameBuffers,
    Uint32                  sizeLuma,
    Uint32                  sizeChroma,
    DRAMConfig*             pDramCfg
    );

RetCode AllocateTiledFrameBufferGdiV2(
    TiledMapType            mapType,
    FrameBuffer*            fbArr,
    Uint32                  numOfFrameBuffers,
    Uint32                  sizeLuma,
    Uint32                  sizeChroma
    );


RetCode CheckEncInstanceValidity(EncHandle handle);
RetCode CheckEncOpenParam(EncOpenParam * pop);
RetCode CheckEncParam(EncHandle handle, EncParam * param);
RetCode GetEncHeader(EncHandle handle, EncHeaderParam * encHeaderParam);
RetCode EncParaSet(EncHandle handle, int paraSetType);
RetCode SetGopNumber(EncHandle handle, Uint32 *gopNumber);
RetCode SetIntraQp(EncHandle handle, Uint32 *intraQp);
RetCode SetBitrate(EncHandle handle, Uint32 *bitrate);
RetCode SetFramerate(EncHandle handle, Uint32 *frameRate);
RetCode SetIntraRefreshNum(EncHandle handle, Uint32 *pIntraRefreshNum);
RetCode SetSliceMode(EncHandle handle, EncSliceMode *pSliceMode);
RetCode SetHecMode(EncHandle handle, int mode);
void    EncSetHostParaAddr(Uint32 coreIdx, PhysicalAddress baseAddr, PhysicalAddress paraAddr);    

RetCode EnterLock(Uint32 coreIdx);
RetCode LeaveLock(Uint32 coreIdx);
RetCode SetClockGate(Uint32 coreIdx, Uint32 on);

RetCode EnterDispFlagLock(Uint32 coreIdx);
RetCode LeaveDispFlagLock(Uint32 coreIdx);

void SetPendingInst(Uint32 coreIdx, CodecInst *inst);
void ClearPendingInst(Uint32 coreIdx);
CodecInst *GetPendingInst(Uint32 coreIdx);
int GetPendingInstIdx(Uint32 coreIdx);

Int32 MaverickCache2Config(
    MaverickCacheConfig* pCache, 
    BOOL                decoder , 
    BOOL                interleave, 
    Uint32              bypass, 
    Uint32              burst, 
    Uint32              merge, 
    TiledMapType        mapType, 
    Uint32              wayshape
    );

int SetTiledMapType(Uint32 coreIdx, TiledMapConfig *pMapCfg, int mapType, int stride, int interleave, DRAMConfig *dramCfg);
int GetXY2AXIAddr(TiledMapConfig *pMapCfg, int ycbcr, int posY, int posX, int stride, FrameBuffer *fb);
int GetLowDelayOutput(CodecInst *pCodecInst, DecOutputInfo *lowDelayOutput);
//for GDI 1.0
void SetTiledFrameBase(Uint32 coreIdx, PhysicalAddress baseAddr);
PhysicalAddress GetTiledFrameBase(Uint32 coreIdx, FrameBuffer *frame, int num);

/**
 * \brief   It returns the stride of framebuffer in byte.
 *
 * \param   width           picture width in pixel.
 * \param   format          YUV format. see FrameBufferFormat structure in vpuapi.h
 * \param   cbcrInterleave  
 * \param   mapType         map type. see TiledMapType in vpuapi.h
 */
Int32 CalcStride(
    Uint32              width,
    Uint32              height,
    FrameBufferFormat   format,
    BOOL                cbcrInterleave,
    TiledMapType        mapType,
    BOOL                isVP9
    );

Int32 CalcLumaSize(
    Int32               productId, 
    Int32               stride, 
    Int32               height, 
    FrameBufferFormat   format, 
    BOOL                cbcrIntl, 
    TiledMapType        mapType, 
    DRAMConfig*         pDramCfg
    );

Int32 CalcChromaSize(
    Int32               productId, 
    Int32               stride, 
    Int32               height, 
    FrameBufferFormat   format, 
    BOOL                cbcrIntl, 
    TiledMapType        mapType, 
    DRAMConfig*         pDramCfg
    );

int LevelCalculation(int MbNumX, int MbNumY, int frameRateInfo, int interlaceFlag, int BitRate, int SliceNum);

/* timescale: 1/90000 */
Uint64 GetTimestamp(
    EncHandle handle
    );

RetCode CalcEncCropInfo(
    EncHevcParam* param,
    int rotMode, 
    int srcWidth, 
    int srcHeight
    );


#ifdef __cplusplus
}
#endif

#endif // endif VPUAPI_UTIL_H_INCLUDED

