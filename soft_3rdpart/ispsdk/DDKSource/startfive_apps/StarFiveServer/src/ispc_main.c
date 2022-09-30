#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "comm.h"
#include "main.h"
#include "tinycthread.h"
#include "ispc_main.h"
#include "rtsp_demo.h"
#include "broadcast_main.h"
#include "tuning_base.h"

#include "ISPC/stflib_isp_base.h"
#include "stf_isp_api.h"

uint32_t g_ispc_loop = 1;

int ispc_main_task(void* pparameters)
{
	int32_t ret = 0;

	printf("Start ISPC task ....\n");

	g_cfg.task_status |= TASK_READY_ISPC;
	do
	{
	} while (g_cfg.task_status & TASK_READY_ISPC);

	return ret;
}

int32_t ispc_main_load_image(ISPC_IMAGE* pimg, char* pfilename)
{
	int ret = 0;
#if 0

	pimg->pfile = NULL;
	pimg->width = pimg->height = pimg->bit = 0;
	pimg->codec = main_parse_filename_info(pimg->filename, &pimg->width, &pimg->height, &pimg->bit);
	if (pimg->codec == RTSP_CODEC_ID_VIDEO_NV12) {
		pimg->stride = pimg->width;
		pimg->memsize = pimg->width * pimg->height * 3 / 2;
	}
	else if (pimg->codec >= RTSP_CODEC_ID_VIDEO_RGGB && pimg->codec <= RTSP_CODEC_ID_VIDEO_BGGR) {
		pimg->stride = pimg->width;
		pimg->memsize = pimg->width * pimg->height;
	} else {
		pimg->memsize = 0;
	}

	printf("ISPC load image file: %s ... ", pimg->filename);
	pimg->pfile = fopen(pimg->filename, "rb");
	//struct stat st;
	//stat(pimg->filename, &st);
	//pimg->filesize = st.st_size;
	fseek(pimg->pfile, 0, SEEK_END);
	pimg->filesize = ftell(pimg->pfile);
	fseek(pimg->pfile, 0, SEEK_SET);

	if (pimg->memsize == 0) {
		pimg->memsize = pimg->filesize;
	}

	if (pimg->pmemaddr) {
		free(pimg->pmemaddr);
	}
	pimg->pmemaddr = (uint8_t*)malloc(pimg->memsize);
	if (!pimg->pmemaddr) {
		printf("fail !!!\n");
		return 0;
	}
	if (!pimg->pfile) {
		printf("fail !!!\n");
		return 0;
	}
	if (pimg->memsize > pimg->filesize) {
		ret = fread(pimg->pmemaddr, 1, pimg->filesize, pimg->pfile);
	} else {
		ret = fread(pimg->pmemaddr, 1, pimg->memsize, pimg->pfile);
	}
	fclose(pimg->pfile);
	printf("OK !!!\n");
#endif

	return ret;
}

void ispc_main_free_image(ISPC_IMAGE* pimg)
{
}

int32_t ispc_main_load_next_frame(ISPC_IMAGE* pimg)
{
	int32_t ret = -1;

	return ret;
}

int32_t ispc_main_driver_isp_start()
{
	int32_t ret = 0;
	int32_t main_step = 0;

	printf("---- STFAPI_ISP_StartIspMainThread() -----\n");
	ret = STFAPI_ISP_StartIspMainThread();

	printf("Wait STFAPI_ISP_StartIspMainThread() ready ...\n");
	do {
		usleep(10 * 1000);
		main_step = ispc_main_driver_get_main_step();
	} while (main_step != ISP_MAIN_STEP_STOP);
	printf("STFAPI_ISP_StartIspMainThread() is ready !!!\n");

	return ret;
}

