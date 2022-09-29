#ifndef __ISPC_MAIN_H__
#define __ISPC_MAIN_H__


#define TESTIMG_FILE "./data/TestImage4_1920x1080_8bit.nv12.yuv"


typedef enum _MEM_KIND
{
    MEM_KIND_UO = 0x0001,
    MEM_KIND_SS0 = 0x0002,
    MEM_KIND_SS1 = 0x0004,
    MEM_KIND_DUMP = 0x0008,
    MEM_KIND_TIL_1_RD = 0x0010,
    MEM_KIND_TIL_1_WR = 0x0020,
    MEM_KIND_SC = 0x0100,
    MEM_KIND_Y_HIST = 0x0200,
    MEM_KIND_OUT = (MEM_KIND_UO
                  | MEM_KIND_SS0
                  | MEM_KIND_SS1
                  ),
    MEM_KIND_TIL_1 = (MEM_KIND_TIL_1_RD
                    | MEM_KIND_TIL_1_WR
                    ),
    MEM_KIND_TIL = (MEM_KIND_TIL_1
                  ),
    MEM_KIND_STAT = (MEM_KIND_SC
                   | MEM_KIND_Y_HIST
                   ),
    MEM_KIND_ALL = (MEM_KIND_OUT
                  | MEM_KIND_DUMP
                  | MEM_KIND_TIL
                  | MEM_KIND_STAT
                  ),
} MEM_KIND;

typedef enum _BAYER_TYPE
{
    BAYER_NONE = 0,
	BAYER_RGGB,
	BAYER_GRBG,
	BAYER_GBRG,
	BAYER_BGGR,
	BAYER_YU12, /*YUV420P, I420, [YU12: YYYYYYYY UUVV]*/
	BAYER_NV12  /*YUV420SP, [NV12: YYYYYYYY UVUV]*/
} BAYER_TYPE;

typedef enum _ISP_MAIN_STEP
{
    ISP_MAIN_STEP_NONE = 0,
    ISP_MAIN_STEP_BUFFER_INIT,
    ISP_MAIN_STEP_PREPARE_STOP,
    ISP_MAIN_STEP_STOP,
    ISP_MAIN_STEP_PREPARE_CAPTURE,
    ISP_MAIN_STEP_CAPTURE,
} ISP_MAIN_STEP;


typedef struct _ISPC_MEM
{
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t bit;
    uint32_t mosaic;
    uint32_t memsize;
    uint8_t *pmemaddr;
} ISPC_MEM;

typedef struct _ISPC_IMAGE
{ 
	FILE* pfile;
	char filename[64];
	uint32_t filesize;
	uint32_t offset;
	//uint32_t kind;
	uint32_t codec;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t bit;
	uint32_t mosaic;
	uint32_t memsize;
	uint8_t* pmemaddr;
} ISPC_IMAGE;

//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ISPC_IMAGE_PACK
{
	ISPC_IMAGE img_uo;
	ISPC_IMAGE img_ss0;
	ISPC_IMAGE img_ss1;
	ISPC_IMAGE img_dump;
	ISPC_IMAGE img_til_1_rd;
	ISPC_IMAGE img_til_1_wr;
	ISPC_IMAGE img_sc;
	ISPC_IMAGE img_yhist;
} ISPC_IMAGE_PACK;

typedef struct _POP_IMAGE
{
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t bit;
	uint32_t mosaic;
	uint32_t memsize;
} POP_IMAGE;


int ispc_main_task(void* pparameters);

int32_t ispc_main_load_image(ISPC_IMAGE* pimg, char* pfilename);
void ispc_main_free_image(ISPC_IMAGE* pimg);
int32_t ispc_main_load_next_frame(ISPC_IMAGE* pimg);
int32_t ispc_main_driver_isp_start();
int32_t ispc_main_driver_isp_start_w_h(int32_t width, int32_t height);
int32_t ispc_main_driver_isp_stop();
int32_t ispc_main_driver_reg_read(uint32_t offset, uint32_t length, uint32_t* pbuffer);
int32_t ispc_main_driver_reg_write(uint32_t offset, uint32_t length, uint32_t* pbuffer);
int32_t ispc_main_driver_table_reg_read(uint32_t length, uint32_t* pbuffer);
int32_t ispc_main_driver_table_reg_write(uint32_t length, uint32_t* pbuffer);
int32_t ispc_main_driver_mem_info_get(uint32_t kind, ISPC_IMAGE_PACK* ppack);
int32_t ispc_main_driver_mem_get(uint32_t kind, ISPC_IMAGE_PACK* ppack, void *pvoid);
int32_t ispc_main_driver_mem_push(void *pvoid);
int32_t ispc_main_driver_mem_pop(uint32_t kind, ISPC_IMAGE_PACK* ppack, void **ppvoid);
int32_t ispc_main_driver_mem_get_completed_info(uint32_t kind, ISPC_IMAGE *pisp_img);
int32_t ispc_main_driver_mem_get_completed(uint32_t kind, ISPC_IMAGE *pisp_img);
uint64_t ispc_main_driver_get_isp_version();
uint64_t ispc_main_driver_get_isp_sdk_version();
int32_t ispc_main_driver_get_resolution(int32_t* pwidth, int32_t* pheight);
int32_t ispc_main_driver_set_resolution(int32_t width, int32_t height);
int32_t ispc_main_driver_capture_start();
int32_t ispc_main_driver_capture_stop();
int32_t ispc_main_driver_get_main_step();
int32_t ispc_main_driver_module_enable(uint8_t isp_idx, uint16_t module_id, uint8_t enable);
uint8_t ispc_main_driver_is_module_enable(uint8_t isp_idx, uint16_t module_id);
int32_t ispc_main_driver_module_update(uint8_t isp_idx, uint16_t module_id, uint8_t update);
uint8_t ispc_main_driver_is_module_update(uint8_t isp_idx, uint16_t module_id);
int32_t ispc_main_driver_control_enable(uint8_t isp_idx, uint16_t control_id, uint8_t enable);
uint8_t ispc_main_driver_is_control_enable(uint8_t isp_idx, uint16_t control_id);
int32_t ispc_main_driver_get_exposure_and_gain(uint32_t *pexposure, double *pgain);
int32_t ispc_main_driver_set_exposure_and_gain(uint32_t exposure, double gain);
int32_t ispc_main_driver_get_awb_gain(double *prgain, double *pbgain, double *pdgain);
int32_t ispc_main_driver_set_awb_gain(double rgain, double bgain, double dgain);
uint16_t ispc_main_driver_read_sensor_reg(uint16_t reg_addr);
int32_t ispc_main_driver_write_sensor_reg(uint16_t reg_addr, uint16_t reg_value);
int32_t ispc_main_driver_get_iq_param(uint32_t mod_ctl_id, void *pparam_buf, uint16_t *pparam_size);
int32_t ispc_main_driver_set_iq_param(uint32_t mod_ctl_id, void *pparam_buf, uint16_t param_size);
int32_t ispc_main_driver_restore_setting(uint8_t setting_kind);


#endif //__ISPC_MAIN_H__
