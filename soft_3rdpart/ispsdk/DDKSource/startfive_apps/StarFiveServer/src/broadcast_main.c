#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef __WIN32__
#include <winsock2.h>
#include <WS2tcpip.h>  
#include <iphlpapi.h>
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#endif //__WIN32__


#ifdef __LINUX__
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
     
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#endif //__LINUX__

#include "comm.h"
#include "main.h"
#include "tinycthread.h"
#include "tuning_base.h"
#include "tuning_main.h"
#include "broadcast_main.h"
#include "rtsp_demo.h"


int broadcast_main_listen_task(void* pparameters)
{
	SOCKET sock_broadcast;
	struct sockaddr_in addr_broadcast;
	struct sockaddr_in addr_client;
	SOCKLEN addrlen = sizeof(addr_broadcast);
	int ret, recvlen, sendlen, command, port;
	char szrecvbuf[BROADCAST_BUFFER_LEN]; 
	char szsendbuf[BROADCAST_BUFFER_LEN];
	char szip[32];

	usleep(200000); //wait for rtsp thread run until wake up
	
	(void)pparameters; // Just to prevent compiler warnings.

	main_print_app_info(BROADCAST_SERVER_NAME, BROADCAST_SERVER_VERSION, BROADCAST_SERVER_PORT);

	if (!broadcast_main_get_adapter_ip(szip, 32, 3))
	{
		err("get adapter ip failed\n");
		return -1;
	}

	sock_broadcast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_broadcast == INVALID_SOCKET)
	{
		err("create broadcast socket failed : %d\n", sock_broadcast);
		return -1;
	}

	/* Construct bind structure */
	memset(&addr_broadcast, 0, sizeof(addr_broadcast));   /* Zero out structure */
	addr_broadcast.sin_family = AF_INET;                 /* Internet address family */
	addr_broadcast.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
	addr_broadcast.sin_port = htons(BROADCAST_SERVER_PORT); /* Broadcast port */

	/* Bind to the broadcast port */
	ret = bind(sock_broadcast, (struct sockaddr*) &addr_broadcast, sizeof(addr_broadcast));
	if (ret == SOCKET_ERROR)
	{
		err("bind broadcast socket to address failed : %d\n", ret);
		closesocket(sock_broadcast);
		return -1;
	}
	printf("Starting the broadcast server on port %d ... \n", BROADCAST_SERVER_PORT);

	g_cfg.task_status |= TASK_READY_BROADCAST;
	do
	{
		//receive broadcast message
		printf("Wait from broadcast message .....\n");
		if ((recvlen = recvfrom(sock_broadcast, szrecvbuf, BROADCAST_BUFFER_LEN, 0, (struct sockaddr*)&addr_client, &addrlen)) < 0)
		{
			err("Broadcast receive message failed : %d.\n", recvlen);
			closesocket(sock_broadcast);
			return -1;
		}
		if (recvlen < BROADCAST_BUFFER_LEN)
			szrecvbuf[recvlen] = '\0';
		else
			szrecvbuf[BROADCAST_BUFFER_LEN - 1] = '\0';
		printf("Broadcast received data form [%s:%d].\n", inet_ntoa(addr_client.sin_addr), htons(addr_client.sin_port));
		printf("-----recv-----\n%s\n-----recv-----\n", szrecvbuf);

		command = broadcast_main_parse_recvbuf(szrecvbuf, &port);
		switch (command)
		{
			case BRCMD_GET_ALL:
			case BRCMD_GET_TUNING:
			case BRCMD_GET_STREAM:
			case BRCMD_GET_RTSP:
				broadcast_main_generate_all_buf(command, szip, szsendbuf, BROADCAST_BUFFER_LEN);
				break;
			case BRCMD_BYE:
				g_cfg.task_status |= TASK_DONE_BROADCAST;
				info("Broadcast bye bye.\n");
				break;
			default:
				err("Broadcast unknow command !!\n");
		}

		if (command != BRCMD_NULL && command != BRCMD_BYE)
		{
			if (port > BROADCAST_SERVER_PORT)
			{
				addr_client.sin_port = htons(port);
				sendlen = sendto(sock_broadcast, szsendbuf, strlen(szsendbuf), 0, (struct sockaddr*)&addr_client, sizeof(addr_client));
				if (sendlen != strlen(szsendbuf))
				{
					err("Broadcast send feedback message failed : %d.\n", sendlen);
					closesocket(sock_broadcast);
					return -1;
				}
				printf("Broadcast send to [%s:%d], %d byte success.\n", inet_ntoa(addr_client.sin_addr), htons(addr_client.sin_port), sendlen);
			}
			else
			{
				err("Broadcast send to port error: %d.\n", port);
			}
		}
	} while (g_cfg.task_status & TASK_READY_BROADCAST);

	closesocket(sock_broadcast);

	main_print_app_end(BROADCAST_SERVER_NAME);
	return 0;
}

int broadcast_main_get_adapter_ip(char* pbuf, uint32_t buflen, int method)
{
	memset(pbuf, 0, buflen);

	int sockfd, portno;
	struct sockaddr_in serv_addr, local_addr;
	struct hostent* server;
	socklen_t local_len = sizeof(local_addr);

	memset(pbuf, 0, buflen);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		err("ERROR opening socket");
		return 0;
	}

	portno = 80;
	server = gethostbyname("www.wxwidgets.org");
	if (server == NULL)
	{
		err("ERROR, no such host\n");
		return 0;
	}

	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = *(long*)(server->h_addr_list[0]);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		err("ERROR connecting");
		return 0;
	}

	getsockname(sockfd, (struct sockaddr*)&local_addr, &local_len);

	printf("Local address: %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

	snprintf(pbuf, buflen, "%s", inet_ntoa(local_addr.sin_addr));

	closesocket(sockfd);

	return 1;
}

