/***************************************************************
 * Name:      tuning_service.c
 * Purpose:   Sifive command process
 * Author:    Jazz (jazz.chang@infotm.com)
 * Created:   2019-06-01
 * Copyright: (C) 2019 Shanghai InfoTM Microelectronics Co., Ltd.
 * License:
 **************************************************************/
	
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "tuning_base.h"
#include "tuning_main.h"
#include "tuning_service.h"
#include "ispc_main.h"


void *g_pvoid = NULL;


void tuning_serv_initial()
{

}

int tuning_serv_get_register(STCOMDDATA* pcomddata)
{
	uint32_t i, size, address;
	uint32_t* pdata = NULL;
	uint32_t* pvalue;

    if (!pcomddata)
    {
        printf("tuning_serv_get_register() failed, pcomddata is NULL!\n");
        return -1;
    }

    pdata = (uint32_t*)pcomddata->szBuffer;

	size = *pdata++;
	ispc_main_driver_table_reg_read(size, pdata);
	
	//printf("Get Size: %d\n", size);
	for (i = 0; i < size; i++)
	{
		address = *pdata++;
		pvalue = pdata++;
		//printf("[%d] Address: %08X, Value: %08X\n", i, address, *pvalue);
		//ispc_main_driver_reg_read(address, 1, pvalue);
	}

	return 0;
}

int tuning_serv_set_register(STCOMDDATA* pcomddata)
{
	uint32_t i, size, address;
	uint32_t* pdata = NULL;
	uint32_t* pvalue;

    if (!pcomddata)
    {
        printf("tuning_serv_set_register() failed, pcomddata is NULL!\n");
        return -1;
    }

    pdata = (uint32_t*)pcomddata->szBuffer;

	size = *pdata++;
	//printf("Set Size: %d\n", size);
	for (i = 0; i < size; i++)
	{
		address = *pdata++;
		pvalue = pdata++;
		//printf("[%d] Address: %08X, Value: %08X\n", i, address, *pvalue);
		//ispc_main_driver_reg_write(address, 1, pvalue);
	}
	
	pdata = (uint32_t*)pcomddata->szBuffer;
	pdata++; //skip size
	ispc_main_driver_table_reg_write(size, pdata);

	return 0;
}

int tuning_serv_get_memory(STCOMDDATA* pcomddata)
{
	uint8_t method = 0;
	uint32_t size, address;

    if (!pcomddata)
    {
        printf("tuning_serv_get_memory() failed, pcomddata is NULL!\n");
        return -1;
    }

	address = pcomddata->stComd.utApp.u32Data[0];
	size = pcomddata->stComd.utApp.u32Data[1] / 4;
	pcomddata->stComd.u32Size = size * 4;
	printf("Address: %08X, Size: %08X\n", address, size);

	ispc_main_driver_reg_read(address, size, (uint32_t*)pcomddata->szBuffer);

	return 0;
}

int tuning_serv_set_memory(STCOMDDATA* pcomddata)
{
	uint32_t size, address;

    if (!pcomddata)
    {
        printf("tuning_serv_set_memory() failed, pcomddata is NULL!\n");
        return -1;
    }

	address = pcomddata->stComd.utApp.u32Data[0];
	size = pcomddata->stComd.utApp.u32Data[1] / 4;
	pcomddata->stComd.u32Size = size * 4;
	printf("Address: %08X, Size: %08X\n", address, size);

	ispc_main_driver_reg_write(address, size, (uint32_t*)pcomddata->szBuffer);

	return 0;
}

int tuning_serv_get_completed_image_info(STCOMDDATA* pcomddata)
{
    uint32_t kind;
	ISPC_IMAGE isp_img;

    if (!pcomddata)
    {
        printf("tuning_serv_get_completed_image_info() failed, pcomddata is NULL!\n");
        return -1;
    }

    kind = pcomddata->stComd.utApp.u32Data[0];
	ispc_main_driver_mem_get_completed_info(kind, &isp_img);

	pcomddata->stComd.utVal.u64Value = 0;
	pcomddata->stComd.utVal.u32Value = (isp_img.mosaic << 16) | (kind & 0xFFFF);
	pcomddata->stComd.utApp.u32Data[0] = isp_img.width;
	pcomddata->stComd.utApp.u32Data[1] = isp_img.height;
	pcomddata->stComd.utApp.u32Data[2] = isp_img.stride;
	pcomddata->stComd.utApp.u32Data[3] = isp_img.bit;
	pcomddata->stComd.utApp.u32Data[4] = isp_img.memsize;

	return 0;
}

