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
#include "jpuapifunc.h"
#include "regdefine.h"
#include "jpulog.h"

/******************************************************************************
Codec Instance Slot Management
******************************************************************************/
// gGetBits defined but not used
//static Uint32 tGetBits(JpgDecInfo *jpg, int endian, int byteCnt);

const char lendian[4] = {0x49, 0x49, 0x2A, 0x00};
const char bendian[4] = {0x4D, 0x4D, 0x00, 0x2A};

const char *jfif = "JFIF";
const char *jfxx = "JFXX";
const char *exif = "Exif";

Uint8 sJpuCompInfoTable[5][24] = {
    { 00, 02, 02, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //420
    { 00, 02, 01, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //422H
    { 00, 01, 02, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //422V
    { 00, 01, 01, 00, 00, 00, 01, 01, 01, 01, 01, 01, 02, 01, 01, 01, 01, 01, 03, 00, 00, 00, 00, 00 }, //444
    { 00, 01, 01, 00, 00, 00, 01, 00, 00, 00, 00, 00, 02, 00, 00, 00, 00, 00, 03, 00, 00, 00, 00, 00 }, //400
};

Uint8 sJpuCompInfoTable_EX[5][24] = {
    { 00, 02, 02, 00, 02, 02, 01, 01, 01, 01, 03, 03, 02, 01, 01, 01, 03, 03, 03, 00, 00, 00, 00, 00 }, //420
    { 00, 02, 01, 00, 02, 02, 01, 01, 01, 01, 03, 03, 02, 01, 01, 01, 03, 03, 03, 00, 00, 00, 00, 00 }, //422H
    { 00, 01, 02, 00, 02, 02, 01, 01, 01, 01, 03, 03, 02, 01, 01, 01, 03, 03, 03, 00, 00, 00, 00, 00 }, //422V
    { 00, 01, 01, 00, 02, 02, 01, 01, 01, 01, 03, 03, 02, 01, 01, 01, 03, 03, 03, 00, 00, 00, 00, 00 }, //444
    { 00, 01, 01, 00, 00, 00, 01, 00, 00, 00, 00, 00, 02, 00, 00, 00, 00, 00, 03, 00, 00, 00, 00, 00 }, //400
};

/*
 * The parameters of input source: iPackMode
 * The parameters of jpeg sample : iFormat
 */
Uint32 GetDec8bitBusReqNum(FrameFormat iFormat, PackedFormat oPackMode)
{
    Uint32 num = 0;

    if (oPackMode == PACKED_FORMAT_NONE) {
        switch (iFormat) {
        case FORMAT_400: num = 2; break;
        case FORMAT_420: num = 4; break;
        case FORMAT_422: num = 4; break;
        case FORMAT_440: num = 8; break;
        case FORMAT_444: num = 8; break;
        default:         num = 0; break;
        }
    }
    else {
        switch (oPackMode) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_VYUY:
            if (iFormat == FORMAT_400)      num = 1;
            else if (iFormat == FORMAT_420) num = 2;
            else if (iFormat == FORMAT_422) num = 2;
            else                            num = 4;
            break;
        case PACKED_FORMAT_444:
            num = 8;
            break;
        default:
            num = 0;
            break;
        }
    }

    return num;
}

Uint32 GetDec12bitBusReqNum(FrameFormat iFormat, PackedFormat oPackMode)
{
    Uint32 num = 0;

    if (oPackMode == PACKED_FORMAT_NONE) {
        switch (iFormat) {
        case FORMAT_400: num = 1; break;
        case FORMAT_420: num = 2; break;
        case FORMAT_422: num = 2; break;
        case FORMAT_440: num = 4; break;
        case FORMAT_444: num = 4; break;
        default:         num = 0; break;
        }
    }
    else {
        switch (oPackMode) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_VYUY:
            if (iFormat == FORMAT_400)      num = 1;
            else if (iFormat == FORMAT_420) num = 1;
            else if (iFormat == FORMAT_422) num = 1;
            else if (iFormat == FORMAT_440) num = 2;
            else if (iFormat == FORMAT_444) num = 2;
            else                            num = 0;
            break;
        case PACKED_FORMAT_444:
            num = 4;
            break;
        default:
            num = 0;
            break;
        }
    }

    return num;
}

/*
 * The parameters of input source: packMode
 * The parameters of jpeg sample : oFormat
 */
Uint32 GetEnc8bitBusReqNum(PackedFormat iPackMode, FrameFormat oFormat)
{
    Uint32 num = 0;

    if (iPackMode == PACKED_FORMAT_NONE) {
        switch (oFormat) {
        case FORMAT_400: num = 4; break;
        case FORMAT_440: num = 8; break;
        case FORMAT_444: num = 8; break;
        default:         num = 4; break;
        }
    }
    else {
        switch (iPackMode) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_VYUY:
            if (oFormat == FORMAT_400)      num = 2;
            else if (oFormat == FORMAT_440) num = 4;
            else if (oFormat == FORMAT_444) num = 4;
            else                            num = 2;
            break;
        case PACKED_FORMAT_444:
            num = 8;
            break;
        default:
            num = 0;
            break;
        }
    }

    return num;
}

/*
 * The parameters of input source: packMode
 * The parameters of jpeg sample : oFormat
 */
Uint32 GetEnc12bitBusReqNum(PackedFormat iPackMode, FrameFormat oFormat)
{
    Uint32 num = 0;

    if (iPackMode == PACKED_FORMAT_NONE) {
        switch (oFormat) {
        case FORMAT_400: num = 2; break;
        case FORMAT_440: num = 4; break;
        case FORMAT_444: num = 4; break;
        default:         num = 2; break;
        }
    }
    else {
        switch (iPackMode) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_VYUY:
            num = (oFormat == FORMAT_440 || oFormat == FORMAT_444) ? 2 : 1;
            break;
        case PACKED_FORMAT_444:
            num = 4;
            break;
        default:
            num = 0;
            break;
        }
    }

    return num;
}

/*
* GetJpgInstance() obtains a instance.
* It stores a pointer to the allocated instance in *ppInst
* and returns JPG_RET_SUCCESS on success.
* Failure results in 0(null pointer) in *ppInst and JPG_RET_FAILURE.
*/

JpgRet GetJpgInstance(JpgInst ** ppInst)
{
    Int32                   i;
    Uint32                  handleSize;
    JpgInst*                pJpgInst = 0;
    jpu_instance_pool_t*    jip;

    jip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!jip)
        return JPG_RET_INVALID_HANDLE;


    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
        pJpgInst = (JpgInst *)jip->jpgInstPool[i];

        if (!pJpgInst) {
            return JPG_RET_FAILURE;
        }
        if (!pJpgInst->inUse)
            break;
    }

    if (i == MAX_NUM_INSTANCE) {
        *ppInst = 0;
        return JPG_RET_FAILURE;
    }

    pJpgInst->inUse = TRUE;
    handleSize = sizeof(JpgDecInfo);
    if (handleSize < sizeof(JpgEncInfo)) {
        handleSize = sizeof(JpgEncInfo);
    }
    if ((pJpgInst->JpgInfo=(void*)malloc(handleSize)) == NULL) {
        return JPG_RET_INSUFFICIENT_RESOURCE;
    }

    memset((void*)pJpgInst->JpgInfo, 0x00, sizeof(handleSize));
    *ppInst = pJpgInst;
    jip->jpu_instance_num++;

    if (jdi_open_instance(pJpgInst->instIndex) < 0) {
        return JPG_RET_FAILURE;
    }

    return JPG_RET_SUCCESS;
}

void FreeJpgInstance(JpgInst * pJpgInst)
{
    jpu_instance_pool_t *jip;

    jip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!jip)
        return;

    pJpgInst->inUse = 0;
    jip->jpu_instance_num--;

    jdi_close_instance(pJpgInst->instIndex);

    free(pJpgInst->JpgInfo);
    pJpgInst->JpgInfo = NULL;
}

JpgRet CheckJpgInstValidity(JpgInst * pci)
{
    int i;
    jpu_instance_pool_t *jip;

    jip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!jip)
        return JPG_RET_FAILURE;

    for (i = 0; i < MAX_NUM_INSTANCE; ++i) {
        if ((JpgInst *)jip->jpgInstPool[i] == pci)
            return JPG_RET_SUCCESS;
    }
    return JPG_RET_INVALID_HANDLE;
}



/******************************************************************************
API Subroutines
******************************************************************************/


JpgRet CheckJpgDecOpenParam(JpgDecOpenParam * pop)
{
    if (pop == 0) {
        return JPG_RET_INVALID_PARAM;
    }
    if (pop->bitstreamBuffer % 8) {
        return JPG_RET_INVALID_PARAM;
    }
    if (pop->bitstreamBufferSize % 1024 || pop->bitstreamBufferSize < 1024) {
        return JPG_RET_INVALID_PARAM;
    }

    if (pop->chromaInterleave != CBCR_SEPARATED &&
        pop->chromaInterleave != CBCR_INTERLEAVE &&
        pop->chromaInterleave != CRCB_INTERLEAVE) {
            return JPG_RET_INVALID_PARAM;
    }

    if (pop->rotation != 0 && pop->rotation != 90 && pop->rotation != 180 && pop->rotation != 270) {
        return JPG_RET_INVALID_PARAM;
    }
    if ((Uint32)pop->mirror > (Uint32)MIRDIR_HOR_VER) {
        return JPG_RET_INVALID_PARAM;
    }

    if (pop->outputFormat == FORMAT_400 || pop->outputFormat == FORMAT_440) {
        return JPG_RET_INVALID_PARAM;
    }

    if (pop->packedFormat > PACKED_FORMAT_444) {
        return JPG_RET_INVALID_PARAM;
    }
    if (pop->packedFormat != PACKED_FORMAT_NONE) {
        if (pop->chromaInterleave != CBCR_SEPARATED) {
            return JPG_RET_INVALID_PARAM;
        }
    }

    return JPG_RET_SUCCESS;
}


