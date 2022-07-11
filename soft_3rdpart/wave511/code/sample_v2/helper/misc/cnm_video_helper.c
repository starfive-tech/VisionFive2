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

#include "main_helper.h"
#include <libavformat/avformat.h>
// include in the ffmpeg header
typedef struct {
    CodStd      codStd;
    Uint32      mp4Class;
    Uint32      codecId;
    Uint32      fourcc;
} CodStdTab;

#ifndef MKTAG
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#endif

static const CodStdTab codstd_tab[] = {
    { STD_AVC,          0, AV_CODEC_ID_H264,            MKTAG('H', '2', '6', '4') },
    { STD_AVC,          0, AV_CODEC_ID_H264,            MKTAG('X', '2', '6', '4') },
    { STD_AVC,          0, AV_CODEC_ID_H264,            MKTAG('A', 'V', 'C', '1') },
    { STD_AVC,          0, AV_CODEC_ID_H264,            MKTAG('V', 'S', 'S', 'H') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('H', '2', '6', '3') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('X', '2', '6', '3') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('T', '2', '6', '3') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('L', '2', '6', '3') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('V', 'X', '1', 'K') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('Z', 'y', 'G', 'o') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('H', '2', '6', '3') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('I', '2', '6', '3') },    /* intel h263 */
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('H', '2', '6', '1') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('U', '2', '6', '3') },
    { STD_H263,         0, AV_CODEC_ID_H263,            MKTAG('V', 'I', 'V', '1') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('F', 'M', 'P', '4') },
    { STD_MPEG4,        5, AV_CODEC_ID_MPEG4,           MKTAG('D', 'I', 'V', 'X') },    // DivX 4
    { STD_MPEG4,        1, AV_CODEC_ID_MPEG4,           MKTAG('D', 'X', '5', '0') },
    { STD_MPEG4,        2, AV_CODEC_ID_MPEG4,           MKTAG('X', 'V', 'I', 'D') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('M', 'P', '4', 'S') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('M', '4', 'S', '2') },    //MPEG-4 version 2 simple profile
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG( 4 ,  0 ,  0 ,  0 ) },    /* some broken avi use this */
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('D', 'I', 'V', '1') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('B', 'L', 'Z', '0') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('M', 'P', '4', 'V') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('U', 'M', 'P', '4') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('W', 'V', '1', 'F') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('S', 'E', 'D', 'G') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('R', 'M', 'P', '4') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('3', 'I', 'V', '2') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('F', 'F', 'D', 'S') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('F', 'V', 'F', 'W') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('D', 'C', 'O', 'D') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('M', 'V', 'X', 'M') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('P', 'M', '4', 'V') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('S', 'M', 'P', '4') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('D', 'X', 'G', 'M') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('V', 'I', 'D', 'M') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('M', '4', 'T', '3') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('G', 'E', 'O', 'X') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('H', 'D', 'X', '4') }, /* flipped video */
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('D', 'M', 'K', '2') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('D', 'I', 'G', 'I') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('I', 'N', 'M', 'C') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('E', 'P', 'H', 'V') }, /* Ephv MPEG-4 */
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('E', 'M', '4', 'A') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('M', '4', 'C', 'C') }, /* Divio MPEG-4 */
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('S', 'N', '4', '0') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('V', 'S', 'P', 'X') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('U', 'L', 'D', 'X') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('G', 'E', 'O', 'V') },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG('S', 'I', 'P', 'P') }, /* Samsung SHR-6040 */
    { STD_DIV3,         0, AV_CODEC_ID_MSMPEG4V3,       MKTAG('D', 'I', 'V', '3') }, /* default signature when using MSMPEG4 */
    { STD_DIV3,         0, AV_CODEC_ID_MSMPEG4V3,       MKTAG('M', 'P', '4', '3') },
    { STD_DIV3,         0, AV_CODEC_ID_MSMPEG4V3,       MKTAG('M', 'P', 'G', '3') },
    { STD_MPEG4,        1, AV_CODEC_ID_MSMPEG4V3,       MKTAG('D', 'I', 'V', '5') },
    { STD_MPEG4,        1, AV_CODEC_ID_MSMPEG4V3,       MKTAG('D', 'I', 'V', '6') },
    { STD_MPEG4,        5, AV_CODEC_ID_MSMPEG4V3,       MKTAG('D', 'I', 'V', '4') },
    { STD_DIV3,         0, AV_CODEC_ID_MSMPEG4V3,       MKTAG('D', 'V', 'X', '3') },
    { STD_DIV3,         0, AV_CODEC_ID_MSMPEG4V3,       MKTAG('A', 'P', '4', '1') },    //Another hacked version of Microsoft's MP43 codec.
    { STD_MPEG4,        0, AV_CODEC_ID_MSMPEG4V3,       MKTAG('C', 'O', 'L', '1') },
    { STD_MPEG4,        0, AV_CODEC_ID_MSMPEG4V3,       MKTAG('C', 'O', 'L', '0') },    // not support ms mpeg4 v1, 2
    { STD_MPEG4,      256, AV_CODEC_ID_FLV1,            MKTAG('F', 'L', 'V', '1') }, /* Sorenson spark */
    { STD_VC1,          0, AV_CODEC_ID_WMV1,            MKTAG('W', 'M', 'V', '1') },
    { STD_VC1,          0, AV_CODEC_ID_WMV2,            MKTAG('W', 'M', 'V', '2') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG1VIDEO,      MKTAG('M', 'P', 'G', '1') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG1VIDEO,      MKTAG('M', 'P', 'G', '2') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('M', 'P', 'G', '2') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('M', 'P', 'E', 'G') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG1VIDEO,      MKTAG('M', 'P', '2', 'V') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG1VIDEO,      MKTAG('P', 'I', 'M', '1') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('P', 'I', 'M', '2') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG1VIDEO,      MKTAG('V', 'C', 'R', '2') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG1VIDEO,      MKTAG( 1 ,  0 ,  0 ,  16) },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG( 2 ,  0 ,  0 ,  16) },
    { STD_MPEG4,        0, AV_CODEC_ID_MPEG4,           MKTAG( 4 ,  0 ,  0 ,  16) },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('D', 'V', 'R', ' ') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('M', 'M', 'E', 'S') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('L', 'M', 'P', '2') }, /* Lead MPEG2 in avi */
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('S', 'L', 'I', 'F') },
    { STD_MPEG2,        0, AV_CODEC_ID_MPEG2VIDEO,      MKTAG('E', 'M', '2', 'V') },
    { STD_VC1,          0, AV_CODEC_ID_WMV3,            MKTAG('W', 'M', 'V', '3') },
    { STD_VC1,          0, AV_CODEC_ID_VC1,             MKTAG('W', 'V', 'C', '1') },
    { STD_VC1,          0, AV_CODEC_ID_VC1,             MKTAG('W', 'M', 'V', 'A') },

    { STD_RV,           0, AV_CODEC_ID_RV30,            MKTAG('R','V','3','0') },
    { STD_RV,           0, AV_CODEC_ID_RV40,            MKTAG('R','V','4','0') },

    { STD_AVS,          0, AV_CODEC_ID_CAVS,            MKTAG('C','A','V','S') },
    { STD_AVS,          0, AV_CODEC_ID_AVS,             MKTAG('A','V','S','2') },
    { STD_VP3,          0, AV_CODEC_ID_VP3,             MKTAG('V', 'P', '3', '0') },
    { STD_VP3,          0, AV_CODEC_ID_VP3,             MKTAG('V', 'P', '3', '1') },
    { STD_THO,          0, AV_CODEC_ID_THEORA,          MKTAG('T', 'H', 'E', 'O') },
    { STD_VP8,          0, AV_CODEC_ID_VP8,             MKTAG('V', 'P', '8', '0') },
    { STD_VP9,          0, AV_CODEC_ID_VP9,             MKTAG('V', 'P', '9', '0') },
    //  { STD_VP6,              0, AV_CODEC_ID_VP6,             MKTAG('V', 'P', '6', '0') },
    //  { STD_VP6,              0, AV_CODEC_ID_VP6,             MKTAG('V', 'P', '6', '1') },
    //  { STD_VP6,              0, AV_CODEC_ID_VP6,             MKTAG('V', 'P', '6', '2') },
    //  { STD_VP6,              0, AV_CODEC_ID_VP6F,            MKTAG('V', 'P', '6', 'F') },
    //  { STD_VP6,              0, AV_CODEC_ID_VP6F,            MKTAG('F', 'L', 'V', '4') },
    { STD_HEVC,         0, AV_CODEC_ID_HEVC,            MKTAG('H', 'E', 'V', 'C') },
    { STD_HEVC,         0, AV_CODEC_ID_HEVC,            MKTAG('H', 'E', 'V', '1') },
    { STD_HEVC,         0, AV_CODEC_ID_HEVC,            MKTAG('H', 'V', 'C', '1') },
    { STD_HEVC,         0, AV_CODEC_ID_HEVC,            MKTAG('h', 'e', 'v', 'c') },
    { STD_HEVC,         0, AV_CODEC_ID_HEVC,            MKTAG('h', 'e', 'v', '1') },
    { STD_HEVC,         0, AV_CODEC_ID_HEVC,            MKTAG('h', 'v', 'c', '1') }
};

static BOOL CalcYuvSize_412(
    Int32           format,
    Int32           picWidth,
    Int32           picHeight,
    Int32           cbcrInterleave,
    size_t          *lumaSize,
    size_t          *chromaSize,
    size_t          *frameSize,
    Int32           *bitDepth,
    Int32           *packedFormat,
    Int32           *yuv3p4b);

static void GeneratePicParam(
    FrameBuffer*    fbSrc,
    VpuRect         cropRect,
    BOOL            enableCrop,
    Uint32*         is422,
    Uint32*         isPack,
    Uint32*         PackMode,
    Uint32*         is10bit,
    Uint32*         isMSB,
    Uint32*         is3pxl4byte,
    Uint32*         srcWidthY,
    Uint32*         srcHeightY,
    Uint32*         srcWidthC,
    Uint32*         srcHeightC,
    Uint32*         chroma_stride,
    Uint32*         dstWidthY,
    Uint32*         dstHeightY,
    Uint32*         dstWidthC,
    Uint32*         dstHeightC);

static void LoadSrcYUV(
    Uint32          coreIdx,
    TiledMapConfig  mapCfg,
    Uint8*          pSrc,
    FrameBuffer*    fbSrc,
    VpuRect         cropRect,
    BOOL            enableCrop,
    Uint32          is10bit,
    Uint32          is3pxl4byte,
    Uint32          isMSB,
    Uint32          srcWidthY,
    Uint32          srcHeightY,
    Uint32          srcWidthC,
    Uint32          srcHeightC,
    Uint32          chroma_stride);

static void DePackedYUV(
    Uint32          is10bit,
    Uint32          PackMode,
    Uint32          srcWidthY,
    Uint32          srcHeightY,
    Uint8*          pSrc,
    Uint32          dstWidthY,
    Uint32          dstHeightY,
    Uint32          dstWidthC,
    Uint32          dstHeightC,
    Uint8*          pDst);

static void Convert422to420(
    Uint8*          pSrc,
    Uint32          dstWidthY,
    Uint32          dstHeightY,
    Uint32          dstWidthC,
    Uint32          dstHeightC,
    Uint8*          pDst);

static void DeFormatYUV(
    Uint32          is3pixel4byte,
    Uint32          isMSB,
    Uint32          interleave,
    Uint32          srcWidthY,
    Uint32          srcHeightY,
    Uint32          srcWidthC,
    Uint32          srcHeightC,
    Uint32          dstWidthY,
    Uint32          dstWidthC,
    Uint8*          pSrc,
    Uint8*          pDst);

static void ByteSwap10bit(
    Uint32          DstSize,
    Uint8*          pSrc,
    Uint8*          pDst);

static void DeInterLeave(
    Uint32          is10bit,
    BOOL            nv21,
    Uint32          srcWidthY,
    Uint32          srcHeightY,
    Uint32          srcWidthC,
    Uint32          srcHeightC,
    Uint8*          pSrc,
    Uint8*          pDst);

Int32 ConvFOURCCToMp4Class(
    Int32   fourcc
    )
{
    Uint32  i;
    Int32   mp4Class = -1;
    unsigned char str[5];

    str[0] = toupper((Int32)fourcc);
    str[1] = toupper((Int32)(fourcc>>8));
    str[2] = toupper((Int32)(fourcc>>16));
    str[3] = toupper((Int32)(fourcc>>24));
    str[4] = '\0';

    for(i=0; i<sizeof(codstd_tab)/sizeof(codstd_tab[0]); i++) {
        if (codstd_tab[i].fourcc == (Int32)MKTAG(str[0], str[1], str[2], str[3]) ) {
            mp4Class = codstd_tab[i].mp4Class;
            break;
        }
    }

    return mp4Class;
}

Int32 ConvFOURCCToCodStd(
    Uint32   fourcc
    )
{
    Int32   codStd = -1;
    Uint32  i;

    char str[5];

    str[0] = toupper((Int32)fourcc);
    str[1] = toupper((Int32)(fourcc>>8));
    str[2] = toupper((Int32)(fourcc>>16));
    str[3] = toupper((Int32)(fourcc>>24));
    str[4] = '\0';

    for(i=0; i<sizeof(codstd_tab)/sizeof(codstd_tab[0]); i++) {
        if (codstd_tab[i].fourcc == (Uint32)MKTAG(str[0], str[1], str[2], str[3])) {
            codStd = codstd_tab[i].codStd;
            break;
        }
    }

    return codStd;
}

Int32 ConvCodecIdToMp4Class(
    Uint32   codecId
    )
{
    Int32   mp4Class = -1;
    Uint32  i;

    for(i=0; i<sizeof(codstd_tab)/sizeof(codstd_tab[0]); i++) {
        if (codstd_tab[i].codecId == codecId) {
            mp4Class = codstd_tab[i].mp4Class;
            break;
        }
    }

    return mp4Class;
}

Int32 ConvCodecIdToCodStd(
    Int32   codecId
    )
{
    Int32   codStd = -1;
    Uint32  i;

    for(i=0; i<sizeof(codstd_tab)/sizeof(codstd_tab[0]); i++) {
        if (codstd_tab[i].codecId == codecId) {
            codStd = codstd_tab[i].codStd;
            break;
        }
    }

    return codStd;
}

Int32 ConvCodecIdToFourcc(
    Int32   codecId
    )
{
    Int32   fourcc = 0;
    Uint32   i;

    for(i=0; i<sizeof(codstd_tab)/sizeof(codstd_tab[0]); i++) {
        if (codstd_tab[i].codecId == codecId) {
            fourcc = codstd_tab[i].fourcc;
            break;
        }
    }
    return fourcc;
}



