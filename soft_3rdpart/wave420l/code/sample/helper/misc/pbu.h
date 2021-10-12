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
// Descriptions : Header file for PBU abstract model
//-----------------------------------------------------------------------------

//#include "com_cnm.h"

#ifndef __PBU_H__
#define __PBU_H__

#include <vputypes.h>

typedef void* spp_enc_context;

#ifdef __cplusplus
extern "C" {
#endif

	spp_enc_context spp_enc_init(Uint8 *buffer, int buffer_size, int enableEPB);
	void spp_enc_deinit(spp_enc_context context);
	void spp_enc_init_rbsp(spp_enc_context context);

	void spp_enc_put_nal_byte(spp_enc_context context, Uint32 var, int n);
	void spp_enc_put_bits(spp_enc_context context, Uint32 var, int n);

	void spp_enc_flush(spp_enc_context context);
	
	void spp_enc_put_ue(spp_enc_context context, Uint32 var);
	Uint32 spp_enc_get_ue_bit_size(Uint32 var);
	void spp_enc_put_se(spp_enc_context context, Uint32 var);
	void spp_enc_put_byte_align(spp_enc_context context, int has_stop_bit);

	Uint32 spp_enc_get_wbuf_remain(spp_enc_context context);
	Uint32 spp_enc_get_rbsp_bit(spp_enc_context context);
	Uint32 spp_enc_get_nal_cnt(spp_enc_context context);
	Uint8* spp_enc_get_wr_ptr(spp_enc_context context);
	Uint8* spp_enc_get_wr_ptr_only(spp_enc_context context);

	Uint32 spp_enc_get_est_nal_cnt(spp_enc_context context);

#ifdef __cplusplus
}
#endif

#endif

