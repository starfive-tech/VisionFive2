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

#ifndef _MAIN_HELPER_H_
#define _MAIN_HELPER_H_


#ifdef USE_FEEDING_METHOD_BUFFER
   #include "wave511/config.h"
   #include "wave511/vpuapi/vpuapifunc.h"
   #include "wave511/vpuapi/vpuapi.h"
   #include "wave511/vpuapi/vputypes.h"
#else
    #include "config.h"
    #include "vpuapifunc.h"
    #include "vpuapi.h"
    #include "vputypes.h"
#endif
#ifdef PLATFORM_QNX
    #include <sys/stat.h>
#endif


#define MATCH_OR_MISMATCH(_expected, _value, _ret)        ((_ret=(_expected == _value)) ? "MATCH" : "MISMATCH")

#if defined(WIN32) || defined(WIN64)
/*
 ( _MSC_VER => 1200 )     6.0     vs6
 ( _MSC_VER => 1310 )     7.1     vs2003
 ( _MSC_VER => 1400 )     8.0     vs2005
 ( _MSC_VER => 1500 )     9.0     vs2008
 ( _MSC_VER => 1600 )    10.0     vs2010
 */
#if (_MSC_VER == 1200)
#define strcasecmp          stricmp
#define strncasecmp         strnicmp
#else
#define strcasecmp          _stricmp
#define strncasecmp         _strnicmp
#endif
#define inline              _inline
#if (_MSC_VER == 1600)
#define strdup              _strdup
#endif
#endif

#define MAX_GETOPT_OPTIONS 100
//extension of option struct in getopt
struct OptionExt
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
    const char *help;
};

#define MAX_FILE_PATH               256
#define MAX_PIC_SKIP_NUM            5



#define EXTRA_SRC_BUFFER_NUM            0
#define VPU_WAIT_TIME_OUT               10  //should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
#define VPU_WAIT_TIME_OUT_CQ            1
#define MAX_NOT_DEC_COUNT               2000
#define COMPARE_RESOLUTION(_src, _dst)  (_src->width == _dst->width && _src->height == _dst->height)

typedef union {
    struct {
        Uint32  ctu_force_mode  :  2; //[ 1: 0]
        Uint32  ctu_coeff_drop  :  1; //[    2]
        Uint32  reserved        :  5; //[ 7: 3]
        Uint32  sub_ctu_qp_0    :  6; //[13: 8]
        Uint32  sub_ctu_qp_1    :  6; //[19:14]
        Uint32  sub_ctu_qp_2    :  6; //[25:20]
        Uint32  sub_ctu_qp_3    :  6; //[31:26]

        Uint32  lambda_sad_0    :  8; //[39:32]
        Uint32  lambda_sad_1    :  8; //[47:40]
        Uint32  lambda_sad_2    :  8; //[55:48]
        Uint32  lambda_sad_3    :  8; //[63:56]
    } field;
} EncCustomMap; // for wave5xx custom map (1 CTU = 64bits)

typedef union {
    struct {
        Uint8  mb_force_mode  :  2; //lint !e46 [ 1: 0]
        Uint8  mb_qp          :  6; //lint !e46 [ 7: 2]
    } field;
} AvcEncCustomMap; // for AVC custom map on wave  (1 MB = 8bits)

typedef enum {
    MODE_YUV_LOAD = 0,
    MODE_COMP_JYUV,
    MODE_SAVE_JYUV,

    MODE_COMP_CONV_YUV,
    MODE_SAVE_CONV_YUV,

    MODE_SAVE_LOAD_YUV,

    MODE_COMP_RECON,
    MODE_SAVE_RECON,

    MODE_COMP_ENCODED,
    MODE_SAVE_ENCODED
} CompSaveMode;

typedef struct {
    int picX;
    int picY;
    int internalBitDepth;
    int losslessEnable;
    int constIntraPredFlag;
    int gopSize;
    int numTemporalLayers;
    int decodingRefreshType;
    int intraQP;
    int intraPeriod;
    int frameRate;

    int confWinTop;
    int confWinBot;
    int confWinLeft;
    int confWinRight;

    int independSliceMode;
    int independSliceModeArg;
    int dependSliceMode;
    int dependSliceModeArg;
    int intraRefreshMode;
    int intraRefreshArg;

    int useRecommendEncParam;
    int scalingListEnable;
    int cuSizeMode;
    int tmvpEnable;
    int wppenable;
    int maxNumMerge;

    int disableDeblk;
    int lfCrossSliceBoundaryEnable;
    int betaOffsetDiv2;
    int tcOffsetDiv2;
    int skipIntraTrans;
    int saoEnable;
    int intraNxNEnable;
    int rcEnable;

    int bitRate;
    int bitAllocMode;
    int fixedBitRatio[MAX_GOP_NUM];
    int cuLevelRCEnable;
    int hvsQPEnable;

    int hvsQpScale;
    int minQp;
    int maxQp;
    int maxDeltaQp;

    int gopPresetIdx;
    // CUSTOM_GOP
    CustomGopParam gopParam;

    // ROI / CTU mode
    int roiEnable;                      /**< It enables ROI map. NOTE: It is valid when rcEnable is on. */
    char roiFileName[MAX_FILE_PATH];
    char roiQpMapFile[MAX_FILE_PATH];

    // VUI
    Uint32 numUnitsInTick;
    Uint32 timeScale;
    Uint32 numTicksPocDiffOne;

    int encAUD;
    int encEOS;
    int encEOB;

    int chromaCbQpOffset;
    int chromaCrQpOffset;

    Uint32 initialRcQp;

    Uint32  nrYEnable;
    Uint32  nrCbEnable;
    Uint32  nrCrEnable;
    Uint32  nrNoiseEstEnable;
    Uint32  nrNoiseSigmaY;
    Uint32  nrNoiseSigmaCb;
    Uint32  nrNoiseSigmaCr;

    Uint32  nrIntraWeightY;
    Uint32  nrIntraWeightCb;
    Uint32  nrIntraWeightCr;

    Uint32  nrInterWeightY;
    Uint32  nrInterWeightCb;
    Uint32  nrInterWeightCr;

    Uint32 useAsLongtermPeriod;
    Uint32 refLongtermPeriod;

    // newly added for encoder
    Uint32 monochromeEnable;
    Uint32 strongIntraSmoothEnable;
    Uint32 roiAvgQp;
    Uint32 weightPredEnable;
    Uint32 bgDetectEnable;
    Uint32 bgThrDiff;
    Uint32 bgThrMeanDiff;
    Uint32 bgLambdaQp;
    int    bgDeltaQp;
    Uint32 lambdaMapEnable;
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
    int    forcePicSkipStart;
    int    forcePicSkipEnd;
    int    forceCoefDropStart;
    int    forceCoefDropEnd;
    char   scalingListFileName[MAX_FILE_PATH];
    char   customLambdaFileName[MAX_FILE_PATH];

    Uint32 enStillPicture;

    // custom map
    int    customLambdaMapEnable;
    char   customLambdaMapFileName[MAX_FILE_PATH];
    int    customModeMapFlag;
    char   customModeMapFileName[MAX_FILE_PATH];

    char   WpParamFileName[MAX_FILE_PATH];

    // for H.264 on WAVE
    int idrPeriod;
    int rdoSkip;
    int lambdaScalingEnable;
    int transform8x8;
    int avcSliceMode;
    int avcSliceArg;
    int intraMbRefreshMode;
    int intraMbRefreshArg;
    int mbLevelRc;
    int entropyCodingMode;

    int s2fmeDisable;
    int forceIdrPicIdx;
    int forcedIdrHeaderEnable;
#ifdef SUPPORT_LOOK_AHEAD_RC
    int larcEnable;
    int larcPass;
    int larcSize;
    int larcWeight;
#endif
    Uint32 rcWeightParam;
    Uint32 rcWeightBuf;
} WAVE_ENC_CFG;