int JpgDecHuffTabSetUp(JpgDecInfo *jpg, int instRegIndex)
{
    int i, j;
    int HuffData;	// 16BITS
    int HuffLength;
    int temp;

    // MIN Tables
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x003);

    //DC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[0][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }

    //DC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[2][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }

    //AC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[1][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }

    //AC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[3][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }
    // MAX Tables
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x403);
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_ADDR_REG, 0x440);

    //DC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[0][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }
    //DC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[2][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }
    //AC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[1][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }
    //AC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[3][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }

    // PTR Tables
    JpuWriteInstReg (instRegIndex, MJPEG_HUFF_CTRL_REG, 0x803);
    JpuWriteInstReg (instRegIndex, MJPEG_HUFF_ADDR_REG, 0x880);


    //DC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[0][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    //DC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[2][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    //AC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[1][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    //AC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[3][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }

    // VAL Tables
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0xC03);

    // VAL DC Luma
    HuffLength = 0;
    for(i=0; i<12; i++)
        HuffLength += jpg->huffBits[0][i];

    if (HuffLength > 256)
        return 0;

    for (i=0; i<HuffLength; i++) {	// 8-bit, 12 row, 1 category (DC Luma)
        HuffData = jpg->huffVal[0][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }

    for (i=0; i<12-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL DC Chroma
    HuffLength = 0;
    for(i=0; i<12; i++)
        HuffLength += jpg->huffBits[2][i];

    if (HuffLength > 256)
        return 0;
    for (i=0; i<HuffLength; i++) {	// 8-bit, 12 row, 1 category (DC Chroma)
        HuffData = jpg->huffVal[2][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<12-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL AC Luma
    HuffLength = 0;
    for(i=0; i<162; i++)
        HuffLength += jpg->huffBits[1][i];

    if (HuffLength > 256)
        return 0;

    for (i=0; i<HuffLength; i++) {	// 8-bit, 162 row, 1 category (AC Luma)
        HuffData = jpg->huffVal[1][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<162-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL AC Chroma
    HuffLength = 0;
    for(i=0; i<162; i++)
        HuffLength += jpg->huffBits[3][i];

    if (HuffLength > 256)
        return 0;

    for (i=0; i<HuffLength; i++) {	// 8-bit, 162 row, 1 category (AC Chroma)
        HuffData = jpg->huffVal[3][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }

    for (i=0; i<162-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // end SerPeriHuffTab
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x000);

    return 1;
}

int JpgDecHuffTabSetUp_12b(JpgDecInfo *jpg, int instRegIndex)
{
    int i, j;
    int HuffData;	// 16BITS
    int HuffLength;
    int temp;

    // MIN Tables
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x003);

    //DC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[0][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }
    //DC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[2][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }
    //AC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[1][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }
    //AC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[3][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }
    //DC EX1
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[4][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }

    //AC EX1
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMin[5][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
    }

    // MAX Tables
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x403);
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_ADDR_REG, 0x480);

    //DC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[0][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }
    //DC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[2][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }
    //AC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[1][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }
    //AC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[3][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }
    //DC EX1
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[4][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }

    //AC EX1
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffMax[5][j];
        temp = (HuffData & 0x8000) >> 15;
        temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
    }

    // PTR Tables
    JpuWriteInstReg (instRegIndex, MJPEG_HUFF_CTRL_REG, 0x803);
    JpuWriteInstReg (instRegIndex, MJPEG_HUFF_ADDR_REG, 0x900);

    //DC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[0][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    //DC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[2][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    //AC Luma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[1][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    //AC Chroma
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[3][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    //DC EX1
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[4][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }

    //AC EX1
    for(j=0; j<16; j++)
    {
        HuffData = jpg->huffPtr[5][j];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }

    // VAL Tables
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0xC03);

    // VAL DC Luma
    HuffLength = 0;
    for(i=0; i<16; i++) {
        HuffLength += jpg->huffBits[0][i];
    }

	if (HuffLength > 256) {
        return 0;
    }

    for (i=0; i<HuffLength; i++) {	// 8-bit, 12 row, 1 category (DC Luma)
        HuffData = jpg->huffVal[0][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<16-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL DC Chroma
    HuffLength = 0;
    for(i=0; i<16; i++) {
        HuffLength += jpg->huffBits[2][i];
    }

	if (HuffLength > 256) {
        return 0;
    }

    for (i=0; i<HuffLength; i++) {	// 8-bit, 12 row, 1 category (DC Chroma)
        HuffData = jpg->huffVal[2][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<16-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL AC Luma
    HuffLength = 0;
    for(i=0; i<256; i++) {
        HuffLength += jpg->huffBits[1][i];
    }

	if (HuffLength > 256) {
        return 0;
    }

    for (i=0; i<HuffLength; i++) {	// 8-bit, 162 row, 1 category (AC Luma)
        HuffData = jpg->huffVal[1][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<256-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL AC Chroma
    HuffLength = 0;
    for(i=0; i<256; i++) {
        HuffLength += jpg->huffBits[3][i];
    }

	if (HuffLength > 256) {
        return 0;
    }

    for (i=0; i<HuffLength; i++) {	// 8-bit, 162 row, 1 category (AC Chroma)
        HuffData = jpg->huffVal[3][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<256-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL DC EX1
    HuffLength = 0;
    for(i=0; i<16; i++) {
        HuffLength += jpg->huffBits[4][i];
    }

	if (HuffLength > 256) {
        return 0;
    }

    for (i=0; i<HuffLength; i++) {	// 8-bit, 12 row, 1 category (DC Luma)
        HuffData = jpg->huffVal[4][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<16-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // VAL AC EX1
    HuffLength = 0;
    for(i=0; i<256; i++) {
        HuffLength += jpg->huffBits[5][i];
    }

	if (HuffLength > 256) {
        return 0;
    }

    for (i=0; i<HuffLength; i++) {	// 8-bit, 162 row, 1 category (AC Luma)
        HuffData = jpg->huffVal[5][i];
        temp = (HuffData & 0x80) >> 7;
        temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
        JpuWriteInstReg (instRegIndex, MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
    }
    for (i=0; i<256-HuffLength; i++) {
        JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
    }

    // end SerPeriHuffTab
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x000);

    return 1;
}

int JpgDecQMatTabSetUp(JpgDecInfo *jpg, int instRegIndex)
{
    int i;
    int table;
    int val;

    // SetPeriQMatTab
    // Comp 0
    JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, 0x03);
    table = jpg->cInfoTab[0][3];

	if (table >= 4)
        return 0;

    for (i=0; i<64; i++) {
        val = jpg->qMatTab[table][i];
        JpuWriteInstReg(instRegIndex, MJPEG_QMAT_DATA_REG, val);
    }
    JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, 0x00);

    // Comp 1
    JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, 0x43);
    table = jpg->cInfoTab[1][3];
	if (table >= 4)
        return 0;

    for (i=0; i<64; i++) {
        val = jpg->qMatTab[table][i];
        JpuWriteInstReg(instRegIndex, MJPEG_QMAT_DATA_REG, val);
    }
    JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, 0x00);

    // Comp 2
    JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, 0x83);
    table = jpg->cInfoTab[2][3];
	if (table >= 4)
        return 0;

    for (i=0; i<64; i++) {
        val = jpg->qMatTab[table][i];
        JpuWriteInstReg(instRegIndex, MJPEG_QMAT_DATA_REG, val);
    }
    JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, 0x00);

    return 1;
}

void JpgDecGramSetup(JpgDecInfo * jpg, int instRegIndex)
{
    int dExtBitBufCurPos;
    int dExtBitBufBaseAddr;
    int dMibStatus;

    dMibStatus			= 1;
    dExtBitBufCurPos	= jpg->pagePtr;
    dExtBitBufBaseAddr	= jpg->streamBufStartAddr;

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_CUR_POS_REG, dExtBitBufCurPos);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_EXT_ADDR_REG, dExtBitBufBaseAddr + (dExtBitBufCurPos << 8));
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_INT_ADDR_REG, (dExtBitBufCurPos & 1) << 6);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_DATA_CNT_REG, JPU_GBU_SIZE / 4);	// 64 * 4 byte == 32 * 8 byte
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_COMMAND_REG, (jpg->streamEndian << 1) | 0);

    while (dMibStatus == 1) {
        dMibStatus = JpuReadInstReg(instRegIndex, MJPEG_BBC_BUSY_REG);
    }

    dMibStatus			= 1;
    dExtBitBufCurPos	= dExtBitBufCurPos + 1;

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_CUR_POS_REG, dExtBitBufCurPos);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_EXT_ADDR_REG, dExtBitBufBaseAddr + (dExtBitBufCurPos << 8));
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_INT_ADDR_REG, (dExtBitBufCurPos & 1) << 6);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_DATA_CNT_REG, JPU_GBU_SIZE / 4);	// 64 * 4 byte == 32 * 8 byte
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_COMMAND_REG, (jpg->streamEndian << 1) | 0);

    while (dMibStatus == 1) {
        dMibStatus = JpuReadInstReg(instRegIndex, MJPEG_BBC_BUSY_REG);
    }

    dExtBitBufCurPos	= dExtBitBufCurPos + 1;

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_CUR_POS_REG, dExtBitBufCurPos);	// next unit page pointer
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_CTRL_REG, (jpg->streamEndian<< 1) | 1);


    JpuWriteInstReg(instRegIndex, MJPEG_GBU_WPTR_REG, jpg->wordPtr);

    JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBSR_REG, 0);
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBER_REG, ((JPU_GBU_SIZE / 4) * 2) - 1);

    if (jpg->pagePtr & 1) {
        JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBIR_REG, 0);
        JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBHR_REG, 0);
    }
    else {
        JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBIR_REG, JPU_GBU_SIZE / 4);	// 64 * 4 byte == 32 * 8 byte
        JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBHR_REG, JPU_GBU_SIZE / 4);	// 64 * 4 byte == 32 * 8 byte
    }
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_CTRL_REG, 4);
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_FF_RPTR_REG, jpg->bitPtr);
}

enum {
    SAMPLE_420 = 0xA,
    SAMPLE_H422 = 0x9,
    SAMPLE_V422 = 0x6,
    SAMPLE_444 = 0x5,
    SAMPLE_400 = 0x1
};

