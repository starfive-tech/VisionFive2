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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitpack.h"
#include "internal.h"
#include "decint.h"


/*The maximum output of the DCT with +/- 255 inputs is +/- 8157.
These minimum quantizers ensure the result after quantization (and after
prediction for DC) will be no more than +/- 510.
The tokenization system can handle values up to +/- 580, so there is no need
to do any coefficient clamping.
I would rather have allowed smaller quantizers and had to clamp, but these
minimums were required when constructing the original VP3 matrices and have
been formalized in the spec.*/
static const uint32_t OC_DC_QUANT_MIN[2]={4<<2,8<<2};
static const uint32_t OC_AC_QUANT_MIN[2]={2<<2,4<<2};


/*The map from external spec-defined tokens to internal tokens.
This is constructed so that any extra bits read with the original token value
can be masked off the least significant bits of its internal token index.
In addition, all of the tokens which require additional extra bits are placed
at the start of the list, and grouped by type.
OC_DCT_REPEAT_RUN3_TOKEN is placed first, as it is an extra-special case, so
giving it index 0 may simplify comparisons on some architectures.
These requirements require some substantial reordering.*/
static const unsigned char OC_DCT_TOKEN_MAP[TH_NDCT_TOKENS]=
{
    /*OC_DCT_EOB1_TOKEN (0 extra bits)*/
    15,
    /*OC_DCT_EOB2_TOKEN (0 extra bits)*/
    16,
    /*OC_DCT_EOB3_TOKEN (0 extra bits)*/
    17,
    /*OC_DCT_REPEAT_RUN0_TOKEN (2 extra bits)*/
    88,
    /*OC_DCT_REPEAT_RUN1_TOKEN (3 extra bits)*/
    80,
    /*OC_DCT_REPEAT_RUN2_TOKEN (4 extra bits)*/
    1,
    /*OC_DCT_REPEAT_RUN3_TOKEN (12 extra bits)*/
    0,
    /*OC_DCT_SHORT_ZRL_TOKEN (3 extra bits)*/
    48,
    /*OC_DCT_ZRL_TOKEN (6 extra bits)*/
    14,
    /*OC_ONE_TOKEN (0 extra bits)*/
    56,
    /*OC_MINUS_ONE_TOKEN (0 extra bits)*/
    57,
    /*OC_TWO_TOKEN (0 extra bits)*/
    58,
    /*OC_MINUS_TWO_TOKEN (0 extra bits)*/
    59,
    /*OC_DCT_VAL_CAT2 (1 extra bit)*/
    60,
    62,
    64,
    66,
    /*OC_DCT_VAL_CAT3 (2 extra bits)*/
    68,
    /*OC_DCT_VAL_CAT4 (3 extra bits)*/
    72,
    /*OC_DCT_VAL_CAT5 (4 extra bits)*/
    2,
    /*OC_DCT_VAL_CAT6 (5 extra bits)*/
    4,
    /*OC_DCT_VAL_CAT7 (6 extra bits)*/
    6,
    /*OC_DCT_VAL_CAT8 (10 extra bits)*/
    8,
    /*OC_DCT_RUN_CAT1A (1 extra bit)*/
    18,
    20,
    22,
    24,
    26,
    /*OC_DCT_RUN_CAT1B (3 extra bits)*/
    32,
    /*OC_DCT_RUN_CAT1C (4 extra bits)*/
    12,
    /*OC_DCT_RUN_CAT2A (2 extra bits)*/
    28,
    /*OC_DCT_RUN_CAT2B (3 extra bits)*/
    40
};

/*The log base 2 of number of internal tokens associated with each of the spec
tokens (i.e., how many of the extra bits are folded into the token value).
Increasing the maximum value beyond 3 will enlarge the amount of stack
required for tree construction.*/
static const unsigned char OC_DCT_TOKEN_MAP_LOG_NENTRIES[TH_NDCT_TOKENS]={
    0,0,0,2,3,0,0,3,0,0,0,0,0,1,1,1,1,2,3,1,1,1,2,1,1,1,1,1,3,1,2,3
};


#define OC_HUFF_SLUSH (2)
#define OC_ROOT_HUFF_SLUSH (7)