typedef struct {
    // ChangePara
    int setParaChgFrmNum;
    int enableOption;
    char cfgName[MAX_FILE_PATH];
} W5ChangeParam;

typedef struct {
    char SrcFileName[MAX_FILE_PATH];
    char BitStreamFileName[MAX_FILE_PATH];
    BOOL srcCbCrInterleave;
    int NumFrame;
    int PicX;
    int PicY;
    int FrameRate;

    // MPEG4 ONLY
    int VerId;
    int DataPartEn;
    int RevVlcEn;
    int ShortVideoHeader;
    int AnnexI;
    int AnnexJ;
    int AnnexK;
    int AnnexT;
    int IntraDcVlcThr;
    int VopQuant;

    // H.264 ONLY
    int ConstIntraPredFlag;
    int DisableDeblk;
    int DeblkOffsetA;
    int DeblkOffsetB;
    int ChromaQpOffset;
    int PicQpY;
    // H.264 VUI information
    int VuiPresFlag;
    int VideoSignalTypePresFlag;
    char VideoFormat;
    char VideoFullRangeFlag;
    int ColourDescripPresFlag;
    char ColourPrimaries;
    char TransferCharacteristics;
    char MatrixCoeff;
    int NumReorderFrame;
    int MaxDecBuffering;
    int aud_en;
    int level;
    // COMMON
    int GopPicNum;
    int SliceMode;
    int SliceSizeMode;
    int SliceSizeNum;
    // COMMON - RC
    int RcEnable;
    int RcBitRate;
    int RcBitRateBL;
    int RcInitDelay;
    int VbvBufferSize;
    int RcBufSize;
    int IntraRefreshNum;
    int ConscIntraRefreshEnable;
    int frameSkipDisable;
    int ConstantIntraQPEnable;
    int MaxQpSetEnable;
    int MaxQp;

    int frameCroppingFlag;
    int frameCropLeft;
    int frameCropRight;
    int frameCropTop;
    int frameCropBottom;

    //H.264 only
    int MaxDeltaQpSetEnable;
    int MaxDeltaQp;
    int MinQpSetEnable;
    int MinQp;
    int MinDeltaQpSetEnable;
    int MinDeltaQp;
    int intraCostWeight;

    //MP4 Only
    int RCIntraQP;
    int HecEnable;

    int GammaSetEnable;
    int Gamma;

    // NEW RC Scheme
    int rcIntervalMode;
    int RcMBInterval;
    int skipPicNums[MAX_PIC_SKIP_NUM];
    int SearchRange;	// for coda960

    int MeUseZeroPmv;	// will be removed. must be 264 = 0, mpeg4 = 1 263 = 0
    int MeBlkModeEnable; // only api option
    int IDRInterval;
    int SrcBitDepth;

    WAVE_ENC_CFG waveCfg;

    int numChangeParam;
    W5ChangeParam changeParam[10];
    int rcWeightFactor;
} ENC_CFG;


void replace_character(char* str,
    char  c,
    char  r);

extern Uint32 randomSeed;

/* yuv & md5 */
#define NO_COMPARE         0
#define YUV_COMPARE        1
#define MD5_COMPARE        2
#define STREAM_COMPARE     3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* The simple load balancer for performance measurement */
void LoadBalancerInit(void);
void LoadBalancerRelease(void);
BOOL LoadBalancerGetMyTurn(Uint32 myInstance);
void LoadBalancerSetNextTurn(void);
void LoadBalancerAddInstance(Uint32 instanceIndex);
void LoadBalancerRemoveInstance(Uint32 instanceIndex);

/* Performance report */
typedef void*   PFCtx;

PFCtx PFMonitorSetup(
    Uint32  coreIndex,
    Uint32  instanceIndex,
    Uint32  referenceClkInMHz,
    Uint32  fps,
    char*   strLogDir,
    BOOL    isEnc
    );

void PFMonitorRelease(
    PFCtx   context
    );

void PFMonitorUpdate(
    Uint32  coreIndex,
    PFCtx   context,
    Uint32  cycles,
    ...
    );

void PrepareDecoderTest(
    DecHandle decHandle
    );

void byte_swap(
    unsigned char* data,
    int len
    );

void word_swap(
    unsigned char* data,
    int len
    );

void dword_swap(
    unsigned char* data,
    int len
    );

void lword_swap(
    unsigned char* data,
    int len
    );

