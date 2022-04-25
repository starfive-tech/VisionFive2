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

#include <stdlib.h>
#include <string.h>
#include "decint.h"
#include "ocintrin.h"



/*The mode alphabets for the various mode coding schemes.
Scheme 0 uses a custom alphabet, which is not stored in this table.*/
static const unsigned char OC_MODE_ALPHABETS[7][OC_NMODES]={
    /*Last MV dominates */
    {
        OC_MODE_INTER_MV_LAST,OC_MODE_INTER_MV_LAST2,OC_MODE_INTER_MV,
            OC_MODE_INTER_NOMV,OC_MODE_INTRA,OC_MODE_GOLDEN_NOMV,OC_MODE_GOLDEN_MV,
            OC_MODE_INTER_MV_FOUR
    },
    {
        OC_MODE_INTER_MV_LAST,OC_MODE_INTER_MV_LAST2,OC_MODE_INTER_NOMV,
            OC_MODE_INTER_MV,OC_MODE_INTRA,OC_MODE_GOLDEN_NOMV,OC_MODE_GOLDEN_MV,
            OC_MODE_INTER_MV_FOUR
        },
        {
            OC_MODE_INTER_MV_LAST,OC_MODE_INTER_MV,OC_MODE_INTER_MV_LAST2,
                OC_MODE_INTER_NOMV,OC_MODE_INTRA,OC_MODE_GOLDEN_NOMV,OC_MODE_GOLDEN_MV,
                OC_MODE_INTER_MV_FOUR
        },
        {
            OC_MODE_INTER_MV_LAST,OC_MODE_INTER_MV,OC_MODE_INTER_NOMV,
                OC_MODE_INTER_MV_LAST2,OC_MODE_INTRA,OC_MODE_GOLDEN_NOMV,
                OC_MODE_GOLDEN_MV,OC_MODE_INTER_MV_FOUR
            },
            /*No MV dominates.*/
            {
                OC_MODE_INTER_NOMV,OC_MODE_INTER_MV_LAST,OC_MODE_INTER_MV_LAST2,
                    OC_MODE_INTER_MV,OC_MODE_INTRA,OC_MODE_GOLDEN_NOMV,OC_MODE_GOLDEN_MV,
                    OC_MODE_INTER_MV_FOUR
            },
            {
                OC_MODE_INTER_NOMV,OC_MODE_GOLDEN_NOMV,OC_MODE_INTER_MV_LAST,
                    OC_MODE_INTER_MV_LAST2,OC_MODE_INTER_MV,OC_MODE_INTRA,OC_MODE_GOLDEN_MV,
                    OC_MODE_INTER_MV_FOUR
                },
                /*Default ordering.*/
                {
                    OC_MODE_INTER_NOMV,OC_MODE_INTRA,OC_MODE_INTER_MV,OC_MODE_INTER_MV_LAST,
                        OC_MODE_INTER_MV_LAST2,OC_MODE_GOLDEN_NOMV,OC_MODE_GOLDEN_MV,
                        OC_MODE_INTER_MV_FOUR
                }
};


/*The original DCT tokens are extended and reordered during the construction of
the Huffman tables.
The extension means more bits can be read with fewer calls to the bitpacker
during the Huffman decoding process (at the cost of larger Huffman tables),
and fewer tokens require additional extra bits (reducing the average storage
per decoded token).
The revised ordering reveals essential information in the token value
itself; specifically, whether or not there are additional extra bits to read
and the parameter to which those extra bits are applied.
The token is used to fetch a code word from the OC_DCT_CODE_WORD table below.
The extra bits are added into code word at the bit position inferred from the
token value, giving the final code word from which all required parameters
are derived.
The number of EOBs and the leading zero run length can be extracted directly.
The coefficient magnitude is optionally negated before extraction, according
to a 'flip' bit.*/

/*The number of additional extra bits that are decoded with each of the
internal DCT tokens.*/
static const unsigned char OC_INTERNAL_DCT_TOKEN_EXTRA_BITS[15]={
    12,4,3,3,4,4,5,5,8,8,8,8,3,3,6
};

/*Whether or not an internal token needs any additional extra bits.*/
#define OC_DCT_TOKEN_NEEDS_MORE(token) \
    (token<(sizeof(OC_INTERNAL_DCT_TOKEN_EXTRA_BITS)/ \
    sizeof(*OC_INTERNAL_DCT_TOKEN_EXTRA_BITS)))

/*This token (OC_DCT_REPEAT_RUN3_TOKEN) requires more than 8 extra bits.*/
#define OC_DCT_TOKEN_FAT_EOB (0)

/*The number of EOBs to use for an end-of-frame token.
Note: We want to set eobs to PTRDIFF_MAX here, but that requires C99, which
is not yet available everywhere; this should be equivalent.*/
#define OC_DCT_EOB_FINISH (~0>>1)

/*The location of the (6) run length bits in the code word.
These are placed at index 0 and given 8 bits (even though 6 would suffice)
because it may be faster to extract the lower byte on some platforms.*/
#define OC_DCT_CW_RLEN_SHIFT (0)
/*The location of the (12) EOB bits in the code word.*/
#define OC_DCT_CW_EOB_SHIFT  (8)
/*The location of the (1) flip bit in the code word.
This must be right under the magnitude bits.*/
#define OC_DCT_CW_FLIP_BIT   (20)
/*The location of the (11) token magnitude bits in the code word.
These must be last, and rely on a sign-extending right shift.*/
#define OC_DCT_CW_MAG_SHIFT  (21)

/*Pack the given fields into a code word.*/
#define OC_DCT_CW_PACK(_eobs,_rlen,_mag,_flip) \
    ((_eobs)<<OC_DCT_CW_EOB_SHIFT| \
    (_rlen)<<OC_DCT_CW_RLEN_SHIFT| \
    (_flip)<<OC_DCT_CW_FLIP_BIT| \
    (_mag)-(_flip)<<OC_DCT_CW_MAG_SHIFT)

/*A special code word value that signals the end of the frame (a long EOB run
of zero).*/
#define OC_DCT_CW_FINISH (0)

/*The position at which to insert the extra bits in the code word.
We use this formulation because Intel has no useful cmov.
A real architecture would probably do better with two of those.
This translates to 11 instructions(!), and is _still_ faster than either a
table lookup (just barely) or the naive double-ternary implementation (which
gcc translates to a jump and a cmov).
This assumes OC_DCT_CW_RLEN_SHIFT is zero, but could easily be reworked if
you want to make one of the other shifts zero.*/
#define OC_DCT_TOKEN_EB_POS(_token) \
    ((OC_DCT_CW_EOB_SHIFT-OC_DCT_CW_MAG_SHIFT&-((_token)<2)) \
    +(OC_DCT_CW_MAG_SHIFT&-((_token)<12)))