const BYTE cDefHuffBits[8][16] =
{
    {	// DC index 0 (Luminance DC)
        0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
    ,
    {	// AC index 0 (Luminance AC)
        0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
            0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D
    }
    ,
    {	// DC index 1 (Chrominance DC)
        0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    }
    ,
    {	// AC index 1 (Chrominance AC)
        0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
            0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77
    }
};

const BYTE cDefHuffVal[8][162] =
{
    {	// DC index 0 (Luminance DC)
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B
    }
    ,
    {	// AC index 0 (Luminance AC)
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
            0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
            0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
            0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0,
            0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
            0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
            0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
            0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
            0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
            0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
            0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
            0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
            0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
            0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
            0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
            0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
            0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4,
            0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
            0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
            0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
            0xF9, 0xFA
    }
    ,
    {	// DC index 1 (Chrominance DC)
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B
    }
    ,
    {	// AC index 1 (Chrominance AC)
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
            0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
            0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
            0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0,
            0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
            0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26,
            0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38,
            0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
            0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
            0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
            0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
            0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
            0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96,
            0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
            0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
            0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3,
            0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2,
            0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
            0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
            0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
            0xF9, 0xFA
    }
};

const BYTE cDefHuffBits_ES[8][16] = {
    {	// DC index 0 (Luminance DC)
        0x00, 0x02, 0x03, 0x01, 0x00, 0x03, 0x01, 0x01,
        0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {	// AC index 0 (Luminance AC)
        0x00, 0x01, 0x04, 0x02, 0x02, 0x02, 0x01, 0x04,
        0x01, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0xEB
    },
    {	// DC index 1 (Chrominance DC)
        0x00, 0x02, 0x02, 0x02, 0x03, 0x01, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {	// AC index 1 (Chrominance AC)
        0x00, 0x01, 0x03, 0x02, 0x05, 0x01, 0x05, 0x05,
        0x03, 0x07, 0x04, 0x04, 0x03, 0x04, 0x06, 0xCB
    },
    {	// DC EX1 (Luminance DC)
        0x00, 0x02, 0x03, 0x01, 0x00, 0x03, 0x01, 0x01,
        0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {	// AC EX1 (Luminance AC)
        0x00, 0x01, 0x04, 0x02, 0x02, 0x02, 0x01, 0x04,
        0x01, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0xEB
    },
    {	// DC EX2 (Chrominance DC)
        0x00, 0x02, 0x02, 0x02, 0x03, 0x01, 0x01, 0x01,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {	// AC EX2 (Chrominance AC)
        0x00, 0x01, 0x03, 0x02, 0x05, 0x01, 0x05, 0x05,
        0x03, 0x07, 0x04, 0x04, 0x03, 0x04, 0x06, 0xCB
    }
};

const BYTE cDefHuffVal_ES[8][256] = {
    {	// DC index 0 (Luminance DC)
        0x08, 0x09, 0x06, 0x07, 0x0A, 0x05, 0x03, 0x04,
        0x0B, 0x02, 0x00, 0x01, 0x0C
    },
    {	// AC index 0 (Luminance AC)
        0x02,0x01,0x03,0x04,0x05,0x06,0x07,0x08,0x12,0x09,0x11,0x13,0x00,0x14,0x21,0x22,
        0x15,0x0a,0x23,0x31,0x16,0x32,0x17,0x24,0x33,0x41,0x18,0x25,0x42,0x51,0x0b,0x26,
        0x19,0x43,0x52,0x61,0x35,0x62,0x71,0x0c,0x0d,0x0e,0x0f,0x10,0x1a,0x1b,0x1c,0x1d,
        0x1e,0x1f,0x20,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x34,0x36,0x37,
        0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,
        0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,
        0x5d,0x5e,0x5f,0x60,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,
        0x6f,0x70,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
        0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
        0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
        0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
        0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
        0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
    },
    {	// DC index 1 (Chrominance DC)
        0x06, 0x07, 0x05, 0x08, 0x04, 0x09, 0x02, 0x03,
        0x0A, 0x01, 0x0B, 0x00, 0x0C
    },
    {	// AC index 1 (Chrominance AC)
        0x01,0x02,0x03,0x11,0x04,0x21,0x00,0x05,0x06,0x12,0x31,0x41,0x07,0x13,0x22,0x51,
        0x61,0x08,0x14,0x32,0x71,0x81,0x42,0x91,0xa1,0x09,0x15,0x23,0x52,0xb1,0xc1,0xf0,
        0x16,0x62,0xd1,0xe1,0x0a,0x24,0x72,0xf1,0x17,0x82,0x92,0x33,0x43,0x53,0xb2,0x0b,
        0x0c,0x18,0x35,0xa2,0xc2,0x0d,0x0e,0x0f,0x10,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
        0x20,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x34,0x36,0x37,
        0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,
        0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,
        0x5e,0x5f,0x60,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
        0x70,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,0x80,0x83,
        0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x90,0x93,0x94,0x95,
        0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,0xa0,0xa3,0xa4,0xa5,0xa6,0xa7,
        0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,
        0xba,0xbb,0xbc,0xbd,0xbe,0xbf,0xc0,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,
        0xcc,0xcd,0xce,0xcf,0xd0,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,
        0xdd,0xde,0xdf,0xe0,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,
        0xee,0xef,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
    },
    {	// DC EX1 (Luminance DC)
        0x08, 0x09, 0x06, 0x07, 0x0A, 0x05, 0x03, 0x04,
            0x0B, 0x02, 0x00, 0x01, 0x0C
    },
    {	// AC EX1 (Luminance AC)
        0x02,0x01,0x03,0x04,0x05,0x06,0x07,0x08,0x12,0x09,0x11,0x13,0x00,0x14,0x21,0x22,
        0x15,0x0a,0x23,0x31,0x16,0x32,0x17,0x24,0x33,0x41,0x18,0x25,0x42,0x51,0x0b,0x26,
        0x19,0x43,0x52,0x61,0x35,0x62,0x71,0x0c,0x0d,0x0e,0x0f,0x10,0x1a,0x1b,0x1c,0x1d,
        0x1e,0x1f,0x20,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x34,0x36,0x37,
        0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,
        0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,
        0x5d,0x5e,0x5f,0x60,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,
        0x6f,0x70,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
        0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
        0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
        0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
        0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
        0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
    },
    {	// DC EX2 (Chrominance DC)
        0x06, 0x07, 0x05, 0x08, 0x04, 0x09, 0x02, 0x03,
        0x0A, 0x01, 0x0B, 0x00, 0x0C
    },
    {	// AC EX2 (Chrominance AC)
        0x01,0x02,0x03,0x11,0x04,0x21,0x00,0x05,0x06,0x12,0x31,0x41,0x07,0x13,0x22,0x51,
        0x61,0x08,0x14,0x32,0x71,0x81,0x42,0x91,0xa1,0x09,0x15,0x23,0x52,0xb1,0xc1,0xf0,
        0x16,0x62,0xd1,0xe1,0x0a,0x24,0x72,0xf1,0x17,0x82,0x92,0x33,0x43,0x53,0xb2,0x0b,
        0x0c,0x18,0x35,0xa2,0xc2,0x0d,0x0e,0x0f,0x10,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
        0x20,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x34,0x36,0x37,
        0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,
        0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,
        0x5e,0x5f,0x60,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
        0x70,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,0x80,0x83,
        0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x90,0x93,0x94,0x95,
        0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,0xa0,0xa3,0xa4,0xa5,0xa6,0xa7,
        0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,
        0xba,0xbb,0xbc,0xbd,0xbe,0xbf,0xc0,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,
        0xcc,0xcd,0xce,0xcf,0xd0,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,
        0xdd,0xde,0xdf,0xe0,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,
        0xee,0xef,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
    }
};

enum {
    Marker			= 0xFF,
    FF_Marker		= 0x00,

    SOI_Marker		= 0xFFD8,			// Start of image
    EOI_Marker		= 0xFFD9,			// End of image

    JFIF_CODE		= 0xFFE0,			// Application
    EXIF_CODE		= 0xFFE1,

    DRI_Marker		= 0xFFDD,			// Define restart interval
    RST_Marker		= 0xD,				// 0xD0 ~0xD7

    DQT_Marker		= 0xFFDB,			// Define quantization table(s)
    DHT_Marker		= 0xFFC4,			// Define Huffman table(s)

    SOF_Marker		= 0xFFC0,			// Start of frame : Baseline DCT
    SOS_Marker		= 0xFFDA,			// Start of scan
    SOF_Marker_ES	= 0xFFC1,			// Start of frame : Extended Sequential
};


int check_start_code(JpgDecInfo *jpg)
{
    if (show_bits(&jpg->gbc, 8) == 0xFF)
        return 1;
    else
        return 0;
}


int find_start_code(JpgDecInfo *jpg)
{
    int word;

    for(;;)
    {
        if (get_bits_left(&jpg->gbc) <= 16)
        {
            ////printf("hit end of stream\n");
            return 0;
        }

        word = show_bits(&jpg->gbc, 16);
        if ((word > 0xFF00) && (word < 0xFFFF))
            break;


        get_bits(&jpg->gbc, 8);
    }

    return word;
}

int find_start_soi_code(JpgDecInfo *jpg)
{
    int word;

    for(;;)
    {
        if (get_bits_left(&jpg->gbc) <= 16)
        {
            ////printf("hit end of stream\n");
            return 0;
        }

        word = show_bits(&jpg->gbc, 16);
        if ((word > 0xFF00) && (word < 0xFFFF))
        {
            if (word != SOI_Marker)
                get_bits(&jpg->gbc, 8);
            break;
        }


        get_bits(&jpg->gbc, 8);
    }

    return word;
}



int decode_app_header(JpgDecInfo *jpg)
{
    int length;

    if (get_bits_left(&jpg->gbc) < 16)
        return 0;
    length = get_bits(&jpg->gbc, 16);
    length -= 2;

    while(length-- > 0)
    {
        if (get_bits_left(&jpg->gbc) < 8)
            return 0;
        get_bits(&jpg->gbc, 8);
    }


    return 1;
}


int decode_dri_header(JpgDecInfo *jpg)
{
    //Length, Lr
    if (get_bits_left(&jpg->gbc) < 16*2)
        return 0;
    get_bits(&jpg->gbc, 16);

    jpg->rstIntval = get_bits(&jpg->gbc, 16);


    return 1;
}

int decode_dqt_header(JpgDecInfo *jpg)
{
    int Pq;
    int Tq;
    int i;
    int tmp;
    if (get_bits_left(&jpg->gbc) < 16)
        return 0;

    // Lq, Length of DQT
    get_bits(&jpg->gbc, 16);

    do {

        if (get_bits_left(&jpg->gbc) < 4+4+8*64)
            return 0;

        // Pq, Quantization Precision
        tmp = get_bits(&jpg->gbc, 8);
        // Tq, Quantization table destination identifier
        Pq = (tmp>>4) & 0xf;
        Tq = tmp&0xf;

        if (Tq == 0)
            jpg->q_prec0 = Pq;
        else if (Tq == 1)
            jpg->q_prec1 = Pq;
        else if (Tq == 2)
            jpg->q_prec2 = Pq;
        else if (Tq == 3)
            jpg->q_prec3 = Pq;
		else
			return 0;

        for (i=0; i<64; i++){
            if (Pq)
                jpg->qMatTab[Tq][i] = (short)get_bits(&jpg->gbc, 16) & 0xffff;
            else
                jpg->qMatTab[Tq][i] = (BYTE)get_bits(&jpg->gbc, 8) & 0x00ff;
        }
    } while(!check_start_code(jpg));

    return 1;
}

int decode_dth_header(JpgDecInfo *jpg)
{
    int Tc;
    int Th;
    int ThTc;
    int bitCnt;
    int i;
    int tmp;
    // Length, Lh
    if (get_bits_left(&jpg->gbc) < 16)
        return 0;

    get_bits(&jpg->gbc, 16);

    do {

        if (get_bits_left(&jpg->gbc) < 8 + 8*16)
            return 0;

        // Table class - DC, AC
        tmp = get_bits(&jpg->gbc, 8);
        // Table destination identifier
        Tc = (tmp>>4) & 0xf;
        Th = tmp&0xf;

        // DC_ID0 (0x00) -> 0
        // AC_ID0 (0x10) -> 1
        // DC_ID1 (0x01) -> 2
        // AC_ID1 (0x11) -> 3
        // DC_EX1 (0x02) -> 4
        // AC_EX1 (0x12) -> 5
        // DC_EX2 (0x03) -> 6
        // AC_EX2 (0x13) -> 7

        //Since only DC_ID0, DC_ID1, if th is 2 or 3, always set userHuffTab
        /* According to ITU-T.81 Table B.5, maximum Tc value is 1 and maximum Th value is 3.*/
        if (Tc > 1 || Th > 3) {
            return 0;
        }

        ThTc = ((Th&3)<<1) | (Tc&1);
        if (jpg->numHuffmanTable >= THTC_LIST_CNT)
            return 0;

        jpg->thtc[jpg->numHuffmanTable++] = ThTc;
        // Get Huff Bits list
        bitCnt = 0;
        for (i=0; i<16; i++) {
            jpg->huffBits[ThTc][i] = (BYTE)get_bits(&jpg->gbc, 8);
            bitCnt += jpg->huffBits[ThTc][i];
        }

		if (bitCnt > 256)
			return 0;

        if (get_bits_left(&jpg->gbc) <  8*bitCnt)
            return 0;

        // Get Huff Val list
        for (i=0; i<bitCnt; i++) {
            jpg->huffVal[ThTc][i] = (BYTE)get_bits(&jpg->gbc, 8);
        }
    } while(!check_start_code(jpg));


    return 1;
}
int decode_sof_header(JpgDecInfo *jpg)
{
    int samplePrecision;
    int sampleFactor;
    int i;
    int Tqi;
    BYTE compID;
    int hSampFact[3] = {0,};
    int vSampFact[3] = {0,};
    int picX, picY;
    int numComp;
    int tmp;

    if (get_bits_left(&jpg->gbc) < 16+8+16+16+8)
        return 0;
    // LF, Length of SOF
    get_bits(&jpg->gbc, 16);

    // Sample Precision: Baseline(8), P
    samplePrecision = get_bits(&jpg->gbc, 8);
    jpg->bitDepth   = samplePrecision;
    jpg->jpg12bit   = (samplePrecision == 12) ? 1 : 0;

    picY = get_bits(&jpg->gbc, 16);
    if (picY > MAX_MJPG_PIC_WIDTH)
    {
        //printf("Picture Vertical Size limits Maximum size\n");
        return 0;
    }

    picX = get_bits(&jpg->gbc, 16);
    if (picX > MAX_MJPG_PIC_HEIGHT)
    {
        //printf("Picture Horizontal Size limits Maximum size\n");
        return 0;
    }

    //Number of Components in Frame: Nf
    numComp = get_bits(&jpg->gbc, 8);
    if (numComp > 3)
    {
        //printf("Picture Horizontal Size limits Maximum size\n");
        return 0;
    }

    if (get_bits_left(&jpg->gbc) < numComp*(8+4+4+8))
        return 0;
    for (i=0; i<numComp; i++)
    {
        // Component ID, Ci 0 ~ 255
        compID = (BYTE)get_bits(&jpg->gbc, 8);
        tmp = get_bits(&jpg->gbc, 8);
        // Horizontal Sampling Factor, Hi
        hSampFact[i] = (tmp>>4) & 0xf;
        // Vertical Sampling Factor, Vi
        vSampFact[i] = tmp&0xf;
        // Quantization Table Selector, Tqi
        Tqi = get_bits(&jpg->gbc, 8);

        jpg->cInfoTab[i][0] = compID;
        jpg->cInfoTab[i][1] = (BYTE)hSampFact[i];
        jpg->cInfoTab[i][2] = (BYTE)vSampFact[i];
        jpg->cInfoTab[i][3] = (BYTE)Tqi;
    }

    //if ( hSampFact[0]>2 || vSampFact[0]>2 || ( numComp == 3 && ( hSampFact[1]!=1 || hSampFact[2]!=1 || vSampFact[1]!=1 || vSampFact[2]!=1) ) )
    //printf("Not Supported Sampling Factor\n");

    if (numComp == 1)
        sampleFactor = SAMPLE_400;
    else
        sampleFactor = ((hSampFact[0]&3)<<2) | (vSampFact[0]&3);

    switch(sampleFactor) {
    case SAMPLE_420:
        jpg->format = FORMAT_420;
        break;
    case SAMPLE_H422:
        jpg->format = FORMAT_422;
        break;
    case SAMPLE_V422:
        jpg->format = FORMAT_440;
        break;
    case SAMPLE_444:
        jpg->format = FORMAT_444;
        break;
    default:	// 4:0:0
        jpg->format = FORMAT_400;
        break;
    }

    jpg->picWidth = picX;
    jpg->picHeight = picY;

    return 1;
}

int decode_sos_header(JpgDecInfo *jpg)
{
    int i, j;
    int len;
    int numComp;
    int compID;
    int ecsPtr;
    int ss, se, ah, al;
    int dcHufTblIdx[3] = {0,};
    int acHufTblIdx[3] = {0,};
    int tmp;

    if (get_bits_left(&jpg->gbc) < 8)
        return 0;
    // Length, Ls
    len = get_bits(&jpg->gbc, 16);

    jpg->ecsPtr = get_bits_count(&jpg->gbc)/8 + len - 2 ;

    ecsPtr = jpg->ecsPtr+jpg->frameOffset;

    //printf("ecsPtr=0x%x frameOffset=0x%x, ecsOffset=0x%x, wrPtr=0x%x, rdPtr0x%x\n", jpg->ecsPtr, jpg->frameOffset, ecsPtr, jpg->streamWrPtr, jpg->streamRdPtr);

    jpg->pagePtr = ecsPtr >> 8;									 //page unit  ecsPtr/256;
    jpg->wordPtr = (ecsPtr & 0xF0) >> 2;	                     // word unit ((ecsPtr % 256) & 0xF0) / 4;

    if (jpg->pagePtr & 1)
        jpg->wordPtr += 64;
    if (jpg->wordPtr & 1)
        jpg->wordPtr -= 1; // to make even.

    jpg->bitPtr = (ecsPtr & 0xF) << 3;	                        // bit unit (ecsPtr & 0xF) * 8;

    if (get_bits_left(&jpg->gbc) < 8)
        return 0;
    //Number of Components in Scan: Ns
    numComp = get_bits(&jpg->gbc, 8);
	if (numComp > 3)
		return 0;

    if (get_bits_left(&jpg->gbc) < numComp*(8+4+4))
        return 0;
    for (i=0; i<numComp; i++) {
        // Component ID, Csj 0 ~ 255
        compID = get_bits(&jpg->gbc, 8);
        tmp = get_bits(&jpg->gbc, 8);
        // dc entropy coding table selector, Tdj
        dcHufTblIdx[i] = (tmp>>4) & 0xf;
        // ac entropy coding table selector, Taj
        acHufTblIdx[i] = tmp&0xf;


        for (j=0; j<numComp; j++)
        {
            if (compID == jpg->cInfoTab[j][0])
            {
                jpg->cInfoTab[j][4] = (BYTE)dcHufTblIdx[i];
                jpg->cInfoTab[j][5] = (BYTE)acHufTblIdx[i];
            }
        }
    }

    if (get_bits_left(&jpg->gbc) < 8+8+4+4)
        return 0;
    // Ss 0
    ss = get_bits(&jpg->gbc, 8);
    // Se 3F
    se = get_bits(&jpg->gbc, 8);
    tmp = get_bits(&jpg->gbc, 8);
    // Ah 0
    ah = (i>>4) & 0xf;
    // Al 0
    al = tmp&0xf;

    if ((ss != 0) || (se != 0x3F) || (ah != 0) || (al != 0))
    {
        //printf("The Jpeg Image must be another profile\n");
        return 0;
    }

    return 1;
}

static void genDecHuffTab(JpgDecInfo *jpg, int tabNum)
{
    unsigned char *huffPtr, *huffBits;
    Uint32 *huffMax, *huffMin;

    int ptrCnt =0;
    int huffCode = 0;
    int zeroFlag = 0;
    int dataFlag = 0;
    int i;

    huffBits	= jpg->huffBits[tabNum];
    huffPtr		= jpg->huffPtr[tabNum];
    huffMax		= jpg->huffMax[tabNum];
    huffMin		= jpg->huffMin[tabNum];

    for (i=0; i<16; i++)
    {
        if (huffBits[i]) // if there is bit cnt value
        {
            huffPtr[i] = (BYTE)ptrCnt;
            ptrCnt += huffBits[i];
            huffMin[i] = huffCode;
            huffMax[i] = huffCode + (huffBits[i] - 1);
            dataFlag = 1;
            zeroFlag = 0;
        }
        else
        {
            huffPtr[i] = 0xFF;
            huffMin[i] = 0xFFFF;
            huffMax[i] = 0xFFFF;
            zeroFlag = 1;
        }

        if (dataFlag == 1)
        {
            if (zeroFlag == 1)
                huffCode <<= 1;
            else
                huffCode = (huffMax[i] + 1) << 1;
        }
    }

}





int JpuGbuInit(vpu_getbit_context_t *ctx, BYTE *buffer, int size)
{

    ctx->buffer = buffer;
    ctx->index = 0;
    ctx->size = size/8;

    return 1;
}

int JpuGbuGetUsedBitCount(vpu_getbit_context_t *ctx)
{
    return ctx->index*8;
}

int JpuGbuGetLeftBitCount(vpu_getbit_context_t *ctx)
{
    return (ctx->size*8) - JpuGbuGetUsedBitCount(ctx);
}

unsigned int JpuGbuGetBit(vpu_getbit_context_t *ctx, int bit_num)
{
    BYTE *p;
    unsigned int b = 0x0;

    if (bit_num > JpuGbuGetLeftBitCount(ctx))
        return (unsigned int)-1;

    p = ctx->buffer + ctx->index;

    if (bit_num == 8)
    {
        b = *p;
        ctx->index++;
    }
    else if(bit_num == 16)
    {
        b = *p++<<8;
        b |= *p++;
        ctx->index += 2;
    }
    else if(bit_num == 32)
    {
        b = *p++<<24;
        b |= (*p++<<16);
        b |= (*p++<<8);
        b |= (*p++<<0);
        ctx->index += 4;
    }


    return b;
}

unsigned int JpuGguShowBit(vpu_getbit_context_t *ctx, int bit_num)
{
    BYTE *p;
    unsigned int b = 0x0;

    if (bit_num > JpuGbuGetLeftBitCount(ctx))
        return (unsigned int)-1;

    p = ctx->buffer + ctx->index;

    if (bit_num == 8)
    {
        b = *p;
    }
    else if(bit_num == 16)
    {
        b = *p++<<8;
        b |= *p++;
    }
    else if(bit_num == 32)
    {
        b = *p++<<24;
        b |= (*p++<<16);
        b |= (*p++<<8);
        b |= (*p++<<0);
    }

    return b;
}



static int wraparound_bistream_data(JpgDecInfo *jpg, int return_type)
{
    BYTE *dst;
    BYTE *src;
    BYTE *data;
    int data_size;
    int src_size;
    int dst_size;

    data_size = jpg->streamWrPtr - jpg->streamBufStartAddr;
    data = (BYTE *)malloc(data_size);

    if (data)
        JpuReadMem(jpg->streamBufStartAddr, data, data_size, jpg->streamEndian);

    src_size = jpg->streamBufSize - jpg->frameOffset;
    src = (BYTE *)malloc(src_size);
    dst_size = ((src_size+(JPU_GBU_SIZE-1))&~(JPU_GBU_SIZE-1));
    dst = (BYTE *)malloc(dst_size);
    if (dst && src)
    {
        memset(dst, 0x00, dst_size);
        JpuReadMem(jpg->streamBufStartAddr+jpg->frameOffset, src, src_size, jpg->streamEndian);
        memcpy(dst+(dst_size-src_size), src, src_size);
        JpuWriteMem(jpg->streamBufStartAddr, dst, dst_size, jpg->streamEndian);
        if (data)
            JpuWriteMem(jpg->streamBufStartAddr+dst_size, data, data_size, jpg->streamEndian);
    }

    if(src)
        free(src);
    if(dst)
        free(dst);
    if (data)
        free(data);

    if (return_type == -2) {// header wraparound
        jpg->streamWrPtr = jpg->streamBufStartAddr+dst_size+data_size;
        jpg->consumeByte = 0;
        return -2;
    }
    else if(return_type == -1){	// ecsPtr wraparound
        jpg->streamWrPtr = jpg->streamBufStartAddr+dst_size+data_size;
        jpg->frameOffset = 0;
        return -1;
    }

    return 0;
}


static void CheckUserHuffmanTable(JpgDecInfo* jpg)
{
    Uint32 ThTc, i, bitCnt;

    for (ThTc=0; ThTc<THTC_LIST_CNT; ThTc++) {
        if (jpg->thtc[ThTc] == -1) break;
        bitCnt = 0;
        for (i=0; i<16; i++) {
            bitCnt += jpg->huffBits[ThTc][i];
            if (jpg->jpg12bit) {
                jpg->userHuffTab = (cDefHuffBits_ES[ThTc][i] != jpg->huffBits[ThTc][i]);
            }
            else {
                jpg->userHuffTab = (cDefHuffBits[ThTc][i] != jpg->huffBits[ThTc][i]);
            }
            if (jpg->userHuffTab == TRUE) return;
        }

		if (bitCnt > 256)
			return;

        // Get Huff Val list
        for (i=0; i<bitCnt; i++) {
            if (jpg->jpg12bit) {
                jpg->userHuffTab = (cDefHuffVal_ES[ThTc][i] != jpg->huffVal[ThTc][i]);
            }
            else {
                jpg->userHuffTab = (cDefHuffVal[ThTc][i] != jpg->huffVal[ThTc][i]);
            }
            if (jpg->userHuffTab == TRUE) return;
        }
    }
}

int JpegDecodeHeader(JpgDecInfo *jpg)
{
    unsigned int code;
    int ret;
    int i;
    int temp;
    int wrOffset;
    BYTE *b = jpg->pBitStream+jpg->frameOffset;
    int size;
    BOOL yuv400_4Blocks = TRUE;   /* process 4block at a time for improving performance */

    for (i=0; i<THTC_LIST_CNT; i++) {
        jpg->thtc[i] = -1;
    }
    jpg->numHuffmanTable = 0;

    ret = 1;
    if (jpg->streamWrPtr == jpg->streamBufStartAddr)
    {
        size = jpg->streamBufSize-jpg->frameOffset;
        wrOffset = jpg->streamBufSize;
    }
    else
    {
        if (jpg->frameOffset >= (int)(jpg->streamWrPtr-jpg->streamBufStartAddr))
            size = jpg->streamBufSize - jpg->frameOffset;
        else
            size = (jpg->streamWrPtr-jpg->streamBufStartAddr)-jpg->frameOffset;
        wrOffset = (jpg->streamWrPtr-jpg->streamBufStartAddr);
    }

    if (!b || !size) {
        ret = -1;
        goto DONE_DEC_HEADER;
    }

    // find start code of next frame
    if (!jpg->ecsPtr)
    {
        int nextOffset = 0;
        int soiOffset = 0;

        if (jpg->consumeByte != 0)	// meaning is frameIdx > 0
        {
            nextOffset = jpg->consumeByte;
            if (nextOffset <= 0)
                nextOffset = 2;	//in order to consume start code.
        }

        //consume to find the start code of next frame.
        b += nextOffset;
        size -= nextOffset;

        if (size < 0)
        {
            jpg->consumeByte -= (b - (jpg->pBitStream+jpg->streamBufSize));
            if (jpg->consumeByte < 0) {
                ret = 0;
                goto DONE_DEC_HEADER;
            }
            ret = -1;
            goto DONE_DEC_HEADER;
        }

        init_get_bits(&jpg->gbc, b, size*8);
        for (;;)
        {
            code = find_start_soi_code(jpg);
            if (code == 0)
            {
                ret = -1;
                goto DONE_DEC_HEADER;
            }

            if (code == SOI_Marker)
                break;

        }

        soiOffset = get_bits_count(&jpg->gbc)/8;

        b += soiOffset;
        size -= soiOffset;
        jpg->frameOffset += (soiOffset+ nextOffset);
    }

    if (jpg->headerSize > 0 && (jpg->headerSize > (jpg->streamBufSize - jpg->frameOffset - JPU_GBU_SIZE))) { // if header size is smaller than room of stream end. copy the buffer to bistream start.
        return wraparound_bistream_data(jpg, -2);
    }


    init_get_bits(&jpg->gbc, b, size*8);
    // Initialize component information table
    for (i=0; i<4; i++)
    {
        jpg->cInfoTab[i][0] = 0;
        jpg->cInfoTab[i][1] = 0;
        jpg->cInfoTab[i][2] = 0;
        jpg->cInfoTab[i][3] = 0;
        jpg->cInfoTab[i][4] = 0;
        jpg->cInfoTab[i][5] = 0;
    }

    for (;;)
    {
        if (find_start_code(jpg) == 0)
        {
            ret = -1;
            goto DONE_DEC_HEADER;
        }

        code = get_bits(&jpg->gbc, 16); //getbit 2byte
        switch (code) {
        case SOI_Marker:
            break;
        case JFIF_CODE:
        case EXIF_CODE:
            if (!decode_app_header(jpg))
            {
                ret = -1;
                goto DONE_DEC_HEADER;
            }
            break;
        case DRI_Marker:
            if (!decode_dri_header(jpg))
            {
                ret = -1;
                goto DONE_DEC_HEADER;
            }
            break;
        case DQT_Marker:
            if (!decode_dqt_header(jpg))
            {
                ret = -1;
                goto DONE_DEC_HEADER;
            }
            break;
        case DHT_Marker:
            if (!decode_dth_header(jpg))
            {
                ret = -1;
                goto DONE_DEC_HEADER;
            }
            break;
        case SOF_Marker:
        case SOF_Marker_ES:
            if (!decode_sof_header(jpg))
            {
                ret = -1;
                goto DONE_DEC_HEADER;
            }
            break;
        case SOS_Marker:
            if (!decode_sos_header(jpg))
            {
                ret = -1;
                goto DONE_DEC_HEADER;
            }
            if (!jpg->headerSize)
                jpg->headerSize = jpg->ecsPtr;	// we assume header size of all frame is same for mjpeg case
            goto DONE_DEC_HEADER;
        case EOI_Marker:
            goto DONE_DEC_HEADER;
        default:
            switch (code&0xFFF0)
            {
            case 0xFFE0:	// 0xFFEX
            case 0xFFF0:	// 0xFFFX
                if (get_bits_left(&jpg->gbc) <=0 ) {
                    {
                        ret = -1;
                        goto DONE_DEC_HEADER;
                    }
                }
                else
                {
                    if (!decode_app_header(jpg))
                    {
                        ret = -1;
                        goto DONE_DEC_HEADER;
                    }
                    break;
                }
            default:
                //in case,  restart marker is founded.
                if( (code&0xFFF0) >= 0xFFD0 && (code&0xFFF0) <= 0xFFD7)
                    break;
                else
                    //printf("code = [%x]\n", code);
                    return	0;
            }
            break;
        }
    }

DONE_DEC_HEADER:

    if (ret == -1)
    {
        if (wrOffset < jpg->frameOffset)
            return -2;

        if (0 == get_bits_left(&jpg->gbc)) {
            return -2;
        }

        return -1;
    }

    if (!jpg->ecsPtr)
        return 0;

    if (wrOffset - (jpg->frameOffset+jpg->ecsPtr)  < JPU_GBU_SIZE &&
        jpg->streamEndflag == 0) {
            return -1;
    }

    // this bellow is workaround to avoid the case that JPU is run over without interrupt.
    if (jpg->streamBufSize - (jpg->frameOffset+jpg->ecsPtr) < JPU_GBU_SIZE)
        return wraparound_bistream_data(jpg, -1);

    CheckUserHuffmanTable(jpg);

    // Generate Huffman table information
    if (jpg->jpg12bit){
        for(i=0; i<8; i++)
            genDecHuffTab(jpg, i);
    }else{
        for(i=0; i<4; i++)
            genDecHuffTab(jpg, i);
    }

    // Q Idx
    temp =             jpg->cInfoTab[0][3];
    temp = temp << 1 | jpg->cInfoTab[1][3];
    temp = temp << 1 | jpg->cInfoTab[2][3];
    jpg->Qidx = temp;


    // Huff Idx[DC, AC]
    temp =             jpg->cInfoTab[0][4];

    if (jpg->jpg12bit == 0){
        temp = temp << 1 | jpg->cInfoTab[1][4];
        temp = temp << 1 | jpg->cInfoTab[2][4];
    }else{
        temp = temp << 2 | jpg->cInfoTab[1][4];
        temp = temp << 2 | jpg->cInfoTab[2][4];
    }
    jpg->huffDcIdx = temp;

    temp =             jpg->cInfoTab[0][5];
    if (jpg->jpg12bit == 0){
        temp = temp << 1 | jpg->cInfoTab[1][5];
        temp = temp << 1 | jpg->cInfoTab[2][5];
    }else{
        temp = temp << 2 | jpg->cInfoTab[1][5];
        temp = temp << 2 | jpg->cInfoTab[2][5];
    }
    jpg->huffAcIdx = temp;


    switch(jpg->format)
    {
    case FORMAT_420:
        jpg->mcuBlockNum = 6;
        jpg->compNum = 3;
        jpg->compInfo[1] = 5;
        jpg->compInfo[2] = 5;
        jpg->alignedWidth = ((jpg->picWidth+15)&~15);
        jpg->alignedHeight = ((jpg->picHeight+15)&~15);
        jpg->mcuWidth  = 16;
        jpg->mcuHeight = 16;
        break;
    case FORMAT_422:
        jpg->mcuBlockNum = 4;
        jpg->compNum = 3;
        jpg->compInfo[1] = 5;
        jpg->compInfo[2] = 5;
        jpg->alignedWidth = ((jpg->picWidth+15)&~15);
        jpg->alignedHeight = ((jpg->picHeight+7)&~7);
        jpg->mcuWidth  = 16;
        jpg->mcuHeight = 8;
        break;
    case FORMAT_440:
        jpg->mcuBlockNum = 4;
        jpg->compNum = 3;
        jpg->compInfo[1] = 5;
        jpg->compInfo[2] = 5;
        jpg->alignedWidth = ((jpg->picWidth+7)&~7);
        jpg->alignedHeight = ((jpg->picHeight+15)&~15);
        jpg->mcuWidth  = 8;
        jpg->mcuHeight = 16;
        break;
    case FORMAT_444:
        jpg->mcuBlockNum = 3;
        jpg->compNum = 3;
        jpg->compInfo[1] = 5;
        jpg->compInfo[2] = 5;
        jpg->alignedWidth = ((jpg->picWidth+7)&~7);
        jpg->alignedHeight = ((jpg->picHeight+7)&~7);
        jpg->mcuWidth  = 8;
        jpg->mcuHeight = 8;
        break;
    case FORMAT_400:
        jpg->compNum = 1;
        jpg->compInfo[1] = 0;
        jpg->compInfo[2] = 0;
        jpg->alignedWidth = ((jpg->picWidth+7)&~7);
        jpg->alignedHeight = ((jpg->picHeight+7)&~7);
        if (jpg->roiEnable == TRUE) {
            Uint32 offsetX = JPU_FLOOR(8, jpg->roiOffsetX);
            yuv400_4Blocks = (BOOL)((offsetX%32) == 0);
        }
        if (yuv400_4Blocks == TRUE) {
            jpg->mcuBlockNum = 4;
            jpg->mcuWidth    = 32;
            jpg->mcuHeight   = 8;
        }
        else {
            jpg->mcuBlockNum = 1;
            jpg->mcuWidth    = 8;
            jpg->mcuHeight   = 8;
        }
        break;
    default:
        return 0;
    }
    jpg->compInfo[0] = (jpg->mcuWidth >> 3) << 3 | (jpg->mcuHeight >> 3);

    jpg->busReqNum = (jpg->jpg12bit == FALSE) ? GetDec8bitBusReqNum(jpg->format, jpg->packedFormat) :
                                                GetDec12bitBusReqNum(jpg->format, jpg->packedFormat);

    return 1;
}

JpgRet CheckJpgEncOpenParam(JpgEncOpenParam * pop, JPUCap* cap)
{
    int picWidth;
    int picHeight;

    if (pop == 0) {
        return JPG_RET_INVALID_PARAM;
    }

    picWidth = pop->picWidth;
    picHeight = pop->picHeight;

    if (pop->bitstreamBuffer % 8) {
        return JPG_RET_INVALID_PARAM;
    }
    if ((pop->bitstreamBufferSize - (4096)) < 4096) {
        return JPG_RET_INVALID_PARAM;
    }

    if ((pop->bitstreamBufferSize - (4096)) % 4096) {
        return JPG_RET_INVALID_PARAM;
    }

    if (picWidth < 16 || picWidth > MAX_MJPG_PIC_WIDTH ) {
        return JPG_RET_INVALID_PARAM;
    }
    if (picHeight < 16 || picHeight > MAX_MJPG_PIC_HEIGHT ) {
        return JPG_RET_INVALID_PARAM;
    }
    if (pop->intrEnableBit == 0) {
        return JPG_RET_INVALID_PARAM;
    }

    if (pop->jpg12bit == TRUE && cap->support12bit == FALSE) {
        return JPG_RET_NOT_SUPPORT;
    }

    return JPG_RET_SUCCESS;
}


JpgRet CheckJpgEncParam(JpgEncHandle handle, JpgEncParam * param)
{
    JpgInst *pJpgInst;
    JpgEncInfo *pEncInfo;

    pJpgInst = handle;
    pEncInfo = &pJpgInst->JpgInfo->encInfo;

    if (param == 0) {
        return JPG_RET_INVALID_PARAM;
    }

    if (pEncInfo->packedFormat != PACKED_FORMAT_NONE) {
        if (pEncInfo->packedFormat == PACKED_FORMAT_444) {
            if (param->sourceFrame->stride < pEncInfo->picWidth*2) {
                return JPG_RET_INVALID_PARAM;
            }
        }
        if (pEncInfo->packedFormat == PACKED_FORMAT_444) {
            if (param->sourceFrame->stride < pEncInfo->picWidth*3) {
                return JPG_RET_INVALID_PARAM;
            }
        }
    }

    return JPG_RET_SUCCESS;
}



int JpgEncGenHuffTab(JpgEncInfo * pEncInfo, int tabNum)
{
    int p, i, l, lastp, si, maxsymbol;

    int huffsize[256] = {0,};
    int huffcode[256] = {0,};
    int code;

    BYTE *bitleng, *huffval;
    Uint32 *ehufco, *ehufsi;

    if (tabNum > 3)
        return 0;

    bitleng	= pEncInfo->pHuffBits[tabNum];
    huffval	= pEncInfo->pHuffVal[tabNum];
    ehufco	= pEncInfo->huffCode[tabNum];
    ehufsi	= pEncInfo->huffSize[tabNum];

    maxsymbol = tabNum & 1 ? 256 : 16;

    /* Figure C.1: make table of Huffman code length for each symbol */

    p = 0;
    for (l=1; l<=16; l++) {
        i = bitleng[l-1];
        if (i < 0 || p + i > maxsymbol)
            return 0;
        while (i--)
            huffsize[p++] = l;
    }
    lastp = p;

    /* Figure C.2: generate the codes themselves */
    /* We also validate that the counts represent a legal Huffman code tree. */

    code = 0;
    si = huffsize[0];
    p = 0;
    while(p < lastp){
        while (huffsize[p] == si) {
            huffcode[p++] = code;
            code++;
        }
        if (code >= (1 << si))
            return 0;
        code <<= 1;
        si++;
    }

    memset(ehufsi, 0, sizeof(int) * 256);
    memset(ehufco, 0, sizeof(int) * 256);

    /* Figure C.3: generate encoding tables */
    /* These are code and size indexed by symbol value */

    for(i=0; i<256; i++)
        ehufsi[i] = 0x00;

    for(i=0; i<256; i++)
        ehufco[i] = 0x00;

    for (p=0; p<lastp; p++) {
        i = huffval[p];
        if (i < 0 || i >= maxsymbol || ehufsi[i])
            return 0;
        ehufco[i] = huffcode[p];
        ehufsi[i] = huffsize[p];
    }

    return 1;
}

int JpgEncLoadHuffTab(JpgInst *pJpgInst, int instRegIndex)
{
    int i, j, t;
    int huffData;
    JpgEncInfo * pEncInfo;

    pEncInfo = &pJpgInst->JpgInfo->encInfo;


    for (i=0; i<4; i++)
        JpgEncGenHuffTab(pEncInfo, i);

    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x3);

    for (j=0; j<4; j++)
    {

        t = (j==0) ? AC_TABLE_INDEX0 : (j==1) ? AC_TABLE_INDEX1 : (j==2) ? DC_TABLE_INDEX0 : DC_TABLE_INDEX1;

        for (i=0; i<256; i++)
        {
            if ((t==DC_TABLE_INDEX0 || t==DC_TABLE_INDEX1) && (i>15))	// DC
                break;

            if ((pEncInfo->huffSize[t][i] == 0) && (pEncInfo->huffCode[t][i] == 0))
                huffData = 0;
            else
            {
                huffData =                    (pEncInfo->huffSize[t][i] - 1);	// Code length (1 ~ 16), 4-bit
                huffData = (huffData << 16) | (pEncInfo->huffCode[t][i]    );	// Code word, 16-bit
            }
            JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, huffData);
        }
    }
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x0);
    return 1;
}

int JpgEncLoadHuffTab_12b(JpgInst *pJpgInst, int instRegIndex)
{
    int i, j, t;
    int huffData;
    JpgEncInfo * pEncInfo;

    pEncInfo = &pJpgInst->JpgInfo->encInfo;

    for (i=0; i<8; i++)
        JpgEncGenHuffTab(pEncInfo, i);

    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x3);

    for (j=0; j<8; j++)
    {

        t = (j==0) ? AC_TABLE_INDEX0 : (j==1) ? AC_TABLE_INDEX1 : (j==2) ? DC_TABLE_INDEX0 : (j==3)? DC_TABLE_INDEX1 : (j==4) ? AC_TABLE_INDEX2 : (j==5) ? AC_TABLE_INDEX3 : (j==6) ? DC_TABLE_INDEX2 : DC_TABLE_INDEX3;

        for (i=0; i<256; i++)
        {
            //only support 3 huffman tables
            if (t==DC_TABLE_INDEX3 || t==AC_TABLE_INDEX3)
                break;

            if ((t==DC_TABLE_INDEX0 || t==DC_TABLE_INDEX1 || t==DC_TABLE_INDEX2 || t==DC_TABLE_INDEX3) && (i>15))	// DC
                break;

            if ((pEncInfo->huffSize[t][i] == 0) && (pEncInfo->huffCode[t][i] == 0))
                huffData = 0;
            else
            {
                huffData =                    (pEncInfo->huffSize[t][i] - 1);	// Code length (1 ~ 16), 4-bit
                huffData = (huffData << 16) | (pEncInfo->huffCode[t][i]    );	// Code word, 16-bit
            }
            JpuWriteInstReg(instRegIndex, MJPEG_HUFF_DATA_REG, huffData);
        }
    }
    JpuWriteInstReg(instRegIndex, MJPEG_HUFF_CTRL_REG, 0x0);
    return 1;
}

int JpgEncLoadQMatTab(JpgInst *pJpgInst, int instRegIndex)
{

#ifdef WIN32
    __int64 dividend = 0x80000;
    __int64 quotient;
    __int64 dividend_32b = 0x80000000;
#else
    long long int dividend = 0x80000;
    long long int quotient;
    long long int dividend_32b = 0x80000000;
#endif
    int quantID;
    int divisor;
    int comp;
    int i, t;
    int qprec = 0;
    JpgEncInfo * pEncInfo;

    pEncInfo = &pJpgInst->JpgInfo->encInfo;

    for (comp=0; comp<3; comp++) {
        quantID = pEncInfo->pCInfoTab[comp][3];
        if (quantID >= 4)
            return 0;
        t = (comp==0)? Q_COMPONENT0 :
            (comp==1)? Q_COMPONENT1 : Q_COMPONENT2;
        JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, 0x3 + t);

        if (pEncInfo->jpg12bit != 0){
            if (comp == 0)
                qprec = pEncInfo->q_prec0;
            else
                qprec = pEncInfo->q_prec1;
        }

        for (i=0; i<64; i++) {
            divisor = pEncInfo->pQMatTab[quantID][i];
            if (qprec)
                quotient= dividend_32b / divisor;
            else
                quotient= dividend / divisor;
            // enhace bit precision & rounding Q
            if (qprec){
                JpuWriteInstReg(instRegIndex, MJPEG_QMAT_DATA_REG, (int)(divisor & 0xFFFF));
                JpuWriteInstReg(instRegIndex, MJPEG_QMAT_DATA_REG, (int)(quotient & 0xFFFFFFFF));
            }else
                JpuWriteInstReg(instRegIndex, MJPEG_QMAT_DATA_REG, (int)(divisor<<20)|(int)(quotient & 0xFFFFF));

        }
        JpuWriteInstReg(instRegIndex, MJPEG_QMAT_CTRL_REG, t);
    }


    return 1;
}


