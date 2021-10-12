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
#include "state.h"
#include "ocintrin.h"
#if defined(OC_DUMP_IMAGES)
# include <stdio.h>
# include "png.h"
#endif


/*The function used to fill in the chroma plane motion vectors for a macro
block when 4 different motion vectors are specified in the luma plane.
This version is for use with chroma decimated in the X and Y directions
(4:2:0).
_cbmvs: The chroma block-level motion vectors to fill in.
_lbmvs: The luma block-level motion vectors.*/
static void oc_set_chroma_mvs00(oc_mv _cbmvs[4],const oc_mv _lbmvs[4])
{
    int32_t dx;
    int32_t dy;
    dx=OC_MV_X(_lbmvs[0])+OC_MV_X(_lbmvs[1])
        +OC_MV_X(_lbmvs[2])+OC_MV_X(_lbmvs[3]);
    dy=OC_MV_Y(_lbmvs[0])+OC_MV_Y(_lbmvs[1])
        +OC_MV_Y(_lbmvs[2])+OC_MV_Y(_lbmvs[3]);
    _cbmvs[0]=OC_MV(OC_DIV_ROUND_POW2(dx,2,2),OC_DIV_ROUND_POW2(dy,2,2));
}

/*The function used to fill in the chroma plane motion vectors for a macro
block when 4 different motion vectors are specified in the luma plane.
This version is for use with chroma decimated in the Y direction.
_cbmvs: The chroma block-level motion vectors to fill in.
_lbmvs: The luma block-level motion vectors.*/
static void oc_set_chroma_mvs01(oc_mv _cbmvs[4],const oc_mv _lbmvs[4]){
    int32_t dx;
    int32_t dy;
    dx=OC_MV_X(_lbmvs[0])+OC_MV_X(_lbmvs[2]);
    dy=OC_MV_Y(_lbmvs[0])+OC_MV_Y(_lbmvs[2]);
    _cbmvs[0]=OC_MV(OC_DIV_ROUND_POW2(dx,1,1),OC_DIV_ROUND_POW2(dy,1,1));
    dx=OC_MV_X(_lbmvs[1])+OC_MV_X(_lbmvs[3]);
    dy=OC_MV_Y(_lbmvs[1])+OC_MV_Y(_lbmvs[3]);
    _cbmvs[1]=OC_MV(OC_DIV_ROUND_POW2(dx,1,1),OC_DIV_ROUND_POW2(dy,1,1));
}

/*The function used to fill in the chroma plane motion vectors for a macro
block when 4 different motion vectors are specified in the luma plane.
This version is for use with chroma decimated in the X direction (4:2:2).
_cbmvs: The chroma block-level motion vectors to fill in.
_lbmvs: The luma block-level motion vectors.*/
static void oc_set_chroma_mvs10(oc_mv _cbmvs[4],const oc_mv _lbmvs[4])
{
    int32_t dx;
    int32_t dy;
    dx=OC_MV_X(_lbmvs[0])+OC_MV_X(_lbmvs[1]);
    dy=OC_MV_Y(_lbmvs[0])+OC_MV_Y(_lbmvs[1]);
    _cbmvs[0]=OC_MV(OC_DIV_ROUND_POW2(dx,1,1),OC_DIV_ROUND_POW2(dy,1,1));
    dx=OC_MV_X(_lbmvs[2])+OC_MV_X(_lbmvs[3]);
    dy=OC_MV_Y(_lbmvs[2])+OC_MV_Y(_lbmvs[3]);
    _cbmvs[2]=OC_MV(OC_DIV_ROUND_POW2(dx,1,1),OC_DIV_ROUND_POW2(dy,1,1));
}

/*The function used to fill in the chroma plane motion vectors for a macro
block when 4 different motion vectors are specified in the luma plane.
This version is for use with no chroma decimation (4:4:4).
_cbmvs: The chroma block-level motion vectors to fill in.
_lmbmv: The luma macro-block level motion vector to fill in for use in
prediction.
_lbmvs: The luma block-level motion vectors.*/
static void oc_set_chroma_mvs11(oc_mv _cbmvs[4],const oc_mv _lbmvs[4])
{
    _cbmvs[0]=_lbmvs[0];
    _cbmvs[1]=_lbmvs[1];
    _cbmvs[2]=_lbmvs[2];
    _cbmvs[3]=_lbmvs[3];
}