int tuning_serv_get_completed_image(STCOMDDATA* pcomddata, void** ppmemaddr)
{
    uint32_t kind;
	ISPC_IMAGE isp_img;

    if (!pcomddata)
    {
        printf("tuning_serv_get_completed_image() failed, pcomddata is NULL!\n");
        return -1;
    }

    kind = pcomddata->stComd.utApp.u32Data[0];
    ispc_main_driver_mem_get_completed(kind, &isp_img);

    pcomddata->stComd.utVal.u64Value = 0;
    pcomddata->stComd.utVal.u32Value = (isp_img.mosaic << 16) | (kind & 0xFFFF);
    pcomddata->stComd.utApp.u32Data[0] = isp_img.width;
    pcomddata->stComd.utApp.u32Data[1] = isp_img.height;
    pcomddata->stComd.utApp.u32Data[2] = isp_img.stride;
    pcomddata->stComd.utApp.u32Data[3] = isp_img.bit;
    pcomddata->stComd.utApp.u32Data[4] = isp_img.memsize;
    *ppmemaddr = isp_img.pmemaddr;

    return 0;
}

int tuning_serv_get_pop_image(SOCKET sock, STCOMDDATA* pcomddata)
{
    uint32_t kind, pksize;
    ISPC_IMAGE_PACK pack;
    POP_IMAGE pop_image;
    int stimg = sizeof(POP_IMAGE);
    void *pvoid = NULL;

    kind = pcomddata->stComd.utApp.u32Data[0];
    ispc_main_driver_mem_pop(kind, &pack, &pvoid);

    pksize = 0;
    if (kind & MEM_KIND_UO)
    {
        pksize += pack.img_uo.memsize + sizeof(POP_IMAGE);
    }
    if (kind & MEM_KIND_SS0)
    {
        pksize += pack.img_ss0.memsize + sizeof(POP_IMAGE);
    }
    if (kind & MEM_KIND_SS1)
    {
        pksize += pack.img_ss1.memsize + sizeof(POP_IMAGE);
    }
#if defined(SUPPORT_DUMP_MODULE)
    if (kind & MEM_KIND_DUMP)
    {
        pksize += pack.img_dump.memsize + sizeof(POP_IMAGE);
    }
#endif //#if defined(SUPPORT_DUMP_MODULE)
    if (kind & MEM_KIND_TIL_1_RD)
    {
        pksize += pack.img_til_1_rd.memsize + sizeof(POP_IMAGE);
    }
    if (kind & MEM_KIND_TIL_1_WR)
    {
        pksize += pack.img_til_1_wr.memsize + sizeof(POP_IMAGE);
    }
#if defined(SUPPORT_VIC_2_0)
    if (kind & MEM_KIND_TIL_2_RD)
    {
        pksize += pack.img_til_2_rd.memsize + sizeof(POP_IMAGE);
    }
    if (kind & MEM_KIND_TIL_2_WR)
    {
        pksize += pack.img_til_2_wr.memsize + sizeof(POP_IMAGE);
    }
#endif //#if defined(SUPPORT_VIC_2_0)
    if (kind & MEM_KIND_SC)
    {
        pksize += pack.img_sc.memsize + sizeof(POP_IMAGE);
    }
    if (kind & MEM_KIND_Y_HIST)
    {
        pksize += pack.img_yhist.memsize + sizeof(POP_IMAGE);
    }

    uint8_t* pdata = (uint8_t*)malloc(pksize);
    uint8_t* ptr = pdata;

    if (kind & MEM_KIND_UO)
    {
        pop_image.width = pack.img_uo.width;
        pop_image.height = pack.img_uo.height;
        pop_image.stride = pack.img_uo.stride;
        pop_image.bit = pack.img_uo.bit;
        pop_image.mosaic = pack.img_uo.mosaic;
        pop_image.memsize = pack.img_uo.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_uo.pmemaddr, pack.img_uo.memsize);
        ptr += pack.img_uo.memsize;
    }
    if (kind & MEM_KIND_SS0)
    {
        pop_image.width = pack.img_ss0.width;
        pop_image.height = pack.img_ss0.height;
        pop_image.stride = pack.img_ss0.stride;
        pop_image.bit = pack.img_ss0.bit;
        pop_image.mosaic = pack.img_ss0.mosaic;
        pop_image.memsize = pack.img_ss0.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_ss0.pmemaddr, pack.img_ss0.memsize);
        ptr += pack.img_ss0.memsize;
    }
    if (kind & MEM_KIND_SS1)
    {
        pop_image.width = pack.img_ss1.width;
        pop_image.height = pack.img_ss1.height;
        pop_image.stride = pack.img_ss1.stride;
        pop_image.bit = pack.img_ss1.bit;
        pop_image.mosaic = pack.img_ss1.mosaic;
        pop_image.memsize = pack.img_ss1.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_ss1.pmemaddr, pack.img_ss1.memsize);
        ptr += pack.img_ss1.memsize;
    }
