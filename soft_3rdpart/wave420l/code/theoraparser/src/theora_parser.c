//--=========================================================================--
//  This file is a part of BlackBird Video Codec Modeling project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2010   CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//-----------------------------------------------------------------------------
//--=========================================================================--

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "decint.h"
#include "theora_parser.h"
#include "theoradec.h"

#define THO_SEQ_HEADER_LEN 32
#define THO_PIC_INFO_LEN   20
#define THO_PIC_DATA_OFFSET 16
#define THO_SIZE_INFO_LEN  8

#define THO_SIZE_SKIP_FRAME 16

#define OFFSET_NOMV_MB  12
#define OFFSET_MV_MB    22
#define MEMORY_SWAP 1

#define THO_MB_FIXED_SIZE 12	 // fixed mb data size(12byte) except mv, qcm
#define THO_MB_MV_SIZE    10   // MV buffer size(10byte)
#define THO_QMAT_SIZE     768    //(3*2*64*sizeof(short)



enum {
    THO_INTRA_FRAME  = 0,
    THO_INTER_FRAME  = 1
};

enum {
	THO_SEQ_INIT      = 1,
	THO_SEQ_END       = 2,
	THO_PIC_RUN       = 3,
	THO_SET_FRAME_BUF = 4,
};

enum
{
    MV_X =0,
    MV_Y
};

enum
{
    LUMA_BLK_0 = 0,
    LUMA_BLK_1 = 1,
    LUMA_BLK_2 = 2,
    LUMA_BLK_3 = 3,
    CHROMA_BLK_0 = 4,
    CHROMA_BLK_1 = 5
};



typedef enum CODED__IDX
{
    CR_CUR  = 5,      // Cr, current block
    CB_CUR  = 4,      // Cb, current block
    LU_CUR3 = 3,     // luma, current MB block #3
    LU_CUR2 = 2,     // luma, current MB block #2
    LU_CUR1 = 1,     // luma, current MB block #1
    LU_CUR0 = 0      // luma, current MB block #0
};

typedef enum {
	VDI_LITTLE_ENDIAN = 0,
	VDI_BIG_ENDIAN,
	VDI_32BIT_LITTLE_ENDIAN,
	VDI_32BIT_BIG_ENDIAN,
} EndianMode;

#ifndef PUT_BYTE
#define PUT_BYTE(_p, _b) \
	*_p++ = (unsigned char)_b; 
#endif

#ifndef PUT_BUFFER
#define PUT_BUFFER(_p, _buf, _len) \
	memcpy(_p, _buf, _len); \
_p += _len;
#endif

#ifndef PUT_LE64
#define PUT_LE64(_p, _var) \
	*_p++ = (unsigned char)((_var)>>0);  \
	*_p++ = (unsigned char)((_var)>>8);  \
	*_p++ = (unsigned char)((_var)>>16); \
	*_p++ = (unsigned char)((_var)>>24); \
	*_p++ = (unsigned char)((_var)>>32); \
	*_p++ = (unsigned char)((_var)>>40); \
	*_p++ = (unsigned char)((_var)>>48); \
	*_p++ = (unsigned char)((_var)>>56); 
#endif

#ifndef PUT_LE32
#define PUT_LE32(_p, _var) \
	*_p++ = (unsigned char)((_var)>>0);  \
	*_p++ = (unsigned char)((_var)>>8);  \
	*_p++ = (unsigned char)((_var)>>16); \
	*_p++ = (unsigned char)((_var)>>24); 
#endif

#ifndef PUT_BE32
#define PUT_BE32(_p, _var) \
	*_p++ = (unsigned char)((_var)>>24); \
	*_p++ = (unsigned char)((_var)>>16); \
	*_p++ = (unsigned char)((_var)>>8);  \
	*_p++ = (unsigned char)((_var)>>0); 
#endif

#ifndef PUT_LE16
#define PUT_LE16(_p, _var) \
	*_p++ = (unsigned char)((_var)>>0);  \
	*_p++ = (unsigned char)((_var)>>8);  
