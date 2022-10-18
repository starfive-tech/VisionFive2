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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../jpuapi/regdefine.h"
#include "../jpuapi/jpuapi.h"
#include "../jpuapi/jpuapifunc.h"
#include "../jpuapi/jputable.h"
#include "jpulog.h"
#include "main_helper.h"

fb_context s_fb[MAX_NUM_INSTANCE];

static int StoreYuvImageBurstFormat_V20(int chromaStride, Uint8 * dst, int picWidth, int picHeight, Uint32 bitDepth,Uint64 addrY, Uint64 addrCb, Uint64 addrCr, Uint32 stride, FrameFormat format, int endian, CbCrInterLeave interLeave, PackedFormat packed);
// Figure A.6 - Zig-zag sequence of quantized DCT coefficients
const int InvScanTable[64] = {
    0,  1,  5,  6, 14, 15, 27, 28,
    2,  4,  7, 13, 16, 26, 29, 42,
    3,  8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

const int ScanTable[64] = {
    0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

/******************************************************************************
DPB Image Data Control
******************************************************************************/
#define MAX_ROW_IN_MCU  8
static void TiledToLinear(Uint8* pSrc, Uint8* pDst, Uint32 width, Uint32 height, Uint32 cWidth, Uint32 cHeight, Uint32 lineWidth)
{
    Uint32 picX, picY;
    Uint32 yuvAddr     = 0;
    Uint32 convAddr    = 0;
    Uint32 chromaBase  = 0;
    Uint32 row    = 0;
    Uint32 col    = 0;
    Uint32 ncols  = width / lineWidth;

    // Luma data.
    for (picY = 0; picY < height; picY++) {
        for (picX = 0; picX < width; picX += lineWidth) {
            convAddr = ((picY/MAX_ROW_IN_MCU)*MAX_ROW_IN_MCU*width) + (row*width) + col*lineWidth;
            memcpy(pDst + convAddr, pSrc + yuvAddr, lineWidth);
            yuvAddr += lineWidth;
            row++;
            row %= 8;
            if (row == 0) col++;
            col %= ncols;
        }
    }
    chromaBase = convAddr + lineWidth; // add last line

    // Chroma data.
    // lineWidth*2. only support Cb Cr interleaved format.
    lineWidth *= 2;
    col = row = 0;
    convAddr = 0;
    ncols = cWidth / lineWidth;
    for (picY = 0; picY < cHeight; picY++) {
        for (picX = 0; picX < cWidth; picX += lineWidth) {
            convAddr = ((picY/MAX_ROW_IN_MCU)*MAX_ROW_IN_MCU*cWidth) + (row*cWidth) + col*lineWidth;
            memcpy(pDst + chromaBase + convAddr, pSrc + yuvAddr, lineWidth);
            yuvAddr += lineWidth;
            row++;
            row %= 8;
            if (row == 0) col++;
            col %= ncols;
        }
    }
}

int SaveYuvImageHelper(
    Uint8*          pYuv,
    FrameBuffer*    fb,
    CbCrInterLeave  interLeave,
    PackedFormat    packed,
    Uint32          picWidth,
    Uint32          picHeight,
    Uint32          bitDepth
    )
{
    int frameSize;

    if (pYuv == NULL) {
        JLOG(ERR, "%s:%d pYuv is NULL\n", __FUNCTION__, __LINE__);
        return 0;
    }

    frameSize = StoreYuvImageBurstFormat_V20(fb->strideC, pYuv, picWidth, picHeight, bitDepth,
        fb->bufY,
        fb->bufCb,
        fb->bufCr,
        fb->stride,
        fb->format,
        fb->endian,
        interLeave,
        packed);

    return frameSize;
}

int StoreYuvImageBurstFormatWhenNotAligned(int chromaStride, Uint8 * dst, int picWidth, int picHeight, Uint32 bitDepth,
    Uint64 addrY, Uint64 addrCb, Uint64 addrCr, Uint32 stride, FrameFormat format, int endian, CbCrInterLeave interLeave, PackedFormat packed)
{
    int size;
    int y, nY = 0, nCb, nCr;
    Uint64 addr;
    int lumaSize, chromaSize = 0, chromaWidth = 0, chromaHeight =0;
    Uint8 * puc;
    int chromaStride_i = 0;
    Uint32  bytesPerPixel = (bitDepth + 7) / 8;

    chromaStride_i = chromaStride;

    switch (format)
    {
    case FORMAT_420:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize  = (picWidth/2) * (picHeight/2);
        chromaWidth = picWidth/2;
        chromaHeight = nY;
        break;
    case FORMAT_440:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth) * (picHeight / 2);
        chromaWidth = picWidth;
        chromaHeight = nY;
        break;
    case FORMAT_422:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = (picWidth/2) * picHeight ;
        chromaWidth = (picWidth/2);
        chromaHeight = nY*2;
        break;
    case FORMAT_444:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = picWidth * picHeight;
        chromaWidth = picWidth;
        chromaHeight = nY*2;
        break;
    case FORMAT_400:
        nY = picHeight;
        nCb = nCr = 0;
        chromaSize = 0;
        chromaWidth = 0;
        chromaHeight = 0;
        break;
    default:
        return 0;
    }

    puc = dst;
    addr = addrY;

    if (packed)
    {
        if (packed == PACKED_FORMAT_444)
            picWidth *= 3;
        else
            picWidth *= 2;

        chromaSize = 0;
    }

    lumaSize = picWidth * nY;

    size = lumaSize + chromaSize*2;

    lumaSize    *= bytesPerPixel;
    chromaSize  *= bytesPerPixel;
    size        *= bytesPerPixel;
    picWidth    *= bytesPerPixel;
    chromaWidth *= bytesPerPixel;

    if (interLeave){
        chromaSize = chromaSize *2;
        chromaWidth = chromaWidth *2;
        chromaStride_i = chromaStride_i;
    }

    if ((picWidth != stride) || (chromaWidth != chromaStride_i))
    {
        for (y = 0; y < nY; ++y) {
            JpuReadMem(addr + stride * y, (Uint8 *)(puc + y * picWidth), picWidth,  endian);
        }

        if(packed)
            return size;

        if (interLeave)
        {

            puc = dst + lumaSize;
            addr = addrCb;
            for (y = 0; y < (chromaHeight/2); ++y) {
                JpuReadMem(addr + (chromaStride_i)*y, (Uint8 *)(puc + y*(chromaWidth)), (chromaWidth), endian);
            }
        }
        else
        {
            puc = dst + lumaSize;
            addr = addrCb;
            for (y = 0; y < nCb; ++y) {
                JpuReadMem(addr + chromaStride_i * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth,  endian);
            }

            puc = dst + lumaSize + chromaSize;
            addr = addrCr;
            for (y = 0; y < nCr; ++y) {
                JpuReadMem(addr + chromaStride_i * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth,  endian);
            }

        }

    }
    else if (!packed && interLeave && lumaSize != (addrCb - addrY))
    {
        JpuReadMem(addr, (Uint8 *)( puc ), lumaSize, endian);

        puc = dst + lumaSize;
        addr = addrCb;
        JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian);
    }
    else if (!packed && !interLeave && (lumaSize != (addrCb - addrY) || chromaSize != addrCr - addrCb))
    {
        JpuReadMem(addr, (Uint8 *)( puc ), lumaSize, endian);

        puc = dst + lumaSize;
        addr = addrCb;
        JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian);

        puc = dst + lumaSize + chromaSize;
        addr = addrCr;
        JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian);
    }
    return size;
}//lint !e429

int SaveYuvImageHelperDma(
    Uint8*          pYuv,
    FrameBuffer*    fb,
    CbCrInterLeave  interLeave,
    PackedFormat    packed,
    Uint32          picWidth,
    Uint32          picHeight,
    Uint32          bitDepth
    )
{
    int frameSize;

    if (pYuv == NULL) {
        JLOG(ERR, "%s:%d pYuv is NULL\n", __FUNCTION__, __LINE__);
        return 0;
    }

    frameSize = StoreYuvImageBurstFormatWhenNotAligned(fb->strideC, pYuv, picWidth, picHeight, bitDepth,
        fb->bufY,
        fb->bufCb,
        fb->bufCr,
        fb->stride,
        fb->format,
        fb->endian,
        interLeave,
        packed);

    return frameSize;
}

