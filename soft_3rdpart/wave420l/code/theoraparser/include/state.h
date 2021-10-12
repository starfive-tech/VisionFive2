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
#if !defined(_state_H)
# define _state_H (1)
# include "internal.h"
# include "dequant.h"
# include "huffman.h"


/*A single quadrant of the map from a super block to fragment numbers.*/
typedef int32_t             oc_sb_map_quad[4];
/*A map from a super block to fragment numbers.*/
typedef oc_sb_map_quad  oc_sb_map[4];
/*A single plane of the map from a macro block to fragment numbers.*/
typedef int32_t             oc_mb_map_plane[4];
/*A map from a macro block to fragment numbers.*/
typedef oc_mb_map_plane oc_mb_map[3];
/*A motion vector.*/
typedef int16_t     oc_mv;
typedef int16_t     oc_qcm[64];

typedef struct oc_sb_flags              oc_sb_flags;
typedef struct oc_fragment              oc_fragment;
typedef struct oc_fragment_plane        oc_fragment_plane;
typedef struct oc_base_opt_vtable       oc_base_opt_vtable;
typedef struct oc_base_opt_data         oc_base_opt_data;
typedef struct oc_theora_state          oc_theora_state;


/*A keyframe.*/
# define OC_INTRA_FRAME (0)
/*A predicted frame.*/
# define OC_INTER_FRAME (1)
/*A frame of unknown type (frame type decision has not yet been made).*/
# define OC_UNKWN_FRAME (-1)

/*The amount of padding to add to the reconstructed frame buffers on all
sides.
This is used to allow unrestricted motion vectors without special casing.
This must be a multiple of 2.*/
# define OC_UMV_PADDING (16)

/*Frame classification indices.*/
/*The previous golden frame.*/
# define OC_FRAME_GOLD      (0)
/*The previous frame.*/
# define OC_FRAME_PREV      (1)
/*The current frame.*/
# define OC_FRAME_SELF      (2)

/*The input or output buffer.*/
# define OC_FRAME_IO        (3)
/*Uncompressed prev golden frame.*/
# define OC_FRAME_GOLD_ORIG (4)
/*Uncompressed previous frame. */
# define OC_FRAME_PREV_ORIG (5)

/*Macroblock modes.*/
/*Macro block is invalid: It is never coded.*/
# define OC_MODE_INVALID        (-1)
/*Encoded difference from the same macro block in the previous frame.*/
# define OC_MODE_INTER_NOMV     (0)
/*Encoded with no motion compensated prediction.*/
# define OC_MODE_INTRA          (1)
/*Encoded difference from the previous frame offset by the given motion
vector.*/
# define OC_MODE_INTER_MV       (2)
/*Encoded difference from the previous frame offset by the last coded motion
vector.*/
# define OC_MODE_INTER_MV_LAST  (3)
/*Encoded difference from the previous frame offset by the second to last
coded motion vector.*/
# define OC_MODE_INTER_MV_LAST2 (4)
/*Encoded difference from the same macro block in the previous golden
frame.*/
# define OC_MODE_GOLDEN_NOMV    (5)
/*Encoded difference from the previous golden frame offset by the given motion
vector.*/
# define OC_MODE_GOLDEN_MV      (6)
/*Encoded difference from the previous frame offset by the individual motion
vectors given for each block.*/
# define OC_MODE_INTER_MV_FOUR  (7)
/*The number of (coded) modes.*/
# define OC_NMODES              (8)

/*Determines the reference frame used for a given MB mode.*/
# define OC_FRAME_FOR_MODE(_x) \
    OC_UNIBBLE_TABLE32(OC_FRAME_PREV,OC_FRAME_SELF,OC_FRAME_PREV,OC_FRAME_PREV, \
    OC_FRAME_PREV,OC_FRAME_GOLD,OC_FRAME_GOLD,OC_FRAME_PREV,(_x))

/*Constants for the packet state machine common between encoder and decoder.*/

/*Next packet to emit/read: Codec info header.*/
# define OC_PACKET_INFO_HDR    (-3)
/*Next packet to emit/read: Comment header.*/
# define OC_PACKET_COMMENT_HDR (-2)
/*Next packet to emit/read: Codec setup header.*/
# define OC_PACKET_SETUP_HDR   (-1)
/*No more packets to emit/read.*/
# define OC_PACKET_DONE        (INT_MAX)