#endif

#ifndef PUT_BE16
#define PUT_BE16(_p, _var) \
	*_p++ = (unsigned char)((_var)>>8);  \
	*_p++ = (unsigned char)((_var)>>0);  
#endif

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif
#ifndef AV_WB16
#   define AV_WB16(p, d) do {                   \
        ((uint8_t*)(p))[1] = (d);               \
        ((uint8_t*)(p))[0] = (d)>>8;            \
    } while(0)
#endif


const unsigned char OC_MB_MAP_RASTER[6]=
{
    0,1,2,3,4,8
};

const unsigned char OC_BLOCK_CODED_IDXS[6]=
{
    LU_CUR0,LU_CUR1,LU_CUR2,LU_CUR3,CB_CUR,CR_CUR
};


struct mb_mv_t
{
    uint16_t mb_size;       // total macro-block data size
    uint8_t  mb_mode;       // one of eight mode(4bit)
    uint8_t  bcoded;        // coded info of block
    uint16_t qii_arr;       // LSB part is qii_arr[0] - 2bit per entry, total 6 entry
    uint8_t  eob_pos_c[2];  // eob position of each chroma blocks
    uint8_t  eob_pos_l[4];  // eob position of each luma blocks
    int16_t  mvxy[5];       // except it when mb_mode is intra mode or no-mv mode
    int16_t  qcm[384];      // zig-zag(decoded) order
};

struct mb_nomv_t
{
    uint16_t mb_size;       // total macro-block data size
    uint8_t  mb_mode;       // one of eight mode(4bit)
    uint8_t  bcoded;        // coded info of block
    uint16_t qii_arr;       // LSB part is qii_arr[0] - 2bit per entry, total 6 entry
    uint8_t  eob_pos_c[2];  // eob position of each chroma blocks
    uint8_t  eob_pos_l[4];  // eob position of each luma blocks
    int16_t  qcm[384];      // zig-zag(decoded) order
} ;



static int32_t get_mb_nomv(oc_theora_state *state, void *data, 
                               uint8_t mb_mode, uint16_t *mb_fragis)
{
    uint16_t qi, bi, fragi;
    uint16_t cb_not_coded=0;
    uint16_t qcm_addr = 0;
    uint8_t  quota;
    uint8_t  *pos;
    uint8_t  coded, bcoded = 0;
    int16_t  dc;
    int32_t  eob_pos;
    int32_t  qcm_cnt = 0;
    int16_t  *out_qcm;

    const unsigned char *map_idxs;
    oc_mb_map     *mb_maps;
    oc_qcm        *frag_qcm;
    uint8_t       *frag_eob;
    oc_fragment   *frags;
    struct mb_nomv_t *mb_nomv;

    frags = state->frags;
    frag_qcm = state->frag_qcm;
    frag_eob = state->frag_eob;
    mb_maps  = state->mb_maps_rater_order;
    map_idxs = OC_MB_MAP_RASTER;
    mb_nomv  = (struct mb_nomv_t *)data;
    out_qcm  = mb_nomv->qcm;

    memset(mb_nomv, 0, sizeof(struct mb_nomv_t));
    for(bi=0;bi<6;bi++)
    {
        fragi = mb_fragis[bi];
        coded = frags[fragi].coded;
        coded = frags[fragi].coded;

        //mb_mode
        if(coded)
        {
            int16_t *in_qcm;

            // update eob_pos
            eob_pos = frag_eob[fragi];
            dc      = frags[fragi].dc;

            //have only dc value from dc precitor
            if(eob_pos == 0 && dc)
                eob_pos = 1;

            // balance dc value from dc predictor
            if(eob_pos == 1 && dc == 0)
                eob_pos = 0;

            pos = (bi<4) ? &(mb_nomv->eob_pos_l[bi]) : &(mb_nomv->eob_pos_c[bi - CHROMA_BLK_0]);
            *pos = eob_pos;

            // update qcm
            in_qcm = frag_qcm[fragi];
            quota  = eob_pos&(~3);

            for(qi=0; qi<quota;qi+=4)
            {
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
            }

            for(;qi<eob_pos;qi++)
            {
                *out_qcm++ = *in_qcm;  *in_qcm++ = 0;
            }

            qcm_cnt += eob_pos;
        }
        else
        {
            pos = (bi<4) ? &(mb_nomv->eob_pos_l[bi]) : &(mb_nomv->eob_pos_c[bi - CHROMA_BLK_0]);
            *pos = 0;
        }
        //coded info for each block
        bcoded |= coded*(1<<bi);
        //qii for each block
        mb_nomv->qii_arr |= (frags[fragi].qii) << (bi*2);
    }

    //update mb info
	mb_nomv->mb_size = THO_MB_FIXED_SIZE
	                 + (qcm_cnt << 1);
    mb_nomv->mb_mode = mb_mode;
    mb_nomv->bcoded  = bcoded;

	// mb_size must be 8byte alligned
	// we should get padding byte
    mb_nomv->mb_size = (mb_nomv->mb_size + 7) & (~0x7);
    return mb_nomv->mb_size;
}