int SaveYuvImageHelperFormat_V20(
    FILE*           yuvFp,
    Uint8*          pYuv,
    FrameBuffer*    fb,
    CbCrInterLeave  interLeave,
    PackedFormat    packed,
    Uint32          picWidth,
    Uint32          picHeight,
    Uint32          bitDepth,
    BOOL            tiled2Linear
    )
{
    int frameSize;

    if (pYuv == NULL) {
        JLOG(ERR, "%s:%d pYuv is NULL\n", __FUNCTION__, __LINE__);
        return 0;
    }

    frameSize = StoreYuvImageBurstFormat_V20(fb->strideC, pYuv, picWidth, picHeight, bitDepth,
        fb->bufY,
        fb->bufCb,
        fb->bufCr,
        fb->stride,
        fb->format,
        fb->endian,
        interLeave,
        packed);

    if (tiled2Linear == TRUE) {
        Uint32 Bpp       = JPU_CEIL(8, bitDepth) / 8;
        Uint32 width     = JPU_CEIL(picWidth,8) * Bpp;
        Uint32 height    = JPU_CEIL(picHeight,8);
        Uint32 tiledFrameSize;
        Uint8* pConv     = NULL;
        Uint8* base      = NULL;
        Uint8* target    = NULL;
        Uint32 cWidth    = (fb->format == FORMAT_420 || fb->format == FORMAT_422) ? width / 2  : width;
        Uint32 cHeight   = (fb->format == FORMAT_420 || fb->format == FORMAT_440) ? height / 2 : height;
        Uint32 lineWidth = 8 * Bpp;
        Uint32 y, linearChromaWidth, linearChromaHeight;

        switch (fb->format) {
        case FORMAT_400: linearChromaWidth = 0,          linearChromaHeight = 0,           tiledFrameSize = width * height; break;
        case FORMAT_420: linearChromaWidth = picWidth,   linearChromaHeight = picHeight/2, tiledFrameSize = width * height * 3 /2; break;
        case FORMAT_422: linearChromaWidth = picWidth,   linearChromaHeight = picHeight,   tiledFrameSize = width * height * 2; break;
        case FORMAT_440: linearChromaWidth = picWidth*2, linearChromaHeight = picHeight/2, tiledFrameSize = width * height * 2; break;
        case FORMAT_444: linearChromaWidth = picWidth*2, linearChromaHeight = picHeight,   tiledFrameSize = width * height * 3; break;
        default:
            JLOG(ERR, "%d:%d Unknown format:(%d)\n", __FUNCTION__, __LINE__, fb->format);
            return 0;
        }
        if ((pConv=(Uint8*)malloc(tiledFrameSize)) == NULL) {
            JLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
            return 0;
        }

        if (fb->format == FORMAT_400) {
            cWidth = cHeight = 0;
        }

        cWidth *= 2; // Must be semi-planar.

        TiledToLinear(pYuv, pConv, width, height, cWidth, cHeight, lineWidth);

        base   = pConv;
        target = pYuv;
        // Copy Luma
        for (y=0; y<picHeight; y++) {
            memcpy((void*)target, (void*)(base + y*width), picWidth);
            target += fb->stride;
        }
        base = pConv + (width*height);
        // Copy Chroma
        for (y=0; y<linearChromaHeight; y++) {
            memcpy((void*)target, (void*)(base+ y*cWidth), linearChromaWidth);
            target += fb->strideC;
        }
        free((void*)pConv);
    }

    if (yuvFp)
    {
        if( !fwrite(pYuv, sizeof(Uint8), frameSize , yuvFp) )
        {
            JLOG(ERR, "Frame Data fwrite failed file handle is 0x%x \n", yuvFp );
            return 0;
        }
        fflush(yuvFp);
    }
    return 1;
}



/******************************************************************************
JPEG specific Helper
******************************************************************************/

static int getTblElement(FILE *fp, char *str)
{
    while (1) {
        if (fgets(str, 256, fp) == NULL)
            return 0;
        if ((str[0] != ';') && (str[0] != '/'))
            break;
    }
    return 1;
}

static int parseHuffmanTable(FILE * fp, EncMjpgParam *param, int prec)
{
    char sLine[258] = {0,};
    unsigned int h[8] = {0,};
    BYTE *huffBit;
    BYTE *huffVal;
    int i, j;

    int max_ac = 162;
    if (prec)
        max_ac = 256;

    huffBit = param->huffBits[DC_TABLE_INDEX0];
    huffVal = param->huffVal[DC_TABLE_INDEX0];

    for (i=0; i<2; i++) // Luma DC BitLength
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);

        for (j=0; j<8; j++)
            *huffBit++ = (BYTE)h[j];
    }

    for (i=0; i<(16/8); i++) // Luma DC HuffValue
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);

        for (j=0; j<8; j++)
            *huffVal++ = (BYTE)h[j];
    }

    huffBit = param->huffBits[AC_TABLE_INDEX0];
    huffVal = param->huffVal[AC_TABLE_INDEX0];

    for (i=0; i<(16/8); i++) // Luma AC BitLength
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++)
            *huffBit++ = (BYTE)h[j];
    }
    for (i=0; i<(max_ac/8); i++) // Luma AC HuffValue
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++)
            *huffVal++ = (BYTE)h[j];
    }

    if (!prec){
        if (getTblElement(fp, sLine) == 0)
            return 0;
        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<2; j++)
            *huffVal++ = (BYTE)h[j];
    }

    huffBit = param->huffBits[DC_TABLE_INDEX1];
    huffVal = param->huffVal[DC_TABLE_INDEX1];

    for (i=0; i<(16/8); i++) // Chroma DC BitLength
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++)
            *huffBit++ = (BYTE)h[j];
    }

    for (i=0; i<(16/8); i++) // Chroma DC HuffValue
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++)
            *huffVal++ = (BYTE)h[j];
    }

    huffBit = param->huffBits[AC_TABLE_INDEX1];
    huffVal = param->huffVal[AC_TABLE_INDEX1];

    for (i=0; i<(16/8); i++) // Chroma AC BitLength
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++)
            *huffBit++ = (BYTE)h[j];
    }

    for (i=0; i<(max_ac/8); i++) // Luma AC HuffValue
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++)
            *huffVal++ = (BYTE)h[j];
    }

    if (!prec){
        if (getTblElement(fp, sLine) == 0)
            return 0;
        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<2; j++)
            *huffVal++ = (BYTE)h[j];
    }

    return 1;
}

static int parseQMatrix(FILE * fp, EncMjpgParam *param)
{
    char sLine[256] = {0,};
    int i,j;
    unsigned int h[8] = {0,};
    short temp_dc[64], temp_ac[64];
    short *qTable_dc, *qTable_ac, *qTable;

    qTable = temp_dc;
    for (i=0; i < (64/8); i++)
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++){
            if (param->lumaQ12bit == TRUE)
                *qTable++ = h[j] & 0xffff;
            else
                *qTable++ = h[j] & 0x00ff;
        }
    }

    qTable = temp_ac;
    for (i=0; i < (64/8); i++)
    {
        if (getTblElement(fp, sLine) == 0)
            return 0;

        memset(h, 0x00, 8*sizeof(unsigned int));
        sscanf(sLine, "%x %x %x %x %x %x %x %x", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7]);
        for (j=0; j<8; j++){
            if (param->chromaQ12bit == TRUE)
                *qTable++ = h[j] & 0xffff;
            else
                *qTable++ = h[j] & 0x00ff;
        }
    }

    qTable_dc = param->qMatTab[DC_TABLE_INDEX0];
    qTable_ac = param->qMatTab[AC_TABLE_INDEX0];

    for (i=0; i < 64; i++)
    {
        qTable_dc[InvScanTable[i]] = temp_dc[i];
        qTable_ac[InvScanTable[i]] = temp_ac[i];
    }

    memcpy(param->qMatTab[DC_TABLE_INDEX1], param->qMatTab[DC_TABLE_INDEX0], 64*sizeof(short));
    memcpy(param->qMatTab[AC_TABLE_INDEX1], param->qMatTab[AC_TABLE_INDEX0], 64*sizeof(short));
    return 1;
}


