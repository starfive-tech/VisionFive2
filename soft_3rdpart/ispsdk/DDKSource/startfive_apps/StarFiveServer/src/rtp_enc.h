/*************************************************************************
	> File Name: rtp_enc.h
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Saturday, December 19, 2015 PM08:27:54 CST
 ************************************************************************/

#ifndef __RTP_ENC_H__
#define __RTP_ENC_H__

#include <stdint.h>

typedef struct codec_data_frame
{
	uint32_t w;
	uint32_t h;
	uint32_t bit;
	uint32_t stride;
	uint32_t mosaic;
	uint32_t fps;
} CODEC_FRAME_S;

typedef struct codec_data_h264
{
	CODEC_FRAME_S frm;
	uint8_t sps[64];
	uint8_t pps[64];
	uint32_t sps_len;
	uint32_t pps_len;
} CODEC_H264_S;

typedef struct codec_data_bin
{
	CODEC_FRAME_S frm;
} CODEC_BIN_S;

typedef struct rtp_enc
{
	uint8_t  pt;
	uint16_t seq;
	uint32_t ssrc;
	uint32_t sample_rate;

	uint8_t *szbuf;
	uint16_t pktsiz;
	uint16_t nbpkts;
} RTP_ENC_S;

int rtp_enc_h264(RTP_ENC_S *e, const uint8_t *frame, int len, uint64_t ts, uint8_t *packets[], int pktsizs[]);
int rtp_enc_bin(RTP_ENC_S *e, const uint8_t *frame, int len, uint64_t ts, uint8_t *packets[], int pktsizs[]);

#endif //__RTP_ENC_H__