/*Unpacks a Huffman codebook.
_opb:    The buffer to unpack from.
_tokens: Stores a list of internal tokens, in the order they were found in
the codebook, and the lengths of their corresponding codewords.
This is enough to completely define the codebook, while minimizing
stack usage and avoiding temporary allocations (for platforms
where free() is a no-op).
Return: The number of internal tokens in the codebook, or a negative value
on error.*/
int32_t oc_huff_tree_unpack(oc_pack_buf *_opb,unsigned char _tokens[256][2])
{
    uint32_t code;
    int32_t          len;
    int32_t          ntokens;
    int32_t          nleaves;
    code=0;
    len=ntokens=nleaves=0;
    for(;;){
        long bits;
        bits=oc_pack_read1(_opb);
        /*Only process nodes so long as there's more bits in the buffer.*/
        if(oc_pack_bytes_left(_opb)<0) return TH_EBADHEADER;
        /*Read an internal node:*/
        if(!bits){
            len++;
            /*Don't allow codewords longer than 32 bits.*/
            if(len>32)return TH_EBADHEADER;
        }
        /*Read a leaf node:*/
        else{
            uint32_t code_bit;
            int32_t          neb;
            int32_t          nentries;
            int32_t          token;
            /*Don't allow more than 32 spec-tokens per codebook.*/
            if(++nleaves>32)return TH_EBADHEADER;
            bits=oc_pack_read(_opb,OC_NDCT_TOKEN_BITS);
            neb=OC_DCT_TOKEN_MAP_LOG_NENTRIES[bits];
            token=OC_DCT_TOKEN_MAP[bits];
            nentries=1<<neb;
            while(nentries-->0){
                _tokens[ntokens][0]=(unsigned char)token++;
                _tokens[ntokens][1]=(unsigned char)(len+neb);
                ntokens++;
            }
            code_bit=0x80000000U>>len-1;
            while(len>0&&(code&code_bit)){
                code^=code_bit;
                code_bit<<=1;
                len--;
            }
            if(len<=0)break;
            code|=code_bit;
        }
    }
    return ntokens;
}

/*Count how many tokens would be required to fill a subtree at depth _depth.
_tokens: A list of internal tokens, in the order they are found in the
codebook, and the lengths of their corresponding codewords.
_depth:  The depth of the desired node in the corresponding tree structure.
Return: The number of tokens that belong to that subtree.*/
static int32_t oc_huff_subtree_tokens(unsigned char _tokens[][2],int32_t _depth)
{
    uint32_t code;
    int32_t          ti;
    code=0;
    ti=0;
    do{
        if(_tokens[ti][1]-_depth<32)code+=0x80000000U>>_tokens[ti++][1]-_depth;
        else{
            /*Because of the expanded internal tokens, we can have codewords as long
            as 35 bits.
            A single recursion here is enough to advance past them.*/
            code++;
            ti+=oc_huff_subtree_tokens(_tokens+ti,_depth+31);
        }
    }
    while(code<0x80000000U);
    return ti;
}

/*Compute the number of bits to use for a collapsed tree node at the given
depth.
_tokens:  A list of internal tokens, in the order they are found in the
codebook, and the lengths of their corresponding codewords.
_ntokens: The number of tokens corresponding to this tree node.
_depth:   The depth of this tree node.
Return: The number of bits to use for a collapsed tree node rooted here.
This is always at least one, even if this was a leaf node.*/
static int32_t oc_huff_tree_collapse_depth(unsigned char _tokens[][2],
                                       int32_t _ntokens,int32_t _depth)
{
    int32_t got_leaves;
    int32_t loccupancy;
    int32_t occupancy;
    int32_t slush;
    int32_t nbits;
    int32_t best_nbits;
    slush=_depth>0?OC_HUFF_SLUSH:OC_ROOT_HUFF_SLUSH;
    /*It's legal to have a tree with just a single node, which requires no bits
    to decode and always returns the same token.
    However, no encoder actually does this (yet).
    To avoid a special case in oc_huff_token_decode(), we force the number of
    lookahead bits to be at least one.
    This will produce a tree that looks ahead one bit and then advances the
    stream zero bits.*/
    nbits=1;
    occupancy=2;
    got_leaves=1;
    do{
        int32_t ti;
        if(got_leaves)best_nbits=nbits;
        nbits++;
        got_leaves=0;
        loccupancy=occupancy;
        for(occupancy=ti=0;ti<_ntokens;occupancy++){
            if(_tokens[ti][1]<_depth+nbits)ti++;
            else if(_tokens[ti][1]==_depth+nbits){
                got_leaves=1;
                ti++;
            }
            else ti+=oc_huff_subtree_tokens(_tokens+ti,_depth+nbits);
        }
    }
    while(occupancy>loccupancy&&occupancy*slush>=1<<nbits);
    return best_nbits;
}

/*Determines the size in words of a Huffman tree node that represents a
subtree of depth _nbits.
_nbits: The depth of the subtree.
This must be greater than zero.
Return: The number of words required to store the node.*/
static size_t oc_huff_node_size(int32_t _nbits){
    return 1+(1<<_nbits);
}