int jpgGetHuffTable(char *huffFileName, EncMjpgParam *param, int prec)
{
    char huffFilePath[256];
    FILE *huffFp = NULL;
    if (huffFileName[0] != 0) {
        strcpy(huffFilePath, huffFileName);
        huffFp = fopen(huffFilePath, "rt");
        if (huffFp == NULL) {
            JLOG( ERR, "Can't open Huffman Table file %s \n", huffFilePath);
            return 0;
        }
        parseHuffmanTable(huffFp, param, prec);
        fclose(huffFp);
    }
    else {
        if (prec){
            memcpy(param->huffBits[DC_TABLE_INDEX0], lumaDcBits_ES, 16);	// Luma DC BitLength
            memcpy(param->huffVal[DC_TABLE_INDEX0], lumaDcValue_ES, 16);	// Luma DC HuffValue
            memcpy(param->huffBits[AC_TABLE_INDEX0], lumaAcBits_ES, 16);	// Luma DC BitLength
            memcpy(param->huffVal[AC_TABLE_INDEX0], lumaAcValue_ES, 256);	// Luma DC HuffValue
            memcpy(param->huffBits[DC_TABLE_INDEX1], chromaDcBits_ES, 16);	// Chroma DC BitLength
            memcpy(param->huffVal[DC_TABLE_INDEX1], chromaDcValue_ES, 16);	// Chroma DC HuffValue
            memcpy(param->huffBits[AC_TABLE_INDEX1], chromaAcBits_ES, 16);	// Chroma AC BitLength
            memcpy(param->huffVal[AC_TABLE_INDEX1], chromaAcValue_ES, 256); // Chorma AC HuffValue
        } else {
            memcpy(param->huffBits[DC_TABLE_INDEX0], lumaDcBits, 16);	// Luma DC BitLength
            memcpy(param->huffVal[DC_TABLE_INDEX0], lumaDcValue, 16);	// Luma DC HuffValue
            memcpy(param->huffBits[AC_TABLE_INDEX0], lumaAcBits, 16);	// Luma DC BitLength
            memcpy(param->huffVal[AC_TABLE_INDEX0], lumaAcValue, 162);	// Luma DC HuffValue
            memcpy(param->huffBits[DC_TABLE_INDEX1], chromaDcBits, 16);	// Chroma DC BitLength
            memcpy(param->huffVal[DC_TABLE_INDEX1], chromaDcValue, 16);	// Chroma DC HuffValue
            memcpy(param->huffBits[AC_TABLE_INDEX1], chromaAcBits, 16);	// Chroma AC BitLength
            memcpy(param->huffVal[AC_TABLE_INDEX1], chromaAcValue, 162); // Chorma AC HuffValue
        }
    }

    return 1;
}



int jpgGetQMatrix(char *qMatFileName, EncMjpgParam *param)
{
    char qMatFilePath[256];
    FILE *qMatFp = NULL;
    int i;

    if ( qMatFileName[0] != 0 ) {
        strcpy(qMatFilePath, qMatFileName);
        qMatFp = fopen(qMatFilePath, "rt" );
        if ( !qMatFp ) {
            JLOG(ERR, "Can't open Q Matrix file %s \n", qMatFilePath);
            return 0;
        }
        parseQMatrix(qMatFp, param);
        fclose(qMatFp);
    }
    else {
        // Rearrange and insert pre-defined Q-matrix to deticated variable.
        for (i=0; i<64; i++){
            param->qMatTab[DC_TABLE_INDEX0][i] = lumaQ2[i] & 0x00ff;
            param->qMatTab[DC_TABLE_INDEX1][i] = lumaQ2[i] & 0x00ff;
            param->qMatTab[AC_TABLE_INDEX0][i] = chromaBQ2[i] & 0x00ff;
            param->qMatTab[AC_TABLE_INDEX1][i] = chromaBQ2[i] & 0x00ff;
        }
    }
    if (param->extendedSequence == TRUE) {
        /* CODAJ12 Constraint: The value of DC quantization should be greater or equal than 8. */
        /* FIXME: It might be wrong naming of DC_TABLE_INDEX and AC_TABLE_INDEX */
        if (param->qMatTab[DC_TABLE_INDEX0][0] < 8) {
            param->qMatTab[DC_TABLE_INDEX0][0] = 8;
        }
        if (param->qMatTab[AC_TABLE_INDEX0][0] < 8) {
            param->qMatTab[AC_TABLE_INDEX0][0] = 8;
        }
        if (param->qMatTab[DC_TABLE_INDEX1][0] < 8) {
            param->qMatTab[DC_TABLE_INDEX1][0] = 8;
        }
        if (param->qMatTab[AC_TABLE_INDEX1][0] < 8) {
            param->qMatTab[AC_TABLE_INDEX1][0] = 8;
        }
    }

    return 1;
}


/******************************************************************************
EncOpenParam Initialization
******************************************************************************/


int getJpgEncOpenParamDefault(JpgEncOpenParam *pEncOP, EncConfigParam *pEncConfig)
{
    int ret;
    EncMjpgParam mjpgParam;

    memset(&mjpgParam, 0x00, sizeof(EncMjpgParam));

    pEncOP->jpg12bit         = pEncConfig->extendedSequential;
    pEncOP->q_prec0          = FALSE;
    pEncOP->q_prec1          = FALSE;
    pEncOP->picWidth         = pEncConfig->picWidth;
    pEncOP->picHeight        = pEncConfig->picHeight;
    pEncOP->tiledModeEnable  = pEncConfig->tiledModeEnable;
    pEncOP->sourceFormat     = pEncConfig->sourceSubsample;
    pEncOP->restartInterval  = 0;
    pEncOP->chromaInterleave = pEncConfig->chromaInterleave;
    pEncOP->packedFormat     = pEncConfig->packedFormat;
    mjpgParam.sourceFormat   = pEncConfig->sourceSubsample;

    ret = jpgGetHuffTable(pEncConfig->huffFileName, &mjpgParam, pEncOP->jpg12bit);
    if (ret == 0) return ret;
    ret = jpgGetQMatrix(pEncConfig->qMatFileName, &mjpgParam);
    if (ret == 0) return ret;

    memcpy(pEncOP->huffVal, mjpgParam.huffVal, 4*256);
    memcpy(pEncOP->huffBits, mjpgParam.huffBits, 4*256);
    memcpy(pEncOP->qMatTab, mjpgParam.qMatTab, 4*64*sizeof(short));

    //Currently only 2DC,2AC huffman table for 12-bit case
    //So, copy them to EX1 to EX2 to modeling rest 2 tables
    if (pEncOP->jpg12bit == TRUE){
        memcpy(&pEncOP->huffVal[4][0], &pEncOP->huffVal[0][0], 4*256);
        memcpy(&pEncOP->huffBits[4][0], &pEncOP->huffBits[0][0], 4*256);
    }

    return 1;
}

/**
* To init EncOpenParam by CFG file
* IN
*   EncConfigParam *pEncConfig
* OUT
*   EncOpenParam *pEncOP
*   char *srcYuvFileName
* @return   TRUE on success, FALSE on failure.
*/