Int32 LoadFirmware(
    Int32       productId,
    Uint8**   retFirmware,
    Uint32*   retSizeInWord,
    const char* path
    );

void PrintDecSeqWarningMessages(
    Uint32          productId,
    DecInitialInfo* seqInfo
    );

void DisplayDecodedInformation(
    DecHandle      handle,
    CodStd         codec,
    Uint32         frameNo,
    DecOutputInfo* decodedInfo,
    ...
    );

void
DisplayEncodedInformation(
    EncHandle      handle,
    CodStd         codec,
    Uint32         frameNo,
    EncOutputInfo* encodedInfo,
    ...
    );

void PrintEncSppStatus(
    Uint32 coreIdx,
    Uint32 productId
    );
/*
 * VPU Helper functions
 */
/************************************************************************/
/* Video                                                                */
/************************************************************************/

#define PUT_BYTE(_p, _b) \
    *_p++ = (unsigned char)_b;

#define PUT_BUFFER(_p, _buf, _len) \
    osal_memcpy(_p, _buf, _len); \
    (_p) += (_len);

#define PUT_LE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>16); \
    *_p++ = (unsigned char)((_var)>>24);

#define PUT_BE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>24);  \
    *_p++ = (unsigned char)((_var)>>16);  \
    *_p++ = (unsigned char)((_var)>>8); \
    *_p++ = (unsigned char)((_var)>>0);

#define PUT_LE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);

#define PUT_BE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>0);

Int32 ConvFOURCCToMp4Class(
    Int32   fourcc
    );

Int32 ConvFOURCCToCodStd(
    Uint32 fourcc
    );

Int32 ConvCodecIdToMp4Class(
    Uint32 codecId
    );

Int32 ConvCodecIdToCodStd(
    Int32   codecId
    );

Int32 ConvCodecIdToFourcc(
    Int32   codecId
    );

/*!
 * \brief       wrapper function of StoreYuvImageBurstFormat()
 */
Uint8* GetYUVFromFrameBuffer(
    DecHandle       decHandle,
    FrameBuffer*    fb,
    VpuRect         rcFrame,
    Uint32*       retWidth,
    Uint32*       retHeight,
    Uint32*       retBpp,
    size_t*		retSize
    );

BOOL GetYUVFromFrameBuffer2(
    Uint8*          pYuv,
    Uint8**         pYuv2,
    Uint32          size,
    DecHandle       decHandle,
    FrameBuffer*    fb,
    VpuRect         rcFrame,
    Uint32*         retWidth,
    Uint32*         retHeight,
    Uint32*         retBpp,
    size_t*         retSize
    );
/************************************************************************/
/* Queue                                                                */
/************************************************************************/
typedef struct {
    void*   data;
} QueueData;
typedef struct {
    Uint8*          buffer;
    Uint32          size;
    Uint32          itemSize;
    Uint32          count;
    Uint32          front;
    Uint32          rear;
    osal_mutex_t    lock;
} Queue;

Queue* Queue_Create(
    Uint32    itemCount,
    Uint32    itemSize
    );

Queue* Queue_Create_With_Lock(
    Uint32    itemCount,
    Uint32    itemSize
    );

void Queue_Destroy(
    Queue*      queue
    );

/**
 * \brief       Enqueue with deep copy
 */
BOOL Queue_Enqueue(
    Queue*      queue,
    void*       data
    );

/**
 * \brief       Caller has responsibility for releasing the returned data
 */
void* Queue_Dequeue(
    Queue*      queue
    );

void Queue_Flush(
    Queue*      queue
    );

void* Queue_Peek(
    Queue*      queue
    );

Uint32 Queue_Get_Cnt(
    Queue*      queue
    );

/**
 * \brief       @dstQ is NULL, it allocates Queue structure and then copy from @srcQ.
 */
Queue* Queue_Copy(
    Queue*  dstQ,
    Queue*  srcQ
    );

/**
 * \brief       Check the queue is full or not.
 */
BOOL Queue_IsFull(
    Queue*      queue
    );

/************************************************************************/
/* ETC                                                                  */
/************************************************************************/
Uint32 GetRandom(
    Uint32 start,
    Uint32 end
    );

/************************************************************************/
/* MD5                                                                  */
/************************************************************************/

typedef struct MD5state_st {
    Uint32 A,B,C,D;
    Uint32 Nl,Nh;
    Uint32 data[16];
    Uint32 num;
} MD5_CTX;

Int32 MD5_Init(
    MD5_CTX *c
    );

Int32 MD5_Update(
    MD5_CTX*    c,
    const void* data,
    size_t      len);

Int32 MD5_Final(
    Uint8*      md,
    MD5_CTX*    c
    );

Uint8* MD5(
    const Uint8*  d,
    size_t        n,
    Uint8*        md
    );

void plane_md5(MD5_CTX *md5_ctx,
    Uint8  *src,
    int    src_x,
    int    src_y,
    int    out_x,
    int    out_y,
    int    stride,
    int    bpp,
    Uint16 zero
);

/************************************************************************/
/* Comparator                                                           */
/************************************************************************/
#define COMPARATOR_SKIP 0xF0F0F0F0
typedef enum {
    COMPARATOR_CONF_SET_GOLDEN_DATA_SIZE,
    COMPARATOR_CONF_SKIP_GOLDEN_DATA,       /*!<< 2nd parameter pointer of Queue
                                                  containing skip command */
    COMPARATOR_CONF_SET_PICINFO,            //!<< This command is followed by YUVInfo structure.
        COMPARATOR_CONF_SET_MONOCHROME,     //!<< It means a monochrome picture
} ComparatorConfType;

typedef void*   Comparator;
typedef struct ComparatorImpl {
    void*       context;
    char*       filename;
    Uint32      curIndex;
    Uint32      numOfFrames;
    BOOL        (*Create)(struct ComparatorImpl* impl, char* path);
    BOOL        (*Destroy)(struct ComparatorImpl* impl);
    BOOL        (*Compare)(struct ComparatorImpl* impl, void* data, PhysicalAddress size);
    BOOL        (*Configure)(struct ComparatorImpl* impl, ComparatorConfType type, void* val);
    BOOL        (*Rewind)(struct ComparatorImpl* impl);
    BOOL        eof;
    BOOL        enableScanMode;
    BOOL        usePrevDataOneTime;
} ComparatorImpl;