BOOL LoadYuvImageByYCbCrLine(
    EncHandle   handle,
    Uint32      coreIdx,
    Uint8*      src,
    size_t      picWidth,
    size_t      picHeight,
    FrameBuffer* fb,
    Uint32      srcFbIndex
    )
{
    Int32                y, nY, nCb;
    PhysicalAddress      addrY, addrCb, addrCr;
    size_t               lumaSize, chromaSize=0, chromaStride = 0, chromaWidth=0;
    Uint8                *srcY, *srcCb, *srcCr;
    size_t               stride      = fb->stride;
    EndianMode           endian      = (EndianMode)fb->endian;
    FrameBufferFormat    format      = fb->format;
    BOOL                 interLeave  = fb->cbcrInterleave;
    int                  twice       = 1 << interLeave;
    int                  cnt=0;
    Uint32               YStartPos=0;

    switch (format) {
    case FORMAT_420:
        nY = picHeight;
        nCb = picHeight / 2;
        chromaSize = picWidth * picHeight / 4;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    case FORMAT_224:
        nY = picHeight;
        nCb = picHeight / 2;
        chromaSize = picWidth * picHeight / 2;
        chromaStride = stride;
        chromaWidth = picWidth;
        break;
    case FORMAT_422:
        nY = picHeight;
        nCb = picHeight;
        chromaSize = picWidth * picHeight / 2;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    case FORMAT_444:
        nY = picHeight;
        nCb = picHeight;
        chromaSize = picWidth * picHeight;
        chromaStride = stride;
        chromaWidth = picWidth;
        break;
    case FORMAT_400:
        nY = picHeight;
        nCb = 0;
        chromaSize = picWidth * picHeight / 4;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
    case FORMAT_YUYV_P10_16BIT_MSB:
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        nY = picHeight;
        nCb = 0;
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        nY = picHeight;
        nCb = picHeight/2;
        chromaSize = picWidth * picHeight/2;
        chromaStride = stride / 2;
        chromaWidth = picWidth;
        picWidth *= 2;
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
        nY = picHeight;
        nCb = picHeight/2;
        picWidth = VPU_ALIGN32(picWidth);
        chromaWidth = ((VPU_ALIGN16(picWidth/2*twice)+2)/3*4);
        chromaStride = VPU_ALIGN16(stride/2)*twice;
        if ( interLeave == 1)
            chromaStride = stride;
        chromaSize = chromaWidth * picHeight/2;
        picWidth   = ((VPU_ALIGN16(picWidth)+2)/3)*4;
        break;
    default:
        nY = picHeight;
        nCb = picHeight / 2;
        chromaSize = picWidth * picHeight / 4;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    }

    lumaSize = picWidth * picHeight;

    srcY  = src;
    srcCb = src + lumaSize;
    srcCr = src + lumaSize + chromaSize;

    addrY  = fb->bufY;
    addrCb = fb->bufCb;
    addrCr = fb->bufCr;

    if ( nCb )
        cnt = nY / nCb;


    for (y = YStartPos; y < nY; ++y) {
        vdi_write_memory(coreIdx, addrY + stride * y, (Uint8 *)(srcY + y * picWidth), picWidth, endian);
        if (format == FORMAT_400) {
            continue;
        }
        if (format >= FORMAT_YUYV && format <= FORMAT_VYUY_P10_32BIT_LSB) {
            continue;
        }
        if (interLeave == TRUE) {
            if (cnt == 2 && ((y%2)==0))
                vdi_write_memory(coreIdx, addrCb + stride * y/cnt, (Uint8 *)(srcCb+ y/cnt * picWidth), picWidth, endian);
        }
        else {
            if (cnt == 2 && ((y%2)==0)) {
                vdi_write_memory(coreIdx, addrCb + chromaStride * y/cnt, (Uint8 *)(srcCb + y/cnt * chromaWidth), chromaWidth, endian);
                vdi_write_memory(coreIdx, addrCr + chromaStride * y/cnt, (Uint8 *)(srcCr + y/cnt * chromaWidth), chromaWidth, endian);
            }
        }
    }

    return TRUE;//lint !e438
}

//////////////////// DRAM Read/Write helper Function ////////////////////////////
BOOL LoadYuvImageBurstFormat(
    Uint32      coreIdx,
    Uint8*      src,
    size_t      picWidth,
    size_t      picHeight,
    FrameBuffer* fb,
    BOOL        convertCbcrIntl
    )
{
    Int32               y, nY, nCb, nCr;
    PhysicalAddress     addr;
    size_t              lumaSize, chromaSize=0, chromaStride = 0, chromaWidth=0;
    Uint8*              puc;
    size_t              stride      = fb->stride;
    EndianMode          endian      = (EndianMode)fb->endian;
    FrameBufferFormat   format      = fb->format;
    BOOL                interLeave  = fb->cbcrInterleave;
    int                 twice       = 1 << interLeave;

    switch (format) {
    case FORMAT_420:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = picWidth * picHeight / 4;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    case FORMAT_224:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = picWidth * picHeight / 2;
        chromaStride = stride;
        chromaWidth = picWidth;
        break;
    case FORMAT_422:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = picWidth * picHeight / 2;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    case FORMAT_444:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = picWidth * picHeight;
        chromaStride = stride;
        chromaWidth = picWidth;
        break;
    case FORMAT_400:
        nY = picHeight;
        nCb = nCr = 0;
        chromaSize = picWidth * picHeight / 4;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
    case FORMAT_YUYV_P10_16BIT_MSB:
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        nY = picHeight;
        nCb = nCr = 0;
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        nY = picHeight;
        nCb = nCr = picHeight/2;
        chromaSize = picWidth * picHeight/2;
        chromaStride = stride / 2;
        chromaWidth = picWidth;
        picWidth *= 2;
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
        nY = picHeight;
        nCb = nCr = picHeight/2;
        picWidth = VPU_ALIGN32(picWidth);
        chromaWidth = ((VPU_ALIGN16(picWidth/2*twice)+2)/3*4);
        chromaStride = VPU_ALIGN16(stride/2)*twice;
        if ( interLeave == 1)
            chromaStride = stride;
        chromaSize = chromaWidth * picHeight/2;
        picWidth   = ((VPU_ALIGN16(picWidth)+2)/3)*4;
        break;
    default:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = picWidth * picHeight / 4;
        chromaStride = stride / 2;
        chromaWidth = picWidth / 2;
        break;
    }

    puc = src;
    addr = fb->bufY;
    lumaSize = picWidth * picHeight;

    if( picWidth == stride) { // for fast write
        vdi_write_memory(coreIdx, addr, (Uint8 *)( puc ), lumaSize, endian);

        if( format == FORMAT_400)
            return FALSE;
        if (format >= FORMAT_YUYV && format <= FORMAT_VYUY_P10_32BIT_LSB)
            return FALSE;

        if (interLeave == TRUE) {
            UNREFERENCED_PARAMETER(convertCbcrIntl);
            puc = src + lumaSize;
            addr = fb->bufCb;
            vdi_write_memory(coreIdx, addr, (Uint8 *)puc, chromaSize*2, endian);
        }
        else {
            if ( chromaWidth == chromaStride )
            {
                puc = src + lumaSize;
                addr = fb->bufCb;
                vdi_write_memory(coreIdx, addr, (Uint8 *)puc, chromaSize, endian);

                puc = src + lumaSize + chromaSize;
                addr = fb->bufCr;
                vdi_write_memory(coreIdx, addr, (Uint8 *)puc, chromaSize, endian);
            }
            else
            {
                puc = src + lumaSize;
                addr = fb->bufCb;
                for (y = 0; y < nCb; ++y) {
                    vdi_write_memory(coreIdx, addr + chromaStride * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth, endian);
                }

                puc = src + lumaSize + chromaSize;
                addr = fb->bufCr;
                for (y = 0; y < nCr; ++y) {
                    vdi_write_memory(coreIdx, addr + chromaStride * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth, endian);
                }
            }
        }
    }
    else {
        for (y = 0; y < nY; ++y) {
            vdi_write_memory(coreIdx, addr + stride * y, (Uint8 *)(puc + y * picWidth), picWidth, endian);
        }

        if (format == FORMAT_400) {
            return FALSE;
        }
        if (format >= FORMAT_YUYV && format <= FORMAT_VYUY_P10_32BIT_LSB) {
            return FALSE;
        }

        if (interLeave == TRUE) {
            UNREFERENCED_PARAMETER(convertCbcrIntl);
            puc = src + lumaSize;
            addr = fb->bufCb;
            for (y = 0; y < nCb; ++y) {
                vdi_write_memory(coreIdx, addr + stride * y, (Uint8 *)(puc + y * picWidth), picWidth, endian);
            }
        }
        else {
            puc = src + lumaSize;
            addr = fb->bufCb;
            for (y = 0; y < nCb; ++y) {
                vdi_write_memory(coreIdx, addr + chromaStride * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth, endian);
            }

            puc = src + lumaSize + chromaSize;
            addr = fb->bufCr;
            for (y = 0; y < nCr; ++y) {
                vdi_write_memory(coreIdx, addr + chromaStride * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth, endian);
            }
        }
    }

    return TRUE;//lint !e438
}

BOOL LoadTiledImageYuvBurst(
    VpuHandle       handle,
    Uint32          coreIdx,
    BYTE*           pYuv,
    size_t          picWidth,
    size_t          picHeight,
    FrameBuffer*    fb,
    TiledMapConfig  mapCfg
    )
{
    BYTE *pSrc;
    size_t              divX, divY;
    PhysicalAddress     pix_addr;
    size_t              rrow, ccol;
    size_t              offsetX,offsetY;
    size_t              stride_c;
    size_t              stride      = fb->stride;
    EndianMode          endian      = (EndianMode)fb->endian;
    FrameBufferFormat   format      = fb->format;
    BOOL                interLeave  = fb->cbcrInterleave;
    Int32               productId;
    Int32               dramBusWidth = 8;

    productId = VPU_GetProductId(coreIdx);
    if (PRODUCT_ID_W_SERIES(productId)) {
        dramBusWidth = 16;
    }

    offsetX = offsetY    = 0;

    divX = format == FORMAT_420 || format == FORMAT_422 ? 2 : 1;
    divY = format == FORMAT_420 || format == FORMAT_224 ? 2 : 1;

    switch (format) {
    case FORMAT_400:
        stride_c = 0;
        break;
    case FORMAT_420:
    case FORMAT_422:
        stride_c = stride / 2;
        break;
    case FORMAT_224:
    case FORMAT_444:
        stride_c = stride;
        break;
    default:
        stride_c = stride / 2;
        break;
    }

    // Y
    pSrc    = pYuv;

    // no opt code
    for (rrow=0; rrow <picHeight; rrow=rrow+1)
    {
        for (ccol=0; ccol<picWidth; ccol=ccol+dramBusWidth)
        {
            pix_addr = GetXY2AXIAddr(&mapCfg, 0/*luma*/, rrow +offsetY, ccol + offsetX, stride, fb);
            vdi_write_memory(coreIdx, pix_addr, pSrc+rrow*picWidth+ccol, 8, endian);
        }
    }

    if (format == FORMAT_400) {
        return 1;
    }

    if (interLeave == FALSE) {
        // CB
        pSrc = pYuv + picWidth*picHeight;

        for (rrow=0; rrow <(picHeight/divY) ; rrow=rrow+1) {
            for (ccol=0; ccol<(picWidth/divX); ccol=ccol+dramBusWidth) {
                pix_addr = GetXY2AXIAddr(&mapCfg, 2, rrow + offsetY, ccol +offsetX, stride_c, fb);
                vdi_write_memory(coreIdx, pix_addr, pSrc+rrow*picWidth/divX+ccol, 8, endian);
            }
        }
        // CR

        pSrc = pYuv + picWidth*picHeight+ (picWidth/divX)*(picHeight/divY);

        for (rrow=0; rrow <picHeight/divY ; rrow=rrow+1) {
            for (ccol=0; ccol<picWidth/divX; ccol=ccol+dramBusWidth) {
                pix_addr = GetXY2AXIAddr(&mapCfg, 3, rrow  + offsetY ,ccol +offsetX, stride_c, fb);
                vdi_write_memory(coreIdx, pix_addr, pSrc+rrow*picWidth/divX+ccol, 8, endian);
            }
        }
    }
    else {

        BYTE * pTemp;
        BYTE * srcAddrCb;
        BYTE * srcAddrCr;

        size_t  cbcr_x;

        switch( format) {
        case FORMAT_444 :
            cbcr_x = picWidth*2;
            break;
        case FORMAT_420 :
            cbcr_x = picWidth  ;
            break;
        case FORMAT_422 :
            cbcr_x = picWidth  ;
            break;
        case FORMAT_224 :
            cbcr_x = picWidth*2;
            break;
        default:
            cbcr_x = picWidth  ;
            break;
        }

        stride = stride_c * 2;

        srcAddrCb = pYuv + picWidth*picHeight;
        srcAddrCr = pYuv + picWidth*picHeight + picWidth/divX*picHeight/divY;


        pTemp = (BYTE*)osal_malloc(sizeof(char)*8);
        if (!pTemp) {
            return FALSE;
        }

        for (rrow=0; rrow <picHeight/divY; rrow=rrow+1) {
            for (ccol=0; ccol<cbcr_x ; ccol=ccol+dramBusWidth) {

                pTemp[0  ] = *srcAddrCb++;
                pTemp[0+2] = *srcAddrCb++;
                pTemp[0+4] = *srcAddrCb++;
                pTemp[0+6] = *srcAddrCb++;
                pTemp[0+1] = *srcAddrCr++;
                pTemp[0+3] = *srcAddrCr++;
                pTemp[0+5] = *srcAddrCr++;
                pTemp[0+7] = *srcAddrCr++;

                pix_addr = GetXY2AXIAddr(&mapCfg, 2, rrow + offsetY ,ccol + (offsetX*2), stride, fb);
                vdi_write_memory(coreIdx, pix_addr, (unsigned char *)pTemp, 8, endian);
            }
        }
        osal_free(pTemp);
    }

    return TRUE;
}


