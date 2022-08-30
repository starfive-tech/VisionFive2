#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>

//#define AUDIO_SUPPORT					1
#define SERVER_CONFIG_FILE				"./data/StarFiveServer.cfg"
#define SESSION_PATH_LEN				64
#define SESSION_FILE_LEN				64
#define SESSION_MAX_NUM					10
#define LINE_LEN						40

#define TASK_READY_TUNING				0x0001
#define TASK_READY_ISPC					0x0002
#define TASK_READY_RTSP					0x0004
#define TASK_READY_BROADCAST			0x0008
#define TASK_READY_STREAM				0x0010

#define TASK_DONE_TUNING				0x0100
#define TASK_DONE_ISPC					0x0200
#define TASK_DONE_RTSP					0x0400
#define TASK_DONE_BROADCAST				0x0800
#define TASK_DONE_STREAM				0x1000


typedef struct server_cfg
{
	int task_status;
	int tuning_port;
	int stream_port;
	int rtsp_port;
	int rtsp_fps;

	int session_count;
	struct
	{
		void* handle;
		char path[SESSION_PATH_LEN];

		char vszfile[SESSION_FILE_LEN];
		int vcodec;
		FILE* vfp;
		uint8_t* vbuf;
		int vbufsize;
		int vflag;

	} session[SESSION_MAX_NUM];
} SERVER_CFG_S;


extern SERVER_CFG_S g_cfg;

int main_load_config(SERVER_CFG_S* pcfg, const char* pfile);
int main_parse_filename_ext(const char* pfilename);
int main_parse_filename_info(const char* pfilename, uint32_t* pwidth, uint32_t* pheight, uint32_t* pbit, uint32_t* pmosaic);
void main_print_char_line(char ch, int len, int cr);
void main_print_name_line(char* pname, int len, int cr);
void main_print_app_info(char* name, uint32_t version, uint32_t port);
void main_print_app_end(char* pname);
void get_peer_ip_port(SOCKET sock);
void get_sock_ip_port(SOCKET sock);

#endif //__MAIN_H__