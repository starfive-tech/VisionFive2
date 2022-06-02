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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "wave511/config.h"
    #include "wave511/sample_v2/helper/main_helper.h"
#else
    #include "config.h"
    #include "main_helper.h"
#endif


enum {
    CNMQC_ENV_NONE,
    CNMQC_ENV_GDBSERVER,            /*!<< It executes gdb server in order to debug F/W on the C&M FPGA env. */
    CNMQC_ENV_MAX,
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void InitializeDebugEnv(Uint32 options);
extern void ReleaseDebugEnv(void);
extern void ExecuteDebugger(void);

void ChekcAndPrintDebugInfo(VpuHandle handle, BOOL isEnc, RetCode result);

void PrintDecVpuStatus(
    DecHandle   handle
    );

void PrintEncVpuStatus(
    EncHandle   handle
    );

void PrintMemoryAccessViolationReason(
    Uint32          core_idx,
    void            *outp
    );

#define VCORE_DBG_ADDR(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x300
#define VCORE_DBG_DATA(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x304
#define VCORE_DBG_READY(__vCoreIdx)     0x8000+(0x1000*__vCoreIdx) + 0x308

void WriteRegVCE(
    Uint32   core_idx,
    Uint32   vce_core_idx,
    Uint32   vce_addr,
    Uint32   udata
    );

Uint32 ReadRegVCE(
    Uint32 core_idx,
    Uint32 vce_core_idx,
    Uint32 vce_addr
    );

extern char dumpTime[200];
#define HEXDUMP_COLS 16
void DisplayHex(void *mem, Uint32 len, char* name);


RetCode PrintVpuProductInfo(
    Uint32      core_idx,
    VpuAttr*    productInfo
    );


Int32 HandleDecInitSequenceError(
    DecHandle       handle,
    Uint32          productId,
    DecOpenParam*   openParam,
    DecInitialInfo* seqInfo,
    RetCode         apiErrorCode
    );

void HandleDecoderError(
    DecHandle       handle,
    Uint32          frameIdx,
    DecOutputInfo*  outputInfo
    );

void DumpCodeBuffer(
    const char* path
    );

void HandleEncoderError(
    EncHandle       handle,
    Uint32          encPicCnt,
    EncOutputInfo*  outputInfo
    );
Uint32 SetEncoderTimeout(
    int width,
    int height
    );

void print_busy_timeout_status(
    Uint32 coreIdx,
    Uint32 product_code,
    Uint32 pc
    );

void wave5xx_vcpu_status (
    unsigned long coreIdx
    );

void vdi_print_vpu_status(
    unsigned long coreIdx
    );

void wave5xx_bpu_status(
    Uint32 coreIdx
    );

void vdi_print_vcore_status(
    Uint32 coreIdx
    );

void vdi_print_vpu_status_dec(
    unsigned long coreIdx
    );

void vdi_print_vpu_status_enc(
    unsigned long coreIdx
    );

void vdi_log(
    unsigned long coreIdx,
    int cmd,
    int step
    );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DEBUG_H_ */

