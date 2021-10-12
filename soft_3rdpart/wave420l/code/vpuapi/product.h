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

#ifndef __VPUAPI_PRODUCT_ABSTRACT_H__
#define __VPUAPI_PRODUCT_ABSTRACT_H__

#include "vpuapi.h"
#include "vpuapifunc.h"

#define IS_DECODER_HANDLE(_instance)      (_instance->codecMode < AVC_ENC)
#define IS_7Q_DECODER_HANDLE(_instance)   (_instance->codecMode < C7_AVC_ENC)

enum {
    FramebufCacheNone,
    FramebufCacheMaverickI,
    FramebufCacheMaverickII,
};

typedef enum {
    AUX_BUF_TYPE_MVCOL,
    AUX_BUF_TYPE_FBC_Y_OFFSET,
    AUX_BUF_TYPE_FBC_C_OFFSET
} AUX_BUF_TYPE;

typedef struct _tag_FramebufferIndex {
    Int16 tiledIndex;
    Int16 linearIndex;
} FramebufferIndex;

typedef struct _tag_VpuAttrStruct {
    Uint32  productId;
    Uint32  productNumber;
    char    productName[8];
    Uint32  supportDecoders;            /* bitmask: See CodStd in vpuapi.h */
    Uint32  supportEncoders;            /* bitmask: See CodStd in vpuapi.h */
    Uint32  numberOfMemProtectRgns;     /* always 10 */
    Uint32  numberOfVCores;
    BOOL    supportWTL;
    BOOL    supportTiled2Linear;
    BOOL    supportTiledMap;
    BOOL    supportMapTypes;            /* Framebuffer map type */
    BOOL    supportLinear2Tiled;        /* Encoder */
    BOOL    support128bitBus;
    BOOL    supportThumbnailMode;
    BOOL    supportBitstreamMode;
    BOOL    supportFBCBWOptimization;   /* WAVE4xx decoder feature */
    BOOL    supportGDIHW;
    BOOL    supportCommandQueue;
    Uint32  supportEndianMask;
    Uint32  framebufferCacheType;       /* 0: for None, 1 - Maverick-I, 2 - Maverick-II */
    Uint32  bitstreamBufferMargin;
    Uint32  interruptEnableFlag;
    Uint32  hwConfigDef0;
    Uint32  hwConfigDef1;
    Uint32  hwConfigFeature;            /**<< supported codec standards */
    Uint32  hwConfigDate;               /**<< Configuation Date Decimal, ex)20151130 */
    Uint32  hwConfigRev;                /**<< SVN revision, ex) 83521 */
    Uint32  hwConfigType;               /**<< Not defined yet */
} VpuAttr;

extern VpuAttr g_VpuCoreAttributes[MAX_NUM_VPU_CORE];

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Returns 0 - not found product id
 *         1 - found product id
 */
extern Uint32 ProductVpuScan(
    Uint32 coreIdx
    );


extern Int32 ProductVpuGetId(
    Uint32 coreIdx
    );

extern RetCode ProductVpuGetVersion(
    Uint32  coreIdx, 
    Uint32* versionInfo, 
    Uint32* revision 
    );
#ifdef DRAM_TEST
extern RetCode ProductVpuDRAMReadWriteTest(
    Uint32  coreIdx, 
    Uint32* dram_source_addr, 
    Uint32* dram_destination_addr,
    Uint32* dram_data_size
    );
#endif

extern RetCode ProductVpuInit(
    Uint32 coreIdx,
    void*  firmware,
    Uint32 size
    );

extern RetCode ProductVpuReInit(
    Uint32 coreIdx,
    void*  firmware,
    Uint32 size
    );

extern Uint32 ProductVpuIsInit(
    Uint32 coreIdx
    );

extern Int32 ProductVpuIsBusy(
    Uint32 coreIdx
    );

extern Int32 ProductVpuWaitInterrupt(
    CodecInst*  instance,
    Int32       timeout
    );

extern RetCode ProductVpuReset(
    Uint32      coreIdx,
    SWResetMode resetMode
    );

extern RetCode ProductVpuSleepWake(
    Uint32 coreIdx,
    int iSleepWake,
     const Uint16* code, 
     Uint32 size
    );

extern RetCode ProductVpuClearInterrupt(
    Uint32      coreIdx,
    Uint32      flags
    );

extern RetCode ProductVpuDecBuildUpOpenParam(
    CodecInst*    instance,
    DecOpenParam* param
    );

extern RetCode ProductCheckDecOpenParam(
    DecOpenParam* param
    );

extern RetCode ProductVpuDecInitSeq(
    CodecInst*  instance
    );

extern RetCode ProductVpuDecFiniSeq(
    CodecInst*  instance
    );