/*Produces a collapsed-tree representation of the given token list.
_tree: The storage for the collapsed Huffman tree.
This may be NULL to compute the required storage size instead of
constructing the tree.
_tokens:  A list of internal tokens, in the order they are found in the
codebook, and the lengths of their corresponding codewords.
_ntokens: The number of tokens corresponding to this tree node.
Return: The number of words required to store the tree.*/
static size_t oc_huff_tree_collapse(int16_t *_tree,
                                    unsigned char _tokens[][2],int32_t _ntokens)
{
    int16_t   node[34];
    unsigned char depth[34];
    unsigned char last[34];
    size_t        ntree;
    int32_t           ti;
    int32_t           l;
    depth[0]=0;
    last[0]=(unsigned char)(_ntokens-1);
    ntree=0;
    ti=0;
    l=0;
    do{
        int32_t nbits;
        nbits=oc_huff_tree_collapse_depth(_tokens+ti,last[l]+1-ti,depth[l]);
        node[l]=(int16_t)ntree;
        ntree+=oc_huff_node_size(nbits);
        if(_tree!=NULL)_tree[node[l]++]=(int16_t)nbits;
        do{
            while(ti<=last[l]&&_tokens[ti][1]<=depth[l]+nbits){
                if(_tree!=NULL){
                    int16_t leaf;
                    int32_t         nentries;
                    nentries=1<<depth[l]+nbits-_tokens[ti][1];
                    leaf=(int16_t)-(_tokens[ti][1]-depth[l]<<8|_tokens[ti][0]);
                    while(nentries-->0)_tree[node[l]++]=leaf;
                }
                ti++;
            }
            if(ti<=last[l]){
                /*We need to recurse*/
                depth[l+1]=(unsigned char)(depth[l]+nbits);
                if(_tree!=NULL)_tree[node[l]++]=(int16_t)ntree;
                l++;
                last[l]=
                    (unsigned char)(ti+oc_huff_subtree_tokens(_tokens+ti,depth[l])-1);
                break;
            }
            /*Pop back up a level of recursion.*/
            else if(l-->0)nbits=depth[l+1]-depth[l];
        }
        while(l>=0);
    }
    while(l>=0);
    return ntree;
}

/*Unpacks a set of Huffman trees, and reduces them to a collapsed
representation.
_opb:   The buffer to unpack the trees from.
_nodes: The table to fill with the Huffman trees.
Return: 0 on success, or a negative value on error.*/
int32_t oc_huff_trees_unpack(oc_pack_buf *_opb,
                         int16_t *_nodes[TH_NHUFFMAN_TABLES])
{
    int32_t ret;
    int32_t i;
    ret=0;
    for(i=0;i<TH_NHUFFMAN_TABLES;i++){
        unsigned char  tokens[256][2];
        int32_t            ntokens;
        int16_t   *tree;
        size_t         size;
        /*Unpack the full tree into a temporary buffer.*/
        ntokens=oc_huff_tree_unpack(_opb,tokens);
        if(ntokens<0){
            ret=ntokens;
            break;
        }
        /*Figure out how big the collapsed tree will be and allocate space for it.*/
        size=oc_huff_tree_collapse(NULL,tokens,ntokens);
        if(size>32767){
            /*This should never happen; if it does it means you set OC_HUFF_SLUSH or
            OC_ROOT_HUFF_SLUSH too large.*/
            ret=TH_EIMPL;
            break;
        }
        tree=(int16_t *)malloc(size*sizeof(*tree));
        if(tree==NULL){
            ret=TH_EFAULT;
            break;
        }
        /*Construct the collapsed the tree.*/
        oc_huff_tree_collapse(tree,tokens,ntokens);
        _nodes[i]=tree;
    }
    if(ret<0)while(i-->0)free(_nodes[i]);
    return ret;
}

/*Determines the size in words of a Huffman subtree.
_tree: The complete Huffman tree.
_node: The index of the root of the desired subtree.
Return: The number of words required to store the tree.*/
static size_t oc_huff_tree_size(const int16_t *_tree,int32_t _node)
{
    size_t size;
    int32_t    nchildren;
    int32_t    n;
    int32_t    i;
    n=_tree[_node];
    size=oc_huff_node_size(n);
    nchildren=1<<n;
    i=0;
    do{
        int32_t child;
        child=_tree[_node+i+1];
        if(child<=0)i+=1<<n-(-child>>8);
        else{
            size+=oc_huff_tree_size(_tree,child);
            i++;
        }
    }
    while(i<nchildren);
    return size;
}

/*Makes a copy of the given set of Huffman trees.
_dst: The array to store the copy in.
_src: The array of trees to copy.*/
int32_t oc_huff_trees_copy(int16_t *_dst[TH_NHUFFMAN_TABLES],
                       const int16_t *const _src[TH_NHUFFMAN_TABLES])
{
    int32_t total;
    int32_t i;
    total=0;
    for(i=0;i<TH_NHUFFMAN_TABLES;i++){
        size_t size;
        size=oc_huff_tree_size(_src[i],0);
        total+=size;
        _dst[i]=(int16_t *)malloc(size*sizeof(*_dst[i]));
        if(_dst[i]==NULL){
            while(i-->0)free(_dst[i]);
            return TH_EFAULT;
        }
        memcpy(_dst[i],_src[i],size*sizeof(*_dst[i]));
    }
    return 0;
}

/*Frees the memory used by a set of Huffman trees.
_nodes: The array of trees to free.*/
void oc_huff_trees_clear(int16_t *_nodes[TH_NHUFFMAN_TABLES]){
    int32_t i;
    for(i=0;i<TH_NHUFFMAN_TABLES;i++)free(_nodes[i]);
}