/*The code words for each internal token.
See the notes at OC_DCT_TOKEN_MAP for the reasons why things are out of
order.*/
static const int32_t OC_DCT_CODE_WORD[92]={
    /*These tokens require additional extra bits for the EOB count.*/
    /*OC_DCT_REPEAT_RUN3_TOKEN (12 extra bits)*/
    OC_DCT_CW_FINISH,
    /*OC_DCT_REPEAT_RUN2_TOKEN (4 extra bits)*/
    OC_DCT_CW_PACK(16, 0,  0,0),
    /*These tokens require additional extra bits for the magnitude.*/
    /*OC_DCT_VAL_CAT5 (4 extra bits-1 already read)*/
    OC_DCT_CW_PACK( 0, 0, 13,0),
    OC_DCT_CW_PACK( 0, 0, 13,1),
    /*OC_DCT_VAL_CAT6 (5 extra bits-1 already read)*/
    OC_DCT_CW_PACK( 0, 0, 21,0),
    OC_DCT_CW_PACK( 0, 0, 21,1),
    /*OC_DCT_VAL_CAT7 (6 extra bits-1 already read)*/
    OC_DCT_CW_PACK( 0, 0, 37,0),
    OC_DCT_CW_PACK( 0, 0, 37,1),
    /*OC_DCT_VAL_CAT8 (10 extra bits-2 already read)*/
    OC_DCT_CW_PACK( 0, 0, 69,0),
    OC_DCT_CW_PACK( 0, 0,325,0),
    OC_DCT_CW_PACK( 0, 0, 69,1),
    OC_DCT_CW_PACK( 0, 0,325,1),
    /*These tokens require additional extra bits for the run length.*/
    /*OC_DCT_RUN_CAT1C (4 extra bits-1 already read)*/
    OC_DCT_CW_PACK( 0,10, +1,0),
    OC_DCT_CW_PACK( 0,10, -1,0),
    /*OC_DCT_ZRL_TOKEN (6 extra bits)
    Flip is set to distinguish this from OC_DCT_CW_FINISH.*/
    OC_DCT_CW_PACK( 0, 0,  0,1),
    /*The remaining tokens require no additional extra bits.*/
    /*OC_DCT_EOB1_TOKEN (0 extra bits)*/
    OC_DCT_CW_PACK( 1, 0,  0,0),
    /*OC_DCT_EOB2_TOKEN (0 extra bits)*/
    OC_DCT_CW_PACK( 2, 0,  0,0),
    /*OC_DCT_EOB3_TOKEN (0 extra bits)*/
    OC_DCT_CW_PACK( 3, 0,  0,0),
    /*OC_DCT_RUN_CAT1A (1 extra bit-1 already read)x5*/
    OC_DCT_CW_PACK( 0, 1, +1,0),
    OC_DCT_CW_PACK( 0, 1, -1,0),
    OC_DCT_CW_PACK( 0, 2, +1,0),
    OC_DCT_CW_PACK( 0, 2, -1,0),
    OC_DCT_CW_PACK( 0, 3, +1,0),
    OC_DCT_CW_PACK( 0, 3, -1,0),
    OC_DCT_CW_PACK( 0, 4, +1,0),
    OC_DCT_CW_PACK( 0, 4, -1,0),
    OC_DCT_CW_PACK( 0, 5, +1,0),
    OC_DCT_CW_PACK( 0, 5, -1,0),
    /*OC_DCT_RUN_CAT2A (2 extra bits-2 already read)*/
    OC_DCT_CW_PACK( 0, 1, +2,0),
    OC_DCT_CW_PACK( 0, 1, +3,0),
    OC_DCT_CW_PACK( 0, 1, -2,0),
    OC_DCT_CW_PACK( 0, 1, -3,0),
    /*OC_DCT_RUN_CAT1B (3 extra bits-3 already read)*/
    OC_DCT_CW_PACK( 0, 6, +1,0),
    OC_DCT_CW_PACK( 0, 7, +1,0),
    OC_DCT_CW_PACK( 0, 8, +1,0),
    OC_DCT_CW_PACK( 0, 9, +1,0),
    OC_DCT_CW_PACK( 0, 6, -1,0),
    OC_DCT_CW_PACK( 0, 7, -1,0),
    OC_DCT_CW_PACK( 0, 8, -1,0),
    OC_DCT_CW_PACK( 0, 9, -1,0),
    /*OC_DCT_RUN_CAT2B (3 extra bits-3 already read)*/
    OC_DCT_CW_PACK( 0, 2, +2,0),
    OC_DCT_CW_PACK( 0, 3, +2,0),
    OC_DCT_CW_PACK( 0, 2, +3,0),
    OC_DCT_CW_PACK( 0, 3, +3,0),
    OC_DCT_CW_PACK( 0, 2, -2,0),
    OC_DCT_CW_PACK( 0, 3, -2,0),
    OC_DCT_CW_PACK( 0, 2, -3,0),
    OC_DCT_CW_PACK( 0, 3, -3,0),
    /*OC_DCT_SHORT_ZRL_TOKEN (3 extra bits-3 already read)
    Flip is set on the first one to distinguish it from OC_DCT_CW_FINISH.*/
    OC_DCT_CW_PACK( 0, 0,  0,1),
    OC_DCT_CW_PACK( 0, 1,  0,0),
    OC_DCT_CW_PACK( 0, 2,  0,0),
    OC_DCT_CW_PACK( 0, 3,  0,0),
    OC_DCT_CW_PACK( 0, 4,  0,0),
    OC_DCT_CW_PACK( 0, 5,  0,0),
    OC_DCT_CW_PACK( 0, 6,  0,0),
    OC_DCT_CW_PACK( 0, 7,  0,0),
    /*OC_ONE_TOKEN (0 extra bits)*/
    OC_DCT_CW_PACK( 0, 0, +1,0),
    /*OC_MINUS_ONE_TOKEN (0 extra bits)*/
    OC_DCT_CW_PACK( 0, 0, -1,0),
    /*OC_TWO_TOKEN (0 extra bits)*/
    OC_DCT_CW_PACK( 0, 0, +2,0),
    /*OC_MINUS_TWO_TOKEN (0 extra bits)*/
    OC_DCT_CW_PACK( 0, 0, -2,0),
    /*OC_DCT_VAL_CAT2 (1 extra bit-1 already read)x4*/
    OC_DCT_CW_PACK( 0, 0, +3,0),
    OC_DCT_CW_PACK( 0, 0, -3,0),
    OC_DCT_CW_PACK( 0, 0, +4,0),
    OC_DCT_CW_PACK( 0, 0, -4,0),
    OC_DCT_CW_PACK( 0, 0, +5,0),
    OC_DCT_CW_PACK( 0, 0, -5,0),
    OC_DCT_CW_PACK( 0, 0, +6,0),
    OC_DCT_CW_PACK( 0, 0, -6,0),
    /*OC_DCT_VAL_CAT3 (2 extra bits-2 already read)*/
    OC_DCT_CW_PACK( 0, 0, +7,0),
    OC_DCT_CW_PACK( 0, 0, +8,0),
    OC_DCT_CW_PACK( 0, 0, -7,0),
    OC_DCT_CW_PACK( 0, 0, -8,0),
    /*OC_DCT_VAL_CAT4 (3 extra bits-3 already read)*/
    OC_DCT_CW_PACK( 0, 0, +9,0),
    OC_DCT_CW_PACK( 0, 0,+10,0),
    OC_DCT_CW_PACK( 0, 0,+11,0),
    OC_DCT_CW_PACK( 0, 0,+12,0),
    OC_DCT_CW_PACK( 0, 0, -9,0),
    OC_DCT_CW_PACK( 0, 0,-10,0),
    OC_DCT_CW_PACK( 0, 0,-11,0),
    OC_DCT_CW_PACK( 0, 0,-12,0),
    /*OC_DCT_REPEAT_RUN1_TOKEN (3 extra bits-3 already read)*/
    OC_DCT_CW_PACK( 8, 0,  0,0),
    OC_DCT_CW_PACK( 9, 0,  0,0),
    OC_DCT_CW_PACK(10, 0,  0,0),
    OC_DCT_CW_PACK(11, 0,  0,0),
    OC_DCT_CW_PACK(12, 0,  0,0),
    OC_DCT_CW_PACK(13, 0,  0,0),
    OC_DCT_CW_PACK(14, 0,  0,0),
    OC_DCT_CW_PACK(15, 0,  0,0),
    /*OC_DCT_REPEAT_RUN0_TOKEN (2 extra bits-2 already read)*/
    OC_DCT_CW_PACK( 4, 0,  0,0),
    OC_DCT_CW_PACK( 5, 0,  0,0),
    OC_DCT_CW_PACK( 6, 0,  0,0),
    OC_DCT_CW_PACK( 7, 0,  0,0),
};



static int32_t oc_sb_run_unpack(oc_pack_buf *_opb)
{
    /*Coding scheme:
    Codeword            Run Length
    0                       1
    10x                     2-3
    110x                    4-5
    1110xx                  6-9
    11110xxx                10-17
    111110xxxx              18-33
    111111xxxxxxxxxxxx      34-4129*/
    static const int16_t OC_SB_RUN_TREE[22]={
        4,
        -(1<<8|1),-(1<<8|1),-(1<<8|1),-(1<<8|1),
        -(1<<8|1),-(1<<8|1),-(1<<8|1),-(1<<8|1),
        -(3<<8|2),-(3<<8|2),-(3<<8|3),-(3<<8|3),
        -(4<<8|4),-(4<<8|5),-(4<<8|2<<4|6-6),17,
        2,
        -(2<<8|2<<4|10-6),-(2<<8|2<<4|14-6),-(2<<8|4<<4|18-6),-(2<<8|12<<4|34-6)
    };
    int32_t ret;
    ret=oc_huff_token_decode(_opb,OC_SB_RUN_TREE);
    if(ret>=0x10){
        int32_t offs;
        offs=ret&0x1F;
        ret=6+offs+(int32_t)oc_pack_read(_opb,ret-offs>>4);
    }
    return ret;
}

