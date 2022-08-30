/*************************************************************************
	> File Name: rtsp_msg.h
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Friday, December 11, 2015 AM03:31:53 CST
 ************************************************************************/

#ifndef __RTSP_MSG_H__
#define __RTSP_MSG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	//RTSP/1.0 Message Parse/Build

	typedef enum __rtsp_msg_type_e
	{
		RTSP_MSG_TYPE_REQUEST = 0,
		RTSP_MSG_TYPE_RESPONSE,
		RTSP_MSG_TYPE_INTERLEAVED,
		RTSP_MSG_TYPE_BUTT,
	} rtsp_msg_type_e;

	typedef enum __rtsp_msg_method_e
	{
		RTSP_MSG_METHOD_OPTIONS = 0,
		RTSP_MSG_METHOD_DESCRIBE,
		RTSP_MSG_METHOD_SETUP,
		RTSP_MSG_METHOD_PLAY,
		RTSP_MSG_METHOD_RECORD,
		RTSP_MSG_METHOD_PAUSE,
		RTSP_MSG_METHOD_TEARDOWN,
		RTSP_MSG_METHOD_ANNOUNCE,
		RTSP_MSG_METHOD_SET_PARAMETER,
		RTSP_MSG_METHOD_GET_PARAMETER,
		RTSP_MSG_METHOD_REDIRECT,
		RTSP_MSG_METHOD_BUTT,
	} rtsp_msg_method_e;

	typedef enum __rtsp_msg_uri_scheme_e
	{
		RTSP_MSG_URI_SCHEME_RTSP = 0,
		RTSP_MSG_URI_SCHEME_RTSPU,
		RTSP_MSG_URI_SCHEME_BUTT,
	} rtsp_msg_uri_scheme_e;

	typedef struct __rtsp_msg_uri_s
	{
		rtsp_msg_uri_scheme_e scheme;
		uint16_t port;
		char ipaddr[32];
		char abspath[64];
	} rtsp_msg_uri_s;

	typedef enum __rtsp_msg_version_e
	{
		RTSP_MSG_VERSION_1_0 = 0,
		RTSP_MSG_VERSION_BUTT,
	} rtsp_msg_version_e;

	typedef struct __rtsp_msg_request_line_s
	{
		rtsp_msg_method_e method;
		rtsp_msg_uri_s    uri;
		rtsp_msg_version_e version;
	} rtsp_msg_request_line_s;

	typedef struct __rtsp_msg_response_line_s
	{
		rtsp_msg_version_e version;
		uint32_t status_code;
	} rtsp_msg_response_line_s;

	typedef struct __rtsp_msg_interleaved_line_s
	{
		uint8_t  channel;
		uint16_t length;
		uint8_t  reserved;
	} rtsp_msg_interleaved_line_s;

	//CSeq g req. all
	typedef struct __rtsp_msg_cseq_s
	{
		uint32_t cseq;
	} rtsp_msg_cseq_s;

	//Date g opt. all
	typedef struct __rtsp_msg_date_s
	{
		char http_date[32];
	} rtsp_msg_date_s;

	//Session Rr req. all but SETUP,OPTIONS
	typedef struct __rtsp_msg_session_s
	{
		uint32_t session;
	} rtsp_msg_session_s;

	typedef enum __rtsp_msg_transport_type_e
	{
		RTSP_MSG_TRANSPORT_TYPE_RTP_AVP = 0, //RTPoverUDP
		RTSP_MSG_TRANSPORT_TYPE_RTP_AVP_TCP, //RTPoverTCP
		RTSP_MSG_TRANSPORT_TYPE_BUTT,
	} rtsp_msg_transport_type_e;

	//Transport Rr req. SETUP
	typedef struct __rtsp_msg_transport_s
	{
		rtsp_msg_transport_type_e type;
		uint32_t flags;
	#define RTSP_MSG_TRANSPORT_FLAG_SSRC			(1<<0)
	#define RTSP_MSG_TRANSPORT_FLAG_UNICAST			(1<<1)
	#define RTSP_MSG_TRANSPORT_FLAG_MULTICAST		(1<<2)
	#define RTSP_MSG_TRANSPORT_FLAG_CLIENT_PORT		(1<<3)
	#define RTSP_MSG_TRANSPORT_FLAG_SERVER_PORT		(1<<4)
	#define RTSP_MSG_TRANSPORT_FLAG_INTERLEAVED		(1<<5)
		uint32_t ssrc;
		uint16_t client_port;//rtcp is rtp + 1
		uint16_t server_port;
		uint8_t interleaved;
	} rtsp_msg_transport_s;

	typedef enum __rtsp_msg_time_type_e
	{
		RTSP_MSG_TIME_TYPE_SMPTE = 0,
		RTSP_MSG_TIME_TYPE_NPT,
		RTSP_MSG_TIME_TYPE_UTC,
		RTSP_MSG_TIME_TYPE_BUTT,
	} rtsp_msg_time_type_e;

	typedef struct __rtsp_msg_time_smpte_s
	{
		//10:07:33:05.01
		uint32_t seconds;	//10*3600 + 07*60 + 33
		uint32_t subframes;	//05*100 + 01
	} rtsp_msg_time_smpte_s;

	typedef struct __rtsp_msg_time_npt_s
	{
		//123.45
		uint32_t secords;	//123
		uint32_t usecords;	//45
	} rtsp_msg_time_npt_s;

	typedef struct __rtsp_msg_time_utc_s
	{
		//19961108T142730.25Z
		uint32_t secords; //1996/11/08 14:27:30 - 1900/1/1 0:0:0
		uint32_t usecords; //25 
	} rtsp_msg_time_utc_s;

	//Range Rr opt. PLAY,PAUSE,RECORD
	typedef struct __rtsp_msg_range_s
	{
		rtsp_msg_time_type_e type;
		union __start_u
		{
			rtsp_msg_time_smpte_s smpte;
			rtsp_msg_time_npt_s npt;
			rtsp_msg_time_utc_s utc;
		} start;
		union __end_u
		{
			rtsp_msg_time_smpte_s smpte;
			rtsp_msg_time_npt_s npt;
			rtsp_msg_time_utc_s utc;
		} end;
	} rtsp_msg_range_s;

	typedef enum __rtsp_msg_content_type_e
	{
		RTSP_MSG_CONTENT_TYPE_SDP = 0,
		RTSP_MSG_CONTENT_TYPE_RTSL,
		RTSP_MSG_CONTENT_TYPE_MHEG,
		RTSP_MSG_CONTENT_TYPE_BUTT,
	} rtsp_msg_content_type_e;

	//Accept R opt. entity
	typedef struct __rtsp_msg_accept_s
	{
		uint32_t accept;
	#define RTSP_MSG_ACCEPT_SDP		(1<<RTSP_MSG_CONTENT_TYPE_SDP)
	#define RTSP_MSG_ACCEPT_RTSL	(1<<RTSP_MSG_CONTENT_TYPE_RTSL)
	#define RTSP_MSG_ACCEPT_MHEG	(1<<RTSP_MSG_CONTENT_TYPE_MHEG)
	} rtsp_msg_accept_s;

	//Authorization R opt. all
	typedef struct __rtsp_msg_authorization_s
	{
		char authorization[128];
	} rtsp_msg_authorization_s;

	//User-Agent R opt. all
	typedef struct __rtsp_msg_user_agent_s
	{
		char user_agent[64];
	} rtsp_msg_user_agent_s;

	//Public r opt. all
	typedef struct __rtsp_msg_public_s
	{
		uint32_t public_;
	#define RTSP_MSG_PUBLIC_OPTIONS		(1<<RTSP_MSG_METHOD_OPTIONS)
	#define RTSP_MSG_PUBLIC_DESCRIBE	(1<<RTSP_MSG_METHOD_DESCRIBE)
	#define RTSP_MSG_PUBLIC_SETUP		(1<<RTSP_MSG_METHOD_SETUP)
	#define RTSP_MSG_PUBLIC_PLAY		(1<<RTSP_MSG_METHOD_PLAY)
	#define RTSP_MSG_PUBLIC_RECORD		(1<<RTSP_MSG_METHOD_RECORD)
	#define RTSP_MSG_PUBLIC_PAUSE		(1<<RTSP_MSG_METHOD_PAUSE)
	#define RTSP_MSG_PUBLIC_TEARDOWN	(1<<RTSP_MSG_METHOD_TEARDOWN)
	#define RTSP_MSG_PUBLIC_ANNOUNCE	(1<<RTSP_MSG_METHOD_ANNOUNCE)
	#define RTSP_MSG_PUBLIC_SET_PARAMETER	(1<<RTSP_MSG_METHOD_SET_PARAMETER)
	#define RTSP_MSG_PUBLIC_GET_PARAMETER	(1<<RTSP_MSG_METHOD_GET_PARAMETER)
	#define RTSP_MSG_PUBLIC_REDIRECT	(1<<RTSP_MSG_METHOD_REDIRECT)
	} rtsp_msg_public_s;

	typedef struct __rtsp_msg_rtp_subinfo_s
	{
		rtsp_msg_uri_s url;
		uint32_t isseq;
		union __param_u
		{
			uint32_t rtptime;
			uint32_t seq;
		} param;
	} rtsp_msg_rtp_subinfo_s;

	//RTP-Info r req. PLAY
	typedef struct __rtsp_msg_rtp_info_s
	{
		uint32_t ninfos;
		rtsp_msg_rtp_subinfo_s **info_array;
	} rtsp_msg_rtp_info_s;

	//Server r opt. all
	typedef struct __rtsp_msg_server_s
	{
		char server[64];
	} rtsp_msg_server_s;

	//Content-Length e req. SET_PARAMETER,ANNOUNCE
	//Content-Length e req. entity
	typedef struct __rtsp_msg_content_length_s
	{
		uint32_t length;
	} rtsp_msg_content_length_s;

	//Content-Type e req. SET_PARAMETER,ANNOUNCE
	//Content-Type r req. entity
	typedef struct __rtsp_msg_content_type_s
	{
		rtsp_msg_content_type_e type;
	} rtsp_msg_content_type_s;

	typedef struct __rtsp_msg_hdr_s
	{
		union __start_line_u
		{
			rtsp_msg_request_line_s reqline;
			rtsp_msg_response_line_s resline;
			rtsp_msg_interleaved_line_s interline;
		} startline;

		//general-headers
		rtsp_msg_cseq_s *cseq;
		rtsp_msg_date_s *date;
		rtsp_msg_session_s *session;
		rtsp_msg_transport_s *transport;
		rtsp_msg_range_s *range;

		//request-headers
		rtsp_msg_accept_s *accept;
		rtsp_msg_authorization_s *authorization;
		rtsp_msg_user_agent_s *user_agent;

		//response-headers
		rtsp_msg_public_s *public_;
		rtsp_msg_rtp_info_s *rtp_info;
		rtsp_msg_server_s *server;

		//entity-headers
		rtsp_msg_content_length_s *content_length;
		rtsp_msg_content_type_s *content_type;
	} rtsp_msg_hdr_s;

	typedef struct __rtsp_msg_body_s
	{
		void *body;
	} rtsp_msg_body_s;

	typedef struct __rtsp_msg_s
	{
		rtsp_msg_type_e type;
		rtsp_msg_hdr_s  hdrs;
		rtsp_msg_body_s body;
	} rtsp_msg_s;

	//bases
	void *rtsp_mem_alloc(int size);
	void  rtsp_mem_free(void *ptr);
	void *rtsp_mem_dup(const void *ptr, int size);
	char *rtsp_str_dup(const char *str);

	int  rtsp_msg_init(rtsp_msg_s *msg);
	void rtsp_msg_free(rtsp_msg_s *msg);

	//return data's bytes which is parsed. when success
	//return 0. when data is not enough
	//return -1. when data is invalid
	int rtsp_msg_parse_from_array(rtsp_msg_s *msg, const void *data, int size);

	//return data's bytes which is used. when success
	//return -1. when failed
	int rtsp_msg_build_to_array(const rtsp_msg_s *msg, void *data, int size);

	//utils XXX
	int rtsp_msg_set_request(rtsp_msg_s *msg, rtsp_msg_method_e mt, const char *ipaddr, const char *abspath);
	int rtsp_msg_set_response(rtsp_msg_s *msg, int status_code);
	int rtsp_msg_get_cseq(const rtsp_msg_s *msg, uint32_t *cseq);
	int rtsp_msg_set_cseq(rtsp_msg_s *msg, uint32_t cseq);
	int rtsp_msg_get_session(const rtsp_msg_s *msg, uint32_t *session);
	int rtsp_msg_set_session(rtsp_msg_s *msg, uint32_t session);
	int rtsp_msg_get_date(const rtsp_msg_s *msg, char *date, int len);
	int rtsp_msg_set_date(rtsp_msg_s *msg, const char *date);
	int rtsp_msg_set_transport_udp(rtsp_msg_s *msg, uint32_t ssrc, int client_port, int server_port);
	int rtsp_msg_set_transport_tcp(rtsp_msg_s *msg, uint32_t ssrc, int interleaved);
	int rtsp_msg_get_accept(const rtsp_msg_s *msg, uint32_t *accept);
	int rtsp_msg_set_accept(rtsp_msg_s *msg, uint32_t accept);
	int rtsp_msg_get_user_agent(const rtsp_msg_s *msg, char *user_agent, int len);
	int rtsp_msg_set_user_agent(rtsp_msg_s *msg, const char *user_agent);
	int rtsp_msg_get_public(const rtsp_msg_s *msg, uint32_t *public_);
	int rtsp_msg_set_public(rtsp_msg_s *msg, uint32_t public_);
	int rtsp_msg_get_server(const rtsp_msg_s *msg, char *server, int len);
	int rtsp_msg_set_server(rtsp_msg_s *msg, const char *server);
	int rtsp_msg_get_content_type(const rtsp_msg_s *msg, int *type);
	int rtsp_msg_set_content_type(rtsp_msg_s *msg, int type);
	int rtsp_msg_get_content_length(const rtsp_msg_s *msg, int *length);
	int rtsp_msg_set_content_length(rtsp_msg_s *msg, int length);

	uint32_t rtsp_msg_gen_session_id(void);

#ifdef __cplusplus
}
#endif
#endif