/*A table of functions used to fill in the chroma plane motion vectors for a
macro block when 4 different motion vectors are specified in the luma
plane.*/
const oc_set_chroma_mvs_func OC_SET_CHROMA_MVS_TABLE[TH_PF_NFORMATS]={
    (oc_set_chroma_mvs_func)oc_set_chroma_mvs00,
    (oc_set_chroma_mvs_func)oc_set_chroma_mvs01,
    (oc_set_chroma_mvs_func)oc_set_chroma_mvs10,
    (oc_set_chroma_mvs_func)oc_set_chroma_mvs11
};



/*Returns the fragment index of the top-left block in a macro block.
This can be used to test whether or not the whole macro block is valid.
_sb_map: The super block map.
_quadi:  The quadrant number.
Return: The index of the fragment of the upper left block in the macro
block, or -1 if the block lies outside the coded frame.*/
static int32_t oc_sb_quad_top_left_frag(oc_sb_map_quad _sb_map[4],int32_t _quadi)
{
    /*It so happens that under the Hilbert curve ordering described below, the
    upper-left block in each macro block is at index 0, except in macro block
    3, where it is at index 2.*/
    return _sb_map[_quadi][_quadi&_quadi<<1];
}

/*Fills in the mapping from block positions to fragment numbers for a single
color plane.
This function also fills in the "valid" flag of each quadrant in the super
block flags.
_sb_maps:  The array of super block maps for the color plane.
_sb_flags: The array of super block flags for the color plane.
_frag0:    The index of the first fragment in the plane.
_hfrags:   The number of horizontal fragments in a coded frame.
_vfrags:   The number of vertical fragments in a coded frame.*/
static void oc_sb_create_plane_mapping(oc_sb_map _sb_maps[],
    oc_sb_flags _sb_flags[],int32_t _frag0,int32_t _hfrags,int32_t _vfrags)
{
    /*Contains the (macro_block,block) indices for a 4x4 grid of
    fragments.
    The pattern is a 4x4 Hilbert space-filling curve.
    A Hilbert curve has the nice property that as the curve grows larger, its
    fractal dimension approaches 2.
    The intuition is that nearby blocks in the curve are also close spatially,
    with the previous element always an immediate neighbor, so that runs of
    blocks should be well correlated.*/
    static const int32_t SB_MAP[4][4][2]={
        {{0,0},{0,1},{3,2},{3,3}},
        {{0,3},{0,2},{3,1},{3,0}},
        {{1,0},{1,3},{2,0},{2,3}},
        {{1,1},{1,2},{2,1},{2,2}}
    };
    uint32_t       sbi;
    int32_t        yfrag;
    int32_t        y;
    sbi=0;
    yfrag=_frag0;
    for(y=0;;y+=4)
    {
        int32_t imax;
        int32_t x;
        /*Figure out how many columns of blocks in this super block lie within the
        image.*/
        imax=_vfrags-y;
        if(imax>4)imax=4;
        else if(imax<=0)break;
        for(x=0;;x+=4,sbi++)
        {
            int32_t  xfrag;
            int32_t  jmax;
            int32_t  quadi;
            int32_t  i;
            /*Figure out how many rows of blocks in this super block lie within the
            image.*/
            jmax=_hfrags-x;
            if(jmax>4)jmax=4;
            else if(jmax<=0)break;
            /*By default, set all fragment indices to -1.*/
            memset(_sb_maps[sbi][0],0xFF,sizeof(_sb_maps[sbi]));
            /*Fill in the fragment map for this super block.*/
            xfrag=yfrag+x;
            for(i=0;i<imax;i++){
                int32_t j;
                for(j=0;j<jmax;j++)
                {
                    _sb_maps[sbi][SB_MAP[i][j][0]][SB_MAP[i][j][1]]=xfrag+j;
                }
                xfrag+=_hfrags;
            }
            /*Mark which quadrants of this super block lie within the image.*/
            for(quadi=0;quadi<4;quadi++)       {
                _sb_flags[sbi].quad_valid|=
                    (oc_sb_quad_top_left_frag(_sb_maps[sbi],quadi)>=0)<<quadi;
            }
        }
        yfrag+=_hfrags<<2;
    }
}