#define PUT_BYTE(_p, _b) \
    if (tot++ > len) return 0; \
    *_p++ = (unsigned char)(_b);


int JpgEncEncodeHeader(JpgEncHandle handle, JpgEncParamSet * para)
{
    JpgInst *pJpgInst;
    JpgEncInfo *pEncInfo;
    BYTE *p;
    int tot, len, pad;
    BYTE *pCInfoTab[4];
    int frameFormat;
    Int32 i;
    Uint32  width, height;

    tot = 0;
    pJpgInst = handle;
    pEncInfo = &pJpgInst->JpgInfo->encInfo;

    p = para->pParaSet;
    len = para->size;

    // SOI Header
    PUT_BYTE(p, 0xff);
    PUT_BYTE(p, 0xD8);


    if (!para->disableAPPMarker)
    {
        // APP9 Header
        PUT_BYTE(p, 0xFF);
        PUT_BYTE(p, 0xE9);

        PUT_BYTE(p, 0x00);
        PUT_BYTE(p, 0x04);

        PUT_BYTE(p, (pEncInfo->frameIdx >> 8));
        PUT_BYTE(p, (pEncInfo->frameIdx & 0xFF));
    }


    // DRI header
    if (pEncInfo->rstIntval) {

        PUT_BYTE(p, 0xFF);
        PUT_BYTE(p, 0xDD);

        PUT_BYTE(p, 0x00);
        PUT_BYTE(p, 0x04);

        PUT_BYTE(p, (pEncInfo->rstIntval >> 8));
        PUT_BYTE(p, (pEncInfo->rstIntval & 0xff));

    }

    // DQT Header
    PUT_BYTE(p, 0xFF);
    PUT_BYTE(p, 0xDB);

    if (para->quantMode == JPG_TBL_NORMAL)
    {
        PUT_BYTE(p, 0x00);
        if (pEncInfo->q_prec0 == TRUE) {
            PUT_BYTE(p, 0x83);
        }
        else {
            PUT_BYTE(p, 0x43);
        }

        if (pEncInfo->q_prec0 == TRUE) {
            PUT_BYTE(p, 0x10);	// Pq
        }
        else {
            PUT_BYTE(p, 0x00);	// Pq
        }

        for (i=0; i<64; i++) {
            if (pEncInfo->q_prec0 == TRUE) {
                Uint16 q = pEncInfo->pQMatTab[0][i];
                PUT_BYTE(p, (q>>8)&0xff);
                PUT_BYTE(p, q&0xff);
            }
            else {
                PUT_BYTE(p, pEncInfo->pQMatTab[0][i]);
            }
        }

        if (pEncInfo->format != FORMAT_400) {
            PUT_BYTE(p, 0xFF);
            PUT_BYTE(p, 0xDB);

            PUT_BYTE(p, 0x00);
            if (pEncInfo->q_prec1 == TRUE) {
                PUT_BYTE(p, 0x83);
            }
            else {
                PUT_BYTE(p, 0x43);
            }
            if (pEncInfo->q_prec1 == TRUE) {
                PUT_BYTE(p, 0x11);	// Pq
            }
            else {
                PUT_BYTE(p, 0x01);	// Pq
            }

            for (i=0; i<64; i++) {
                if (pEncInfo->q_prec1 == TRUE) {
                    Uint16 q = pEncInfo->pQMatTab[1][i];
                    PUT_BYTE(p, (q>>8)&0xff);
                    PUT_BYTE(p, q&0xff);
                }
                else {
                    PUT_BYTE(p, pEncInfo->pQMatTab[1][i]);
                }
            }
        }
    }
    else// if (para->quantMode == JPG_TBL_MERGE)
    {

        if (pEncInfo->format != FORMAT_400)
        {
            Uint16 qLength = 0x84;
            if (pEncInfo->q_prec0 == TRUE) {
                qLength += 64;
            }
            if (pEncInfo->q_prec1 == TRUE) {
                qLength += 64;
            }
            PUT_BYTE(p, (qLength>>8)&0xff);
            PUT_BYTE(p, qLength&0xff);
        }
        else
        {
            Uint16 qLength = 0x43;
            if (pEncInfo->q_prec0 == TRUE) {
                qLength += 64;
            }
            PUT_BYTE(p, (qLength>>8)&0xff);
            PUT_BYTE(p, qLength&0xff);
        }

        PUT_BYTE(p, 0x00 | (pEncInfo->q_prec0 << 4));	// Luma
        for (i=0; i<64; i++) {
            if (pEncInfo->q_prec0 == TRUE) {
                Uint16 q = pEncInfo->pQMatTab[0][i];
                PUT_BYTE(p, (q>>8)&0xff);
                PUT_BYTE(p, q&0xff);
            }
            else {
                PUT_BYTE(p, pEncInfo->pQMatTab[0][i]);
            }
        }

        if (pEncInfo->format != FORMAT_400) {
            PUT_BYTE(p, 0x01 | (pEncInfo->q_prec1 << 4));	// Croma
            for (i=0; i<64; i++) {
                if (pEncInfo->q_prec0 == TRUE) {
                    Uint16 q = pEncInfo->pQMatTab[1][i];
                    PUT_BYTE(p, (q>>8)&0xff);
                    PUT_BYTE(p, q&0xff);
                }
                else {
                    PUT_BYTE(p, pEncInfo->pQMatTab[1][i]);
                }
            }
        }
    }

    // DHT Header
    PUT_BYTE(p, 0xFF);
    PUT_BYTE(p, 0xC4);

    if (para->huffMode == JPG_TBL_NORMAL)
    {
        Int32  numHuffValDC = pEncInfo->jpg12bit == TRUE ? 13 : 12;
        Int32  numHuffValAC = pEncInfo->jpg12bit == TRUE ? 256: 162;
        Uint16 LhDC         = pEncInfo->jpg12bit == TRUE ? 32 : 31;
        Uint16 LhAC         = pEncInfo->jpg12bit == TRUE ? 275 : 181;

        /* Lh: Huffman table definition length */

        PUT_BYTE(p, (LhDC>>8));
        PUT_BYTE(p, LhDC&0xff);

        PUT_BYTE(p, 0x00);  /* TcTh : DC : ID0 */

        for (i=0; i<16; i++) {
            PUT_BYTE(p, pEncInfo->pHuffBits[0][i]);
        }

        for (i=0; i<numHuffValDC; i++) {
            PUT_BYTE(p, pEncInfo->pHuffVal[0][i]);
        }

        PUT_BYTE(p, 0xFF);
        PUT_BYTE(p, 0xC4);

        PUT_BYTE(p, (LhAC>>8));
        PUT_BYTE(p, LhAC&0xff);

        PUT_BYTE(p, 0x10);  /* TcTh = AC : ID0 */

        for (i=0; i<16; i++) {
            PUT_BYTE(p, pEncInfo->pHuffBits[1][i]);
        }

        for (i=0; i<numHuffValAC; i++) {
            PUT_BYTE(p, pEncInfo->pHuffVal[1][i]);
        }

        if (pEncInfo->format != FORMAT_400) {

            PUT_BYTE(p, 0xFF);
            PUT_BYTE(p, 0xC4);

            PUT_BYTE(p, (LhDC>>8)&0xff);
            PUT_BYTE(p, LhDC&0xff);

            PUT_BYTE(p, 0x01);


            for (i=0; i<16; i++) {
                PUT_BYTE(p, pEncInfo->pHuffBits[2][i]);
            }
            for (i=0; i<numHuffValDC; i++) {
                PUT_BYTE(p, pEncInfo->pHuffVal[2][i]);
            }

            PUT_BYTE(p, 0xFF);
            PUT_BYTE(p, 0xC4);

            PUT_BYTE(p, (LhAC>>8)&0xff);
            PUT_BYTE(p, LhAC&0xff);

            PUT_BYTE(p, 0x11);


            for (i=0; i<16; i++) {
                PUT_BYTE(p, pEncInfo->pHuffBits[3][i]);
            }

            for (i=0; i<numHuffValAC; i++) {
                PUT_BYTE(p, pEncInfo->pHuffVal[3][i]);
            }
            if (pEncInfo->jpg12bit == TRUE) {
                PUT_BYTE(p, 0xFF);
                PUT_BYTE(p, 0xC4);

                PUT_BYTE(p, (LhDC>>8)&0xff);
                PUT_BYTE(p, LhDC&0xff);

                PUT_BYTE(p, 0x02);  /* TcTh = DC : ID2 */


                for (i=0; i<16; i++) {
                    PUT_BYTE(p, pEncInfo->pHuffBits[4][i]);
                }
                for (i=0; i<numHuffValDC; i++) {
                    PUT_BYTE(p, pEncInfo->pHuffVal[4][i]);
                }

                PUT_BYTE(p, 0xFF);
                PUT_BYTE(p, 0xC4);

                PUT_BYTE(p, (LhAC>>8)&0xff);
                PUT_BYTE(p, LhAC&0xff);

                PUT_BYTE(p, 0x12);  /* TcTh = AC : ID2 */


                for (i=0; i<16; i++) {
                    PUT_BYTE(p, pEncInfo->pHuffBits[5][i]);
                }

                for (i=0; i<numHuffValAC; i++) {
                    PUT_BYTE(p, pEncInfo->pHuffVal[5][i]);
                }
            }
        }
    }
    else// if (para->huffMode == JPG_TBL_MERGE)
    {
        if (pEncInfo->format != FORMAT_400)
        {
            PUT_BYTE(p, 0x01);
            PUT_BYTE(p, 0xA2);
            //PUT_BYTE(p, 2 + 1*4 + 28 + 178 + 28 + 178 );
        }
        else
        {
            PUT_BYTE(p, 0x00);
            PUT_BYTE(p, 0xD4);
            //PUT_BYTE(p, 2 + 1*4 + 28 + 178 );
        }

        PUT_BYTE(p, 0x00);

        for (i=0; i<16; i++) {
            PUT_BYTE(p, pEncInfo->pHuffBits[0][i]);
        }

        for (i=0; i<12; i++) {
            PUT_BYTE(p, pEncInfo->pHuffVal[0][i]);
        }


        PUT_BYTE(p, 0x10);

        for (i=0; i<16; i++) {
            PUT_BYTE(p, pEncInfo->pHuffBits[1][i]);
        }

        for (i=0; i<162; i++) {
            PUT_BYTE(p, pEncInfo->pHuffVal[1][i]);
        }


        if (pEncInfo->format != FORMAT_400) {

            PUT_BYTE(p, 0x01);


            for (i=0; i<16; i++) {
                PUT_BYTE(p, pEncInfo->pHuffBits[2][i]);
            }
            for (i=0; i<12; i++) {
                PUT_BYTE(p, pEncInfo->pHuffVal[2][i]);
            }


            PUT_BYTE(p, 0x11);

            for (i=0; i<16; i++) {
                PUT_BYTE(p, pEncInfo->pHuffBits[3][i]);
            }

            for (i=0; i<162; i++) {
                PUT_BYTE(p, pEncInfo->pHuffVal[3][i]);
            }
        }
    }

    // SOF header
    PUT_BYTE(p, 0xFF);
    PUT_BYTE(p, (pEncInfo->jpg12bit == TRUE ? 0xC1 : 0xC0));
    PUT_BYTE(p, (((8+(pEncInfo->compNum*3)) >> 8) & 0xFF));
    PUT_BYTE(p, ((8+(pEncInfo->compNum*3)) & 0xFF));
    PUT_BYTE(p, (pEncInfo->jpg12bit == TRUE ? 12 : 8));
    width       = pEncInfo->picWidth;
    height      = pEncInfo->picHeight;
    frameFormat = pEncInfo->format;
    if (pEncInfo->rotationIndex == 1 || pEncInfo->rotationIndex == 3) {
        // rotation angle = rotationIndex x 90;
        width  = pEncInfo->picHeight;
        height = pEncInfo->picWidth;
        if (frameFormat == FORMAT_422)      frameFormat = FORMAT_440;
        else if (frameFormat == FORMAT_440) frameFormat = FORMAT_422;
    }
    PUT_BYTE(p, (height >> 8));
    PUT_BYTE(p, (height & 0xFF));
    PUT_BYTE(p, (width >> 8));
    PUT_BYTE(p, (width & 0xFF));
    PUT_BYTE(p, pEncInfo->compNum);


    pCInfoTab[0] = sJpuCompInfoTable[frameFormat];
    pCInfoTab[1] = pCInfoTab[0] + 6;
    pCInfoTab[2] = pCInfoTab[1] + 6;
    pCInfoTab[3] = pCInfoTab[2] + 6;

    for (i=0; i<pEncInfo->compNum; i++) {
        PUT_BYTE(p, (i+1));
        PUT_BYTE(p, ((pCInfoTab[i][1]<<4) & 0xF0) + (pCInfoTab[i][2] & 0x0F));
        PUT_BYTE(p, pCInfoTab[i][3]);
    }

    if (para->enableSofStuffing)
    {
        pad = 0;
        if (tot % 8) {
            pad = tot % 8;
            pad = 8-pad;
            for (i=0; i<pad; i++) {
                PUT_BYTE(p, 0xFF);
            }
        }
    }

    pEncInfo->frameIdx++;
    para->size = tot;

    return tot;
}


