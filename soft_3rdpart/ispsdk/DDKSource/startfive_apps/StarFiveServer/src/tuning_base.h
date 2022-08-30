#ifndef __TUNING_BASE_H__
#define __TUNING_BASE_H__

#include <stdint.h>
#include "comm.h"

#define MODID_RANGE							9999
#define MODID_GEN_BASE						0
#define MODID_ISP_BASE						10000
#define MODID_ISPOST_BASE					20000
#define MODID_H264_BASE						30000
#define CMD_DATA_BUF_SIZE                   (1024 * 16)
#define CMD_FILE_BUF_SIZE                   250

typedef enum _CMD_DIR
{
	CMD_DIR_GET = 0,
	CMD_DIR_SET,
	CMD_DIR_MAX
} CMD_DIR, *PCMD_DIR;

typedef enum _CMD_TYPE
{
	CMD_TYPE_SINGLE = 0,
	CMD_TYPE_GROUP,
	CMD_TYPE_GROUP_ASAP,
	CMD_TYPE_ALL,
	CMD_TYPE_FILE,
	CMD_TYPE_MAX
} CMD_TYPE, *PCMD_TYPE;

typedef enum _GEN_ID
{
	GEN_GET_BASE = 0,
	GEN_GET_SERVER_VERSION,
	GET_GET_SERVER_ALIVE_COUNT,
	GEN_GET_REGISTER,
	GEN_GET_MEMORY,
	GEN_GET_IMAGE_INFO,
	GEN_GET_IMAGE_DATA,
	GEN_GET_IMAGE_DATA_ONLY,
	GEN_GET_IMAGE_POP,
	GEN_GET_ISP_VERSION,
	GEN_GET_MODULE_ENABLE,
	GEN_GET_MODULE_UPDATE,
	GEN_GET_CONTROL_ENABLE,
	GEN_GET_EXPOSURE_GAIN,
	GEN_GET_AWB_GAIN,
	GEN_GET_SENSOR_REG,

	GEN_SET_BASE = 100,
	GEN_SET_SERVER_DEBUG_ENABLE,
	GEN_SET_REGISTER,
	GEN_SET_MEMORY,
	GEN_SET_ISP_START,
	GEN_SET_ISP_STOP,
	GEN_SET_MODULE_ENABLE,
	GEN_SET_MODULE_UPDATE,
	GEN_SET_CONTROL_ENABLE,
	GEN_SET_EXPOSURE_GAIN,
	GEN_SET_AWB_GAIN,
	GEN_SET_SENSOR_REG,
	GEN_SET_SETTING_FILE,
	GEN_SET_DEL_SETTING_FILE,
	GEN_SET_RESTORE_SETTING,

	GEN_SET_FILE = 9988,
} GEN_ID;

typedef union _UTVALUE
{
	INT64 i64Value;
	UINT64 u64Value;
	DOUBLE dValue;
	int32_t i32Value;
	uint32_t u32Value;
	FLOAT fValue;
	int16_t i16Value;
	uint16_t u16Value;
	int8_t i8Value;
	uint8_t u8Value;
	BOOL bValue;
} UTVALUE, *PUTVALUE;

typedef union _UTPARA
{
	uint32_t u32Value;
	struct
	{
		uint32_t u16ParamOrder : 16;	//[15:0]:parameter order.
		uint32_t u8Rev16 : 8;           //[23:16]:revered.
		uint32_t u4GroupIdx : 4;        //[27:24}:group index.
		uint32_t u3Type : 3;            //[30:28]:command type.
		uint32_t u1Dir : 1;             //[31]:command direction.
	};
} UTPARA, *PUTPARA;

typedef union _UTAPPEND
{
	uint32_t u32Data[5];
} UTAPPEND, *PUTAPPEND;

typedef struct _STCOMD
{
	uint32_t u32Cmd;
	UTPARA utPara;
	uint32_t u32Ver;
	uint32_t u32Size;
	UTVALUE utVal;
	UTAPPEND utApp;
} STCOMD;

typedef struct _STCOMDDATA
{
	STCOMD stComd;
	char szBuffer[CMD_DATA_BUF_SIZE];
} STCOMDDATA;

typedef struct _STQSDKVER
{
	uint32_t u32Version;
	uint8_t u8QsdkCmdTbl[256];
} STQSDKVER;