BOOL broadcast_main_get_host_ip(char* pbuf, uint32_t buflen)
{
	char szhost[64];
	struct hostent* phost;
	int i = 0;

	if (pbuf == NULL)
		return FALSE;

	if (gethostname(szhost, sizeof(szhost)) == SOCKET_ERROR)
	{
		printf("gethostname() error for hostname: %s\n", szhost);
		return FALSE;
	}	


	phost = gethostbyname(szhost);
	if (phost == NULL)
	{
		printf("gethostbyname() error for hostname: %s\n", szhost);
		return FALSE;
	}

	printf("hostname: %s\n", phost->h_name);
	printf("addrtype: %d\n", phost->h_addrtype);
	while (phost->h_aliases[i] != NULL)
	{
		printf(" aliases: %s\n", phost->h_aliases[i]);
		i++;
	}
	while (phost->h_addr_list[i] != NULL)
	{
		printf("      ip: %s\n", inet_ntoa(*(struct in_addr*)phost->h_addr_list[i]));
		i++;
	}

	if (phost->h_addr_list[0] != NULL)
	{
		sprintf(pbuf, "%s", inet_ntoa(*(struct in_addr*)phost->h_addr_list[0]));
		return TRUE;
	}

	return FALSE;
}

void broadcast_main_generate_all_buf(int command, char* pszip, char* pbuf, uint32_t buflen)
{
	char szpath[128];
	memset(pbuf, 0, buflen);
	strcpy(pbuf, "StarFive\n");

	if (command == BRCMD_GET_ALL)
		strcat(pbuf, "RetAll\n");
	else if (command == BRCMD_GET_TUNING)
		strcat(pbuf, "RetTuning\n");
	else if (command == BRCMD_GET_STREAM)
		strcat(pbuf, "RetStream\n");
	else if (command == BRCMD_GET_RTSP)
		strcat(pbuf, "RetRtsp\n");


	if (command == BRCMD_GET_TUNING || command == BRCMD_GET_ALL)
	{
		sprintf(szpath, "tuning://%s:%d\n", pszip, g_cfg.tuning_port);
		strcat(pbuf, szpath);
	}
	if (command == BRCMD_GET_STREAM || command == BRCMD_GET_ALL)
	{
		sprintf(szpath, "stream://%s:%d\n", pszip, g_cfg.stream_port);
		strcat(pbuf, szpath);
	}
	if (command == BRCMD_GET_RTSP || command == BRCMD_GET_ALL)
	{
		//printf("broadcast: session_count=%d\n", g_cfg.session_count);
		for (int ch = 0; ch < g_cfg.session_count; ch++)
		{
			//printf("broadcast: ch=%d, vfp=%p\n", ch, g_cfg.session[ch].vfp);
			if (g_cfg.session[ch].vfp)
			{
				sprintf(szpath, "rtsp://%s:%d%s\n", pszip, g_cfg.rtsp_port, g_cfg.session[ch].path);
				strcat(pbuf, szpath);
			}
		}
	}
	
	printf("-----buf-----\n%s-----buf-----\n", pbuf);
}

void broadcast_main_generate_tuning_buf(char* pszip, char* pbuf, uint32_t buflen)
{
	char szpath[128];

	memset(pbuf, 0, buflen);
	strcpy(pbuf, "StarFive\n");
	strcat(pbuf, "RetTuning\n");

	sprintf(szpath, "tuning://%s:%d\n", pszip, g_cfg.tuning_port);
	strcat(pbuf, szpath);

	printf("-----tuning_buf-----\n%s\n-----tuning_buf-----\n", pbuf);
}

void broadcast_main_generate_rtsp_buf(char* pszip, char* pbuf, uint32_t buflen)
{
	char szpath[128];
	int ch;

	memset(pbuf, 0, buflen);
	strcpy(pbuf, "StarFive\n");
	strcat(pbuf, "RetRtsp\n");

	//printf("broadcast: session_count=%d\n", g_cfg.session_count);
	for (ch = 0; ch < g_cfg.session_count; ch++)
	{
		//printf("broadcast: ch=%d, vfp=%p\n", ch, g_cfg.session[ch].vfp);
		if (g_cfg.session[ch].vfp)
		{
			sprintf(szpath, "rtsp://%s:%d%s\n", pszip, g_cfg.rtsp_port, g_cfg.session[ch].path);
			strcat(pbuf, szpath);
		}

	}
	printf("-----rtsp_buf-----\n%s\n-----rtsp_buf-----\n", pbuf);
}

int broadcast_main_parse_recvbuf(char* pbuf, int* pPort)
{
	char* pdelim = "\n";
	char* psubstr = NULL;
	int line = 0;
	int cmd = BRCMD_NULL;

	psubstr = strtok(pbuf, pdelim);
	while (psubstr)
	{
		line++;
		if (line == 1)
		{
			if (strcmp(psubstr, "StarFive") != 0)
				break;
		}
		else if (line == 2)
		{
			if (strcmp(psubstr, "GetAll") == 0)
				cmd = BRCMD_GET_ALL;
			else if (strcmp(psubstr, "GetTuning") == 0)
				cmd = BRCMD_GET_TUNING;
			else if (strcmp(psubstr, "GetRtsp") == 0)
				cmd = BRCMD_GET_RTSP;
			else if (strcmp(psubstr, "GetStream") == 0)
				cmd = BRCMD_GET_STREAM;
			else if (strcmp(psubstr, "Bye") == 0)
				cmd = BRCMD_BYE;
		}
		else if (line == 3)
		{
			if (pPort)
				*pPort = atoi(psubstr);
		}
		psubstr = strtok(NULL, pdelim);
	}

	return cmd;
}