static int32_t get_mb_mv(oc_theora_state *state, void *data, 
                     uint8_t mb_mode, uint16_t *mb_fragis)
{
    uint16_t qi, bi, fragi;
    uint16_t cb_not_coded = 0;
    uint16_t qcm_addr = 0;
    uint8_t  quota;
    uint8_t  *pos;
    uint8_t  coded, bcoded = 0, qii_arr=0;
    int16_t  dc;
    int32_t  qcm_cnt = 0;
    int16_t  *out_qcm;

    oc_mv          *frag_mvs;
    oc_qcm         *frag_qcm;
    uint8_t        *frag_eob;
    oc_fragment    *frags;
    signed char    *mb_modes;
    struct mb_mv_t *mb_mv;

    frags    = state->frags;
    frag_mvs = state->frag_mvs;
    frag_qcm = state->frag_qcm;
    frag_eob = state->frag_eob;
    mb_modes = state->mb_modes;

    mb_mv = (struct mb_mv_t *)data;
    out_qcm    = mb_mv->qcm;
    
    memset(mb_mv, 0, sizeof(struct mb_mv_t));
    for(bi=0;bi<6;bi++)
    {
        fragi = mb_fragis[bi];
        coded = frags[fragi].coded;

        //mb_mode
        if(coded)
        {
            int16_t *in_qcm;
            int16_t mv;
            int32_t  eob_pos;
            // update mb_mode
            mv = frag_mvs[fragi];

            // mv of cr block should not be sent to bpu
            if(bi!=5)
            {
                // sign flag of mv_y should be reversed
                // because origin position is differnce to otber standard codec
                // mvx : lsb 8bit, mvy : msb 8bit
                mb_mv->mvxy[bi] = (OC_MV_X(mv)&0xff) | (OC_MV_Y(mv)*(-1)<<8);
            }
            //cr_mv is write, if cb is not coded
            else if(cb_not_coded)
            {
                mb_mv->mvxy[CHROMA_BLK_0] = (OC_MV_X(mv)&0xff) | (OC_MV_Y(mv)*(-1)<<8);
            }

			// update eob_pos
            eob_pos = frag_eob[fragi];
            dc      = frags[fragi].dc;

            //have only dc value from dc precitor
            if(eob_pos == 0 && dc)
                eob_pos = 1;

            // balance dc value from dc predictor
            if(eob_pos == 1 && dc == 0)
                eob_pos = 0;

			pos = (bi<4) ? &(mb_mv->eob_pos_l[bi]) : &(mb_mv->eob_pos_c[bi - CHROMA_BLK_0]);
			*pos = eob_pos;

            // update qcm
            in_qcm     = frag_qcm[fragi];
            quota      = eob_pos&(~3);

            for(qi=0; qi<quota;qi+=4)
            {
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
                *out_qcm++ = *in_qcm; *in_qcm++ = 0;
            }

            for(;qi<eob_pos;qi++)
            {
                *out_qcm++ = *in_qcm;  *in_qcm++ = 0;
            }

            qcm_cnt += eob_pos;
		}
		else
		{
            pos = (bi<4) ? &(mb_mv->eob_pos_l[bi]) : &(mb_mv->eob_pos_c[bi - CHROMA_BLK_0]);
            *pos = 0;

			//mv of cr block is sent, if cb block is not coded
			if(bi==CHROMA_BLK_0)
			{
				cb_not_coded = 1;
				continue;
			}

			if(bi!=CHROMA_BLK_1)
				mb_mv->mvxy[bi] = 0;
		}
        //coded info for each block
        bcoded |= coded*(1<<bi);
        //qii info for each block
        mb_mv->qii_arr |= (frags[fragi].qii) << (bi*2);
    }

    //update mb info
    mb_mv->mb_size  = THO_MB_FIXED_SIZE 
                    + THO_MB_MV_SIZE
                    + (qcm_cnt<<1);
    mb_mv->mb_mode  = mb_mode;
    mb_mv->bcoded   = bcoded;

	// mb_size must be 8byte alligned
	// we should get padding byte
    mb_mv->mb_size = (mb_mv->mb_size + 7) & (~0x7);
	return mb_mv->mb_size;
}