/*Unpacks a single token using the given Huffman tree.
_opb:  The buffer to unpack the token from.
_node: The tree to unpack the token with.
Return: The token value.*/
int32_t oc_huff_token_decode_c(oc_pack_buf *_opb,const int16_t *_tree)
{
    const unsigned char *ptr;
    const unsigned char *stop;
    oc_pb_window         window;
    int32_t                  available;
    long                 bits;
    int32_t                  node;
    int32_t                  n;
    ptr=_opb->ptr;
    window=_opb->window;
    stop=_opb->stop;
    available=_opb->bits;
    node=0;
    for(;;){
        n=_tree[node];
        if(n>available){
            uint32_t shift;
            shift=OC_PB_WINDOW_SIZE-available;
            do{
                /*We don't bother setting eof because we won't check for it after we've
                started decoding DCT tokens.*/
                if(ptr>=stop){
                    shift=-OC_LOTS_OF_BITS;
                    break;
                }
                shift-=8;
                window|=(oc_pb_window)*ptr++<<shift;
            }
            while(shift>=8);
            /*Note: We never request more than 24 bits, so there's no need to fill in
            the last partial byte here.*/
            available=OC_PB_WINDOW_SIZE-shift;
        }
        bits=window>>OC_PB_WINDOW_SIZE-n;
        node=_tree[node+1+bits];
        if(node<=0)break;
        window<<=n;
        available-=n;
    }
    node=-node;
    n=node>>8;
    window<<=n;
    available-=n;
    _opb->ptr=ptr;
    _opb->window=window;
    _opb->bits=available;
    return node&255;
}


void oc_pack_readinit(oc_pack_buf *_b,unsigned char *_buf,long _bytes)
{
    memset(_b,0,sizeof(*_b));
    _b->ptr=_buf;
    _b->stop=_buf+_bytes;
}

static oc_pb_window oc_pack_refill(oc_pack_buf *_b,int32_t _bits)
{
    const unsigned char *ptr;
    const unsigned char *stop;
    oc_pb_window         window;
    int32_t              available;
    uint32_t             shift;
    stop=_b->stop;
    ptr=_b->ptr;
    window=_b->window;
    available=_b->bits;
    shift=OC_PB_WINDOW_SIZE-available;
    while(7<shift&&ptr<stop){
        shift-=8;
        window|=(oc_pb_window)*ptr++<<shift;
    }
    _b->ptr=ptr;
    available=OC_PB_WINDOW_SIZE-shift;
    if(_bits>available){
        if(ptr>=stop){
            _b->eof=1;
            available=OC_LOTS_OF_BITS;
        }
        else window|=*ptr>>(available&7);
    }
    _b->bits=available;
    return window;
}

int32_t oc_pack_look1(oc_pack_buf *_b)
{
    oc_pb_window window;
    int32_t          available;
    window=_b->window;
    available=_b->bits;
    if(available<1)_b->window=window=oc_pack_refill(_b,1);
    return window>>OC_PB_WINDOW_SIZE-1;
}

void oc_pack_adv1(oc_pack_buf *_b)
{
    _b->window<<=1;
    _b->bits--;
}

/*Here we assume that 0<=_bits&&_bits<=32.*/
long oc_pack_read_c(oc_pack_buf *_b,int32_t _bits)
{
    oc_pb_window window;
    int32_t          available;
    long         result;
    window=_b->window;
    available=_b->bits;
    if(_bits==0)return 0;
    if(available<_bits){
        window=oc_pack_refill(_b,_bits);
        available=_b->bits;
    }
    result=window>>OC_PB_WINDOW_SIZE-_bits;
    available-=_bits;
    window<<=1;
    window<<=_bits-1;
    _b->window=window;
    _b->bits=available;
    return result;
}

int32_t oc_pack_read1_c(oc_pack_buf *_b)
{
    oc_pb_window window;
    int32_t          available;
    int32_t          result;
    window=_b->window;
    available=_b->bits;
    if(available<1){
        window=oc_pack_refill(_b,1);
        available=_b->bits;
    }
    result=window>>OC_PB_WINDOW_SIZE-1;
    available--;
    window<<=1;
    _b->window=window;
    _b->bits=available;
    return result;
}


long oc_pack_bytes_left(oc_pack_buf *_b)
{
    if(_b->eof)return -1;
    return _b->stop-_b->ptr+(_b->bits>>3);
}

/*Unpacks a series of octets from a given byte array into the pack buffer.
No checking is done to ensure the buffer contains enough data.
_opb: The pack buffer to read the octets from.
_buf: The byte array to store the unpacked bytes in.
_len: The number of octets to unpack.*/
static void oc_unpack_octets(oc_pack_buf *_opb,char *_buf,size_t _len)
{
    while(_len-->0){
        long val;
        val=oc_pack_read(_opb,8);
        *_buf++=(char)val;
    }
}

/*Unpacks a 32-bit integer encoded by octets in little-endian form.*/
static long oc_unpack_length(oc_pack_buf *_opb)
{
    long ret[4];
    int32_t  i;
    for(i=0;i<4;i++)ret[i]=oc_pack_read(_opb,8);
    return ret[0]|ret[1]<<8|ret[2]<<16|ret[3]<<24;
}

