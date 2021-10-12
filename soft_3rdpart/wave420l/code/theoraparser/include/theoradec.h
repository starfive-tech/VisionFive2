/********************************************************************
Copyright (C) 2002-2009 Xiph.org Foundation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

/**\file
* The <tt>libtheoradec</tt> C decoding API.*/

#if !defined(_O_THEORA_THEORADEC_H_)
# define _O_THEORA_THEORADEC_H_ (1)
# include <stddef.h>
# include "codec.h"
# include "decint.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**The decoder context.*/
typedef struct th_dec_ctx    th_dec_ctx;
typedef struct th_setup_info th_setup_info;
typedef struct tho_ctx       tho_ctx;


struct tho_ctx
{
    th_info           ti;
	th_comment        tc;
	th_setup_info    *ts;
	th_dec_ctx       *td;

    int32_t frame_skip_mode;
};

extern th_dec_ctx *th_decode_alloc(const th_info *_info, const th_setup_info *_setup);
extern void        th_setupsys_mem_free(th_setup_info *_setup);
extern void        th_decodesys_mem_free(th_dec_ctx *_dec);


extern int32_t         th_decode_headerin(th_info *_info,th_comment *_tc, th_setup_info **_setup, 
                                      unsigned char *packet, int32_t packet_size);
extern int32_t         th_decode_packetin(th_dec_ctx *_dec,unsigned char *packet, 
                                      int32_t packet_size, int64_t *_granpos);


extern int32_t   theora_parser_open(void *handle, unsigned char *header, int32_t header_size, int32_t* scaleInfo);
extern int32_t   theora_parser_end(void *handle);
/*       refer to 6.2.3.1 Stream Parsing in Programmer User Guide */
extern int32_t   theora_parser_frame(void *handle, unsigned char * chunkData, 
                                 int32_t chunkSize);



#if defined(__cplusplus)
}
#endif

#endif
