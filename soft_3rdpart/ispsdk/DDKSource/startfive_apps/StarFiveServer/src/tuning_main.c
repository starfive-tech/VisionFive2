#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include "comm.h"
#include "main.h"
#include "tinycthread.h"
#include "tuning_base.h"
#include "tuning_main.h"
#include "tuning_service.h"
#include "rtsp_demo.h"
#include "ispc_main.h"

/*-----------------------------------------------------------*/
uint32_t g_alive_count = 1;
BOOL g_terminate_listening = FALSE;
BOOL g_tuning_debug_enable = FALSE;

/*-----------------------------------------------------------*/
int tuning_main_listen_task(void* pparameters)
{
	SOCKET sock_listen, sock_connect;
	struct sockaddr_in inaddr;
	SOCKLEN addrlen = sizeof(inaddr);
	int ret;
	thrd_t thrd_client;

	/* Just to prevent compiler warnings. */
	(void)pparameters;

	if (g_cfg.tuning_port == 0)
		g_cfg.tuning_port = TUNING_SERVER_PORT;

	main_print_app_info(TUNING_SERVER_NAME, TUNING_SERVER_VERSION, g_cfg.tuning_port);
	tuning_base_initial_client();
	tuning_serv_initial();

	sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_listen == INVALID_SOCKET)
	{
		err("create socket failed : %d\n", sock_listen);
		return -1;
	}

	memset(&inaddr, 0, sizeof(inaddr));
	inaddr.sin_family = AF_INET;
	inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inaddr.sin_port = htons(g_cfg.tuning_port);
	ret = bind(sock_listen, (struct sockaddr*)&inaddr, sizeof(inaddr));
	if (ret == SOCKET_ERROR)
	{
		err("bind socket to address failed : %d\n", ret);
		closesocket(sock_listen);
		return -1;
	}
	ret = listen(sock_listen, TUNING_SERVER_MAX_CONNECTION);
	if (ret == SOCKET_ERROR)
	{
		err("listen socket failed : %d\n", ret);
		closesocket(sock_listen);
		return -1;
	}

	printf("Starting the StarFive tuning server on port %d ... \n", g_cfg.tuning_port);

	g_cfg.task_status |= TASK_READY_TUNING;
	do
	{
		if (tuning_base_get_client_count() < TUNING_SERVER_MAX_CONNECTION)
		{
			/* Wait for a client to connect. */
			sock_connect = accept(sock_listen, (struct sockaddr*)&inaddr, &addrlen);
			if (sock_connect == INVALID_SOCKET)
			{
				err("accept failed : %d\n", sock_connect);
				return -1;
			}

			if (thrd_create(&thrd_client, tuning_main_connection_task, (void*)&sock_connect) != thrd_success)
			{
				err("Fail create tuning_main_listen_task !\n");
				return -1;
			}

			tuning_base_insert_client(sock_connect);
			tuning_base_dump_client();
		}
	} while (g_cfg.task_status & TASK_READY_TUNING);

	main_print_app_end(TUNING_SERVER_NAME);
	return 0;
}

/*-----------------------------------------------------------*/
int tuning_main_connection_task(void* pparameters)
{
	SOCKET sock_connected = *(SOCKET*)pparameters;
	SOCKET sock_max = sock_connected;
	STCOMDDATA comddata;
	int32_t bytes_received;
	fd_set fds_read, fds_write, fds_except;
	struct timeval tv;
	int result;

	for (;;)
	{
		FD_ZERO(&fds_read);
		FD_ZERO(&fds_write);
		FD_ZERO(&fds_except);
		FD_SET(sock_connected, &fds_read);
		FD_SET(sock_connected, &fds_write);
		FD_SET(sock_connected, &fds_except);

		memset(&tv, 0, sizeof(tv));
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		result = select(sock_max + 1, &fds_read, NULL, &fds_except, &tv);
		if (result > 0)
		{
			if (FD_ISSET(sock_connected, &fds_read))
			{
				/* Receive another block of data into the cRxedData buffer. */
				bytes_received = tuning_base_tcp_recv(sock_connected, &comddata.stComd, sizeof(STCOMD));
				if (bytes_received > 0)
				{
					bytes_received = tuning_main_porcess_data(sock_connected, &comddata);
					if (bytes_received < 0)
					{
						err("tuning_main_porcess_data error ...\n");
						break;
					}
				}
				else
				{
					/* Error (maybe the connected socket already shut down the socket?). */
					err("connected shutdown ...\n");
					break;
				}
			}
			if (FD_ISSET(sock_connected, &fds_except))
			{
				info("fds_except ...\n");
				break;
			}
		}
	}

	printf("Close the connect %08X ...\n", (uint32_t)sock_connected);
	tuning_base_remove_client(sock_connected);
	tuning_base_dump_client();

	closesocket(sock_connected);

	return 0;
}

