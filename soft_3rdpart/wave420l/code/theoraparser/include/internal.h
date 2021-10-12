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
#if !defined(_internal_H)
# define _internal_H (1)
# include <stdlib.h>
# include <limits.h>
# if defined(HAVE_CONFIG_H)
#  include "config.h"
# endif
# include "codec.h"


# if defined(_MSC_VER)
/*Disable missing EMMS warnings.*/
#  pragma warning(disable:4799)
/*Thank you Microsoft, I know the order of operations.*/
#  pragma warning(disable:4554)
# endif
/*You, too, gcc.*/
# if defined(__GNUC_PREREQ)
#  if __GNUC_PREREQ(4,2)
#   pragma GCC diagnostic ignored "-Wparentheses"
#  endif
# endif

/*Some assembly constructs require aligned operands.
  The following macros are _only_ intended for structure member declarations.
  Although they will sometimes work on stack variables, gcc will often silently
   ignore them.
  A separate set of macros could be made for manual stack alignment, but we
   don't actually require it anywhere.*/
# if defined(OC_X86_ASM)||defined(OC_ARM_ASM)
#  if defined(__GNUC__)
#   define OC_ALIGN8(expr) expr __attribute__((aligned(8)))
#   define OC_ALIGN16(expr) expr __attribute__((aligned(16)))
#  elif defined(_MSC_VER)
#   define OC_ALIGN8(expr) __declspec (align(8)) expr
#   define OC_ALIGN16(expr) __declspec (align(16)) expr
#  else
#   error "Alignment macros required for this platform."
#  endif
# endif
# if !defined(OC_ALIGN8)
#  define OC_ALIGN8(expr) expr
# endif
# if !defined(OC_ALIGN16)
#  define OC_ALIGN16(expr) expr
# endif


/*Modern gcc (4.x) can compile the naive versions of min and max with cmov if
given an appropriate architecture, but the branchless bit-twiddling versions
are just as fast, and do not require any special target architecture.
Earlier gcc versions (3.x) compiled both code to the same assembly
instructions, because of the way they represented ((_b)>(_a)) internally.*/
#define OC_MAXI(_a,_b)      ((_a)-((_a)-(_b)&-((_b)>(_a))))
#define OC_MINI(_a,_b)      ((_a)+((_b)-(_a)&-((_b)<(_a))))
/*Clamps an integer into the given range.
If _a>_c, then the lower bound _a is respected over the upper bound _c (this
behavior is required to meet our documented API behavior).
_a: The lower bound.
_b: The value to clamp.
_c: The upper boud.*/
#define OC_CLAMPI(_a,_b,_c) (OC_MAXI(_a,OC_MINI(_b,_c)))
/*This has a chance of compiling branchless, and is just as fast as the
bit-twiddling method, which is slightly less portable, since it relies on a
sign-extended rightshift, which is not guaranteed by ANSI (but present on
every relevant platform).*/
#define OC_SIGNI(_a)        (((_a)>0)-((_a)<0))
/*Slightly more portable than relying on a sign-extended right-shift (which is
not guaranteed by ANSI), and just as fast, since gcc (3.x and 4.x both)
compile it into the right-shift anyway.*/
#define OC_SIGNMASK(_a)     (-((_a)<0))
/*Divides an integer by a power of two, truncating towards 0.
_dividend: The integer to divide.
_shift:    The non-negative power of two to divide by.
_rmask:    (1<<_shift)-1*/
#define OC_DIV_POW2(_dividend,_shift,_rmask)\
    ((_dividend)+(OC_SIGNMASK(_dividend)&(_rmask))>>(_shift))
/*Divides _x by 65536, truncating towards 0.*/
#define OC_DIV2_16(_x) OC_DIV_POW2(_x,16,0xFFFF)
/*Divides _x by 2, truncating towards 0.*/
#define OC_DIV2(_x) OC_DIV_POW2(_x,1,0x1)
/*Divides _x by 8, truncating towards 0.*/
#define OC_DIV8(_x) OC_DIV_POW2(_x,3,0x7)
/*Divides _x by 16, truncating towards 0.*/
#define OC_DIV16(_x) OC_DIV_POW2(_x,4,0xF)
/*Right shifts _dividend by _shift, adding _rval, and subtracting one for
negative dividends first.
When _rval is (1<<_shift-1), this is equivalent to division with rounding
ties away from zero.*/
#define OC_DIV_ROUND_POW2(_dividend,_shift,_rval)\
    ((_dividend)+OC_SIGNMASK(_dividend)+(_rval)>>(_shift))
