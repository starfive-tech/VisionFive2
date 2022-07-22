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

#ifndef __PBU_H__
#define __PBU_H__

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "wave511/vpuapi/vputypes.h"
#else
    #include <vputypes.h>
#endif


typedef void* spp_enc_context;

#ifdef __cplusplus
extern "C" {
#endif

	spp_enc_context spp_enc_init(Uint8 *buffer, int buffer_size, int enableEPB);
	void spp_enc_deinit(spp_enc_context context);
	void spp_enc_init_rbsp(spp_enc_context context);

	void spp_enc_put_nal_byte(spp_enc_context context, Uint32 var, int n);
	void spp_enc_put_bits(spp_enc_context context, Uint32 var, int n);

	void spp_enc_flush(spp_enc_context context);
	
	void spp_enc_put_ue(spp_enc_context context, Uint32 var);
	Uint32 spp_enc_get_ue_bit_size(Uint32 var);
	void spp_enc_put_se(spp_enc_context context, Int32 var);
	void spp_enc_put_byte_align(spp_enc_context context, int has_stop_bit);

	Uint32 spp_enc_get_wbuf_remain(spp_enc_context context);
	Uint32 spp_enc_get_rbsp_bit(spp_enc_context context);
	Uint32 spp_enc_get_nal_cnt(spp_enc_context context);
	Uint8* spp_enc_get_wr_ptr(spp_enc_context context);
	Uint8* spp_enc_get_wr_ptr_only(spp_enc_context context);

	Uint32 spp_enc_get_est_nal_cnt(spp_enc_context context);

#ifdef __cplusplus
}
#endif

#endif

