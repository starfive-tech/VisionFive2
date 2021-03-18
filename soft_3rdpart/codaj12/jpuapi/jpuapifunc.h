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

#ifndef JPUAPI_UTIL_H_INCLUDED
#define JPUAPI_UTIL_H_INCLUDED

#include "jpuapi.h"

#define DC_TABLE_INDEX0				0
#define AC_TABLE_INDEX0				1
#define DC_TABLE_INDEX1				2
#define AC_TABLE_INDEX1				3

#define DC_TABLE_INDEX2				4
#define AC_TABLE_INDEX2				5
#define DC_TABLE_INDEX3				6
#define AC_TABLE_INDEX3				7

#define Q_COMPONENT0				0
#define Q_COMPONENT1				0x40
#define Q_COMPONENT2				0x80

#define THTC_LIST_CNT               8

/* CODAJ12 Constraints
* The minimum value of Qk is 8 for 16bit quantization element, 2 for 8bit quantization element
*/
#define MIN_Q16_ELEMENT     8
#define MIN_Q8_ELEMENT      2

typedef enum {
    JPG_START_PIC = 0,
    JPG_START_INIT,
    JPG_START_PARTIAL,
    JPG_ENABLE_START_PIC = 4,
    JPG_DEC_SLICE_ENABLE_START_PIC = 5
}JpgStartCmd;


typedef enum {
    INST_CTRL_IDLE = 0,
    INST_CTRL_LOAD = 1,
    INST_CTRL_RUN = 2,
    INST_CTRL_PAUSE = 3,
    INST_CTRL_ENC = 4,
    INST_CTRL_PIC_DONE = 5,
    INST_CTRL_SLC_DONE = 6
}InstCtrlStates;

typedef struct{
    Uint32	tag;
    Uint32	type;
    int		count;
    int		offset;
}TAG;

enum {
    JFIF    	= 0,
    JFXX_JPG	= 1,
    JFXX_PAL  	= 2,
    JFXX_RAW 	= 3,
    EXIF_JPG	= 4
};

typedef struct {
    int			PicX;
    int			PicY;
    int			BitPerSample[3];
    int			Compression; // 1 for uncompressed / 6 for compressed(jpeg)
    int			PixelComposition; // 2 for RGB / 6 for YCbCr
    int			SamplePerPixel;
    int  		PlanrConfig; // 1 for chunky / 2 for planar
    int			YCbCrSubSample; // 00020002 for YCbCr 4:2:0 / 00020001 for YCbCr 4:2:2
    Uint32		JpegOffset;
    Uint32		JpegThumbSize;
} EXIF_INFO;



typedef struct {
    BYTE *buffer;
    int index;
    int size;
} vpu_getbit_context_t;

#define init_get_bits(CTX, BUFFER, SIZE) JpuGbuInit(CTX, BUFFER, SIZE)
#define show_bits(CTX, NUM) JpuGguShowBit(CTX, NUM)
#define get_bits(CTX, NUM) JpuGbuGetBit(CTX, NUM)
#define get_bits_left(CTX) JpuGbuGetLeftBitCount(CTX)
#define get_bits_count(CTX) JpuGbuGetUsedBitCount(CTX)

typedef struct {

    PhysicalAddress streamWrPtr;
    PhysicalAddress streamRdPtr;
    int	streamEndflag;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
    int streamBufSize;
    BYTE *pBitStream;

    int frameOffset;
    int consumeByte;
    int nextOffset;
    int currOffset;

    FrameBuffer * frameBufPool;
    int numFrameBuffers;
    int stride;
    int initialInfoObtained;
    int minFrameBufferNum;
    int streamEndian;
    int frameEndian;
    Uint32  chromaInterleave;

    Uint32  picWidth;
    Uint32  picHeight;
    Uint32  alignedWidth;
    Uint32  alignedHeight;
    int headerSize;
    int ecsPtr;
    int pagePtr;
    int wordPtr;
    int bitPtr;
    FrameFormat format;
    int rstIntval;

    int userHuffTab;
    int userqMatTab;

    int huffDcIdx;
    int huffAcIdx;
    int Qidx;

    BYTE huffVal[8][256];
    BYTE huffBits[8][256];
    short qMatTab[4][64];
    Uint32 huffMin[8][16];
    Uint32 huffMax[8][16];
    BYTE huffPtr[8][16];
    BYTE cInfoTab[4][6];

    int busReqNum;
    int compNum;
    int mcuBlockNum;
    int compInfo[3];
    int frameIdx;
    int bitEmpty;
    int iHorScaleMode;
    int iVerScaleMode;
    Uint32  mcuWidth;
    Uint32  mcuHeight;
    vpu_getbit_context_t gbc;


    //ROI
    Uint32          roiEnable;
    Uint32          roiOffsetX;
    Uint32          roiOffsetY;
    Uint32          roiWidth;
    Uint32          roiHeight;
    Uint32          roiMcuOffsetX;
    Uint32          roiMcuOffsetY;
    Uint32          roiMcuWidth;
    Uint32          roiMcuHeight;
    Uint32          roiFrameWidth;
    Uint32          roiFrameHeight;
    PackedFormat    packedFormat;

    int             jpg12bit;
    int             q_prec0;
    int             q_prec1;
    int             q_prec2;
    int             q_prec3;
    Uint32          pixelJustification;
    Uint32          ofmt;
    Uint32          stride_c;
    Uint32          bitDepth;
    Uint32          intrEnableBit;
    Uint32          decIdx;
    Uint32          rotationIndex;                  /*!<< 0: 0, 1: 90 CCW, 2: 180 CCW, 3: 270 CCW CCW(Counter Clockwise)*/
    JpgMirrorDirection mirrorIndex;                 /*!<< 0: none, 1: vertical mirror, 2: horizontal mirror, 3: both */
    Int32           thtc[THTC_LIST_CNT];            /*!<< Huffman table definition length and table class list : -1 indicates not exist. */
    Uint32          numHuffmanTable;
} JpgDecInfo;

