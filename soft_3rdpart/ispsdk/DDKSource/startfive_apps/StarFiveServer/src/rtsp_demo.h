/*************************************************************************
	> File Name: rtsp_demo.h
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Monday, November 23, 2015 AM12:22:43 CST
 ************************************************************************/

#ifndef __RTSP_DEMO_H__
#define __RTSP_DEMO_H__

#include <stdint.h>


#define RTSP_SERVER_NAME					"StarFive RTSP Server"
#define RTSP_SERVER_VERSION					0x00010000
#define RTSP_SERVER_PORT					8554
#define RTSP_SERVER_MAX_CONNECTION			10
	
#define RTSP_CC_STATE_INIT					0
#define RTSP_CC_STATE_READY					1
#define RTSP_CC_STATE_PLAYING				2
#define RTSP_CC_STATE_RECORDING				3

#define RTP_MAX_PKTSIZ						((1500-42)/4*4)
#define RTP_MAX_NBPKTS						(3000)

#define VRTP_PT_ID_H264						(96)
#define VRTP_PT_ID_RAW						(112)

//#define VRTP_PT_YU12_ID						(110)
//#define VRTP_PT_NV12_ID						(111)
//#define VRTP_PT_RGGB_ID						(112)
//#define VRTP_PT_GRBG_ID						(113)
//#define VRTP_PT_GBRG_ID						(114)
//#define VRTP_PT_BGGR_ID						(115)

#define VRTSP_SUBPATH						"track1"


typedef enum _RTSP_CODEC_ID
{
	RTSP_CODEC_ID_NONE = 0,
	RTSP_CODEC_ID_VIDEO_H264 = 0x0001,	/* codec_data is SPS + PPS frames */
	RTSP_CODEC_ID_VIDEO_YUV,			/* YUV420P, YUV420SP */
	RTSP_CODEC_ID_VIDEO_RAW,			/* BAYER_RAW */
	RTSP_CODEC_ID_VIDEO_FRAW,			/* BAYER_FRAW */
	RTSP_CODEC_ID_VIDEO_UO = 0x6001,    /* ISP unscale output */
	RTSP_CODEC_ID_VIDEO_SS0,			/* ISP stream scale 0 */
	RTSP_CODEC_ID_VIDEO_SS1,			/* ISP stream scale 1 */
	RTSP_CODEC_ID_VIDEO_DUMP,			/* ISP stream dump raw */
} RTSP_CODEC_ID;

typedef void * rtsp_demo_handle;
typedef void * rtsp_session_handle;

int sk_errno(void);
const char* sk_strerror(int err);

rtsp_demo_handle rtsp_new_demo(int port);
int rtsp_do_event(rtsp_demo_handle demo);
rtsp_session_handle rtsp_new_session(rtsp_demo_handle demo, const char *path);
int rtsp_set_video(rtsp_session_handle session, int codec_id, const uint8_t *codec_data, int data_len, int width, int height, int bit, int stride, int mosaic);

int rtsp_tx_video(rtsp_session_handle session, const uint8_t *frame, int len, uint64_t ts);

int rtsp_get_next_video_frame(rtsp_session_handle session, FILE *fp, uint8_t **buff, int *size);

void rtsp_del_session(rtsp_session_handle session);
void rtsp_del_demo(rtsp_demo_handle demo);

uint64_t rtsp_get_reltime(void);
uint64_t rtsp_get_ntptime(void);
int rtsp_sync_video_ts(rtsp_session_handle session, uint64_t ts, uint64_t ntptime);

int rtp_tcp_send(SOCKET s, const char* buf, int len, int flags);
int rtp_udp_send(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen);

int rtsp_server_main(void* pparameters);


#endif //__RTSP_DEMO_H__
