//--=========================================================================--
//  This file is a part of QC Tool project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2004 - 2011   CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "main_helper.h"
#include <libavformat/avformat.h>
#ifdef SUPPORT_ENCODE_CUSTOM_HEADER
#include "header_struct.h"
#include "pbu.h"
#endif

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


Int32 ConvFOURCCToMp4Class(
    Int32   fourcc
    )
{
    Int32   i;
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
    Int32   i;

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
    Int32   i;

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
    Int32   i;

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
    Int32   i;

    for(i=0; i<sizeof(codstd_tab)/sizeof(codstd_tab[0]); i++) {
        if (codstd_tab[i].codecId == codecId) {
            fourcc = codstd_tab[i].fourcc;
            break;
        }
    }
    return fourcc;
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
    Int32               addr;
    size_t              lumaSize, chromaSize=0, chromaStride, chromaWidth=0;
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

    return TRUE;
}

BOOL LoadTiledImageYuvBurst(
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
    size_t              pix_addr;
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
    Uint32          chromaHeight;
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
    pCbTemp = (Uint8*)osal_malloc(stride*4 * chromaHeight);
    pCb = (Uint8*)osal_malloc(stride*4 * chromaHeight);
    pCr = (Uint8*)osal_malloc(stride*2 * chromaHeight);
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
    rowBufferCb = (Uint8*)osal_malloc(stride*4);
    rowBufferCr = (Uint8*)osal_malloc(stride*2);

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


    CalcYuvSize(fb->format, picWidth, fb->height, fb->cbcrInterleave, &frameSizeY, &frameSizeC, &frameSize, NULL, NULL, NULL);

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
    {
        Int32   temp_picWidth;
        Int32   chromaWidth;

        switch (fb->format) {
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
            temp_picWidth = VPU_ALIGN32(picWidth);
            chromaWidth = ((VPU_ALIGN16(temp_picWidth / 2*(1 << 1)) + 2) / 3 * 4);
            frameSizeY = (temp_picWidth + 2) / 3 * 4 * picHeight;
            frameSizeC = chromaWidth * picHeight / 2 * 2;
            frameSize = frameSizeY + frameSizeC;
            break;
        default:
            break;
        }       
    }
    if ((pYuv=(Uint8*)osal_malloc(frameSize)) == NULL) {
        VLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    VPU_DecGiveCommand(decHandle, GET_TILEDMAP_CONFIG, &mapCfg);
    if (fb->mapType == LINEAR_FRAME_MAP || fb->mapType == COMPRESSED_FRAME_MAP) {
        if (decHandle->codecMode == C7_VP9_DEC) {
            *retSize = StoreYuvImageBurstLinear(coreIdx, fb, mapCfg, pYuv, rcFrame, TRUE, TRUE); 
        }
        else {
            *retSize = StoreYuvImageBurstLinear(coreIdx, fb, mapCfg, pYuv, rcFrame, TRUE, FALSE);
        }
    }
    else {
        *retSize = StoreYuvImageBurstFormat(coreIdx, fb, mapCfg, pYuv, rcFrame, TRUE); 
    }

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

void PreparationWorkForDecTest(
    DecHandle handle
    )
{
    UNREFERENCED_PARAMETER(handle);
}

void PreparationWorkForEncTest(
    EncHandle   handle 
    )
{
    UNREFERENCED_PARAMETER(handle);
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

#ifdef SUPPORT_ENCODE_CUSTOM_HEADER
Uint32 activeParameterSEIEncode(sei_active_parameter_t *sap, hrd_t *hrd, Uint8 *pBuffer, Uint32 bufferSize)
{
    spp_enc_context spp;
    Uint32 code;
    const Uint32 layer_id = 0;
    const Uint32 temporal_id_plus_one = 1;
    const Uint32 payload_type = (Uint32)ACTIVE_PARAMETER_SETS;

    Uint32 put_bit_byte_size;
    Uint32 payload_bit_size;
    Uint32 payload_byte_size;
    int i;

    payload_bit_size = 0;
    payload_byte_size = 0;

    spp = spp_enc_init(pBuffer, bufferSize, 1);

    // put start code
    spp_enc_put_nal_byte(spp, 1, 4);

    // put nal header
    code = (SNT_PREFIX_SEI << 9) | (layer_id << 3) | temporal_id_plus_one;
    spp_enc_put_nal_byte(spp, code, 2);

    // put payload type
    spp_enc_put_nal_byte(spp, payload_type, 1);

    // put payload size
    payload_bit_size = 4/*active_video_parameter_set_id*/ + 1/*self_contained_cvs_flag*/ + 1/*no_parameter_set_update_flag*/;
    payload_bit_size += spp_enc_get_ue_bit_size(sap->num_sps_ids_minus1);
    for (i=0; i <= sap->num_sps_ids_minus1; i++)
        payload_bit_size+= spp_enc_get_ue_bit_size(sap->active_seq_parameter_set_id[i]);

    payload_byte_size = ((payload_bit_size+7)/8);
    spp_enc_put_nal_byte(spp, payload_byte_size, 1); 

    spp_enc_put_bits(spp, sap->active_video_parameter_set_id, 4); 
    spp_enc_put_bits(spp, sap->self_contained_cvs_flag, 1); 
    spp_enc_put_bits(spp, sap->no_parameter_set_update_flag, 1); 
    spp_enc_put_ue(spp, sap->num_sps_ids_minus1); 		
    for (i=0; i <= sap->num_sps_ids_minus1; i++)
    {
        spp_enc_put_ue(spp, sap->active_seq_parameter_set_id[i]); 		
    }

    if (payload_bit_size & 7)
    {
        spp_enc_put_bits(spp, 1/*payload_bit_equal_to_one*/, 1);
        spp_enc_put_bits(spp, 0/*payload_bit_equal_to_zero*/,  (8 - (payload_bit_size & 7)-1));
    }
    spp_enc_put_byte_align(spp, 1);
    spp_enc_flush(spp);

    put_bit_byte_size = spp_enc_get_nal_cnt(spp);

    spp_enc_deinit(spp);
    return put_bit_byte_size;
}
Uint32 picTimingSEIEncode(sei_pic_timing_t *spt, hrd_t *hrd, Uint8 *pBuffer, Uint32 bufferSize)
{
    spp_enc_context spp;
    Uint32 code;
    const Uint32 layer_id = 0;
    const Uint32 temporal_id_plus_one = 1;
    const Uint32 payload_type = (Uint32)PICTURE_TIMING;

    Uint32 put_bit_byte_size;
    Uint32 payload_bit_size;
    Uint32 payload_byte_size;
    int i;

    payload_bit_size = 0;
    payload_byte_size = 0;

    if (hrd->nal_hrd_parameters_present_flag || hrd->vcl_hrd_parameters_present_flag)
        spt->CpbDpbDelaysPresentFlag = 1;

    spp = spp_enc_init(pBuffer, bufferSize, 1);

    // put start code
    spp_enc_put_nal_byte(spp, 1, 4);

    // put nal header
    code = (SNT_PREFIX_SEI << 9) | (layer_id << 3) | temporal_id_plus_one;
    spp_enc_put_nal_byte(spp, code, 2);

    // put payload type
    spp_enc_put_nal_byte(spp, payload_type, 1);

    // put payload size
    payload_bit_size = 4/*pic_struct*/ + 2/*source_scan_type*/ + 1/*duplicate_flag*/;

    if (spt->CpbDpbDelaysPresentFlag)
    {
        payload_bit_size += (hrd->au_cpb_removal_delay_length_minus1 + 1); //spt->au_cpb_removal_delay_minus1
        payload_bit_size += (hrd->dpb_output_delay_length_minus1+ 1);		// spt->pic_dpb_output_delay
        if (hrd->sub_pic_hrd_params_present_flag)
            payload_bit_size += (hrd->dpb_output_delay_du_length_minus1 + 1); //spt->pic_dpb_output_du_delay
        if (hrd->sub_pic_hrd_params_present_flag &&
            hrd->sub_pic_cpb_params_in_pic_timing_sei_flag)
        {
            payload_bit_size += spp_enc_get_ue_bit_size(spt->num_decoding_units_minus1);	//spt->num_decoding_units_minus1
            payload_bit_size += 1;	// spt->du_common_cpb_removal_delay_flag
            if (spt->du_common_cpb_removal_delay_flag)
                payload_bit_size +=  (hrd->du_cpb_removal_delay_increment_length_minus1 + 1);	//spt->du_common_cpb_removal_delay_increment_minus1
            for (i=0; i <= spt->num_decoding_units_minus1; i++)
            {
                payload_bit_size += spp_enc_get_ue_bit_size(spt->num_nalus_in_du_minus1[i]);	//spt->num_nalus_in_du_minus1[i]
                if (!spt->du_common_cpb_removal_delay_flag && i < spt->num_decoding_units_minus1)
                {
                    payload_bit_size += (hrd->du_cpb_removal_delay_increment_length_minus1 + 1);	// spt->du_cpb_removal_delay_increment_minus1[i]
                }
            }
        }
    }

    payload_byte_size = ((payload_bit_size+7)/8);
    spp_enc_put_nal_byte(spp, payload_byte_size, 1); 

    spp_enc_put_bits(spp, spt->pic_struct, 4); 
    spp_enc_put_bits(spp, spt->source_scan_type, 2); 
    spp_enc_put_bits(spp, spt->duplicate_flag, 1); 		

    if (spt->CpbDpbDelaysPresentFlag)
    {
        spp_enc_put_bits(spp, spt->au_cpb_removal_delay_minus1, (hrd->au_cpb_removal_delay_length_minus1 + 1));
        spp_enc_put_bits(spp, spt->pic_dpb_output_delay, (hrd->dpb_output_delay_length_minus1+ 1));
        if (hrd->sub_pic_hrd_params_present_flag)
            spp_enc_put_bits(spp, spt->pic_dpb_output_du_delay, (hrd->dpb_output_delay_du_length_minus1 + 1));
        if (hrd->sub_pic_hrd_params_present_flag &&
            hrd->sub_pic_cpb_params_in_pic_timing_sei_flag)
        {
            spp_enc_put_ue(spp, spt->num_decoding_units_minus1);
            spp_enc_put_bits(spp, spt->du_common_cpb_removal_delay_flag, 1);
            if (spt->du_common_cpb_removal_delay_flag)
                spp_enc_put_bits(spp, spt->du_common_cpb_removal_delay_increment_minus1, (hrd->du_cpb_removal_delay_increment_length_minus1 + 1));
            for (i=0; i <= spt->num_decoding_units_minus1; i++)
            {
                spp_enc_put_ue(spp, spt->num_nalus_in_du_minus1[i]);
                if (!spt->du_common_cpb_removal_delay_flag && i < spt->num_decoding_units_minus1)
                {
                    spp_enc_put_bits(spp, spt->du_cpb_removal_delay_increment_minus1[i], (hrd->du_cpb_removal_delay_increment_length_minus1 + 1));
                }
            }

        }

    }
    if (payload_bit_size & 7)
    {
        spp_enc_put_bits(spp, 1/*payload_bit_equal_to_one*/, 1);
        spp_enc_put_bits(spp, 0/*payload_bit_equal_to_zero*/,  (8 - (payload_bit_size & 7)-1));
    }

    spp_enc_put_byte_align(spp, 1);
    spp_enc_flush(spp);

    put_bit_byte_size = spp_enc_get_nal_cnt(spp);

    spp_enc_deinit(spp);
    return put_bit_byte_size;
}

Uint32 bufferingPeriodSeiEncode(sei_buffering_period_t *sbp, hrd_t *hrd, Uint8 *pBuffer, Uint32 bufferSize)
{
    spp_enc_context spp;
    Uint32 code;
    const Uint32 layer_id = 0;
    const Uint32 temporal_id_plus_one = 1;
    const Uint32 payload_type = (Uint32)BUFFERING_PERIOD;
    int i;
    Uint32 put_bit_byte_size;
    Uint32 payload_bit_size;
    Uint32 payload_byte_size;

    payload_bit_size = 0;
    payload_byte_size = 0;

    sbp->sub_pic_hrd_params_present_flag = hrd->sub_pic_hrd_params_present_flag;
    sbp->au_cpb_removal_delay_length_minus1 =  hrd->au_cpb_removal_delay_length_minus1;
    sbp->initial_cpb_removal_delay_length_minus1 =  hrd->initial_cpb_removal_delay_length_minus1;
    sbp->CpbCnt =  hrd->cpb_cnt_minus1[0]+1;
    sbp->NalHrdBpPresentFlag =  hrd->nal_hrd_parameters_present_flag;
    sbp->VclHrdBpPresentFlag =  hrd->vcl_hrd_parameters_present_flag;

    payload_bit_size += spp_enc_get_ue_bit_size(sbp->bp_seq_parameter_set_id);
    if (!sbp->sub_pic_hrd_params_present_flag)
        payload_bit_size += 1;	//irap_cpb_params_present_flag
    if (sbp->irap_cpb_params_present_flag)
    {
        payload_bit_size += (sbp->au_cpb_removal_delay_length_minus1+1);	//cpb_delay_offset
        payload_bit_size += (sbp->au_cpb_removal_delay_length_minus1+1);	//dpb_delay_offset
    }	
    payload_bit_size += 1;	//concatenation_flag
    payload_bit_size += (sbp->au_cpb_removal_delay_length_minus1+1); //au_cpb_removal_delay_delta_minus1
    if (sbp->NalHrdBpPresentFlag)
    {
        for (i=0; i < (int)sbp->CpbCnt; i++)
        {
            payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	//nal_initial_cpb_removal_delay
            payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	//nal_initial_cpb_removal_offset

            if (sbp->sub_pic_hrd_params_present_flag || sbp->irap_cpb_params_present_flag)
            {
                payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	// nal_initial_alt_cpb_removal_delay
                payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	//nal_initial_alt_cpb_removal_offset
            }
        }
    }
    if (sbp->VclHrdBpPresentFlag )
    {
        for (i=0; i < (int)sbp->CpbCnt; i++)
        {
            payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	//vcl_initial_cpb_removal_delay
            payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	//vcl_initial_cpb_removal_offset

            if (sbp->sub_pic_hrd_params_present_flag || sbp->irap_cpb_params_present_flag)
            {
                payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	//vcl_initial_alt_cpb_removal_delay
                payload_bit_size += (sbp->initial_cpb_removal_delay_length_minus1+1);	//vcl_initial_alt_cpb_removal_offset
            }
        }
    }

    spp = spp_enc_init(pBuffer, bufferSize, 1);

    // put start code
    spp_enc_put_nal_byte(spp, 1, 4);

    // put nal header
    code = (SNT_PREFIX_SEI << 9) | (layer_id << 3) | temporal_id_plus_one;
    spp_enc_put_nal_byte(spp, code, 2);

    // put payload type
    spp_enc_put_nal_byte(spp, payload_type, 1);

    // put payload size
    payload_byte_size = ((payload_bit_size+7)/8);
    spp_enc_put_nal_byte(spp, payload_byte_size, 1); 

    // put buffering period
    spp_enc_put_ue(spp, sbp->bp_seq_parameter_set_id);
    if (!sbp->sub_pic_hrd_params_present_flag)
        spp_enc_put_bits(spp, sbp->irap_cpb_params_present_flag, 1);
    if (sbp->irap_cpb_params_present_flag)
    {
        spp_enc_put_bits(spp, sbp->dpb_delay_offset, (sbp->au_cpb_removal_delay_length_minus1+1));
        spp_enc_put_bits(spp, sbp->dpb_delay_offset, (sbp->au_cpb_removal_delay_length_minus1+1));
    }
    spp_enc_put_bits(spp, sbp->concatenation_flag, 1); 
    spp_enc_put_bits(spp, sbp->au_cpb_removal_delay_delta_minus1, (sbp->au_cpb_removal_delay_length_minus1+1)); 
    if (sbp->NalHrdBpPresentFlag)
    {
        for (i=0; i < (int)sbp->CpbCnt; i++)
        {
            spp_enc_put_bits(spp, sbp->nal_initial_cpb_removal_delay[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
            spp_enc_put_bits(spp, sbp->nal_initial_cpb_removal_offset[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
            if (sbp->sub_pic_hrd_params_present_flag || sbp->irap_cpb_params_present_flag)
            {
                spp_enc_put_bits(spp, sbp->nal_initial_alt_cpb_removal_delay[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
                spp_enc_put_bits(spp, sbp->nal_initial_alt_cpb_removal_offset[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
            }
        }
    }
    if (sbp->VclHrdBpPresentFlag)
    {
        for (i=0; i < (int)sbp->CpbCnt; i++)
        {
            spp_enc_put_bits(spp, sbp->vcl_initial_cpb_removal_delay[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
            spp_enc_put_bits(spp, sbp->vcl_initial_cpb_removal_offset[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
            if (sbp->sub_pic_hrd_params_present_flag || sbp->irap_cpb_params_present_flag)
            {
                spp_enc_put_bits(spp, sbp->vcl_initial_alt_cpb_removal_delay[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
                spp_enc_put_bits(spp, sbp->vcl_initial_alt_cpb_removal_offset[i], (sbp->initial_cpb_removal_delay_length_minus1+1));
            }
        }
    }		

    if (payload_bit_size & 7)
    {
        spp_enc_put_bits(spp, 1/*payload_bit_equal_to_one*/, 1);
        spp_enc_put_bits(spp, 0/*payload_bit_equal_to_zero*/,  (8 - (payload_bit_size & 7)-1));
    }

    spp_enc_put_byte_align(spp, 1);
    spp_enc_flush(spp);

    put_bit_byte_size = spp_enc_get_nal_cnt(spp);

    spp_enc_deinit(spp);

    return put_bit_byte_size;
}

Uint32 EncodePrefixSEI( sei_active_parameter_t *sap, sei_pic_timing_t *spt, sei_buffering_period_t *sbp, hrd_t *hrd, Uint8 *pBuffer, Uint32 bufferSize)
{
    Uint8 *ptrBuffer;
    Uint32 byteSize;

    byteSize = 0;

    ptrBuffer = (Uint8 *)(pBuffer + byteSize);
    byteSize += activeParameterSEIEncode(sap, hrd, ptrBuffer, bufferSize);

    ptrBuffer = (Uint8 *)(pBuffer + byteSize);
    byteSize += picTimingSEIEncode(spt, hrd, ptrBuffer, bufferSize);

    ptrBuffer = (Uint8 *)(pBuffer + byteSize);
    byteSize += bufferingPeriodSeiEncode(sbp, hrd, ptrBuffer, bufferSize);

    return byteSize;
}

// calculate scale value of bitrate and initial delay
int calcScale(int x)
{
    Uint32 iMask = 0xffffffff;
    int scaleValue = 32;

    if (x==0)
    {
        return 0;
    }

    while ((x&iMask) != 0)
    {
        scaleValue--;
        iMask = (iMask >> 1);
    }

    return scaleValue;
}

static void hrdEncode(spp_enc_context spp, hrd_t *hrd)
{
    Uint32 layer;
    Uint32 cpb;

    hrd->commonInfPresentFlag = 1;
    // the other parameters can be configured according to the target application.
    if (hrd->commonInfPresentFlag)
    {
        spp_enc_put_bits(spp, hrd->nal_hrd_parameters_present_flag, 1);
        spp_enc_put_bits(spp, hrd->vcl_hrd_parameters_present_flag, 1);

        if (hrd->nal_hrd_parameters_present_flag || hrd->vcl_hrd_parameters_present_flag)
        {
            spp_enc_put_bits(spp, hrd->sub_pic_hrd_params_present_flag, 1);
            if (hrd->sub_pic_hrd_params_present_flag)
            {
                spp_enc_put_bits(spp, hrd->tick_divisor_minus2, 8);
                spp_enc_put_bits(spp, hrd->du_cpb_removal_delay_increment_length_minus1, 5);
                spp_enc_put_bits(spp, hrd->sub_pic_cpb_params_in_pic_timing_sei_flag, 1);
                spp_enc_put_bits(spp, hrd->dpb_output_delay_du_length_minus1, 5);
            }

            spp_enc_put_bits(spp, hrd->bit_rate_scale, 4);

            spp_enc_put_bits(spp, hrd->cpb_size_scale, 4);

            if (hrd->sub_pic_hrd_params_present_flag)
            {
                spp_enc_put_bits(spp, hrd->cpb_size_du_scale, 4);
            }

            spp_enc_put_bits(spp, hrd->initial_cpb_removal_delay_length_minus1, 5);
            spp_enc_put_bits(spp, hrd->au_cpb_removal_delay_length_minus1, 5);
            spp_enc_put_bits(spp, hrd->dpb_output_delay_length_minus1, 5);
        }
    }

    for (layer=0; layer <= (Uint32)hrd->vps_max_sub_layers_minus1; layer++)
    {
        spp_enc_put_bits(spp, hrd->fixed_pic_rate_general_flag[layer], 1);

        if (!hrd->fixed_pic_rate_general_flag[layer])
            spp_enc_put_bits(spp, hrd->fixed_pic_rate_within_cvs_flag[layer], 1);

        if (hrd->fixed_pic_rate_within_cvs_flag[layer])
            spp_enc_put_ue(spp, hrd->elemental_duration_in_tc_minus1[layer]);
        else
            spp_enc_put_bits(spp, hrd->low_delay_hrd_flag[layer], 1);

        if (!hrd->low_delay_hrd_flag[layer])
            spp_enc_put_ue(spp, hrd->cpb_cnt_minus1[layer]);

        if (hrd->nal_hrd_parameters_present_flag)
        {
            for(cpb=0; cpb <= (Uint32)hrd->cpb_cnt_minus1[layer]; cpb++ )
            {
                spp_enc_put_ue(spp, hrd->bit_rate_value_minus1[cpb][layer]);

                spp_enc_put_ue(spp, hrd->cpb_size_value_minus1[cpb][layer]);

                if (hrd->sub_pic_hrd_params_present_flag)
                {
                    spp_enc_put_ue(spp, hrd->cpb_size_du_value_minus1[cpb][layer]);

                    spp_enc_put_ue(spp, hrd->bit_rate_du_value_minus1[cpb][layer]);
                }
                spp_enc_put_bits(spp, hrd->cbr_flag[cpb][layer], 1);					
            }
        }

        if (hrd->vcl_hrd_parameters_present_flag)
        {
            for(cpb=0; cpb <= (Uint32)hrd->cpb_cnt_minus1[layer]; cpb++ )
            {
                spp_enc_put_ue(spp, hrd->bit_rate_value_minus1[cpb][layer]);

                spp_enc_put_ue(spp, hrd->cpb_size_value_minus1[cpb][layer]);

                if (hrd->sub_pic_hrd_params_present_flag)
                {
                    spp_enc_put_ue(spp, hrd->cpb_size_du_value_minus1[cpb][layer]);

                    spp_enc_put_ue(spp, hrd->bit_rate_du_value_minus1[cpb][layer]);
                }
                spp_enc_put_bits(spp, hrd->cbr_flag[cpb][layer], 1);					
            }
        }
    }
}

BOOL EncodeVUI(hrd_t *hrd, vui_t *vui, Uint8 *pBuffer, Uint32 bufferSize, Uint32 *pByteSize, Uint32 *pBitSize, double dframeRate)
{
    spp_enc_context spp;
    Uint32 put_bit_byte_size;
    Uint32 put_bit_bit_size;

    // the other parameters can be configured according to the target application.

    spp = spp_enc_init(pBuffer, bufferSize, 0);

    spp_enc_put_bits(spp, vui->aspect_ratio_info_present_flag, 1); 
    if (vui->aspect_ratio_info_present_flag)
    {
        spp_enc_put_bits(spp, vui->aspect_ratio_idc, 8); 

        if (vui->aspect_ratio_idc == EXTENDED_SAR)
        {
            spp_enc_put_bits(spp, vui->sar_width, 16); 
            spp_enc_put_bits(spp, vui->sar_height, 16); 
        }
    }

    spp_enc_put_bits(spp, vui->overscan_info_present_flag, 1); 
    if (vui->overscan_info_present_flag)
    {
        spp_enc_put_bits(spp, vui->overscan_appropriate_flag, 1); 
    }

    spp_enc_put_bits(spp, vui->video_signal_type_present_flag, 1); 
    if (vui->video_signal_type_present_flag)
    {
        spp_enc_put_bits(spp, vui->video_format, 3); 
        spp_enc_put_bits(spp, vui->video_full_range_flag, 1); 
        spp_enc_put_bits(spp, vui->colour_description_present_flag, 1); 

        if (vui->colour_description_present_flag)
        {
            spp_enc_put_bits(spp, vui->colour_primaries, 8); 
            spp_enc_put_bits(spp, vui->transfer_characteristics, 1); 
            spp_enc_put_bits(spp, vui->matrix_coeffs, 1); 
        }
    }

    spp_enc_put_bits(spp, vui->chroma_loc_info_present_flag, 1); 
    if (vui->chroma_loc_info_present_flag)
    {
        spp_enc_put_ue(spp, vui->chroma_sample_loc_type_top_field);
        spp_enc_put_ue(spp, vui->chroma_sample_loc_type_bottom_field);
    }

    spp_enc_put_bits(spp, vui->neutral_chroma_indication_flag, 1); 
    spp_enc_put_bits(spp, vui->field_seq_flag, 1); 
    spp_enc_put_bits(spp, vui->frame_field_info_present_flag, 1); 
    spp_enc_put_bits(spp, vui->default_display_window_flag, 1); 
    if (vui->default_display_window_flag)
    {
        spp_enc_put_ue(spp, vui->def_disp_win_left_offset);
        spp_enc_put_ue(spp, vui->def_disp_win_right_offset);
        spp_enc_put_ue(spp, vui->def_disp_win_top_offset);
        spp_enc_put_ue(spp, vui->def_disp_win_bottom_offset);
    }
    spp_enc_put_bits(spp, vui->vui_timing_info_present_flag, 1); 
    if (vui->vui_timing_info_present_flag)
    {
        spp_enc_put_bits(spp, vui->vui_num_units_in_tick, 32); 
        spp_enc_put_bits(spp, vui->vui_time_scale, 32); 
        spp_enc_put_bits(spp, vui->vui_poc_proportional_to_timing_flag, 1); 
        if (vui->vui_poc_proportional_to_timing_flag)
        {
            spp_enc_put_ue(spp, vui->vui_num_ticks_poc_diff_one_minus1); 
        }

        spp_enc_put_bits(spp, vui->vui_hrd_parameters_present_flag, 1); 
        if (vui->vui_hrd_parameters_present_flag)
            hrdEncode(spp, hrd);
    }

    spp_enc_put_bits(spp, vui->bitstream_restriction_flag, 1); 
    if (vui->bitstream_restriction_flag)
    {
        spp_enc_put_bits(spp, vui->tiles_fixed_structure_flag, 1);
        spp_enc_put_bits(spp, vui->motion_vectors_over_pic_boundaries_flag, 1);
        spp_enc_put_bits(spp, vui->restricted_ref_pic_lists_flag, 1);
        spp_enc_put_ue(spp, vui->min_spatial_segmentation_idc);
        spp_enc_put_ue(spp, vui->max_bytes_per_pic_denom);
        spp_enc_put_ue(spp, vui->max_bits_per_min_cu_denom);
        spp_enc_put_ue(spp, vui->log2_max_mv_length_horizontal);
        spp_enc_put_ue(spp, vui->log2_max_mv_length_vertical);
    }

    spp_enc_flush(spp);		

    put_bit_byte_size = spp_enc_get_nal_cnt(spp);
    put_bit_bit_size = spp_enc_get_rbsp_bit(spp);

    if (pByteSize)
        *pByteSize = put_bit_byte_size;

    if (pBitSize)
        *pBitSize = put_bit_bit_size;

    spp_enc_deinit(spp);
    return 1;
}

#endif 


