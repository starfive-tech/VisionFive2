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
#ifndef __WAVE410_FUNCTION_H__
#define __WAVE410_FUNCTION_H__

#include "vpuapi.h"
#include "product.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern RetCode Wave4VpuDecInitSeq(
    CodecInst*  instance 
    );

extern RetCode Wave4VpuDecode(
    CodecInst*      instance, 
    DecParam*       option
    );

extern RetCode Wave4VpuDecGetResult(
    CodecInst*      instance,
    DecOutputInfo*  result
    );

extern RetCode Wave4VpuDecGetSeqInfo(
    CodecInst*      instance,
    DecInitialInfo* info
    );

extern RetCode Wave4VpuEncRegisterFramebuffer(
    CodecInst*      instance, 
    FrameBuffer*    fb,
    TiledMapType    mapType, 
    Uint32          count
    );

extern RetCode Wave4VpuEncSetup(
    CodecInst*      instance
    );

extern RetCode Wave4VpuEncode(
    CodecInst* instance, 
    EncParam* option
    );
extern RetCode Wave4VpuEncGetResult(
    CodecInst* instance,
    EncOutputInfo* result
    );
extern RetCode Wave4VpuEncGiveCommand(
    CodecInst*      pCodecInst, 
    CodecCommand    cmd, 
    void*           param
    );

extern RetCode CheckEncCommonParamValid(
    EncOpenParam* pop
    );

extern RetCode CheckEncRcParamValid(
    EncOpenParam* pop
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WAVE410_FUNCTION_H__ */