static int32_t oc_info_unpack(oc_pack_buf *_opb,th_info *_info)
{
    long val;
    /*Check the codec bitstream version.*/
    val=oc_pack_read(_opb,8);
    _info->version_major=(unsigned char)val;
    val=oc_pack_read(_opb,8);
    _info->version_minor=(unsigned char)val;
    val=oc_pack_read(_opb,8);
    _info->version_subminor=(unsigned char)val;
    /*verify we can parse this bitstream version.
    We accept earlier minors and all subminors, by spec*/
    if(_info->version_major>TH_VERSION_MAJOR||
        _info->version_major==TH_VERSION_MAJOR&&
        _info->version_minor>TH_VERSION_MINOR){
            return TH_EVERSION;
    }
    /*Read the encoded frame description.*/
    val=oc_pack_read(_opb,16);
    _info->frame_width=(uint32_t)val<<4;
    val=oc_pack_read(_opb,16);
    _info->frame_height=(uint32_t)val<<4;
    val=oc_pack_read(_opb,24);
    _info->pic_width=(uint32_t)val;
    val=oc_pack_read(_opb,24);
    _info->pic_height=(uint32_t)val;
    val=oc_pack_read(_opb,8);
    _info->pic_x=(uint32_t)val;
    val=oc_pack_read(_opb,8);
    _info->pic_y=(uint32_t)val;
    val=oc_pack_read(_opb,32);
    _info->fps_numerator=(uint32_t)val;
    val=oc_pack_read(_opb,32);
    _info->fps_denominator=(uint32_t)val;
    if(_info->frame_width==0||_info->frame_height==0||
        _info->pic_width+_info->pic_x>_info->frame_width||
        _info->pic_height+_info->pic_y>_info->frame_height||
        _info->fps_numerator==0||_info->fps_denominator==0){
            return TH_EBADHEADER;
    }
    /*Note: The sense of pic_y is inverted in what we pass back to the
    application compared to how it is stored in the bitstream.
    This is because the bitstream uses a right-handed coordinate system, while
    applications expect a left-handed one.*/
    _info->pic_y=_info->frame_height-_info->pic_height-_info->pic_y;
    val=oc_pack_read(_opb,24);
    _info->aspect_numerator=(uint32_t)val;
    val=oc_pack_read(_opb,24);
    _info->aspect_denominator=(uint32_t)val;
    val=oc_pack_read(_opb,8);
    _info->colorspace=(th_colorspace)val;
    val=oc_pack_read(_opb,24);
    _info->target_bitrate=(int32_t)val;
    val=oc_pack_read(_opb,6);
    _info->quality=(int32_t)val;
    val=oc_pack_read(_opb,5);
    _info->keyframe_granule_shift=(int32_t)val;
    val=oc_pack_read(_opb,2);
    _info->pixel_fmt=(th_pixel_fmt)val;
    if(_info->pixel_fmt==TH_PF_RSVD)return TH_EBADHEADER;
    val=oc_pack_read(_opb,3);
    if(val!=0||oc_pack_bytes_left(_opb)<0)return TH_EBADHEADER;
    return 0;
}

static int32_t oc_comment_unpack(oc_pack_buf *_opb,th_comment *_tc)
{
    long len;
    int32_t  i;
    /*Read the vendor string.*/
    len=oc_unpack_length(_opb);
    if(len<0||len>oc_pack_bytes_left(_opb))return TH_EBADHEADER;
    _tc->vendor=malloc((size_t)len+1);
    if(_tc->vendor==NULL)return TH_EFAULT;
    oc_unpack_octets(_opb,_tc->vendor,len);
    _tc->vendor[len]='\0';
    /*Read the user comments.*/
    _tc->comments=(int32_t)oc_unpack_length(_opb);
    len=_tc->comments;
    if(len<0||len>(LONG_MAX>>2)||len<<2>oc_pack_bytes_left(_opb)){
        _tc->comments=0;
        return TH_EBADHEADER;
    }
    _tc->comment_lengths=(int32_t *)malloc(
        _tc->comments*sizeof(_tc->comment_lengths[0]));
    _tc->user_comments=(char **)malloc(
        _tc->comments*sizeof(_tc->user_comments[0]));
    if(_tc->comment_lengths==NULL||_tc->user_comments==NULL){
        _tc->comments=0;
        return TH_EFAULT;
    }
    for(i=0;i<_tc->comments;i++){
        len=oc_unpack_length(_opb);
        if(len<0||len>oc_pack_bytes_left(_opb)){
            _tc->comments=i;
            return TH_EBADHEADER;
        }
        _tc->comment_lengths[i]=len;
        _tc->user_comments[i]=malloc((size_t)len+1);
        if(_tc->user_comments[i]==NULL){
            _tc->comments=i;
            return TH_EFAULT;
        }
        oc_unpack_octets(_opb,_tc->user_comments[i],len);
        _tc->user_comments[i][len]='\0';
    }
    return oc_pack_bytes_left(_opb)<0?TH_EBADHEADER:0;
}

static int32_t oc_setup_unpack(oc_pack_buf *_opb,th_setup_info *_setup)
{
    int32_t ret;
    /*Read the quantizer tables.*/
    ret=oc_quant_params_unpack(_opb,&_setup->qinfo);
    if(ret<0)return ret;
    /*Read the Huffman trees.*/
    return oc_huff_trees_unpack(_opb,_setup->huff_tables);
}

