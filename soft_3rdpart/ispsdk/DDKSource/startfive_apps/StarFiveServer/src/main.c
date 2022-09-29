#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include "comm.h"
#include "rtsp_demo.h"
#include "main.h"
#include "tinycthread.h"
#include "tuning_main.h"
#include "ispc_main.h"
#include "broadcast_main.h"


//#define ENABLE_RTSP_SERVER


SERVER_CFG_S g_cfg;

int main_load_config(SERVER_CFG_S* pcfg, const char* pfile)
{
	//cfgline: path=%s video=%s audio=%s
	FILE* fp = fopen(pfile, "r");
	char line[256];
	int count = 0;

	if (!fp)
	{
		err("open %s failed\n", pfile);
		return -1;
	}

	memset(pcfg, 0, sizeof(*pcfg));
	while (fgets(line, sizeof(line) - 1, fp))
	{
		const char* p;
		memset(&pcfg->session[count], 0, sizeof(pcfg->session[count]));

		if (line[0] == '#')
			continue;

		if ((p = strstr(line, "tuning_port=")))
		{
			if (sscanf(p, "tuning_port=%d", &pcfg->tuning_port) != 1)
			{
				err("parse tuning port failed %s\n", p);
			}
			continue;
		}

		if ((p = strstr(line, "rtsp_port=")))
		{
			if (sscanf(p, "rtsp_port=%d", &pcfg->rtsp_port) != 1)
			{
				err("parse rtsp port failed %s\n", p);
			}
			continue;
		}

		if ((p = strstr(line, "rtsp_fps=")))
		{
			if (sscanf(p, "rtsp_fps=%d", &pcfg->rtsp_fps) != 1)
			{
				err("parse rtsp FPS failed %s\n", p);
			}
			continue;
		}

		if ((p = strstr(line, "stream_port=")))
		{
			if (sscanf(p, "stream_port=%d", &pcfg->stream_port) != 1)
			{
				err("parse stream port failed %s\n", p);
			}
			continue;
		}

		p = strstr(line, "path=");
		if (!p)
			continue;
		if (sscanf(p, "path=%s", pcfg->session[count].path) != 1)
			continue;
		if ((p = strstr(line, "video=")))
		{
			if (sscanf(p, "video=%s", pcfg->session[count].vszfile) != 1)
			{
				err("parse video file failed %s\n", p);
			}
		}

		if (strlen(pcfg->session[count].vszfile))
		{
			count++;
		}
		else
		{
			err("parse line %s failed\n", line);
		}
	}
	pcfg->session_count = count;
	fclose(fp);
	return count;
}

int main_parse_filename_ext(const char* pfilename)
{
	const char* p;

	p = strrchr(pfilename, '.');
	if (strcmp(p, ".h264") == 0)
		return RTSP_CODEC_ID_VIDEO_H264;
	else if (strcmp(p, ".raw") == 0)
		return RTSP_CODEC_ID_VIDEO_RAW;
	else if (strcmp(p, ".fraw") == 0)
		return RTSP_CODEC_ID_VIDEO_FRAW;
	else if (strcmp(p, ".yuv") == 0)
		return RTSP_CODEC_ID_VIDEO_YUV;
	else if (strcmp(p, ".uo") == 0)
		return RTSP_CODEC_ID_VIDEO_UO;
	else if (strcmp(p, ".ss0") == 0)
		return RTSP_CODEC_ID_VIDEO_SS0;
	else if (strcmp(p, ".ss1") == 0)
		return RTSP_CODEC_ID_VIDEO_SS1;
	else if (strcmp(p, ".dump") == 0)
		return RTSP_CODEC_ID_VIDEO_DUMP;
	else
		return RTSP_CODEC_ID_NONE;
}

int main_parse_filename_info(const char* pfilename, uint32_t* pwidth, uint32_t* pheight, uint32_t* pbit, uint32_t* pmosaic)
{
	char* pdelim = "_";
	char* pdupstr = strdup(pfilename);
	char* psubstr = NULL;
	const char* p;
	int n;

	*pwidth = *pheight = *pbit = *pmosaic = 0;
	psubstr = strtok(pdupstr, pdelim);
	do
	{
		if ((p = strstr(psubstr, "x")))
		{
			if ((n = sscanf(psubstr, "%dx%d", pwidth, pheight)) != 2)
			{
				warn("parse file pname field \"width x height\" fail: %s\n", psubstr);
			}
		}
		else if ((p = strstr(psubstr, "bit")))
		{
			if ((n = sscanf(psubstr, "%dbit", pbit)) != 1)
			{
				warn("parse file pname field \"bit\" fail: %s\n", psubstr);
			}
		}
		else if ((p = strstr(psubstr, "rggb")))
		{
			*pmosaic = BAYER_RGGB;
		}
		else if ((p = strstr(psubstr, "grbg")))
		{
			*pmosaic = BAYER_GRBG;
		}
		else if ((p = strstr(psubstr, "gbrg")))
		{
			*pmosaic = BAYER_GBRG;
		}
		else if ((p = strstr(psubstr, "bggr")))
		{
			*pmosaic = BAYER_BGGR;
		}
		else if ((p = strstr(psubstr, "yu12")))
		{
			*pmosaic = BAYER_YU12;
		}
		else if ((p = strstr(psubstr, "nv12")))
		{
			*pmosaic = BAYER_NV12;
		}
		psubstr = strtok(NULL, pdelim);
	} while (psubstr);

	free(pdupstr);

	return main_parse_filename_ext(pfilename);
}