#define OC_MV(_x,_y)         ((oc_mv)((_x)&0xFF|(_y)<<8))
#define OC_MV_X(_mv)         ((signed char)(_mv))
#define OC_MV_Y(_mv)         ((_mv)>>8)
#define OC_MV_ADD(_mv1,_mv2) \
    OC_MV(OC_MV_X(_mv1)+OC_MV_X(_mv2), \
    OC_MV_Y(_mv1)+OC_MV_Y(_mv2))
#define OC_MV_SUB(_mv1,_mv2) \
    OC_MV(OC_MV_X(_mv1)-OC_MV_X(_mv2), \
    OC_MV_Y(_mv1)-OC_MV_Y(_mv2))



/*Super blocks are 32x32 segments of pixels in a single color plane indexed
in image order.
Internally, super blocks are broken up into four quadrants, each of which
contains a 2x2 pattern of blocks, each of which is an 8x8 block of pixels.
Quadrants, and the blocks within them, are indexed in a special order called
a "Hilbert curve" within the super block.

In order to differentiate between the Hilbert-curve indexing strategy and
the regular image order indexing strategy, blocks indexed in image order
are called "fragments".
Fragments are indexed in image order, left to right, then bottom to top,
from Y' plane to Cb plane to Cr plane.

The co-located fragments in all image planes corresponding to the location
of a single quadrant of a luma plane super block form a macro block.
Thus there is only a single set of macro blocks for all planes, each of which
contains between 6 and 12 fragments, depending on the pixel format.
Therefore macro block information is kept in a separate set of arrays from
super blocks to avoid unused space in the other planes.
The lists are indexed in super block order.
That is, the macro block corresponding to the macro block mbi in (luma plane)
super block sbi is at index (sbi<<2|mbi).
Thus the number of macro blocks in each dimension is always twice the number
of super blocks, even when only an odd number fall inside the coded frame.
These "extra" macro blocks are just an artifact of our internal data layout,
and not part of the coded stream; they are flagged with a negative MB mode.*/



/*Super block information.*/
struct oc_sb_flags{
    unsigned char coded_fully:1;
    unsigned char coded_partially:1;
    unsigned char quad_valid:4;
};



/*Fragment information.*/
struct oc_fragment{
    /*A flag indicating whether or not this fragment is coded.*/
    uint32_t   coded:1;
    /*A flag indicating that this entire fragment lies outside the displayable
    region of the frame.
    Note the contrast with an invalid macro block, which is outside the coded
    frame, not just the displayable one.
    /*The index of the quality index used for this fragment's AC coefficients.*/
    uint32_t   qii:6;
    /*The mode of the macroblock this fragment belongs to.*/
    uint32_t   mb_mode:3;
    /*The prediction-corrected DC component.
    Note that the C standard requires an explicit signed keyword for bitfield
    types, since some compilers may treat them as unsigned without it.*/
    int32_t dc:16;
};

/*A description of each fragment plane.*/
struct oc_fragment_plane{
    /*The number of fragments in the horizontal direction.*/
    int32_t       nhfrags;
    /*The number of fragments in the vertical direction.*/
    int32_t       nvfrags;
    /*The offset of the first fragment in the plane.*/
    int32_t       froffset;
    /*The total number of fragments in the plane.*/
    int32_t       nfrags;
    /*The number of super blocks in the horizontal direction.*/
    uint32_t  nhsbs;
    /*The number of super blocks in the vertical direction.*/
    uint32_t  nvsbs;
    /*The offset of the first super block in the plane.*/
    uint32_t  sboffset;
    /*The total number of super blocks in the plane.*/
    uint32_t  nsbs;
};


/*The shared (encoder and decoder) tables that vary according to which variants
of the above functions are used.*/
struct oc_base_opt_data{
    const unsigned char *dct_fzig_zag;
};


