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
#ifndef __CODA7Q_FUNCTION_H__
#define __CODA7Q_FUNCTION_H__

#include "vpuapi.h"
#include "product.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern RetCode Coda7qVpuDecInitSeq(
    CodecInst*  instance 
    );

extern RetCode Coda7qVpuDecode(
    CodecInst*      instance, 
    DecParam*       option
    );

extern RetCode Coda7qVpuDecGetResult(
    CodecInst*      instance,
    DecOutputInfo*  result
    );

extern RetCode Coda7qVpuDecGetSeqInfo(
    CodecInst*      instance,
    DecInitialInfo* info
    );

extern RetCode Coda7qVpuEncSetup(
    CodecInst* instance
    );

extern RetCode Coda7qVpuEncRegisterFramebuffer(
    CodecInst* instance, 
    FrameBuffer* fbArr, 
    TiledMapType mapType, 
    Uint32 count
    );

extern RetCode Coda7qVpuEncode(
    CodecInst* instance, 
    EncParam* option
    );

extern RetCode Coda7qVpuEncGetResult(
    CodecInst* instance, 
    EncOutputInfo* result
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CODA7Q_FUNCTION_H__ */