BOOL GetJpgEncOpenParam(JpgEncOpenParam *pEncOP, EncConfigParam *pEncConfig)
{
    ENC_CFG         encCfg;
    Int32           ret;
    EncMjpgParam    mjpgParam;
    char            huffmanTablePath[MAX_FILE_PATH];
    char            QMatTabPath[MAX_FILE_PATH];
    char            cfgPath[MAX_FILE_PATH];
    char*           srcYuvFileName = pEncConfig->yuvFileName;

    memset(&encCfg, 0x00, sizeof(ENC_CFG));
    memset(&mjpgParam, 0x00, sizeof(EncMjpgParam));

    encCfg.prec = pEncConfig->extendedSequential;
    snprintf(cfgPath, sizeof(cfgPath), "%s/%s", pEncConfig->strCfgDir, pEncConfig->cfgFileName);

    if (parseJpgCfgFile(&encCfg, cfgPath) == 0) {
        return FALSE;
    }
    if (strlen(srcYuvFileName) == 0) {
        strcpy(srcYuvFileName, encCfg.SrcFileName);
    }

    if (encCfg.FrmFormat == 0) {
        pEncConfig->chromaInterleave= CBCR_SEPARATED;
    }
    else if (encCfg.FrmFormat == 1) {
        pEncConfig->chromaInterleave = CBCR_INTERLEAVE;
    }
    else if (encCfg.FrmFormat == 2) {
        pEncConfig->chromaInterleave = CRCB_INTERLEAVE;
    }
    else {
        pEncConfig->chromaInterleave = CBCR_SEPARATED;
    }
    pEncConfig->mjpgFramerate = encCfg.FrameRate;

    /* Source format */
    pEncConfig->packedFormat = (PackedFormat)(encCfg.FrmFormat - 2);
    if (pEncConfig->packedFormat >= PACKED_FORMAT_MAX)
        pEncConfig->packedFormat = PACKED_FORMAT_NONE;

    pEncConfig->outNum          = encCfg.NumFrame;

    if (pEncConfig->picWidth == 0)  pEncConfig->picWidth  = encCfg.PicX;
    if (pEncConfig->picHeight == 0) pEncConfig->picHeight = encCfg.PicY;
    if (pEncOP) {
        pEncOP->q_prec0          = encCfg.QMatPrec0;
        pEncOP->q_prec1          = encCfg.QMatPrec1;
        pEncOP->picWidth         = encCfg.PicX;
        pEncOP->picHeight        = encCfg.PicY;
        pEncOP->chromaInterleave = pEncConfig->chromaInterleave;
        pEncOP->packedFormat     = pEncConfig->packedFormat;
        pEncOP->restartInterval  = encCfg.RstIntval;
        pEncOP->tiledModeEnable  = pEncConfig->tiledModeEnable;
        pEncOP->sourceFormat     = encCfg.SrcFormat;

        mjpgParam.sourceFormat = encCfg.SrcFormat;
        pEncOP->jpg12bit = pEncConfig->extendedSequential;
        if (pEncOP->jpg12bit) {
            mjpgParam.lumaQ12bit   = (BOOL)encCfg.QMatPrec0;
            mjpgParam.chromaQ12bit = (BOOL)encCfg.QMatPrec1;
            mjpgParam.extendedSequence = TRUE;
        }
        snprintf(huffmanTablePath, sizeof(huffmanTablePath), "%s/%s", pEncConfig->strCfgDir, encCfg.HuffTabName);
        ret = jpgGetHuffTable(huffmanTablePath, &mjpgParam, pEncOP->jpg12bit);
        if (ret == 0) {
            return ret;
        }

        snprintf(QMatTabPath, sizeof(QMatTabPath), "%s/%s", pEncConfig->strCfgDir, encCfg.QMatTabName);
        if ((ret=jpgGetQMatrix(QMatTabPath, &mjpgParam)) == FALSE) {
            return ret;
        }

        memcpy(pEncOP->huffVal,  mjpgParam.huffVal,  4*256);
        memcpy(pEncOP->huffBits, mjpgParam.huffBits, 4*256);
        memcpy(pEncOP->qMatTab,  mjpgParam.qMatTab,  4*64*sizeof(short));

        //Currently only 2DC,2AC huffman table for 12-bit case
        //So, copy them to EX1 to EX2 to modeling rest 2 tables
        if (pEncOP->jpg12bit){
            memcpy(&pEncOP->huffVal[4][0], &pEncOP->huffVal[0][0], 4*256);
            memcpy(&pEncOP->huffBits[4][0], &pEncOP->huffBits[0][0], 4*256);
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
// ENCODE PARAMETER PARSE FUNCSIONS
//------------------------------------------------------------------------------
// Parameter parsing helper
static int GetValue(FILE *fp, char *para, char *value)
{
    char lineStr[256];
    char paraStr[256];
    fseek(fp, 0, SEEK_SET);

    while (1) {
        if (fgets(lineStr, 256, fp) == NULL)
            return 0;
        sscanf(lineStr, "%s %s", paraStr, value);
        if (paraStr[0] != ';') {
            if (strcmp(para, paraStr) == 0)
                return 1;
        }
    }
}

int parseJpgCfgFile(ENC_CFG *pEncCfg, char *FileName)
{
    FILE *Fp;
    char sLine[256] = {0,};
    int res = 0;

    Fp = fopen(FileName, "rt");
    if (Fp == NULL) {
        fprintf(stderr, "   > ERROR: File not exist <%s>, %d\n", FileName, errno);
        goto end;
    }

    // source file name
    if (GetValue(Fp, "YUV_SRC_IMG", sLine) == 0)
        goto end;
    sscanf(sLine, "%s", (char *)pEncCfg->SrcFileName);

    // frame format
    // 	; 0-planar, 1-NV12,NV16(CbCr interleave) 2-NV21,NV61(CbCr alternative)
    // 		; 3-YUYV, 4-UYVY, 5-YVYU, 6-VYUY, 7-YUV packed (444 only)
    if (GetValue(Fp, "FRAME_FORMAT", sLine) == 0)
        goto end;
    sscanf(sLine, "%d", &pEncCfg->FrmFormat);

    // width
    if (GetValue(Fp, "PICTURE_WIDTH", sLine) == 0)
        goto end;
    sscanf(sLine, "%d", &pEncCfg->PicX);

    // height
    if (GetValue(Fp, "PICTURE_HEIGHT", sLine) == 0)
        goto end;
    sscanf(sLine, "%d", &pEncCfg->PicY);

    // frame_rate
    if (GetValue(Fp, "FRAME_RATE", sLine) == 0)
        goto end;
    {
        double frameRate;
        int  timeRes, timeInc;

        frameRate = (double)(int)atoi(sLine);

        timeInc = 1;
        while ((int)frameRate != frameRate) {
            timeInc *= 10;
            frameRate *= 10;
        }
        timeRes = (int) frameRate;
        // divide 2 or 5
        if (timeInc%2 == 0 && timeRes%2 == 0) {
            timeInc /= 2;
            timeRes /= 2;
        }
        if (timeInc%5 == 0 && timeRes%5 == 0) {
            timeInc /= 5;
            timeRes /= 5;
        }

        if (timeRes == 2997 && timeInc == 100) {
            timeRes = 30000;
            timeInc = 1001;
        }
        pEncCfg->FrameRate = (timeInc - 1) << 16;
        pEncCfg->FrameRate |= timeRes;
    }

    // frame count
    if (GetValue(Fp, "FRAME_NUMBER_ENCODED", sLine) == 0)
        goto end;
    sscanf(sLine, "%d", &pEncCfg->NumFrame);

    if (GetValue(Fp, "VERSION_ID", sLine) == 0)
        goto end;
    sscanf(sLine, "%d", &pEncCfg->VersionID);

    if (GetValue(Fp, "RESTART_INTERVAL", sLine) == 0)
        goto end;
    sscanf(sLine, "%d", &pEncCfg->RstIntval);

    if (GetValue(Fp, "IMG_FORMAT", sLine) == 0)
        goto end;
    sscanf(sLine, "%d", (Int32*)&pEncCfg->SrcFormat);

    if (GetValue(Fp, "QMATRIX_TABLE", sLine) == 0)
        goto end;
    sscanf(sLine, "%s", (char *)pEncCfg->QMatTabName);

    if (GetValue(Fp, "QMATRIX_PREC0", sLine) == 0)
        pEncCfg->QMatPrec0 = 0;
    else
        sscanf(sLine, "%d", &pEncCfg->QMatPrec0);

    if (GetValue(Fp, "QMATRIX_PREC1", sLine) == 0)
        pEncCfg->QMatPrec1 = 0;
    else
        sscanf(sLine, "%d", &pEncCfg->QMatPrec1);

    if (GetValue(Fp, "HUFFMAN_TABLE", sLine) == 0)
        goto end;
    sscanf(sLine, "%s", (char *)pEncCfg->HuffTabName);

    res = 1;
end:
    if(Fp)
        fclose(Fp);
    return res;
}




int StoreYuvImageBurstFormat_V20(int chromaStride, Uint8 * dst, int picWidth, int picHeight, Uint32 bitDepth,
    Uint64 addrY, Uint64 addrCb, Uint64 addrCr, Uint32 stride, FrameFormat format, int endian, CbCrInterLeave interLeave, PackedFormat packed)
{
    int size;
    int y, nY = 0, nCb, nCr;
    Uint64 addr;
    int lumaSize, chromaSize = 0, chromaWidth = 0, chromaHeight =0;
    Uint8 * puc;
    int chromaStride_i = 0;
    Uint32  bytesPerPixel = (bitDepth + 7) / 8;

    chromaStride_i = chromaStride;

    switch (format)
    {
    case FORMAT_420:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize  = (picWidth/2) * (picHeight/2);
        chromaWidth = picWidth/2;
        chromaHeight = nY;
        break;
    case FORMAT_440:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth) * (picHeight / 2);
        chromaWidth = picWidth;
        chromaHeight = nY;
        break;
    case FORMAT_422:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = (picWidth/2) * picHeight ;
        chromaWidth = (picWidth/2);
        chromaHeight = nY*2;
        break;
    case FORMAT_444:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = picWidth * picHeight;
        chromaWidth = picWidth;
        chromaHeight = nY*2;
        break;
    case FORMAT_400:
        nY = picHeight;
        nCb = nCr = 0;
        chromaSize = 0;
        chromaWidth = 0;
        chromaHeight = 0;
        break;
    default:
        return 0;
    }

    puc = dst;
    addr = addrY;

    if (packed)
    {
        if (packed == PACKED_FORMAT_444)
            picWidth *= 3;
        else
            picWidth *= 2;

        chromaSize = 0;
    }

    lumaSize = picWidth * nY;

    size = lumaSize + chromaSize*2;

    lumaSize    *= bytesPerPixel;
    chromaSize  *= bytesPerPixel;
    size        *= bytesPerPixel;
    picWidth    *= bytesPerPixel;
    chromaWidth *= bytesPerPixel;

    if (interLeave){
        chromaSize = chromaSize *2;
        chromaWidth = chromaWidth *2;
        chromaStride_i = chromaStride_i;
    }

    if ((picWidth == stride) && (chromaWidth == chromaStride_i))
    {
        JpuReadMem(addr, (Uint8 *)( puc ), lumaSize, endian);

        if(packed)
            return size;

        if (interLeave)
        {
            puc = dst + lumaSize;
            addr = addrCb;
            JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian);
        }
        else
        {
            puc = dst + lumaSize;
            addr = addrCb;
            JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian);

            puc = dst + lumaSize + chromaSize;
            addr = addrCr;
            JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian);
        }
    }
    else
    {
        for (y = 0; y < nY; ++y) {
            JpuReadMem(addr + stride * y, (Uint8 *)(puc + y * picWidth), picWidth,  endian);
        }

        if(packed)
            return size;

        if (interLeave)
        {

            puc = dst + lumaSize;
            addr = addrCb;
            for (y = 0; y < (chromaHeight/2); ++y) {
                JpuReadMem(addr + (chromaStride_i)*y, (Uint8 *)(puc + y*(chromaWidth)), (chromaWidth), endian);
            }
        }
        else
        {
            puc = dst + lumaSize;
            addr = addrCb;
            for (y = 0; y < nCb; ++y) {
                JpuReadMem(addr + chromaStride_i * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth,  endian);
            }

            puc = dst + lumaSize + chromaSize;
            addr = addrCr;
            for (y = 0; y < nCr; ++y) {
                JpuReadMem(addr + chromaStride_i * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth,  endian);
            }

        }

    }
    return size;
}//lint !e429