int32_t ispc_main_driver_isp_start_w_h(int32_t width, int32_t height)
{
	int32_t ret = 0;
	int32_t main_step = 0;

	printf("---- STFAPI_ISP_SetResolution(%d, %d) -----\n", width, height);
	ret = (int32_t)STFAPI_ISP_SetResolution(width, height);
	//if (ret) {
	//	printf("Failed to call STFAPI_ISP_SetResolution(%d, %d)!!!!!\n",
	//		width, height);
	//	return ret;
	//}

	printf("---- STFAPI_ISP_StartIspMainThread() -----\n");
	ret = STFAPI_ISP_StartIspMainThread();

	printf("Wait STFAPI_ISP_StartIspMainThread() ready ...\n");
	do {
		usleep(10 * 1000);
		main_step = ispc_main_driver_get_main_step();
	} while (main_step != ISP_MAIN_STEP_STOP);
	printf("STFAPI_ISP_StartIspMainThread() is ready !!!\n");

	return ret;
}

int32_t ispc_main_driver_isp_stop()
{
	int32_t ret = 0;

	printf("---- STFAPI_ISP_StopIspMainThread() -----\n");
	ret = STFAPI_ISP_StopIspMainThread();

	return ret;
}

int32_t ispc_main_driver_reg_read(uint32_t offset, uint32_t length, uint32_t* pbuffer)
{
	int32_t ret = 0;

	ret = STFAPI_ISP_RegReadSeries(-1, offset, length, pbuffer);

	return ret;
}

int32_t ispc_main_driver_reg_write(uint32_t offset, uint32_t length, uint32_t* pbuffer)
{
	int32_t ret = 0;

	ret = STFAPI_ISP_RegWriteSeries(-1, offset, length, pbuffer);

	return ret;
}

int32_t ispc_main_driver_table_reg_read(uint32_t length, uint32_t* pbuffer)
{
	int32_t ret = 0;

	ret = STFAPI_ISP_RegReadByTable(-1, length, (ST_REG_TBL *)pbuffer);

	return ret;
}

int32_t ispc_main_driver_table_reg_write(uint32_t length, uint32_t* pbuffer)
{
	int32_t ret = 0;

	ret = STFAPI_ISP_RegWriteByTable(-1, length, (ST_REG_TBL *)pbuffer);

	return ret;
}

void ispc_main_driver_mem_to_img(ISPC_IMAGE* pdst, ST_MEM_INFO* psrc)
{

	pdst->width = psrc->u32ImgWidth;
	pdst->height = psrc->u32ImgHeight;
	pdst->stride = psrc->u32ImgStride;
	pdst->bit = psrc->u32BitDepth;
	pdst->mosaic = psrc->u32MosaicType;
	pdst->memsize = psrc->u32MemSize;
	pdst->pmemaddr = psrc->pvMemBuf;
}

int32_t ispc_main_driver_mem_info_get(uint32_t kind, ISPC_IMAGE_PACK* ppack)
{
	int32_t ret = 0;
	ST_ISP_MEMS_INFO isp_mems_info;

	memset(&isp_mems_info, 0, sizeof(isp_mems_info));
	ret = STFAPI_ISP_MemBufInfoGet(kind, &isp_mems_info);

	//=== Godspeed === Add new memory/buffer type support here.
	if (kind & MEM_KIND_UO)
	{
		ispc_main_driver_mem_to_img(&ppack->img_uo, &isp_mems_info.stMemInfoUo);
	}

	if (kind & MEM_KIND_SS0)
	{
		ispc_main_driver_mem_to_img(&ppack->img_ss0, &isp_mems_info.stMemInfoSs0);
	}

	if (kind & MEM_KIND_SS1)
	{
		ispc_main_driver_mem_to_img(&ppack->img_ss1, &isp_mems_info.stMemInfoSs1);
	}

	if (kind & MEM_KIND_DUMP)
	{
		ispc_main_driver_mem_to_img(&ppack->img_dump, &isp_mems_info.stMemInfoDump);
	}

	if (kind & MEM_KIND_TIL_1_RD)
	{
		ispc_main_driver_mem_to_img(&ppack->img_til_1_rd, &isp_mems_info.stMemInfoTiling_1_Read);
	}

	if (kind & MEM_KIND_TIL_1_WR)
	{
		ispc_main_driver_mem_to_img(&ppack->img_til_1_wr, &isp_mems_info.stMemInfoTiling_1_Write);
	}

	if (kind & MEM_KIND_SC)
	{
		ispc_main_driver_mem_to_img(&ppack->img_sc, &isp_mems_info.stMemInfoScDump);
	}

	if (kind & MEM_KIND_Y_HIST)
	{
		ispc_main_driver_mem_to_img(&ppack->img_yhist, &isp_mems_info.stMemInfoYHist);
	}

	return ret;
}