#if defined(SUPPORT_DUMP_MODULE)
    if (kind & MEM_KIND_DUMP)
    {
        pop_image.width = pack.img_dump.width;
        pop_image.height = pack.img_dump.height;
        pop_image.stride = pack.img_dump.stride;
        pop_image.bit = pack.img_dump.bit;
        pop_image.mosaic = pack.img_dump.mosaic;
        pop_image.memsize = pack.img_dump.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_dump.pmemaddr, pack.img_dump.memsize);
        ptr += pack.img_dump.memsize;
    }
#endif //#if defined(SUPPORT_DUMP_MODULE)
    if (kind & MEM_KIND_TIL_1_RD)
    {
        pop_image.width = pack.img_til_1_rd.width;
        pop_image.height = pack.img_til_1_rd.height;
        pop_image.stride = pack.img_til_1_rd.stride;
        pop_image.bit = pack.img_til_1_rd.bit;
        pop_image.mosaic = pack.img_til_1_rd.mosaic;
        pop_image.memsize = pack.img_til_1_rd.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_til_1_rd.pmemaddr, pack.img_til_1_rd.memsize);
        ptr += pack.img_til_1_rd.memsize;
    }
    if (kind & MEM_KIND_TIL_1_WR)
    {
        pop_image.width = pack.img_til_1_wr.width;
        pop_image.height = pack.img_til_1_wr.height;
        pop_image.stride = pack.img_til_1_wr.stride;
        pop_image.bit = pack.img_til_1_wr.bit;
        pop_image.mosaic = pack.img_til_1_wr.mosaic;
        pop_image.memsize = pack.img_til_1_wr.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_til_1_wr.pmemaddr, pack.img_til_1_wr.memsize);
        ptr += pack.img_til_1_wr.memsize;
    }
#if defined(SUPPORT_VIC_2_0)
    if (kind & MEM_KIND_TIL_2_RD)
    {
        pop_image.width = pack.img_til_2_rd.width;
        pop_image.height = pack.img_til_2_rd.height;
        pop_image.stride = pack.img_til_2_rd.stride;
        pop_image.bit = pack.img_til_2_rd.bit;
        pop_image.mosaic = pack.img_til_2_rd.mosaic;
        pop_image.memsize = pack.img_til_2_rd.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_til_2_rd.pmemaddr, pack.img_til_2_rd.memsize);
        ptr += pack.img_til_2_rd.memsize;
    }
    if (kind & MEM_KIND_TIL_2_WR)
    {
        pop_image.width = pack.img_til_2_wr.width;
        pop_image.height = pack.img_til_2_wr.height;
        pop_image.stride = pack.img_til_2_wr.stride;
        pop_image.bit = pack.img_til_2_wr.bit;
        pop_image.mosaic = pack.img_til_2_wr.mosaic;
        pop_image.memsize = pack.img_til_2_wr.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_til_2_wr.pmemaddr, pack.img_til_2_wr.memsize);
        ptr += pack.img_til_2_wr.memsize;
    }