/*Fills in the Y plane fragment map for a macro block given the fragment
coordinates of its upper-left hand corner.
_mb_map:    The macro block map to fill.
_fplane: The description of the Y plane.
_xfrag0: The X location of the upper-left hand fragment in the luma plane.
_yfrag0: The Y location of the upper-left hand fragment in the luma plane.*/
static void oc_mb_fill_ymapping(oc_mb_map_plane _mb_map[3],
    const oc_fragment_plane *_fplane,int32_t _xfrag0,int32_t _yfrag0)
{
    int32_t i;
    int32_t j;
    for(i=0;i<2;i++)for(j=0;j<2;j++){
        _mb_map[0][i<<1|j]=(_yfrag0+i)*(int32_t)_fplane->nhfrags+_xfrag0+j;
    }
}

static void oc_mb_fill_ymapping_raster(oc_mb_map_plane _mb_map[3], 
    const oc_fragment_plane *_fplane,int32_t _xfrag0,int32_t _yfrag0)
{
    _mb_map[0][0]=(_yfrag0+1)*(int32_t)_fplane->nhfrags+_xfrag0;
    _mb_map[0][1]=(_yfrag0+1)*(int32_t)_fplane->nhfrags+_xfrag0+1;
    _mb_map[0][2]=(_yfrag0)*(int32_t)_fplane->nhfrags+_xfrag0;
    _mb_map[0][3]=(_yfrag0)*(int32_t)_fplane->nhfrags+_xfrag0+1;
}

/*Fills in the chroma plane fragment maps for a macro block.
This version is for use with chroma decimated in the X and Y directions
(4:2:0).
_mb_map:  The macro block map to fill.
_fplanes: The descriptions of the fragment planes.
_xfrag0:  The X location of the upper-left hand fragment in the luma plane.
_yfrag0:  The Y location of the upper-left hand fragment in the luma plane.*/
static void oc_mb_fill_cmapping00(oc_mb_map_plane _mb_map[3],
    const oc_fragment_plane _fplanes[3],int32_t _xfrag0,int32_t _yfrag0)
{
    int32_t fragi;
    _xfrag0>>=1;
    _yfrag0>>=1;
    fragi=_yfrag0*(int32_t)_fplanes[1].nhfrags+_xfrag0;
    _mb_map[1][0]=fragi+_fplanes[1].froffset;
    _mb_map[2][0]=fragi+_fplanes[2].froffset;
}

/*Fills in the chroma plane fragment maps for a macro block.
This version is for use with chroma decimated in the Y direction.
_mb_map:  The macro block map to fill.
_fplanes: The descriptions of the fragment planes.
_xfrag0:  The X location of the upper-left hand fragment in the luma plane.
_yfrag0:  The Y location of the upper-left hand fragment in the luma plane.*/
static void oc_mb_fill_cmapping01(oc_mb_map_plane _mb_map[3],
    const oc_fragment_plane _fplanes[3],int32_t _xfrag0,int32_t _yfrag0)
{
    int32_t  fragi;
    int32_t  j;
    _yfrag0>>=1;
    fragi=_yfrag0*(int32_t)_fplanes[1].nhfrags+_xfrag0;
    for(j=0;j<2;j++){
        _mb_map[1][j]=fragi+_fplanes[1].froffset;
        _mb_map[2][j]=fragi+_fplanes[2].froffset;
        fragi++;
    }
}

