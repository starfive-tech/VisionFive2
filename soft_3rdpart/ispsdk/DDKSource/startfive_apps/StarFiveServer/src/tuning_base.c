#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "comm.h"
#include "main.h"
#include "tinycthread.h"
#include "tuning_base.h"
#include "tuning_main.h"
#include "tuning_service.h"
#include "rtsp_demo.h"


/*-----------------------------------------------------------*/
static uint32_t g_client_count = 0;
static SOCKET g_sock_client_array[TUNING_SERVER_MAX_CONNECTION];



/*-----------------------------------------------------------*/
uint32_t tuning_base_get_client_count()
{
	return g_client_count;
}

void tuning_base_initial_client(void)
{
	uint16_t i;
	for (i = 0; i < TUNING_SERVER_MAX_CONNECTION; i++)
		g_sock_client_array[i] = 0;
}

uint16_t tuning_base_insert_client(SOCKET sock)
{
	uint16_t i;
	for (i = 0; i < TUNING_SERVER_MAX_CONNECTION; i++)
	{
		if (g_sock_client_array[i] == 0)
		{
			g_sock_client_array[i] = sock;
			g_client_count++;
			return i;
		}
	}
	return 0xFFFF;
}

uint16_t tuning_base_remove_client(SOCKET sock)
{
	uint16_t i;
	for (i = 0; i < TUNING_SERVER_MAX_CONNECTION; i++)
	{
		if (g_sock_client_array[i] == sock)
		{
			g_sock_client_array[i] = 0;
			g_client_count--;
			return i;
		}
	}
	return 0xFFFF;
}

void tuning_base_dump_client()
{
	SOCKET sock;
	struct sockaddr_in inaddr;
	SOCKLEN addrlen = sizeof(inaddr);
	uint16_t i;

	printf("Client count: %d\n", g_client_count);
	printf("----+----------+-------------------------------------------------\n");

	for (i = 0; i < g_client_count; i++)
	{
		sock = g_sock_client_array[i];
		if (sock != 0)
		{
			getpeername(sock, (struct sockaddr*)&inaddr, &addrlen);
			printf(" %02d | %08X | %s:%d\n",
				   i,
				   (uint32_t)sock,
				   inet_ntoa(inaddr.sin_addr),
				   ntohs(inaddr.sin_port));
		}
	}

	printf("----+----------+-------------------------------------------------\n");
}

void tuning_base_dump_data(void* pdata, uint32_t size)
{
	uint8_t* pch = (uint8_t*)pdata;
	uint32_t i;

	printf("-------------------------------\n");
	for (i = 0; i < size; i++)
	{
		printf("%02X ", *(pch + i));
	}
	printf("\n");
	printf("-------------------------------\n");
}

void tuning_base_client_print(SOCKET sock, char* pstring)
{
	struct sockaddr_in inaddr;
	SOCKLEN addrlen = sizeof(inaddr);
	getpeername(sock, (struct sockaddr*)&inaddr, &addrlen);

	printf(" [%08X] [%s:%d] : %s\n",
		(uint32_t)sock,
		   inet_ntoa(inaddr.sin_addr),
		   ntohs(inaddr.sin_port),
		   pstring);
}

int32_t tuning_base_tcp_send(SOCKET sock, void* pdata, int32_t length)
{
	int32_t transmitted = 0, bytessent = 0, trycount = 0;
	uint32_t packetsize;
	uint8_t* pbuffer = (uint8_t*)pdata;

	//printf("Send Size: %d\n", length);

	/* Keep sending until the entire buffer has been sent. */
	while (transmitted < length)
	{
		/* How many bytes are left to send? */
		packetsize = length - transmitted;
		bytessent = send(sock, (const char*)&(pbuffer[transmitted]), packetsize, 0);
		//printf("Send: %d, %d\n", bytessent, transmitted);
		if (bytessent > 0)
		{
			/* Data was sent successfully. */
			transmitted += bytessent;
		}
		else
		{
			int err = sk_errno();
			if (err == SK_EAGAIN || err == SK_EINTR)
			{
				trycount++;
				printf("Try send again: %d\n", trycount);
				if (trycount <= 10)
					continue;
			}

			printf("Send error: %d\n", err);
			tuning_base_remove_client(sock);
			tuning_base_dump_client();
			return -1;
		}
	}

	return transmitted;
}