static int32_t oc_block_run_unpack(oc_pack_buf *_opb)
{
    /*Coding scheme:
    Codeword             Run Length
    0x                      1-2
    10x                     3-4
    110x                    5-6
    1110xx                  7-10
    11110xx                 11-14
    11111xxxx               15-30*/
    static const int16_t OC_BLOCK_RUN_TREE[61]={
        5,
        -(2<<8|1),-(2<<8|1),-(2<<8|1),-(2<<8|1),
        -(2<<8|1),-(2<<8|1),-(2<<8|1),-(2<<8|1),
        -(2<<8|2),-(2<<8|2),-(2<<8|2),-(2<<8|2),
        -(2<<8|2),-(2<<8|2),-(2<<8|2),-(2<<8|2),
        -(3<<8|3),-(3<<8|3),-(3<<8|3),-(3<<8|3),
        -(3<<8|4),-(3<<8|4),-(3<<8|4),-(3<<8|4),
        -(4<<8|5),-(4<<8|5),-(4<<8|6),-(4<<8|6),
        33,       36,       39,       44,
        1,-(1<<8|7),-(1<<8|8),
        1,-(1<<8|9),-(1<<8|10),
        2,-(2<<8|11),-(2<<8|12),-(2<<8|13),-(2<<8|14),
        4,
        -(4<<8|15),-(4<<8|16),-(4<<8|17),-(4<<8|18),
        -(4<<8|19),-(4<<8|20),-(4<<8|21),-(4<<8|22),
        -(4<<8|23),-(4<<8|24),-(4<<8|25),-(4<<8|26),
        -(4<<8|27),-(4<<8|28),-(4<<8|29),-(4<<8|30)
    };
    return oc_huff_token_decode(_opb,OC_BLOCK_RUN_TREE);
}



static int32_t oc_dec_init(oc_dec_ctx *_dec,const th_info *_info,
    const th_setup_info *_setup)
{
    int32_t qti;
    int32_t pli;
    int32_t qi;
    int32_t ret;
    ret=oc_state_init(&_dec->state,_info,3);
    if(ret<0)return ret;
    ret=oc_huff_trees_copy(_dec->huff_tables,
        (const int16_t *const *)_setup->huff_tables);
    if(ret<0){
        oc_state_clear(&_dec->state);
        return ret;
    }
    /*For each fragment, allocate one byte for every DCT coefficient token, plus
    one byte for extra-bits for each token, plus one more byte for the long
    EOB run, just in case it's the very last token and has a run length of
    one.*/
    _dec->dct_tokens=(unsigned char *)malloc((64+64+1)*
        _dec->state.nfrags*sizeof(_dec->dct_tokens[0]));
    if(_dec->dct_tokens==NULL){
        oc_huff_trees_clear(_dec->huff_tables);
        oc_state_clear(&_dec->state);
        return TH_EFAULT;
    }
    for(qi=0;qi<64;qi++)for(pli=0;pli<3;pli++)for(qti=0;qti<2;qti++){
        _dec->state.dequant_tables[qi][pli][qti]=
            _dec->state.dequant_table_data[qi][pli][qti];
    }
    oc_dequant_tables_init(_dec->state.dequant_tables,_dec->pp_dc_scale,
        &_setup->qinfo);
    for(qi=0;qi<64;qi++){
        int32_t qsum;
        qsum=0;
        for(qti=0;qti<2;qti++) {
		for(pli=0;pli<3;pli++) {
			qsum += _dec->state.dequant_tables[qi][pli][qti][12]+
			_dec->state.dequant_tables[qi][pli][qti][17]+
			_dec->state.dequant_tables[qi][pli][qti][18]+
			_dec->state.dequant_tables[qi][pli][qti][24]<<(pli==0);
		}
        }
       _dec->pp_sharp_mod[qi]=-(qsum>>11);
    }
    memcpy(_dec->state.loop_filter_limits,_setup->qinfo.loop_filter_limits,
        sizeof(_dec->state.loop_filter_limits));
    return 0;
}

static void oc_dec_clear(oc_dec_ctx *_dec)
{
    free(_dec->dct_tokens);
    oc_huff_trees_clear(_dec->huff_tables);
    oc_state_clear(&_dec->state);
}


static int32_t oc_dec_frame_header_unpack(oc_dec_ctx *_dec)
{
    long val;
    /*Check to make sure this is a data packet.*/
    val=oc_pack_read1(&_dec->opb);
    if(val!=0)return TH_EBADPACKET;
    /*Read in the frame type (I or P).*/
    val=oc_pack_read1(&_dec->opb);
    _dec->state.frame_type=(int32_t)val;
    /*Read in the qi list.*/
    val=oc_pack_read(&_dec->opb,6);
    _dec->state.qis[0]=(unsigned char)val;
    val=oc_pack_read1(&_dec->opb);
    if(!val)_dec->state.nqis=1;
    else{
        val=oc_pack_read(&_dec->opb,6);
        _dec->state.qis[1]=(unsigned char)val;
        val=oc_pack_read1(&_dec->opb);
        if(!val)_dec->state.nqis=2;
        else{
            val=oc_pack_read(&_dec->opb,6);
            _dec->state.qis[2]=(unsigned char)val;
            _dec->state.nqis=3;
        }
    }
    if(_dec->state.frame_type==OC_INTRA_FRAME){
        /*Keyframes have 3 unused configuration bits, holdovers from VP3 days.
        Most of the other unused bits in the VP3 headers were eliminated.
        I don't know why these remain.*/
        /*I wanted to eliminate wasted bits, but not all config wiggle room
        --Monty.*/
        val=oc_pack_read(&_dec->opb,3);
        if(val!=0)return TH_EIMPL;
    }
    return 0;
}

/*Mark all fragments as coded and in OC_MODE_INTRA.
This also builds up the coded fragment list (in coded order), and clears the
uncoded fragment list.
It does not update the coded macro block list nor the super block flags, as
those are not used when decoding INTRA frames.*/
static void oc_dec_mark_all_intra(oc_dec_ctx *_dec)
{
    const oc_sb_map   *sb_maps;
    const oc_sb_flags *sb_flags;
    oc_fragment       *frags;
    int32_t           *coded_fragis;
    int32_t           ncoded_fragis;
    int32_t           prev_ncoded_fragis;
    uint32_t          nsbs;
    uint32_t          sbi;
    int32_t           pli;
    coded_fragis=_dec->state.coded_fragis;
    prev_ncoded_fragis=ncoded_fragis=0;
    sb_maps=(const oc_sb_map *)_dec->state.sb_maps;
    sb_flags=_dec->state.sb_flags;
    frags=_dec->state.frags;
    sbi=nsbs=0;
    for(pli=0;pli<3;pli++)
    {
        nsbs+=_dec->state.fplanes[pli].nsbs;
        for(;sbi<nsbs;sbi++){
            int32_t quadi;
            for(quadi=0;quadi<4;quadi++)if(sb_flags[sbi].quad_valid&1<<quadi){
                int32_t bi;
                for(bi=0;bi<4;bi++){
                    int32_t fragi;
                    fragi=sb_maps[sbi][quadi][bi];
                    if(fragi>=0){
                        frags[fragi].coded=1;
                        frags[fragi].mb_mode=OC_MODE_INTRA;
                        coded_fragis[ncoded_fragis++]=fragi;
                    }
                }
            }
        }
        _dec->state.ncoded_fragis[pli]=ncoded_fragis-prev_ncoded_fragis;
        prev_ncoded_fragis=ncoded_fragis;
    }
    _dec->state.ntotal_coded_fragis=ncoded_fragis;
}

/*Decodes the bit flags indicating whether each super block is partially coded
or not.
Return: The number of partially coded super blocks.*/
static unsigned oc_dec_partial_sb_flags_unpack(oc_dec_ctx *_dec)
{
    oc_sb_flags *sb_flags;
    uint32_t     nsbs;
    uint32_t     sbi;
    uint32_t     npartial;
    uint32_t     run_count;
    long         val;
    int32_t          flag;
    val=oc_pack_read1(&_dec->opb);
    flag=(int32_t)val;
    sb_flags=_dec->state.sb_flags;
    nsbs=_dec->state.nsbs;
    sbi=npartial=0;
    while(sbi<nsbs){
        int32_t full_run;
        run_count=oc_sb_run_unpack(&_dec->opb);
        full_run=run_count>=4129;
        do{
            sb_flags[sbi].coded_partially=flag;
            sb_flags[sbi].coded_fully=0;
            npartial+=flag;
            sbi++;
        }
        while(--run_count>0&&sbi<nsbs);
        if(full_run&&sbi<nsbs){
            val=oc_pack_read1(&_dec->opb);
            flag=(int32_t)val;
        }
        else flag=!flag;
    }
    /*TODO: run_count should be 0 here.
    If it's not, we should issue a warning of some kind.*/
    return npartial;
}