int32_t tuning_main_porcess_data(SOCKET sock, STCOMDDATA* pcomddata)
{
	int32_t ret = 0;
	int32_t bytes_received = 0;

	if (GEN_SET_FILE == pcomddata->stComd.u32Cmd &&
		CMD_TYPE_FILE == pcomddata->stComd.utPara.u3Type &&
		CMD_DIR_SET == pcomddata->stComd.utPara.u1Dir)
	{
		bytes_received = tuning_base_receive_file(sock, pcomddata);
		if (bytes_received < 0)
			return -1;
	}
	else
	{
		if (pcomddata->stComd.u32Size > 0)
		{
			bytes_received = tuning_base_tcp_recv(sock, pcomddata->szBuffer, pcomddata->stComd.u32Size);
			if (bytes_received != (int32_t)pcomddata->stComd.u32Size)
				return -1;
		}

		ret = tuning_main_parse_command(sock, pcomddata);

		//if (g_tuning_debug_enable)
		//{
		//	char msg[20];
		//	sprintf(msg, "%d bytes", bytes_received);
		//	tuning_base_client_print(sock, msg);
		//}
	}

	return ret;
}

int32_t tuning_main_parse_command(SOCKET sock, STCOMDDATA* pcomddata)
{
	int32_t ret = 0;
	char szcomd[][8] = { "GEN", "ISP" };

	if (g_tuning_debug_enable)
	{
		if (pcomddata->stComd.u32Cmd != GET_GET_SERVER_ALIVE_COUNT)
		{
			printf("ittd_cmd: %u, [%s] ", pcomddata->stComd.u32Cmd, szcomd[pcomddata->stComd.u32Cmd / 10000]);
			printf("ver: %u, ", pcomddata->stComd.u32Ver);
			printf("u1Dir: %u, ", pcomddata->stComd.utPara.u1Dir);
			printf("u3Type: %u, ", pcomddata->stComd.utPara.u3Type);
			printf("u4GroupIdx: %u, ", pcomddata->stComd.utPara.u4GroupIdx);
			printf("u16ParamOrder: %u, ", pcomddata->stComd.utPara.u16ParamOrder);
			printf("utVal: 0x%016llX, \n", (long long unsigned int)pcomddata->stComd.utVal.u64Value);
			printf("utApp.u32Data[0]: 0x%08X, ", pcomddata->stComd.utApp.u32Data[0]);
			printf("utApp.u32Data[1]: 0x%08X, ", pcomddata->stComd.utApp.u32Data[1]);
			printf("utApp.u32Data[2]: 0x%08X, ", pcomddata->stComd.utApp.u32Data[2]);
			printf("utApp.u32Data[3]: 0x%08X, ", pcomddata->stComd.utApp.u32Data[3]);
			printf("utApp.u32Data[4]: 0x%08X, ", pcomddata->stComd.utApp.u32Data[4]);
			printf("datasize: %u \n", pcomddata->stComd.u32Size);
		}
	}

	if (pcomddata->stComd.u32Cmd >= MODID_GEN_BASE && pcomddata->stComd.u32Cmd < MODID_GEN_BASE + MODID_RANGE)
	{
		ret = tuning_main_parse_general_command(sock, pcomddata);
	}
	else if (pcomddata->stComd.u32Cmd >= MODID_ISP_BASE && pcomddata->stComd.u32Cmd < MODID_ISP_BASE + MODID_RANGE)
	{
		ret = tuning_main_parse_isp_command(sock, pcomddata);
	}

	return ret;
}