void GetMcuUnitSize(int format, int *mcuWidth, int *mcuHeight)
{
    switch(format)
    {
    case FORMAT_420:
        *mcuWidth = 16;
        *mcuHeight = 16;
        break;
    case FORMAT_422:
        *mcuWidth = 16;
        *mcuHeight = 8;
        break;
    case FORMAT_440:
        *mcuWidth = 8;
        *mcuHeight = 16;
        break;
    default://FORMAT_444,400
        *mcuWidth = 8;
        *mcuHeight = 8;
        break;
    }
}

int GetFrameBufSize(int framebufFormat, int picWidth, int picHeight)
{
    int framebufSize = 0;
    int framebufWidth, framebufHeight;
    framebufWidth = picWidth;
    framebufHeight = picHeight;

    switch (framebufFormat)
    {
    case FORMAT_420:
        framebufSize = framebufWidth * ((framebufHeight+1)/2*2) + ((framebufWidth+1)/2)*((framebufHeight+1)/2)*2;
        break;
    case FORMAT_440:
        framebufSize = framebufWidth * ((framebufHeight+1)/2*2) + framebufWidth*((framebufHeight+1)/2)*2;
        break;
    case FORMAT_422:
        framebufSize = framebufWidth * framebufHeight + ((framebufWidth+1)/2)*framebufHeight*2;
        break;
    case FORMAT_444:
        framebufSize = framebufWidth * framebufHeight * 3;
        break;
    case FORMAT_400:
        framebufSize = framebufWidth * framebufHeight;
        break;
    }

    framebufSize = ((framebufSize+7)&~7);

    return framebufSize;
}

STATIC void GetFrameBufStride(FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed, BOOL scalerOn,
                              Uint32 width, Uint32 height, Uint32 bytePerPixel,
                              Uint32* oLumaStride, Uint32* oLumaHeight, Uint32* oChromaStride, Uint32* oChromaHeight)
{
    Uint32 lStride, cStride;
    Uint32 lHeight, cHeight;

    lStride = JPU_CEIL(8, width);
    lHeight = height;
    cHeight = height/2;

    if (packed == PACKED_FORMAT_NONE) {
        Uint32 chromaDouble = (cbcrIntlv == CBCR_SEPARATED) ? 1 : 2;

        switch (subsample) {
        case FORMAT_400: cStride = 0;                        cHeight = 0;        break;
        case FORMAT_420: cStride = (lStride/2)*chromaDouble; cHeight = height/2; break;
        case FORMAT_422: cStride = (lStride/2)*chromaDouble; cHeight = height;   break;
        case FORMAT_440: cStride = lStride*chromaDouble;     cHeight = height/2; break;
        case FORMAT_444: cStride = lStride*chromaDouble;     cHeight = height;   break;
        default:         cStride = 0; lStride = 0; cHeight = 0; break;
        }
    }
    else {
        switch (packed) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_VYUY: lStride = lStride*2; cStride = 0; cHeight = 0; break;
        case PACKED_FORMAT_444:      lStride = lStride*3; cStride = 0; cHeight = 0; break;
        default:                     lStride = 0;         cStride  = 0;             break;
        }
    }


    if (scalerOn == TRUE) {
        /* Luma stride */
        if (subsample == FORMAT_420 || subsample == FORMAT_422 || (PACKED_FORMAT_422_YUYV <= packed && packed <= PACKED_FORMAT_422_VYUY)) {
            lStride = JPU_CEIL(32, lStride);
        }
        else {
            lStride = JPU_CEIL(16, lStride);
        }
        /* Chroma stride */
        if (cbcrIntlv == CBCR_SEPARATED) {
            if (subsample == FORMAT_444) {
                cStride = JPU_CEIL(16, cStride);
            }
            else {
                cStride = JPU_CEIL(8, cStride);
            }
        }
        else {
            cStride = JPU_CEIL(32, cStride);

        }
    }
    else {
        lStride = JPU_CEIL(8, lStride);
        if (subsample == FORMAT_420 || subsample == FORMAT_422) {
            cStride = JPU_CEIL(16, cStride);
        }
        else {
            cStride = JPU_CEIL(8, cStride);
        }
    }
    lHeight = JPU_CEIL(8, lHeight);
    cHeight = JPU_CEIL(8, cHeight);

    lStride *= bytePerPixel;
    cStride *= bytePerPixel;

    if (oLumaStride)   *oLumaStride   = lStride;
    if (oLumaHeight)   *oLumaHeight   = lHeight;
    if (oChromaStride) *oChromaStride = cStride;
    if (oChromaHeight) *oChromaHeight = cHeight;
}

BOOL AllocateFrameBuffer(Uint32 instIdx, FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                         Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth, Uint32 num)
{
    fb_context *fb;
    Uint32  fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
    Uint32  fbLumaSize, fbChromaSize, fbSize;
    Uint32  i;
    Uint32  bytePerPixel = (bitDepth + 7)/8;

    if (rotation == 90 || rotation == 270) {
        if (subsample == FORMAT_422) subsample = FORMAT_440;
        else if (subsample == FORMAT_440) subsample = FORMAT_422;
    }

    GetFrameBufStride(subsample, cbcrIntlv, packed, scalerOn, width, height, bytePerPixel, &fbLumaStride, &fbLumaHeight, &fbChromaStride, &fbChromaHeight);

    fbLumaSize   = fbLumaStride * fbLumaHeight;
    fbChromaSize = fbChromaStride * fbChromaHeight;

    if (cbcrIntlv == CBCR_SEPARATED) {
        /* fbChromaSize MUST be zero when format is packed mode */
        fbSize = fbLumaSize + 2*fbChromaSize;

    }
    else {
        /* Semi-planar */
        fbSize = fbLumaSize + fbChromaSize;
    }

    fb = &s_fb[instIdx];

    if (fb->vb_base.size == 0) {
        fb->vb_base.size = fbSize;
        fb->vb_base.size *= num;
        if (jdi_allocate_dma_memory(&fb->vb_base) < 0) {
            JLOG(ERR, "Fail to allocate frame buffer size=%d\n", fb->vb_base.size);
            return FALSE;
        }

        fb->last_addr = fb->vb_base.phys_addr;
    }

    for (i=fb->last_num; i<fb->last_num+num; i++) {
        fb->frameBuf[i].Format = subsample;
        fb->frameBuf[i].Index  = i;
        fb->frameBuf[i].vbY.phys_addr = fb->last_addr;
        fb->frameBuf[i].vbY.size = fbLumaSize;

        fb->last_addr += fb->frameBuf[i].vbY.size;
        fb->last_addr = JPU_CEIL(8, fb->last_addr);

        if (fbChromaSize) {
            fb->frameBuf[i].vbCb.phys_addr = fb->last_addr;
            fb->frameBuf[i].vbCb.size = fbChromaSize;

            fb->last_addr += fb->frameBuf[i].vbCb.size;
            fb->last_addr = JPU_CEIL(8, fb->last_addr);

            fb->frameBuf[i].vbCr.phys_addr = (cbcrIntlv == CBCR_SEPARATED) ? fb->last_addr : 0;
            fb->frameBuf[i].vbCr.size      = (cbcrIntlv == CBCR_SEPARATED) ? fbChromaSize  : 0;

            fb->last_addr += fb->frameBuf[i].vbCr.size;
            fb->last_addr = JPU_CEIL(8, fb->last_addr);
        }

        fb->frameBuf[i].strideY = fbLumaStride;
        fb->frameBuf[i].strideC = fbChromaStride;
    }

    fb->last_num += num;

    return TRUE;
}