/*Decodes the bit flags for whether or not each non-partially-coded super
block is fully coded or not.
This function should only be called if there is at least one
non-partially-coded super block.
Return: The number of partially coded super blocks.*/
static void oc_dec_coded_sb_flags_unpack(oc_dec_ctx *_dec)
{
    oc_sb_flags *sb_flags;
    uint32_t     nsbs;
    uint32_t     sbi;
    uint32_t     run_count;
    long         val;
    int32_t          flag;
    sb_flags=_dec->state.sb_flags;
    nsbs=_dec->state.nsbs;
    /*Skip partially coded super blocks.*/
    for(sbi=0;sb_flags[sbi].coded_partially;sbi++);
    val=oc_pack_read1(&_dec->opb);
    flag=(int32_t)val;
    do{
        int32_t full_run;
        run_count=oc_sb_run_unpack(&_dec->opb);
        full_run=run_count>=4129;
        for(;sbi<nsbs;sbi++){
            if(sb_flags[sbi].coded_partially)continue;
            if(run_count--<=0)break;
            sb_flags[sbi].coded_fully=flag;
        }
        if(full_run&&sbi<nsbs){
            val=oc_pack_read1(&_dec->opb);
            flag=(int32_t)val;
        }
        else flag=!flag;
    }
    while(sbi<nsbs);
    /*TODO: run_count should be 0 here.
    If it's not, we should issue a warning of some kind.*/
}

static void oc_dec_coded_flags_unpack(oc_dec_ctx *_dec)
{
    const oc_sb_map   *sb_maps;
    const oc_sb_flags *sb_flags;
    oc_fragment       *frags;
    uint32_t           nsbs;
    uint32_t           sbi;
    uint32_t           npartial;
    long               val;
    int32_t                pli;
    int32_t                flag;
    int32_t                run_count;
    int32_t               *coded_fragis;
    int32_t               *uncoded_fragis;
    int32_t                ncoded_fragis;
    int32_t                nuncoded_fragis;
    int32_t                prev_ncoded_fragis;

    npartial=oc_dec_partial_sb_flags_unpack(_dec);
    if(npartial<_dec->state.nsbs)oc_dec_coded_sb_flags_unpack(_dec);
    if(npartial>0){
        val=oc_pack_read1(&_dec->opb);
        flag=!(int32_t)val;
    }
    else flag=0;
    sb_maps=(const oc_sb_map *)_dec->state.sb_maps;
    sb_flags=_dec->state.sb_flags;
    frags=_dec->state.frags;
    sbi=nsbs=run_count=0;
    coded_fragis=_dec->state.coded_fragis;
    uncoded_fragis=coded_fragis+_dec->state.nfrags;
    prev_ncoded_fragis=ncoded_fragis=nuncoded_fragis=0;
    for(pli=0;pli<3;pli++)
    {
        nsbs+=_dec->state.fplanes[pli].nsbs;
        for(;sbi<nsbs;sbi++)
        {
            int32_t quadi;
            for(quadi=0;quadi<4;quadi++)if(sb_flags[sbi].quad_valid&1<<quadi)
            {
                int32_t bi;
                for(bi=0;bi<4;bi++)
                {
                    int32_t fragi;
                    fragi=sb_maps[sbi][quadi][bi];
                    if(fragi>=0){
                        int32_t coded;
                        if(sb_flags[sbi].coded_fully)coded=1;
                        else if(!sb_flags[sbi].coded_partially)coded=0;
                        else{
                            if(run_count<=0){
                                run_count=oc_block_run_unpack(&_dec->opb);
                                flag=!flag;
                            }
                            run_count--;
                            coded=flag;
                        }
                        if(coded)coded_fragis[ncoded_fragis++]=fragi;
                        else *(uncoded_fragis-++nuncoded_fragis)=fragi;
                        frags[fragi].coded=coded;
                        frags[fragi].mb_mode = 0;
                    }
                }
            }
        }
        _dec->state.ncoded_fragis[pli]=ncoded_fragis-prev_ncoded_fragis;
        prev_ncoded_fragis=ncoded_fragis;
    }
    _dec->state.ntotal_coded_fragis=ncoded_fragis;
    /*TODO: run_count should be 0 here.
    If it's not, we should issue a warning of some kind.*/
}


/*Coding scheme:
Codeword            Mode Index
0                       0
10                      1
110                     2
1110                    3
11110                   4
111110                  5
1111110                 6
1111111                 7*/
static const int16_t OC_VLC_MODE_TREE[26]={
    4,
    -(1<<8|0),-(1<<8|0),-(1<<8|0),-(1<<8|0),
    -(1<<8|0),-(1<<8|0),-(1<<8|0),-(1<<8|0),
    -(2<<8|1),-(2<<8|1),-(2<<8|1),-(2<<8|1),
    -(3<<8|2),-(3<<8|2),-(4<<8|3),17,
    3,
    -(1<<8|4),-(1<<8|4),-(1<<8|4),-(1<<8|4),
    -(2<<8|5),-(2<<8|5),-(3<<8|6),-(3<<8|7)
};

static const int16_t OC_CLC_MODE_TREE[9]={
    3,
    -(3<<8|0),-(3<<8|1),-(3<<8|2),-(3<<8|3),
    -(3<<8|4),-(3<<8|5),-(3<<8|6),-(3<<8|7)
};

/*Unpacks the list of macro block modes for INTER frames.*/
static void oc_dec_mb_modes_unpack(oc_dec_ctx *_dec)
{
    const oc_mb_map     *mb_maps;
    signed char         *mb_modes;
    const oc_fragment   *frags;
    const unsigned char *alphabet;
    unsigned char        scheme0_alphabet[8];
    const int16_t   *mode_tree;
    size_t               nmbs;
    size_t               mbi;
    long                 val;
    int32_t                  mode_scheme;
    val=oc_pack_read(&_dec->opb,3);
    mode_scheme=(int32_t)val;
    if(mode_scheme==0){
        int32_t mi;
        /*Just in case, initialize the modes to something.
        If the bitstream doesn't contain each index exactly once, it's likely
        corrupt and the rest of the packet is garbage anyway, but this way we
        won't crash, and we'll decode SOMETHING.*/
        /*LOOP VECTORIZES*/
        for(mi=0;mi<OC_NMODES;mi++)scheme0_alphabet[mi]=OC_MODE_INTER_NOMV;
        for(mi=0;mi<OC_NMODES;mi++){
            val=oc_pack_read(&_dec->opb,3);
            scheme0_alphabet[val]=OC_MODE_ALPHABETS[6][mi];
        }
        alphabet=scheme0_alphabet;
    }
    else alphabet=OC_MODE_ALPHABETS[mode_scheme-1];
    mode_tree=mode_scheme==7?OC_CLC_MODE_TREE:OC_VLC_MODE_TREE;
    mb_modes=_dec->state.mb_modes;
    mb_maps=(const oc_mb_map *)_dec->state.mb_maps;
    nmbs=_dec->state.nmbs;
    frags=_dec->state.frags;
    for(mbi=0;mbi<nmbs;mbi++){
        if(mb_modes[mbi]!=OC_MODE_INVALID){
            /*Check for a coded luma block in this macro block.*/
            if(frags[mb_maps[mbi][0][0]].coded
                ||frags[mb_maps[mbi][0][1]].coded
                ||frags[mb_maps[mbi][0][2]].coded
                ||frags[mb_maps[mbi][0][3]].coded){
                    /*We found one, decode a mode.*/
                    mb_modes[mbi]=alphabet[oc_huff_token_decode(&_dec->opb,mode_tree)];
            }
            /*There were none: INTER_NOMV is forced.*/
            else mb_modes[mbi]=OC_MODE_INTER_NOMV;
        }
    }
}



