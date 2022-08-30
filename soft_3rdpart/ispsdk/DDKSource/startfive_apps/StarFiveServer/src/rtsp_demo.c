/*************************************************************************
	> File Name: rtsp_demo.c
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Monday, November 23, 2015 AM12:34:09 CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include "comm.h"

#include "tinycthread.h"
#include "main.h"
#include "rtsp_demo.h"
#include "rtsp_msg.h"
#include "rtp_enc.h"
#include "rtsp_utils.h"
#include "ispc_main.h"
#include "broadcast_main.h"



typedef struct rtsp_session
{
	char path[SESSION_PATH_LEN];
	int  vcodec_id;
	union
	{
		CODEC_H264_S h264;
		CODEC_BIN_S bin;
	} vcodec_data;
	struct rtp_enc vrtpe;

	uint64_t video_ntptime_of_zero_ts;
	struct rtsp_demo *demo;
	struct rtsp_client_connection *client_connection[RTSP_SERVER_MAX_CONNECTION];
} RTSP_SESSION_S;

typedef struct rtp_connection
{
	int is_over_tcp;
	SOCKET tcp_sockfd; //if is_over_tcp=1. rtsp socket
	int tcp_interleaved[2];//if is_over_tcp=1. [0] is rtp interleaved, [1] is rtcp interleaved
	SOCKET udp_sockfd[2]; //if is_over_tcp=0. [0] is rtp socket, [1] is rtcp socket
	uint16_t udp_localport[2]; //if is_over_tcp=0. [0] is rtp local port, [1] is rtcp local port
	uint16_t udp_peerport[2]; //if is_over_tcp=0. [0] is rtp peer port, [1] is rtcp peer port
	struct in_addr peer_addr; //peer ipv4 addr

	uint32_t ssrc;
	uint32_t rtcp_packet_count;
	uint32_t rtcp_octet_count;
	uint64_t rtcp_last_ts;
} RTP_CONNECTION_S;

typedef struct rtsp_client_connection
{
	int state;	//session state

	SOCKET client_sockfd;		//rtsp client socket
	struct in_addr peer_addr; //peer ipv4 addr
	unsigned long session_id;	//session id

	char reqbuf[1024];
	int  reqlen;

	struct rtp_connection *vrtp;
	struct rtsp_demo *demo;
	struct rtsp_session *session;
} RTSP_CLIENT_CONNECTION_S;

typedef struct rtsp_demo
{
	SOCKET listen_sockfd;	//rtsp server socket 0:invalid
	//SocketSet_t sockset;
	struct rtsp_session *session[SESSION_MAX_NUM];
	struct rtsp_client_connection *client_connection[RTSP_SERVER_MAX_CONNECTION];
	int session_count; //total session 
	int client_connection_count; //total client connection
} RTSP_DEMO_S;


int sk_errno(void)
{
	return (errno);
}

const char *sk_strerror(int err)
{
	return strerror(err);
}

static unsigned long __rtp_gen_ssrc(void)
{
	static unsigned long ssrc = 0x22345678;
	return ssrc++;
}

static RTSP_DEMO_S *__alloc_demo(void)
{
	RTSP_DEMO_S *d = (RTSP_DEMO_S*) calloc(1, sizeof(RTSP_DEMO_S));
	if (NULL == d)
	{
		err("alloc memory for rtsp_demo failed\n");
		return NULL;
	}
	return d;
}

static void __free_demo(RTSP_DEMO_S *d)
{
	if (d)
	{
		free(d);
	}
}

static int __find_demo_session(RTSP_DEMO_S *d, RTSP_SESSION_S *s)
{
	int i;
	for (i = 0; i < SESSION_MAX_NUM; i++)
	{
		if (d->session[i] == s)
			return i;
	}
	return -1;
}

static int __find_demo_client_connection(RTSP_DEMO_S *d, RTSP_CLIENT_CONNECTION_S *cc)
{
	int i;
	for (i = 0; i < RTSP_SERVER_MAX_CONNECTION; i++)
	{
		if (d->client_connection[i] == cc)
			return i;
	}
	return -1;
}

static RTSP_SESSION_S *__alloc_session(RTSP_DEMO_S *d)
{
	RTSP_SESSION_S *s = (RTSP_SESSION_S*) calloc(1, sizeof(RTSP_SESSION_S));
	int index = -1;
	if (NULL == s)
	{
		err("alloc memory for rtsp_session failed\n");
		return NULL;
	}

	s->demo = d;
	index = __find_demo_session(d, NULL);
	if (index < 0)
	{
		free(s);
		return NULL;
	}
	//dbg("alloc session: %d\n", index);
	d->session[index] = s;
	d->session_count++;

	return s;
}

static void __free_session(RTSP_SESSION_S *s)
{
	if (s)
	{
		int index = -1;
		RTSP_DEMO_S *d = s->demo;
		index = __find_demo_session(d, s);
		if (index >= 0 && index < SESSION_MAX_NUM)
		{
			dbg("free session: %d\n", index);
			d->session[index] = NULL;
			d->session_count--;
			free(s);
		}
		else
		{
			err("Find session fail !\n");
		}
	}
}

static int __find_session_client_connection(RTSP_SESSION_S *s, RTSP_CLIENT_CONNECTION_S *cc)
{
	int i;
	for (i = 0; i < RTSP_SERVER_MAX_CONNECTION; i++)
	{
		if (s->client_connection[i] == cc)
			return i;
	}
	return -1;
}

static RTSP_CLIENT_CONNECTION_S *__alloc_client_connection(RTSP_DEMO_S *d)
{
	int index = -1;
	RTSP_CLIENT_CONNECTION_S *cc = (RTSP_CLIENT_CONNECTION_S*) calloc(1, sizeof(RTSP_CLIENT_CONNECTION_S));
	if (NULL == cc)
	{
		err("alloc memory for rtsp_session failed\n");
		return NULL;
	}

	cc->demo = d;
	index = __find_demo_client_connection(d, NULL); //find null client connection
	if (index < 0)
	{
		free(cc);
		return NULL;
	}
	//dbg("alloc client_connection: %d\n", index);
	d->client_connection[index] = cc;
	d->client_connection_count++;

	return cc;
}

static void __free_client_connection(RTSP_CLIENT_CONNECTION_S *cc)
{
	if (cc)
	{
		int index = -1;
		RTSP_DEMO_S *d = cc->demo;
		index = __find_demo_client_connection(d, cc); //find exist client connection
		if (index >= 0 && index < RTSP_SERVER_MAX_CONNECTION)
		{
			//dbg("free client_connection: %d\n", index);
			d->client_connection[index] = NULL;
			d->client_connection_count--;
			free(cc);
		}
		else
		{
			err("Find client connection fail !\n");
		}
	}
}

static void __client_connection_bind_session(RTSP_CLIENT_CONNECTION_S *cc, RTSP_SESSION_S *s)
{
	if (cc->session == NULL)
	{
		int index = -1;
		cc->session = s;
		index = __find_session_client_connection(s, NULL);
		if (index < 0)
		{
			err("session client connection array is full !");
			return;
		}
		//dbg("bind session: %d\n", index);
		s->client_connection[index] = cc;
	}
}

static void __client_connection_unbind_session(RTSP_CLIENT_CONNECTION_S *cc)
{
	int index = -1;
	RTSP_SESSION_S *s = cc->session;
	if (s)
	{
		index = __find_session_client_connection(s, cc);
		if (index >= 0 && index < RTSP_SERVER_MAX_CONNECTION)
		{
			//dbg("unbind session: %d\n", index);
			s->client_connection[index] = NULL;
			cc->session = NULL;
		}
		else
		{
			err("Find session client connection fail !\n");
		}
	}
}

static int __rtp_udp_local_setup(RTP_CONNECTION_S *rtp)
{
	int i, ret;

	for (i = 65536 / 4 * 3 / 2 * 2; i < 65536; i += 2)
	{
		SOCKET rtpsock, rtcpsock;
		struct sockaddr_in inaddr;
		uint16_t port;

		rtpsock = socket(AF_INET, SOCK_DGRAM, 0);
		if (rtpsock == INVALID_SOCKET)
		{
			err("create rtp socket failed: %s\n", sk_strerror(sk_errno()));
			return -1;
		}
		rtcpsock = socket(AF_INET, SOCK_DGRAM, 0);
		if (rtcpsock == INVALID_SOCKET)
		{
			err("create rtcp socket failed: %s\n", sk_strerror(sk_errno()));
			closesocket(rtpsock);
			return -1;
		}
		port = i;

		memset(&inaddr, 0, sizeof(inaddr));
		inaddr.sin_family = AF_INET;
		inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		inaddr.sin_port = htons(port);
		ret = bind(rtpsock, (struct sockaddr*)&inaddr, sizeof(inaddr));
		if (ret == SOCKET_ERROR)
		{
			closesocket(rtpsock);
			closesocket(rtcpsock);
			continue;
		}
		port = i + 1;

		memset(&inaddr, 0, sizeof(inaddr));
		inaddr.sin_family = AF_INET;
		inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		inaddr.sin_port = htons(port);
		ret = bind(rtcpsock, (struct sockaddr*)&inaddr, sizeof(inaddr));
		if (ret == SOCKET_ERROR)
		{
			closesocket(rtpsock);
			closesocket(rtcpsock);
			continue;
		}

		ret = fcntl(rtpsock, F_GETFL, 0);
		if (ret < 0)
		{
			warn("fcntl F_GETFL failed: %s\n", strerror(errno));
		}
		else
		{
			ret |= O_NONBLOCK;
			ret = fcntl(rtpsock, F_SETFL, ret);
			if (ret < 0)
			{
				warn("fcntl F_SETFL failed: %s\n", strerror(errno));
			}
		}
		ret = fcntl(rtcpsock, F_GETFL, 0);
		if (ret < 0)
		{
			warn("fcntl F_GETFL failed: %s\n", strerror(errno));
		}
		else
		{
			ret |= O_NONBLOCK;
			ret = fcntl(rtcpsock, F_SETFL, ret);
			if (ret < 0)
			{
				warn("fcntl F_SETFL failed: %s\n", strerror(errno));
			}
		}

		rtp->is_over_tcp = 0;
		rtp->udp_sockfd[0] = rtpsock;
		rtp->udp_sockfd[1] = rtcpsock;
		rtp->udp_localport[0] = i;
		rtp->udp_localport[1] = i + 1;

		return 0;
	}

	err("not found free local port for rtp/rtcp\n");
	return -1;
}

static int rtsp_path_match(const char *main_path, const char *full_path)
{
	char path0[64] = { 0 };
	char path1[64] = { 0 };

	strncpy(path0, main_path, sizeof(path0) - 2);
	strncpy(path1, full_path, sizeof(path1) - 2);

	if (path0[strlen(path0) - 1] != '/')
		strcat(path0, "/");
	if (path1[strlen(path1) - 1] != '/')
		strcat(path1, "/");

	if (strncmp(path0, path1, strlen(path0)))
		return 0;
	return 1;
}

rtsp_demo_handle rtsp_new_demo(int port)
{
	RTSP_DEMO_S *d = NULL;
	struct sockaddr_in inaddr;
	SOCKET sockfd;
	int ret;

	d = __alloc_demo();
	if (d == NULL)
		return NULL;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == INVALID_SOCKET)
	{
		err("create socket failed : %s\n", sk_strerror(sk_errno()));
		__free_demo(d);
		return NULL;
	}

	if (port <= 0)
		port = RTSP_SERVER_PORT;

	memset(&inaddr, 0, sizeof(inaddr));
	inaddr.sin_family = AF_INET;
	inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inaddr.sin_port = htons(port);
	ret = bind(sockfd, (struct sockaddr*)&inaddr, sizeof(inaddr));
	if (ret == SOCKET_ERROR)
	{
		err("bind socket to address failed : %s\n", sk_strerror(sk_errno()));
		closesocket(sockfd);
		__free_demo(d);
		return NULL;
	}
	ret = listen(sockfd, RTSP_SERVER_MAX_CONNECTION);
	if (ret == SOCKET_ERROR)
	{
		err("listen socket failed : %s\n", sk_strerror(sk_errno()));
		closesocket(sockfd);
		__free_demo(d);
		return NULL;
	}

	d->listen_sockfd = sockfd;
	d->session_count = 0;
	d->client_connection_count = 0;

	info("Starting the StarFive RTSP server on port %d ...\n", port);
	return (rtsp_demo_handle)d;
}

/////
static int rtsp_set_client_socket(SOCKET sockfd)
{
	int ret;
	int sndbufsiz = 1024 * 512;
	int keepalive = 1;
	int keepidle = 60;
	int keepinterval = 3;
	int keepcount = 5;
	struct linger ling;

	ret = fcntl(sockfd, F_GETFL, 0);
	if (ret < 0)
	{
		warn("fcntl F_GETFL failed: %s\n", strerror(errno));
	}
	else
	{
		ret |= O_NONBLOCK;
		ret = fcntl(sockfd, F_SETFL, ret);
		if (ret < 0)
		{
			warn("fcntl F_SETFL failed: %s\n", strerror(errno));
		}
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&sndbufsiz, sizeof(sndbufsiz));
	if (ret == SOCKET_ERROR)
	{
		warn("setsockopt SO_SNDBUF failed: %s\n", sk_strerror(sk_errno()));
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepalive, sizeof(keepalive));
	if (ret == SOCKET_ERROR)
	{
		warn("setsockopt SO_KEEPALIVE failed: %s\n", sk_strerror(sk_errno()));
	}

	ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (const char*)&keepidle, sizeof(keepidle));
	if (ret == SOCKET_ERROR)
	{
		warn("setsockopt TCP_KEEPIDLE failed: %s\n", sk_strerror(sk_errno()));
	}

	ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (const char*)&keepinterval, sizeof(keepinterval));
	if (ret == SOCKET_ERROR)
	{
		warn("setsockopt TCP_KEEPINTVL failed: %s\n", sk_strerror(sk_errno()));
	}

	ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (const char*)&keepcount, sizeof(keepcount));
	if (ret == SOCKET_ERROR)
	{
		warn("setsockopt TCP_KEEPCNT failed: %s\n", sk_strerror(sk_errno()));
	}

	memset(&ling, 0, sizeof(ling));
	ling.l_onoff = 1;
	ling.l_linger = 0;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char*)&ling, sizeof(ling)); //resolve too many TCP CLOSE_WAIT
	if (ret == SOCKET_ERROR)
	{
		warn("setsockopt SO_LINGER failed: %s\n", sk_strerror(sk_errno()));
	}

	return 0;
}

static RTSP_CLIENT_CONNECTION_S *rtsp_new_client_connection(RTSP_DEMO_S *d)
{
	RTSP_CLIENT_CONNECTION_S *cc = NULL;
	struct sockaddr_in inaddr;
	SOCKET sockfd;
	SOCKLEN addrlen = sizeof(inaddr);

	sockfd = accept(d->listen_sockfd, (struct sockaddr*)&inaddr, &addrlen);
	if (sockfd == INVALID_SOCKET)
	{
		err("accept failed : %s\n", sk_strerror(sk_errno()));
		return NULL;
	}

	rtsp_set_client_socket(sockfd);//XXX DEBUG

	info("new rtsp client %s:%u comming\n", inet_ntoa(inaddr.sin_addr), ntohs(inaddr.sin_port));

	cc = __alloc_client_connection(d);
	if (cc == NULL)
	{
		warn("clinet num too many! this client will lost\n");
		closesocket(sockfd);
		return NULL;
	}

	cc->state = RTSP_CC_STATE_INIT;
	dbg("Set state: RTSP_CC_STATE_INIT (NEW)\n");
	cc->client_sockfd = sockfd;
	cc->peer_addr = inaddr.sin_addr;

	return cc;
}

static int rtsp_new_rtp_connection(RTSP_CLIENT_CONNECTION_S *cc, int isaudio, int istcp, int peer_port, int peer_interleaved)
{
	RTP_CONNECTION_S *rtp;
	struct in_addr peer_addr = cc->peer_addr;

	rtp = (RTP_CONNECTION_S*) calloc(1, sizeof(RTP_CONNECTION_S));
	if (rtp == NULL)
	{
		err("alloc mem for rtp session failed: %s\n", strerror(errno));
		return -1;
	}

	rtp->is_over_tcp = !!istcp;
	rtp->peer_addr = peer_addr;
	rtp->ssrc = __rtp_gen_ssrc();
	//info("new rtp connection %s:%u\n", inet_ntoa(peer_addr), ntohs(peer_port));

	if (istcp)
	{
		rtp->tcp_sockfd = cc->client_sockfd;
		rtp->tcp_interleaved[0] = peer_interleaved;
		rtp->tcp_interleaved[1] = peer_interleaved + 1;
		info("new rtp over tcp for %s ssrc:%08x peer_addr:%s interleaved:%u-%u\n",
			(isaudio ? "audio" : "video"),
			 rtp->ssrc,
			 inet_ntoa(peer_addr),
			 rtp->tcp_interleaved[0], rtp->tcp_interleaved[1]);
	}
	else
	{
		if (__rtp_udp_local_setup(rtp) < 0)
		{
			free(rtp);
			return -1;
		}
		rtp->udp_peerport[0] = peer_port;
		rtp->udp_peerport[1] = peer_port + 1;
		info("new rtp over udp for %s ssrc:%08x local_port:%u-%u peer_addr:%s peer_port:%u-%u\n",
			(isaudio ? "audio" : "video"),
			 rtp->ssrc,
			 rtp->udp_localport[0], rtp->udp_localport[1],
			 inet_ntoa(peer_addr),
			 rtp->udp_peerport[0], rtp->udp_peerport[1]);
	}

	cc->vrtp = rtp;

	return 0;
}


rtsp_session_handle rtsp_new_session(rtsp_demo_handle demo, const char *path)
{
	RTSP_DEMO_S *d = (RTSP_DEMO_S*)demo;
	RTSP_SESSION_S *s = NULL;
	int i;

	if (!d || !path || strlen(path) == 0)
	{
		err("param invalid\n");
		goto fail;
	}

	for (i = 0; i < SESSION_MAX_NUM; i++)
	{
		if (!d->session[i]->path)
			continue;
		
		if (rtsp_path_match(d->session[i]->path, path) || rtsp_path_match(path, d->session[i]->path))
		{
			err("path:%s (%s) is exist!!!\n", s->path, path);
			goto fail;
		}
	}

	s = __alloc_session(d);
	if (!s)
	{
		goto fail;
	}

	strncpy(s->path, path, sizeof(s->path) - 1);
	s->vcodec_id = RTSP_CODEC_ID_NONE;


	//dbg("add session path: %s\n", s->path);
	return (rtsp_session_handle)s;
fail:
	if (s)
	{
		free(s);
	}
	return NULL;
}

static void rtsp_del_rtp_connection(RTSP_CLIENT_CONNECTION_S *cc, int isaudio)
{
	RTP_CONNECTION_S *rtp;

	rtp = cc->vrtp;
	cc->vrtp = NULL;

	if (rtp)
	{
		if (!(rtp->is_over_tcp))
		{
			closesocket(rtp->udp_sockfd[0]);
			closesocket(rtp->udp_sockfd[1]);
		}
		free(rtp);
	}
}

static void rtsp_del_client_connection(RTSP_CLIENT_CONNECTION_S *cc)
{
	if (cc)
	{
		info("delete client %d from %s\n", (int)cc->client_sockfd, inet_ntoa(cc->peer_addr));
		__client_connection_unbind_session(cc);
		rtsp_del_rtp_connection(cc, 0);
		//rtsp_del_rtp_connection(cc, 1);
		closesocket(cc->client_sockfd);
		__free_client_connection(cc);
	}
}

void rtsp_del_session(rtsp_session_handle session)
{
	int i;
	RTSP_SESSION_S *s = (RTSP_SESSION_S*)session;
	if (s)
	{
		for (i = 0; i < RTSP_SERVER_MAX_CONNECTION; i++)
		{
			if (s->client_connection[i])
				s->client_connection[i] = NULL;
		}

		if (s->vrtpe.szbuf)
			free(s->vrtpe.szbuf);
		__free_session(s);
	}
}

void rtsp_del_demo(rtsp_demo_handle demo)
{
	RTSP_DEMO_S *d = (RTSP_DEMO_S*)demo;
	int i;
	if (d)
	{
		for (i = 0; i < RTSP_SERVER_MAX_CONNECTION; i++)
		{
			if (d->client_connection[i])
				rtsp_del_client_connection(d->client_connection[i]);
		}

		for (i = 0; i < SESSION_MAX_NUM; i++)
		{
			if (d->session[i])
				rtsp_del_session(d->session[i]);
		}

		closesocket(d->listen_sockfd);

		__free_demo(d);
	}
}

int rtsp_build_sdp_media_attr_h264(int pt, int sample_rate, const struct codec_data_h264 *pst_codec_data, char *sdpbuf, int maxlen)
{
	char *p = sdpbuf;
	//	dbg("\n");

	p += sprintf(p, "m=video 0 RTP/AVP %d\r\n", pt);
	p += sprintf(p, "c=IN IP4 0.0.0.0\r\n");
	p += sprintf(p, "a=rtpmap:%d H264/%d\r\n", pt, sample_rate);
	if (pst_codec_data->sps_len > 0 && pst_codec_data->pps_len > 0)
	{
		const uint8_t *sps = pst_codec_data->sps;
		const uint8_t *pps = pst_codec_data->pps;
		int sps_len = pst_codec_data->sps_len;
		int pps_len = pst_codec_data->pps_len;
		p += sprintf(p, "a=fmtp:%d packetization-mode=1;sprop-parameter-sets=", pt);
		base64_encode(p, (maxlen - (p - sdpbuf)), sps, sps_len);
		p += strlen(p);
		p += sprintf(p, ",");
		base64_encode(p, (maxlen - (p - sdpbuf)), pps, pps_len);
		p += strlen(p);
		p += sprintf(p, "\r\n");
	}
	else
	{
		p += sprintf(p, "a=fmtp:%d packetization-mode=1\r\n", pt);
	}

	return (p - sdpbuf);
}

int rtsp_build_sdp_media_attr_uncompressed_video(int pt, int sample_rate, const struct codec_data_bin *pst_codec_data, char *sdpbuf, int codec, int frame_size, char* pcodecstr)
{
	//RFC 4175
	char *p = sdpbuf;

	//packetization-mode: the properties of an RTP payload codec (0: NAL, 1: non-interleaved, 2:interleaved)
	//profile-level-id:
	//  sps[1]  AVCProfileIndication : Baseline Profile (BP, 66=0x42), Extended Profile (XP, 88=0x58), Main Profile (MP, 77=0x4D), High Profile (HiP, 100=0x64)
	//	sps[2]  profile_compatibility : 
	//	sps[3]  AVCLevlIndication : 0x28=40 = 40 / 10 = 4 level (1,920!N1,080@30.1)  

	p += sprintf(p, "m=video 30000 RTP/AVP %d\r\n", pt);
	p += sprintf(p, "c=IN IP4 0.0.0.0\r\n");
	p += sprintf(p, "b=AS:%d\r\n", frame_size); //band width
	p += sprintf(p, "a=rtpmap:%d %s/%d\r\n", pt, pcodecstr, sample_rate);
	//p += sprintf(p, "a=fmtp:%d packetization-mode=1;profile-level-id=4D6028\r\n", pt);
	p += sprintf(p, "a=fmtp:%d packetization-mode=1;profile-level-id=4D6028;width=%d;height=%d;depth=%d;stride=%d;mosaic=%d;codec=%d\r\n", pt, pst_codec_data->frm.w, pst_codec_data->frm.h, pst_codec_data->frm.bit, pst_codec_data->frm.stride, pst_codec_data->frm.mosaic, codec);
	p += sprintf(p, "a=x-dimensions:%d,%d\r\n", pst_codec_data->frm.w, pst_codec_data->frm.h);

	return (p - sdpbuf);
}

static int build_simple_sdp(RTSP_SESSION_S *s, const char *uri, char *sdpbuf, int maxlen)
{
	char *p = sdpbuf;
	CODEC_FRAME_S* pfrm = &s->vcodec_data.bin.frm;
	int frame_size;

	p += sprintf(p, "v=0\r\n");
	p += sprintf(p, "o=- 0 0 IN IP4 0.0.0.0\r\n");
	p += sprintf(p, "s=rtsp_demo\r\n");
	p += sprintf(p, "t=0 0\r\n");
	p += sprintf(p, "a=control:%s\r\n", uri ? uri : "*");
	p += sprintf(p, "a=range:npt=0-\r\n");

	if (s->vcodec_id != RTSP_CODEC_ID_NONE)
	{
		switch (s->vcodec_id)
		{
			case RTSP_CODEC_ID_VIDEO_H264:
				p += rtsp_build_sdp_media_attr_h264(s->vrtpe.pt, s->vrtpe.sample_rate, &s->vcodec_data.h264, p, maxlen - (p - sdpbuf));
				break;
			case RTSP_CODEC_ID_VIDEO_YUV:
			case RTSP_CODEC_ID_VIDEO_UO:
			case RTSP_CODEC_ID_VIDEO_SS0:
			case RTSP_CODEC_ID_VIDEO_SS1:
				frame_size = pfrm->w * pfrm->h * 3 / 2;
				p += rtsp_build_sdp_media_attr_uncompressed_video(s->vrtpe.pt, s->vrtpe.sample_rate, &s->vcodec_data.bin, p, s->vcodec_id, frame_size, "JAZZ");
				break;

			case RTSP_CODEC_ID_VIDEO_RAW:
				frame_size = pfrm->stride * pfrm->h;
				p += rtsp_build_sdp_media_attr_uncompressed_video(s->vrtpe.pt, s->vrtpe.sample_rate, &s->vcodec_data.bin, p, s->vcodec_id, frame_size, "JAZZ");
				break;

			case RTSP_CODEC_ID_VIDEO_FRAW:
			case RTSP_CODEC_ID_VIDEO_DUMP:
				frame_size = pfrm->stride * pfrm->h;
				p += rtsp_build_sdp_media_attr_uncompressed_video(s->vrtpe.pt, s->vrtpe.sample_rate, &s->vcodec_data.bin, p, s->vcodec_id, frame_size, "JAZZ");
				break;
		}
		if (uri)
			p += sprintf(p, "a=control:%s/%s\r\n", uri, VRTSP_SUBPATH);
		else
			p += sprintf(p, "a=control:%s\r\n", VRTSP_SUBPATH);
	}
	//dbg("\n\n\n%s\n\n\n", sdpbuf);

	return (p - sdpbuf);
}

static int rtsp_handle_OPTIONS(RTSP_CLIENT_CONNECTION_S *cc, const rtsp_msg_s *reqmsg, rtsp_msg_s *resmsg)
{
	//	RTSP_DEMO_S *d = cc->demo;
	//	RTSP_SESSION_S *s = cc->session;
	uint32_t public_ = 0;
	public_ |= RTSP_MSG_PUBLIC_OPTIONS;
	public_ |= RTSP_MSG_PUBLIC_DESCRIBE;
	public_ |= RTSP_MSG_PUBLIC_SETUP;
	public_ |= RTSP_MSG_PUBLIC_PAUSE;
	public_ |= RTSP_MSG_PUBLIC_PLAY;
	public_ |= RTSP_MSG_PUBLIC_TEARDOWN;
	rtsp_msg_set_public(resmsg, public_);
	return 0;
}

static int rtsp_handle_DESCRIBE(RTSP_CLIENT_CONNECTION_S *cc, const rtsp_msg_s *reqmsg, rtsp_msg_s *resmsg)
{
	//	RTSP_DEMO_S *d = cc->demo;
	RTSP_SESSION_S *s = cc->session;
	char sdpbuf[1024] = "";
	int sdplen = 0;
	uint32_t accept = 0;
	const rtsp_msg_uri_s *puri = &reqmsg->hdrs.startline.reqline.uri;
	char uri[128] = "";

	if (rtsp_msg_get_accept(reqmsg, &accept) < 0 && !(accept & RTSP_MSG_ACCEPT_SDP))
	{
		rtsp_msg_set_response(resmsg, 406);
		warn("client not support accept SDP\n");
		return -1;
	}

	//build uri
	if (puri->scheme == RTSP_MSG_URI_SCHEME_RTSPU)
		strcat(uri, "rtspu://");
	else
		strcat(uri, "rtsp://");
	strcat(uri, puri->ipaddr);
	if (puri->port != 0)
		sprintf(uri + strlen(uri), ":%u", puri->port);
	strcat(uri, s->path);

	sdplen = build_simple_sdp(s, uri, sdpbuf, sizeof(sdpbuf));

	rtsp_msg_set_content_type(resmsg, RTSP_MSG_CONTENT_TYPE_SDP);
	rtsp_msg_set_content_length(resmsg, sdplen);
	resmsg->body.body = rtsp_mem_dup(sdpbuf, sdplen);
	return 0;
}

static int rtsp_handle_SETUP(RTSP_CLIENT_CONNECTION_S *cc, const rtsp_msg_s *reqmsg, rtsp_msg_s *resmsg)
{
	//	RTSP_DEMO_S *d = cc->demo;
	RTSP_SESSION_S *s = cc->session;
	RTP_CONNECTION_S *rtp = NULL;
	int istcp = 0, isaudio = 0;
	char vpath[64] = "";

	int ret;

	if (cc->state != RTSP_CC_STATE_INIT && cc->state != RTSP_CC_STATE_READY)
	{
		rtsp_msg_set_response(resmsg, 455);
		err("rtsp status err\n");
		return -1;
	}

	if (!reqmsg->hdrs.transport)
	{
		rtsp_msg_set_response(resmsg, 461);
		err("rtsp no transport err\n");
		return -1;
	}

	if (reqmsg->hdrs.transport->type == RTSP_MSG_TRANSPORT_TYPE_RTP_AVP_TCP)
	{
		istcp = 1;
		if (!(reqmsg->hdrs.transport->flags & RTSP_MSG_TRANSPORT_FLAG_INTERLEAVED))
		{
			err("rtsp no interleaved err\n");
			rtsp_msg_set_response(resmsg, 461);
			return -1;
		}
	}
	else
	{
		if (!(reqmsg->hdrs.transport->flags & RTSP_MSG_TRANSPORT_FLAG_CLIENT_PORT))
		{
			err("rtsp no client_port err\n");
			rtsp_msg_set_response(resmsg, 461);
			return -1;
		}
	}

	snprintf(vpath, sizeof(vpath) - 1, "%s/%s", s->path, VRTSP_SUBPATH);

	if (s->vcodec_id != RTSP_CODEC_ID_NONE && rtsp_path_match(vpath, reqmsg->hdrs.startline.reqline.uri.abspath))
	{
		isaudio = 0;
	}
	else
	{
		warn("rtsp urlpath:%s err\n", reqmsg->hdrs.startline.reqline.uri.abspath);
		rtsp_msg_set_response(resmsg, 461);
		return -1;
	}

	rtsp_del_rtp_connection(cc, isaudio);

	ret = rtsp_new_rtp_connection(cc, isaudio, istcp, reqmsg->hdrs.transport->client_port, reqmsg->hdrs.transport->interleaved);
	if (ret < 0)
	{
		rtsp_msg_set_response(resmsg, 500);
		return -1;
	}


	rtp = cc->vrtp;


	if (istcp)
	{
		rtsp_msg_set_transport_tcp(resmsg, rtp->ssrc, rtp->tcp_interleaved[0]);
	}
	else
	{
		rtsp_msg_set_transport_udp(resmsg, rtp->ssrc, rtp->udp_peerport[0], rtp->udp_localport[0]);
	}


	if (cc->state == RTSP_CC_STATE_INIT)
	{
		cc->state = RTSP_CC_STATE_READY;
		dbg("Set state: RTSP_CC_STATE_READY (SETUP), TCP: %d\n", istcp);
		cc->session_id = rtsp_msg_gen_session_id();
		rtsp_msg_set_session(resmsg, cc->session_id);
	}

	return 0;
}

static int rtsp_handle_PAUSE(RTSP_CLIENT_CONNECTION_S *cc, const rtsp_msg_s *reqmsg, rtsp_msg_s *resmsg)
{
	//	RTSP_DEMO_S *d = cc->demo;
	//	RTSP_SESSION_S *s = cc->session;

	if (cc->state != RTSP_CC_STATE_READY && cc->state != RTSP_CC_STATE_PLAYING)
	{
		rtsp_msg_set_response(resmsg, 455);
		err("rtsp status err\n");
		return -1;
	}

	if (cc->state != RTSP_CC_STATE_READY)
	{
		cc->state = RTSP_CC_STATE_READY;
		dbg("Set state: RTSP_CC_STATE_READY (PAUSE)\n");
	}
	return 0;
}

static int rtsp_handle_PLAY(RTSP_CLIENT_CONNECTION_S *cc, const rtsp_msg_s *reqmsg, rtsp_msg_s *resmsg)
{
	//	RTSP_DEMO_S *d = cc->demo;
	RTSP_SESSION_S *s = cc->session;

	if (cc->state != RTSP_CC_STATE_READY && cc->state != RTSP_CC_STATE_PLAYING)
	{
		rtsp_msg_set_response(resmsg, 455);
		err("rtsp status err\n");
		return -1;
	}

	if (cc->state != RTSP_CC_STATE_PLAYING)
	{
		cc->state = RTSP_CC_STATE_PLAYING;
		dbg("Set state: RTSP_CC_STATE_PLAYING (PLAY)\n");
	}
	return 0;
}

static int rtsp_handle_TEARDOWN(RTSP_CLIENT_CONNECTION_S *cc, const rtsp_msg_s *reqmsg, rtsp_msg_s *resmsg)
{
	//	RTSP_DEMO_S *d = cc->demo;
	RTSP_SESSION_S *s = cc->session;
	char vpath[64] = "";

	snprintf(vpath, sizeof(vpath) - 1, "%s/%s", s->path, VRTSP_SUBPATH);

	if (rtsp_path_match(vpath, reqmsg->hdrs.startline.reqline.uri.abspath))
	{
		rtsp_del_rtp_connection(cc, 0);
	}
	else
	{
		rtsp_del_rtp_connection(cc, 0);
		rtsp_del_rtp_connection(cc, 1);
	}
	if (!cc->vrtp)
	{
		cc->state = RTSP_CC_STATE_INIT;
		dbg("Set state: RTSP_CC_STATE_INIT (TEARDOWN)\n");
		rtsp_del_client_connection(cc);
	}
	return 0;
}

static int rtsp_process_request(RTSP_CLIENT_CONNECTION_S *cc, const rtsp_msg_s *reqmsg, rtsp_msg_s *resmsg)
{
	RTSP_DEMO_S *d = cc->demo;
	RTSP_SESSION_S *s = cc->session;
	const char *path = reqmsg->hdrs.startline.reqline.uri.abspath;
	uint32_t cseq = 0, session = 0;
	int i;

	rtsp_msg_set_response(resmsg, 200);
	rtsp_msg_set_date(resmsg, NULL);
	rtsp_msg_set_server(resmsg, RTSP_SERVER_NAME);

	if (rtsp_msg_get_cseq(reqmsg, &cseq) < 0)
	{
		rtsp_msg_set_response(resmsg, 400);
		warn("No CSeq field\n");
		return -1;
	}
	rtsp_msg_set_cseq(resmsg, cseq);

	if (cc->state != RTSP_CC_STATE_INIT)
	{
		if (rtsp_msg_get_session(reqmsg, &session) < 0 || session != cc->session_id)
		{
			warn("Invalid Session field\n");
			rtsp_msg_set_response(resmsg, 454);
			return -1;
		}
		rtsp_msg_set_session(resmsg, session);
	}

	if (s)
	{
		if (rtsp_path_match(s->path, path) == 0)
		{ // /live/chn0
			warn("path is not matched %s (old:%s)\n", path, s->path);
			rtsp_msg_set_response(resmsg, 451);
			return -1;
		}
	}
	else if (reqmsg->hdrs.startline.reqline.method != RTSP_MSG_METHOD_OPTIONS)
	{
		s = NULL;
		for (i = 0; i < SESSION_MAX_NUM; i++)
		{
			if (!d->session[i]->path)
				continue;

			if (rtsp_path_match(d->session[i]->path, path))
			{
				s = d->session[i];
				break;
			}
		}
		if (s == NULL)
		{
			warn("Not found session path: %s\n", path);
			rtsp_msg_set_response(resmsg, 454);
			return -1;
		}
		//info("session path: %s = %s\n", s->path, path);
		__client_connection_bind_session(cc, s);
	}

	switch (reqmsg->hdrs.startline.reqline.method)
	{
		case RTSP_MSG_METHOD_OPTIONS:
			dbg("rtsp_handle_OPTIONS()\n");
			return rtsp_handle_OPTIONS(cc, reqmsg, resmsg);
		case RTSP_MSG_METHOD_DESCRIBE:
			dbg("rtsp_handle_DESCRIBE()\n");
			return rtsp_handle_DESCRIBE(cc, reqmsg, resmsg);
		case RTSP_MSG_METHOD_SETUP:
			dbg("rtsp_handle_SETUP()\n");
			return rtsp_handle_SETUP(cc, reqmsg, resmsg);
		case RTSP_MSG_METHOD_PAUSE:
			dbg("rtsp_handle_PAUSE()\n");
			return rtsp_handle_PAUSE(cc, reqmsg, resmsg);
		case RTSP_MSG_METHOD_PLAY:
			dbg("rtsp_handle_PLAY()\n");
			return rtsp_handle_PLAY(cc, reqmsg, resmsg);
		case RTSP_MSG_METHOD_TEARDOWN:
			dbg("rtsp_handle_TEARDOWN()\n");
			return rtsp_handle_TEARDOWN(cc, reqmsg, resmsg);
		default:
			break;
	}

	rtsp_msg_set_response(resmsg, 501);
	return 0;
}

static int rtsp_recv_msg(RTSP_CLIENT_CONNECTION_S *cc, rtsp_msg_s *msg)
{
	int ret;

	if (sizeof(cc->reqbuf) - cc->reqlen - 1 > 0)
	{
		ret = recv(cc->client_sockfd, cc->reqbuf + cc->reqlen, sizeof(cc->reqbuf) - cc->reqlen - 1, MSG_DONTWAIT);
		if (ret == 0)
		{
			err("peer closed\n");
			return -1;
		}
		if (ret < 0) //== SOCKET_ERROR)
		{
			if (sk_errno() != SK_EAGAIN && sk_errno() != SK_EINTR)
			{
				err("recv data failed: %s\n", sk_strerror(sk_errno()));
				return -1;
			}
			ret = 0;
		}
		cc->reqlen += ret;
		cc->reqbuf[cc->reqlen] = 0;
		//dbg("rtsp_recv_msg[%d]: \n<<<<<\n%s\n>>>>>\n", cc->reqlen, cc->reqbuf);
	}

	if (cc->reqlen == 0)
	{
		return 0;
	}

	ret = rtsp_msg_parse_from_array(msg, cc->reqbuf, cc->reqlen);
	if (ret < 0)
	{
		err("Invalid frame\n");
		return -1;
	}
	if (ret == 0)
	{
		return 0;
	}

	//dbg("recv %d bytes rtsp message from %s\n", ret, inet_ntoa(cc->peer_addr));

	memmove(cc->reqbuf, cc->reqbuf + ret, cc->reqlen - ret);
	cc->reqlen -= ret;
	return ret;
}

static int rtsp_send_msg(RTSP_CLIENT_CONNECTION_S *cc, rtsp_msg_s *msg)
{
	char szbuf[1024] = "";
	int ret = rtsp_msg_build_to_array(msg, szbuf, sizeof(szbuf));
	if (ret < 0)
	{
		err("rtsp_msg_build_to_array failed\n");
		return -1;
	}

#if defined(RTSP_USE_TCP_IP_PROTOCOL)
	ret = rtp_tcp_send(cc->client_sockfd, szbuf, ret, 0);
#else
	ret = send(cc->client_sockfd, szbuf, ret, 0);
#endif //#if defined(RTSP_USE_TCP_IP_PROTOCOL)
	//dbg("rtsp_send_msg: \n<<<<<\n%s\n>>>>>\n", szbuf);
	if (ret < 0) //== SOCKET_ERROR)
	{
		err("rtsp response send failed: %s\n", sk_strerror(sk_errno()));
		return -1;
	}

	//dbg("sent %d bytes rtsp message to %s\n", ret, inet_ntoa(cc->peer_addr));
	return ret;
}

static const uint8_t *rtsp_find_h264_nalu(const uint8_t *buff, int len, uint8_t *type, int *size)
{
	const uint8_t *str = NULL;
	while (len >= 3)
	{
		if (buff[0] == 0 && buff[1] == 0 && buff[2] == 1)
		{
			if (!str)
			{
				if (len < 4)
					return NULL;
				str = buff;
				*type = buff[3] & 0x1f;
			}
			else
			{
				*size = (buff - str);
				return str;
			}
			buff += 3;
			len -= 3;
			continue;
		}
		if (len >= 4 && buff[0] == 0 && buff[1] == 0 && buff[2] == 0 && buff[3] == 1)
		{
			if (!str)
			{
				if (len < 5)
					return NULL;
				str = buff;
				*type = buff[4] & 0x1f;
			}
			else
			{
				*size = (buff - str);
				return str;
			}
			buff += 4;
			len -= 4;
			continue;
		}
		buff++;
		len--;
	}
	if (!str)
		return NULL;
	*size = (buff - str + len);
	return str;
}

int rtsp_codec_data_parse_from_user_h264(const uint8_t *codec_data, int data_len, struct codec_data_h264 *pst_codec_data)
{
	const uint8_t *s = codec_data;
	const uint8_t *frame = NULL;
	int len = data_len;
	int size = 0;
	int ret = 0;

	while (len > 3)
	{
		uint8_t type = 0;
		if (pst_codec_data->sps_len > 0 && pst_codec_data->pps_len > 0)
		{
			break;
		}

		frame = rtsp_find_h264_nalu(s, len, &type, &size);
		if (!frame)
		{
			break;
		}

		len = len - (frame - s + size);
		s = frame + size;

		if (frame[2] == 0)
		{
			frame += 4; //drop 0001
			size -= 4;
		}
		else
		{
			frame += 3; //drop 001
			size -= 3;
		}

		type = frame[0] & 0x1f;
		if (type == 7) //NALU_TYPE_SPS
		{
			//dbg("sps %d\n", size);
			if (size > (int)sizeof(pst_codec_data->sps))
				size = sizeof(pst_codec_data->sps);
			memcpy(pst_codec_data->sps, frame, size);
			pst_codec_data->sps_len = size;
			ret++;
		}
		if (type == 8) //NALU_TYPE_PPS
		{
			//dbg("pps %d\n", size);
			if (size > (int)sizeof(pst_codec_data->pps))
				size = sizeof(pst_codec_data->pps);
			memcpy(pst_codec_data->pps, frame, size);
			pst_codec_data->pps_len = size;
			ret++;
		}
	}

	return (ret >= 2 ? 1 : 0);
}

int rtsp_set_video(rtsp_session_handle session, int codec_id, const uint8_t *codec_data, int data_len, int width, int height, int bit, int stride, int mosaic)
{
	RTSP_SESSION_S *s = (RTSP_SESSION_S*)session;
	if (!s || (s->vcodec_id != RTSP_CODEC_ID_NONE && s->vcodec_id != codec_id))
		return -1;

	s->vcodec_id = codec_id;
	s->vrtpe.pt = VRTP_PT_ID_H264;
	s->vrtpe.seq = 0;
	s->vrtpe.ssrc = 0;
	s->vrtpe.sample_rate = 90000;
	memset(&s->vcodec_data, 0, sizeof(s->vcodec_data));

	switch (codec_id)
	{
		case RTSP_CODEC_ID_VIDEO_H264:
			s->vrtpe.pt = VRTP_PT_ID_H264;
			s->vcodec_data.h264.frm.w = width;
			s->vcodec_data.h264.frm.h = height;
			s->vcodec_data.h264.frm.bit = bit;
			if (codec_data && data_len > 0)
			{
				if (rtsp_codec_data_parse_from_user_h264(codec_data, data_len, &s->vcodec_data.h264) <= 0)
				{
					warn("parse codec_data failed\n");
					break;
				}
			}
			break;

		case RTSP_CODEC_ID_VIDEO_YUV:
		case RTSP_CODEC_ID_VIDEO_RAW:
		case RTSP_CODEC_ID_VIDEO_FRAW:
		case RTSP_CODEC_ID_VIDEO_UO:
		case RTSP_CODEC_ID_VIDEO_SS0:
		case RTSP_CODEC_ID_VIDEO_SS1:
		case RTSP_CODEC_ID_VIDEO_DUMP:
			s->vrtpe.pt = VRTP_PT_ID_RAW;
			s->vcodec_data.bin.frm.w = width;
			s->vcodec_data.bin.frm.h = height;
			s->vcodec_data.bin.frm.bit = bit;
			s->vcodec_data.bin.frm.stride = stride;
			s->vcodec_data.bin.frm.mosaic = mosaic;
			break;

		default:
			err("not supported codec_id %d for video\n", codec_id);
			return -1;
	}

	if (!s->vrtpe.szbuf)
	{
		s->vrtpe.szbuf = (uint8_t *)malloc(RTP_MAX_PKTSIZ * RTP_MAX_NBPKTS);
		if (!s->vrtpe.szbuf)
		{
			err("alloc memory for video rtp queue failed\n");
			s->vcodec_id = RTSP_CODEC_ID_NONE;
			return -1;
		}
		s->vrtpe.nbpkts = RTP_MAX_NBPKTS;
		s->vrtpe.pktsiz = RTP_MAX_PKTSIZ;
	}

	return 0;
}

int rtsp_get_next_video_frame_h264(RTSP_SESSION_S *s, FILE *fp, uint8_t **buff, int *size)
{
	uint8_t szbuf[1024];
	//int frame_size = s->vcodec_data.h264.frm.w * s->vcodec_data.h264.frm.h;
	int szlen = 0;
	int ret;
	if (!(*buff))
	{
		*buff = (uint8_t*)malloc(2 * 1024 * 1024);
		if (!(*buff))
			return -1;
	}

	*size = 0;

	while ((ret = fread(szbuf + szlen, 1, sizeof(szbuf) - szlen, fp)) > 0)
	{
		int i = 3;
		szlen += ret;
		while (i < szlen - 3 && !(szbuf[i] == 0 && szbuf[i + 1] == 0 && (szbuf[i + 2] == 1 || (szbuf[i + 2] == 0 && szbuf[i + 3] == 1)))) i++;
		memcpy(*buff + *size, szbuf, i);
		*size += i;
		memmove(szbuf, szbuf + i, szlen - i);
		szlen -= i;
		if (szlen > 3)
		{
			//dbg("szlen %d\n", szlen);
			fseek(fp, -szlen, SEEK_CUR);
			break;
		}
	}
	if (ret > 0)
		return *size;
	return 0;
}

int rtsp_get_next_video_frame_yuv420(RTSP_SESSION_S *s, FILE *fp, uint8_t **buff, int *size)
{
	int ret;
	int frame_size = s->vcodec_data.bin.frm.w * s->vcodec_data.bin.frm.h * 3 / 2;
	if (!(*buff))
	{
		*buff = (uint8_t*)malloc(frame_size);
		if (!(*buff))
			return -1;
	}

	ret = fread(*buff, 1, frame_size, fp);
	//dbg("w: %d, h: %d, frame_size: %d, read: %d\n", s->vcodec_data.bin.frm.w, s->vcodec_data.bin.frm.h, frame_size, ret);
	if (ret > 0)
	{
		*size = ret;
		return *size;
	}
	return 0;
}

int rtsp_get_next_video_frame_raw(RTSP_SESSION_S *s, FILE *fp, uint8_t **buff, int *size)
{
	int ret;
	CODEC_FRAME_S* pfrm = &s->vcodec_data.bin.frm;
	int frame_size = (pfrm->bit > 8) ? pfrm->w * 2 * pfrm->h : pfrm->w * pfrm->h;
	if (!(*buff))
	{
		*buff = (uint8_t*)malloc(frame_size);
		if (!(*buff))
			return -1;
	}

	ret = fread(*buff, 1, frame_size, fp);
	//dbg("w: %d, h: %d, frame_size: %d, read: %d\n", s->vcodec_data.bin.frm.w, s->vcodec_data.bin.frm.h, frame_size, ret);
	if (ret > 0)
	{
		*size = ret;
		return *size;
	}
	return 0;
}

int rtsp_get_next_video_frame_isp_yuv420(RTSP_SESSION_S* s, uint8_t** buff, int* size)
{
	uint32_t kind;
	ISPC_IMAGE img;
	int ret;

	switch (s->vcodec_id)
	{
		case RTSP_CODEC_ID_VIDEO_UO:
			kind = MEM_KIND_UO;
			break;
		case RTSP_CODEC_ID_VIDEO_SS0:
			kind = MEM_KIND_SS0;
			break;
		case RTSP_CODEC_ID_VIDEO_SS1:
			kind = MEM_KIND_SS1;
			break;
		default:
			return -1;
	}

	ret = ispc_main_driver_mem_get_completed(kind, &img);
	//if (!(*buff))
	//{
	//	dbg("allocbuf k=%d, w=%d, h=%d, s=%d, b=%d, mo=%d, ms=%d, mp=%p\n", img.kind, img.width, img.height, img.stride, img.bit, img.mosaic, img.memsize, img.pmemaddr);
	//	*buff = (uint8_t*)malloc(img.memsize);
	//	if (!(*buff))
	//		return -1;
	//}

	if (img.pmemaddr != 0)
		*buff = img.pmemaddr;
		//memcpy(*buff, img.pmemaddr, img.memsize);

	*size = img.memsize;
	return *size;

}

int rtsp_get_next_video_frame(rtsp_session_handle session, FILE *fp, uint8_t **buff, int *size)
{
	RTSP_SESSION_S *s = (RTSP_SESSION_S*)session;
	int ret = 0;
	if (!s)
		return 0;

	switch (s->vcodec_id)
	{
		case RTSP_CODEC_ID_VIDEO_H264:
			ret = rtsp_get_next_video_frame_h264(s, fp, buff, size);
			break;
		case RTSP_CODEC_ID_VIDEO_YUV:
			ret = rtsp_get_next_video_frame_yuv420(s, fp, buff, size);
			break;
		case RTSP_CODEC_ID_VIDEO_RAW:
		case RTSP_CODEC_ID_VIDEO_FRAW:
			ret = rtsp_get_next_video_frame_raw(s, fp, buff, size);
			break;

		//case RTSP_CODEC_ID_VIDEO_UO:
		//case RTSP_CODEC_ID_VIDEO_SS0:
		//case RTSP_CODEC_ID_VIDEO_SS1:
		//	ret = rtsp_get_next_video_frame_isp_yuv420(s, buff, size);
		//	break;
	}

	return ret;
}

int rtp_tcp_send(SOCKET s, const char* buf, int len, int flags)
{
	int ret, err, count = 0;

	do
	{
		ret = send(s, buf, len, flags);
		if (ret > 0)
			break;
		err = sk_errno();
		count++;
	} while (err == SK_EAGAIN || err == SK_EINTR);
	return ret;
}

int rtp_udp_send(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen)
{
	int ret, err, count = 0;

	do
	{
		ret = sendto(s, buf, len, flags, to, tolen);
		if (ret > 0)
			break;
		err = sk_errno();
		count++;
	} while (err == SK_EAGAIN || err == SK_EINTR);
	return ret;
}

int rtp_tx_data(RTP_CONNECTION_S *c, const uint8_t *data, int size)
{
	int err = 0;
	int try_count = 0;
	SOCKET sockfd;
	int ret = -1;

	if (c->is_over_tcp)
	{
		uint8_t szbuf[4];
		sockfd = c->tcp_sockfd;
		szbuf[0] = '$';
		szbuf[1] = c->tcp_interleaved[0];
		*((uint16_t*)&szbuf[2]) = htons(size);

		ret = rtp_tcp_send(sockfd, (const char*)szbuf, 4, 0);
		if (ret <= 0)// == SOCKET_ERROR)
		{
			warn("rtp over tcp send interlaced frame to %s failed: %s\n", inet_ntoa(c->peer_addr), sk_strerror(sk_errno()));
			return ret;
		}

#if defined(RTSP_USE_TCP_IP_PROTOCOL)
		ret = rtp_tcp_send(sockfd, (const char*)data, size, 0);
#else
		ret = send(sockfd, (const char*)data, size, 0);
#endif //#if defined(RTSP_USE_TCP_IP_PROTOCOL)
		if (ret <= 0)// == SOCKET_ERROR)
		{
			warn("rtp over tcp send %d bytes to %s failed: %s\n", size, inet_ntoa(c->peer_addr), sk_strerror(sk_errno()));
			return ret;
		}
	}
	else
	{
		struct sockaddr_in inaddr;
		sockfd = c->udp_sockfd[0];
	
		memset(&inaddr, 0, sizeof(inaddr));
		inaddr.sin_family = AF_INET;
		inaddr.sin_addr = c->peer_addr;
		inaddr.sin_port = htons(c->udp_peerport[0]);
		ret = rtp_udp_send(sockfd, (const char*)data, size, 0, (struct sockaddr*)&inaddr, sizeof(inaddr));
		if (ret <= 0)// == SOCKET_ERROR)
		{
			warn("rtp over udp send %d bytes to %s failed: %s\n", size, inet_ntoa(c->peer_addr), sk_strerror(sk_errno()));
			return ret;
		}
	}
	return size;
}

int rtsp_tx_video_internal(rtsp_session_handle session, uint8_t *packets[], int pktsizs[], int count)
{
	RTSP_SESSION_S *s = (RTSP_SESSION_S*) session;
	int i, j, size;

	for (i = 0; i < RTSP_SERVER_MAX_CONNECTION; i++)
	{
		RTSP_CLIENT_CONNECTION_S *cc = s->client_connection[i];
		if (!cc)
			continue;
		if (cc->state != RTSP_CC_STATE_PLAYING || !cc->vrtp)
			continue;

		for (j = 0; j < count; j++)
		{
			size = rtp_tx_data(cc->vrtp, packets[j], pktsizs[j]);
			if (size <= 0)
			{
				err("rtp_tx_data transmit error: client=%d, size=%d, err=%d\n", i, size, sk_errno());
#if defined(RTSP_USE_TCP_IP_PROTOCOL)
				rtsp_del_client_connection(cc); //Jazz: send fail to delete RTSP client.
#else
#endif //#if defined(RTSP_USE_TCP_IP_PROTOCOL)
				break;
			}
		}
	}
	return 1;
}

int rtsp_tx_video(rtsp_session_handle session, const uint8_t *frame, int len, uint64_t ts)
{
	RTSP_SESSION_S *s = (RTSP_SESSION_S*)session;
	uint8_t *packets[RTP_MAX_NBPKTS] = { NULL };
	int pktsizs[RTP_MAX_NBPKTS] = { 0 };
	int count = 0;
	int ret = 0;

	if (!s || !frame || s->vcodec_id == RTSP_CODEC_ID_NONE)
		return -1;

	if (s->vcodec_id == RTSP_CODEC_ID_VIDEO_H264 && (s->vcodec_data.h264.sps_len == 0 || s->vcodec_data.h264.pps_len == 0))
	{
		if (rtsp_codec_data_parse_from_user_h264(frame, len, &s->vcodec_data.h264) < 0)
		{
			warn("rtsp_codec_data_parse_from_frame_h264 failed\n");
		}
		//dbg("%s: sps: %d, pps: %d\n", s->path, s->vcodec_data.h264.sps_len, s->vcodec_data.h264.pps_len);
	}

	while (len > 0)
	{
		const uint8_t *start = NULL;
		uint8_t type = 0;
		int size = 0;

		switch (s->vcodec_id)
		{
			case RTSP_CODEC_ID_VIDEO_H264:
				start = rtsp_find_h264_nalu(frame, len, &type, &size);
				if (!start)
				{
					err("not found nal header\n");
					break;
				}
				count = rtp_enc_h264(&s->vrtpe, start, size, ts, packets, pktsizs);
				if (count <= 0)
				{
					err("rtp_enc_h264 ret = %d\n", count);
					continue;
				}
				break;
			case RTSP_CODEC_ID_VIDEO_YUV:
			case RTSP_CODEC_ID_VIDEO_RAW:
			case RTSP_CODEC_ID_VIDEO_FRAW:
			case RTSP_CODEC_ID_VIDEO_UO:
			case RTSP_CODEC_ID_VIDEO_SS0:
			case RTSP_CODEC_ID_VIDEO_SS1:
			case RTSP_CODEC_ID_VIDEO_DUMP:
				start = frame;
				size = len;
				count = rtp_enc_bin(&s->vrtpe, start, size, ts, packets, pktsizs);
#if defined(RTSP_USE_TCP_IP_PROTOCOL)
				//if (s->vcodec_id == RTSP_CODEC_ID_VIDEO_DUMP)
				//	dbg("dump: %d, count: %d\n", size, count);
#else
#endif //#if defined(RTSP_USE_TCP_IP_PROTOCOL)
				break;
		}

		len -= (start - frame) + size;
		frame = start + size;
	}

	rtsp_tx_video_internal(session, packets, pktsizs, count);
	return ret;
}

int rtsp_do_event(rtsp_demo_handle demo)
{
	RTSP_DEMO_S *d = (RTSP_DEMO_S*)demo;
	RTSP_CLIENT_CONNECTION_S *cc = NULL;
	fd_set rfds;
	fd_set wfds;
	rtsp_msg_s reqmsg;
	rtsp_msg_s resmsg;
	struct timeval tv;
	int i, ret;
	SOCKET maxfd;

	if (d->listen_sockfd == INVALID_SOCKET)
	{
		err("server socket descriptor error\n");
		return -1;
	}

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_SET(d->listen_sockfd, &rfds);
	maxfd = d->listen_sockfd;
	
	for (i = 0; i < RTSP_SERVER_MAX_CONNECTION; i++)
	{
		cc = d->client_connection[i];
		if (!cc)
			continue;

		FD_SET(cc->client_sockfd, &rfds);
		if (cc->client_sockfd > maxfd)
			maxfd = cc->client_sockfd;
	}


	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	ret = select(maxfd + 1, &rfds, &wfds, NULL, &tv);
	if (ret < 0)
	{
		err("select failed : %s\n", strerror(errno));
		return -1;
	}
	if (ret == 0)
	{
		return 0;
	}


	if (FD_ISSET(d->listen_sockfd, &rfds))
	{
		//new client_connection
		cc = rtsp_new_client_connection(d);
		if (!cc)
		{
			warn("new session beyond the max session number\n");
			return 0;
		}
	}

	for (i = 0; i < RTSP_SERVER_MAX_CONNECTION; i++)
	{
		cc = d->client_connection[i];
		if (!cc)
			continue;

		if (!FD_ISSET(cc->client_sockfd, &rfds))
		continue;
	
		rtsp_msg_init(&reqmsg);
		rtsp_msg_init(&resmsg);

		ret = rtsp_recv_msg(cc, &reqmsg);
		if (ret == 0)
			break;
		if (ret < 0)
		{
			rtsp_del_client_connection(cc);
			//dbg("rtsp_del_client_connection()\n");
			cc = NULL;
			break;
		}

		if (reqmsg.type == RTSP_MSG_TYPE_INTERLEAVED)
		{
			//TODO process RTCP over TCP frame
			//dbg("process RTCP over TCP frame.\n");
			rtsp_msg_free(&reqmsg);
			continue;
		}

		if (reqmsg.type != RTSP_MSG_TYPE_REQUEST)
		{
			err("not request frame.\n");
			rtsp_msg_free(&reqmsg);
			continue;
		}
				
		// process client request
		ret = rtsp_process_request(cc, &reqmsg, &resmsg);
		if (ret < 0)
		{
			err("request internal err\n");
		}
		else
		{
			ret = rtsp_send_msg(cc, &resmsg);
			//dbg("rtsp_send_msg() == %d\n", ret);
		}

		rtsp_msg_free(&reqmsg);
		rtsp_msg_free(&resmsg);

	}

	return 1;
}


//return us from system running
uint64_t rtsp_get_reltime(void)
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (tp.tv_sec * 1000000ULL + tp.tv_nsec / 1000ULL);
}

//return us from 1970/1/1 00:00:00
static uint64_t rtsp_get_abstime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000ULL + tv.tv_usec);
}

//return us from 1900/1/1 00:00:00
uint64_t rtsp_get_ntptime(void)
{
#define NTP_OFFSET_US (2208988800000000ULL)
	return (rtsp_get_abstime() + NTP_OFFSET_US);
}

int rtsp_sync_video_ts(rtsp_session_handle session, uint64_t ts, uint64_t ntptime)
{
	RTSP_SESSION_S *s = (RTSP_SESSION_S*) session;

	if (!s || s->vcodec_id == RTSP_CODEC_ID_NONE)
		return -1;

	s->video_ntptime_of_zero_ts = ntptime - ts; //XXX
	return 0;
}