typedef struct {
    Uint32          totalFrames;
    ComparatorImpl* impl;
} AbstractComparator;

// YUV Comparator
typedef struct {
    Uint32            width;
    Uint32            height;
    FrameBufferFormat   format;
    BOOL                cbcrInterleave;
    BOOL                isVp9;
} PictureInfo;

Comparator Comparator_Create(
    Uint32    type,               //!<<   1: yuv
    char* goldenPath,
    ...
    );

BOOL Comparator_Destroy(
    Comparator  comp
    );

BOOL Comparator_Act(
    Comparator  comp,
    void*       data,
    Uint32      size
    );

BOOL Comparator_CheckFrameCount(
    Comparator  comp
    );

BOOL Comparator_SetScanMode(
    Comparator  comp,
    BOOL        enable
    );

BOOL Comparator_Rewind(
    Comparator  comp
    );

BOOL Comparator_CheckEOF(
    Comparator  comp
    );

Uint32 Comparator_GetFrameCount(
    Comparator comp
    );

BOOL Comparator_Configure(
    Comparator              comp,
    ComparatorConfType      cmd,
    void*                   val
    );

BOOL IsEndOfFile(
    FILE* fp
    );

/************************************************************************/
/* Bitstream Feeder                                                     */
/************************************************************************/
typedef enum {
    FEEDING_METHOD_FIXED_SIZE,
    FEEDING_METHOD_FRAME_SIZE,
    FEEDING_METHOD_SIZE_PLUS_ES,
#ifdef USE_FEEDING_METHOD_BUFFER
    FEEDING_METHOD_BUFFER,
#endif
    FEEDING_METHOD_MAX
} FeedingMethod;

typedef struct {
    void*       data;
    Uint32    size;
    BOOL        eos;        //!<< End of stream
    int seqHeaderSize;
} BSChunk;

typedef void* BSFeeder;

typedef void (*BSFeederHook)(BSFeeder feeder, void* data, Uint32 size, void* arg);

/**
 * \brief           BitstreamFeeder consumes bitstream and updates information of bitstream buffer of VPU.
 * \param handle    handle of decoder
 * \param path      bitstream path
 * \param method    feeding method. see FeedingMethod.
 * \param loopCount If @loopCount is greater than 1 then BistreamFeeder reads the start of bitstream again
 *                  when it encounters the end of stream @loopCount times.
 * \param ...       FEEDING_METHOD_FIXED_SIZE:
 *                      This value of parameter is size of chunk at a time.
 *                      If the size of chunk is equal to zero than the BitstreamFeeder reads bistream in random size.(1Byte ~ 4MB)
 * \return          It returns the pointer of handle containing the context of the BitstreamFeeder.
 */
void* BitstreamFeeder_Create(
    Uint32          coreIdx,
    const char*     path,
    CodStd          codecId,
    FeedingMethod   method,
    EndianMode      endian
    );

/**
 * \brief           This is helper function set to simplify the flow that update bit-stream
 *                  to the VPU.
 */
Uint32 BitstreamFeeder_Act(
    BSFeeder        feeder,
    vpu_buffer_t*   bsBuffer,
    PhysicalAddress wrPtr,
    Uint32          room,
    PhysicalAddress* newWrPtr
    );

BOOL BitstreamFeeder_SetFeedingSize(
    BSFeeder    feeder,
    Uint32      size
    );
/**
 * \brief           Set filling bitstream as ringbuffer mode or linebuffer mode.
 * \param   mode    0 : auto
 *                  1 : ringbuffer
 *                  2 : linebuffer.
 */
#define BSF_FILLING_AUTO                    0
#define BSF_FILLING_RINGBUFFER              1
#define BSF_FILLING_LINEBUFFER              2
/* BSF_FILLING_RINBGUFFER_WITH_ENDFLAG:
 * Scenario:
 * - Application writes 1 ~ 10 frames into bitstream buffer.
 * - Set stream end flag by using VPU_DecUpdateBitstreamBuffer(handle, 0).
 * - Application clears stream end flag by using VPU_DecUpdateBitstreamBuffer(handle, -1).
 *   when indexFrameDisplay is equal to -1.
 * NOTE:
 * - Last frame cannot be a complete frame.
 */
#define BSF_FILLING_RINGBUFFER_WITH_ENDFLAG 3
void BitstreamFeeder_SetFillMode(
    BSFeeder    feeder,
    Uint32      mode
    );

BOOL BitstreamFeeder_IsEos(
    BSFeeder    feeder
    );


Uint32 BitstreamFeeder_GetSeqHeaderSize(
    BSFeeder    feeder
    );


BOOL BitstreamFeeder_Destroy(
    BSFeeder    feeder
    );

BOOL BitstreamFeeder_Rewind(
    BSFeeder feeder
    );

BOOL BitstreamFeeder_SetHook(
    BSFeeder        feeder,
    BSFeederHook    hookFunc,
    void*           arg
    );

/************************************************************************/
/* YUV Feeder                                                           */
/************************************************************************/
#define SOURCE_YUV                  0
#define SOURCE_YUV_WITH_LOADER      2
#ifdef USE_FEEDING_METHOD_BUFFER
    #define SOURCE_YUV_WITH_BUFFER  4
#endif
typedef struct {
    Uint32   cbcrInterleave;
    Uint32   nv21;
    Uint32   packedFormat;
    Uint32   srcFormat;
    Uint32   srcPlanar;
    Uint32   srcStride;
    Uint32   srcHeight;
} YuvInfo;

typedef void*  YuvFeeder;
typedef struct YuvFeederImpl {
    void*     context;
    BOOL      (*Create)(struct YuvFeederImpl* impl, const char* path, Uint32 packed, Uint32 fbStride, Uint32 fbHeight);
    BOOL      (*Feed)(struct YuvFeederImpl* impl, Int32   coreIdx, FrameBuffer *fb, size_t picWidth, size_t picHeight, Uint32 srcFbIndex, void* arg);
    BOOL      (*Destroy)(struct YuvFeederImpl* impl);
    BOOL      (*Configure)(struct YuvFeederImpl* impl, Uint32 cmd, YuvInfo yuv);
    EncHandle handle;
} YuvFeederImpl;

