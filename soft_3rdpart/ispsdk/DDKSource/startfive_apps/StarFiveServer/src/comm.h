/*************************************************************************
	> File Name: comm.h
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Sunday, December 20, 2015 AM07:37:50 CST
 ************************************************************************/

#ifndef __COMM_H__
#define __COMM_H__

#include <stdio.h>
#include <stdint.h>

//#define dbg(fmt, ...) do {printf("[DEBUG %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
//#define info(fmt, ...) do {printf("[INFO  %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
//#define warn(fmt, ...) do {printf("[WARN  %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
//#define err(fmt, ...) do {printf("[ERROR %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)

#define dbg(fmt, ...) do {printf("[DEBUG] " fmt, ##__VA_ARGS__);} while(0)
//#define info(fmt, ...) do {printf("[INFO] " fmt, ##__VA_ARGS__);} while(0)
#define info printf
#define warn(fmt, ...) do {printf("[WARN] " fmt, ##__VA_ARGS__);} while(0)
#define err(fmt, ...) do {printf("[ERROR] " fmt, ##__VA_ARGS__);} while(0)


#include <unistd.h>
#include <netdb.h>   
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#define RTSP_USE_TCP_IP_PROTOCOL
#define SOCKET_ERROR		(-1)
#define INVALID_SOCKET		(-1)
#define SK_EAGAIN			(EAGAIN)
#define SK_EINTR			(EINTR)
#define closesocket(x)		close(x)
typedef int8_t         		int8_t, * PINT8;
typedef int16_t    		    int16_t, * PINT16;
typedef int32_t 	        int32_t, * PINT32;
typedef int64_t			    INT64, * PINT64;
typedef int8_t			    BOOL, * PBOOL;
typedef int8_t		        CHAR, * PCHAR;
typedef uint8_t       		uint8_t, * PUINT8;
typedef uint16_t      		uint16_t, * PUINT16;
typedef uint32_t        	uint32_t, * PUINT32;
typedef uint64_t			UINT64, * PUINT64;
typedef uint8_t			    BYTE, * PBYTE;
typedef uint16_t		    WORD, * PWORD;
typedef uint32_t	        DWORD, * PDWORD;
typedef float				FLOAT, * PFLOAT;
typedef double				DOUBLE, * PDOUBLE;
typedef int32_t				SOCKET;
typedef socklen_t			SOCKLEN;
#define TRUE				1
#define FALSE				0

#endif

