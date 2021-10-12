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
#if !defined(_bitpack_H)
# define _bitpack_H (1)
# include <stddef.h>
# include <limits.h>
#include "stdint.h"
#include "codec.h"


typedef uint16_t   oc_quant_table[64];
typedef size_t     oc_pb_window;

typedef struct oc_pack_buf oc_pack_buf;


/*Custom bitpacker implementations.*/
# if defined(OC_ARM_ASM)
#  include "arm/armbits.h"
# endif

# if !defined(oc_pack_read)
#  define oc_pack_read oc_pack_read_c
# endif
# if !defined(oc_pack_read1)
#  define oc_pack_read1 oc_pack_read1_c
# endif
# if !defined(oc_huff_token_decode)
#  define oc_huff_token_decode oc_huff_token_decode_c
# endif

/*Maximum scaled quantizer value.*/
#define OC_QUANT_MAX          (1024<<2)
#define OC_PB_WINDOW_SIZE ((int32_t)sizeof(oc_pb_window)*CHAR_BIT)
#define OC_LOTS_OF_BITS (0x40000000)
#define OC_DCT_VAL_RANGE         (580)

#define OC_NDCT_TOKEN_BITS       (5)
#define OC_DCT_EOB1_TOKEN        (0)
#define OC_DCT_EOB2_TOKEN        (1)
#define OC_DCT_EOB3_TOKEN        (2)
#define OC_DCT_REPEAT_RUN0_TOKEN (3)
#define OC_DCT_REPEAT_RUN1_TOKEN (4)
#define OC_DCT_REPEAT_RUN2_TOKEN (5)
#define OC_DCT_REPEAT_RUN3_TOKEN (6)

#define OC_DCT_SHORT_ZRL_TOKEN   (7)
#define OC_DCT_ZRL_TOKEN         (8)

#define OC_ONE_TOKEN             (9)
#define OC_MINUS_ONE_TOKEN       (10)
#define OC_TWO_TOKEN             (11)
#define OC_MINUS_TWO_TOKEN       (12)

#define OC_DCT_VAL_CAT2          (13)
#define OC_DCT_VAL_CAT3          (17)
#define OC_DCT_VAL_CAT4          (18)
#define OC_DCT_VAL_CAT5          (19)
#define OC_DCT_VAL_CAT6          (20)
#define OC_DCT_VAL_CAT7          (21)
#define OC_DCT_VAL_CAT8          (22)

#define OC_DCT_RUN_CAT1A         (23)
#define OC_DCT_RUN_CAT1B         (28)
#define OC_DCT_RUN_CAT1C         (29)
#define OC_DCT_RUN_CAT2A         (30)
#define OC_DCT_RUN_CAT2B         (31)

#define OC_NDCT_EOB_TOKEN_MAX    (7)
#define OC_NDCT_ZRL_TOKEN_MAX    (9)
#define OC_NDCT_VAL_MAX          (23)
#define OC_NDCT_VAL_CAT1_MAX     (13)
#define OC_NDCT_VAL_CAT2_MAX     (17)
#define OC_NDCT_VAL_CAT2_SIZE    (OC_NDCT_VAL_CAT2_MAX-OC_DCT_VAL_CAT2)
#define OC_NDCT_RUN_MAX          (32)
#define OC_NDCT_RUN_CAT1A_MAX    (28)


#define TH_NHUFFMAN_TABLES (80)
#define TH_NDCT_TOKENS     (32)

struct oc_pack_buf
{
    const unsigned char *stop;
    const unsigned char *ptr;
    oc_pb_window         window;
    int32_t                  bits;
    int32_t                  eof;
};

/**A Huffman code for a Theora DCT token.
* Each set of Huffman codes in a given table must form a complete, prefix-free
*  code.
* There is no requirement that all the tokens in a table have a valid code,
*  but the current encoder is not optimized to take advantage of this.
* If each of the five grouops of 16 tables does not contain at least one table
*  with a code for every token, then the encoder may fail to encode certain
*  frames.
* The complete table in the first group of 16 does not have to be in the same
*  place as the complete table in the other groups, but the complete tables in
*  the remaining four groups must all be in the same place.*/
typedef struct
{
    /**The bit pattern for the code, with the LSbit of the pattern aligned in
    *   the LSbit of the word.*/
    uint32_t pattern;
    /**The number of bits in the code.
    * This must be between 0 and 32, inclusive.*/
    int32_t          nbits;
}th_huff_code;


extern const unsigned char OC_DCT_TOKEN_EXTRA_BITS[TH_NDCT_TOKENS];

//   Huffman decoder
int32_t  oc_huff_trees_unpack(oc_pack_buf *_opb,int16_t *_nodes[TH_NHUFFMAN_TABLES]);
int32_t  oc_huff_trees_copy(int16_t *_dst[TH_NHUFFMAN_TABLES], const int16_t *const _src[TH_NHUFFMAN_TABLES]);
void oc_huff_trees_clear(int16_t *_nodes[TH_NHUFFMAN_TABLES]);
int32_t  oc_huff_token_decode_c(oc_pack_buf *_opb,const int16_t *_node);

//   pack read
void oc_pack_readinit(oc_pack_buf *_b,unsigned char *_buf,long _bytes);
int32_t  oc_pack_look1(oc_pack_buf *_b);
void oc_pack_adv1(oc_pack_buf *_b);
long oc_pack_read_c(oc_pack_buf *_b,int32_t _bits);
int32_t  oc_pack_read1_c(oc_pack_buf *_b);
long oc_pack_bytes_left(oc_pack_buf *_b);


void oc_dequant_tables_init(uint16_t *_dequant[64][3][2], int32_t _pp_dc_scale[64],const th_quant_info *_qinfo);
int32_t  oc_quant_params_unpack(oc_pack_buf *_opb, th_quant_info *_qinfo);
void oc_quant_params_clear(th_quant_info *_qinfo);

#endif