static const int16_t OC_VLC_MV_COMP_TREE[101]={
    5,
    -(3<<8|32+0),-(3<<8|32+0),-(3<<8|32+0),-(3<<8|32+0),
    -(3<<8|32+1),-(3<<8|32+1),-(3<<8|32+1),-(3<<8|32+1),
    -(3<<8|32-1),-(3<<8|32-1),-(3<<8|32-1),-(3<<8|32-1),
    -(4<<8|32+2),-(4<<8|32+2),-(4<<8|32-2),-(4<<8|32-2),
    -(4<<8|32+3),-(4<<8|32+3),-(4<<8|32-3),-(4<<8|32-3),
    33,          36,          39,          42,
    45,          50,          55,          60,
    65,          74,          83,          92,
    1,-(1<<8|32+4),-(1<<8|32-4),
    1,-(1<<8|32+5),-(1<<8|32-5),
    1,-(1<<8|32+6),-(1<<8|32-6),
    1,-(1<<8|32+7),-(1<<8|32-7),
    2,-(2<<8|32+8),-(2<<8|32-8),-(2<<8|32+9),-(2<<8|32-9),
    2,-(2<<8|32+10),-(2<<8|32-10),-(2<<8|32+11),-(2<<8|32-11),
    2,-(2<<8|32+12),-(2<<8|32-12),-(2<<8|32+13),-(2<<8|32-13),
    2,-(2<<8|32+14),-(2<<8|32-14),-(2<<8|32+15),-(2<<8|32-15),
    3,
    -(3<<8|32+16),-(3<<8|32-16),-(3<<8|32+17),-(3<<8|32-17),
    -(3<<8|32+18),-(3<<8|32-18),-(3<<8|32+19),-(3<<8|32-19),
    3,
    -(3<<8|32+20),-(3<<8|32-20),-(3<<8|32+21),-(3<<8|32-21),
    -(3<<8|32+22),-(3<<8|32-22),-(3<<8|32+23),-(3<<8|32-23),
    3,
    -(3<<8|32+24),-(3<<8|32-24),-(3<<8|32+25),-(3<<8|32-25),
    -(3<<8|32+26),-(3<<8|32-26),-(3<<8|32+27),-(3<<8|32-27),
    3,
    -(3<<8|32+28),-(3<<8|32-28),-(3<<8|32+29),-(3<<8|32-29),
    -(3<<8|32+30),-(3<<8|32-30),-(3<<8|32+31),-(3<<8|32-31)
};

static const int16_t OC_CLC_MV_COMP_TREE[65]={
    6,
    -(6<<8|32 +0),-(6<<8|32 -0),-(6<<8|32 +1),-(6<<8|32 -1),
    -(6<<8|32 +2),-(6<<8|32 -2),-(6<<8|32 +3),-(6<<8|32 -3),
    -(6<<8|32 +4),-(6<<8|32 -4),-(6<<8|32 +5),-(6<<8|32 -5),
    -(6<<8|32 +6),-(6<<8|32 -6),-(6<<8|32 +7),-(6<<8|32 -7),
    -(6<<8|32 +8),-(6<<8|32 -8),-(6<<8|32 +9),-(6<<8|32 -9),
    -(6<<8|32+10),-(6<<8|32-10),-(6<<8|32+11),-(6<<8|32-11),
    -(6<<8|32+12),-(6<<8|32-12),-(6<<8|32+13),-(6<<8|32-13),
    -(6<<8|32+14),-(6<<8|32-14),-(6<<8|32+15),-(6<<8|32-15),
    -(6<<8|32+16),-(6<<8|32-16),-(6<<8|32+17),-(6<<8|32-17),
    -(6<<8|32+18),-(6<<8|32-18),-(6<<8|32+19),-(6<<8|32-19),
    -(6<<8|32+20),-(6<<8|32-20),-(6<<8|32+21),-(6<<8|32-21),
    -(6<<8|32+22),-(6<<8|32-22),-(6<<8|32+23),-(6<<8|32-23),
    -(6<<8|32+24),-(6<<8|32-24),-(6<<8|32+25),-(6<<8|32-25),
    -(6<<8|32+26),-(6<<8|32-26),-(6<<8|32+27),-(6<<8|32-27),
    -(6<<8|32+28),-(6<<8|32-28),-(6<<8|32+29),-(6<<8|32-29),
    -(6<<8|32+30),-(6<<8|32-30),-(6<<8|32+31),-(6<<8|32-31)
};


static oc_mv oc_mv_unpack(oc_pack_buf *_opb,const int16_t *_tree)
{
    int32_t dx;
    int32_t dy;
    dx=oc_huff_token_decode(_opb,_tree)-32;
    dy=oc_huff_token_decode(_opb,_tree)-32;
    return OC_MV(dx,dy);
}

/*Unpacks the list of motion vectors for INTER frames, and propagtes the macro
block modes and motion vectors to the individual fragments.*/
static void oc_dec_mv_unpack_and_frag_modes_fill(oc_dec_ctx *_dec)
{
    const oc_mb_map        *mb_maps;
    const signed char      *mb_modes;
    oc_set_chroma_mvs_func  set_chroma_mvs;
    const int16_t      *mv_comp_tree;
    oc_fragment            *frags;
    oc_mv                  *frag_mvs;
    const unsigned char    *map_idxs;
    int32_t                     map_nidxs;
    oc_mv                   last_mv;
    oc_mv                   prior_mv;
    oc_mv                   cbmvs[4];
    size_t                  nmbs;
    size_t                  mbi;
    long                    val;
    set_chroma_mvs=OC_SET_CHROMA_MVS_TABLE[_dec->state.info.pixel_fmt];
    val=oc_pack_read1(&_dec->opb);
    mv_comp_tree=val?OC_CLC_MV_COMP_TREE:OC_VLC_MV_COMP_TREE;
    map_idxs=OC_MB_MAP_IDXS[_dec->state.info.pixel_fmt];
    map_nidxs=OC_MB_MAP_NIDXS[_dec->state.info.pixel_fmt];
    prior_mv=last_mv=0;
    frags=_dec->state.frags;
    frag_mvs=_dec->state.frag_mvs;
    mb_maps=(const oc_mb_map *)_dec->state.mb_maps;
    mb_modes=_dec->state.mb_modes;
    nmbs=_dec->state.nmbs;
    for(mbi=0;mbi<nmbs;mbi++)
    {
        int32_t          mb_mode;
        mb_mode=mb_modes[mbi];
        if(mb_mode!=OC_MODE_INVALID)
        {
            oc_mv        mbmv = 0;
            int32_t          fragi;
            int32_t          coded[13];
            int32_t          mb_fragi[13];
            int32_t          codedi;
            int32_t          ncoded;
            int32_t          mapi;
            int32_t          mapii;
            /*Search for at least one coded fragment.*/
            ncoded=mapii=0;
            do{
                mapi=map_idxs[mapii];
                fragi=mb_maps[mbi][mapi>>2][mapi&3];
                if(frags[fragi].coded)
                {
                    coded[ncoded]=mapi;
                    mb_fragi[ncoded++] = fragi;
                }
            }
            while(++mapii<map_nidxs);
            if(ncoded<=0)continue;
            switch(mb_mode){
            case OC_MODE_INTER_MV_FOUR:{
                oc_mv       lbmvs[4];
                int32_t         bi;
                /*Mark the tail of the list, so we don't accidentally go past it.*/
                coded[ncoded]=-1;
                for(bi=codedi=0;bi<4;bi++)
                {
                    if(coded[codedi]==bi)
                    {
                        fragi = mb_fragi[codedi];
                        frags[fragi].mb_mode=mb_mode;
                        lbmvs[bi]=oc_mv_unpack(&_dec->opb,mv_comp_tree);
                        frag_mvs[fragi]=lbmvs[bi];
                        codedi++;
                    }
                    else lbmvs[bi]=0;
                }
                if(codedi>0)
                {
                    prior_mv=last_mv;
                    last_mv=lbmvs[coded[codedi-1]];
                }
                if(codedi<ncoded)
                {
                    (*set_chroma_mvs)(cbmvs,lbmvs);
                    for(;codedi<ncoded;codedi++)
                    {
                        mapi=coded[codedi];
                        bi=mapi&3;
                        fragi = mb_fragi[codedi];
                        frags[fragi].mb_mode=mb_mode;
                        frag_mvs[fragi]=cbmvs[bi];
                    }
                }
            }break;
            case OC_MODE_INTER_MV:{
                prior_mv=last_mv;
                last_mv=mbmv=oc_mv_unpack(&_dec->opb,mv_comp_tree);
            }break;
            case OC_MODE_INTER_MV_LAST:mbmv=last_mv;break;
            case OC_MODE_INTER_MV_LAST2:{
                mbmv=prior_mv;
                prior_mv=last_mv;
                last_mv=mbmv;
                                        }break;
            case OC_MODE_GOLDEN_MV:{
                mbmv=oc_mv_unpack(&_dec->opb,mv_comp_tree);
                                   }break;
            default:mbmv=0;break;
            }
            /*4MV mode fills in the fragments itself.
            For all other modes we can use this common code.*/
            if(mb_mode!=OC_MODE_INTER_MV_FOUR)
            {
                for(codedi=0;codedi<ncoded;codedi++)
                {
                    fragi = mb_fragi[codedi];
                    frags[fragi].mb_mode=mb_mode;
                    frag_mvs[fragi]=mbmv;
                }
            }
        }
    }
}