static void oc_setup_clear(th_setup_info *_setup){
    oc_quant_params_clear(&_setup->qinfo);
    oc_huff_trees_clear(_setup->huff_tables);
}



static int32_t split_headers(uint8_t *packet, int32_t packet_size,
                          int32_t first_header_size, uint8_t *header_start[3],
                          int32_t header_len[3])
{
    int32_t i;

    if (packet_size >= 6 && (((((const uint8_t*)(packet))[0] << 8) | ((const uint8_t*)(packet))[1]) == first_header_size))
    {
        int32_t overall_len = 6;
        for (i=0; i<3; i++)
        {
            header_len[i] = ((((const uint8_t*)(packet))[0] << 8) | ((const uint8_t*)(packet))[1]);
            packet += 2;
            header_start[i] = packet;
            packet += header_len[i];
            if (overall_len > packet_size - header_len[i])
                return -1;
            overall_len += header_len[i];
        }
    }
    else if (packet_size >= 3 && packet_size < INT_MAX - 0x1ff && packet[0] == 2)
    {
        int32_t overall_len = 3;
        packet++;
        for (i=0; i<2; i++, packet++)
        {
            header_len[i] = 0;
            for (; overall_len < packet_size && *packet==0xff; packet++)
            {
                header_len[i] += 0xff;
                overall_len   += 0xff + 1;
            }
            header_len[i] += *packet;
            overall_len   += *packet;
            if (overall_len > packet_size)
                return -1;
        }
        header_len[2]   = packet_size - overall_len;
        header_start[0] = packet;
        header_start[1] = header_start[0] + header_len[0];
        header_start[2] = header_start[1] + header_len[1];
    }
    else
    {
        return -1;
    }

    return 0;
}



static int32_t oc_dec_headerin(oc_pack_buf *_opb,th_info *_info,
                           th_comment *_tc,th_setup_info **_setup)
{
    char buffer[6];
    long val;
    int32_t  packtype;
    int32_t  ret;
    val=oc_pack_read(_opb,8);
    packtype=(int32_t)val;
    /*If we're at a data packet and we have received all three headers, we're
    done.*/
    if(!(packtype&0x80)&&_info->frame_width>0&&_tc->vendor!=NULL&&*_setup!=NULL){
        return 0;
    }
    /*Check the codec string.*/
    oc_unpack_octets(_opb,buffer,6);
    if(memcmp(buffer,"theora",6)!=0)return TH_ENOTFORMAT;
    switch(packtype)
    {
        /*Codec info header.*/
    case 0x80:{
        /*This should be the first packet, and we should not already be
        initialized.*/
        ret=oc_info_unpack(_opb,_info);
        if(ret<0)th_info_clear(_info);
        else ret=3;
              }break;
        /*Comment header.*/
    case 0x81:{
        if(_tc==NULL)return TH_EFAULT;
        /*We shoud have already decoded the info header, and should not yet have
        decoded the comment header.*/
        ret=oc_comment_unpack(_opb,_tc);
        if(ret<0)th_comment_clear(_tc);
        else ret=2;
              }break;
        /*Codec setup header.*/
    case 0x82:{
        oc_setup_info *setup;
        setup=(oc_setup_info *)calloc(1,sizeof(*setup));
        if(setup==NULL)return TH_EFAULT;
        ret=oc_setup_unpack(_opb,setup);
        if(ret<0){
            oc_setup_clear(setup);
            free(setup);
        }
        else{
            *_setup=setup;
            ret=1;
        }
              }break;
    default:{
        /*We don't know what this header is.*/
        return TH_EBADHEADER;
            }break;
    }
    return ret;
}


/*Decodes one header packet.
This should be called repeatedly with the packets at the beginning of the
stream until it returns 0.*/
int32_t th_decode_headerin(th_info *_info,th_comment *_tc, th_setup_info **_setup,
                       unsigned char *header, int32_t header_size)
{
    int32_t i;
    int32_t header_len[3];
    uint8_t *header_start[3];
    oc_pack_buf opb;

    if(header==NULL)return TH_EBADHEADER;
    if(_info==NULL)return TH_EFAULT;

    if (split_headers(header, header_size, 42,
        header_start, header_len) < 0)
    {
        fprintf(stderr, "[Error] Corrupt extradata\n");
        return -1;
    }

    for (i = 0; i < 3; i++)
    {
        oc_pack_readinit(&opb, header_start[i], header_len[i]);
        oc_dec_headerin(&opb,_info,_tc,_setup);

        // conform!!
        //if (s->theora < 0x030200)
        //    break;
    }

    return 0;
}

void th_setupsys_mem_free(th_setup_info *_setup)
{
    if(_setup!=NULL){
        oc_setup_clear(_setup);
        free(_setup);
    }
}