/*Fills in the chroma plane fragment maps for a macro block.
This version is for use with chroma decimated in the X direction (4:2:2).
_mb_map:  The macro block map to fill.
_fplanes: The descriptions of the fragment planes.
_xfrag0:  The X location of the upper-left hand fragment in the luma plane.
_yfrag0:  The Y location of the upper-left hand fragment in the luma plane.*/
static void oc_mb_fill_cmapping10(oc_mb_map_plane _mb_map[3],
    const oc_fragment_plane _fplanes[3],int32_t _xfrag0,int32_t _yfrag0)
{
    int32_t  fragi;
    int32_t  i;
    _xfrag0>>=1;
    fragi=_yfrag0*(int32_t)_fplanes[1].nhfrags+_xfrag0;
    for(i=0;i<2;i++){
        _mb_map[1][i<<1]=fragi+_fplanes[1].froffset;
        _mb_map[2][i<<1]=fragi+_fplanes[2].froffset;
        fragi+=_fplanes[1].nhfrags;
    }
}

/*Fills in the chroma plane fragment maps for a macro block.
This version is for use with no chroma decimation (4:4:4).
This uses the already filled-in luma plane values.
_mb_map:  The macro block map to fill.
_fplanes: The descriptions of the fragment planes.*/
static void oc_mb_fill_cmapping11(oc_mb_map_plane _mb_map[3],
    const oc_fragment_plane _fplanes[3])
{
    int32_t k;
    for(k=0;k<4;k++){
        _mb_map[1][k]=_mb_map[0][k]+_fplanes[1].froffset;
        _mb_map[2][k]=_mb_map[0][k]+_fplanes[2].froffset;
    }
}

/*The function type used to fill in the chroma plane fragment maps for a
macro block.
_mb_map:  The macro block map to fill.
_fplanes: The descriptions of the fragment planes.
_xfrag0:  The X location of the upper-left hand fragment in the luma plane.
_yfrag0:  The Y location of the upper-left hand fragment in the luma plane.*/
typedef void (*oc_mb_fill_cmapping_func)(oc_mb_map_plane _mb_map[3],
    const oc_fragment_plane _fplanes[3],int32_t _xfrag0,int32_t _yfrag0);

/*A table of functions used to fill in the chroma plane fragment maps for a
macro block for each type of chrominance decimation.*/
static const oc_mb_fill_cmapping_func OC_MB_FILL_CMAPPING_TABLE[4]={
    oc_mb_fill_cmapping00,
    oc_mb_fill_cmapping01,
    oc_mb_fill_cmapping10,
    (oc_mb_fill_cmapping_func)oc_mb_fill_cmapping11
};

