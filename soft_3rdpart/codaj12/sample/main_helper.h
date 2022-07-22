/*
 * Copyright (c) 2018, Chips&Media
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

#ifndef JPUHELPER_H_INCLUDED
#define JPUHELPER_H_INCLUDED

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "codaj12/jpuapi/jpuapi.h"
    #include "codaj12/sample/helper/jpulog.h"
    #include "codaj12/sample/helper/cnm_fpga.h"
    #include "codaj12/sample/helper/yuv_feeder.h"
#else
    #include "jpulog.h"
    #include "jpuapi.h"
    #include "cnm_fpga.h"
    #include "yuv_feeder.h"
#endif

#define MAX_FILE_PATH    256

typedef struct {
    Uint32       Format;
    Uint32       Index;
    jpu_buffer_t vbY;
    jpu_buffer_t vbCb;
    jpu_buffer_t vbCr;
    Uint32       strideY;
    Uint32       strideC;
} FRAME_BUF;

typedef struct {
    FRAME_BUF frameBuf[MAX_FRAME];
    jpu_buffer_t vb_base;
    int instIndex;
    int last_num;
    int last_addr;
} fb_context;

typedef enum {
    FEEDING_METHOD_FIXED_SIZE,
    FEEDING_METHOD_FRAME_SIZE,      /*!<< use FFMPEG demuxer */
#ifdef USE_FEEDING_METHOD_BUFFER
    FEEDING_METHOD_BUFFER,
#endif
    FEEDING_METHOD_MAX
} FeedingMethod;

typedef enum {
    BSWRITER_ES,
    BSWRITER_CONTAINER,
    BSWRITER_MAX
} BSWriterType;

typedef struct
{
    char            yuvFileName[MAX_FILE_PATH];
    char            bitstreamFileName[MAX_FILE_PATH];
    char            huffFileName[MAX_FILE_PATH];
    char            qMatFileName[MAX_FILE_PATH];
    char            cfgFileName[MAX_FILE_PATH];
    Uint32          picWidth;
    Uint32          picHeight;
    Uint32          mjpgChromaFormat;
    Uint32          mjpgFramerate;
    Uint32          outNum;

    Uint32          StreamEndian;
    Uint32          FrameEndian;
    FrameFormat     sourceSubsample;
    CbCrInterLeave  chromaInterleave;
    PackedFormat    packedFormat;
    Uint32          bEnStuffByte;
    Uint32          encHeaderMode;

    char            strCfgDir[MAX_FILE_PATH];
    char            strYuvDir[MAX_FILE_PATH];

    Uint32          bsSize;
    Uint32          encQualityPercentage;
    Uint32          tiledModeEnable;
    Uint32          sliceHeight;
    Uint32          sliceInterruptEnable;
    BOOL            extendedSequential;
    Uint32          pixelJustification;
    Uint32          rotation;
    JpgMirrorDirection mirror;
    BSWriterType    writerType;
} EncConfigParam;

typedef struct
{
    char            yuvFileName[MAX_FILE_PATH];
    char            bitstreamFileName[MAX_FILE_PATH];
    Uint32          outNum;
    Uint32          checkeos;
    Uint32          StreamEndian;
    Uint32          FrameEndian;
    Uint32          iHorScaleMode;
    Uint32          iVerScaleMode;
    //ROI
    Uint32          roiEnable;
    Uint32          roiWidth;
    Uint32          roiHeight;
    Uint32          roiOffsetX;
    Uint32          roiOffsetY;
    Uint32          roiWidthInMcu;
    Uint32          roiHeightInMcu;
    Uint32          roiOffsetXInMcu;
    Uint32          roiOffsetYInMcu;
    Uint32          rotation;
    JpgMirrorDirection mirror;
    FrameFormat     subsample;
    PackedFormat    packedFormat;
    CbCrInterLeave  cbcrInterleave;
    Uint32          bsSize;
    Uint32          pixelJustification;
    FeedingMethod   feedingMode;
} DecConfigParam;

typedef struct {
    char    SrcFileName[256];
    Uint32  NumFrame;
    Uint32  PicX;
    Uint32  PicY;
    Uint32  FrameRate;

    // MJPEG ONLY
    char    HuffTabName[256];
    char    QMatTabName[256];
    Uint32  VersionID;
    Uint32  FrmFormat;
    FrameFormat SrcFormat;
    Uint32  RstIntval;
    Uint32  ThumbEnable;
    Uint32  ThumbSizeX;
    Uint32  ThumbSizeY;
    Uint32  prec;
    Uint32  QMatPrec0;
    Uint32  QMatPrec1;
} ENC_CFG;


typedef struct {
    Uint32  sourceFormat;
    Uint32  restartInterval;
    BYTE    huffBits[4][256];
    BYTE    huffVal[4][256];
    short   qMatTab[4][64];
    BOOL    lumaQ12bit;
    BOOL    chromaQ12bit;
    BOOL    extendedSequence;      /* 12bit JPEG */
} EncMjpgParam;

typedef enum {
    JPU_ENCODER,
    JPU_DECODER,
    JPU_NONE,
} JPUComponentType;