int32_t ispc_main_driver_mem_get(uint32_t kind, ISPC_IMAGE_PACK* ppack, void *pvoid)
{
	int32_t ret = 0;
	ST_ISP_MEMS_INFO isp_mems_info;

	memset(&isp_mems_info, 0, sizeof(isp_mems_info));
	ret = STFAPI_ISP_MemBufGet(kind, &isp_mems_info, pvoid);

	//=== Godspeed === Add new memory/buffer type support here.
	if (kind & MEM_KIND_UO)
	{
		ispc_main_driver_mem_to_img(&ppack->img_uo, &isp_mems_info.stMemInfoUo);
	}

	if (kind & MEM_KIND_SS0)
	{
		ispc_main_driver_mem_to_img(&ppack->img_ss0, &isp_mems_info.stMemInfoSs0);
	}

	if (kind & MEM_KIND_SS1)
	{
		ispc_main_driver_mem_to_img(&ppack->img_ss1, &isp_mems_info.stMemInfoSs1);
	}

	if (kind & MEM_KIND_DUMP)
	{
		ispc_main_driver_mem_to_img(&ppack->img_dump, &isp_mems_info.stMemInfoDump);
	}

	if (kind & MEM_KIND_TIL_1_RD)
	{
		ispc_main_driver_mem_to_img(&ppack->img_til_1_rd, &isp_mems_info.stMemInfoTiling_1_Read);
	}

	if (kind & MEM_KIND_TIL_1_WR)
	{
		ispc_main_driver_mem_to_img(&ppack->img_til_1_wr, &isp_mems_info.stMemInfoTiling_1_Write);
	}

	if (kind & MEM_KIND_SC)
	{
		ispc_main_driver_mem_to_img(&ppack->img_sc, &isp_mems_info.stMemInfoScDump);
	}

	if (kind & MEM_KIND_Y_HIST)
	{
		ispc_main_driver_mem_to_img(&ppack->img_yhist, &isp_mems_info.stMemInfoYHist);
	}

	return ret;
}

int32_t ispc_main_driver_mem_push(void *pvoid)
{
    int32_t ret = 0;

    ret = STFAPI_ISP_MemBufPush(pvoid);

    return ret;
}

int32_t ispc_main_driver_mem_pop(uint32_t kind, ISPC_IMAGE_PACK* ppack, void **ppvoid)
{
    int32_t ret = 0;
    ST_ISP_MEMS_INFO isp_mems_info;

    memset(&isp_mems_info, 0, sizeof(isp_mems_info));
    ret = STFAPI_ISP_MemBufPop(kind, &isp_mems_info, ppvoid);

    //=== Godspeed === Add new memory/buffer type support here.
	if (kind & MEM_KIND_UO)
	{
		ispc_main_driver_mem_to_img(&ppack->img_uo, &isp_mems_info.stMemInfoUo);
	}

	if (kind & MEM_KIND_SS0)
	{
		ispc_main_driver_mem_to_img(&ppack->img_ss0, &isp_mems_info.stMemInfoSs0);
	}

	if (kind & MEM_KIND_SS1)
	{
		ispc_main_driver_mem_to_img(&ppack->img_ss1, &isp_mems_info.stMemInfoSs1);
	}

	if (kind & MEM_KIND_DUMP)
	{
		ispc_main_driver_mem_to_img(&ppack->img_dump, &isp_mems_info.stMemInfoDump);
	}

	if (kind & MEM_KIND_TIL_1_RD)
	{
		ispc_main_driver_mem_to_img(&ppack->img_til_1_rd, &isp_mems_info.stMemInfoTiling_1_Read);
	}

	if (kind & MEM_KIND_TIL_1_WR)
	{
		ispc_main_driver_mem_to_img(&ppack->img_til_1_wr, &isp_mems_info.stMemInfoTiling_1_Write);
	}

	if (kind & MEM_KIND_SC)
	{
		ispc_main_driver_mem_to_img(&ppack->img_sc, &isp_mems_info.stMemInfoScDump);
	}

	if (kind & MEM_KIND_Y_HIST)
	{
		ispc_main_driver_mem_to_img(&ppack->img_yhist, &isp_mems_info.stMemInfoYHist);
	}

    return ret;
}