typedef struct {
    YuvFeederImpl* impl;
    Uint8          pYuv;
} AbstractYuvFeeder;

typedef struct {
    osal_file_t*    fp;
    Uint8*          pYuv;
    size_t          fbStride;
    size_t          cbcrInterleave;
    BOOL            srcPlanar;
} yuvContext;

YuvFeeder YuvFeeder_Create(
    Uint32        type,
    const char*   srcFilePath,
    YuvInfo       yuvInfo
    );

BOOL YuvFeeder_Feed(
    YuvFeeder       feeder,
    Uint32          coreIdx,
    FrameBuffer*    fb,
    size_t          picWidth,
    size_t          picHeight,
    Uint32          srcFbIndex,
    void*           arg
    );

BOOL YuvFeeder_Destroy(
    YuvFeeder feeder
    );

/************************************************************************/
/* CNM video helper                                                    */
/************************************************************************/
/**
 *  \param  convertCbcrIntl     If this value is TRUE, it stores YUV as NV12 or NV21 to @fb
 */
BOOL LoadYuvImageByYCbCrLine(
    EncHandle   handle,
    Uint32      coreIdx,
    Uint8*      src,
    size_t      picWidth,
    size_t      picHeight,
    FrameBuffer* fb,
    Uint32      srcFbIndex
    );

typedef enum {
    SRC_0LINE_WRITE           = 0,
    SRC_64LINE_WRITE          = 64,
    SRC_128LINE_WRITE         = 128,
    SRC_192LINE_WRITE         = 192,
    //...
    REMAIN_SRC_DATA_WRITE     = 0x80000000
} SOURCE_LINE_WRITE;
BOOL LoadYuvImageBurstFormat(
    Uint32      coreIdx,
    Uint8*      src,
    size_t      picWidth,
    size_t      picHeight,
    FrameBuffer *fb,
    BOOL        convertCbcrIntl
    );


int ProcessEncodedBitstreamBurst(
    Uint32 core_idx,
    osal_file_t fp,
    int targetAddr,
    PhysicalAddress bsBufStartAddr,
    PhysicalAddress bsBufEndAddr,
    int size,
    int endian,
    Comparator comparator
    );

BOOL LoadTiledImageYuvBurst(
    VpuHandle       handle,
    Uint32          coreIdx,
    BYTE*           pYuv,
    size_t          picWidth,
    size_t          picHeight,
    FrameBuffer*    fb,
    TiledMapConfig  mapCfg
    );

Uint32 StoreYuvImageBurstFormat(
    Uint32          coreIndex,
    FrameBuffer*    fbSrc,
    TiledMapConfig  mapCfg,
    Uint8*          pDst,
    VpuRect         cropRect,
    BOOL            enableCrop
    );

/************************************************************************/
/* Bit Reader                                                           */
/************************************************************************/
#define BS_RESET_BUF 0
#define BS_RING_BUF 1
#define BUFFER_MODE_TYPE_LINEBUFFER     0
#define BUFFER_MODE_TYPE_RINGBUFFER     1
typedef void*   BitstreamReader;
typedef struct BitstreamReaderImpl {
    void*       context;
    BOOL      (*Create)(struct BitstreamReaderImpl* impl, const char* path);
    Uint32  (*Act)(struct BitstreamReaderImpl* impl, Int32   coreIdx, PhysicalAddress bitstreamBuffer, Uint32 bitstreamBufferSize, int endian, Comparator comparator);
    BOOL      (*Destroy)(struct BitstreamReaderImpl* impl);
    BOOL      (*Configure)(struct BitstreamReaderImpl* impl, Uint32 cmd, void* val);
} BitstreamReaderImpl;

/*!
 * \param   type                0: Linebuffer, 1: Ringbuffer
 * \param   path                output filepath.
 * \param   endian              Endianness of bitstream buffer
 * \param   handle              Pointer of encoder handle
 */
BitstreamReader BitstreamReader_Create(
    Uint32    type,
    char*       path,
    EndianMode  endian,
    EncHandle*  handle
    );

/*!
 * \param   bitstreamBuffer     base address of bitstream buffer
 * \param   bitstreamBufferSize size of bitstream buffer
 */
BOOL BitstreamReader_Act(
    BitstreamReader reader,
    PhysicalAddress bitstreamBuffer,
    Uint32        bitstreamBufferSize,
    Uint32        defaultsize,
    Comparator      comparator
    );

BOOL BitstreamReader_Destroy(
    BitstreamReader reader
    );

/************************************************************************/
/* Simple Renderer                                                      */
/************************************************************************/
typedef void*       Renderer;

typedef enum {
    RENDER_DEVICE_NULL,
    RENDER_DEVICE_FBDEV,
    RENDER_DEVICE_HDMI,
    RENDER_DEVICE_MAX
} RenderDeviceType;

typedef struct RenderDevice {
    void*       context;
    DecHandle   decHandle;
    BOOL (*Open)(struct RenderDevice* device);
    void (*Render)(struct RenderDevice* device, DecOutputInfo* fbInfo, Uint8* yuv, Uint32 width, Uint32 height);
    BOOL (*Close)(struct RenderDevice* device);
} RenderDevice;

Renderer SimpleRenderer_Create(
    DecHandle           decHandle,
    RenderDeviceType    deviceType,
    const char*         yuvPath            //!<< path to store yuv iamge.
    );

Uint32 SimpleRenderer_Act(
    Renderer        renderer,
    DecOutputInfo*  fbInfo,
    Uint8*          pYuv,
    Uint32        width,
    Uint32        height
    );

void* SimpleRenderer_GetFreeFrameInfo(
    Renderer        renderer
    );

/* \brief       Flush display queues and clear display indexes
 */
void SimpleRenderer_Flush(
    Renderer        renderer
    );

BOOL SimpleRenderer_Destroy(
    Renderer    renderer
    );

BOOL SimpleRenderer_SetFrameRate(
    Renderer        renderer,
    Uint32          fps
    );