int32_t tuning_main_parse_general_command(SOCKET sock, STCOMDDATA* pcomddata)
{
	int32_t ret = 0;

	if (pcomddata->stComd.utPara.u1Dir == CMD_DIR_SET)
	{
		ret = tuning_main_set_general_command(sock, pcomddata);
	}
	else if (pcomddata->stComd.utPara.u1Dir == CMD_DIR_GET)
	{
		ret = tuning_main_get_general_command(sock, pcomddata);
	}

	return ret;
}

int32_t tuning_main_parse_isp_command(SOCKET sock, STCOMDDATA* pcomddata)
{
	int32_t ret = 0;

	if (pcomddata->stComd.utPara.u1Dir == CMD_DIR_SET)
	{
		tuning_serv_set_iq(pcomddata);
	}
	else if (pcomddata->stComd.utPara.u1Dir == CMD_DIR_GET)
	{
		tuning_serv_get_iq(pcomddata);
		ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)pcomddata->szBuffer, pcomddata->stComd.u32Size);
	}

	return ret;
}


int32_t tuning_main_get_general_command(SOCKET sock, STCOMDDATA* pcomddata)
{
	int32_t ret = 0;
	void* pmemaddr;


	switch (pcomddata->stComd.u32Cmd - MODID_GEN_BASE)
	{
		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_SERVER_VERSION:
		{
			pcomddata->stComd.utVal.u32Value = TUNING_SERVER_VERSION;
			if (g_tuning_debug_enable)
				printf("GEN_GET_SERVER_VERSION: 0x%X\n", pcomddata->stComd.utVal.u32Value);
			ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GET_GET_SERVER_ALIVE_COUNT:
		{
			pcomddata->stComd.utVal.u32Value = g_alive_count++;
			if (pcomddata->stComd.utVal.u32Value == 0)
				pcomddata->stComd.utVal.u32Value++;
			ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_REGISTER:
		{
			if (g_tuning_debug_enable)
				printf("GEN_GET_REGISTER: %d\n", pcomddata->stComd.u32Size);
			tuning_serv_get_register(pcomddata);
			ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)pcomddata->szBuffer, pcomddata->stComd.u32Size);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_MEMORY:
		{
			if (g_tuning_debug_enable)
				printf("GEN_GET_MEMORY: %d\n", pcomddata->stComd.u32Size);
			tuning_serv_get_memory(pcomddata);
			ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)pcomddata->szBuffer, pcomddata->stComd.u32Size);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_IMAGE_INFO:
		{
			if (g_tuning_debug_enable)
				printf("GEN_GET_IMAGE_INFO: %d, Kind: %d\n", pcomddata->stComd.u32Size, pcomddata->stComd.utApp.u32Data[0]);
			tuning_serv_get_completed_image_info(pcomddata);
			ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_IMAGE_DATA:
		{
			if (g_tuning_debug_enable)
				printf("GEN_GET_IMAGE_DATA: %d\n", pcomddata->stComd.u32Size);
			tuning_serv_get_completed_image(pcomddata, &pmemaddr);
			ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
			tuning_base_send_packet(sock, pmemaddr, pcomddata->stComd.utApp.u32Data[4]);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_IMAGE_DATA_ONLY:
		{
			if (g_tuning_debug_enable)
				printf("GEN_GET_IMAGE_DATA_ONLY: %d\n", pcomddata->stComd.u32Size);
			tuning_serv_get_completed_image(pcomddata, &pmemaddr);
			tuning_base_send_packet(sock, pmemaddr, pcomddata->stComd.utApp.u32Data[4]);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_IMAGE_POP:
		{
			if (g_tuning_debug_enable)
				printf("GEN_GET_IMAGE_POP: %d\n", pcomddata->stComd.u32Size);
			tuning_serv_get_pop_image(sock, pcomddata);
			break;
		}

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_GET_ISP_VERSION:
        {
            if (g_tuning_debug_enable)
                printf("GEN_GET_ISP_VERSION: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_get_isp_version(pcomddata);
            ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_GET_SENSOR_REG:
        {
            if (g_tuning_debug_enable)
                printf("GEN_GET_SENSOR_REG: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_get_sensor_reg(pcomddata);
            ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_MODULE_ENABLE:
		{
            if (g_tuning_debug_enable)
                printf("GEN_GET_MODULE_ENABLE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_is_module_enable(pcomddata);
            ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
		    break;
		}

        /////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_MODULE_UPDATE:
        {
            if (g_tuning_debug_enable)
                printf("GEN_GET_MODULE_UPDATE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_is_module_update(pcomddata);
            ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_CONTROL_ENABLE:
        {
            if (g_tuning_debug_enable)
                printf("GEN_GET_CONTROL_ENABLE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_is_control_enable(pcomddata);
            ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_EXPOSURE_GAIN:
        {
            if (g_tuning_debug_enable)
                printf("GEN_GET_EXPOSURE_GAIN: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_get_exposure_gain(pcomddata);
            ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
		case GEN_GET_AWB_GAIN:
        {
            if (g_tuning_debug_enable)
                printf("GEN_GET_AWB_GAIN: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_get_awb_gain(pcomddata);
            ret = tuning_base_send_data_command(sock, (STCOMD*)pcomddata, (void*)NULL, 0);
            break;
        }
	}

	return ret;
}

int32_t tuning_main_set_general_command(SOCKET sock, STCOMDDATA* pcomddata)
{
	int32_t s32Ret = 0;
	sock = 0;

	switch (pcomddata->stComd.u32Cmd - MODID_GEN_BASE)
	{
		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_SET_SERVER_DEBUG_ENABLE:
		{
			if (g_tuning_debug_enable)
				printf("GEN_SET_SERVER_DEBUG_ENABLE: %d\n", pcomddata->stComd.utVal.u32Value);
			g_tuning_debug_enable = (BOOL)pcomddata->stComd.utVal.u32Value;
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_SET_REGISTER:
		{
			if (g_tuning_debug_enable)
				printf("GEN_SET_REGISTER: %d\n", pcomddata->stComd.u32Size);
			tuning_serv_set_register(pcomddata);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_SET_MEMORY:
		{
			if (g_tuning_debug_enable)
				printf("GEN_SET_MEMORY: %d\n", pcomddata->stComd.u32Size);
			tuning_serv_set_memory(pcomddata);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_SET_ISP_START:
		{
			if (g_tuning_debug_enable)
				printf("GEN_SET_ISP_START: %d\n", pcomddata->stComd.u32Size);
			ispc_main_driver_isp_start();
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		case GEN_SET_ISP_STOP:
		{
			if (g_tuning_debug_enable)
				printf("GEN_SET_ISP_STOP: %d\n", pcomddata->stComd.u32Size);
			ispc_main_driver_isp_stop();
			break;
		}

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_SENSOR_REG:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_SENSOR_REG: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_set_sensor_reg(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_MODULE_ENABLE:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_MODULE_ENABLE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_module_enable(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_MODULE_UPDATE:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_MODULE_UPDATE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_module_update(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_CONTROL_ENABLE:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_CONTROL_ENABLE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_control_enable(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_EXPOSURE_GAIN:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_EXPOSURE_GAIN: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_set_exposure_gain(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_AWB_GAIN:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_AWB_GAIN: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_set_awb_gain(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_SETTING_FILE:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_SETTING_FILE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_set_setting_file(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_DEL_SETTING_FILE:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_DEL_SETTING_FILE: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_set_delete_setting_file(pcomddata);
            break;
        }

        /////////////////////////////////////////////////////////////////////////////////////
        case GEN_SET_RESTORE_SETTING:
        {
            if (g_tuning_debug_enable)
                printf("GEN_SET_RESTORE_SETTING: %d\n", pcomddata->stComd.u32Size);
            tuning_serv_set_restore_setting(pcomddata);
            break;
        }
	}

	return s32Ret;
}


