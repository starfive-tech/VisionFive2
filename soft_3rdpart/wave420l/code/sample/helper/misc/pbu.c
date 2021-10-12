//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//                    (C) COPYRIGHT 2003 - 2013   CHIPS&MEDIA INC.
//                           ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//     copies.
//
// File         : $Id: gbu.c 43121 2013-07-25 07:09:33Z lafley.kim $
// Author       : Tee Jung
// Modifier     : $LastChangedBy$
// Descriptions : Soruce file for PBU abstract model
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "main_helper.h"
#include "pbu.h"

enum
{
    SPP_IN,
    SPP_OUT_RBSP,
    SPP_OUT_NAL,
    SPP_UNALIGN
};

#   define NAL_BUF_SIZE 512

typedef struct 
{
	// stream buffer status
	unsigned char*  rd_ptr;
	unsigned char*  wr_ptr;

	unsigned char*  bb_start;
	unsigned char*  bb_end;

	int     explicit_end_flag;
	int     stream_end_flag;

	// instance handler ... for interrupt handling
	void*   handle;

} gbu_if_t;

typedef struct
{
	// for interfacing
	gbu_if_t    *param;

	// internal read pointer
	Uint8* rd_ptr;

	// nal buffer things
	Uint8  nal_buf[2][NAL_BUF_SIZE];

	int      nal_buf_idx;
	int      nal_buf_cnt;

	int      nal_ptr;
	Uint32 nal_cnt;

	int      last_nal_byte;

	// rbsp buffer things
	Uint32  wbuf[2];
	Int8   wbuf_emul_info;

	Int8   bptr;
	Int8   rbsp_bit_cnt;

	int      rbsp_init; 

	// rbsp consumed bit count
	int      tcnt;

	int      tc;

	// trailing zero counter
	int      zero_cnt;

	// eos
	int      eos;

	Uint32 est_nal_byte_cnt;

	int epbRequired;

} gbu_t;

static void enc_flush_rbsp(spp_enc_context context);
static void enc_flush_nal_buf(spp_enc_context context);
static void put_bits(spp_enc_context context, Uint32 var, int n);
static void put_ue(spp_enc_context context, Uint32 var);

spp_enc_context spp_enc_init(Uint8 *buffer, int buffer_size, int enableEPB)
{
	gbu_t* gbu;
	
	gbu = (gbu_t*)osal_malloc(sizeof(gbu_t));
	if (!gbu)
		return NULL;
	gbu->param = (gbu_if_t *)osal_malloc(sizeof(gbu_if_t));
	if (!gbu->param) {
		osal_free(gbu);
		return NULL;
	}

    // connect interface structure
	gbu->param->bb_start = buffer;
	gbu->param->bb_end = buffer + buffer_size;
	gbu->param->rd_ptr = buffer;
	gbu->param->wr_ptr = buffer;
	
    // reset write pointer
    gbu->rd_ptr = gbu->param->wr_ptr;

    // reset nal memory index/cnt
    gbu->nal_buf_idx = 0;
    gbu->nal_buf_cnt = 0;

    gbu->nal_ptr = 0;
    gbu->nal_cnt = 0;
    gbu->est_nal_byte_cnt = 0;

    // reset trailing zero count
    gbu->zero_cnt = 0;

    // reset rbsp consumed count
    gbu->bptr = 32;

    gbu->rbsp_bit_cnt = 0;
    gbu->tcnt = 0;

    // reset wbuf
    gbu->wbuf[0] = gbu->wbuf[1] = 0x00;

	gbu->epbRequired = enableEPB;

	return (spp_enc_context)gbu;
    
}

void spp_enc_deinit(spp_enc_context context)
{
	gbu_t* gbu = (gbu_t *)context;

	if (gbu) {
		if (gbu->param) {
			osal_free(gbu->param);
		}
		osal_free(gbu);
	}
}

void spp_enc_init_rbsp(spp_enc_context context)
{
    gbu_t* gbu = (gbu_t *)context;

    gbu->tcnt = 0;

    // reset trailing zero count
    gbu->zero_cnt = 0;

    // reset rbsp consumed count
    gbu->bptr = 32;

    // reset wbuf
    gbu->wbuf[0] = gbu->wbuf[1] = 0x00;
}

void spp_enc_put_nal_byte(spp_enc_context context, Uint32 var, int n)
{
    int i;
    gbu_t* gbu = (gbu_t *)context;

    for (i = n-1 ; i >= 0 ; i--)
    {
        gbu->nal_buf[gbu->nal_buf_idx][gbu->nal_ptr] = ((var >> (i<<3)) & 0xFF);
        gbu->nal_ptr++;
        gbu->nal_cnt++;
        gbu->est_nal_byte_cnt++;

		if (gbu->nal_ptr == NAL_BUF_SIZE)
        {
            enc_flush_nal_buf(context);

            gbu->nal_buf_idx = (gbu->nal_buf_idx+1)&1;
            gbu->nal_ptr     = 0;
        }
    }
}