BOOL UpdateFrameBuffers(Uint32 instIdx, Uint32 num, FRAME_BUF *frameBuf)
{
    fb_context *fb = &s_fb[instIdx];
    Uint32 index = num - 1;
    fb->last_num = num;
    fb->last_addr = frameBuf[index].vbY.phys_addr;
    fb->last_addr += fb->frameBuf[index].vbCb.size;
    fb->last_addr = JPU_CEIL(8, fb->last_addr);
    fb->last_addr += fb->frameBuf[index].vbCr.size;
    fb->last_addr = JPU_CEIL(8, fb->last_addr);
    JLOG(INFO, "%s: Update %d buffers\r\n",__FUNCTION__, num);
    memcpy(fb->frameBuf, frameBuf, sizeof(FRAME_BUF) * num);
    return TRUE;
}

BOOL AttachOneFrameBuffer(Uint32 instIdx, FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                         Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth,
                         void *virtAddress, Uint32 size, Uint32 *bufferIndex)
{
    fb_context *fb;
    Uint32  fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
    Uint32  fbLumaSize, fbChromaSize;
    Uint32  fbSize;
    Uint32  bytePerPixel = (bitDepth + 7)/8;
    Uint32  i;


    JLOG(INFO, "%s function in width, height = [%d, %d]\r\n", __FUNCTION__, width, height);
    if (rotation == 90 || rotation == 270) {
        if (subsample == FORMAT_422) subsample = FORMAT_440;
        else if (subsample == FORMAT_440) subsample = FORMAT_422;
    }

    GetFrameBufStride(subsample, cbcrIntlv, packed, scalerOn, width, height, bytePerPixel, &fbLumaStride, &fbLumaHeight, &fbChromaStride, &fbChromaHeight);
    fbLumaSize   = fbLumaStride * fbLumaHeight;
    fbChromaSize = fbChromaStride * fbChromaHeight;

    if (cbcrIntlv == CBCR_SEPARATED) {
        /* fbChromaSize MUST be zero when format is packed mode */
        fbSize = fbLumaSize + 2*fbChromaSize;

    }
    else {
        /* Semi-planar */
        fbSize = fbLumaSize + fbChromaSize;
    }

    fb = &s_fb[instIdx];
    fb->vb_base.virt_addr = (unsigned long)virtAddress;
    fb->vb_base.size = fbSize;
    if (jdi_attach_dma_memory(&fb->vb_base) < 0) {
        JLOG(ERR, "Fail to attach frame buffer\n");
        return FALSE;
    }
    fb->last_addr = fb->vb_base.phys_addr;

    i = fb->last_num;
    JLOG(INFO, "%s: store on index %d\r\n", __FUNCTION__, i);
    fb->frameBuf[i].Format = subsample;
    fb->frameBuf[i].Index  = i;
    fb->frameBuf[i].vbY.phys_addr = fb->vb_base.phys_addr;
    fb->frameBuf[i].vbY.size = fbLumaSize;
    // Store virt addr
    fb->frameBuf[i].vbY.virt_addr = fb->vb_base.virt_addr;

    fb->last_addr += fb->frameBuf[i].vbY.size;
    fb->last_addr = JPU_CEIL(8, fb->last_addr);
    JLOG(INFO, "%s: fbChromaSize = %d, fbLumaSize = %d\r\n", __FUNCTION__, fbChromaSize, fbLumaSize);
    if (fbChromaSize) {
        fb->frameBuf[i].vbCb.phys_addr = fb->last_addr;
        fb->frameBuf[i].vbCb.size = fbChromaSize;

        fb->last_addr += fb->frameBuf[i].vbCb.size;
        fb->last_addr = JPU_CEIL(8, fb->last_addr);

        fb->frameBuf[i].vbCr.phys_addr = (cbcrIntlv == CBCR_SEPARATED) ? fb->last_addr : 0;
        fb->frameBuf[i].vbCr.size      = (cbcrIntlv == CBCR_SEPARATED) ? fbChromaSize  : 0;

        fb->last_addr += fb->frameBuf[i].vbCr.size;
        fb->last_addr = JPU_CEIL(8, fb->last_addr);
    }
    JLOG(INFO, "%s vbCb = %x size = %d, vbCr = %x size = %d\r\n", __FUNCTION__, fb->frameBuf[i].vbCb.phys_addr,
        fb->frameBuf[i].vbCb.size, fb->frameBuf[i].vbCr.phys_addr, fb->frameBuf[i].vbCr.size);
    fb->frameBuf[i].strideY = fbLumaStride;
    fb->frameBuf[i].strideC = fbChromaStride;

    *bufferIndex = fb->last_num;
    fb->last_num += 1;

    JLOG(INFO, "%s function OUT, number = %d\r\n", __FUNCTION__, fb->last_num);
    return TRUE;

}

void *AllocateOneFrameBuffer(Uint32 instIdx, FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                         Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth, Uint32 *bufferIndex)
{
    fb_context *fb;
    Uint32  fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
    Uint32  fbLumaSize, fbChromaSize, fbSize;
    Uint32  i;
    Uint32  bytePerPixel = (bitDepth + 7)/8;
    void * virt_addr = NULL;

    JLOG(INFO, "%s function in width, height = [%d, %d]\r\n", __FUNCTION__, width, height);
    if (rotation == 90 || rotation == 270) {
        if (subsample == FORMAT_422) subsample = FORMAT_440;
        else if (subsample == FORMAT_440) subsample = FORMAT_422;
    }

    GetFrameBufStride(subsample, cbcrIntlv, packed, scalerOn, width, height, bytePerPixel, &fbLumaStride, &fbLumaHeight, &fbChromaStride, &fbChromaHeight);
    fbLumaSize   = fbLumaStride * fbLumaHeight;
    fbChromaSize = fbChromaStride * fbChromaHeight;

    if (cbcrIntlv == CBCR_SEPARATED) {
        /* fbChromaSize MUST be zero when format is packed mode */
        fbSize = fbLumaSize + 2*fbChromaSize;

    }
    else {
        /* Semi-planar */
        fbSize = fbLumaSize + fbChromaSize;
    }

    if (fbSize == 0)
    {
        switch (subsample)
        {
        case FORMAT_420:
            fbSize = width * height * 3 / 2;
            break;
        case FORMAT_422:
        case FORMAT_440:
            fbSize = width * height * 2;
        case FORMAT_444:
            fbSize = width * height * 3;
            /* code */
            break;
        default:
            break;
        }
        JLOG(WARN, "%s: fbSize == 0! Use defalut size %d\r\n", __FUNCTION__, fbSize);
    }

    fb = &s_fb[instIdx];
    fb->vb_base.size = fbSize;
    if (jdi_allocate_dma_memory(&fb->vb_base) < 0) {
        JLOG(ERR, "Fail to allocate frame buffer size=%d\n", fb->vb_base.size);
        return NULL;
    }
    fb->last_addr = fb->vb_base.phys_addr;

    i = fb->last_num;
    JLOG(INFO, "%s: store on index %d\r\n", __FUNCTION__, i);
    fb->frameBuf[i].Format = subsample;
    fb->frameBuf[i].Index  = i;
    fb->frameBuf[i].vbY.phys_addr = fb->vb_base.phys_addr;
    fb->frameBuf[i].vbY.size = fbLumaSize;
    // Store virt addr
    fb->frameBuf[i].vbY.virt_addr = fb->vb_base.virt_addr;

    fb->last_addr += fb->frameBuf[i].vbY.size;
    fb->last_addr = JPU_CEIL(8, fb->last_addr);
    JLOG(INFO, "%s: fbChromaSize = %d, fbLumaSize = %d\r\n", __FUNCTION__, fbChromaSize, fbLumaSize);
    if (fbChromaSize) {
        fb->frameBuf[i].vbCb.phys_addr = fb->last_addr;
        fb->frameBuf[i].vbCb.size = fbChromaSize;

        fb->last_addr += fb->frameBuf[i].vbCb.size;
        fb->last_addr = JPU_CEIL(8, fb->last_addr);

        fb->frameBuf[i].vbCr.phys_addr = (cbcrIntlv == CBCR_SEPARATED) ? fb->last_addr : 0;
        fb->frameBuf[i].vbCr.size      = (cbcrIntlv == CBCR_SEPARATED) ? fbChromaSize  : 0;

        fb->last_addr += fb->frameBuf[i].vbCr.size;
        fb->last_addr = JPU_CEIL(8, fb->last_addr);
    }

    fb->frameBuf[i].strideY = fbLumaStride;
    fb->frameBuf[i].strideC = fbChromaStride;

    *bufferIndex = fb->last_num;
    fb->last_num += 1;

    virt_addr = (void *)fb->vb_base.virt_addr;

    JLOG(INFO, "%s function OUT, number = %d, return = %p\r\n", __FUNCTION__, fb->last_num, virt_addr);
    return virt_addr;
}