int32_t tuning_base_tcp_recv(SOCKET sock, void* pdata, int32_t length)
{
	int32_t received = 0, bytesrecv = 0, trycount = 0;
	uint32_t packetsize;
	uint8_t* pbuffer = (uint8_t*)pdata;

	/* Keep sending until the entire buffer has been sent. */
	while (received < length)
	{
		/* How many bytes are left to send? */
		packetsize = length - received;
		bytesrecv = recv(sock, (char*)&(pbuffer[received]), packetsize, 0);
		if (bytesrecv > 0)
		{
			/* Data was sent successfully. */
			received += bytesrecv;
		}
		else if (bytesrecv == 0)
		{
			/* No data was received, but FreeRTOS_recv() did not return an error. Timeout? */
			trycount++;
			if (trycount > 3)
			{
				tuning_base_remove_client(sock);
				tuning_base_dump_client();
				return -1;
			}
		}
		else
		{
			/* Error - break out of the loop for graceful socket close. */
			tuning_base_remove_client(sock);
			tuning_base_dump_client();
			return -1;
		}
	}

	return received;
}

int32_t tuning_base_send_command(SOCKET sock, uint32_t mod, uint32_t dir, uint32_t type, uint32_t datasize, UTVALUE val)
{
	STCOMD comd;
	comd.u32Cmd = mod;
	comd.utPara.u1Dir = dir;
	comd.utPara.u3Type = type;
	comd.utPara.u4GroupIdx = 0;
	comd.u32Size = datasize;
	comd.utVal = val;

	return tuning_base_send_data_command(sock, &comd, (void*)NULL, 0);
}

int32_t tuning_base_send_data_command(SOCKET sock, STCOMD* pcomd, void* pdata, uint32_t datasize)
{
	int32_t transmitted = 0, bytessent = 0;

	if (pcomd != NULL)
	{
		pcomd->u32Size = datasize;
		bytessent = tuning_base_tcp_send(sock, pcomd, sizeof(STCOMD));
		if (bytessent < 0 || bytessent != sizeof(STCOMD))
			return -1;
		transmitted += bytessent;
	}
	if (pdata != NULL && datasize > 0)
	{
		bytessent = tuning_base_tcp_send(sock, pdata, datasize);
		if (bytessent < 0 || (uint32_t)bytessent != datasize)
			return -1;
		transmitted += bytessent;
	}

	return transmitted;
}

int32_t tuning_base_send_file(SOCKET sock, char* pfilename, void* pdata, uint32_t datasize)
{
	FILE* pfd_raed;
	uint32_t filesize;
	UTVALUE val;
	char szfilenamebuf[CMD_FILE_BUF_SIZE];
	uint32_t totalbytes, readbytes, offset;
	int32_t pos, sendbytes;
	uint8_t* pbuffer = (uint8_t*)pdata;

	printf("open file: %s\n", pfilename);
	pfd_raed = fopen(pfilename, "rb");
	if (pfd_raed == NULL)
		return -1;

	pos = ftell(pfd_raed);
	fseek(pfd_raed, 0, SEEK_END);
	filesize = ftell(pfd_raed);
	fseek(pfd_raed, pos, SEEK_SET);
	pos = ftell(pfd_raed);

	val.u32Value = (uint32_t)filesize;
	sendbytes = tuning_base_send_command(sock, GEN_SET_FILE, CMD_DIR_GET, CMD_TYPE_FILE, CMD_FILE_BUF_SIZE, val);
	if (sendbytes < 0 || sendbytes != sizeof(STCOMD))
		return -1;
	printf("Send file Length: %d\n", (uint32_t)filesize);

	strncpy(szfilenamebuf, pfilename, CMD_FILE_BUF_SIZE);
	sendbytes = tuning_base_send_data_command(sock, (STCOMD*)NULL, (void*)szfilenamebuf, CMD_FILE_BUF_SIZE);
	if (sendbytes < 0 || sendbytes != CMD_FILE_BUF_SIZE)
		return -1;
	printf("Send file name: %d\n", (uint32_t)sendbytes);


	totalbytes = (uint32_t)filesize;
	do
	{
		readbytes = (totalbytes > datasize) ? datasize : totalbytes;
		readbytes = fread(pbuffer, 1, readbytes, pfd_raed);
		offset = 0;

		do
		{
			printf("Send file offset: %d, TotalBytes: %d\n", offset, totalbytes);
			sendbytes = tuning_base_tcp_send(sock, pbuffer + offset, readbytes);
			if (sendbytes < 0 || sendbytes != (int32_t)readbytes)
			{
				printf("Send file [%s] error!!! err: %d\n", pfilename, sendbytes);
				fclose(pfd_raed);
				return -1;
			}

			offset += sendbytes;
			readbytes -= sendbytes;
			totalbytes -= sendbytes;
		} while (readbytes > 0);
	} while (totalbytes > 0);

	fclose(pfd_raed);
	printf("Send file [%s] success %d bytes.\n", pfilename, filesize);

	return (int32_t)filesize;
}