extern RetCode ProductVpuDecSetBitstreamFlag(
    CodecInst*  instance,
    BOOL        running,
    Int32       size
    );

extern RetCode ProductCpbFlush(
    CodecInst*  instance
    );
/*
 * FINI_SEQ
 */
extern RetCode ProductVpuDecEndSequence(
    CodecInst*  instance
    );

/**
 *  @brief      Abstract function for SEQ_INIT.
 */
extern RetCode ProductVpuDecGetSeqInfo(
    CodecInst*      instance, 
    DecInitialInfo* info
    );

/**
 *  \brief      Check parameters for product specific decoder.
 */
extern RetCode ProductVpuDecCheckCapability(
    CodecInst*  instance
    );

/**
 * \brief       Decode a coded picture.  
 */
extern RetCode ProductVpuDecode(
    CodecInst*  instance,
    DecParam*   option
    );

/**
 *
 */
extern RetCode ProductVpuDecGetResult(
    CodecInst*      instance,
    DecOutputInfo*  result
    );

/**
 * \brief                   Flush framebuffers to prepare decoding new sequence
 * \param   instance        decoder handle
 * \param   retIndexes      Storing framebuffer indexes in display order. 
 *                          If retIndexes[i] is -1 then there is no display index from i-th.
 */
extern RetCode ProductVpuDecFlush(
    CodecInst*          instance,
    FramebufferIndex*   retIndexes,
    Uint32              size
    );

/**
 *  \brief      Allocate framebuffers with given parameters 
 */
extern RetCode ProductVpuAllocateFramebuffer(
    CodecInst*          instance, 
    FrameBuffer*        fbArr, 
    TiledMapType        mapType, 
    Int32               num, 
    Int32               stride, 
    Int32               height,
    FrameBufferFormat   format,
    BOOL                cbcrInterleave,
    BOOL                nv21,
    Int32               endian,
    vpu_buffer_t*       vb,
    Int32               gdiIndex,
    FramebufferAllocType fbType
    );

/**
 *  \brief      Register framebuffers to VPU
 */
extern RetCode ProductVpuRegisterFramebuffer(
    CodecInst*      instance
    );

extern RetCode ProductVpuDecUpdateFrameBuffer(
    CodecInst*   instance,
    FrameBuffer* fbcFb,
    FrameBuffer* linearFb,
    Uint32       mvColIndex,
    Uint32       picWidth, 
    Uint32       picHeight
    );

extern Int32 ProductCalculateFrameBufSize(
    Int32               productId, 
    Int32               stride, 
    Int32               height, 
    TiledMapType        mapType, 
    FrameBufferFormat   format,
    BOOL                interleave,
    DRAMConfig*         pDramCfg
    );
extern Int32 ProductCalculateAuxBufferSize(
    AUX_BUF_TYPE    type, 
    CodStd          codStd, 
    Int32           width, 
    Int32           height
    );

extern RetCode ProductClrDispFlag(
    CodecInst* instance, 
    Uint32 index
    );

extern RetCode ProductSetDispFlag(
    CodecInst* instance, 
    Uint32 dispFlag
    );
/************************************************************************/
/* ENCODER                                                              */
/************************************************************************/
extern RetCode ProductVpuEncBuildUpOpenParam(
    CodecInst*      pCodec, 
    EncOpenParam*   param
    );

extern RetCode ProductVpuEncFiniSeq(
    CodecInst*      instance
    );

extern RetCode ProductCheckEncOpenParam(
    EncOpenParam*   param
    );

extern RetCode ProductVpuEncSetup(
    CodecInst*      instance
    );

extern RetCode ProductVpuEncode(
    CodecInst*      instance, 
    EncParam*       param
    );

extern RetCode ProductVpuEncGetResult(
    CodecInst*      instance, 
    EncOutputInfo*  result
    );
extern RetCode ProductVpuEncGiveCommand(
    CodecInst* instance, 
    CodecCommand cmd, 
    void* param);

extern RetCode Wave4VpuEncGetHeader(
    EncHandle instance, 
    EncHeaderParam * encHeaderParam
    );

extern RetCode Wave4VpuEncParaChange(
    EncHandle instance, 
    EncChangeParam* param
    );
extern RetCode Coda7qVpuEncGetHeader(
    EncHandle instance, 
    EncHeaderParam * encHeaderParam
    );

extern RetCode ProductVpuEncInitSeq(
    CodecInst*  instance
    );
extern RetCode ProductVpuEncGetSeqInfo(
    CodecInst* instance, 
    EncInitialInfo* info
    );


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* __VPUAPI_PRODUCT_ABSTRACT_H__ */
 