/*Divides a _x by 2, rounding towards even numbers.*/
#define OC_DIV2_RE(_x) ((_x)+((_x)>>1&1)>>1)
/*Divides a _x by (1<<(_shift)), rounding towards even numbers.*/
#define OC_DIV_POW2_RE(_x,_shift) \
    ((_x)+((_x)>>(_shift)&1)+((1<<(_shift))-1>>1)>>(_shift))
/*Swaps two integers _a and _b if _a>_b.*/
#define OC_SORT2I(_a,_b) \
    do{ \
    int32_t t__; \
    t__=((_a)^(_b))&-((_b)<(_a)); \
    (_a)^=t__; \
    (_b)^=t__; \
    } \
    while(0)

/*Accesses one of four (signed) bytes given an index.
This can be used to avoid small lookup tables.*/
#define OC_BYTE_TABLE32(_a,_b,_c,_d,_i) \
    ((signed char) \
    (((_a)&0xFF|((_b)&0xFF)<<8|((_c)&0xFF)<<16|((_d)&0xFF)<<24)>>(_i)*8))
/*Accesses one of eight (unsigned) nibbles given an index.
This can be used to avoid small lookup tables.*/
#define OC_UNIBBLE_TABLE32(_a,_b,_c,_d,_e,_f,_g,_h,_i) \
    ((((_a)&0xF|((_b)&0xF)<<4|((_c)&0xF)<<8|((_d)&0xF)<<12| \
    ((_e)&0xF)<<16|((_f)&0xF)<<20|((_g)&0xF)<<24|((_h)&0xF)<<28)>>(_i)*4)&0xF)



/*All of these macros should expect floats as arguments.*/
#define OC_MAXF(_a,_b)      ((_a)<(_b)?(_b):(_a))
#define OC_MINF(_a,_b)      ((_a)>(_b)?(_b):(_a))
#define OC_CLAMPF(_a,_b,_c) (OC_MINF(_a,OC_MAXF(_b,_c)))
#define OC_FABSF(_f)        ((float)fabs(_f))
#define OC_SQRTF(_f)        ((float)sqrt(_f))
#define OC_POWF(_b,_e)      ((float)pow(_b,_e))
#define OC_LOGF(_f)         ((float)log(_f))
#define OC_IFLOORF(_f)      ((int32_t)floor(_f))
#define OC_ICEILF(_f)       ((int32_t)ceil(_f))



/*This library's version.*/
# define OC_VENDOR_STRING "Xiph.Org libtheora 1.2.0alpha 20100923 (Ptalarbvorm)"

/*Theora bitstream version.*/
# define TH_VERSION_MAJOR (3)
# define TH_VERSION_MINOR (2)
# define TH_VERSION_SUB   (1)
# define TH_VERSION_CHECK(_info,_maj,_min,_sub) \
 ((_info)->version_major>(_maj)||(_info)->version_major==(_maj)&& \
 ((_info)->version_minor>(_min)||(_info)->version_minor==(_min)&& \
 (_info)->version_subminor>=(_sub)))



/*A map from the index in the zig zag scan to the coefficient number in a
   block.*/
extern const unsigned char OC_FZIG_ZAG[128];
/*A map from the coefficient number in a block to its index in the zig zag
   scan.*/
extern const unsigned char OC_IZIG_ZAG[64];
/*A map from physical macro block ordering to bitstream macro block
   ordering within a super block.*/
extern const unsigned char OC_MB_MAP[2][2];
/*A list of the indices in the oc_mb_map array that can be valid for each of
   the various chroma decimation types.*/
extern const unsigned char OC_MB_MAP_IDXS[TH_PF_NFORMATS][12];
/*The number of indices in the oc_mb_map array that can be valid for each of
   the various chroma decimation types.*/
extern const unsigned char OC_MB_MAP_NIDXS[TH_PF_NFORMATS];


int32_t   oc_ilog(uint32_t _v);
void* oc_aligned_malloc(size_t _sz,size_t _align);
void  oc_alignedsys_mem_free(void *_ptr);

#endif
