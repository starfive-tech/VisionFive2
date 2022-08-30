#ifndef __BROADCAST_MAIN_H__
#define __BROADCAST_MAIN_H__

#define BROADCAST_SERVER_NAME					"StarFive Broadcast Server"
#define BROADCAST_SERVER_VERSION				0x00010000
#define BROADCAST_SERVER_PORT					22233
#define BROADCAST_BUFFER_LEN					512

typedef enum
{
	BRCMD_NULL = 0,
	BRCMD_GET_ALL,
	BRCMD_GET_TUNING,
	BRCMD_GET_STREAM,
	BRCMD_GET_RTSP,
	BRCMD_BYE
} BRCMD;

int broadcast_main_listen_task(void* pparameters);
int broadcast_main_get_adapter_ip(char* pbuf, uint32_t buflen, int method);
BOOL broadcast_main_get_host_ip(char* pbuf, uint32_t buflen);
void broadcast_main_generate_all_buf(int command, char* pszip, char* pbuf, uint32_t buflen);
void broadcast_main_generate_tuning_buf(char* pszip, char* pbuf, uint32_t buflen);
void broadcast_main_generate_rtsp_buf(char* pszip, char* pbuf, uint32_t buflen);
int broadcast_main_parse_recvbuf(char* pbuf, int* pPort);

#endif //__BROADCAST_MAIN_H__