static void SwapDword(unsigned char* data, int len)
{
    Uint32  temp;
    Uint32* ptr = (Uint32*)data;
    Int32   i, size = len/sizeof(Uint32);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

static void SwapLword(unsigned char* data, int len)
{
    Uint64  temp;
    Uint64* ptr = (Uint64*)data;
    Int32   i, size = len/sizeof(Uint64);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

static void SwapPixelOrder(
    Uint8*      data
    )
{
    Uint32*     temp;
    Uint32      temp2[4]={0,};
    Int32       i,j;

    for (i=0, j=3 ; i < 16 ; i+=4, j--) {
        temp = (Uint32*)(data+i);
        temp2[j] =  (*temp & 0xffc00000)>>20;
        temp2[j] |= (*temp & 0x003ff000);
        temp2[j] |= (*temp & 0x00000ffc)<<20;
    }

    osal_memcpy(data, temp2, 16);

//for matching with Ref-C
    SwapDword(data, 16);
    SwapLword(data, 16);
}

Uint32 StoreYuvImageBurst(
    Uint32          coreIdx,
    FrameBuffer     *fbSrc,
    TiledMapConfig  mapCfg,
    Uint8           *pDst,
    VpuRect         cropRect,
    BOOL            enableCrop
    )
{
    int      interLeave  = fbSrc->cbcrInterleave;
    BOOL     nv21        = fbSrc->nv21;
    Uint32   totSize = 0;
    Uint32   is422;
    Uint32   isPack;
    Uint32   PackMode;
    Uint32   is10bit;
    Uint32   isMSB;
    Uint32   is3pxl4byte;

    Uint32   srcWidthY;
    Uint32   srcHeightY;
    Uint32   srcWidthC;
    Uint32   srcHeightC;
    Uint32   chroma_stride;

    Uint32   dstWidthY;
    Uint32   dstHeightY;
    Uint32   dstWidthC;
    Uint32   dstHeightC;

    Uint32   DstSize = 0;
    Uint32   SrcSize = 0;
    Uint8*   pSrc = NULL;

    Uint8*   pSrcFormat;
    Uint8*   pDstFormat = NULL;
    Uint32   deFormatSize = 0;
    Uint8*   pFormat = NULL;

    Uint8*   pSrcInterLeave;
    Uint8*   pDstInterLeave = NULL;
    Uint32   InterLeaveSize = 0;
    Uint8*   pInterLeave = NULL;

    Uint8*   pSrcPack;
    Uint8*   pDstPack = NULL;
    Uint32   PackSize = 0;
    Uint8*   pPack = NULL;

    Uint8*   pSrc422;
    Uint8*   pDst422 = NULL;

    Uint8*   pSrcSwap;
    Uint8*   pDstSwap;

    GeneratePicParam(fbSrc, cropRect, enableCrop,
        &is422, &isPack, &PackMode, &is10bit, &isMSB, &is3pxl4byte,
        &srcWidthY, &srcHeightY, &srcWidthC, &srcHeightC, &chroma_stride,
        &dstWidthY, &dstHeightY, &dstWidthC, &dstHeightC);

     //1. Source YUV memory allocation
    SrcSize = srcWidthY*srcHeightY + srcWidthC*srcHeightC;
    if (interLeave != TRUE)
        SrcSize += srcWidthC*srcHeightC;
    pSrc = (Uint8*)osal_malloc(SrcSize);
    if (!pSrc)
        return 0;

    //2. Destination YUV size
    DstSize = dstWidthY*dstHeightY + dstWidthC*dstHeightC * 2;
    totSize = DstSize;

    //3. Source YUV Load
    LoadSrcYUV(coreIdx, mapCfg, pSrc, fbSrc, cropRect, enableCrop,
        is10bit, is3pxl4byte, isMSB,
        srcWidthY, srcHeightY, srcWidthC, srcHeightC, chroma_stride);

    //4.    osal_memcpy and termination for 8bit, 420, normal(non packed) YUV
    //SrcYUV => DstYUV
#if defined(SAVE_YUV)
    osal_memcpy(pDst, pSrc, SrcSize);
    totSize = SrcSize;

    osal_free(pSrc);
    return totSize;
#else
    // if dec video format is I420 NV21 NV12, just dump it
    if (is10bit != TRUE  && is422 != TRUE && isPack != TRUE) {
        osal_memcpy(pDst, pSrc, SrcSize);
        osal_free(pSrc);
        return totSize;
    }
#endif

    //5. deFormat
    //all format for 10bit => 1 pixel 2byte LSB
    if (is10bit == TRUE) {
        if (is3pxl4byte == TRUE) {
            deFormatSize = dstWidthY*srcHeightY + (dstWidthC*srcHeightY * 2);
            pFormat      = (Uint8*)osal_malloc(deFormatSize);
        }
        pSrcFormat = pSrc;
        pDstFormat = (is3pxl4byte == TRUE) ? pFormat : pSrc;

        DeFormatYUV( is3pxl4byte, isMSB, interLeave, srcWidthY, srcHeightY, srcWidthC, srcHeightC,
            dstWidthY, dstWidthC, pSrcFormat, pDstFormat);
    }

    //6. dePack or interleave
    //Pack => Normal
    //interleave => Normal
    if (isPack) {
        //pPack allocation
        PackSize = dstWidthY*dstHeightY + dstWidthC*dstHeightY * 2;
        pPack    = (Uint8*)osal_malloc(PackSize);
        if ( !pPack )
            return 0;
        pSrcPack = pSrc; //3pixel4byte no support
        pDstPack = pPack;

        DePackedYUV(is10bit, PackMode, srcWidthY, srcHeightY, pSrcPack,
            dstWidthY, dstHeightY, dstWidthC, dstHeightY/*Pack : 422*/, pDstPack);
    }
    else if (interLeave == TRUE) {
        pSrcInterLeave = (is10bit) ? pDstFormat : pSrc;
        if (is422) {
            InterLeaveSize = dstWidthY*srcHeightY + dstWidthC*srcHeightC * 2;
            pInterLeave    = (Uint8*)osal_malloc(InterLeaveSize);
            if ( !pInterLeave )
                return 0;
            pDstInterLeave = pInterLeave;
        }
        else
            pDstInterLeave = pDst;

        DeInterLeave(is10bit, nv21, dstWidthY, srcHeightY, dstWidthC*2, srcHeightC, pSrcInterLeave, pDstInterLeave);
    }

    //7. 422 => 420
    if (is422 || isPack) {
        pSrc422 =  (isPack) ?      pDstPack         :
                   ((interLeave) ? pDstInterLeave   :
                   ((is10bit) ?    pDstFormat       : pSrc));
        pDst422 = pDst;

        Convert422to420(pSrc422, dstWidthY, dstHeightY, dstWidthC, dstHeightC, pDst422);
    }

    //8.byte swap for short-type for Windows
    if (is10bit) {
        pSrcSwap = (is422 || isPack) ?     pDst422        :
                   ((interLeave == TRUE) ? pDstInterLeave : pDstFormat);
        pDstSwap = pDst;
        ByteSwap10bit(DstSize, pSrcSwap, pDstSwap);
    }

    //9: free memories
    osal_free(pSrc);
    if (pFormat != NULL)
        osal_free(pFormat);
    if (pPack != NULL)
        osal_free(pPack);
    if (pInterLeave != NULL)
        osal_free(pInterLeave);
    return totSize;
}

static void DeInterLeave(
    Uint32   is10bit,
    BOOL     nv21,
    Uint32   srcWidthY,
    Uint32   srcHeightY,
    Uint32   srcWidthC,
    Uint32   srcHeightC,
    Uint8*   pSrc,
    Uint8*   pDst
    )
{
    int x, y;
    int src_stride;
    int dst_stride;
    if (!pSrc)
        return ;
    if (!pDst)
        return ;

    //luma copy
    osal_memcpy(pDst, pSrc, srcWidthY*srcHeightY);

    //interleave => normal
    if (is10bit) {
        Uint16   *pSrcChroma;
        Uint16   *pDstCb, *pDstCr;
        src_stride = srcWidthC / 2;
        dst_stride = srcWidthC / 4;

        pSrcChroma = (Uint16*)(pSrc   + srcWidthY*srcHeightY);
        pDstCb     = (Uint16*)(pDst   + srcWidthY*srcHeightY);
        pDstCr     = (Uint16*)(pDstCb + dst_stride*srcHeightC);

        for (y = 0; y < (int)srcHeightC; y++) {
            for (x = 0; x < (int)src_stride; x += 2) {
                if (nv21) {
                    pDstCr[x / 2] = pSrcChroma[x + 0];
                    pDstCb[x / 2] = pSrcChroma[x + 1];
                }
                else {
                    pDstCb[x / 2] = pSrcChroma[x + 0];
                    pDstCr[x / 2] = pSrcChroma[x + 1];
                }
            }
            pSrcChroma += src_stride;
            pDstCb     += dst_stride;
            pDstCr     += dst_stride;
        }
    }
    else {
        Uint8   *pSrcChroma;
        Uint8   *pDstCb, *pDstCr;
        src_stride = srcWidthC;
        dst_stride = srcWidthC / 2;

        pSrcChroma = pSrc   + srcWidthY*srcHeightY;
        pDstCb     = pDst   + srcWidthY*srcHeightY;
        pDstCr     = pDstCb + dst_stride*srcHeightC;
        for (y = 0; y < (int)srcHeightC; y++) {
            for (x = 0; x < (int)src_stride; x += 2) {
                if (nv21) {
                    pDstCr[x / 2] = pSrcChroma[x + 0];
                    pDstCb[x / 2] = pSrcChroma[x + 1];
                }
                else {
                    pDstCb[x / 2] = pSrcChroma[x + 0];
                    pDstCr[x / 2] = pSrcChroma[x + 1];
                }
            }
            pSrcChroma += src_stride;
            pDstCb     += dst_stride;
            pDstCr     += dst_stride;
        }
    }
}

void ByteSwap10bit(
    Uint32   DstSize,
    Uint8*   pSrc,
    Uint8*   pDst
    )
{
    int x;
    if (!pSrc)
        return;
    for (x = 0; x < (int)DstSize; x += 2) {
        Uint8   temp0 = pSrc[x + 0];
        Uint8   temp1 = pSrc[x + 1];
        pDst[x + 0] = temp1;
        pDst[x + 1] = temp0;
    }
}

void DeFormatYUV(
    Uint32   is3pixel4byte,
    Uint32   isMSB,
    Uint32   interLeave,
    Uint32   srcWidthY,
    Uint32   srcHeightY,
    Uint32   srcWidthC,
    Uint32   srcHeightC,
    Uint32   dstWidthY,
    Uint32   dstWidthC,
    Uint8*   pSrc,
    Uint8*   pDst
    )
{
    int x, y;
    Uint8   *pSrcY, *pSrcCb, *pSrcCr;
    Uint8   *pDstY, *pDstCb, *pDstCr;

    if (!pSrc)
        return;
    if (!pDst)
        return;
    if (is3pixel4byte) {
        int src_luma_stride     = srcWidthY;
        int src_chroma_stride   = srcWidthC;
        int dst_luma_stride     = dstWidthY;
        int dst_chroma_stride   = (interLeave) ? dstWidthC * 2 : dstWidthC;
        int i;

        //luma
        pSrcY  = pSrc;
        pDstY  = pDst;
        for (y = 0; y < (int)srcHeightY; y++) {
            i = 0;
            for (x = 0; x < (int)src_luma_stride; x += 4) {
                Uint32   temp0 = pSrcY[x + 3];
                Uint32   temp1 = pSrcY[x + 2];
                Uint32   temp2 = pSrcY[x + 1];
                Uint32   temp3 = pSrcY[x + 0];
                Uint32   temp  = (temp3 << 24) + (temp2 << 16) + (temp1 << 8) + temp0;
                if (isMSB) {
                    temp = temp >> 2;
                    temp0 = (temp >> 20) & 0x3ff;
                    temp1 = (temp >> 10) & 0x3ff;
                    temp2 = (temp >> 0) & 0x3ff;
                }
                else {
                    temp2 = (temp >> 20) & 0x3ff;
                    temp1 = (temp >> 10) & 0x3ff;
                    temp0 = (temp >> 0) & 0x3ff;
                }

                //first pixel
                pDstY[i] = (temp0 >> 2) & 0xff; i++;
                pDstY[i] = (temp0 << 6) & 0xc0; i++;
                if (i < dst_luma_stride) {
                    pDstY[i] = (temp1 >> 2) & 0xff; i++;
                    pDstY[i] = (temp1 << 6) & 0xc0; i++;
                }
                if (i < dst_luma_stride) {
                    pDstY[i] = (temp2 >> 2) & 0xff; i++;
                    pDstY[i] = (temp2 << 6) & 0xc0; i++;
                }
            }
            pDstY += dst_luma_stride;
            pSrcY += src_luma_stride;
        }

        pSrcCb  = pSrc + srcWidthY*srcHeightY;
        pDstCb  = pDst + dstWidthY*srcHeightY;

        for (y = 0; y < (int)srcHeightC; y++) {
            i = 0;
            for (x = 0; x < (int)src_chroma_stride; x += 4) {
                Uint32   temp0 = pSrcCb[x + 3];
                Uint32   temp1 = pSrcCb[x + 2];
                Uint32   temp2 = pSrcCb[x + 1];
                Uint32   temp3 = pSrcCb[x + 0];
                Uint32   temp  = (temp3 << 24) + (temp2 << 16) + (temp1 << 8) + temp0;
                if (isMSB) {
                    temp = temp >> 2;
                    temp0 = (temp >> 20) & 0x3ff;
                    temp1 = (temp >> 10) & 0x3ff;
                    temp2 = (temp >> 0) & 0x3ff;
                }
                else {
                    temp2 = (temp >> 20) & 0x3ff;
                    temp1 = (temp >> 10) & 0x3ff;
                    temp0 = (temp >> 0) & 0x3ff;
                }
                                                //first pixel
                pDstCb[i] = (temp0 >> 2) & 0xff; i++;
                pDstCb[i] = (temp0 << 6) & 0xc0; i++;
                if (i < dst_chroma_stride) {
                    pDstCb[i] = (temp1 >> 2) & 0xff; i++;
                    pDstCb[i] = (temp1 << 6) & 0xc0; i++;
                }
                if (i < dst_chroma_stride) {
                    pDstCb[i] = (temp2 >> 2) & 0xff; i++;
                    pDstCb[i] = (temp2 << 6) & 0xc0; i++;
                }
            }
            pDstCb += dst_chroma_stride;
            pSrcCb += src_chroma_stride;
        }

        if (interLeave != TRUE) {
            pSrcCr  = pSrc + srcWidthY*srcHeightY + srcWidthC*srcHeightC;
            pDstCr  = pDst + dstWidthY*srcHeightY + dstWidthC*srcHeightC;

            for (y = 0; y < (int)srcHeightC; y++) {
                i = 0;
                for (x = 0; x < (int)src_chroma_stride; x += 4) {
                    Uint32   temp0 = pSrcCr[x + 3];
                    Uint32   temp1 = pSrcCr[x + 2];
                    Uint32   temp2 = pSrcCr[x + 1];
                    Uint32   temp3 = pSrcCr[x + 0];
                    Uint32   temp  = (temp3 << 24) + (temp2 << 16) + (temp1 << 8) + temp0;
                    if (isMSB) {
                        temp = temp >> 2;
                        temp0 = (temp >> 20) & 0x3ff;
                        temp1 = (temp >> 10) & 0x3ff;
                        temp2 = (temp >> 0) & 0x3ff;
                    }
                    else {
                        temp2 = (temp >> 20) & 0x3ff;
                        temp1 = (temp >> 10) & 0x3ff;
                        temp0 = (temp >> 0) & 0x3ff;
                    }
                                                            //first pixel
                    pDstCr[i] = (temp0 >> 2) & 0xff; i++;
                    pDstCr[i] = (temp0 << 6) & 0xc0; i++;
                    if (i < dst_chroma_stride) {
                        pDstCr[i] = (temp1 >> 2) & 0xff; i++;
                        pDstCr[i] = (temp1 << 6) & 0xc0; i++;
                    }
                    if (i < dst_chroma_stride) {
                        pDstCr[i] = (temp2 >> 2) & 0xff; i++;
                        pDstCr[i] = (temp2 << 6) & 0xc0; i++;
                    }
                }
                pDstCr += dst_chroma_stride;
                pSrcCr += src_chroma_stride;
            }
        }
    }
    else if (isMSB != TRUE) {
        int luma_stride         = srcWidthY;
        int chroma_stride       = srcWidthC;
        //luma
        pSrcY  = pSrc;
        pDstY  = pDst;
        for (y = 0; y < (int)srcHeightY; y++) {
            for (x = 0; x < (int)luma_stride; x += 2) {
                Uint16   temp0 = pSrcY[x + 0];
                Uint16   temp1 = pSrcY[x + 1];
                Uint16   temp  = (temp0 << (8 + 6)) + (temp1 << 6);
                Uint16   pxl8bit1 = (temp >> 0) & 0xc0;
                Uint16   pxl8bit0 = (temp >> 8) & 0xff;
                pDstY[x + 0] = (Uint8)pxl8bit0;
                pDstY[x + 1] = (Uint8)pxl8bit1;
            }
            pDstY += luma_stride;
            pSrcY += luma_stride;
        }

        pSrcCb = (pSrc + srcWidthY*srcHeightY);
        pDstCb = (pDst + srcWidthY*srcHeightY);

        for (y = 0; y < (int)srcHeightC; y++) {
            for (x = 0; x < (int)chroma_stride; x += 2) {
                Uint16   temp0 = pSrcCb[x + 0];
                Uint16   temp1 = pSrcCb[x + 1];
                Uint16   temp  = (temp0 << (8 + 6)) + (temp1 << 6);
                Uint16   pxl8bit1 = (temp >> 0) & 0xc0;
                Uint16   pxl8bit0 = (temp >> 8) & 0xff;
                pDstCb[x + 0] = (Uint8)pxl8bit0;
                pDstCb[x + 1] = (Uint8)pxl8bit1;
            }
            pDstCb += chroma_stride;
            pSrcCb += chroma_stride;
        }
        if (interLeave != TRUE) {
            pSrcCr = (pSrc + srcWidthY*srcHeightY + srcHeightC*srcWidthC);
            pDstCr = (pDst + srcWidthY*srcHeightY + srcHeightC*srcWidthC);
            for (y = 0; y < (int)srcHeightC; y++) {
                for (x = 0; x < (int)chroma_stride; x += 2) {
                    Uint16   temp0 = pSrcCr[x + 0];
                    Uint16   temp1 = pSrcCr[x + 1];
                    Uint16   temp  = (temp0 << (8 + 6)) + (temp1 << 6);
                    Uint16   pxl8bit1 = (temp >> 0) & 0xc0;
                    Uint16   pxl8bit0 = (temp >> 8) & 0xff;
                    pDstCr[x + 0] = (Uint8)pxl8bit0;
                    pDstCr[x + 1] = (Uint8)pxl8bit1;
                }
                pDstCr += chroma_stride;
                pSrcCr += chroma_stride;
            }
        }
    }
}

void Convert422to420(
    Uint8*   pSrc,
    Uint32   dstWidthY,
    Uint32   dstHeightY,
    Uint32   dstWidthC,
    Uint32   dstHeightC,
    Uint8*   pDst
    )
{
    int y;
    int x;
    int luma_stride;
    int chroma_stride;
    Uint8*   pSrcCb;
    Uint8*   pSrcCr;
    Uint8*   pDstCb;
    Uint8*   pDstCr;

    luma_stride   = dstWidthY;
    chroma_stride = dstWidthC;

    if(!pSrc)
        return ;
    if(!pDst)
        return ;
    //1. luma osal_memcpy
    osal_memcpy(pDst, pSrc, luma_stride*dstHeightY);

    //2.1 chroma copy :Cb
    pSrcCb = pSrc + luma_stride*dstHeightY;
    pDstCb = pDst + luma_stride*dstHeightY;
    for (y = 0; y < (int)dstHeightY; y += 2) {
        //checek SrcCb
        for (x = 0; x < (int)chroma_stride; x++)
            if (pSrcCb[x] != pSrcCb[x + chroma_stride])
                pSrcCb[x] = pSrcCb[x]; //assert
        osal_memcpy(pDstCb, pSrcCb, chroma_stride);
        pDstCb += chroma_stride;
        pSrcCb += 2*chroma_stride;
    }

    //2.2 chroma copy : Cr
    pSrcCr = pSrc + luma_stride*dstHeightY + chroma_stride*dstHeightY;
    pDstCr = pDst + luma_stride*dstHeightY + chroma_stride*dstHeightC;

    for (y = 0; y < (int)dstHeightY; y += 2) {
        //checek SrcCr
        for (x = 0; x < (int)chroma_stride; x++)
            if (pSrcCr[x] != pSrcCr[x + chroma_stride])
                pSrcCr[x] = pSrcCr[x]; //assert
        osal_memcpy(pDstCr, pSrcCr, chroma_stride);
        pDstCr += chroma_stride;
        pSrcCr += 2*chroma_stride;
    }
}

void DePackedYUV(
    Uint32   is10bit,
    Uint32   PackMode,
    Uint32   srcWidthY,
    Uint32   srcHeightY,
    Uint8*   pSrc,
    Uint32   dstWidthY,
    Uint32   dstHeightY,
    Uint32   dstWidthC,
    Uint32   dstHeightC,
    Uint8*   pDst
    )
{
    int x, y;
    int stride;
    int dst_strideY;
    int dst_strideC;
    if (is10bit) {
        Uint16*   pSrcPack;
        Uint16*   pDstY;
        Uint16*   pDstCb;
        Uint16*   pDstCr;
        Uint16    Y0, Y1, U, V;

        stride          = srcWidthY / 2;
        dst_strideY     = dstWidthY / 2;
        dst_strideC     = dstWidthC / 2;

        pDstY  = (Uint16*)pDst;
        pDstCb = (Uint16*)(pDst + dstWidthY * dstHeightY);
        pDstCr = (Uint16*)(pDst + dstWidthY * dstHeightY + dstWidthC * dstHeightC);

        for (y = 0; y < (int)srcHeightY; y++) {
            pSrcPack = ((Uint16*)pSrc) + y*stride;
            for (x = 0; x < stride; x += 4) {
                switch (PackMode) {
                case 0 : //YUYV
                    Y0 = pSrcPack[x + 0];
                    U  = pSrcPack[x + 1];
                    Y1 = pSrcPack[x + 2];
                    V  = pSrcPack[x + 3];
                    break;
                case 1 : //YVYU
                    Y0 = pSrcPack[x + 0];
                    V  = pSrcPack[x + 1];
                    Y1 = pSrcPack[x + 2];
                    U  = pSrcPack[x + 3];
                    break;
                case 2 : //UYVY
                    U  = pSrcPack[x + 0];
                    Y0 = pSrcPack[x + 1];
                    V  = pSrcPack[x + 2];
                    Y1 = pSrcPack[x + 3];
                    break;
                default : //VYUY
                    V  = pSrcPack[x + 0];
                    Y0 = pSrcPack[x + 1];
                    U  = pSrcPack[x + 2];
                    Y1 = pSrcPack[x + 3];
                    break;
                }
                pDstY[x / 2 + 0] = Y0;
                pDstY[x / 2 + 1] = Y1;
                pDstCb[x / 4]    = U;
                pDstCr[x / 4]    = V;
            }
            pSrcPack += stride;
            pDstY    += dst_strideY;
            pDstCb   += dst_strideC;
            pDstCr   += dst_strideC;
        }
    }
    else {
        Uint8*   pSrcPack;
        Uint8*   pDstY;
        Uint8*   pDstCb;
        Uint8*   pDstCr;
        Uint8    Y0, Y1, U, V;

        stride = srcWidthY;
        dst_strideY     = dstWidthY;
        dst_strideC     = dstWidthC;
        pDstY  = pDst;
        pDstCb = pDst + dstWidthY * dstHeightY;
        pDstCr = pDst + dstWidthY * dstHeightY + dstWidthC * dstHeightC;

        for (y = 0; y < (int)srcHeightY; y++) {
            pSrcPack = pSrc + y*stride;
            for (x = 0; x < (int)stride; x += 4) {
                switch (PackMode) {
                case 0 : //YUYV
                    Y0 = pSrcPack[x + 0];
                    U  = pSrcPack[x + 1];
                    Y1 = pSrcPack[x + 2];
                    V  = pSrcPack[x + 3];
                    break;
                case 1 : //YVYU
                    Y0 = pSrcPack[x + 0];
                    V  = pSrcPack[x + 1];
                    Y1 = pSrcPack[x + 2];
                    U  = pSrcPack[x + 3];
                    break;
                case 2 : //UYVY
                    U  = pSrcPack[x + 0];
                    Y0 = pSrcPack[x + 1];
                    V  = pSrcPack[x + 2];
                    Y1 = pSrcPack[x + 3];
                    break;
                default : //VYUY
                    V  = pSrcPack[x + 0];
                    Y0 = pSrcPack[x + 1];
                    U  = pSrcPack[x + 2];
                    Y1 = pSrcPack[x + 3];
                    break;
                }
                pDstY[x / 2 + 0] = Y0;
                pDstY[x / 2 + 1] = Y1;
                pDstCb[x / 4]    = U;
                pDstCr[x / 4]    = V;
            }
            pSrcPack += stride;
            pDstY    += dst_strideY;
            pDstCb   += dst_strideC;
            pDstCr   += dst_strideC;
        }
    }
}

void DePxlOrder8bit(Uint8* pPxl16Src, Uint8* pPxl16Dst)
{
#if 1
    //-------------8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit-
    //mode    1: | p15 | p14 | p13 | p12 | p11 | p10 | p09 | p08 | p07 | p06 | p05 | p04 | p03 | p02 | p01 | p00 |
    //HW output: | p00 | p01 | p02 | p03 | p04 | p05 | p06 | p07 | p08 | p09 | p10 | p11 | p12 | p13 | p14 | p15 |
    //mode    0: | p00 | p01 | p02 | p03 | p04 | p05 | p06 | p07 | p08 | p09 | p10 | p11 | p12 | p13 | p14 | p15 |

    // mode 1 vs HW output : endian swap
    // HW output = mode 0
    //1) no change
#else
    //-------------8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit--8bit-
    //mode    1: | p15 | p14 | p13 | p12 | p11 | p10 | p09 | p08 | p07 | p06 | p05 | p04 | p03 | p02 | p01 | p00 |
    //HW output: | p15 | p14 | p13 | p12 | p11 | p10 | p09 | p08 | p07 | p06 | p05 | p04 | p03 | p02 | p01 | p00 |
    //mode    0: | p00 | p01 | p02 | p03 | p04 | p05 | p06 | p07 | p08 | p09 | p10 | p11 | p12 | p13 | p14 | p15 |

    // mode 1 = HW output
    // HW output vs mode 0 : endian swap
    //1) swap pixels
    int i;
    for (i = 0; i < 16; i++)
        pPxl16Dst[15 - i] = pPxl16Src[i];
#endif
}
void DePxlOrder1pxl2byte(Uint8* pPxl16Src, Uint8* pPxl16Dst)
{
#if 1
    //MSB
    //-----------|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|
    //mode    1: |    p7[9:2]   | p7[1:0],6'b0 |    p6[9:2]   | p6[1:0],6'b0 |    p5[9:2]   | p5[1:0],6'b0 |    p4[9:2]   | p4[1:0],6'b0 |    p3[9:2]   | p3[1:0],6'b0 |    p2[9:2]   | p2[1:0],6'b0 |    p1[9:2]   | p1[1:0],6'b0 |    p0[9:2]   | p0[1:0],6'b0 |
    //HW output: | p0[1:0],6'b0 |    p0[9:2]   | p1[1:0],6'b0 |    p1[9:2]   | p2[1:0],6'b0 |    p2[9:2]   | p3[1:0],6'b0 |    p3[9:2]   | p4[1:0],6'b0 |    p4[9:2]   | p5[1:0],6'b0 |    p5[9:2]   | p6[1:0],6'b0 |    p6[9:2]   | p7[1:0],6'b0 |    p7[9:2]   |
    //mode    0: |    p0[9:2]   | p0[1:0],6'b0 |    p1[9:2]   | p1[1:0],6'b0 |    p2[9:2]   | p2[1:0],6'b0 |    p3[9:2]   | p3[1:0],6'b0 |    p4[9:2]   | p4[1:0],6'b0 |    p5[9:2]   | p5[1:0],6'b0 |    p6[9:2]   | p6[1:0],6'b0 |    p7[9:2]   | p7[1:0],6'b0 |

    //LSB
    //-----------|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|
    //mode    1: | 6'b0,p7[9:8] |    p7[7:0]   | 6'b0,p6[9:8] |    p6[7:0]   | 6'b0,p5[9:8] |    p5[7:0]   | 6'b0,p4[9:8] |    p4[7:0]   | 6'b0,p3[9:8] |    p3[7:0]   | 6'b0,p2[9:8] |    p2[7:0]   | 6'b0,p1[9:8] |    p1[7:0]   | 6'b0,p0[9:8] |    p0[7:0]   |
    //HW output: |    p0[7:0]   | 6'b0,p0[9:8] |    p1[7:0]   | 6'b0,p1[9:8] |    p2[7:0]   | 6'b0,p2[9:8] |    p3[7:0]   | 6'b0,p3[9:8] |    p4[7:0]   | 6'b0,p4[9:8] |    p5[7:0]   | 6'b0,p5[9:8] |    p6[7:0]   | 6'b0,p6[9:8] |    p7[7:0]   | 6'b0,p7[9:8] |
    //mode    0: | 6'b0,p0[9:8] |    p0[7:0]   | 6'b0,p1[9:8] |    p1[7:0]   | 6'b0,p2[9:8] |    p2[7:0]   | 6'b0,p3[9:8] |    p3[7:0]   | 6'b0,p4[9:8] |    p4[7:0]   | 6'b0,p5[9:8] |    p5[7:0]   | 6'b0,p6[9:8] |    p6[7:0]   | 6'b0,p7[9:8] |    p7[7:0]   |

    // mode 1 vs HW output : endian swap
    // HW output vs mode 1 : byte swap in 2 bytes
    //1) byte swap in 2 bytes
    int i;
    Uint8* pSrc = pPxl16Src;
    Uint8* pDst = pPxl16Dst;
    for (i = 0; i < 8; i++) {
        pDst[i * 2 + 1] = pSrc[i * 2 + 0];
        pDst[i * 2 + 0] = pSrc[i * 2 + 1];
    }
#else
    //MSB
    //-----------|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|
    //mode    1: |    p7[9:2]   | p7[1:0],6'b0 |    p6[9:2]   | p6[1:0],6'b0 |    p5[9:2]   | p5[1:0],6'b0 |    p4[9:2]   | p4[1:0],6'b0 |    p3[9:2]   | p3[1:0],6'b0 |    p2[9:2]   | p2[1:0],6'b0 |    p1[9:2]   | p1[1:0],6'b0 |    p0[9:2]   | p0[1:0],6'b0 |
    //HW output: |    p7[9:2]   | p7[1:0],6'b0 |    p6[9:2]   | p6[1:0],6'b0 |    p5[9:2]   | p5[1:0],6'b0 |    p4[9:2]   | p4[1:0],6'b0 |    p3[9:2]   | p3[1:0],6'b0 |    p2[9:2]   | p2[1:0],6'b0 |    p1[9:2]   | p1[1:0],6'b0 |    p0[9:2]   | p0[1:0],6'b0 |
    //mode    0: |    p0[9:2]   | p0[1:0],6'b0 |    p1[9:2]   | p1[1:0],6'b0 |    p2[9:2]   | p2[1:0],6'b0 |    p3[9:2]   | p3[1:0],6'b0 |    p4[9:2]   | p4[1:0],6'b0 |    p5[9:2]   | p5[1:0],6'b0 |    p6[9:2]   | p6[1:0],6'b0 |    p7[9:2]   | p7[1:0],6'b0 |

    //LSB
    //-----------|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|-----8bit-----|
    //mode    1: | 6'b0,p7[9:8] |    p7[7:0]   | 6'b0,p6[9:8] |    p6[7:0]   | 6'b0,p5[9:8] |    p5[7:0]   | 6'b0,p4[9:8] |    p4[7:0]   | 6'b0,p3[9:8] |    p3[7:0]   | 6'b0,p2[9:8] |    p2[7:0]   | 6'b0,p1[9:8] |    p1[7:0]   | 6'b0,p0[9:8] |    p0[7:0]   |
    //HW output: | 6'b0,p7[9:8] |    p7[7:0]   | 6'b0,p6[9:8] |    p6[7:0]   | 6'b0,p5[9:8] |    p5[7:0]   | 6'b0,p4[9:8] |    p4[7:0]   | 6'b0,p3[9:8] |    p3[7:0]   | 6'b0,p2[9:8] |    p2[7:0]   | 6'b0,p1[9:8] |    p1[7:0]   | 6'b0,p0[9:8] |    p0[7:0]   |
    //mode    0: | 6'b0,p0[9:8] |    p0[7:0]   | 6'b0,p1[9:8] |    p1[7:0]   | 6'b0,p2[9:8] |    p2[7:0]   | 6'b0,p3[9:8] |    p3[7:0]   | 6'b0,p4[9:8] |    p4[7:0]   | 6'b0,p5[9:8] |    p5[7:0]   | 6'b0,p6[9:8] |    p6[7:0]   | 6'b0,p7[9:8] |    p7[7:0]   |

    // mode 1 = HW output
    // HW output vs mode 0 : 2-byte word swap
    //1) 2-byte word swap
    int i;
    Uint16*   pSrc = (Uint16*) pPxl16Src;
    Uint16*   pDst = (Uint16*) pPxl16Dst;
    for (i = 0; i < 8; i++)
        pDst[7 - i] = pSrc[i];
#endif
}
void DePxlOrder3pxl4byteLSB(Uint8* pPxl16Src, Uint8* pPxl16Dst)
{
#if 1
    //-----------|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|
    //mode    1: | 2'b0,p11[9:4] | p11[3:0],p10[9:6] | p10[5:0],p09[9:8] | p09[7:0] | 2'b0,p08[9:4] | p08[3:0],p07[9:6] | p07[5:0],p06[9:8] | p06[7:0] | 2'b0,p05[9:4] | p05[3:0],p04[9:6] | p04[5:0],p03[9:8] | p03[7:0] | 2'b0,p02[9:4] | p02[3:0],p01[9:6] | p01[5:0],p00[9:8] | p00[7:0] |
    //                       |       A       |         B         |         C         |     D    |       E       |         F         |         G         |     H    |       I       |         J         |         K         |     L    |       M       |         N         |         O         |     P    |

    //-----------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|
    //HW output: | p00[7:0] | p01[5:0],p00[9:8] | p02[3:0],p01[9:6] | 2'b0,p02[9:4] | p03[7:0] | p04[5:0],p03[9:8] | p05[3:0],p04[9:6] | 2'b0,p05[9:4] | p06[7:0] | p07[5:0],p06[9:8] | p08[3:0],p07[9:6] | 2'b0,p08[9:4] | p09[7:0] | p10[5:0],p09[9:8] | p11[3:0],p10[9:6] | 2'b0,p11[9:4] |
    //           |     P    |         O         |         N         |       M       |     L    |         K         |         J         |       I       |     H    |         G         |         F         |       E       |     D    |         C         |         B         |       A       |

    //-----------|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|
    //mode    0: | 2'b0,p02[9:4] | p02[3:0],p01[9:6] | p01[5:0],p00[9:8] | p00[7:0] | 2'b0,p05[9:4] | p05[3:0],p04[9:6] | p04[5:0],p03[9:8] | p03[7:0] | 2'b0,p08[9:4] | p08[3:0],p07[9:6] | p07[5:0],p06[9:8] | p06[7:0] | 2'b0,p11[9:4] | p11[3:0],p10[9:6] | p10[5:0],p09[9:8] | p09[7:0] |
    //           |       M       |         N         |         O         |     P    |       I       |         J         |         K         |     L    |       E       |         F         |         G         |     H    |       A       |         B         |         C         |     D    |

    // mode 1 vs HW output : endian swap
    // HW output vs mode 1 : byte swap in 4 bytes
    //1) byte swap in 4 bytes
    int i;
    Uint8* pSrc = pPxl16Src;
    Uint8* pDst = pPxl16Dst;
    for (i = 0; i < 4; i++) {
        pDst[i * 4 + 3] = pSrc[i * 4 + 0];
        pDst[i * 4 + 2] = pSrc[i * 4 + 1];
        pDst[i * 4 + 1] = pSrc[i * 4 + 2];
        pDst[i * 4 + 0] = pSrc[i * 4 + 3];
    }

#else
    //-----------|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|
    //mode    1: | 2'b0,p11[9:4] | p11[3:0],p10[9:6] | p10[5:0],p09[9:8] | p09[7:0] | 2'b0,p08[9:4] | p08[3:0],p07[9:6] | p07[5:0],p06[9:8] | p06[7:0] | 2'b0,p05[9:4] | p05[3:0],p04[9:6] | p04[5:0],p03[9:8] | p03[7:0] | 2'b0,p02[9:4] | p02[3:0],p01[9:6] | p01[5:0],p00[9:8] | p00[7:0] |
    //                       |       A       |         B         |         C         |     D    |       E       |         F         |         G         |     H    |       I       |         J         |         K         |     L    |       M       |         N         |         O         |     P    |
    //HW output: | 2'b0,p11[9:4] | p11[3:0],p10[9:6] | p10[5:0],p09[9:8] | p09[7:0] | 2'b0,p08[9:4] | p08[3:0],p07[9:6] | p07[5:0],p06[9:8] | p06[7:0] | 2'b0,p05[9:4] | p05[3:0],p04[9:6] | p04[5:0],p03[9:8] | p03[7:0] | 2'b0,p02[9:4] | p02[3:0],p01[9:6] | p01[5:0],p00[9:8] | p00[7:0] |
    //                       |       A       |         B         |         C         |     D    |       E       |         F         |         G         |     H    |       I       |         J         |         K         |     L    |       M       |         N         |         O         |     P    |
    //mode    0: | 2'b0,p02[9:4] | p02[3:0],p01[9:6] | p01[5:0],p00[9:8] | p00[7:0] | 2'b0,p05[9:4] | p05[3:0],p04[9:6] | p04[5:0],p03[9:8] | p03[7:0] | 2'b0,p08[9:4] | p08[3:0],p07[9:6] | p07[5:0],p06[9:8] | p06[7:0] | 2'b0,p11[9:4] | p11[3:0],p10[9:6] | p10[5:0],p09[9:8] | p09[7:0] |
    //                       |       M       |         N         |         O         |     P    |       I       |         J         |         K         |     L    |       E       |         F         |         G         |     H    |       A       |         B         |         C         |     D    |

    // mode 1 = HW output
    // HW output vs mode 0 : 4-byte word swap
    //1) 4-byte word swap
    int i;
    Uint32*   pSrc = (Uint32*) pPxl16Src;
    Uint32*   pDst = (Uint32*) pPxl16Dst;
    for (i = 0; i < 4; i++)
        pDst[3 - i] = pSrc[i];
#endif
}
void DePxlOrder3pxl4byteMSB(Uint8* pPxl16Src, Uint8* pPxl16Dst)
{
#if 1
    //-----------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|
    //mode    1: | p11[9:2] | p11[1:0],p10[9:4] | p10[3:0],p09[9:6] | p09[5:0],2'b0 | p08[9:2] | p08[1:0],p07[9:4] | p07[3:0],p06[9:6] | p06[5:0],2'b0 | p05[9:2] | p05[1:0],p04[9:4] | p04[3:0],p03[9:6] | p03[5:0],2'b0 | p02[9:2] | p02[1:0],p01[9:4] | p01[3:0],p00[9:6] | p00[5:0],2'b0 |
    //                       |     A    |         B         |         C         |       D       |     E    |         F         |         G         |       H       |     I    |         J         |         K         |       L       |     M    |         N         |         O         |       P       |

    //-----------|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|-----8bit------|-------8bit--------|-------8bit--------|---8bit---|
    //HW output: | p00[5:0],2'b0 | p01[3:0],p00[9:6] | p02[1:0],p01[9:4] | p02[9:2] | p03[5:0],2'b0 | p04[3:0],p03[9:6] | p05[1:0],p04[9:4] | p05[9:2] | p06[5:0],2'b0 | p07[3:0],p06[9:6] | p08[1:0],p07[9:4] | p08[9:2] | p09[5:0],2'b0 | p10[3:0],p09[9:6] | p11[1:0],p10[9:4] | p11[9:2] |
    //                       |       P       |         O         |         N         |     M    |       L       |         K         |         J         |     I    |       H       |         G         |         F         |     E    |       D       |         C         |         B         |     A    |

    //-----------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|
    //mode    0: | p00[9:2] | p00[1:0],p01[9:4] | p01[3:0],p02[9:6] | p02[5:0],2'b0 | p03[9:2] | p03[1:0],p04[9:4] | p04[3:0],p05[9:6] | p05[5:0],2'b0 | p06[9:2] | p06[1:0],p07[9:4] | p07[3:0],p08[9:6] | p08[5:0],2'b0 | p09[9:2] | p09[1:0],p10[9:4] | p10[3:0],p11[9:6] | p11[5:0],2'b0 |

    // mode 1 vs HW output : endian swap
    // HW output vs mode 1 : byte swap in 4 bytes & pixel swap in 3 pixels
    //1) byte swap in 4 bytes
    //2) pixel swap in 3 pixels
    int i;
    Uint8*   pSrc = pPxl16Src;
    Uint8*   pDst = pPxl16Dst;
    Uint32   temp ;
    Uint32   temp0;
    Uint32   temp1;
    Uint32   temp2;
    for (i = 0; i < 4; i++) {
        pDst[i * 4 + 3] = pSrc[i * 4 + 0];
        pDst[i * 4 + 2] = pSrc[i * 4 + 1];
        pDst[i * 4 + 1] = pSrc[i * 4 + 2];
        pDst[i * 4 + 0] = pSrc[i * 4 + 3]; //| p02[9:2] | p02[1:0],p01[9:4] | p01[3:0],p00[9:6] | p00[5:0],2'b0 |
        temp  = (pDst[4*i + 0] << 24) + (pDst[4*i + 1] << 16) + (pDst[4*i + 2] << 8) + (pDst[4*i + 3] << 0);
        temp0 = (temp >> 2) & 0x3ff;
        temp1 = (temp >> 12) & 0x3ff;
        temp2 = (temp >> 22) & 0x3ff;
        temp = (temp2 << 2) + (temp1 << 12) + (temp0 << 22);
        pDst[i * 4 + 0] = (temp >> 24) & 0xff;
        pDst[i * 4 + 1] = (temp >> 16) & 0xff;
        pDst[i * 4 + 2] = (temp >> 8) & 0xff;
        pDst[i * 4 + 3] = (temp >> 0) & 0xff; //| p00[9:2] | p00[1:0],p01[9:4] | p01[3:0],p02[9:6] | p02[5:0],2'b0 |
    }
#else
    //-----------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|---8bit---|-------8bit--------|-------8bit--------|-----8bit------|
    //mode    1: | p11[9:2] | p11[1:0],p10[9:4] | p10[3:0],p09[9:6] | p09[5:0],2'b0 | p08[9:2] | p08[1:0],p07[9:4] | p07[3:0],p06[9:6] | p06[5:0],2'b0 | p05[9:2] | p05[1:0],p04[9:4] | p04[3:0],p03[9:6] | p03[5:0],2'b0 | p02[9:2] | p02[1:0],p01[9:4] | p01[3:0],p00[9:6] | p00[5:0],2'b0 |
    //                       |     A    |         B         |         C         |       D       |     E    |         F         |         G         |       H       |     I    |         J         |         K         |       L       |     M    |         N         |         O         |       P       |

    //HW output: | p11[9:2] | p11[1:0],p10[9:4] | p10[3:0],p09[9:6] | p09[5:0],2'b0 | p08[9:2] | p08[1:0],p07[9:4] | p07[3:0],p06[9:6] | p06[5:0],2'b0 | p05[9:2] | p05[1:0],p04[9:4] | p04[3:0],p03[9:6] | p03[5:0],2'b0 | p02[9:2] | p02[1:0],p01[9:4] | p01[3:0],p00[9:6] | p00[5:0],2'b0 |
    //                       |     A    |         B         |         C         |       D       |     E    |         F         |         G         |       H       |     I    |         J         |         K         |       L       |     M    |         N         |         O         |       P       |

    //mode    0: | p00[9:2] | p00[1:0],p01[9:4] | p01[3:0],p02[9:6] | p02[5:0],2'b0 | p03[9:2] | p03[1:0],p04[9:4] | p04[3:0],p05[9:6] | p05[5:0],2'b0 | p06[9:2] | p06[1:0],p07[9:4] | p07[3:0],p08[9:6] | p08[5:0],2'b0 | p09[9:2] | p09[1:0],p10[9:4] | p10[3:0],p11[9:6] | p11[5:0],2'b0 |

    // mode 1 = HW output
    // HW output vs mode 0 : 4-byte word swap & pixel swap in 3 pixels
    //1) 4-byte word swap
    //2) pixel swap in 3 pixels
    int i;
    Uint8*   pSrc = pPxl16Src;
    Uint8*   pDst = pPxl16Dst;
    for (i = 0; i < 4; i++) {
        Uint32   temp  = (pSrc[4*i + 0] << 24) + (pSrc[4*i + 1] << 16) + (pSrc[4*i + 2] << 8) + (pSrc[4*i + 3] << 0);
        Uint32   temp0 = (temp >> 2) & 0x3ff;
        Uint32   temp1 = (temp >> 12) & 0x3ff;
        Uint32   temp2 = (temp >> 22) & 0x3ff;
        temp = (temp2 << 2) + (temp1 << 12) + (temp0 << 22);
        pDst[4*(3 - i) + 0] = (temp >> 24) & 0xff;
        pDst[4*(3 - i) + 1] = (temp >> 16) & 0xff;
        pDst[4*(3 - i) + 2] = (temp >> 8) & 0xff;
        pDst[4*(3 - i) + 3] = (temp >> 0) & 0xff;
    }
#endif
}
//srcWidthY and srcWidthC must be alinged with 16 pixels
void DePxlOrder(
    Uint32   srcWidthY,
    Uint32   srcHeightY,
    Uint32   srcWidthC,
    Uint32   srcHeightC,
    Uint32   interLeave,
    Uint32   is10bit,
    Uint32   is3pxl4byte,
    Uint32   isMSB,
    Uint8*   pY,
    Uint8*   pCb,
    Uint8*   pCr
    )
{
    //srcWidthY and srcWidthC must be alinged with 16 pixels
    int x, y, i, uv;
    int uv_num;
    Uint8*   pChroma;
    Uint8   temp[16];
    for (y = 0; y < (int)srcHeightY; y++) {
        for (x = 0; x < (int)srcWidthY; x += 16) {
            for (i = 0; i < 16; i++)
                temp[i] = pY[x + i];
            if (is10bit != TRUE)  //8bit
                DePxlOrder8bit(temp, &(pY[x]));
            else if (is3pxl4byte != TRUE)  //10bit : 1pxl2 byte
                DePxlOrder1pxl2byte(temp, &(pY[x]));
            else if (isMSB)  //10bit : is3pxl4byte and MSB
                DePxlOrder3pxl4byteMSB(temp, &(pY[x]));
            else  //10bit : is3pxl4byte and LSB
                DePxlOrder3pxl4byteLSB(temp, &(pY[x]));
        }
        pY += srcWidthY;
    }

    if (srcHeightC == 0)
        return;

    uv_num = (interLeave) ? 1 : 2;
    for (uv = 0; uv < uv_num; uv++) {
        if (uv == 0)
            pChroma = pCb;
        else
            pChroma = pCr;

        for (y = 0; y < (int)srcHeightC; y++) {
            for (x = 0; x < (int)srcWidthC; x += 16) {
                for (i = 0; i < 16; i++)
                    temp[i] = pChroma[x + i];
                if (is10bit != TRUE)  //8bit
                    DePxlOrder8bit(temp, &(pChroma[x]));
                else if (is3pxl4byte != TRUE)  //10bit : 1pxl2 byte
                    DePxlOrder1pxl2byte(temp, &(pChroma[x]));
                else if (isMSB)  //10bit : is3pxl4byte and MSB
                    DePxlOrder3pxl4byteMSB(temp, &(pChroma[x]));
                else  //10bit : is3pxl4byte and LSB
                     DePxlOrder3pxl4byteLSB(temp, &(pChroma[x]));
            }
            pChroma += srcWidthC;
        }
    }
}

void LoadSrcYUV(
    Uint32          coreIdx,
    TiledMapConfig  mapCfg,
    Uint8*          pSrc,
    FrameBuffer*    fbSrc,
    VpuRect         cropRect,
    BOOL            enableCrop,
    Uint32          is10bit,
    Uint32          is3pxl4byte,
    Uint32          isMSB,
    Uint32          srcWidthY,
    Uint32          srcHeightY,
    Uint32          srcWidthC,
    Uint32          srcHeightC,
    Uint32          chroma_stride
    )
{
    Uint32      y;
    Uint32      pix_addr;
    Uint8*      puc;
    Uint8*      rowBufferY, *rowBufferCb, *rowBufferCr;

    Int32       baseY;
    Int32       baseCb;
    Int32       baseCr;

    Uint8       *pY = NULL;
    Uint8       *pCb = NULL;
    Uint8       *pCr = NULL;
    Uint32      offsetX, offsetY;

    int         interLeave  = fbSrc->cbcrInterleave;
    Uint32      stride      = fbSrc->stride;
#ifdef SAVE_YUV
    EndianMode  endian      = 31;
#else
    EndianMode  endian      = (EndianMode)fbSrc->endian;
#endif

    offsetX   = (enableCrop == TRUE ? cropRect.left : 0);
    offsetY   = (enableCrop == TRUE ? cropRect.top  : 0);

    //base address
    baseY  = fbSrc->bufY;
    baseCb = fbSrc->bufCb;
    baseCr = fbSrc->bufCr;

    //1. memory allocation for FPGA YUV frame
    pY  = (Uint8*)osal_malloc(stride * srcHeightY);
    if (!pY)
        return ;
    if (srcHeightC != 0) {
        pCb = (Uint8*)osal_malloc(chroma_stride * srcHeightC);
        if (!pCb)
            return ;
        if (interLeave != TRUE) {
            pCr = (Uint8*)osal_malloc(chroma_stride * srcHeightC);
            if (!pCr)
                return ;
        }
    }

    //2.1 load data from FPGA memory + endian
    vdi_read_memory(coreIdx, fbSrc->bufY, pY, stride * srcHeightY, endian);
    if (srcHeightC != 0) {
        if (!pCb)
            return ;
        vdi_read_memory(coreIdx, fbSrc->bufCb, pCb, chroma_stride * srcHeightC, endian);
        if (interLeave != TRUE) {
            if (!pCr)
                return ;
            vdi_read_memory(coreIdx, fbSrc->bufCr, pCr, chroma_stride * srcHeightC, endian);
        }
    }
#if defined(SAVE_YUV)
#else
    DePxlOrder(stride, srcHeightY, chroma_stride, srcHeightC,
        interLeave, is10bit, is3pxl4byte, isMSB, pY, pCb, pCr);
#endif

    //3.1 : Luma YUV source generate
    puc = pSrc;
    for (y = 0; y < srcHeightY; y += 1) {
        pix_addr = GetXY2AXIAddr(&mapCfg, 0, y + offsetY, 0, stride, fbSrc);
        rowBufferY = pY + (pix_addr - baseY);
        osal_memcpy(puc + y*srcWidthY, rowBufferY + offsetX, srcWidthY);
    }

    //3.2 : Chroma YUV source generate
    if (srcHeightC != 0) {
        //Cb
        puc = pSrc + srcWidthY*srcHeightY;
        if ( !pCb )
            return ;
        for (y = 0; y < srcHeightC; y += 1) {
            pix_addr = GetXY2AXIAddr(&mapCfg, 2, y + (offsetY / 2), 0, chroma_stride, fbSrc);
            rowBufferCb = pCb + (pix_addr - baseCb);
            osal_memcpy(puc + (y*srcWidthC), rowBufferCb + (offsetX / 2), srcWidthC);
        }

        //Cr
        if (interLeave != TRUE) {
            if ( !pCr )
                return ;
            puc = puc + srcWidthC*srcHeightC;
            for (y = 0; y < srcHeightC; y += 1) {
                pix_addr = GetXY2AXIAddr(&mapCfg, 3, y + (offsetY / 2), 0, chroma_stride, fbSrc);
                rowBufferCr = pCr + (pix_addr - baseCr);
                osal_memcpy(puc + (y*srcWidthC), rowBufferCr + (offsetX / 2), srcWidthC);
            }
        }
    }

    if (pY)
        osal_free(pY);
    if (pCb)
        osal_free(pCb);
    if (pCr)
        osal_free(pCr);
}
BOOL CalcYuvSize_412(
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
)
{
#if defined(SAVE_YUV)
    return CalcYuvSize(format, picWidth, picHeight, cbcrInterleave, lumaSize, chromaSize, frameSize, NULL, NULL, NULL);
#else
    if (format >= FORMAT_YUYV) {
        if (format == FORMAT_YUYV || format == FORMAT_YVYU || format == FORMAT_UYVY || format == FORMAT_VYUY)
            format = FORMAT_420;
        else
            format = FORMAT_420_P10_16BIT_LSB;
    }
    else if (format == FORMAT_422_P10_16BIT_LSB || format == FORMAT_422_P10_16BIT_MSB)
        format = FORMAT_420_P10_16BIT_LSB;
    else if (format == FORMAT_422_P10_32BIT_LSB || format == FORMAT_422_P10_32BIT_MSB)
        format = FORMAT_420_P10_16BIT_LSB;
    else if (format == FORMAT_420_P10_32BIT_LSB || format == FORMAT_420_P10_32BIT_MSB)
        format = FORMAT_420_P10_16BIT_LSB;
    else if (format == FORMAT_422)
        format = FORMAT_420;

    return CalcYuvSize(format, picWidth, picHeight, 0/*fb->cbcrInterleave*/, lumaSize, chromaSize, frameSize, NULL, NULL, NULL);
#endif
}
void GeneratePicParam(
    FrameBuffer* fbSrc,
    VpuRect      cropRect,
    BOOL         enableCrop,
    Uint32*      is422,
    Uint32*      isPack,
    Uint32*      PackMode,
    Uint32*      is10bit,
    Uint32*      isMSB,
    Uint32*      is3pxl4byte,
    Uint32*      srcWidthY,
    Uint32*      srcHeightY,
    Uint32*      srcWidthC,
    Uint32*      srcHeightC,
    Uint32*      chroma_stride,
    Uint32*      dstWidthY,
    Uint32*      dstHeightY,
    Uint32*      dstWidthC,
    Uint32*      dstHeightC
    )
{

    Uint32      cropWidth;
    Uint32      cropHeight;

    Uint32      stride      = fbSrc->stride;
    Uint32      height      = fbSrc->height;

    int         interLeave  = fbSrc->cbcrInterleave;

    cropWidth  = (enableCrop == TRUE ? cropRect.right - cropRect.left : stride);
    cropHeight = (enableCrop == TRUE ? cropRect.bottom - cropRect.top : height);

    //initial setting
    *isPack      = 0;
    *PackMode    = 0;
    *is10bit     = 0;
    *isMSB       = 0;
    *is3pxl4byte = 0;
    *is422       = 0;
    *srcWidthY   = cropWidth;
    *srcHeightY  = cropHeight;
    *srcWidthC   = cropWidth / 2;
    *srcHeightC  = cropHeight / 2;

    *dstWidthY   = cropWidth;
    *dstHeightY  = cropHeight;
    *dstWidthC   = cropWidth / 2;
    *dstHeightC  = cropHeight / 2;

    switch (fbSrc->format) {
        case FORMAT_420               :
            break;
        case FORMAT_420_P10_16BIT_LSB :
            *is10bit = 1;
            break;
        case FORMAT_420_P10_16BIT_MSB :
            *is10bit = 1;
            *isMSB = 1;
            break;
        case FORMAT_420_P10_32BIT_LSB :
            *is3pxl4byte = 1;
            *is10bit = 1;
            break;
        case FORMAT_420_P10_32BIT_MSB :
            *is3pxl4byte = 1;
            *is10bit = 1;
            *isMSB = 1;
            break;
        case FORMAT_422               :
            *is422 = 1;
            break;
        case FORMAT_422_P10_16BIT_MSB :
            *is422 = 1;
            *is10bit = 1;
            *isMSB = 1;
            break;
        case FORMAT_422_P10_16BIT_LSB :
            *is422 = 1;
            *is10bit = 1;
            break;
        case FORMAT_422_P10_32BIT_MSB :
            *is422 = 1;
            *is3pxl4byte = 1;
            *is10bit = 1;
            *isMSB = 1;
            break;
        case FORMAT_422_P10_32BIT_LSB :
            *is422 = 1;
            *is3pxl4byte = 1;
            *is10bit = 1;
            break;
        case FORMAT_YUYV              :
            *isPack = 1;
            break;
        case FORMAT_YVYU              :
            *isPack = 1;
            *PackMode = 1;
            break;
        case FORMAT_UYVY              :
            *isPack = 1;
            *PackMode = 2;
            break;
        case FORMAT_VYUY              :
            *isPack = 1;
            *PackMode = 3;
            break;
        case FORMAT_YUYV_P10_16BIT_LSB:
            *is10bit = 1;
            *isPack = 1;
            break;
        case FORMAT_YUYV_P10_16BIT_MSB:
            *is10bit = 1;
            *isMSB = 1;
            *isPack = 1;
            break;
        case FORMAT_YVYU_P10_16BIT_LSB:
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 1;
            break;
        case FORMAT_YVYU_P10_16BIT_MSB:
            *is10bit = 1;
            *isMSB = 1;
            *isPack = 1;
            *PackMode = 1;
            break;
        case FORMAT_UYVY_P10_16BIT_LSB:
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 2;
            break;
        case FORMAT_UYVY_P10_16BIT_MSB:
            *is10bit = 1;
            *isMSB = 1;
            *isPack = 1;
            *PackMode = 2;
            break;
        case FORMAT_VYUY_P10_16BIT_LSB:
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 3;
            break;
        case FORMAT_VYUY_P10_16BIT_MSB:
            *is10bit = 1;
            *isMSB = 1;
            *isPack = 1;
            *PackMode = 3;
            break;
        case FORMAT_YUYV_P10_32BIT_LSB:
            *is3pxl4byte = 1;
            *is10bit = 1;
            *isPack = 1;
            break;
        case FORMAT_YUYV_P10_32BIT_MSB:
            *is3pxl4byte = 1;
            *isMSB = 1;
            *is10bit = 1;
            *isPack = 1;
            break;
        case FORMAT_YVYU_P10_32BIT_LSB:
            *is3pxl4byte = 1;
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 1;
            break;
        case FORMAT_YVYU_P10_32BIT_MSB:
            *is3pxl4byte = 1;
            *isMSB = 1;
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 1;
            break;
        case FORMAT_UYVY_P10_32BIT_LSB:
            *is3pxl4byte = 1;
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 2;
            break;
        case FORMAT_UYVY_P10_32BIT_MSB:
            *is3pxl4byte = 1;
            *isMSB = 1;
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 2;
            break;
        case FORMAT_VYUY_P10_32BIT_LSB:
            *is3pxl4byte = 1;
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 3;
            break;
        case FORMAT_VYUY_P10_32BIT_MSB:
            *is3pxl4byte = 1;
            *isMSB = 1;
            *is10bit = 1;
            *isPack = 1;
            *PackMode = 3;
            break;
        default:
           break;
    }

    //Luma src width
    if (*is10bit) {
        if (*is3pxl4byte)
            *srcWidthY = ((cropWidth + 2) / 3 * 4);
        else
            *srcWidthY = cropWidth * 2;
    }
    if (*isPack)
        *srcWidthY = *srcWidthY * 2;

    //Chroma src width
    if (interLeave == TRUE) {
        if (*is10bit) {
            if (*is3pxl4byte)
                *srcWidthC = ((cropWidth + 2) / 3 * 4);
            else
                *srcWidthC = cropWidth * 2;
        }
        else
            *srcWidthC = cropWidth;
    }
    else {
        if (*is10bit) {
            if (*is3pxl4byte)
                *srcWidthC = (((cropWidth / 2) + 2) / 3 * 4);
            else
                *srcWidthC = cropWidth;
        }
    }

    //Chroma src height
    if (*is422 == TRUE)
        *srcHeightC = cropHeight;
    else if (*isPack)
        *srcHeightC = 0;

//Chroma stride
    if (interLeave == TRUE)
        *chroma_stride = stride;
    else
        *chroma_stride = stride / 2;

//Chroma dst height
    *dstHeightC = cropHeight / 2;

    if (*is10bit == TRUE) {
        *dstWidthY = cropWidth * 2;
        *dstWidthC = cropWidth;
    }
}

Uint32 StoreYuvImageBurstLinear(
    Uint32      coreIdx,
    FrameBuffer *fbSrc,
    TiledMapConfig  mapCfg,
    Uint8       *pDst,
    VpuRect     cropRect,
    BOOL        enableCrop,
    BOOL        isVP9
    )
{
    Uint32          y, x;
    Uint32          pix_addr, div_x, div_y, chroma_stride=0;
    Uint8*          puc;
    Uint8*          rowBufferY, *rowBufferCb, *rowBufferCr;
    Uint32          stride      = fbSrc->stride;
    Uint32          height      = fbSrc->height;
    int             interLeave  = fbSrc->cbcrInterleave;
    BOOL            nv21        = fbSrc->nv21;
    EndianMode      endian      = (EndianMode)fbSrc->endian;
    FrameBufferFormat format    = (FrameBufferFormat)fbSrc->format;
    Uint32          width;
    Uint32          dstWidth=0, dstHeight=0;
    Uint32          offsetX, offsetY;
    Uint32          dstChromaHeight = 0;
    Uint32          dstChromaWidth = 0;
    Uint32          chromaHeight = 0;
    Uint32          bpp=8;
    Uint32          p10_32bit_interleave = 0;
    Int32           productId;
    Int32           dramBusWidth = 8;
    Uint32          totSize = 0;
    BOOL            copyLumaOnly = FALSE;

    //Int32           addr;
    Int32           baseY;
    Int32           baseCb;
    Int32           baseCr;
    Uint8           *pY;
    Uint8           *pCbTemp;
    Uint8           *pCb;
    Uint8           *pCr;

    productId = VPU_GetProductId(coreIdx);
    if (PRODUCT_ID_W_SERIES(productId)) {
        dramBusWidth = 16;
    }
    switch (fbSrc->format) {
    case FORMAT_420:
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_422:
    case FORMAT_422_P10_16BIT_LSB:
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_32BIT_LSB:
    case FORMAT_422_P10_32BIT_MSB:
        div_x = 2;
        break;
    default:
        div_x = 1;
    }

    switch (fbSrc->format) {
    case FORMAT_420:
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_224:
        div_y = 2;
        break;
    default:
        div_y = 1;
    }

    //for matching with Ref-C
    width     = (enableCrop == TRUE ? cropRect.right - cropRect.left : stride);
    dstHeight = (enableCrop == TRUE ? cropRect.bottom - cropRect.top : height);
    offsetX   = (enableCrop == TRUE ? cropRect.left : 0);
    offsetY   = (enableCrop == TRUE ? cropRect.top  : 0);

    switch (fbSrc->format) {
    case FORMAT_400:
        copyLumaOnly = TRUE;
        break;
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
        copyLumaOnly    = TRUE;
        dstWidth        = width * 2;
        dstChromaHeight = 0;
        chromaHeight    = 0;
        break;
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
        copyLumaOnly    = TRUE;
        dstWidth        = (width * 2)*2;
        dstChromaHeight = 0;
        chromaHeight    = 0;
        break;
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
        copyLumaOnly    = TRUE;
        dstWidth        = ((width+2)/3*4)*2;
        dstChromaHeight = 0;
        chromaHeight    = 0;
        break;
    case FORMAT_422_P10_16BIT_LSB:
    case FORMAT_422_P10_16BIT_MSB:
        dstWidth = width * 2;
        bpp = 16;
        dstChromaWidth  = dstWidth / div_x;
        dstChromaHeight = dstHeight / div_y;
        chromaHeight    = height / div_y;
        chroma_stride   = (stride / div_x);
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        dstWidth = width * 2;
        bpp = 16;
        dstChromaWidth  = dstWidth / div_x;
        dstChromaHeight = dstHeight / div_y;
        chromaHeight    = height / div_y;
        chroma_stride = (stride / div_x);
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
#ifdef DUMP_YUV_WHOLE_DATA
        if (interLeave)
        {
            dstChromaWidth = ((VPU_ALIGN16(width/div_x))*2+11)/12*16;
            dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            dstChromaHeight = dstHeight / div_y;

            stride = stride;
            chroma_stride = stride;
            dstWidth = (VPU_ALIGN16(width)+11)/12*16;

            interLeave = 0;
        }
        else
        {
            dstChromaWidth = ((VPU_ALIGN16(width/div_x))+11)/12*16;
            dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            dstChromaHeight = dstHeight / div_y;
            chroma_stride = dstChromaWidth;
            stride    = (VPU_ALIGN16(stride)+11)/12*16;
            dstWidth = (VPU_ALIGN16(dstWidth)+11)/12*16;
        }
        break;
#else
        if (interLeave) {
            dstChromaWidth = ((VPU_ALIGN16(width*2/div_x))+11)/12*16;
            dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            if(isVP9 == TRUE) {
                dstChromaWidth = VPU_ALIGN32(dstChromaWidth);
            }
            chroma_stride = stride;

            dstChromaWidth = (width/div_x+2)/3*4;

            dstChromaHeight = dstHeight / div_y;
            chromaHeight    = height / div_y;

            dstWidth = (width+2)/3*4;

            interLeave = 0;
            p10_32bit_interleave = 1;
        }
        else {
            //dstChromaWidth = ((VPU_ALIGN16(width/div_x))+11)/12*16;
//          dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            //chroma_stride = dstChromaWidth;
            chroma_stride = stride / 2;

            dstChromaWidth = (width/2+2)/3*4;

            dstChromaHeight = dstHeight / div_y;
            chromaHeight    = height / div_y;

            dstWidth = (width+2)/3*4;
        }
        break;
#endif
    default:
        dstWidth = width;
        dstChromaWidth  = width / div_x;
        dstChromaHeight = dstHeight / div_y;
        chromaHeight    = height / div_y;
        chroma_stride   = (stride / div_x);
        break;
    }

    puc         = pDst;

    pY = (Uint8*)osal_malloc(stride * height);
    if ( !pY )
        return 0;
    pCbTemp = (Uint8*)osal_malloc(stride*4 * chromaHeight);
    if ( !pCbTemp )
        return 0;
    pCb = (Uint8*)osal_malloc(stride*4 * chromaHeight);
    if ( !pCb )
        return 0;
    pCr = (Uint8*)osal_malloc(stride*2 * chromaHeight);
    if ( !pCr )
        return 0;
    baseY = fbSrc->bufY;
    baseCb = fbSrc->bufCb;
    baseCr = fbSrc->bufCr;

    vdi_read_memory(coreIdx, fbSrc->bufY, pY, stride * height, endian);

    for (y=0 ; y<dstHeight ; y+=1) {
        pix_addr = GetXY2AXIAddr(&mapCfg, 0, y+offsetY, 0, stride, fbSrc);
        rowBufferY = pY + (pix_addr - baseY);
        // CHECK POINT
        for (x=0; x<stride ; x+=dramBusWidth) {
            if ( fbSrc->format == FORMAT_420_P10_32BIT_MSB )
                SwapPixelOrder(rowBufferY+x);
        }
        // CHECK POINT
        osal_memcpy(puc+y*dstWidth, rowBufferY+offsetX, dstWidth);
        totSize += dstWidth;
    }

    if (copyLumaOnly == TRUE) {
        osal_free(pY);
        osal_free(pCb);
        osal_free(pCr);
        osal_free(pCbTemp);
        return totSize;
    }

    if (interLeave || p10_32bit_interleave) {
        Int32    cbcr_per_2pix=1;

        cbcr_per_2pix = (format==FORMAT_224||format==FORMAT_444) ? 2 : 1;
        vdi_read_memory(coreIdx, fbSrc->bufCb, pCbTemp, stride*cbcr_per_2pix * chromaHeight, endian);
    } else {
        vdi_read_memory(coreIdx, fbSrc->bufCb, pCb, chroma_stride * chromaHeight, endian);
        if ( (fbSrc->format == FORMAT_420_P10_32BIT_LSB || fbSrc->format == FORMAT_420_P10_32BIT_MSB) &&
            p10_32bit_interleave == 1) {
            // Nothing to do
        }
        else {
            vdi_read_memory(coreIdx, fbSrc->bufCr, pCr, chroma_stride * chromaHeight, endian);
        }
    }

    if (interLeave == TRUE || p10_32bit_interleave == TRUE) {
        //Uint8    pTemp[16];
        Uint8*   pTemp;
        Uint8*   dstAddrCb;
        Uint8*   dstAddrCr;
        Uint8*   ptrCb, *ptrCr;
        Int32    cbcr_per_2pix=1, k;
        Uint32*   pTempLeft32, *pTempRight32;
        Uint32   temp_32;

        dstAddrCb = pDst + dstWidth*dstHeight;
        dstAddrCr = dstAddrCb + dstChromaWidth*dstChromaHeight;

        cbcr_per_2pix = (format==FORMAT_224||format==FORMAT_444) ? 2 : 1;

        for ( y = 0 ; y < dstChromaHeight; ++y ) {
            ptrCb = pCb;
            ptrCr = pCr;
            for ( x = 0 ; x < stride*cbcr_per_2pix ; x += dramBusWidth ) {
                pix_addr = GetXY2AXIAddr(&mapCfg, 2, y+(offsetY/div_y), x, stride, fbSrc);
                pTemp = pCbTemp + (pix_addr - baseCb);
                // CHECK POINT
                if ( fbSrc->format == FORMAT_420_P10_32BIT_MSB )
                    SwapPixelOrder(pTemp);
                // CHECK POINT

                if (interLeave == TRUE) {
                    for (k=0; k<dramBusWidth && (x+k) < stride; k+=(2*bpp/8)) {
                        if (bpp == 8) {
                            if (nv21) {
                                *ptrCr++ = pTemp[k];
                                *ptrCb++ = pTemp[k+1];
                            }
                            else {
                                *ptrCb++ = pTemp[k];
                                *ptrCr++ = pTemp[k+1];
                            }
                        }
                        else {
                            if (nv21) {
                                *ptrCr++ = pTemp[k];
                                *ptrCr++ = pTemp[k+1];
                                *ptrCb++ = pTemp[k+2];
                                *ptrCb++ = pTemp[k+3];
                            }
                            else {
                                *ptrCb++ = pTemp[k];
                                *ptrCb++ = pTemp[k+1];
                                *ptrCr++ = pTemp[k+2];
                                *ptrCr++ = pTemp[k+3];
                            }
                        }
                    }
                }
                else {
                    for (k=0; k<dramBusWidth && (x+k) < stride; k+=8) {//(2*bpp/8)) {
                        pTempLeft32 = (Uint32*)&pTemp[k];
                        pTempRight32 = (Uint32*)&pTemp[k+4];

                        if (format==FORMAT_420_P10_32BIT_MSB) {
                            temp_32 = *pTempLeft32 & 0x003ff000;
                            *pTempLeft32 = (*pTempLeft32 & 0xffc00000)
                                | (*pTempLeft32 & 0x00000ffc) << 10
                                | (*pTempRight32 & 0x003ff000) >> 10;
                            *pTempRight32 = (temp_32) << 10
                                | (*pTempRight32 & 0xffc00000) >> 10
                                | (*pTempRight32 & 0x00000ffc);
                        }
                        else if (format==FORMAT_420_P10_32BIT_LSB) {
                            temp_32 = *pTempLeft32 & 0x000ffc00;
                            *pTempLeft32 = (*pTempLeft32 & 0x000003ff)
                                | (*pTempLeft32 & 0x3ff00000) >> 10
                                | (*pTempRight32 & 0x000ffc00) << 10;
                            *pTempRight32 = (temp_32) >> 10
                                | (*pTempRight32 & 0x000003ff) << 10
                                | (*pTempRight32 & 0x3ff00000);
                        }

                        if (nv21) {
                            *ptrCr++ = pTemp[k];
                            *ptrCr++ = pTemp[k+1];
                            *ptrCr++ = pTemp[k+2];
                            *ptrCr++ = pTemp[k+3];
                            *ptrCb++ = pTemp[k+4];
                            *ptrCb++ = pTemp[k+5];
                            *ptrCb++ = pTemp[k+6];
                            *ptrCb++ = pTemp[k+7];
                        }
                        else {
                            *ptrCb++ = pTemp[k];
                            *ptrCb++ = pTemp[k+1];
                            *ptrCb++ = pTemp[k+2];
                            *ptrCb++ = pTemp[k+3];
                            *ptrCr++ = pTemp[k+4];
                            *ptrCr++ = pTemp[k+5];
                            *ptrCr++ = pTemp[k+6];
                            *ptrCr++ = pTemp[k+7];
                        }
                    }
                }
            }
            osal_memcpy(dstAddrCb+y*dstChromaWidth, pCb+offsetX/div_x, dstChromaWidth);
            totSize += dstChromaWidth;
            osal_memcpy(dstAddrCr+y*dstChromaWidth, pCr+offsetX/div_x, dstChromaWidth);
            totSize += dstChromaWidth;
        }
    }
    else {
        puc = pDst + dstWidth*dstHeight;

        for (y = 0 ; y < dstChromaHeight; y += 1) {
            x = 0;
            pix_addr = GetXY2AXIAddr(&mapCfg, 2, y+(offsetY/div_y), x, chroma_stride, fbSrc);
            rowBufferCb = pCb + (pix_addr - baseCb);
            // CHECK POINT
            for (x = 0 ; x < chroma_stride; x += dramBusWidth) {
                if ( fbSrc->format == FORMAT_420_P10_32BIT_MSB )
                    SwapPixelOrder(rowBufferCb+x);
            }
            // CHECK POINT
            osal_memcpy(puc + (y*dstChromaWidth), rowBufferCb+offsetX/div_x, dstChromaWidth);
            totSize += dstChromaWidth;
        }

        puc += dstChromaWidth * dstChromaHeight;
        if ( (fbSrc->format == FORMAT_420_P10_32BIT_LSB || fbSrc->format == FORMAT_420_P10_32BIT_MSB) &&
            p10_32bit_interleave == 1)
        {
        }
        else
        {
            for (y = 0 ; y < dstChromaHeight; y += 1) {
                x = 0;
                pix_addr = GetXY2AXIAddr(&mapCfg, 3, y+(offsetY/div_y), x, chroma_stride, fbSrc);
                //vdi_read_memory(coreIdx, pix_addr, rowBufferCr+x, dramBusWidth,  endian);
                rowBufferCr = pCr + (pix_addr - baseCr);
                // CHECK POINT
                for ( x = 0 ; x < chroma_stride; x += dramBusWidth ) {
                    if ( fbSrc->format == FORMAT_420_P10_32BIT_MSB )
                        SwapPixelOrder(rowBufferCr+x);
                }
                // CHECK POINT
                osal_memcpy(puc + (y*dstChromaWidth), rowBufferCr+offsetX/div_x, dstChromaWidth);
                totSize += dstChromaWidth;
            }
        }
    }

    osal_free(pY);
    osal_free(pCb);
    osal_free(pCr);
    osal_free(pCbTemp);

    return totSize;
}

Uint32 StoreYuvImageBurstFormat(
    Uint32          coreIdx,
    FrameBuffer*    fbSrc,
    TiledMapConfig  mapCfg,
    Uint8*          pDst,
    VpuRect         cropRect,
    BOOL            enableCrop
    )
{
    Uint32          y, x;
    Uint32          pix_addr, div_x, div_y, chroma_stride;
    Uint8*          puc;
    Uint8*          rowBufferY, *rowBufferCb, *rowBufferCr;
    Uint32          stride      = fbSrc->stride;
    Uint32          height      = fbSrc->height;
    int             interLeave  = fbSrc->cbcrInterleave;
    BOOL            nv21        = fbSrc->nv21;
    EndianMode      endian      = (EndianMode)fbSrc->endian;
    FrameBufferFormat format    = (FrameBufferFormat)fbSrc->format;
    Uint32          width;
    Uint32          dstWidth, dstHeight;
    Uint32          offsetX, offsetY;
    Uint32          dstChromaHeight;
    Uint32          dstChromaWidth;
    Uint32          bpp=8;
    Uint32          p10_32bit_interleave = 0;
    Int32           productId;
    Int32           dramBusWidth = 8;
        Uint32                  totSize = 0;

    productId = VPU_GetProductId(coreIdx);
    if (PRODUCT_ID_W_SERIES(productId)) {
        dramBusWidth = 16;
    }
    switch (fbSrc->format) {
    case FORMAT_420:
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_422:
    case FORMAT_422_P10_16BIT_LSB:
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_32BIT_LSB:
    case FORMAT_422_P10_32BIT_MSB:
        div_x = 2;
        break;
    default:
        div_x = 1;
    }

    switch (fbSrc->format) {
    case FORMAT_420:
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_224:
        div_y = 2;
        break;
    default:
        div_y = 1;
    }

    width     = (enableCrop == TRUE ? cropRect.right - cropRect.left : stride);
    dstHeight = (enableCrop == TRUE ? cropRect.bottom - cropRect.top : height);
    offsetX   = (enableCrop == TRUE ? cropRect.left : 0);
    offsetY   = (enableCrop == TRUE ? cropRect.top  : 0);

    switch (fbSrc->format) {
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        dstWidth = width * 2;
        bpp = 16;
        dstChromaWidth  = dstWidth / div_x;
        dstChromaHeight = dstHeight / div_y;
        chroma_stride   = (stride / div_x);
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
#ifdef DUMP_YUV_WHOLE_DATA
        if (interLeave)
        {
            dstChromaWidth = ((VPU_ALIGN16(width/div_x))*2+11)/12*16;
            dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            dstChromaHeight = dstHeight / div_y;

            stride = stride;
            chroma_stride = stride;
            dstWidth = (VPU_ALIGN16(width)+11)/12*16;

            interLeave = 0;
        }
        else
        {
            dstChromaWidth = ((VPU_ALIGN16(width/div_x))+11)/12*16;
            dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            dstChromaHeight = dstHeight / div_y;
            chroma_stride = dstChromaWidth;
            stride    = (VPU_ALIGN16(stride)+11)/12*16;
            dstWidth = (VPU_ALIGN16(dstWidth)+11)/12*16;
        }
        break;
#else
        if (interLeave)
        {
            dstChromaWidth = ((VPU_ALIGN16(width*2/div_x))+11)/12*16;
            dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            chroma_stride = stride;

            dstChromaWidth = (width+2)/3*4;
            dstChromaHeight = dstHeight / div_y;

            dstWidth = (width+2)/3*4;

            interLeave = 0;
            p10_32bit_interleave = 1;
        }
        else
        {
            dstChromaWidth = ((VPU_ALIGN16(width/div_x))+11)/12*16;
            dstChromaWidth = VPU_ALIGN16(dstChromaWidth);
            chroma_stride = dstChromaWidth;

            dstChromaWidth = (width/2+2)/3*4;
            dstChromaHeight = dstHeight / div_y;

            dstWidth = (width+2)/3*4;
        }
        break;
#endif
    case FORMAT_YUYV:
    case FORMAT_YUYV_P10_16BIT_MSB:
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY:
    case FORMAT_VYUY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        dstWidth        = stride;
        dstChromaWidth  = 0;
        dstChromaHeight = 0;
        chroma_stride   = 0;
        break;
    default:
        dstWidth = width;
        dstChromaWidth  = width / div_x;
        dstChromaHeight = dstHeight / div_y;
        chroma_stride = (stride / div_x);
        break;
    }

    puc         = pDst;
    rowBufferY  = (Uint8*)osal_malloc(stride);
    if ( !rowBufferY )
        return 0;
    rowBufferCb = (Uint8*)osal_malloc(stride*4);
    if ( !rowBufferCb )
        return 0;
    rowBufferCr = (Uint8*)osal_malloc(stride*2);
    if ( !rowBufferCr )
        return 0;

    for ( y=0 ; y<dstHeight ; y+=1 )
    {
        for ( x=0; x<stride ; x+=dramBusWidth )
        {
            pix_addr = GetXY2AXIAddr(&mapCfg, 0, y+offsetY, x, stride, fbSrc);
            vdi_read_memory(coreIdx, pix_addr, rowBufferY+x, dramBusWidth,  endian);
        }
        osal_memcpy(puc+y*dstWidth, rowBufferY+offsetX, dstWidth);
                totSize += dstWidth;
    }

    if (format == FORMAT_400) {
        osal_free(rowBufferY);
        osal_free(rowBufferCb);
        osal_free(rowBufferCr);
        return totSize;
    }

    if (interLeave == TRUE) {
        Uint8    pTemp[16];
        Uint8*   dstAddrCb;
        Uint8*   dstAddrCr;
        Uint8*   ptrCb, *ptrCr;
        Int32    cbcr_per_2pix=1, k;

        dstAddrCb = pDst + dstWidth*dstHeight;
        dstAddrCr = dstAddrCb + dstChromaWidth*dstChromaHeight;

        cbcr_per_2pix = (format==FORMAT_224||format==FORMAT_444) ? 2 : 1;

        for ( y = 0 ; y < dstChromaHeight; ++y ) {
            ptrCb = rowBufferCb;
            ptrCr = rowBufferCr;
            for ( x = 0 ; x < stride*cbcr_per_2pix ; x += dramBusWidth ) {
                pix_addr = GetXY2AXIAddr(&mapCfg, 2, y+(offsetY/div_y), x, stride, fbSrc);
                vdi_read_memory(coreIdx, pix_addr,  pTemp, dramBusWidth,  endian);
                // CHECK POINT
                if ( fbSrc->format == FORMAT_420_P10_32BIT_MSB )
                    SwapPixelOrder(pTemp);
                // CHECK POINT
                for (k=0; k<dramBusWidth && (x+k) < stride; k+=(2*bpp/8)) {
                    if (bpp == 8) {
                        if (nv21) {
                            *ptrCr++ = pTemp[k];
                            *ptrCb++ = pTemp[k+1];
                        }
                        else {
                            *ptrCb++ = pTemp[k];
                            *ptrCr++ = pTemp[k+1];
                        }
                    }
                    else {
                        if (nv21) {
                            *ptrCr++ = pTemp[k];
                            *ptrCr++ = pTemp[k+1];
                            *ptrCb++ = pTemp[k+2];
                            *ptrCb++ = pTemp[k+3];
                        }
                        else {
                            *ptrCb++ = pTemp[k];
                            *ptrCb++ = pTemp[k+1];
                            *ptrCr++ = pTemp[k+2];
                            *ptrCr++ = pTemp[k+3];
                        }
                    }
                }
            }
            osal_memcpy(dstAddrCb+y*dstChromaWidth, rowBufferCb+offsetX/div_x, dstChromaWidth);
                        totSize += dstChromaWidth;
            osal_memcpy(dstAddrCr+y*dstChromaWidth, rowBufferCr+offsetX/div_x, dstChromaWidth);
                        totSize += dstChromaWidth;
        }
    }
    else {
        puc = pDst + dstWidth*dstHeight;

        for (y = 0 ; y < dstChromaHeight; y += 1) {
            for (x = 0 ; x < chroma_stride; x += dramBusWidth) {
                pix_addr = GetXY2AXIAddr(&mapCfg, 2, y+(offsetY/div_y), x, chroma_stride, fbSrc);
                vdi_read_memory(coreIdx, pix_addr, rowBufferCb+x, dramBusWidth,  endian);
            }
            osal_memcpy(puc + (y*dstChromaWidth), rowBufferCb+offsetX/div_x, dstChromaWidth);
                        totSize += dstChromaWidth;
        }

        puc += dstChromaWidth * dstChromaHeight;
        if ( (fbSrc->format == FORMAT_420_P10_32BIT_LSB || fbSrc->format == FORMAT_420_P10_32BIT_MSB) &&
            p10_32bit_interleave == 1)
        {
        }
        else
        {
            for (y = 0 ; y < dstChromaHeight; y += 1) {
                for ( x = 0 ; x < chroma_stride; x += dramBusWidth ) {
                    pix_addr = GetXY2AXIAddr(&mapCfg, 3, y+(offsetY/div_y), x, chroma_stride, fbSrc);
                    vdi_read_memory(coreIdx, pix_addr, rowBufferCr+x, dramBusWidth,  endian);
                }
                osal_memcpy(puc + (y*dstChromaWidth), rowBufferCr+offsetX/div_x, dstChromaWidth);
                                totSize += dstChromaWidth;
            }
        }
    }

    osal_free(rowBufferY);
    osal_free(rowBufferCb);
    osal_free(rowBufferCr);

    return totSize;
}

Uint8* GetYUVFromFrameBuffer(
    DecHandle       decHandle,
    FrameBuffer*    fb,
    VpuRect         rcFrame,
    Uint32*         retWidth,
    Uint32*         retHeight,
    Uint32*         retBpp,
    size_t*         retSize
    )
{
    Uint32          coreIdx = VPU_HANDLE_CORE_INDEX(decHandle);
    size_t          frameSizeY;                                         // the size of luma
    size_t          frameSizeC;                                         // the size of chroma
    size_t          frameSize;                                          // the size of frame
    Uint32          Bpp = 1;                                            //!<< Byte per pixel
    Uint32          picWidth, picHeight;
    Uint8*          pYuv;
    TiledMapConfig  mapCfg;

    picWidth  = rcFrame.right - rcFrame.left;
    picHeight = rcFrame.bottom - rcFrame.top;

    CalcYuvSize_412(fb->format, picWidth, fb->height, fb->cbcrInterleave, &frameSizeY, &frameSizeC, &frameSize, NULL, NULL, NULL);

    switch (fb->format) {
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        Bpp = 2;
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
        picWidth = (picWidth/3)*4 + ((picWidth%3) ? 4 : 0);
        Bpp = 1;
        break;
    case FORMAT_422:
    case FORMAT_422_P10_32BIT_MSB:
    case FORMAT_422_P10_32BIT_LSB:
        break;
    default:
        Bpp = 1;
        break;
    }
    if ((pYuv=(Uint8*)osal_malloc(frameSize)) == NULL) {
        VLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    VPU_DecGiveCommand(decHandle, GET_TILEDMAP_CONFIG, &mapCfg);
    *retSize = StoreYuvImageBurst(coreIdx, fb, mapCfg, pYuv, rcFrame, TRUE);

    *retWidth  = picWidth;
    *retHeight = picHeight;
    *retBpp    = Bpp;

    return pYuv;
}

void PrepareDecoderTest(
    DecHandle decHandle
    )
{
    UNREFERENCED_PARAMETER(decHandle);
}

int ProcessEncodedBitstreamBurst(Uint32 coreIdx, osal_file_t fp, int targetAddr,
    PhysicalAddress bsBufStartAddr, PhysicalAddress bsBufEndAddr,
    int size, int endian, Comparator comparator)
{
    Uint8 * buffer = 0;
    int room = 0;
    int file_wr_size = 0;

    buffer = (Uint8 *)osal_malloc(size);
    if( ( targetAddr + size ) > (int)bsBufEndAddr )
    {
        room = bsBufEndAddr - targetAddr;
        vdi_read_memory(coreIdx, targetAddr, buffer, room,  endian);
        vdi_read_memory(coreIdx, bsBufStartAddr, buffer+room, (size-room), endian);
    }
    else
    {
        vdi_read_memory(coreIdx, targetAddr, buffer, size, endian);
    }

    if ( comparator) {
        if (Comparator_Act(comparator, buffer, size) == FALSE) {
            osal_free(buffer);
            return 0;
        }
    }

    if (fp) {
        file_wr_size = osal_fwrite(buffer, sizeof(Uint8), size, fp);
        osal_fflush(fp);
    }

    osal_free( buffer );

    return file_wr_size;
}

#ifdef HLS_TEST
#endif

Uint32 CalcScaleDown(
    Uint32   origin,
    Uint32   scaledValue
    )
{
    Uint32   minScaleValue;
    Uint32   retVal;

    minScaleValue = ((origin/8)+7)&~7;
    minScaleValue = (minScaleValue < 8) ? 8 : minScaleValue;
    if (origin == 99) {
        retVal = GetRandom(minScaleValue, origin);
        retVal = VPU_ALIGN8(retVal);
    }
    else {
        if (scaledValue == 0) {
            retVal = origin;
        }
        else {
            if ( scaledValue < origin ) {
                retVal = VPU_ALIGN8(scaledValue);
                if (retVal < minScaleValue) {
                    retVal = minScaleValue;
                }
            } else {
                retVal = origin;
            }
        }
    }

    return retVal;
}