/*Initializes the dequantization tables from a set of quantizer info.
Currently the dequantizer (and elsewhere enquantizer) tables are expected to
be initialized as pointing to the storage reserved for them in the
oc_theora_state (resp. oc_enc_ctx) structure.
If some tables are duplicates of others, the pointers will be adjusted to
point to a single copy of the tables, but the storage for them will not be
freed.
If you're concerned about the memory footprint, the obvious thing to do is
to move the storage out of its fixed place in the structures and allocate
it on demand.
However, a much, much better option is to only store the quantization
matrices being used for the current frame, and to recalculate these as the
qi values change between frames (this is what VP3 did).*/
void oc_dequant_tables_init(uint16_t *_dequant[64][3][2],
                            int32_t _pp_dc_scale[64],const th_quant_info *_qinfo)
{
    /*Coding mode: intra or inter.*/
    int32_t          qti;
    /*Y', C_b, C_r*/
    int32_t          pli;
    for(qti=0;qti<2;qti++)for(pli=0;pli<3;pli++)
    {
        /*Quality index.*/
        int32_t qi;
        /*Range iterator.*/
        int32_t qri;
        for(qi=0,qri=0;qri<=_qinfo->qi_ranges[qti][pli].nranges;qri++){
            th_quant_base base;
            uint32_t  q;
            int32_t           qi_start;
            int32_t           qi_end;
            memcpy(base,_qinfo->qi_ranges[qti][pli].base_matrices[qri],
                sizeof(base));
            qi_start=qi;
            if(qri==_qinfo->qi_ranges[qti][pli].nranges)qi_end=qi+1;
            else qi_end=qi+_qinfo->qi_ranges[qti][pli].sizes[qri];
            /*Iterate over quality indicies in this range.*/
            for(;;){
                uint32_t qfac;
                int32_t          zzi;
                int32_t          ci;
                /*In the original VP3.2 code, the rounding offset and the size of the
                dead zone around 0 were controlled by a "sharpness" parameter.
                The size of our dead zone is now controlled by the per-coefficient
                quality thresholds returned by our HVS module.
                We round down from a more accurate value when the quality of the
                reconstruction does not fall below our threshold and it saves bits.
                Hence, all of that VP3.2 code is gone from here, and the remaining
                floating point code has been implemented as equivalent integer code
                with exact precision.*/
                qfac=(uint32_t)_qinfo->dc_scale[qi]*base[0];
                /*For postprocessing, not dequantization.*/
                if(_pp_dc_scale!=NULL)_pp_dc_scale[qi]=(int32_t)(qfac/160);
                /*Scale DC the coefficient from the proper table.*/
                q=(qfac/100)<<2;
                q=OC_CLAMPI(OC_DC_QUANT_MIN[qti],q,OC_QUANT_MAX);
                _dequant[qi][pli][qti][0]=(uint16_t)q;
                /*Now scale AC coefficients from the proper table.*/
                for(zzi=1;zzi<64;zzi++){
                    q=((uint32_t)_qinfo->ac_scale[qi]*base[zzi]/100)<<2;
                    q=OC_CLAMPI(OC_AC_QUANT_MIN[qti],q,OC_QUANT_MAX);
                    _dequant[qi][pli][qti][zzi]=(uint16_t)q;
                }
                /*If this is a duplicate of a previous matrix, use that instead.
                This simple check helps us improve cache coherency later.*/
                {
                    int32_t dupe;
                    int32_t qtj;
                    int32_t plj;
                    dupe=0;
                    for(qtj=0;qtj<=qti;qtj++){
                        for(plj=0;plj<(qtj<qti?3:pli);plj++){
                            if(!memcmp(_dequant[qi][pli][qti],_dequant[qi][plj][qtj],
                                sizeof(oc_quant_table))){
                                    dupe=1;
                                    break;
                            }
                        }
                        if(dupe)break;
                    }
                    if(dupe)_dequant[qi][pli][qti]=_dequant[qi][plj][qtj];
                }
                if(++qi>=qi_end)break;
                /*Interpolate the next base matrix.*/
                for(ci=0;ci<64;ci++){
                    base[ci]=(unsigned char)(
                        (2*((qi_end-qi)*_qinfo->qi_ranges[qti][pli].base_matrices[qri][ci]+
                        (qi-qi_start)*_qinfo->qi_ranges[qti][pli].base_matrices[qri+1][ci])
                        +_qinfo->qi_ranges[qti][pli].sizes[qri])/
                        (2*_qinfo->qi_ranges[qti][pli].sizes[qri]));
                }
            }
        }
    }
}