int32_t ispc_main_driver_mem_get_completed_info(uint32_t kind, ISPC_IMAGE *pisp_img)
{
    int32_t ret = 0;
    ST_MEM_INFO meminfo;

    memset(&meminfo, 0, sizeof(meminfo));
    ret = STFAPI_ISP_ShotMemBufGet(EN_SHOT_KIND_COMPLETED, kind, &meminfo, NULL, 1);
    //printf("get k=%d, w=%d, h=%d, s=%d, b=%d, m=%d, p=%p\n", kind, meminfo.u32ImgWidth, meminfo.u32ImgHeight, meminfo.u32ImgStride, meminfo.u32BitDepth, meminfo.u32MemSize, meminfo.pvMemBuf);

	ispc_main_driver_mem_to_img(pisp_img, &meminfo);

    return ret;
}

int32_t ispc_main_driver_mem_get_completed(uint32_t kind, ISPC_IMAGE *pisp_img)
{
    int32_t ret = 0;
    ST_MEM_INFO meminfo;

    memset(&meminfo, 0, sizeof(meminfo));
    ret = STFAPI_ISP_ShotMemBufGet(EN_SHOT_KIND_COMPLETED, kind, &meminfo, NULL, 0);
    //printf("get k=%d, w=%d, h=%d, s=%d, b=%d, m=%d, p=%p\n", kind, meminfo.u32ImgWidth, meminfo.u32ImgHeight, meminfo.u32ImgStride, meminfo.u32BitDepth, meminfo.u32MemSize, meminfo.pvMemBuf);

	ispc_main_driver_mem_to_img(pisp_img, &meminfo);

    return ret;
}

uint64_t ispc_main_driver_get_isp_version()
{

    return (uint64_t)STFAPI_ISP_GetIspVersion();
}

uint64_t ispc_main_driver_get_isp_sdk_version()
{

    return (uint64_t)STFAPI_ISP_GetIspSdkVersion();
}

int32_t ispc_main_driver_get_resolution(int32_t* pwidth, int32_t* pheight)
{

    return (int32_t)STFAPI_ISP_GetResolution(pwidth, pheight);
}

int32_t ispc_main_driver_set_resolution(int32_t width, int32_t height)
{

    return (int32_t)STFAPI_ISP_SetResolution(width, height);
}

int32_t ispc_main_driver_capture_start()
{
	int32_t ret = 0;
	int32_t main_step = 0;

	printf("---- STFAPI_ISP_StartIspCapture() -----\n");
	STFAPI_ISP_StartIspCapture();
	printf("Wait STFAPI_ISP_StartIspCapture() ready ...\n");
	do {
		usleep(10 * 1000);
		main_step = ispc_main_driver_get_main_step();
	} while (main_step != ISP_MAIN_STEP_CAPTURE);
	printf("STFAPI_ISP_StartIspCapture() is ready !!!\n");

	return 0;
}

int32_t ispc_main_driver_capture_stop()
{

	printf("---- STFAPI_ISP_StopIspCapture() -----\n");
	STFAPI_ISP_StopIspCapture();

	return 0;
}

int32_t ispc_main_driver_get_main_step()
{

	return (int32_t)STFAPI_ISP_GetIspMainStep();
}