void JpgEncSetQualityFactor(JpgEncHandle handle, Uint32 quality, BOOL useStdTable)
{
    JpgInst *pJpgInst;
    JpgEncInfo *pEncInfo;
    Uint32 scaleFactor;
    Uint32 i;
    Uint32 temp;

    pJpgInst = handle;
    pEncInfo = &pJpgInst->JpgInfo->encInfo;
    /* These are the sample quantization tables given in JPEG spec section K.1.
    * The spec says that the values given produce "good" quality, and
    * when divided by 2, "very good" quality.
    */
    static const Uint32 std_luminance_quant_tbl[64] = {
        16, 11, 10, 16, 24, 40, 51, 61,
        12, 12, 14, 19, 26, 58, 60, 55,
        14, 13, 16, 24, 40, 57, 69, 56,
        14, 17, 22, 29, 51, 87, 80, 62,
        18, 22, 37, 56, 68, 109, 103, 77,
        24, 35, 55, 64, 81, 104, 113, 92,
        49, 64, 78, 87, 103, 121, 120, 101,
        72, 92, 95, 98, 112, 100, 103, 99
    };
    static const Uint32 std_chrominance_quant_tbl[64] = {
        17, 18, 24, 47, 99, 99, 99, 99,
        18, 21, 26, 66, 99, 99, 99, 99,
        24, 26, 56, 99, 99, 99, 99, 99,
        47, 66, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99
    };
    Uint32 pLumaQTable[64];
    Uint32 pChromaQTable[64];
    Uint32 qSize = 1, qTableSize;
    Uint32 minQvalue = MIN_Q8_ELEMENT;

    if (quality <= 0) quality = 1;
    if (quality > 100) quality = 100;

    for (i = 0; i<64; i++) {
        pLumaQTable[i] = (useStdTable == TRUE) ? std_luminance_quant_tbl[i] : (Int32)pEncInfo->pQMatTab[DC_TABLE_INDEX0][i];
    }
    for (i = 0; i<64; i++) {
        pChromaQTable[i] = (useStdTable == TRUE) ? std_chrominance_quant_tbl[i] : (Int32)pEncInfo->pQMatTab[AC_TABLE_INDEX0][i];
    }

    minQvalue = (pEncInfo->jpg12bit == TRUE) ? MIN_Q16_ELEMENT : MIN_Q8_ELEMENT;

    /* The basic table is used as-is (scaling 100) for a quality of 50.
    * Qualities 50..100 are converted to scaling percentage 200 - 2*Q;
    * note that at Q=100 the scaling is 0, which will cause jpeg_add_quant_table
    * to make all the table entries 1 (hence, minimum quantization loss).
    * Qualities 1..50 are converted to scaling percentage 5000/Q.
    */
    if (quality < 50)
        scaleFactor = 5000 / quality;
    else
        scaleFactor = 200 - quality * 2;


    for (i = 0; i < 64; i++) {
        temp = (pLumaQTable[i] * scaleFactor + 50) / 100;
        /* limit the values to the valid range */
        temp = (temp < minQvalue) ? minQvalue : temp;

        if (temp > 32767)
            temp = 32767; /* max quantizer needed for 12 bits */
        if (pEncInfo->q_prec0 == FALSE && temp > 255)
            temp = 255;                 /* limit to baseline range if requested */
        pEncInfo->pQMatTab[DC_TABLE_INDEX0][i] = temp;
    }

    for (i = 0; i < 64; i++) {
        temp = (pChromaQTable[i] * scaleFactor + 50) / 100;
        /* limit the values to the valid range */
        temp = (temp < minQvalue) ? minQvalue : temp;

        if (temp > 32767) temp = 32767;    /* max quantizer needed for 12 bits */
        if (pEncInfo->q_prec1 == FALSE && temp > 255)
            temp = 255;                     /* limit to baseline range if requested */

        pEncInfo->pQMatTab[AC_TABLE_INDEX0][i] = temp;
    }

    qSize = pEncInfo->jpg12bit ? 2 : 1;

    //setting of qmatrix table information
#ifdef USE_CNM_DEFAULT_QMAT_TABLE
    memset((void*)pEncInfo->pQMatTab[DC_TABLE_INDEX0], 0x00, 64 * qSize);
    memcpy((void*)pEncInfo->pQMatTab[DC_TABLE_INDEX0], (void*)lumaQ2, 64 * qSize);

    memset((void*)pEncInfo->pQMatTab[AC_TABLE_INDEX0], 0x00, 64 * qSize);
    memcpy((void*)ppEncInfo->pQMatTab[AC_TABLE_INDEX0], (void*)chromaBQ2, 64 * qSize);
#endif

    qTableSize = 64 * qSize;
    memcpy((void*)pEncInfo->pQMatTab[DC_TABLE_INDEX1], (void*)pEncInfo->pQMatTab[DC_TABLE_INDEX0], qTableSize);
    memcpy((void*)pEncInfo->pQMatTab[AC_TABLE_INDEX1], (void*)pEncInfo->pQMatTab[AC_TABLE_INDEX0], qTableSize);
}