typedef struct {
    Uint32              numInstances;
    JPUComponentType    type[MAX_NUM_INSTANCE];
    union {
        DecConfigParam  decConfig;
        EncConfigParam  encConfig;
    } u[MAX_NUM_INSTANCE];
    TestDevConfig       devConfig;
} TestMultiConfig;

typedef enum { YUV444, YUV422, YUV420, NV12, NV21, YUV400, YUYV, YVYU, UYVY, VYUY, YYY, RGB_PLANAR, RGB32, RGB24, RGB16 } yuv2rgb_color_format;


#if defined (__cplusplus)
extern "C"{
#endif

extern BOOL TestDecoder(DecConfigParam *param);
extern BOOL TestEncoder(EncConfigParam *param);
extern int jpgGetHuffTable(char *huffFileName, EncMjpgParam *param, int prec);
extern int jpgGetQMatrix(char *qMatFileName, EncMjpgParam *param);
extern int getJpgEncOpenParamDefault(JpgEncOpenParam *pEncOP, EncConfigParam *pEncConfig);
extern BOOL GetJpgEncOpenParam(JpgEncOpenParam *pEncOP, EncConfigParam *pEncConfig);
extern int parseJpgCfgFile(ENC_CFG *pEncCfg, char *FileName);

extern JpgRet ReadJpgBsBufHelper(JpgEncHandle handle,
    FILE *bsFp,
    JpgEncOpenParam *pEncOP,
    JpgEncOutputInfo *pEncOutput);

extern int LoadYuvImageHelperFormat_V20( int prec, FILE *yuvFp,
        Uint8 *pYuv,
        PhysicalAddress addrY,
        PhysicalAddress addrCb,
        PhysicalAddress addrCr,
        int picWidth,
        int picHeight,
        int stride,
        int interleave,
        int format,
        int endian,
        int packed,
        Uint32 justification);

extern int SaveYuvImageHelperFormat_V20(
    FILE*           yuvFp,
    Uint8*          pYuv,
    FrameBuffer*    fb,
    CbCrInterLeave  interLeave,
    PackedFormat    packed,
    Uint32          picWidth,
    Uint32          picHeight,
    Uint32          bitDepth,
    BOOL            tiled2Linear
    );

extern int GetFrameBufSize(int framebufFormat, int picWidth, int picHeight);
extern void GetMcuUnitSize(int format, int *mcuWidth, int *mcuHeight);

//DPBBufSize may not same with FrameBufSize due to format convert rounding
extern int GetDPBBufSize(int framebufFormat, int picWidth, int picHeight, int picWidth_C, int interleave);
extern BOOL ParseDecTestLongArgs(void* config, const char* argName, char* value);
extern BOOL ParseEncTestLongArgs(void* config, const char* argName, char* value);
extern BOOL ParseMultiLongOptions(TestMultiConfig* config, const char* argName, char* value);
extern BOOL AllocateFrameBuffer(Uint32 instIdx, FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                                Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth, Uint32 num);
extern void FreeFrameBuffer(int instIdx);
extern FRAME_BUF *GetFrameBuffer(int instIdx, int index);
extern int GetFrameBufBase(int instIdx);
extern int GetFrameBufAllocSize(int instIdx);
extern FRAME_BUF* FindFrameBuffer(int instIdx, PhysicalAddress addrY);

/* --------------------------------------------------------------------------
 * BS feeder
   -------------------------------------------------------------------------- */
typedef struct {
    void*   data;
    Uint32  size;
    BOOL    eos;        //!<< End of stream
} BSChunk;

typedef void* BSFeeder;

extern BSFeeder BitstreamFeeder_Create(const char* path, FeedingMethod method, EndianMode endian);
extern Uint32 BitstreamFeeder_Act(BSFeeder feeder, JpgDecHandle handle, jpu_buffer_t* bsBuffer);
extern BOOL BitstreamFeeder_Destroy(BSFeeder feeder);
extern BOOL BitstreamFeeder_IsEos(BSFeeder feeder);

/* --------------------------------------------------------------------------
 * BS writer
   -------------------------------------------------------------------------- */
typedef void* BSWriter;

typedef struct BitstreamWriterImpl {
    void*   context;
    BOOL    (*Create)(struct BitstreamWriterImpl* impl, EncConfigParam* config, const char* path);
    Uint32  (*Act)(struct BitstreamWriterImpl* impl, Uint8* es, Uint32 size);
    BOOL    (*Destroy)(struct BitstreamWriterImpl* impl);
} BitstreamWriterImpl;

extern BSWriter BitstreamWriter_Create(BSWriterType type, EncConfigParam* config, const char* path);
extern BOOL BitstreamWriter_Act(BSWriter writer, Uint8* es, Uint32 size, BOOL delayedWrite);
extern void BitstreamWriter_Destroy(BSWriter writer);

/* --------------------------------------------------------------------------
 * String
   -------------------------------------------------------------------------- */
extern char* GetFileExtension(const char* filename);

#if defined (__cplusplus)
}
#endif

#endif /* JPUHELPER_H_INCLUDED */
