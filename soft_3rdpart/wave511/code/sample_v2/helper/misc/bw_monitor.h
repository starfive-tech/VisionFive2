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

#ifndef __BANDWIDTH_MONITOR_H__
#define __BANDWIDTH_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void*   BWCtx;

BWCtx
BWMonitorSetup(
    CodecInst*  instance,
    BOOL        perFrame,
    char*       strLogDir
    );

/**
 * @brief           Releases all resources allocated on BWMonitorset().
 *                  @ctx is released internally.
 */
void
BWMonitorRelease(
    BWCtx        ctx
    );

void
BWMonitorReset(
    BWCtx        ctx
    );

void
BWMonitorUpdate(
    BWCtx        ctx,
    Uint32       numCores
    );

void 
    BWMonitorUpdatePrint(
    BWCtx       context,
    Uint32      picType
    );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BANDWIDTH_MONITOR_H__ */
 