int32_t theora_make_mb_data(oc_theora_state *state, unsigned char* pMbData)
{

    const unsigned char *mapIdxs;
    int32_t            nmbs, mbi;
    int32_t            mbsSize = 0;

    oc_mb_map       *mb_maps  = state->mb_maps_rater_order;
    oc_fragment     *frags   =  state->frags;
    oc_fragment_plane *fplanes = &state->fplanes[0];
    unsigned char   *pData = pMbData;

    nmbs    = (fplanes->nvfrags>>1)*(fplanes->nhfrags>>1);
    mapIdxs = OC_MB_MAP_RASTER;

    for(mbi=0; mbi< nmbs;mbi++)
    {
        /*Loop through the macro blocks in each super block in display order.*/
        uint8_t  mb_mode;
        uint16_t bytes;
        uint16_t mb_fragi[6];
        int32_t     *mb_map = (int32_t     *)mb_maps[mbi];

        // block 0
        mb_fragi[0] = mb_map[0]; mb_mode  =frags[mb_fragi[0]].mb_mode;
        // block 1
        mb_fragi[1] = mb_map[1]; mb_mode |=frags[mb_fragi[1]].mb_mode;
        // block 2
        mb_fragi[2] = mb_map[2]; mb_mode |=frags[mb_fragi[2]].mb_mode;
        // block 3
        mb_fragi[3] = mb_map[3]; mb_mode |=frags[mb_fragi[3]].mb_mode;
        // block 4
        mb_fragi[4] = mb_map[4]; mb_mode |=frags[mb_fragi[4]].mb_mode;
        // block 5
        mb_fragi[5] = mb_map[8]; mb_mode |=frags[mb_fragi[5]].mb_mode;

        if(mb_mode != OC_MODE_INTER_NOMV
        && mb_mode != OC_MODE_INTRA
        && mb_mode != OC_MODE_GOLDEN_NOMV)
        {
    	    bytes = get_mb_mv(state, pData, mb_mode, mb_fragi);
        }
        else
    	    bytes = get_mb_nomv(state, pData, mb_mode, mb_fragi);

        mbsSize += bytes;
        pData  += bytes;

    }

    return mbsSize;
}



// mem swap 64bit little endian to 64bit big endian
int32_t tho_swap_endian(unsigned char *data, int32_t len, int32_t swap_flag)
{
	uint32_t *p;
	uint32_t v1, v2, v3;
	int32_t i;
	p = (uint32_t *)data;

	if (swap_flag)
	{		

        for (i=0; i<len/4; i+=2)
        {
            v1 = p[i];
            v2  = ( v1 >> 24) & 0xFF;
            v2 |= ((v1 >> 16) & 0xFF) <<  8;
            v2 |= ((v1 >>  8) & 0xFF) << 16;
            v2 |= ((v1 >>  0) & 0xFF) << 24;
            v3 =  v2;
            v1  = p[i+1];
            v2  = ( v1 >> 24) & 0xFF;
            v2 |= ((v1 >> 16) & 0xFF) <<  8;
            v2 |= ((v1 >>  8) & 0xFF) << 16;
            v2 |= ((v1 >>  0) & 0xFF) << 24;
            p[i]   =  v2;
            p[i+1] = v3;
        }
	}

	return 0;
}