#endif //#if defined(SUPPORT_VIC_2_0)
    if (kind & MEM_KIND_SC)
    {
        pop_image.width = pack.img_sc.width;
        pop_image.height = pack.img_sc.height;
        pop_image.stride = pack.img_sc.stride;
        pop_image.bit = pack.img_sc.bit;
        pop_image.mosaic = pack.img_sc.mosaic;
        pop_image.memsize = pack.img_sc.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_sc.pmemaddr, pack.img_sc.memsize);
        ptr += pack.img_sc.memsize;
    }
    if (kind & MEM_KIND_Y_HIST)
    {
        pop_image.width = pack.img_yhist.width;
        pop_image.height = pack.img_yhist.height;
        pop_image.stride = pack.img_yhist.stride;
        pop_image.bit = pack.img_yhist.bit;
        pop_image.mosaic = pack.img_yhist.mosaic;
        pop_image.memsize = pack.img_yhist.memsize;
        memcpy(ptr, &pop_image, sizeof(POP_IMAGE));
        ptr += sizeof(POP_IMAGE);
        memcpy(ptr, pack.img_yhist.pmemaddr, pack.img_yhist.memsize);
        ptr += pack.img_yhist.memsize;
    }

    if (pdata)
    {
        tuning_base_send_packet(sock, pdata, pksize);
        free(pdata);
    }

    if (pvoid)
    {
        ispc_main_driver_mem_push(pvoid);
        pvoid = NULL;
    }

    return 0;
}

int tuning_serv_get_isp_version(STCOMDDATA* pcomddata)
{
    uint64_t *pisp_sdk_version = NULL;

    if (!pcomddata)
    {
        printf("tuning_serv_get_isp_version() failed, pcomddata is NULL!\n");
        return -1;
    }

    pcomddata->stComd.utVal.u64Value = ispc_main_driver_get_isp_version();
    pisp_sdk_version = (uint64_t *)&pcomddata->stComd.utApp;
    *pisp_sdk_version = ispc_main_driver_get_isp_sdk_version();

    return 0;
}

int tuning_serv_get_sensor_reg(STCOMDDATA* pcomddata)
{
    uint16_t reg_value;

    if (!pcomddata)
    {
        printf("tuning_serv_get_sensor_reg() failed, pcomddata is NULL!\n");
        return -1;
    }

    reg_value = ispc_main_driver_read_sensor_reg(
        pcomddata->stComd.utVal.u16Value
        );
    pcomddata->stComd.utApp.u32Data[0] = reg_value;

    return 0;
}

int tuning_serv_set_sensor_reg(STCOMDDATA* pcomddata)
{

    if (!pcomddata)
    {
        printf("tuning_serv_set_sensor_reg() failed, pcomddata is NULL!\n");
        return -1;
    }

    ispc_main_driver_write_sensor_reg(
        pcomddata->stComd.utVal.u16Value,
        (uint16_t)pcomddata->stComd.utApp.u32Data[0]
        );

    return 0;
}

int tuning_serv_module_enable(STCOMDDATA* pcomddata)
{

    if (!pcomddata)
    {
        printf("tuning_serv_module_enable() failed, pcomddata is NULL!\n");
        return -1;
    }

    ispc_main_driver_module_enable(
        pcomddata->stComd.utVal.u8Value,
        (uint16_t)pcomddata->stComd.utApp.u32Data[0],
        (uint8_t)pcomddata->stComd.utApp.u32Data[1]
        );

    return 0;
}

int tuning_serv_is_module_enable(STCOMDDATA* pcomddata)
{
    uint8_t enable;

    if (!pcomddata)
    {
        printf("tuning_serv_is_module_enable() failed, pcomddata is NULL!\n");
        return -1;
    }

    enable = ispc_main_driver_is_module_enable(
        pcomddata->stComd.utVal.u8Value,
        (uint16_t)pcomddata->stComd.utApp.u32Data[0]
        );
    pcomddata->stComd.utApp.u32Data[1] = enable;

    return 0;
}