typedef struct {
    PhysicalAddress streamRdPtr;
    PhysicalAddress streamWrPtr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;

    FrameBuffer * frameBufPool;
    int numFrameBuffers;
    int stride;
    int initialInfoObtained;

    Uint32  picWidth;
    Uint32  picHeight;
    Uint32  alignedWidth;
    Uint32  alignedHeight;
    Uint32  mcuWidth;
    Uint32  mcuHeight;
    int seqInited;
    int frameIdx;
    FrameFormat format;

    int streamEndian;
    int frameEndian;
    Uint32 chromaInterleave;

    int rstIntval;
    int busReqNum;
    int mcuBlockNum;
    int compNum;
    int compInfo[3];

    // give command
    int disableAPPMarker;
    int quantMode;
    int stuffByteEnable;

    Uint32  huffCode[8][256];
    Uint32  huffSize[8][256];
    BYTE*   pHuffVal[8];
    BYTE*   pHuffBits[8];
    short*  pQMatTab[4];
    int     jpg12bit;
    int     q_prec0;
    int     q_prec1;

    BYTE *pCInfoTab[4];

    PackedFormat    packedFormat;

    Uint32  pixelJustification;
    Int32   sliceHeight;
    Uint32  intrEnableBit;
    Uint32  encIdx;
    Uint32  encSlicePosY;
    Uint32  tiledModeEnable;
    Uint32  rotationIndex;          /*!<< 0: 0, 1: 90 CCW, 2: 180 CCW, 3: 270 CCW CCW(Counter Clockwise)*/
    Uint32  mirrorIndex;            /*!<< 0: none, 1: vertical mirror, 2: horizontal mirror, 3: both */
} JpgEncInfo;

typedef struct JpgInst {
    Int32   inUse;
    Int32   instIndex;
    Int32   loggingEnable;
	BOOL sliceInstMode;
    union {
        JpgEncInfo encInfo;
        JpgDecInfo decInfo;
    } *JpgInfo;
} JpgInst;



extern Uint8 sJpuCompInfoTable[5][24];
extern Uint8 sJpuCompInfoTable_EX[5][24];


#ifdef __cplusplus
extern "C" {
#endif
    JpgRet GetJpgInstance(JpgInst ** ppInst);
    void	FreeJpgInstance(JpgInst * pJpgInst);
    JpgRet CheckJpgInstValidity(JpgInst * pci);
    JpgRet CheckJpgDecOpenParam(JpgDecOpenParam * pop);

    int JpuGbuInit(vpu_getbit_context_t *ctx, BYTE *buffer, int size);
    int JpuGbuGetUsedBitCount(vpu_getbit_context_t *ctx);
    int JpuGbuGetLeftBitCount(vpu_getbit_context_t *ctx);
    unsigned int JpuGbuGetBit(vpu_getbit_context_t *ctx, int bit_num);
    unsigned int JpuGguShowBit(vpu_getbit_context_t *ctx, int bit_num);

    int JpegDecodeHeader(JpgDecInfo * jpg);
    int JpgDecQMatTabSetUp(JpgDecInfo *jpg, int instRegIndex);
    int JpgDecHuffTabSetUp(JpgDecInfo *jpg, int instRegIndex);
    int JpgDecHuffTabSetUp_12b(JpgDecInfo *jpg, int instRegIndex);
    void JpgDecGramSetup(JpgDecInfo * jpg, int instRegIndex);


    JpgRet CheckJpgEncOpenParam(JpgEncOpenParam * pop, JPUCap* cap);
    JpgRet CheckJpgEncParam(JpgEncHandle handle, JpgEncParam * param);
    int JpgEncLoadHuffTab(JpgInst *pJpgInst, int instRegIndex);
    int JpgEncLoadHuffTab_12b(JpgInst *pJpgInst, int instRegIndex);
    int JpgEncLoadQMatTab(JpgInst *pJpgInst, int instRegIndex);
    int JpgEncEncodeHeader(JpgEncHandle handle, JpgEncParamSet * para);
    void JpgEncSetQualityFactor(JpgEncHandle handle, Uint32 level, BOOL useStdTable);

    JpgRet JpgEnterLock();
    JpgRet JpgLeaveLock();
    JpgRet JpgSetClockGate(Uint32 on);

    JpgInst *GetJpgPendingInstEx(Uint32 instIdx);
    void SetJpgPendingInstEx(JpgInst *inst, Uint32 instIdx);
    void ClearJpgPendingInstEx(Uint32 instIdx);

    Uint32 GetDec8bitBusReqNum(FrameFormat iFormat, PackedFormat oPackMode);
    Uint32 GetDec12bitBusReqNum(FrameFormat iFormat, PackedFormat oPackMode);
    Uint32 GetEnc8bitBusReqNum(PackedFormat iPackMode, FrameFormat oFormat);
    Uint32 GetEnc12bitBusReqNum(PackedFormat iPackMode, FrameFormat oFormat);

#ifdef __cplusplus
}
#endif


#endif // endif JPUAPI_UTIL_H_INCLUDED