/*State information common to both the encoder and decoder.*/
struct oc_theora_state{
    /*The stream information.*/
    th_info             info;
    /*CPU flags to detect the presence of extended instruction sets.*/
    uint32_t        cpu_flags;
    /*The fragment plane descriptions.*/
    oc_fragment_plane   fplanes[3];
    /*The list of fragments, indexed in image order.*/
    oc_fragment        *frags;
    /*The the offset into the reference frame buffer to the upper-left pixel of
    each fragment.*/
    int32_t                *frag_buf_offs;
    /*The motion vector for each fragment.*/
    oc_mv              *frag_mvs;
    /*The coeficients for each fragment.*/
    oc_qcm             *frag_qcm;
    /*The end-of-block position for each fragment.*/
    uint8_t            *frag_eob;
    /*The total number of fragments in a single frame.*/
    int32_t                 nfrags;
    /*The list of super block maps, indexed in image order.*/
    oc_sb_map          *sb_maps;
    /*The list of super block flags, indexed in image order.*/
    oc_sb_flags        *sb_flags;
    /*The total number of super blocks in a single frame.*/
    uint32_t            nsbs;
    /*The fragments from each color plane that belong to each macro block.
    Fragments are stored in image order (left to right then top to bottom).
    When chroma components are decimated, the extra fragments have an index of
    -1.*/
    oc_mb_map          *mb_maps;
    /*The fragments from each color plane that belong to each macro block.
    Fragments are stored in raster order (left to right then top to bottom).
    When chroma components are decimated, the extra fragments have an index of
    -1.*/
    oc_mb_map          *mb_maps_rater_order;
    /*The list of macro block modes.
    A negative number indicates the macro block lies entirely outside the
    coded frame.*/
    signed char        *mb_modes;
    /*The number of macro blocks in the X direction.*/
    uint32_t            nhmbs;
    /*The number of macro blocks in the Y direction.*/
    uint32_t            nvmbs;
    /*The total number of macro blocks.*/
    size_t              nmbs;
    /*The list of coded fragments, in coded order.
    Uncoded fragments are stored in reverse order from the end of the list.*/
    int32_t                *coded_fragis;
    /*The number of coded fragments in each plane.*/
    int32_t                 ncoded_fragis[3];
    /*The number of sum coded fragments in each plane.*/
    int32_t                 prev_coded_offset;
    /*The total number of coded fragments.*/
    int32_t                 ntotal_coded_fragis;
    /*The frame number of the last keyframe.*/
    int64_t             keyframe_num;
    /*The frame number of the current frame.*/
    int64_t             curframe_num;
    /*The granpos of the current frame.*/
    int64_t             granpos;
    /*The type of the current frame.*/
    signed char         frame_type;
    /*The bias to add to the frame count when computing granule positions.*/
    unsigned char       granpos_bias;
    /*The number of quality indices used in the current frame.*/
    unsigned char       nqis;
    /*The quality indices of the current frame.*/
    unsigned char       qis[3];
    /*The dequantization tables, stored in zig-zag order, and indexed by
    qi, pli, qti, and zzi.*/
    uint16_t       *dequant_tables[64][3][2];
    OC_ALIGN16(oc_quant_table      dequant_table_data[64][3][2]);
    /*Loop filter strength parameters.*/
    unsigned char       loop_filter_limits[64];

    const unsigned char *dct_fzig_zag;

};



/*The function type used to fill in the chroma plane motion vectors for a
macro block when 4 different motion vectors are specified in the luma
plane.
_cbmvs: The chroma block-level motion vectors to fill in.
_lmbmv: The luma macro-block level motion vector to fill in for use in
prediction.
_lbmvs: The luma block-level motion vectors.*/
typedef void (*oc_set_chroma_mvs_func)(oc_mv _cbmvs[4],const oc_mv _lbmvs[4]);



/*A table of functions used to fill in the Cb,Cr plane motion vectors for a
macro block when 4 different motion vectors are specified in the luma
plane.*/
extern const oc_set_chroma_mvs_func OC_SET_CHROMA_MVS_TABLE[TH_PF_NFORMATS];



int32_t oc_state_init(oc_theora_state *_state,const th_info *_info,int32_t _nrefs);
void oc_state_clear(oc_theora_state *_state);
int32_t oc_state_mbi_for_pos(oc_theora_state *_state,int32_t _mbx,int32_t _mby);


#endif
