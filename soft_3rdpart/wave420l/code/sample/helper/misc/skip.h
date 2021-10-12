//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#ifndef _SKIP_H_
#define _SKIP_H_

typedef enum {
    NUT_TRAIL_N         = 0,
    NUT_TRAIL_R         = 1,
    NUT_TSA_N           = 2,
    NUT_TSA_R           = 3,
    NUT_STSA_N          = 4,
    NUT_STSA_R          = 5,
    NUT_RADL_N          = 6,
    NUT_RADL_R          = 7,
    NUT_RASL_N          = 8,
    NUT_RASL_R          = 9,
    NUT_RSV_N10         = 10,
    NUT_RSV_R11         = 11,
    NUT_RSV_N12         = 12,
    NUT_RSV_R13         = 13,
    NUT_RSV_N14         = 14,
    NUT_RSV_R15         = 15,
    NUT_BLA_W_LP        = 16,
    NUT_BLA_W_RADL      = 17,
    NUT_BLA_N_LP        = 18,
    NUT_IDR_W_RADL      = 19,
    NUT_IDR_N_LP        = 20,
    NUT_CRA             = 21,
    NUT_RSV_IRAP22      = 22,
    NUT_RSV_IRAP23      = 23,
    NUT_VPS             = 32,
    NUT_SPS             = 33,
    NUT_PPS             = 34,
    NUT_AUD             = 35,
    NUT_EOS             = 36,
    NUT_EOB             = 37,
    NUT_FD              = 38,
    NUT_PREFIX_SEI      = 39,
    NUT_SUFFIX_SEI      = 40,
    NUT_MAX             = 63
} NalType;

#define H265_MAX_TEMPORAL_ID        6

enum {
    SKIP_CMD_DECODE,
    SKIP_CMD_NON_IRAP,          //<<! Skip except IRAP pictures, H.265
    SKIP_CMD_NON_REF,           //<<! Skip except non-reference pictures
    SKIP_CMD_TARGET_SUBLAYER,   //<<! Temporal scalability
    SKIP_CMD_NONE
};

typedef struct _SkipCmd {
    Uint32      cmd;
    Uint32      tid;                        //<<! valid when cmd is SKIP_CMD_TARGET_SUBLAYER
} SkipCmd;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SKIP_H_ */