void main_print_char_line(char ch, int len, int cr)
{
	int i;

	for (i = 0; i < len; i++)
		printf("%c", ch);
	if (cr)
		printf("\n");
}

void main_print_name_line(char* pname, int len, int cr)
{
	int name_len = strlen(pname);
	int space_len = (len - name_len) / 2;

	main_print_char_line(' ', space_len, 0);
	printf("%s", pname);
	main_print_char_line(' ', space_len, 0);
	if (cr)
		printf("\n");
}

void main_print_app_info(char* pname, uint32_t version, uint32_t port)
{
	int line_len = LINE_LEN;
	int ver_hi = version >> 16;
	int ver_lo = version & 0xFFFF;

	printf("\n");
	main_print_char_line('=', line_len, 1);
	main_print_name_line(pname, line_len, 1);
	main_print_char_line('=', line_len, 1);
	printf(" Version  : %d.%02d\n", ver_hi, ver_lo);
	printf(" Port     : %d\n", port);
	printf(" Copyright (c) 2019\n");
	main_print_char_line('=', line_len, 1);
}

void main_print_app_end(char* pname)
{
	int line_len = LINE_LEN;
	main_print_char_line('*', line_len, 1);
	main_print_name_line(pname, line_len, 1);
	main_print_name_line("bye bye", line_len, 1);
	main_print_char_line('*', line_len, 1);
}

void get_peer_ip_port(SOCKET sock)
{
	struct sockaddr_in addr;
	SOCKLEN len = sizeof(addr);
	getpeername(sock, (struct sockaddr*) &addr, &len);
	printf("Peer Address: %s : %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

void get_sock_ip_port(SOCKET sock)
{
	struct sockaddr_in addr;
	SOCKLEN len = sizeof(addr);
	getsockname(sock, (struct sockaddr*) & addr, &len);
	printf("Sock Address: %s : %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

int parse_cmd(int argc, char* argv[], char* cfg_name, int* width, int* height)
{
	int i;
	int cfg_done = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcasecmp(argv[i], "-w") == 0)
		{
			i++;
			*width = atoi(argv[i]);
		}
		else if (strcasecmp(argv[i], "-h") == 0)
		{
			i++;
			*height = atoi(argv[i]);
		}
		else if (cfg_done == 0)
		{
			strcpy(cfg_name, argv[i]);
			cfg_done = 1;
		}
	}
}

int main(int argc, char* argv[])
{
	thrd_t thrd_tuning, thrd_rtsp, thrd_broadcast;
	char szcfgname[100] = SERVER_CONFIG_FILE;
	int width = -1, height = -1;
	int ret;

	if (argc >= 2)
		parse_cmd(argc, argv, szcfgname, &width, &height);

	printf("Load config file: %s\n", szcfgname);
	ret = main_load_config(&g_cfg, szcfgname);
	if (ret < 0)
	{
		err("load config file fail !!!\n");
		return -1;
	}

	g_cfg.task_status = 0;
	ret = ispc_main_driver_isp_start_w_h(width, height);
	if (ret)
	{
		err("failed to start the ISP thread !!!\n");
		return -1;
	}
	//usleep(1000000);
	ret = ispc_main_driver_capture_start();
	if (ret)
	{
		err("failed to enter the ISP capture mode !!!\n");
		ispc_main_driver_capture_stop();
		return -1;
	}

	//Start Tuning
	if (thrd_create(&thrd_tuning, tuning_main_listen_task, (void*)0) != thrd_success)
	{
		err("Create task [tuning] fail !\n");
		return -1;
	}
	printf("Wait task [tuning] ready ...\n");
	while ((g_cfg.task_status & TASK_READY_TUNING) != TASK_READY_TUNING)
	{
		usleep(100);
	}
	printf("Task [tuning] is ready !!!\n");

#if defined(ENABLE_RTSP_SERVER)
	//Start RTSP
	if (thrd_create(&thrd_rtsp, rtsp_server_main, (void*)0) != thrd_success)
	{
		err("Create task [rtsp] fail !\n");
		return -1;
	}
	printf("Wait task [rtsp] ready ...\n");
	while ((g_cfg.task_status & TASK_READY_RTSP) != TASK_READY_RTSP)
	{
		usleep(100);
	}
	printf("Task [rtsp] is ready !!!\n");

#endif //#if defined(ENABLE_RTSP_SERVER)
	//Start Broadcast
	if (thrd_create(&thrd_broadcast, broadcast_main_listen_task, (void*)0) != thrd_success)
	{
		err("Create task [broadcast] fail !\n");
		return -1;
	}
	printf("Wait task [broadcast] ready ...\n");
	while ((g_cfg.task_status & TASK_READY_BROADCAST) != TASK_READY_BROADCAST)
	{
		usleep(100);
	}
	printf("Task [broadcast] is ready !!!\n");

	printf("All task is launch !!!\n");
	thrd_join(thrd_tuning, NULL);

	ispc_main_driver_capture_stop();
	usleep(1000);
	ispc_main_driver_isp_stop();
	printf("bye !!!\n");

	return 0;
}