int tuning_serv_module_update(STCOMDDATA* pcomddata)
{

    if (!pcomddata)
    {
        printf("tuning_serv_module_update() failed, pcomddata is NULL!\n");
        return -1;
    }

    ispc_main_driver_module_update(
        pcomddata->stComd.utVal.u8Value,
        (uint16_t)pcomddata->stComd.utApp.u32Data[0],
        (uint8_t)pcomddata->stComd.utApp.u32Data[1]
        );

    return 0;
}

int tuning_serv_is_module_update(STCOMDDATA* pcomddata)
{
    uint8_t update;

    if (!pcomddata)
    {
        printf("tuning_serv_is_module_update() failed, pcomddata is NULL!\n");
        return -1;
    }

    update = ispc_main_driver_is_module_update(
        pcomddata->stComd.utVal.u8Value,
        (uint16_t)pcomddata->stComd.utApp.u32Data[0]
        );
    pcomddata->stComd.utApp.u32Data[1] = update;

    return 0;
}

int tuning_serv_control_enable(STCOMDDATA* pcomddata)
{

    if (!pcomddata)
    {
        printf("tuning_serv_control_enable() failed, pcomddata is NULL!\n");
        return -1;
    }

    ispc_main_driver_control_enable(
        pcomddata->stComd.utVal.u8Value,
        (uint16_t)pcomddata->stComd.utApp.u32Data[0],
        (uint8_t)pcomddata->stComd.utApp.u32Data[1]
        );

    return 0;
}

int tuning_serv_is_control_enable(STCOMDDATA* pcomddata)
{
    uint8_t enable;

    if (!pcomddata)
    {
        printf("tuning_serv_is_control_enable() failed, pcomddata is NULL!\n");
        return -1;
    }

    enable = ispc_main_driver_is_control_enable(
        pcomddata->stComd.utVal.u8Value,
        (uint16_t)pcomddata->stComd.utApp.u32Data[0]
        );
    pcomddata->stComd.utApp.u32Data[1] = enable;

    return 0;
}

int tuning_serv_get_exposure_gain(STCOMDDATA* pcomddata)
{
    ST_EXPO_GAIN *pexposure_gain = NULL;
    uint32_t exposure;
    DOUBLE gain;

    if (!pcomddata)
    {
        printf("tuning_serv_get_exposure_gain() failed, pcomddata is NULL!\n");
        return -1;
    }

    ispc_main_driver_get_exposure_and_gain(&exposure, &gain);
    pexposure_gain = (ST_EXPO_GAIN *)(&pcomddata->stComd.utVal.u64Value);
    pexposure_gain->u32Exposure = exposure;
    pexposure_gain->dGain = gain;

    return 0;
}

int tuning_serv_set_exposure_gain(STCOMDDATA* pcomddata)
{
    ST_EXPO_GAIN *pexposure_gain = NULL;

    if (!pcomddata)
    {
        printf("tuning_serv_set_exposure_gain() failed, pcomddata is NULL!\n");
        return -1;
    }

    pexposure_gain = (ST_EXPO_GAIN *)(&pcomddata->stComd.utVal.u64Value);
    ispc_main_driver_set_exposure_and_gain(pexposure_gain->u32Exposure, pexposure_gain->dGain);

    return 0;
}

int tuning_serv_get_awb_gain(STCOMDDATA* pcomddata)
{
    ST_AWB_GAIN *pawb_gain = NULL;
    DOUBLE rgain;
    DOUBLE bgain;
    DOUBLE dgain;

    if (!pcomddata)
    {
        printf("tuning_serv_get_awb_gain() failed, pcomddata is NULL!\n");
        return -1;
    }

    ispc_main_driver_get_awb_gain(&rgain, &bgain, &dgain);
    pawb_gain = (ST_AWB_GAIN *)(&pcomddata->stComd.utVal.u64Value);
    pawb_gain->dRGain = rgain;
    pawb_gain->dBGain = bgain;
    pawb_gain->dDGain = dgain;

    return 0;
}

int tuning_serv_set_awb_gain(STCOMDDATA* pcomddata)
{
    ST_AWB_GAIN *pawb_gain = NULL;

    if (!pcomddata)
    {
        printf("tuning_serv_set_awb_gain() failed, pcomddata is NULL!\n");
        return -1;
    }

    pawb_gain = (ST_AWB_GAIN *)(&pcomddata->stComd.utVal.u64Value);
    ispc_main_driver_set_awb_gain(pawb_gain->dRGain, pawb_gain->dBGain, pawb_gain->dDGain);

    return 0;
}