/*Fills in the mapping from macro blocks to their corresponding fragment
numbers in each plane.
_mb_maps:   The list of macro block maps.
_mb_modes:  The list of macro block modes; macro blocks completely outside
the coded region are marked invalid.
_fplanes:   The descriptions of the fragment planes.
_pixel_fmt: The chroma decimation type.*/
static void oc_mb_create_mapping(oc_mb_map _mb_maps[], oc_mb_map _mb_maps_rater_order[],
    signed char _mb_modes[],const oc_fragment_plane _fplanes[3],int32_t _pixel_fmt)
{
    oc_mb_fill_cmapping_func  mb_fill_cmapping;
    int32_t    sbi, mbi, mbx, mby, ymb, xmb;
    int32_t                       y;

    mb_fill_cmapping=OC_MB_FILL_CMAPPING_TABLE[_pixel_fmt];
    /*Loop through the luma plane super blocks.*/
    for(sbi=y=0;y<_fplanes[0].nvfrags;y+=4){
        int32_t x;
        for(x=0;x<_fplanes[0].nhfrags;x+=4,sbi++){
            int32_t ymb;
            /*Loop through the macro blocks in each super block in display order.*/
            for(ymb=0;ymb<2;ymb++){
                int32_t xmb;
                for(xmb=0;xmb<2;xmb++){
                    uint32_t     mbi;
                    int32_t      mbx;
                    int32_t      mby;
                    mbi=sbi<<2|OC_MB_MAP[ymb][xmb];
                    mbx=x|xmb<<1;
                    mby=y|ymb<<1;
                    /*Initialize fragment indices to -1.*/
                    memset(_mb_maps[mbi],0xFF,sizeof(_mb_maps[mbi]));
                    /*Make sure this macro block is within the encoded region.*/
                    if(mbx>=_fplanes[0].nhfrags||mby>=_fplanes[0].nvfrags){
                        _mb_modes[mbi]=OC_MODE_INVALID;
                        continue;
                    }
                    /*Fill in the fragment indices for the luma plane.*/
                    oc_mb_fill_ymapping(_mb_maps[mbi],_fplanes,mbx,mby);
                    /*Fill in the fragment indices for the chroma planes.*/
                    (*mb_fill_cmapping)(_mb_maps[mbi],_fplanes,mbx,mby);
                }
            }
        }
    }

    /*Loop through the luma plane super blocks.*/
    for(mbi=ymb=0; ymb<_fplanes[0].nvfrags;ymb+=2)
    {
        for(xmb=0; xmb<_fplanes[0].nhfrags; xmb+=2 ,mbi++)
        {
            /*Loop through the macro blocks in each super block in display order.*/
            mbx=xmb;
            mby=ymb;

            /*Initialize fragment indices to -1.*/
            memset(_mb_maps_rater_order[mbi],0xFF,sizeof(_mb_maps_rater_order[mbi]));
            /*Make sure this macro block is within the encoded region.*/
            if(mbx>=_fplanes[0].nhfrags||mby>=_fplanes[0].nvfrags)
            {
                _mb_modes[mbi]=OC_MODE_INVALID;
                continue;
            }
            /*Fill in the fragment indices for the luma plane.*/
            oc_mb_fill_ymapping_raster(_mb_maps_rater_order[mbi],_fplanes,mbx,mby);
            /*Fill in the fragment indices for the chroma planes.*/
            (*mb_fill_cmapping)(_mb_maps_rater_order[mbi],_fplanes,mbx,mby);
        }
    }
}