BOOL MkDir(
    char* path
    );
/*******************************************************************************
 * DATATYPES AND FUNCTIONS RELATED TO REPORT
 *******************************************************************************/
typedef struct
{
    osal_file_t     fpPicDispInfoLogfile;
    osal_file_t     fpPicTypeLogfile;
    osal_file_t     fpSeqDispInfoLogfile;
    osal_file_t     fpUserDataLogfile;
    osal_file_t     fpSeqUserDataLogfile;

    // encoder report file
    osal_file_t     fpEncSliceBndInfo;
    osal_file_t     fpEncQpInfo;
    osal_file_t     fpEncmvInfo;
    osal_file_t     fpEncsliceInfo;

    // Report Information
    BOOL            reportOpened;
    Int32           decIndex;
    vpu_buffer_t    vb_rpt;
    BOOL            userDataEnable;
    BOOL            userDataReportMode;

    Int32           profile;
    Int32           level;
} vpu_rpt_info_t;

typedef struct VpuReportConfig_t {
    PhysicalAddress userDataBufAddr;
    BOOL            userDataEnable;
    Int32           userDataReportMode; // (0 : Int32errupt mode, 1 Int32errupt disable mode)
    Int32           userDataBufSize;

} VpuReportConfig_t;

void OpenDecReport(
    Uint32              core_idx,
    VpuReportConfig_t*  cfg
    );

void CloseDecReport(
    DecHandle handle
    );

void ConfigDecReport(
    Uint32      core_idx,
    DecHandle   handle,
    CodStd      bitstreamFormat
    );

void SaveDecReport(
    Uint32          core_idx,
    DecHandle       handle,
    DecOutputInfo*  pDecInfo,
    CodStd          bitstreamFormat,
    Uint32          mbNumX,
    Uint32          mbNumY
    );

void CheckUserDataInterrupt(
    Uint32      core_idx,
    DecHandle   handle,
    Int32       decodeIdx,
    CodStd      bitstreamFormat,
    Int32       int_reason
    );

RetCode VPU_GetFBCOffsetTableSize(
    CodStd  codStd,
    int     width,
    int     height,
    int*     ysize,
    int*     csize
    );
extern Int32 ProductCalculateAuxBufferSize(
    AUX_BUF_TYPE    type,
    CodStd          codStd,
    Int32           width,
    Int32           height
    );

#if   defined(SUPPORT_LOOK_AHEAD_RC)
#define MAX_CFG                 (187)
#else
#define MAX_CFG                 (183)
#endif


#define MAX_ROI_LEVEL           (8)
#define LOG2_CTB_SIZE           (5)
#define CTB_SIZE                (1<<LOG2_CTB_SIZE)
#define LAMBDA_SCALE_FACTOR     (100000)
#define FLOATING_POINT_LAMBDA   (1)
#define TEMP_SCALABLE_RC        (1)
#define UI16_MAX                (0xFFFF)
#ifndef INT_MAX
#define INT_MAX                 (2147483647)
#endif

typedef struct {
    char  *name;
    int    min;
    int    max;
    int    def;
} WaveCfgInfo;

Int32 GetEncOpenParamChange(
    EncOpenParam*   pEncOP,
    char*           cfgFileName,
    ENC_CFG*        pEncCfg,
    EncHandle       handle
    );

void PrintVpuVersionInfo(
    Uint32 coreIdx
    );

void ChangePathStyle(
    char *str
    );

BOOL CalcYuvSize(
    Int32   format,
    Int32   picWidth,
    Int32   picHeight,
    Int32   cbcrInterleave,
    size_t  *lumaSize,
    size_t  *chromaSize,
    size_t  *frameSize,
    Int32   *bitDepth,
    Int32   *packedFormat,
    Int32   *yuv3p4b
    );


FrameBufferFormat GetPackedFormat (
    int srcBitDepth,
    int packedType,
    int p10bits,
    int msb
    );

char* GetDirname(
    const char* path
    );

char* GetBasename(
    const char* pathname
    );

char* GetFileExtension(
    const char* filename
    );

int parseAvcCfgFile(
    ENC_CFG*    pEncCfg,
    char*       filename
    );

int parseMp4CfgFile(
    ENC_CFG*    pEncCfg,
    char*       filename
    );

int parseWaveEncCfgFile(
    ENC_CFG*    pEncCfg,
    char*       FileName,
    int bitFormat
    );

int parseWaveChangeParamCfgFile(
    ENC_CFG*    pEncCfg,
    char*       FileName
    );