int tuning_serv_get_iq(STCOMDDATA* pcomddata)
{
	STCOMD *pcomd = NULL;
    void *pparam_buf = NULL;
    uint32_t mod_ctl_id;
    uint16_t param_size = 0;
    int ret = -1;

	if (!pcomddata)
	{
		printf("tuning_serv_get_iq() failed, pcomddata is NULL!\n");
		return -1;
	}

	pcomd = (STCOMD *)pcomddata;
	pparam_buf = (void *)((uint8_t *)pcomddata + sizeof(STCOMD));

    switch (pcomd->utPara.u3Type)
    {
        case CMD_TYPE_SINGLE:
            printf("tuning_serv_get_iq() failed, does not support \"CMD_TYPE_SINGLE\" type!\n");
            break;

        case CMD_TYPE_GROUP:
            printf("tuning_serv_get_iq() failed, does not support \"CMD_TYPE_GROUP\" type!\n");
            break;

        case CMD_TYPE_GROUP_ASAP:
            printf("tuning_serv_get_iq() failed, does not support \"CMD_TYPE_GROUP_ASAP\" type!\n");
            break;

        case CMD_TYPE_ALL:
            mod_ctl_id = pcomd->u32Cmd - MODID_ISP_BASE;
            ret = ispc_main_driver_get_iq_param(mod_ctl_id, pparam_buf, &param_size);
            if (ret)
            {
                pcomd->u32Size = 0;
                ret = -1;
            }
            else
            {
                pcomd->u32Size = param_size;
                ret = 0;
            }
            break;
    }

    return ret;
}

int tuning_serv_set_iq(STCOMDDATA* pcomddata)
{
	STCOMD* pcomd = NULL;
    void *pparam_buf = NULL;
    uint32_t mod_ctl_id;
    uint16_t param_size = 0;
    int ret = -1;

	if (!pcomddata)
	{
		printf("tuning_serv_set_iq() failed, pcomddata is NULL!\n");
		return -1;
	}

	pcomd = (STCOMD *)pcomddata;
	pparam_buf = (void *)((uint8_t *)pcomddata + sizeof(STCOMD));

    switch (pcomd->utPara.u3Type)
    {
        case CMD_TYPE_SINGLE:
            break;

        case CMD_TYPE_GROUP:
            break;

        case CMD_TYPE_GROUP_ASAP:
            break;

        case CMD_TYPE_ALL:
            mod_ctl_id = pcomd->u32Cmd - MODID_ISP_BASE;
            param_size = pcomd->u32Size;
            ret = ispc_main_driver_set_iq_param(mod_ctl_id, pparam_buf, param_size);
            if (ret)
            {
                ret = -1;
            }
            else
            {
                ret = 0;
            }
            break;
    }

    return ret;
}