static int32_t oc_state_frarray_init(oc_theora_state *_state)
{
    int32_t       yhfrags;
    int32_t       yvfrags;
    int32_t       chfrags;
    int32_t       cvfrags;
    int32_t       yfrags;
    int32_t       cfrags;
    int32_t       nfrags;
    uint32_t      yhsbs;
    uint32_t      yvsbs;
    uint32_t      chsbs;
    uint32_t      cvsbs;
    uint32_t      ysbs;
    uint32_t      csbs;
    uint32_t      nsbs;
    size_t        nmbs;
    int32_t       hdec;
    int32_t       vdec;
    int32_t       pli;
    /*Figure out the number of fragments in each plane.*/
    /*These parameters have already been validated to be multiples of 16.*/
    yhfrags=_state->info.frame_width>>3;
    yvfrags=_state->info.frame_height>>3;
    hdec=!(_state->info.pixel_fmt&1);
    vdec=!(_state->info.pixel_fmt&2);
    chfrags=yhfrags+hdec>>hdec;
    cvfrags=yvfrags+vdec>>vdec;
    yfrags=yhfrags*(int32_t)yvfrags;
    cfrags=chfrags*(int32_t)cvfrags;
    nfrags=yfrags+2*cfrags;
    /*Figure out the number of super blocks in each plane.*/
    yhsbs=yhfrags+3>>2;
    yvsbs=yvfrags+3>>2;
    chsbs=chfrags+3>>2;
    cvsbs=cvfrags+3>>2;
    ysbs=yhsbs*yvsbs;
    csbs=chsbs*cvsbs;
    nsbs=ysbs+2*csbs;
    nmbs=(size_t)ysbs<<2;
    /*Check for overflow.
    We support the ridiculous upper limits of the specification (1048560 by
    1048560, or 3 TB frames) if the target architecture has 64-bit pointers,
    but for those with 32-bit pointers (or smaller!) we have to check.
    If the caller wants to prevent denial-of-service by imposing a more
    reasonable upper limit on the size of attempted allocations, they must do
    so themselves; we have no platform independent way to determine how much
    system memory there is nor an application-independent way to decide what a
    "reasonable" allocation is.*/
    if(yfrags/yhfrags!=yvfrags||2*cfrags<cfrags||nfrags<yfrags||
        ysbs/yhsbs!=yvsbs||2*csbs<csbs||nsbs<ysbs||nmbs>>2!=ysbs){
            return TH_EIMPL;
    }
    /*Initialize the fragment array.*/
    _state->fplanes[0].nhfrags=yhfrags;
    _state->fplanes[0].nvfrags=yvfrags;
    _state->fplanes[0].froffset=0;
    _state->fplanes[0].nfrags=yfrags;
    _state->fplanes[0].nhsbs=yhsbs;
    _state->fplanes[0].nvsbs=yvsbs;
    _state->fplanes[0].sboffset=0;
    _state->fplanes[0].nsbs=ysbs;
    _state->fplanes[1].nhfrags=_state->fplanes[2].nhfrags=chfrags;
    _state->fplanes[1].nvfrags=_state->fplanes[2].nvfrags=cvfrags;
    _state->fplanes[1].froffset=yfrags;
    _state->fplanes[2].froffset=yfrags+cfrags;
    _state->fplanes[1].nfrags=_state->fplanes[2].nfrags=cfrags;
    _state->fplanes[1].nhsbs=_state->fplanes[2].nhsbs=chsbs;
    _state->fplanes[1].nvsbs=_state->fplanes[2].nvsbs=cvsbs;
    _state->fplanes[1].sboffset=ysbs;
    _state->fplanes[2].sboffset=ysbs+csbs;
    _state->fplanes[1].nsbs=_state->fplanes[2].nsbs=csbs;
    _state->nfrags=nfrags;
    _state->frags=calloc(nfrags,sizeof(*_state->frags));
    _state->frag_eob=calloc(nfrags,sizeof(*_state->frag_eob));
    _state->frag_qcm=calloc(nfrags,sizeof(*_state->frag_qcm));
    _state->frag_mvs=malloc(nfrags*sizeof(*_state->frag_mvs));
    _state->nsbs=nsbs;
    _state->sb_maps=malloc(nsbs*sizeof(*_state->sb_maps));
    _state->sb_flags=calloc(nsbs,sizeof(*_state->sb_flags));
    _state->nhmbs=yhsbs<<1;
    _state->nvmbs=yvsbs<<1;
    _state->nmbs=nmbs;
    _state->mb_maps=calloc(nmbs,sizeof(*_state->mb_maps));
    _state->mb_maps_rater_order= calloc(nmbs,sizeof(*_state->mb_maps_rater_order));
    _state->mb_modes=calloc(nmbs,sizeof(*_state->mb_modes));
    _state->coded_fragis=malloc(nfrags*sizeof(*_state->coded_fragis));
    if(_state->frags==NULL||_state->frag_mvs==NULL||_state->sb_maps==NULL||
        _state->sb_flags==NULL||_state->mb_maps==NULL||_state->mb_modes==NULL||
        _state->coded_fragis==NULL){
            return TH_EFAULT;
    }
    /*Create the mapping from super blocks to fragments.*/
    for(pli=0;pli<3;pli++){
        oc_fragment_plane *fplane;
        fplane=_state->fplanes+pli;
        oc_sb_create_plane_mapping(_state->sb_maps+fplane->sboffset,
            _state->sb_flags+fplane->sboffset,fplane->froffset,
            fplane->nhfrags,fplane->nvfrags);
    }
    /*Create the mapping from macro blocks to fragments.*/
    oc_mb_create_mapping(_state->mb_maps,_state->mb_maps_rater_order, _state->mb_modes,
        _state->fplanes,_state->info.pixel_fmt);
    return 0;
}

static void oc_state_frarray_clear(oc_theora_state *_state)
{
    free(_state->coded_fragis);
    free(_state->mb_modes);
    free(_state->mb_maps);
    free(_state->mb_maps_rater_order);
    free(_state->sb_flags);
    free(_state->sb_maps);
    free(_state->frag_mvs);
    free(_state->frag_qcm);
    free(_state->frag_eob);
    free(_state->frags);
}