void spp_enc_put_bits(spp_enc_context context, Uint32 var, int n)
{
    Uint32 mask;
    Uint32 data;
    gbu_t* gbu = (gbu_t *)context;

    if (n == 0)
        return;

    // update total rbsp count
    gbu->tcnt += n;

    if (n > gbu->bptr)
    {
        // write data
        mask = (1 << gbu->bptr) - 1;
        data = (var >> (n - gbu->bptr)) & mask;

        gbu->wbuf[0] |= data;
        gbu->rbsp_bit_cnt += gbu->bptr;

        // update input data
        n   -= gbu->bptr;

        // update gbu bptr
        gbu->bptr = 0;

        // write rbsp data to nbuf
        enc_flush_rbsp(context);
    }

    mask = ((Uint64)1 << n) - 1;
    data = var & mask;

    // write data
    gbu->wbuf[0] |= data << (gbu->bptr - n);
    gbu->rbsp_bit_cnt += n;

    // update bptr
    gbu->bptr -= n;

    if (gbu->bptr == 0)
        enc_flush_rbsp(context);
}

void spp_enc_flush(spp_enc_context context)
{

    enc_flush_rbsp(context);
    
    enc_flush_nal_buf(context);
}

void enc_flush_rbsp(spp_enc_context context)
{
	gbu_t* gbu = (gbu_t *)context;
    
    int     cnt = (32 - gbu->bptr + 7) >> 3;
    int     pos = 4 - cnt;
    Uint8 data;

    gbu->wbuf[1] = gbu->wbuf[0];

    for (cnt = cnt-1 ; cnt >= 0 ; cnt--)
    {
        data = (gbu->wbuf[0] >> ((cnt+pos)<<3)) & 0xFF;
        
		if (gbu->epbRequired == 1)
		{
			// insert EPB if needed
			if ((gbu->zero_cnt == 2) && (data <= 0x03))
			{
				spp_enc_put_nal_byte(context, 0x03, 1);
				gbu->zero_cnt = 0;
			}
		}
        spp_enc_put_nal_byte(context, data, 1);

        // update number of trailing zeroes
        if (data == 0x00)
            gbu->zero_cnt++;
        else
            gbu->zero_cnt = 0;
    }

    // reset bit ptr
    gbu->bptr = 32;

    // reset wbuf
    gbu->wbuf[0] = 0;

}

static void enc_flush_nal_buf(spp_enc_context context)
{
    int i, cnt;
    int left, room;
    Uint8* ptr;
	Uint8* align_wr_ptr;
    gbu_t* gbu = (gbu_t *)context;
    gbu_if_t* io = gbu->param;
    ptr = (Uint8 *)io->wr_ptr;

    align_wr_ptr = (Uint8*)(ptr);

    for (cnt = 0 ; cnt < gbu->nal_ptr ; )
    {
        room = left = io->rd_ptr - io->wr_ptr;

        // wraparound case
        if (left <= 0)
        {
            left  = io->bb_end - io->wr_ptr;
            room += io->bb_end - io->bb_start;
        }

        // write stream to CPB
        for (i = 0 ; i < left && cnt < gbu->nal_ptr ; i++, cnt++)
            *(align_wr_ptr + i) = gbu->nal_buf[gbu->nal_buf_idx][cnt];

        gbu->param->wr_ptr = align_wr_ptr + i;

        // handle wraparound case
        if (io->wr_ptr == io->bb_end)
            io->wr_ptr = io->bb_start;

        // if room is small enough, then flush data to external memory
        if ((room - i) < 256)
        {
            //host_isr(gbu->param, INT_BIT_BIT_BUF_FULL, NULL);
            continue;
        }
    }

    // clear nal_ptr
    gbu->nal_ptr = 0;
}

// put unsigned exp-golomb code
void spp_enc_put_ue(spp_enc_context context, Uint32 var)
{
    Uint32  num;
    Uint32  data;
   
    num  = -1;
    data = var + 1;

    while (data) 
    {
        data = data >> 1;
        num++;
    }

    // leading zero bits
    put_bits(context, 0, num);              
    put_bits(context, var+1, num+1);
}

Uint32 spp_enc_get_ue_bit_size(Uint32 var)
{
	Uint32 size;
	Uint32  num;
	Uint32  data;

	num  = -1;
	data = var + 1;

	while (data) 
	{
		data = data >> 1;
		num++;
	}

	// leading zero bits
	size = num;
	size += (num+1);

	return size;
}

// put signed exp-golomb code
void spp_enc_put_se(spp_enc_context context, Uint32 var)
{
    Uint32  data;
  
	// (-1)^(code_num + 1)
    if (var > 0)
        data = (( var)<<1) - 1;                 
    else
        data = ((-var)<<1);

    put_ue(context, data);
}

