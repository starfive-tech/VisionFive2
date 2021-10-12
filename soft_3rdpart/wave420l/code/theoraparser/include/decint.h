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

#include <limits.h>
#if !defined(_decint_H)
# define _decint_H (1)
# include "theoradec.h"
# include "state.h"
# include "bitpack.h"
# include "huffdec.h"
# include "dequant.h"

typedef struct th_setup_info         oc_setup_info;
typedef struct th_dec_ctx            oc_dec_ctx;


/*Next packet to read: Data packet.*/
#define OC_PACKET_DATA (0)


struct th_setup_info
{
  /*The Huffman codes.*/
  int16_t   *huff_tables[TH_NHUFFMAN_TABLES];
  /*The quantization parameters.*/
  th_quant_info  qinfo;
};

struct th_dec_ctx
{
  /*Shared encoder/decoder state.*/
  oc_theora_state        state;
  /*Whether or not packets are ready to be emitted.
    This takes on negative values while there are remaining header packets to
     be emitted, reaches 0 when the codec is ready for input, and goes to 1
     when a frame has been processed and a data packet is ready.*/
  int32_t                    packet_state;
  /*Buffer in which to assemble packets.*/
  oc_pack_buf            opb;
  /*Huffman decode trees.*/
  int16_t               *huff_tables[TH_NHUFFMAN_TABLES];
  /*The index of the first token in each plane for each coefficient.*/
  int32_t                    ti0[3][64];
  /*The number of outstanding EOB runs at the start of each coefficient in each
     plane.*/
  int32_t                    eob_runs[3][64];
  /*The DCT token lists.*/
  unsigned char         *dct_tokens;
  /*The extra bits associated with DCT tokens.*/
  unsigned char         *extra_bits;
  /*The number of dct tokens unpacked so far.*/
  int32_t                    dct_tokens_count;
  /*The DC scale used for out-of-loop deblocking.*/
  int32_t                    pp_dc_scale[64];
  /*The sharpen modifier used for out-of-loop deringing.*/
  int32_t                    pp_sharp_mod[64];
};


#endif
