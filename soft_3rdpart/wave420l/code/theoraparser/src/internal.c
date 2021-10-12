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
#include <limits.h>
#include <string.h>
#include "internal.h"
#include "bitpack.h"



/*A map from the index in the zig zag scan to the coefficient number in a
block.
All zig zag indices beyond 63 are sent to coefficient 64, so that zero runs
past the end of a block in bogus streams get mapped to a known location.*/
const unsigned char OC_FZIG_ZAG[128]={
    0, 1, 8,16, 9, 2, 3,10,
    17,24,32,25,18,11, 4, 5,
    12,19,26,33,40,48,41,34,
    27,20,13, 6, 7,14,21,28,
    35,42,49,56,57,50,43,36,
    29,22,15,23,30,37,44,51,
    58,59,52,45,38,31,39,46,
    53,60,61,54,47,55,62,63,
    64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64
};

/*A map from the coefficient number in a block to its index in the zig zag
scan.*/
const unsigned char OC_IZIG_ZAG[64]={
    0, 1, 5, 6,14,15,27,28,
    2, 4, 7,13,16,26,29,42,
    3, 8,12,17,25,30,41,43,
    9,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63
};

/*A map from physical macro block ordering to bitstream macro block
ordering within a super block.*/
const unsigned char OC_MB_MAP[2][2]={{0,3},{1,2}};

/*A list of the indices in the oc_mb.map array that can be valid for each of
the various chroma decimation types.*/
const unsigned char OC_MB_MAP_IDXS[TH_PF_NFORMATS][12]={
    {0,1,2,3,4,8},
    {0,1,2,3,4,5,8,9},
    {0,1,2,3,4,6,8,10},
    {0,1,2,3,4,5,6,7,8,9,10,11}
};

/*The number of indices in the oc_mb.map array that can be valid for each of
the various chroma decimation types.*/
const unsigned char OC_MB_MAP_NIDXS[TH_PF_NFORMATS]={6,8,8,12};

/*The number of extra bits that are coded with each of the DCT tokens.
Each DCT token has some fixed number of additional bits (possibly 0) stored
after the token itself, containing, for example, coefficient magnitude,
sign bits, etc.*/
const unsigned char OC_DCT_TOKEN_EXTRA_BITS[TH_NDCT_TOKENS]={
    0,0,0,2,3,4,12,3,6,
    0,0,0,0,
    1,1,1,1,2,3,4,5,6,10,
    1,1,1,1,1,3,4,
    2,3
};



int32_t oc_ilog(uint32_t _v)
{
    int32_t ret;
    for(ret=0;_v;ret++)_v>>=1;
    return ret;
}

void *oc_aligned_malloc(size_t _sz,size_t _align)
{
    unsigned char *p;
    if(_align>UCHAR_MAX||(_align&_align-1)||_sz>~(size_t)0-_align)return NULL;
    p=(unsigned char *)malloc(_sz+_align);
    if(p!=NULL)
    {
        int32_t offs;
        offs=((p-(unsigned char *)0)-1&_align-1);
        p[offs]=offs;
        p+=offs+1;
    }
    return p;
}

void oc_alignedsys_mem_free(void *_ptr)
{
    unsigned char *p;
    p=(unsigned char *)_ptr;
    if(p!=NULL){
        int32_t offs;
        offs=*--p;
        free(p-offs);
    }
}