int GetDPBBufSize(int framebufFormat, int picWidth, int picHeight, int picWidth_C, int interleave)
{
    int framebufSize = 0;
    int framebufWidth, framebufHeight, framebufWidth_C;
    framebufWidth = picWidth;
    framebufHeight = picHeight;
    framebufWidth_C = picWidth_C;

    switch (framebufFormat)
    {
    case FORMAT_420:
        if (interleave == 0)
            framebufSize = framebufWidth * ((framebufHeight+1)/2*2) + framebufWidth_C *((framebufHeight+1)/2)*2;
        else
            framebufSize = framebufWidth * ((framebufHeight+1)/2*2) + framebufWidth_C *((framebufHeight+1)/2);
        break;
    case FORMAT_440:
        if (interleave == 0)
            framebufSize = framebufWidth * ((framebufHeight+1)/2*2) + framebufWidth_C*((framebufHeight+1)/2)*2 *2;
        else
            framebufSize = framebufWidth * ((framebufHeight+1)/2*2) + framebufWidth_C*((framebufHeight+1)/2)*2;
        break;
    case FORMAT_422:
        if (interleave == 0)
            framebufSize = framebufWidth * framebufHeight + framebufWidth_C*framebufHeight*2;
        else
            framebufSize = framebufWidth * framebufHeight + framebufWidth_C*framebufHeight;
        break;
    case FORMAT_444:
        framebufSize = framebufWidth * framebufHeight * 3;
        break;
    case FORMAT_400:
        framebufSize = framebufWidth * framebufHeight;
        if (interleave != 0)
            printf("Warning: 400 does not have interleave mode ! \n" );
        break;
    }

    framebufSize = ((framebufSize+7)&~7);

    return framebufSize;
}

BOOL ParseDecTestLongArgs(void* config, const char* argName, char* value)
{
    BOOL            ret = TRUE;
    DecConfigParam* dec = (DecConfigParam*)config;

    if (strcmp(argName, "output") == 0) {
        strcpy(dec->yuvFileName, value);
    }
    else if (strcmp(argName, "input") == 0) {
        strcpy(dec->bitstreamFileName, value);
    }
    else if (strcmp(argName, "stream-endian") == 0) {
        dec->StreamEndian = atoi(value);
    }
    else if (strcmp(argName, "frame-endian") == 0) {
        dec->FrameEndian = atoi(value);
    }
    else if (strcmp(argName, "pixelj") == 0) {
        dec->pixelJustification = atoi(value);
        if (dec->pixelJustification >= PIXEL_16BIT_JUSTIFICATION_MAX) {
            JLOG(ERR, "Invalid pixel justification\n");
            ret = FALSE;
        }
    }
    else if (strcmp(argName, "bs-size") == 0) {
        dec->bsSize = atoi(value);
		JLOG(INFO, "bitstream buffer size is:%d\n",dec->bsSize);
        if (dec->bsSize == 0) {
            JLOG(ERR, "bitstream buffer size is 0\n");
            ret = FALSE;
        }
    }
    else if (strcmp(argName, "roi") == 0) {
        char* val;
        val = strtok(value, ",");
        if (val == NULL) {
            JLOG(ERR, "Invalid ROI option: %s\n", value);
            ret = FALSE;
        }
        else {
            dec->roiOffsetX = atoi(val);
        }
        val = strtok(NULL, ",");
        if (val == NULL) {
            JLOG(ERR, "Invalid ROI option: %s\n", value);
            ret = FALSE;
        }
        else {
            dec->roiOffsetY = atoi(val);
        }
        val = strtok(NULL, ",");
        if (val == NULL) {
            JLOG(ERR, "Invalid ROI option: %s\n", value);
            ret = FALSE;
        }
        else {
            dec->roiWidth = atoi(val);
        }
        val = strtok(NULL, ",");
        if (val == NULL) {
            JLOG(ERR, "Invalid ROI option: %s\n", value);
            ret = FALSE;
        }
        else {
            dec->roiHeight = atoi(val);
        }
        dec->roiEnable = TRUE;
    }
    else if (strcmp(argName, "subsample") == 0) {
        if (strcasecmp(value, "none") == 0) {
            dec->subsample = FORMAT_MAX;
        }
        else if (strcasecmp(value, "420") == 0) {
            dec->subsample = FORMAT_420;
        }
        else if (strcasecmp(value, "422") == 0) {
            dec->subsample = FORMAT_422;
        }
        else if (strcasecmp(value, "444") == 0) {
            dec->subsample = FORMAT_444;
        }
        else {
            JLOG(ERR, "Not supported sub-sample: %s\n", value);
            ret = FALSE;
        }
    }
    else if (strcmp(argName, "ordering") == 0) {
        if (strcasecmp(value, "none") == 0) {
            dec->cbcrInterleave = CBCR_SEPARATED;
            dec->packedFormat   = PACKED_FORMAT_NONE;
        }
        else if (strcasecmp(value, "nv12") == 0) {
            dec->cbcrInterleave = CBCR_INTERLEAVE;
            dec->packedFormat   = PACKED_FORMAT_NONE;
        }
        else if (strcasecmp(value, "nv21") == 0) {
            dec->cbcrInterleave = CRCB_INTERLEAVE;
            dec->packedFormat   = PACKED_FORMAT_NONE;
        }
        else if (strcasecmp(value, "yuyv") == 0) {
            dec->cbcrInterleave = CBCR_SEPARATED;
            dec->packedFormat   = PACKED_FORMAT_422_YUYV;
        }
        else if (strcasecmp(value, "uyvy") == 0) {
            dec->cbcrInterleave = CBCR_SEPARATED;
            dec->packedFormat   = PACKED_FORMAT_422_UYVY;
        }
        else if (strcasecmp(value, "yvyu") == 0) {
            dec->cbcrInterleave = CBCR_SEPARATED;
            dec->packedFormat   = PACKED_FORMAT_422_YVYU;
        }
        else if (strcasecmp(value, "vyuy") == 0) {
            dec->cbcrInterleave = CBCR_SEPARATED;
            dec->packedFormat   = PACKED_FORMAT_422_VYUY;
        }
        else if (strcasecmp(value, "ayuv") == 0) {
            dec->cbcrInterleave = CBCR_SEPARATED;
            dec->packedFormat   = PACKED_FORMAT_444;
        }
        else {
            JLOG(ERR, "Not supported ordering: %s\n", value);
            ret = FALSE;
        }
    }
    else if (strcmp(argName, "rotation") == 0) {
        dec->rotation = atoi(value);
    }
    else if (strcmp(argName, "mirror") == 0) {
        dec->mirror = (JpgMirrorDirection)atoi(value);
    }
    else if (strcmp(argName, "scaleH") == 0) {
        dec->iHorScaleMode = atoi(value);
    }
    else if (strcmp(argName, "scaleV") == 0) {
        dec->iVerScaleMode = atoi(value);
    }
    else {
        JLOG(ERR, "Not defined option: %s\n", argName);
        ret = FALSE;
    }

    return ret;
}