// put stop one bit & padding zeroes
void spp_enc_put_byte_align(spp_enc_context context, int has_stop_bit)
{
    gbu_t* gbu = (gbu_t *)context;

    // stop one bit
    if (has_stop_bit)
        spp_enc_put_bits(context, 1, 1);                     

    // padding zeroes
    if (gbu->bptr & 7)
        spp_enc_put_bits(context, 0, gbu->bptr & 7);    
}

// number of remain bit in wbuf
Uint32 spp_enc_get_wbuf_remain(spp_enc_context context)
{
    gbu_t* gbu = (gbu_t *)context;

    return (Uint32)gbu->bptr;
}

// number of total bit after gbu initialization 
Uint32 spp_enc_get_rbsp_bit(spp_enc_context context)
{
    gbu_t* gbu = (gbu_t *)context;

    return (Uint32)gbu->tcnt;
}

// number of nal byte after gbu initialization
Uint32 spp_enc_get_nal_cnt(spp_enc_context context)
{
    gbu_t* gbu = (gbu_t *)context;

    return (Uint32)gbu->nal_cnt;
}

// return wr_ptr of stream buffer
Uint8* spp_enc_get_wr_ptr(spp_enc_context context)
{
    gbu_t* gbu = (gbu_t *)context;

    enc_flush_rbsp(context);
    enc_flush_nal_buf(context);

    return gbu->param->wr_ptr;
}

Uint8* spp_enc_get_wr_ptr_only(spp_enc_context context)
{
    gbu_t* gbu = (gbu_t *)context;

    enc_flush_rbsp(context);

    return gbu->param->wr_ptr + gbu->nal_ptr;
}

void spp_enc_set_wr_ptr(spp_enc_context context, Uint32 wr_ptr)
{
    gbu_t* gbu = (gbu_t *)context;
    gbu_if_t*  io  = gbu->param;
	intptr_t wr_int_ptr = wr_ptr;
	intptr_t bb_end_int_ptr = (intptr_t)io->bb_end;
	intptr_t gbu_param_bb_end_int_ptr = (intptr_t)gbu->param->bb_end;
	intptr_t gbu_param_bb_start_int_ptr = (intptr_t)gbu->param->bb_start;
	
    while(wr_ptr > bb_end_int_ptr)
        wr_int_ptr = wr_int_ptr + gbu_param_bb_end_int_ptr - gbu_param_bb_start_int_ptr;

    io->wr_ptr = (Uint8 *)wr_int_ptr;
}

// get a estimated NAL count in byte
Uint32 spp_enc_get_est_nal_cnt(spp_enc_context context)
{
    gbu_t* gbu = (gbu_t *)context;
    Uint32 est_nal_byte = 0;
    Uint8  data;
    Uint32  cnt = (32 - gbu->bptr ) >> 3;
    Uint32  pos = 4 - cnt;
    Uint32  zero_cnt = gbu->zero_cnt;

    for (cnt = cnt-1 ; cnt >= 0 ; cnt--)
    {
        data = (gbu->wbuf[0] >> ((cnt+pos)<<3)) & 0xFF;

        // insert EPB if needed
        if ((zero_cnt == 2) && (data <= 0x03))
        {
            est_nal_byte++;
            zero_cnt = 0;
        }

        est_nal_byte++;

        // update number of trailing zeroes
        if (data == 0x00)
            zero_cnt++;
        else
            zero_cnt = 0;
    }

    return gbu->est_nal_byte_cnt + est_nal_byte;
}

static void put_bits(spp_enc_context context, Uint32 var, int n)
{
    Uint32 mask;
    Uint32 data;
    gbu_t* gbu = (gbu_t *)context;

    if (n == 0)
        return;

    // update total rbsp count
    gbu->tcnt += n;

    if (n > gbu->bptr)
    {
        // write data
        mask = (1 << gbu->bptr) - 1;
        data = (var >> (n - gbu->bptr)) & mask;

        gbu->wbuf[0] |= data;
        gbu->rbsp_bit_cnt += gbu->bptr;

        // update input data
        n   -= gbu->bptr;

        // update gbu bptr
        gbu->bptr = 0;

        // write rbsp data to nbuf
        enc_flush_rbsp(context);
    }

    mask = (1 << n) - 1;
    data = var & mask;

    // write data
    gbu->wbuf[0] |= data << (gbu->bptr - n);
    gbu->rbsp_bit_cnt += n;

    // update bptr
    gbu->bptr -= n;

    if (gbu->bptr == 0)
        enc_flush_rbsp(context);
}

static void put_ue(spp_enc_context context, Uint32 var)
{
    Uint32  num;
    Uint32  data;

    num  = -1;
    data = var + 1;

    while (data) 
    {
        data = data >> 1;
        num++;
    }

    // leading zero bits
    put_bits(context, 0, num);
    put_bits(context, var+1, num+1);
}
 