int32_t theora_parser_init(void **parser)
{
    tho_ctx      *ctx;
    tho_parser_t *tho;

    tho = calloc(1, sizeof(tho_parser_t));
	ctx = calloc(1, sizeof(tho_ctx));

    tho->open       = theora_parser_open;
    tho->close      = theora_parser_end;
    tho->read_frame = theora_parser_frame;
    tho->handle     = (void*)ctx;

    *parser = (void**)tho;
    return 1;
}

int32_t theora_parser_end(void *parser)
{
    tho_parser_t      *tho = (tho_parser_t *)parser;
    tho_ctx           *ctx = (tho_ctx *)tho->handle;
	th_info           *ti = &ctx->ti;
	th_comment        *tc = &ctx->tc;
	th_dec_ctx        *td = ctx->td;

    if(tho)
    {
        if(ctx)
        {
            th_decodesys_mem_free(td);
            th_comment_clear(tc);
            th_info_clear(ti);
            free(ctx);
        }

        free(tho);
    }
    return 1;
}

int32_t theora_parser_open(void *handle, unsigned char *header, int32_t header_size, int32_t* scaleInfo)
{
    int32_t               ret;
    tho_ctx           *ctx = (tho_ctx *)handle;
	th_info           *ti = &ctx->ti;
	th_comment        *tc = &ctx->tc;
	th_setup_info     *ts = NULL;
	th_dec_ctx        *td = NULL;


    /* init supporting Theora structures needed in header parsing */
    th_comment_init(tc);
    th_info_init(ti);


    ret = th_decode_headerin(ti, tc, &ts, header, header_size);

    /* and now we have it all.  initialize decoders */
    if(ret == 0)
    {
        // decode alloc
        td = th_decode_alloc(ti, ts);
        ctx->td = td;
        ctx->ts = ts;
    }else{
        /* tear down the partial theora setup */
        th_info_clear(ti);
        th_comment_clear(tc);
    }
    
	/*Either way, we're done with the codec setup data.*/
    th_setupsys_mem_free(ts);

    /* return scale infomation for display*/
    scaleInfo[0] = ti->frame_width;
    scaleInfo[1] = ti->frame_height;
    scaleInfo[2] = ti->pic_width;
    scaleInfo[3] = ti->pic_height;
    scaleInfo[4] = ti->pic_x;
    scaleInfo[5] = ti->pic_y;

    return 1;
}



int32_t theora_parser_frame(void *handle, unsigned char * chunkData, int32_t chunkSize)
{
    int32_t ret=0;
    tho_ctx           *ctx = (tho_ctx *)handle;
    th_dec_ctx        *td  = ctx->td;

    ret = th_decode_packetin(td,chunkData, chunkSize, NULL);
    if(ret == 2)
   	    ctx->frame_skip_mode = 1;
    else
        ctx->frame_skip_mode = 0;

    return 1;
}