static void oc_state_ref_bufs_clear(oc_theora_state *_state)
{
    free(_state->frag_buf_offs);
}


int32_t oc_state_init(oc_theora_state *_state,const th_info *_info,int32_t _nrefs)
{
    int32_t ret;
    /*First validate the parameters.*/
    if(_info==NULL)return TH_EFAULT;
    /*The width and height of the encoded frame must be multiples of 16.
    They must also, when divided by 16, fit into a 16-bit unsigned integer.
    The displayable frame offset coordinates must fit into an 8-bit unsigned
    integer.
    Note that the offset Y in the API is specified on the opposite side from
    how it is specified in the bitstream, because the Y axis is flipped in
    the bitstream.
    The displayable frame must fit inside the encoded frame.
    The color space must be one known by the encoder.*/
    if((_info->frame_width&0xF)||(_info->frame_height&0xF)||
        _info->frame_width<=0||_info->frame_width>=0x100000||
        _info->frame_height<=0||_info->frame_height>=0x100000||
        _info->pic_x+_info->pic_width>_info->frame_width||
        _info->pic_y+_info->pic_height>_info->frame_height||
        _info->pic_x>255||_info->frame_height-_info->pic_height-_info->pic_y>255||
        /*Note: the following <0 comparisons may generate spurious warnings on
        platforms where enums are unsigned.
        We could cast them to unsigned and just use the following >= comparison,
        but there are a number of compilers which will mis-optimize this.
        It's better to live with the spurious warnings.*/
        _info->colorspace<0||_info->colorspace>=TH_CS_NSPACES||
        _info->pixel_fmt<0||_info->pixel_fmt>=TH_PF_NFORMATS){
            return TH_EINVAL;
    }
    memset(_state,0,sizeof(*_state));
    memcpy(&_state->info,_info,sizeof(*_info));
    /*Invert the sense of pic_y to match Theora's right-handed coordinate
    system.*/
    _state->info.pic_y=_info->frame_height-_info->pic_height-_info->pic_y;
    _state->frame_type=OC_UNKWN_FRAME;
    _state->dct_fzig_zag=OC_FZIG_ZAG;

    ret=oc_state_frarray_init(_state);
    if(ret<0){
        oc_state_frarray_clear(_state);
        return ret;
    }
    /*If the keyframe_granule_shift is out of range, use the maximum allowable
    value.*/
    if(_info->keyframe_granule_shift<0||_info->keyframe_granule_shift>31){
        _state->info.keyframe_granule_shift=31;
    }
    _state->keyframe_num=0;
    _state->curframe_num=-1;
    /*3.2.0 streams mark the frame index instead of the frame count.
    This was changed with stream version 3.2.1 to conform to other Ogg
    codecs.
    We add an extra bias when computing granule positions for new streams.*/
    _state->granpos_bias=TH_VERSION_CHECK(_info,3,2,1);
    return 0;
}

void oc_state_clear(oc_theora_state *_state)
{
    oc_state_ref_bufs_clear(_state);
    oc_state_frarray_clear(_state);
}

int64_t th_granule_frame(void *_encdec,int64_t _granpos)
{
    oc_theora_state *state;
    state=(oc_theora_state *)_encdec;
    if(_granpos>=0){
        int64_t iframe;
        int64_t pframe;
        iframe=_granpos>>state->info.keyframe_granule_shift;
        pframe=_granpos-(iframe<<state->info.keyframe_granule_shift);
        /*3.2.0 streams store the frame index in the granule position.
        3.2.1 and later store the frame count.
        We return the index, so adjust the value if we have a 3.2.1 or later
        stream.*/
        return iframe+pframe-TH_VERSION_CHECK(&state->info,3,2,1);
    }
    return -1;
}

#if 0
double th_granule_time(void *_encdec,int64_t _granpos)
{
    oc_theora_state *state;
    state=(oc_theora_state *)_encdec;
    if(_granpos>=0){
        return (th_granule_frame(_encdec, _granpos)+1)*(
            (double)state->info.fps_denominator/state->info.fps_numerator);
    }
    return -1;
}
#endif