int tuning_serv_set_setting_file(STCOMDDATA* pcomddata)
{
    STCOMD* pcomd = NULL;
    char szFilename[CMD_FILE_BUF_SIZE];
    char szSetting[CMD_FILE_BUF_SIZE];
    char szTemp[CMD_FILE_BUF_SIZE];
    char szCmd[512];
    char *pszFilename = NULL;
    struct stat stStat;
    int bFileExist;
    int iIdx;
    int32_t s32Width;
    int32_t s32Height;
    int ret;

    if (!pcomddata)
    {
        printf("tuning_serv_set_setting_file() failed, pcomddata is NULL!\n");
        return -1;
    }

    pcomd = (STCOMD *)pcomddata;

    // Get the setting filename.
    strcpy(szTemp, pcomddata->szBuffer);
    //printf("The pcomddata->szBuffer is \'%s\'\n", szTemp);
    pszFilename = strrchr(szTemp, '/');
    if (NULL != pszFilename)
    {
        iIdx = &pszFilename[0] - &szTemp[0];
        iIdx++;
    }
    else
    {
        iIdx = 0;
    }
    strcpy(szFilename, &szTemp[iIdx]);
    //printf("The setting filename is \'%s\'\n", szFilename);

    // Check the file is exist or not?
    bFileExist = (0 == stat(szFilename, &stStat)) ? TRUE : FALSE;
    if (FALSE == bFileExist)
    {
        printf("The setting filename \'%s\' is not exist\n", szFilename);
        return -1;
    }

    ispc_main_driver_get_resolution(&s32Width, &s32Height);
    if ((0 >= s32Width) || (0 >= s32Height)) {
        printf("Error: s32Width = %d, s32Height = %d\n", s32Width, s32Height);
        return -1;
    }

    // Check the seting folder is exist or not?
    sprintf(szSetting, "/root/.isp_setting");
    bFileExist = (0 == stat(szSetting, &stStat)) ? TRUE : FALSE;
    if (FALSE == bFileExist)
    {
        printf("The setting folder \'%s\' is not exist\n", szSetting);
        printf("Create the \'%s\' folder.\n", szSetting);
        sprintf(szCmd, "mkdir %s", szSetting);
        ret = system(szCmd);
    }
    else if (0 == (stStat.st_mode & S_IFDIR))
    {
        printf("The \'%s\' is not a folder!!!\n", szSetting);
        return -1;
    }

    // Remove backup file.
    sprintf(szTemp, "/root/.isp_setting/IspSetting_%dx%d.bak",
        s32Width, s32Height);
    bFileExist = (0 == stat(szTemp, &stStat)) ? TRUE : FALSE;
    if (TRUE == bFileExist)
    {
        sprintf(szCmd, "rm -rf %s", szTemp);
        ret = system(szCmd);
    }

    // Rename setting file as backup file.
    sprintf(szSetting, "/root/.isp_setting/IspSetting_%dx%d.ybn",
        s32Width, s32Height);
    bFileExist = (0 == stat(szSetting, &stStat)) ? TRUE : FALSE;
    if (TRUE == bFileExist)
    {
        sprintf(szCmd, "mv %s %s", szSetting, szTemp);
        ret = system(szCmd);
    }

    // Move setting file to setting folder.
    sprintf(szCmd, "mv %s %s", szFilename, szSetting);
    ret = system(szCmd);
    //printf("Move setting file to setting folder completed!!!\n");

    return 0;
}

int tuning_serv_set_delete_setting_file(STCOMDDATA* pcomddata)
{
    char szSetting[CMD_FILE_BUF_SIZE];
    char szCmd[512];
    struct stat stStat;
    int bFileExist;
    int32_t s32Width;
    int32_t s32Height;
    int ret;

    if (!pcomddata)
    {
        printf("tuning_serv_set_delete_setting_file() failed, "\
            "pcomddata is NULL!\n");
        return -1;
    }

    ispc_main_driver_get_resolution(&s32Width, &s32Height);
    if ((0 >= s32Width) || (0 >= s32Height)) {
        printf("Error: s32Width = %d, s32Height = %d\n", s32Width, s32Height);
        return -1;
    }

    // Remove backup file.
    sprintf(szSetting, "/root/.isp_setting/IspSetting_%dx%d.bak",
        s32Width, s32Height);
    bFileExist = (0 == stat(szSetting, &stStat)) ? TRUE : FALSE;
    if (TRUE == bFileExist)
    {
        sprintf(szCmd, "rm -rf %s", szSetting);
        ret = system(szCmd);
    }

    // Remove setting file.
    sprintf(szSetting, "/root/.isp_setting/IspSetting_%dx%d.ybn",
        s32Width, s32Height);
    bFileExist = (0 == stat(szSetting, &stStat)) ? TRUE : FALSE;
    if (TRUE == bFileExist)
    {
        sprintf(szCmd, "rm -rf %s", szSetting);
        ret = system(szCmd);
    }

    return 0;
}

int tuning_serv_set_restore_setting(STCOMDDATA* pcomddata)
{
    int ret = -1;

    if (!pcomddata)
    {
        printf("tuning_serv_set_delete_setting_file() failed, "\
            "pcomddata is NULL!\n");
        return -1;
    }

    ret = ispc_main_driver_restore_setting(pcomddata->stComd.utVal.u8Value);

    return ret;
}