int32_t oc_quant_params_unpack(oc_pack_buf *_opb,th_quant_info *_qinfo)
{
    th_quant_base *base_mats;
    long           val;
    int32_t            nbase_mats;
    int32_t            sizes[64];
    int32_t            indices[64];
    int32_t            nbits;
    int32_t            bmi;
    int32_t            ci;
    int32_t            qti;
    int32_t            pli;
    int32_t            qri;
    int32_t            qi;
    int32_t            i;
    val=oc_pack_read(_opb,3);
    nbits=(int32_t)val;
    for(qi=0;qi<64;qi++){
        val=oc_pack_read(_opb,nbits);
        _qinfo->loop_filter_limits[qi]=(unsigned char)val;
    }
    val=oc_pack_read(_opb,4);
    nbits=(int32_t)val+1;
    for(qi=0;qi<64;qi++){
        val=oc_pack_read(_opb,nbits);
        _qinfo->ac_scale[qi]=(uint16_t)val;
    }
    val=oc_pack_read(_opb,4);
    nbits=(int32_t)val+1;
    for(qi=0;qi<64;qi++){
        val=oc_pack_read(_opb,nbits);
        _qinfo->dc_scale[qi]=(uint16_t)val;
    }
    val=oc_pack_read(_opb,9);
    nbase_mats=(int32_t)val+1;
    base_mats=malloc(nbase_mats*sizeof(base_mats[0]));
    if(base_mats==NULL)return TH_EFAULT;
    for(bmi=0;bmi<nbase_mats;bmi++){
        for(ci=0;ci<64;ci++){
            val=oc_pack_read(_opb,8);
            base_mats[bmi][ci]=(unsigned char)val;
        }
    }
    nbits=oc_ilog(nbase_mats-1);
    for(i=0;i<6;i++){
        th_quant_ranges *qranges;
        th_quant_base   *qrbms;
        int32_t             *qrsizes;
        qti=i/3;
        pli=i%3;
        qranges=_qinfo->qi_ranges[qti]+pli;
        if(i>0){
            val=oc_pack_read1(_opb);
            if(!val){
                int32_t qtj;
                int32_t plj;
                if(qti>0){
                    val=oc_pack_read1(_opb);
                    if(val){
                        qtj=qti-1;
                        plj=pli;
                    }
                    else{
                        qtj=(i-1)/3;
                        plj=(i-1)%3;
                    }
                }
                else{
                    qtj=(i-1)/3;
                    plj=(i-1)%3;
                }
                *qranges=*(_qinfo->qi_ranges[qtj]+plj);
                continue;
            }
        }
        val=oc_pack_read(_opb,nbits);
        indices[0]=(int32_t)val;
        for(qi=qri=0;qi<63;){
            val=oc_pack_read(_opb,oc_ilog(62-qi));
            sizes[qri]=(int32_t)val+1;
            qi+=(int32_t)val+1;
            val=oc_pack_read(_opb,nbits);
            indices[++qri]=(int32_t)val;
        }
        /*Note: The caller is responsible for cleaning up any partially
        constructed qinfo.*/
        if(qi>63){
            free(base_mats);
            return TH_EBADHEADER;
        }
        qranges->nranges=qri;
        qranges->sizes=qrsizes=(int32_t *)malloc(qri*sizeof(qrsizes[0]));
        if(qranges->sizes==NULL){
            /*Note: The caller is responsible for cleaning up any partially
            constructed qinfo.*/
            free(base_mats);
            return TH_EFAULT;
        }
        memcpy(qrsizes,sizes,qri*sizeof(qrsizes[0]));
        qrbms=(th_quant_base *)malloc((qri+1)*sizeof(qrbms[0]));
        if(qrbms==NULL){
            /*Note: The caller is responsible for cleaning up any partially
            constructed qinfo.*/
            free(base_mats);
            return TH_EFAULT;
        }
        qranges->base_matrices=(const th_quant_base *)qrbms;
        do{
            bmi=indices[qri];
            /*Note: The caller is responsible for cleaning up any partially
            constructed qinfo.*/
            if(bmi>=nbase_mats){
                free(base_mats);
                return TH_EBADHEADER;
            }
            memcpy(qrbms[qri],base_mats[bmi],sizeof(qrbms[qri]));
        }
        while(qri-->0);
    }
    free(base_mats);
    return 0;
}

void oc_quant_params_clear(th_quant_info *_qinfo)
{
    int32_t i;
    for(i=6;i-->0;){
        int32_t qti;
        int32_t pli;
        qti=i/3;
        pli=i%3;
        /*Clear any duplicate pointer references.*/
        if(i>0){
            int32_t qtj;
            int32_t plj;
            qtj=(i-1)/3;
            plj=(i-1)%3;
            if(_qinfo->qi_ranges[qti][pli].sizes==
                _qinfo->qi_ranges[qtj][plj].sizes){
                    _qinfo->qi_ranges[qti][pli].sizes=NULL;
            }
            if(_qinfo->qi_ranges[qti][pli].base_matrices==
                _qinfo->qi_ranges[qtj][plj].base_matrices){
                    _qinfo->qi_ranges[qti][pli].base_matrices=NULL;
            }
        }
        if(qti>0){
            if(_qinfo->qi_ranges[1][pli].sizes==
                _qinfo->qi_ranges[0][pli].sizes){
                    _qinfo->qi_ranges[1][pli].sizes=NULL;
            }
            if(_qinfo->qi_ranges[1][pli].base_matrices==
                _qinfo->qi_ranges[0][pli].base_matrices){
                    _qinfo->qi_ranges[1][pli].base_matrices=NULL;
            }
        }
        /*Now free all the non-duplicate storage.*/
        free((void *)_qinfo->qi_ranges[qti][pli].sizes);
        free((void *)_qinfo->qi_ranges[qti][pli].base_matrices);
    }
}