// Return generated frame size
int32_t theora_make_stream(void *handle, unsigned char *stream, int32_t run_cmd)
{
    int32_t               size;
    tho_ctx           *ctx = (tho_ctx *)handle;

    switch(run_cmd)
    {
    case THO_SEQ_INIT:
        {
            unsigned char *header;
            th_info       *ti = &ctx->ti;

            // signature  : 4Byte
            header = stream;
            PUT_BYTE(header, 'C');
            PUT_BYTE(header, 'N');
            PUT_BYTE(header, 'M');
            PUT_BYTE(header, 'V');

            // version  : 2Byte
            PUT_LE16(header, 0);

            // header length: 2Byte
            PUT_LE16(header, THO_SEQ_HEADER_LEN); 

            // FourCC : 4Byte
            PUT_BYTE(header, 'V');
            PUT_BYTE(header, 'P');
            PUT_BYTE(header, '3');
            PUT_BYTE(header, '0');

            // Size Info : 4Byte
            PUT_LE16(header, ti->frame_width);    // Picture Width
            PUT_LE16(header, ti->frame_height);   // Picture Height     

            // Etc : 16Byte
            PUT_LE32(header, 0);     // Frame Rate
            PUT_LE32(header, 0);     // Time Scale
            PUT_LE32(header, 0);     // Frame Number
            PUT_LE32(header, 0);     // Reserved

            size = THO_SEQ_HEADER_LEN;
        }
    break;

    case THO_PIC_RUN:
        {
            unsigned char *header;
            uint32_t qi;
            uint32_t picChunkSize;
            uint64_t  mbsSize, qmatSize;

            unsigned char   *pData;

            oc_theora_state *state  = &ctx->td->state;
            oc_mb_map       *mbMaps = state->mb_maps_rater_order;
            oc_fragment     *frags   =  state->frags;
            oc_fragment_plane *fplanes = &state->fplanes[0];

            // get quantization matrix
            if(ctx->frame_skip_mode)
            {
                /*
                /* Set Picture Header
                */
                header = stream;
                PUT_LE32(header, 4);          // Chunk Size
                PUT_LE32(header, 0);          // Time Stamp
                PUT_LE32(header, 0);          // Time Stamp
                PUT_BYTE(header, 1);          // skip frame flag        
                PUT_BYTE(header, 0);          // aligned zero bit
                PUT_BYTE(header, 0);          // aligned zero bit
                PUT_BYTE(header, 0);          // aligned zero bit

                size = THO_SIZE_SKIP_FRAME;
            }
            else
            {
                /************************/
                /* STORE QMAT DATA      */ 
                /* **********************/
                header   =  stream + THO_PIC_DATA_OFFSET;
                pData    =  stream + THO_PIC_DATA_OFFSET + THO_SIZE_INFO_LEN;
                qmatSize = (state->nqis*THO_QMAT_SIZE);

                for(qi=0; qi<state->nqis; qi++)
                {
                    uint16_t *dst = (uint16_t*)&pData[qi*3*2*64*2];
                    uint16_t *src = (uint16_t*)&state->dequant_tables[state->qis[qi]][0][0][0];
                    memcpy(dst,src, sizeof(uint16_t)*64*3*2);
                }
                    
                // To set Quantization Matrix Size
                PUT_LE64(header, qmatSize);                
                // swap 64bit little endian to 64bit big endian
                tho_swap_endian(pData, (int32_t)qmatSize, MEMORY_SWAP);
                /*************************/
                /* STORE MB DATA         */
                /*************************/
                header  = pData + qmatSize;
                pData  += (qmatSize + THO_SIZE_INFO_LEN);
                mbsSize = theora_make_mb_data(state, pData);

                // To set MB data size
                PUT_LE64(header, mbsSize);                
                // swap 64bit little endia to 64bit big endian
                tho_swap_endian(pData, (int32_t)mbsSize, MEMORY_SWAP);

                picChunkSize = (uint32_t)(THO_PIC_INFO_LEN + mbsSize + qmatSize);
                /*************************/
                /* Set Picture Header
                /*************************/
                header = stream;
                PUT_LE32(header, picChunkSize);              // frame size
                PUT_LE32(header, 0);                         // time stamp
                PUT_LE32(header, 0);                         // time stamp
                PUT_BYTE(header, 0);                         // skip frame flag
                PUT_BYTE(header, state->frame_type);         // frame type
                PUT_BYTE(header, state->nqis);               // nqis
                PUT_BYTE(header, state->loop_filter_limits[state->qis[0]]);  // limits-value

                size =  picChunkSize + 12;
            }
        }
    break;
    }

    return size;
}