int32_t tuning_base_receive_file(SOCKET sock, STCOMDDATA* pcomddata)
{
	FILE* pfd_write = NULL;
	int32_t packetsize;
	int32_t leftbytes = 0;
	int32_t bytesrecv = 0;
	char szrecv_filename[CMD_FILE_BUF_SIZE];
	//char szfilepath[CMD_FILE_BUF_SIZE];
	//char szfilename[CMD_FILE_BUF_SIZE];
	int32_t index = 0;
	//char* ptmp;

	leftbytes = pcomddata->stComd.utVal.u32Value;

	// Receive Filename.
	bytesrecv = tuning_base_tcp_recv(sock, szrecv_filename, CMD_FILE_BUF_SIZE);
	if (bytesrecv < 0 || bytesrecv != CMD_FILE_BUF_SIZE)
		return -1;
	//printf("Received file name = %s\n", szrecv_filename);

	// Extract file path and szfilename.
	//szfilepath[0] = '\0';
	//szfilename[0] = '\0';
	//ptmp = strrchr(szrecv_filename, '/');
	//if (ptmp != NULL)
	//{
	//	index = &ptmp[0] - &szrecv_filename[0];
	//	index++;
	//	strncpy(szfilepath, szrecv_filename, (uint32_t)index);
	//	szfilepath[index] = '\0';
	//	strcpy(szfilename, &szrecv_filename[index]);
	//}
	//else
	//{
	//	strcpy(szfilename, szrecv_filename);
	//}
	//printf("File path = %s\n", szfilepath);
	//printf("File name = %s\n", szfilename);

	// Open a file for write the data.
	//if (szfilepath[0] == '\0')
	//	sprintf(szrecv_filename, "/ptmp/%s", szfilename);

	//printf("Try to open %s ...\n", szrecv_filename);
	pfd_write = fopen(szrecv_filename, "wb");
	if (pfd_write == NULL)
	{
		printf("Cannot open %s to store received data.\n", szrecv_filename);
		return -1;
	}

	// Receive File data.
	//printf("Start Receiving file ");
	do
	{
		packetsize = (leftbytes < 4096) ? leftbytes : 4096;
		bytesrecv = tuning_base_tcp_recv(sock, pcomddata->szBuffer, packetsize);
		if (bytesrecv < 0 || bytesrecv != packetsize)
		{
			printf("Receive file [%s] error!!! err: %d\n", szrecv_filename, bytesrecv);
			fclose(pfd_write);
			return -1;
		}

		fwrite(pcomddata->szBuffer, bytesrecv, 1, pfd_write);
		leftbytes -= bytesrecv;
		//printf(".");
	} while (leftbytes > 0);
	//printf("\nReceived file complete.\n");

	// Close this file.
	fclose(pfd_write);

	return (int32_t)pcomddata->stComd.utVal.u32Value;
}

int32_t tuning_base_send_packet(SOCKET sock, void* pdata, uint32_t datasize)
{
	uint8_t* pbuffer = (uint8_t*)pdata;
	uint32_t packetsize = 4096;
	uint32_t remainsize = datasize;
	int32_t sendbytes;
	uint32_t packetparam[2];
	uint32_t packet_cnt = 0;

	//printf("Start send packet ...\n");
	packetparam[0] = datasize;
	packetparam[1] = packetsize;
	sendbytes = tuning_base_tcp_send(sock, packetparam, sizeof(packetparam));
	//printf("datasize: %d, packetsize: %d, sendbyte: %d\n", datasize, packetsize, sendbytes);

	while (remainsize > 0)
	{
		if (remainsize < packetsize)
			packetsize = remainsize;

		sendbytes = tuning_base_tcp_send(sock, pbuffer, packetsize);
		if (sendbytes < 0 || sendbytes != (int32_t)packetsize)
		{
			printf("Send packet error!!! err: %d, remain: %d, send: %.2f%%\n", sendbytes, remainsize, (float)(datasize - remainsize) / (float)datasize * 100);
			return -1;
		}

		pbuffer += sendbytes;
		remainsize -= sendbytes;
		//printf("sendbytes: %d, datasize: %d\n", sendbytes, datasize);
		packet_cnt++;
		if ((remainsize > 0) && ((packet_cnt & 0x000F) == 0))
		{
			usleep(100);
		}
	}
	//printf("Finish send packet !!!\n");
	return 0;
}
