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

#include "cnm_app.h"
#include "misc/bw_monitor.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct DecListenerContext {
    Component           renderer;
    Comparator          comparator;
    Int32               lastSeqNo;
    DecGetFramebufInfo  fbInfo;
    Uint32              compareType;
    BOOL                match;
    Uint32              notDecodedCount;

    /* performance & bandwidth */
    BOOL                performance;
    BOOL                bandwidth;
    PFCtx               pfCtx;
    BWCtx*              bwCtx;
    Uint32              fps;
    Uint32              pfClock;
    char                inputPath[MAX_FILE_PATH];
    Uint32              numVCores;
    CodStd              bitFormat;
    Int32               productId;
    BOOL                enableScaler;
} DecListenerContext;

void DecoderListener(Component com, Uint64 event, void* data, void* context);
BOOL SetupDecListenerContext(DecListenerContext* ctx, CNMComponentConfig* config, Component renderer);
void ClearDecListenerContext(DecListenerContext* ctx);
void HandleDecCompleteSeqEvent(Component com, CNMComListenerDecCompleteSeq* param, DecListenerContext* ctx);
void HandleDecRegisterFbEvent(Component com, CNMComListenerDecRegisterFb* param, DecListenerContext* ctx);
void HandleDecGetOutputEvent(Component com, CNMComListenerDecDone* param, DecListenerContext* ctx);
void HandleDecCloseEvent(Component com, CNMComListenerDecClose* param, DecListenerContext* ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