int parseRoiCtuModeParam(
    char* lineStr,
    VpuRect* roiRegion,
    int* roiLevel,
    int picX,
    int picY
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

/************************************************************************/
/* Structure                                                            */
/************************************************************************/
typedef struct TestDecConfig_struct {
    char                outputPath[MAX_FILE_PATH];
    char                inputPath[MAX_FILE_PATH];
    Int32               forceOutNum;
    CodStd              bitFormat;
    Int32               reorder;
    TiledMapType        mapType;
    BitStreamMode       bitstreamMode;
    BOOL                enableWTL;
    FrameFlag           wtlMode;
    FrameBufferFormat   wtlFormat;
    Int32               coreIdx;
    ProductId           productId;
    BOOL                enableCrop;                 //!<< option for saving yuv
    BOOL                cbcrInterleave;             //!<< 0: None, 1: NV12, 2: NV21
    BOOL                nv21;                       //!<< FALSE: NV12, TRUE: NV21,
                                                    //!<< This variable is valid when cbcrInterleave is TRUE
    EndianMode          streamEndian;
    EndianMode          frameEndian;
    Uint32              secondaryAXI;
    Int32               compareType;
    char                md5Path[MAX_FILE_PATH];
    char                fwPath[MAX_FILE_PATH];
    char                refYuvPath[MAX_FILE_PATH];
    RenderDeviceType    renderType;
    BOOL                thumbnailMode;
    Int32               skipMode;
    size_t              bsSize;
    BOOL                streamEndFlag;
    Uint32            scaleDownWidth;
    Uint32            scaleDownHeight;
    struct {
        BOOL        enableMvc;                      //!<< H.264 MVC
        BOOL        enableTiled2Linear;
        FrameFlag   tiled2LinearMode;
        BOOL        enableBWB;
        Uint32      rotate;                         //!<< 0, 90, 180, 270
        Uint32      mirror;
        BOOL        enableDering;                   //!<< MPEG-2/4
        BOOL        enableDeblock;                  //!<< MPEG-2/4
        Uint32      mp4class;                       //!<< MPEG_4
        Uint32      frameCacheBypass;
        Uint32      frameCacheBurst;
        Uint32      frameCacheMerge;
        Uint32      frameCacheWayShape;
        LowDelayInfo    lowDelay;                   //!<< H.264
    } coda9;
    struct {
        Uint32      numVCores;                      //!<< This numVCores is valid on PRODUCT_ID_4102 multi-core version
        BOOL        bwOptimization;                 //!<< On/Off bandwidth optimization function
        BOOL        craAsBla;
        Uint32      av1Format;
    } wave;
    Uint32          pfClock;                        //!<< performance clock in Hz
    BOOL            performance;
    BOOL            bandwidth;
    Uint32          fps;
    Uint32          enableUserData;
    /* FEEDER */
    FeedingMethod       feedingMode;
    Uint32              feedingSize;
    Uint32              loopCount;
    BOOL                errorInject;
} TestDecConfig;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void SetDefaultDecTestConfig(
    TestDecConfig* testConfig
    );

void Coda9SetDefaultDecTestConfig(
    TestDecConfig* testConfig
    );

struct option* ConvertOptions(
    struct OptionExt*   cnmOpt,
    Uint32              nItems
    );

void ReleaseVideoMemory(
    DecHandle   handle,
    vpu_buffer_t*   memoryArr,
    Uint32        count
    );

void *AllocateDecFrameBuffer2(
    DecHandle decHandle,
    TestDecConfig* config,
    Uint32 size,
    FrameBuffer* retFbArray,
    vpu_buffer_t* retFbAddrs
    );

BOOL AttachDecDMABuffer(
    DecHandle decHandle,
    TestDecConfig* config,
    Uint64 virtAddress,
    Uint32 size,
    FrameBuffer* retFbArray,
    vpu_buffer_t* retFbAddrs
    );

BOOL AllocateDecFrameBuffer(
    DecHandle       decHandle,
    TestDecConfig*  config,
    Uint32          tiledFbCount,
    Uint32          linearFbCount,
    FrameBuffer*    retFbArray,
    vpu_buffer_t*   retFbAddrs,
#if 0
    vpu_buffer_t*   inFbAddrs,
    Uint32          inFbCount,
#endif
    Uint32*         retStride
    );

BOOL AllocFBMemory(
    Uint32 coreIdx,
    vpu_buffer_t *pFbMem,
    FrameBuffer *pFb,
    Uint32 memSize,
    Uint32 memNum,
    Int32 memTypes,
    Int32 instIndex
    );

BOOL Coda9AllocateDecPPUFrameBuffer(
    BOOL*           pEnablePPU,
    DecHandle       decHandle,
    TestDecConfig*  config,
    FrameBuffer*    retFbArray,
    vpu_buffer_t*   retFbAddrs,
    Queue*          ppuQ
    );

RetCode SetUpDecoderOpenParam(
    DecOpenParam*   param,
    TestDecConfig*  config
    );

#define OUTPUT_FP_NUMBER 4
BOOL OpenDisplayBufferFile(
    CodStd  codec,
    char *outputPath,
    VpuRect rcDisplay,
    TiledMapType mapType,
    FILE *fp[]
    );

void CloseDisplayBufferFile(
    FILE *fp[]
    );

void SaveDisplayBufferToFile(
    DecHandle handle,
    CodStd codStd,
    FrameBuffer dispFrame,
    VpuRect rcDisplay,
    FILE *fp[]
    );


void GetUserData(
    Int32 coreIdx,
    Uint8* pBase,
    vpu_buffer_t vbUserData,
    DecOutputInfo outputInfo
    );

Uint32 CalcScaleDown(
    Uint32 origin,
    Uint32 scaledValue
    );


#ifdef __cplusplus
}
#endif /* __cplusplus */