JpgRet JpgEnterLock()
{
    jdi_lock();

    return JPG_RET_SUCCESS;

}

JpgRet JpgLeaveLock()
{

    jdi_unlock();
    return JPG_RET_SUCCESS;
}

JpgRet JpgSetClockGateEx(Uint32 on, Uint32 instIdx)
{
    JpgInst *inst;
    jpu_instance_pool_t *jip;

    jip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!jip)
        return JPG_RET_FAILURE;

    inst = (JpgInst *)jip->instPendingInst[instIdx];
    if(inst && !on)
        return JPG_RET_SUCCESS;

    jdi_set_clock_gate(on);

    return JPG_RET_SUCCESS;
}


JpgInst *GetJpgPendingInstEx(Uint32 instIdx)
{
    jpu_instance_pool_t *jip;

    jip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!jip)
        return NULL;

    if (instIdx >= MAX_NUM_INSTANCE)
        return NULL;

    return jip->instPendingInst[instIdx];
}



void SetJpgPendingInstEx(JpgInst *inst, Uint32 instIdx)
{
    jpu_instance_pool_t *jip;

    jip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!jip)
        return;

    if (instIdx >= MAX_NUM_INSTANCE)
        return;
    jip->instPendingInst[instIdx] = inst;
}

void ClearJpgPendingInstEx(Uint32 instIdx)
{
    jpu_instance_pool_t *jip;

    jip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!jip)
        return;

    if (instIdx >= MAX_NUM_INSTANCE)
        return;

    if(jip->instPendingInst[instIdx])
        jip->instPendingInst[instIdx] = 0;
}