typedef struct _SC_AE_WS_R_G
{
	uint32_t u32R;
	uint32_t u32G;
} SC_AE_WS_R_G;

typedef struct _SC_AE_WS_B_Y
{
	uint32_t u32B;
	uint32_t u32Y;
} SC_AE_WS_B_Y;

typedef struct _SC_AWB_PS_R_G
{
	uint32_t u32R;
	uint32_t u32G;
} SC_AWB_PS_R_G;

typedef struct _SC_AWB_PS_B_CNT
{
	uint32_t u32B;
	uint32_t u32CNT;
} SC_AWB_PS_B_CNT;

typedef struct _SC_AWB_WGS_W_RW
{
	uint32_t u32W;
	uint32_t u32RW;
} SC_AWB_WGS_W_RW;

typedef struct _SC_AWB_WGS_GW_BW
{
	uint32_t u32GW;
	uint32_t u32BW;
} SC_AWB_WGS_GW_BW;

typedef struct _SC_AWB_WGS_GRW_GBW
{
	uint32_t u32GRW;
	uint32_t u32GBW;
} SC_AWB_WGS_GRW_GBW;

typedef struct _SC_AF_ES_DAT_CNT
{
	uint32_t u32DAT;
	uint32_t u32CNT;
} SC_AF_ES_DAT_CNT;

typedef struct _SC_AE_HIST_R_G
{
	uint32_t u32R;
	uint32_t u32G;
} SC_AE_HIST_R_G;

typedef struct _SC_AE_HIST_B_Y
{
	uint32_t u32B;
	uint32_t u32Y;
} SC_AE_HIST_B_Y;


typedef struct _SC_DATA
{
	SC_AE_WS_R_G AeWsRG[16];
	SC_AE_WS_B_Y AeWsBY[16];
	SC_AWB_PS_R_G AwbPsRG[16];
	SC_AWB_PS_B_CNT AwbPsBCNT[16];
	SC_AWB_WGS_W_RW AwbWgsWRW[16];
	SC_AWB_WGS_GW_BW AwbWgsGWBW[16];
	SC_AWB_WGS_GRW_GBW AwbWgsGRWGBW[16];
	SC_AF_ES_DAT_CNT AfEsDATCNT[16];
} SC_DATA;

typedef struct _SC_HIST
{
	SC_AE_HIST_R_G AeHistRG[64];
	SC_AE_HIST_B_Y AeHistBY[64];
} SC_HIST;

typedef struct _SC_DUMP
{
	SC_DATA ScData[16];
	SC_HIST ScHist;
} SC_DUMP;

#pragma pack(push, 1)

typedef struct _ST_EXPO_GAIN
{
    uint32_t u32Exposure;
    DOUBLE dGain;
} ST_EXPO_GAIN;

typedef struct _ST_AWB_GAIN
{
    DOUBLE dRGain;
    DOUBLE dBGain;
    DOUBLE dDGain;
} ST_AWB_GAIN;

#pragma pack(pop)


uint32_t tuning_base_get_client_count();
void tuning_base_initial_client(void);
uint16_t tuning_base_insert_client(SOCKET sock);
uint16_t tuning_base_remove_client(SOCKET sock);
void tuning_base_dump_client();
void tuning_base_dump_data(void* pData, uint32_t size);
void tuning_base_client_print(SOCKET sock, char* pstring);
int32_t tuning_base_tcp_send(SOCKET sock, void* pdata, int32_t length);
int32_t tuning_base_tcp_recv(SOCKET sock, void* pdata, int32_t length);
int32_t tuning_base_send_command(SOCKET sock, uint32_t mod, uint32_t dir, uint32_t type, uint32_t datasize, UTVALUE val);
int32_t tuning_base_send_data_command(SOCKET sock, STCOMD* pcomd, void* pdata, uint32_t datasize);
int32_t tuning_base_send_file(SOCKET sock, char* pfilename, void* pdata, uint32_t datasize);
int32_t tuning_base_receive_file(SOCKET sock, STCOMDDATA* pcomddata);
int32_t tuning_base_send_packet(SOCKET sock, void* pdata, uint32_t datasize);





#endif //__TUNING_BASE_H__