static void oc_dec_block_qis_unpack(oc_dec_ctx *_dec)
{
    oc_fragment     *frags;
    const int32_t       *coded_fragis;
    int32_t              ncoded_fragis;
    int32_t              fragii;
    int32_t              fragi;
    ncoded_fragis=_dec->state.ntotal_coded_fragis;
    if(ncoded_fragis<=0)return;
    frags=_dec->state.frags;
    coded_fragis=_dec->state.coded_fragis;
    if(_dec->state.nqis==1){
        /*If this frame has only a single qi value, then just use it for all coded
        fragments.*/
        for(fragii=0;fragii<ncoded_fragis;fragii++){
            frags[coded_fragis[fragii]].qii=0;
        }
    }
    else{
        long val;
        int32_t  flag;
        int32_t  nqi1;
        int32_t  run_count;
        /*Otherwise, we decode a qi index for each fragment, using two passes of
        the same binary RLE scheme used for super-block coded bits.
        The first pass marks each fragment as having a qii of 0 or greater than
        0, and the second pass (if necessary), distinguishes between a qii of
        1 and 2.
        At first we just store the qii in the fragment.
        After all the qii's are decoded, we make a final pass to replace them
        with the corresponding qi's for this frame.*/
        val=oc_pack_read1(&_dec->opb);
        flag=(int32_t)val;
        nqi1=0;
        fragii=0;
        while(fragii<ncoded_fragis){
            int32_t full_run;
            run_count=oc_sb_run_unpack(&_dec->opb);
            full_run=run_count>=4129;
            do{
                frags[coded_fragis[fragii++]].qii=flag;
                nqi1+=flag;
            }
            while(--run_count>0&&fragii<ncoded_fragis);
            if(full_run&&fragii<ncoded_fragis){
                val=oc_pack_read1(&_dec->opb);
                flag=(int32_t)val;
            }
            else flag=!flag;
        }
        /*TODO: run_count should be 0 here.
        If it's not, we should issue a warning of some kind.*/
        /*If we have 3 different qi's for this frame, and there was at least one
        fragment with a non-zero qi, make the second pass.*/
        if(_dec->state.nqis==3&&nqi1>0){
            /*Skip qii==0 fragments.*/
            for(fragii=0;frags[coded_fragis[fragii]].qii==0;fragii++);
            val=oc_pack_read1(&_dec->opb);
            flag=(int32_t)val;
            do{
                int32_t full_run;
                run_count=oc_sb_run_unpack(&_dec->opb);
                full_run=run_count>=4129;
                for(;fragii<ncoded_fragis;fragii++){
                    fragi=coded_fragis[fragii];
                    if(frags[fragi].qii==0)continue;
                    if(run_count--<=0)break;
                    frags[fragi].qii+=flag;
                }
                if(full_run&&fragii<ncoded_fragis){
                    val=oc_pack_read1(&_dec->opb);
                    flag=(int32_t)val;
                }
                else flag=!flag;
            }
            while(fragii<ncoded_fragis);
            /*TODO: run_count should be 0 here.
            If it's not, we should issue a warning of some kind.*/
        }
    }
}



/*Unpacks the DC coefficient tokens.
Unlike when unpacking the AC coefficient tokens, we actually need to decode
the DC coefficient values now so that we can do DC prediction.
_huff_idx:   The index of the Huffman table to use for each color plane.
_ntoks_left: The number of tokens left to be decoded in each color plane for
each coefficient.
This is updated as EOB tokens and zero run tokens are decoded.
Return: The length of any outstanding EOB run.*/
static int32_t oc_dec_dc_coeff_unpack(oc_dec_ctx *_dec,int32_t _huff_idxs[2],
    int32_t _ntoks_left[3][64])
{
        unsigned char   *dct_tokens;
        oc_fragment     *frags;
        const int32_t       *coded_fragis;
        int32_t              ncoded_fragis;
        int32_t              fragii;
        int32_t              eobs;
        int32_t              ti;
        int32_t              pli;
        dct_tokens=_dec->dct_tokens;
        frags=_dec->state.frags;
        coded_fragis=_dec->state.coded_fragis;
        ncoded_fragis=fragii=eobs=ti=0;
        for(pli=0;pli<3;pli++)
        {
            int32_t  run_counts[64];
            int32_t  eob_count;
            int32_t  eobi;
            int32_t  rli;
            ncoded_fragis+=_dec->state.ncoded_fragis[pli];
            memset(run_counts,0,sizeof(run_counts));
            _dec->eob_runs[pli][0]=eobs;
            _dec->ti0[pli][0]=ti;
            /*Continue any previous EOB run, if there was one.*/
            eobi=eobs;
            if(ncoded_fragis-fragii<eobi)eobi=ncoded_fragis-fragii;
            eob_count=eobi;
            eobs-=eobi;
            while(eobi-->0)
                frags[coded_fragis[fragii++]].dc=0;
            while(fragii<ncoded_fragis){
                int32_t token;
                int32_t cw;
                int32_t eb;
                int32_t skip;
                token=oc_huff_token_decode(&_dec->opb,
                    _dec->huff_tables[_huff_idxs[pli+1>>1]]);
                dct_tokens[ti++]=(unsigned char)token;
                if(OC_DCT_TOKEN_NEEDS_MORE(token)){
                    eb=(int32_t)oc_pack_read(&_dec->opb,
                        OC_INTERNAL_DCT_TOKEN_EXTRA_BITS[token]);
                    dct_tokens[ti++]=(unsigned char)eb;
                    if(token==OC_DCT_TOKEN_FAT_EOB)dct_tokens[ti++]=(unsigned char)(eb>>8);
                    eb<<=OC_DCT_TOKEN_EB_POS(token);
                }
                else eb=0;
                cw=OC_DCT_CODE_WORD[token]+eb;
                eobs=cw>>OC_DCT_CW_EOB_SHIFT&0xFFF;
                if(cw==OC_DCT_CW_FINISH)eobs=OC_DCT_EOB_FINISH;
                if(eobs){
                    eobi=OC_MINI(eobs,ncoded_fragis-fragii);
                    eob_count+=eobi;
                    eobs-=eobi;
                    while(eobi-->0)frags[coded_fragis[fragii++]].dc=0;
                }
                else{
                    int32_t coeff;
                    skip=(unsigned char)(cw>>OC_DCT_CW_RLEN_SHIFT);
                    cw^=-(cw&1<<OC_DCT_CW_FLIP_BIT);
                    coeff=cw>>OC_DCT_CW_MAG_SHIFT;
                    if(skip)coeff=0;
                    run_counts[skip]++;
                    frags[coded_fragis[fragii++]].dc=coeff;
                }
            }
            /*Add the total EOB count to the longest run length.*/
            run_counts[63]+=eob_count;
            /*And convert the run_counts array to a moment table.*/
            for(rli=63;rli-->0;)run_counts[rli]+=run_counts[rli+1];
            /*Finally, subtract off the number of coefficients that have been
            accounted for by runs started in this coefficient.*/
            for(rli=64;rli-->0;)_ntoks_left[pli][rli]-=run_counts[rli];
        }
        _dec->dct_tokens_count=ti;
        return eobs;
}