typedef struct TestEncConfig_struct {
    char    yuvSourceBaseDir[MAX_FILE_PATH];
    char    yuvFileName[MAX_FILE_PATH];
    char    cmdFileName[MAX_FILE_PATH];
    char    bitstreamFileName[MAX_FILE_PATH];
    char    huffFileName[MAX_FILE_PATH];
    char    cInfoFileName[MAX_FILE_PATH];
    char    qMatFileName[MAX_FILE_PATH];
    char    qpFileName[MAX_FILE_PATH];
    char    cfgFileName[MAX_FILE_PATH];
#ifdef SUPPORT_LOOK_AHEAD_RC
    char    cfgFileName_larc_pass1[MAX_FILE_PATH];
#endif
    CodStd  stdMode;
    int     picWidth;
    int     picHeight;
    int     kbps;
    int     rotAngle;
    int     mirDir;
    int     useRot;
    int     qpReport;
    int     ringBufferEnable;
    int     rcIntraQp;
    int     outNum;
    int     skipPicNums[MAX_PIC_SKIP_NUM];
    Uint32     coreIdx;
    TiledMapType mapType;
    // 2D cache option

    int lineBufIntEn;
    int en_container;                   //enable container
    int container_frame_rate;           //framerate for container
    int picQpY;

    int cbcrInterleave;
    int nv21;
    BOOL needSourceConvert;         //!<< If the format of YUV file is YUV planar mode and EncOpenParam::cbcrInterleave or EncOpenParam::nv21 is true
                                    //!<< the value of needSourceConvert should be true.
    int packedFormat;
    FrameBufferFormat srcFormat;
#ifdef SUPPORT_LOOK_AHEAD_RC
    int look_ahead_rc;
#endif
    int bitdepth;
    int secondaryAXI;
    EndianMode stream_endian;
    int frame_endian;
    int source_endian;

    ProductId productId;

    int compareType;
#define YUV_MODE_YUV 0
#define YUV_MODE_YUV_LOADER 2
#define YUV_MODE_CFBC       3
    int yuv_mode;
    char ref_stream_path[MAX_FILE_PATH];
    int loopCount;
    char ref_recon_md5_path[MAX_FILE_PATH];
    BOOL    performance;
    BOOL    bandwidth;
    Uint32  fps;
    Uint32  pfClock;
    char roi_file_name[MAX_FILE_PATH];
    FILE *roi_file;
    int roi_enable;

    int encAUD;
    int encEOS;
    int encEOB;
    struct {
        BOOL        enableLinear2Tiled;
        FrameFlag   linear2TiledMode;
    } coda9;
    int useAsLongtermPeriod;
    int refLongtermPeriod;

    // newly added for encoder
    FILE*  scaling_list_file;
    char   scaling_list_fileName[MAX_FILE_PATH];

    FILE*  custom_lambda_file;
    char   custom_lambda_fileName[MAX_FILE_PATH];
    Uint32 roi_avg_qp;

    FILE*  lambda_map_file;
    Uint32 lambda_map_enable;
    char   lambda_map_fileName[MAX_FILE_PATH];

    FILE*  mode_map_file;
    Uint32 mode_map_flag;
    char   mode_map_fileName[MAX_FILE_PATH];

    FILE*  wp_param_file;
    Uint32 wp_param_flag;
    char   wp_param_fileName[MAX_FILE_PATH];

    Int32  force_picskip_start;
    Int32  force_picskip_end;
    Int32  force_coefdrop_start;
    Int32  force_coefdrop_end;
    Int32  numChangeParam;
    W5ChangeParam   changeParam[10];

    int    forceIdrPicIdx;
    Int32  lowLatencyMode;

    char            optYuvPath[256];
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
    int             srcReleaseIntEnable;
#endif
    int             ringBufferWrapEnable;

#ifdef SUPPORT_LOOK_AHEAD_RC
#define LOOK_AHEAD_RC_PASS1 1
#define LOOK_AHEAD_RC_PASS2 2
    char  optYuvPath_larc_pass1[MAX_FILE_PATH];
    int   larcEnable; //look ahead RC
    int   larcPass; //look ahead RCPass. look ahead Rc is enabled if value is not 0
#endif
} TestEncConfig;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
BOOL SetupEncoderOpenParam(
    EncOpenParam*   param,
    TestEncConfig*  config,
    ENC_CFG*        encConfig
    );

Int32   GetEncOpenParam(
    EncOpenParam*   pEncOP,
    TestEncConfig*  pEncConfig,
    ENC_CFG*        pEncCfg
    );

Int32 GetEncOpenParamDefault(
    EncOpenParam*   pEncOP,
    TestEncConfig*  pEncConfig
    );

void GenRegionToMap(
    VpuRect *region,        /**< The size of the ROI region for H.265 (start X/Y in CTU, end X/Y in CTU)  */
    int *roiLevel,
    int num,
    Uint32 mapWidth,
    Uint32 mapHeight,
    Uint8 *roiCtuMap
    );

int setRoiMap(
    int coreIdx,
    TestEncConfig *encConfig,
    EncOpenParam encOP,
    PhysicalAddress addrRoiMap,
    Uint8 *roiMapBuf,
    int srcFrameWidth,
    int srcFrameHeight,
    EncParam *encParam,
    int maxCtuNum
    );

void setEncBgMode(
    EncParam *encParam,
    TestEncConfig encConfig
    );

void GenRegionToQpMap(
    VpuRect *region,        /**< The size of the ROI region for H.265 (start X/Y in CTU, end X/Y int CTU)  */
    int *roiLevel,
    int num,
    int initQp,
    Uint32 mapWidth,
    Uint32 mapHeight,
    Uint8 *roiCtuMap
    );

void CheckParamRestriction(
    Uint32 productId,
    TestEncConfig *encConfig
    );
int openRoiMapFile(
    TestEncConfig *encConfig
    );
int allocateRoiMapBuf(
    EncHandle handle,
    TestEncConfig encConfig,
    vpu_buffer_t *vbROi,
    int srcFbNum,
    int ctuNum
    );

void SetMapData(
    int coreIdx,
    TestEncConfig encConfig,
    EncOpenParam encOP,
    EncParam *encParam,
    int srcFrameWidth,
    int srcFrameHeight,
    unsigned long addrCustomMap
    );

RetCode SetChangeParam(
    EncHandle handle,
    TestEncConfig encConfig,
    EncOpenParam encOP,
    Int32 changedCount
    );


BOOL GetBitstreamToBuffer(
    EncHandle handle,
    Uint8* pBuffer,
    PhysicalAddress rdAddr,
    PhysicalAddress wrAddr,
    PhysicalAddress streamBufStartAddr,
    PhysicalAddress streamBufEndAddr,
    Uint32 streamSize,
    EndianMode endian,
    BOOL enabledRinbuffer
    );


void SetDefaultEncTestConfig(
    TestEncConfig* testConfig
    );

/************************************************************************/
/* User Parameters (ENCODER)                                            */
/************************************************************************/
// user scaling list
#define SL_NUM_MATRIX (6)

typedef struct
{
    Uint8 s4[SL_NUM_MATRIX][16]; // [INTRA_Y/U/V,INTER_Y/U/V][NUM_COEFF]
    Uint8 s8[SL_NUM_MATRIX][64];
    Uint8 s16[SL_NUM_MATRIX][64];
    Uint8 s32[SL_NUM_MATRIX][64];
    Uint8 s16dc[SL_NUM_MATRIX];
    Uint8 s32dc[2];
}UserScalingList;

enum ScalingListSize
{
    SCALING_LIST_4x4 = 0,
    SCALING_LIST_8x8,
    SCALING_LIST_16x16,
    SCALING_LIST_32x32,
    SCALING_LIST_SIZE_NUM
};

int parse_user_scaling_list(
    UserScalingList* sl,
    FILE* fp_sl,
    CodStd  stdMode
    );

// custom lambda
#define NUM_CUSTOM_LAMBDA   (2*52)
int parse_custom_lambda(Uint32 buf[NUM_CUSTOM_LAMBDA], FILE* fp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _MAIN_HELPER_H_ */

