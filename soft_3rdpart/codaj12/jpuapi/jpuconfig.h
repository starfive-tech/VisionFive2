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
#ifndef _JPU_CONFIG_H_
#define _JPU_CONFIG_H_

#include "../config.h"

#define MAX_NUM_JPU_CORE                1
#define MAX_NUM_INSTANCE                4
#define MAX_INST_HANDLE_SIZE            48

#define JPU_FRAME_ENDIAN                JDI_LITTLE_ENDIAN
#define JPU_STREAM_ENDIAN               JDI_LITTLE_ENDIAN
#define JPU_CHROMA_INTERLEAVE           1        // 0 (chroma separate mode), 1 (cbcr interleave mode), 2 (crcb interleave mode)

#define JPU_STUFFING_BYTE_FF            0        // 0 : ON ("0xFF"), 1 : OFF ("0x00") for stuffing


#define MAX_MJPG_PIC_WIDTH              32768
#define MAX_MJPG_PIC_HEIGHT             32768


#define MAX_FRAME                       (19*MAX_NUM_INSTANCE)

#define STREAM_FILL_SIZE                0x10000
#define STREAM_END_SIZE                 0

#define JPU_GBU_SIZE                    256

#define STREAM_BUF_SIZE                 0x200000

#define JPU_CHECK_WRITE_RESPONSE_BVALID_SIGNAL 0

#define JPU_INTERRUPT_TIMEOUT_MS        (5000*4)

#define JPU_INST_CTRL_TIMEOUT_MS        (5000*4)
#endif    /* _JPU_CONFIG_H_ */