/*Unpacks the AC coefficient tokens.
This can completely discard coefficient values while unpacking, and so is
somewhat simpler than unpacking the DC coefficient tokens.
_huff_idx:   The index of the Huffman table to use for each color plane.
_ntoks_left: The number of tokens left to be decoded in each color plane for
each coefficient.
This is updated as EOB tokens and zero run tokens are decoded.
_eobs:       The length of any outstanding EOB run from previous
coefficients.
Return: The length of any outstanding EOB run.*/
static int32_t oc_dec_ac_coeff_unpack(oc_dec_ctx *_dec,int32_t _zzi,int32_t _huff_idxs[2],
    int32_t _ntoks_left[3][64],int32_t _eobs)
{
    unsigned char *dct_tokens;
    int32_t            ti;
    int32_t            pli;
    dct_tokens=_dec->dct_tokens;
    ti=_dec->dct_tokens_count;
    for(pli=0;pli<3;pli++)
    {
        int32_t       run_counts[64];
        int32_t       eob_count;
        size_t    ntoks_left;
        size_t    ntoks;
        int32_t       rli;
        _dec->eob_runs[pli][_zzi]=_eobs;
        _dec->ti0[pli][_zzi]=ti;
        ntoks_left=_ntoks_left[pli][_zzi];
        memset(run_counts,0,sizeof(run_counts));
        eob_count=0;
        ntoks=0;
        while(ntoks+_eobs<ntoks_left)
        {
            int32_t token;
            int32_t cw;
            int32_t eb;
            int32_t skip;
            ntoks+=_eobs;
            eob_count+=_eobs;
            token=oc_huff_token_decode(&_dec->opb,
                _dec->huff_tables[_huff_idxs[pli+1>>1]]);
            dct_tokens[ti++]=(unsigned char)token;
            if(OC_DCT_TOKEN_NEEDS_MORE(token)){
                eb=(int32_t)oc_pack_read(&_dec->opb,
                    OC_INTERNAL_DCT_TOKEN_EXTRA_BITS[token]);
                dct_tokens[ti++]=(unsigned char)eb;
                if(token==OC_DCT_TOKEN_FAT_EOB)dct_tokens[ti++]=(unsigned char)(eb>>8);
                eb<<=OC_DCT_TOKEN_EB_POS(token);
            }
            else eb=0;
            cw=OC_DCT_CODE_WORD[token]+eb;
            skip=(unsigned char)(cw>>OC_DCT_CW_RLEN_SHIFT);
            _eobs=cw>>OC_DCT_CW_EOB_SHIFT&0xFFF;
            if(cw==OC_DCT_CW_FINISH)_eobs=OC_DCT_EOB_FINISH;
            if(_eobs==0){
                run_counts[skip]++;
                ntoks++;
            }
        }
        /*Add the portion of the last EOB run actually used by this coefficient.*/
        eob_count+=ntoks_left-ntoks;
        /*And remove it from the remaining EOB count.*/
        _eobs-=ntoks_left-ntoks;
        /*Add the total EOB count to the longest run length.*/
        run_counts[63]+=eob_count;
        /*And convert the run_counts array to a moment table.*/
        for(rli=63;rli-->0;)run_counts[rli]+=run_counts[rli+1];
        /*Finally, subtract off the number of coefficients that have been
        accounted for by runs started in this coefficient.*/
        for(rli=64-_zzi;rli-->0;)_ntoks_left[pli][_zzi+rli]-=run_counts[rli];
    }
    _dec->dct_tokens_count=ti;
    return _eobs;
}

/*Tokens describing the DCT coefficients that belong to each fragment are
stored in the bitstream grouped by coefficient, not by fragment.

This means that we either decode all the tokens in order, building up a
separate coefficient list for each fragment as we go, and then go back and
do the iDCT on each fragment, or we have to create separate lists of tokens
for each coefficient, so that we can pull the next token required off the
head of the appropriate list when decoding a specific fragment.

The former was VP3's choice, and it meant 2*w*h extra storage for all the
decoded coefficient values.

We take the second option, which lets us store just one to three bytes per
token (generally far fewer than the number of coefficients, due to EOB
tokens and zero runs), and which requires us to only maintain a counter for
each of the 64 coefficients, instead of a counter for every fragment to
determine where the next token goes.

We actually use 3 counters per coefficient, one for each color plane, so we
can decode all color planes simultaneously.
This lets color conversion, etc., be done as soon as a full MCU (one or
two super block rows) is decoded, while the image data is still in cache.*/

static void oc_dec_residual_tokens_unpack(oc_dec_ctx *_dec)
{
    static const unsigned char OC_HUFF_LIST_MAX[5]={1,6,15,28,64};
    int32_t        ntoks_left[3][64];
    int32_t        huff_idxs[2];
    int32_t        eobs;
    long       val;
    int32_t        pli;
    int32_t        zzi;
    int32_t        hgi;
    for(pli=0;pli<3;pli++)for(zzi=0;zzi<64;zzi++)
    {
        ntoks_left[pli][zzi]=_dec->state.ncoded_fragis[pli];
    }
    val=oc_pack_read(&_dec->opb,4);
    huff_idxs[0]=(int32_t)val;
    val=oc_pack_read(&_dec->opb,4);
    huff_idxs[1]=(int32_t)val;
    _dec->eob_runs[0][0]=0;
    eobs=oc_dec_dc_coeff_unpack(_dec,huff_idxs,ntoks_left);

    val=oc_pack_read(&_dec->opb,4);
    huff_idxs[0]=(int32_t)val;
    val=oc_pack_read(&_dec->opb,4);
    huff_idxs[1]=(int32_t)val;
    zzi=1;
    for(hgi=1;hgi<5;hgi++)
    {
        huff_idxs[0]+=16;
        huff_idxs[1]+=16;
        for(;zzi<OC_HUFF_LIST_MAX[hgi];zzi++)
        {
            eobs=oc_dec_ac_coeff_unpack(_dec,zzi,huff_idxs,ntoks_left,eobs);
        }
    }
    /*TODO: eobs should be exactly zero, or 4096 or greater.
    The second case occurs when an EOB run of size zero is encountered, which
    gets treated as an infinite EOB run (where infinity is PTRDIFF_MAX).
    If neither of these conditions holds, then a warning should be issued.*/
}