BOOL ParseEncTestLongArgs(void* config, const char* argName, char* value)
{
    BOOL            ret = TRUE;
    EncConfigParam* enc = (EncConfigParam*)config;

    if (strcmp(argName, "output") == 0) {
        strcpy(enc->bitstreamFileName, value);
    }
    else if (strcmp(argName, "input") == 0) {
        strcpy(enc->cfgFileName, value);
    }
    else if (strcmp(argName, "12bit") == 0) {
        enc->extendedSequential = TRUE;
    }
    else if (strcmp(argName, "cfg-dir") == 0) {
        strcpy(enc->strCfgDir, value);
    }
    else if (strcmp(argName, "yuv-dir") == 0) {
        strcpy(enc->strYuvDir, value);
    }
    else if (strcmp(argName, "stream-endian") == 0) {
        enc->StreamEndian = atoi(value);
    }
    else if (strcmp(argName, "frame-endian") == 0) {
        enc->FrameEndian = atoi(value);
    }
    else if (strcmp(argName, "pixelj") == 0) {
        enc->pixelJustification = atoi(value);
        if (enc->pixelJustification >= PIXEL_16BIT_JUSTIFICATION_MAX) {
            JLOG(ERR, "Invalid pixel justification\n");
            ret = FALSE;
        }
    }
    else if (strcmp(argName, "bs-size") == 0) {
        enc->bsSize = atoi(value);
        if (enc->bsSize == 0) {
            JLOG(ERR, "bitstream buffer size is 0\n");
            ret = FALSE;
        }
    }
    else if (strcmp(argName, "quality") == 0) {
        enc->encQualityPercentage = atoi(value);
        if (enc->encQualityPercentage > 100) {
            JLOG(ERR, "Invalid quality factor: %d\n", enc->encQualityPercentage);
            ret = FALSE;
        }
    }
    else if (strcmp(argName, "enable-tiledMode") == 0) {
        enc->tiledModeEnable = (BOOL)atoi(value);
    }
    else if (strcmp(argName, "slice-height") == 0) {
        enc->sliceHeight = atoi(value);
    }
    else if (strcmp(argName, "enable-slice-intr") == 0) {
        enc->sliceInterruptEnable = atoi(value);
    }
    else if (strcmp(argName, "rotation") == 0) {
        enc->rotation = atoi(value);
    }
    else if (strcmp(argName, "mirror") == 0) {
        enc->mirror = (JpgMirrorDirection)atoi(value);
    }
    else {
        JLOG(ERR, "Not defined option: %s\n", argName);
        ret = FALSE;
    }

    return ret;
}

static BOOL ParseNoneTestLongArgs(void* config, const char* argName, char* value)
{
    return FALSE;
}

static BOOL ParseMultipleTestArgs(TestMultiConfig* config, const char* argName, char* value)
{
    char*   tempArg;
    Uint32  i;
    BOOL (*parsers[3])(void* config, const char*, char*) = {
        ParseEncTestLongArgs,
        ParseDecTestLongArgs,
        ParseNoneTestLongArgs,
    };

    tempArg = strtok(value, ",");
    for(i = 0; i < MAX_NUM_INSTANCE; i++) {
        if (parsers[config->type[i]]((void*)&config->u[i], argName, tempArg) == FALSE) {
            return FALSE;
        }
        tempArg = strtok(NULL, ",");
        if (tempArg == NULL) {
            break;
        }
    }

    return TRUE;
}

BOOL ParseMultiLongOptions(TestMultiConfig* config, const char* argName, char* value)
{
    BOOL    ret = TRUE;
    Uint32  i;
    char*   tempArg;

    if (strcmp(argName, "instance-num") == 0) {
        config->numInstances = atoi(value);
    }
    else if (strcmp(argName, "input") == 0) {
        ret = ParseMultipleTestArgs(config, argName, value);
    }
    else if (strcmp(argName, "output") == 0) {
        ret = ParseMultipleTestArgs(config, argName, value);
    }
    else if (strcmp(argName, "stream-endian") == 0) {
        ret = ParseMultipleTestArgs(config, argName, value);
    }
    else if (strcmp(argName, "frame-endian") == 0) {
        ret = ParseMultipleTestArgs(config, argName, value);
    }
    else if (strcmp(argName, "pixelj") == 0) {
        ret = ParseMultipleTestArgs(config, argName, value);
    }
    else if (strcmp(argName, "bs-size") == 0) {
        ret = ParseMultipleTestArgs(config, argName, value);
    }
    else if (strcmp(argName, "cfg-dir") == 0) {
        for (i=0; i<MAX_NUM_INSTANCE; i++) {
            if (config->type[i] == JPU_ENCODER) {
                ParseEncTestLongArgs((void*)&config->u[i], argName, value);
            }
        }
    }
    else if (strcmp(argName, "yuv-dir") == 0) {
        for (i=0; i<MAX_NUM_INSTANCE; i++) {
            if (config->type[i] == JPU_ENCODER) {
                ParseEncTestLongArgs((void*)&config->u[i], argName, value);
            }
        }
    }
    else if (strcmp(argName, "slice-height") == 0) {
        tempArg = strtok(value, ",");
        for(i = 0; i < MAX_NUM_INSTANCE; i++) {
            if (config->type[i] == JPU_ENCODER) {
                ParseEncTestLongArgs((void*)&config->u[i], argName, tempArg);
            }
            else {
                ParseDecTestLongArgs((void*)&config->u[i], argName, tempArg);
            }
            tempArg = strtok(NULL, ",");
            if (tempArg == NULL)
                break;
        }
    }
    else if (strcmp(argName, "enable-slice-intr") == 0) {
        tempArg = strtok(value, ",");
        for(i = 0; i < MAX_NUM_INSTANCE; i++) {
            if (config->type[i] == JPU_ENCODER) {
                ParseEncTestLongArgs((void*)&config->u[i], argName, tempArg);
            }
            else {
                ParseDecTestLongArgs((void*)&config->u[i], argName, tempArg);
            }
            tempArg = strtok(NULL, ",");
            if (tempArg == NULL)
                break;
        }
    }
    else if (strcmp(argName, "enable-tiledMode") == 0) {
        tempArg = strtok(value, ",");
        for(i = 0; i < MAX_NUM_INSTANCE; i++) {
            if (config->type[i] == JPU_ENCODER) {
                ParseEncTestLongArgs((void*)&config->u[i], argName, tempArg);
            }
            tempArg = strtok(NULL, ",");
            if (tempArg == NULL)
                break;
        }
    }
    else {
        JLOG(ERR, "Not defined option: %s\n", argName);
        ret = FALSE;
    }

    return ret;
}

int GetFrameBufBase(int instIdx)
{
    fb_context *fb;
    fb = &s_fb[instIdx];

    return fb->vb_base.phys_addr;
}

int GetFrameBufAllocSize(int instIdx)
{
    fb_context *fb;
    fb = &s_fb[instIdx];

    return (fb->last_addr - fb->vb_base.phys_addr);
}


FRAME_BUF *GetFrameBuffer(int instIdx, int idx)
{
    fb_context *fb;
    fb = &s_fb[instIdx];
    return &fb->frameBuf[idx];
}

Uint32 GetFrameBufferCount(int instIdx)
{
    fb_context *fb;
    fb = &s_fb[instIdx];
    return fb->last_num;
}

FRAME_BUF* FindFrameBuffer(int instIdx, PhysicalAddress addrY)
{
    int i;
    fb_context *fb;

    fb = &s_fb[instIdx];

    for (i=0; i <MAX_FRAME; i++)
    {
        if (fb->frameBuf[i].vbY.phys_addr == addrY)
        {
            return &fb->frameBuf[i];
        }
    }

    return NULL;
}

void FreeFrameBuffer(int instIdx)
{
    fb_context *fb;

    fb = &s_fb[instIdx];

    fb->last_num = 0;
    fb->last_addr = -1;

    if(fb->vb_base.base)
    {
        jdi_free_dma_memory(&fb->vb_base);
        fb->vb_base.base = 0;
    }
}

char* GetFileExtension(const char* filename)
{
    Int32      len;
    Int32      i;

    len = strlen(filename);
    for (i=len-1; i>=0; i--) {
        if (filename[i] == '.') {
            return (char*)&filename[i+1];
        }
    }

    return NULL;
}