int32_t ispc_main_driver_module_enable(uint8_t isp_idx, uint16_t module_id, uint8_t enable)
{

    return (int32_t)STFAPI_ISP_Pipeline_ModuleEnable(isp_idx, module_id, enable);
}

uint8_t ispc_main_driver_is_module_enable(uint8_t isp_idx, uint16_t module_id)
{

    return (uint8_t)STFAPI_ISP_Pipeline_IsModuleEnable(isp_idx, module_id);
}

int32_t ispc_main_driver_module_update(uint8_t isp_idx, uint16_t module_id, uint8_t update)
{

    return (int32_t)STFAPI_ISP_Pipeline_ModuleUpdate(isp_idx, module_id, update);
}

uint8_t ispc_main_driver_is_module_update(uint8_t isp_idx, uint16_t module_id)
{

    return (uint8_t)STFAPI_ISP_Pipeline_IsModuleUpdate(isp_idx, module_id);
}

int32_t ispc_main_driver_control_enable(uint8_t isp_idx, uint16_t control_id, uint8_t enable)
{

    return (int32_t)STFAPI_ISP_Pipeline_ControlEnable(isp_idx, control_id, enable);
}

uint8_t ispc_main_driver_is_control_enable(uint8_t isp_idx, uint16_t control_id)
{

    return (uint8_t)STFAPI_ISP_Pipeline_IsControlEnable(isp_idx, control_id);
}

int32_t ispc_main_driver_get_exposure_and_gain(uint32_t *pexposure, double *pgain)
{
    int32_t ret = 0;
    uint32_t exposure = 0;
    double gain = 0.0;

    ret = STFAPI_ISP_GetExposureAndGain(&exposure, &gain);
    *pexposure = exposure;
    *pgain = gain;
	
    return ret;
}

int32_t ispc_main_driver_set_exposure_and_gain(uint32_t exposure, double gain)
{

    return (int32_t)STFAPI_ISP_SetExposureAndGain(exposure, gain);
}

int32_t ispc_main_driver_get_awb_gain(double *prgain, double *pbgain, double *pdgain)
{
    int32_t ret = 0;
    double rgain = 0.0;
    double bgain = 0.0;
    double dgain = 0.0;

    ret = STFAPI_ISP_GetAwbGain(&rgain, &bgain, &dgain);
    *prgain = rgain;
    *pbgain = bgain;
    *pdgain = dgain;
	
    return ret;
}

int32_t ispc_main_driver_set_awb_gain(double rgain, double bgain, double dgain)
{
    int32_t ret = 0;

    ret = STFAPI_ISP_SetAwbGain(rgain, bgain, dgain);
	
    return ret;
}

uint16_t ispc_main_driver_read_sensor_reg(uint16_t reg_addr)
{
    uint16_t reg_value = 0;

    reg_value = STFAPI_ISP_ReadSensorReg(reg_addr);

    return reg_value;
}

int32_t ispc_main_driver_write_sensor_reg(uint16_t reg_addr, uint16_t reg_value)
{
    int32_t ret = 0;

    ret = STFAPI_ISP_WriteSensorReg(reg_addr, reg_value);
	
    return ret;
}

int32_t ispc_main_driver_get_iq_param(uint32_t mod_ctl_id, void *pparam_buf, uint16_t *pparam_size)
{
    int32_t ret = 0;

    ret = STFAPI_ISP_GetIqParam(mod_ctl_id, pparam_buf, pparam_size);
	
    return ret;
}

int32_t ispc_main_driver_set_iq_param(uint32_t mod_ctl_id, void *pparam_buf, uint16_t param_size)
{
    int32_t ret = 0;

    ret = STFAPI_ISP_SetIqParam(mod_ctl_id, pparam_buf, param_size);

    return ret;
}

int32_t ispc_main_driver_restore_setting(uint8_t setting_kind)
{
    int32_t ret = 0;

    ret = STFAPI_ISP_RestoreSetting(setting_kind);

    return ret;
}