/*Undo the DC prediction in a single plane of an MCU (one or two super block
rows).
As a side effect, the number of coded and uncoded fragments in this plane of
the MCU is also computed.*/
void oc_dec_dc_unpredict_mcu_plane(oc_dec_ctx *_dec, int32_t pli)
{
    const oc_fragment_plane *fplane;
    oc_fragment             *frags;
    int32_t                      pred_last[3];
    int32_t                      ncoded_fragis;
    int32_t                      fragi;
    int32_t                      fragx;
    int32_t                      fragy;
    int32_t                      nvfrags;
    int32_t                      nhfrags;
    /*Compute the first and last fragment row of the current MCU for this
    plane.*/
    fplane  = _dec->state.fplanes+pli;
    nhfrags = fplane->nhfrags;
    nvfrags =fplane->nvfrags;

    frags = _dec->state.frags;
    fragi = fplane->froffset;

    ncoded_fragis=0;
    pred_last[0]=
    pred_last[1]=
    pred_last[2]=0;

    // bottom line
    for(fragx=0;fragx<nhfrags;fragx++,fragi++)
    {
        if(frags[fragi].coded)
        {
            int32_t ref;
            // num of coefficients update 
            // in case dc coefficient was not counted
            ref=OC_FRAME_FOR_MODE(frags[fragi].mb_mode);
            pred_last[ref]=frags[fragi].dc+=pred_last[ref];
            ncoded_fragis++;
        }
    }

    for(fragy=1; fragy<nvfrags ;fragy++)
    {
        oc_fragment *u_frags;
        int32_t          l_ref;
        int32_t          ul_ref;
        int32_t          u_ref;
        u_frags=frags-nhfrags;
        l_ref=-1;
        ul_ref=-1;
        u_ref=u_frags[fragi].coded?OC_FRAME_FOR_MODE(u_frags[fragi].mb_mode):-1;
        for(fragx=0;fragx<nhfrags;fragx++,fragi++)
        {
            int32_t ur_ref;
            if(fragx+1>=nhfrags)ur_ref=-1;
            else
            {
                ur_ref=u_frags[fragi+1].coded?
                    OC_FRAME_FOR_MODE(u_frags[fragi+1].mb_mode):-1;
            }
            if(frags[fragi].coded)
            {
                int32_t pred;
                int32_t ref;
                ref=OC_FRAME_FOR_MODE(frags[fragi].mb_mode);
                /*We break out a separate case based on which of our neighbors use
                the same reference frames.
                This is somewhat faster than trying to make a generic case which
                handles all of them, since it reduces lots of poorly predicted
                jumps to one switch statement, and also lets a number of the
                multiplications be optimized out by strength reduction.*/
                switch((l_ref==ref)|(ul_ref==ref)<<1|(u_ref==ref)<<2|(ur_ref==ref)<<3)
                {
                default:pred=pred_last[ref];break;
                case  1:
                case  3:pred=frags[fragi-1].dc;break;
                case  2:pred=u_frags[fragi-1].dc;break;
                case  4:
                case  6:
                case 12:pred=u_frags[fragi].dc;break;
                case  5:pred=(frags[fragi-1].dc+u_frags[fragi].dc)/2;break;
                case  8:pred=u_frags[fragi+1].dc;break;
                case  9:
                case 11:
                case 13:{
                    /*The TI compiler mis-compiles this line.*/
                    pred=(75*frags[fragi-1].dc+53*u_frags[fragi+1].dc)/128;
                        }break;
                case 10:pred=(u_frags[fragi-1].dc+u_frags[fragi+1].dc)/2;break;
                case 14:{
                    pred=(3*(u_frags[fragi-1].dc+u_frags[fragi+1].dc)
                        +10*u_frags[fragi].dc)/16;
                        }break;
                case  7:
                case 15:{
                    int32_t p0;
                    int32_t p1;
                    int32_t p2;
                    p0=frags[fragi-1].dc;
                    p1=u_frags[fragi-1].dc;
                    p2=u_frags[fragi].dc;
                    pred=(29*(p0+p2)-26*p1)/32;
                    if(abs(pred-p2)>128)pred=p2;
                    else if(abs(pred-p0)>128)pred=p0;
                    else if(abs(pred-p1)>128)pred=p1;
                        }break;
                }
                pred_last[ref]=frags[fragi].dc+=pred;
                ncoded_fragis++;
                l_ref=ref;
            }
            else l_ref=-1;
            ul_ref=u_ref;
            u_ref=ur_ref;
        }
    }
}
/*Reconstructs all coded fragments in a single MCU (one or two super block
rows).
This requires that each coded fragment have a proper macro block mode and
motion vector (if not in INTRA mode), and have its DC value decoded, with
the DC prediction process reversed, and the number of coded and uncoded
fragments in this plane of the MCU be counted.
The token lists for each color plane and coefficient should also be filled
in, along with initial token offsets, extra bits offsets, and EOB run
counts.*/
static void oc_dec_residual_coefficients(oc_dec_ctx *_dec, int32_t pli)
{
    unsigned char       *dct_tokens;
    const unsigned char *dct_fzig_zag;
    oc_fragment         *frags;
    oc_qcm              *frag_qcm;
    uint8_t             *frag_eob;
    const int32_t           *coded_fragis;
    int32_t                  ncoded_fragis;
    int32_t                  fragii;
    int32_t                 *ti;
    int32_t                 *eob_runs;
    int32_t                  qti;
    int32_t                  ncoded_fragis_offset = 0;

    dct_tokens=_dec->dct_tokens;
    dct_fzig_zag=_dec->state.dct_fzig_zag;
    frags = _dec->state.frags;
    frag_qcm = _dec->state.frag_qcm;
    frag_eob = _dec->state.frag_eob;

    coded_fragis	= _dec->state.coded_fragis + _dec->state.prev_coded_offset;
    ncoded_fragis	= _dec->state.ncoded_fragis[pli];
    ti=_dec->ti0[pli];
    eob_runs=_dec->eob_runs[pli];

    for(fragii=0;fragii<ncoded_fragis;fragii++)
    {
        int16_t  *qcm;
        int32_t      fragi;
        int32_t      last_zzi;
        int32_t      zzi;
        fragi=coded_fragis[fragii];
        qcm = frag_qcm[fragi];
        qti=frags[fragi].mb_mode!=OC_MODE_INTRA;
        /*Decode the AC coefficients.*/
        for(zzi=0;zzi<64;)
        {
            int32_t token;
            last_zzi=zzi;
            if(eob_runs[zzi])
            {
                eob_runs[zzi]--;
                break;
            }
            else{
                int32_t   eob;
                int32_t   cw;
                int32_t   rlen;
                int32_t   coeff;
                int32_t   lti;
                lti=ti[zzi];
                token=dct_tokens[lti++];
                cw=OC_DCT_CODE_WORD[token];
                /*These parts could be done branchless, but the branches are fairly
                predictable and the C code translates into more than a few
                instructions, so it's worth it to avoid them.*/
                if(OC_DCT_TOKEN_NEEDS_MORE(token)){
                    cw+=dct_tokens[lti++]<<OC_DCT_TOKEN_EB_POS(token);
                }
                eob=cw>>OC_DCT_CW_EOB_SHIFT&0xFFF;
                if(token==OC_DCT_TOKEN_FAT_EOB)
                {
                    eob+=dct_tokens[lti++]<<8;
                    if(eob==0)eob=OC_DCT_EOB_FINISH;
                }
                rlen=(unsigned char)(cw>>OC_DCT_CW_RLEN_SHIFT);
                cw^=-(cw&1<<OC_DCT_CW_FLIP_BIT);
                coeff=cw>>OC_DCT_CW_MAG_SHIFT;

                eob_runs[zzi]=eob;
                ti[zzi]=lti;
                zzi+=rlen;
                qcm[zzi] = (int16_t)(coeff);
                zzi+=!eob;
            }
        }
        // dc value
        qcm[0] = frags[fragi].dc; 
        /*TODO: zzi should be exactly 64 here.
        If it's not, we should report some kind of warning.*/
        zzi=OC_MINI(zzi,64);
        frag_eob[fragi] = zzi;
    }
    _dec->state.prev_coded_offset += ncoded_fragis;
}

th_dec_ctx *th_decode_alloc(const th_info *_info,const th_setup_info *_setup)
{
    oc_dec_ctx *dec;
    if(_info==NULL||_setup==NULL)return NULL;
    dec=oc_aligned_malloc(sizeof(*dec),16);
    if(dec==NULL||oc_dec_init(dec,_info,_setup)<0)
    {
        oc_alignedsys_mem_free(dec);
        return NULL;
    }
    dec->state.curframe_num=0;
    return dec;
}

void th_decodesys_mem_free(th_dec_ctx *_dec)
{
    if(_dec!=NULL){
        oc_dec_clear(_dec);
        oc_alignedsys_mem_free(_dec);
    }
}


int32_t th_decode_packetin(th_dec_ctx *_dec,unsigned char *packet, int32_t packet_size, int64_t *_granpos)
{
    int32_t ret, pli;
    if(_dec==NULL||packet==NULL)return TH_EFAULT;
    /*A completely empty packet indicates a dropped frame and is treated exactly
    like an inter frame with no coded blocks.*/
    if(packet_size==1)
    {
        _dec->state.frame_type=OC_INTER_FRAME;
        _dec->state.ntotal_coded_fragis=0;
    }
    else{
        oc_pack_readinit(&_dec->opb, packet, packet_size);

        /* add temporal initialize code, must be removed*/
        /* if consider merging mb packing and dct coeff, it seems that the performance may be good.*/
        ret=oc_dec_frame_header_unpack(_dec);
        if(ret<0)return ret;

        if(_dec->state.frame_type==OC_INTRA_FRAME) 
            oc_dec_mark_all_intra(_dec);
        else 
            // coded flasg
            oc_dec_coded_flags_unpack(_dec);
    }
    /*If this was an inter frame with no coded blocks...*/
    if(_dec->state.ntotal_coded_fragis<=0)
    {
        _dec->state.curframe_num++;
        return TH_DUPFRAME;
    }
    else
    {
        if(_dec->state.frame_type==OC_INTRA_FRAME)
        {
            _dec->state.keyframe_num=_dec->state.curframe_num;
        }
        else
        {
            // mb mode umpack
            oc_dec_mb_modes_unpack(_dec);
            // mv unpack
            oc_dec_mv_unpack_and_frag_modes_fill(_dec);
        }
        // Block-Level qi Decode
        oc_dec_block_qis_unpack(_dec);
        // tokens unpack
        oc_dec_residual_tokens_unpack(_dec);

        for(pli=0; pli<3; pli++)
        {
            // Undoing DC Prediction
            oc_dec_dc_unpredict_mcu_plane(_dec, pli);

            // decode codefficients     
            oc_dec_residual_coefficients(_dec, pli);
        }
        
        _dec->state.prev_coded_offset = 0;
    }

    _dec->state.curframe_num++;

    return 1;
}
