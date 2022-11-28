/**
  ******************************************************************************
  * @file sc2235dvp.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/12/2020
  * @brief SC2235 camera sensor implementation
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STARFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai StarFive</center></h2>
  */


#include "sensors/sc2235dvp.h"

#if defined(USE_CD_SYSTEM_LIB)
#include "ci/driver/stfdrv_cd_system.h"
#endif //#if defined(USE_CD_SYSTEM_LIB)
#include "sensorapi/sensorapi.h"
#include "sensorapi/sensor_phy.h"
//#include "sensors/ddk_sensor_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>


#if 0
#define V_LOG_INFO LOG_INFO
#else
#define V_LOG_INFO(strings, arg...)
#endif
#if 0
#define V_LOG_DEBUG LOG_DEBUG
#else
#define V_LOG_DEBUG(strings, arg...)
#endif


/* Sensor specific configuration */
#define SENSOR_NAME                 "SC2235_SENSOR"
#define LOG_TAG                     SENSOR_NAME
#include "stf_common/userlog.h"
#ifdef SIFIVE_ISP
#define DEV_PATH                    ("/dev/ddk_sensor")
#define EXTRA_CFG_PATH              ("/root/.ispddk/")
#endif //SIFIVE_ISP
#define SC2235_EXPO_GAIN_METHOD     (EN_EXPO_GAIN_METHOD_SAME_TIME)

#if defined(V4L2_DRIVER)
//#define SC2235_I2C_CHN              (1)     // For JH7100 VisionFive
#define SC2235_I2C_CHN              (4)     // For JH7110
#else
#define SC2235_I2C_CHN              (0)
#endif //#if defined(V4L2_DRIVER)
#define SC2235_I2C_ADDR             (0x30)      // in 7-bits
#define SC2235_WRITE_ADDR           (0x60 >> 1)
#define SC2235_READ_ADDR            (0x61 >> 1)

#define SC2235_SENSOR_VERSION	    "not-verified"

#define GET_CURRENT_FROM_SENSOR


#define ARRAY_SIZE(n)		        (sizeof(n) / sizeof(n[0]))

#define SENSOR_TUNNING		        0
#define TUNE_SLEEP(n)	            { if (SENSOR_TUNNING) sleep(n); }

/* Assert */
#define ASSERT_INITIALIZED(p)		\
{ if (!p) { LOG_ERROR("Sensor not initialised\n"); return STF_ERROR_NOT_INITIALISED; } }
#define ASSERT_MODE_RANGE(n)		\
{ if (n >= g_u8SensorModeNum) { LOG_ERROR("Invalid mode_id %d, there is only %d modes\n", n, g_u8SensorModeNum); return STF_ERROR_INVALID_ID; } }


typedef struct _ST_SC2235_CAM {
    SENSOR_FUNCS stFuncs;
    SENSOR_PHY *pstSensorPhy;
    STF_INT fdI2c;

    /* Sensor status */
    STF_BOOL8 bEnabled;         // in using or not
    STF_U8 u8ModeId;            // index of current mode
    STF_U8 u8Flipping;
    STF_U32 u32Exposure;
    STF_U32 u32ExpoRegVal;		// current exposure register value
    STF_DOUBLE dGain;
    STF_U16 u16GainRegVal;      // current gain register value
    STF_U16 u16GainRegValLimit;
    STF_DOUBLE dCurFps;
    STF_U32 u32FrameLength;

    /* Sensor config params */
    STF_U8 u8Imager;            // 0: DVP, 1: MIPI
    STF_BOOL8 bUseSensorAecAgc;	// Use sensor AEC & AGC function
} ST_SC2235_CAM, *PST_SC2235_CAM;

/* Predefined sensor mode */
typedef struct _ST_SENSOR_MODE {
    SENSOR_MODE stMode;
    STF_U8 u8HSync;
    STF_U8 u8VSync;
    STF_U32 u32LineTime;		// in ns. for exposure
    STF_U16 *pu16Registers;
    STF_U32 u32RegNum;
} ST_SENSOR_MODE, *PST_SENSOR_MODE;

static SENSOR_INTFC g_stInterface = {
    .u8DataInterface = 0,                       // DVP.
    .u8I2cChannel = SC2235_I2C_CHN,
    .u8I2cSlaveAddrbit = 0,                     // 7 bits.
    .u8I2cRegAddrBit = 1,                       // 16 bits.
    .u8I2cRegDataBit = 1,                       // 8 bits.
    .u16I2cSlaveAddr = SC2235_I2C_ADDR,
    .s16PowerGpioId = 27,
    .u16PowerGpioInitLevel = 0,
    .s16ResetGpioId = -1,
    .u16ResetGpioInitLevel = 0,
    .u16ResetIntervalTime = 0,
    .u8SensorPolarity = 2, // 0:EN_DVP_SYNC_POL_H_NEG_V_NEG, 1: EN_DVP_SYNC_POL_H_NEG_V_POS, 2:EN_DVP_SYNC_POL_H_POS_V_NEG, 3:EN_DVP_SYNC_POL_H_POS_V_POS.
};


// from SmartSens data sheet
#define SENSOR_BAYER_FORMAT		    MOSAIC_BGGR
#if 1
#define SENSOR_MAX_GAIN				240.25      // This value is for formula.
#else
#define SENSOR_MAX_GAIN			    248.0       // This value is for table mapping.
#endif

#define SENSOR_MAX_GAIN_REG_VAL		0xf40
#define SENSOR_MIN_GAIN_REG_VAL		0x82

#define SENSOR_AECAGC_CTRL_REG_ADDR 0x3e03
#if 0
#define SENSOR_GAIN_REG_ADDR_H	    0x3e07
#define SENSOR_GAIN_REG_ADDR_M	    0x3e08
#define SENSOR_GAIN_REG_ADDR_L		0x3e09
#else
#define SENSOR_GAIN_REG_ADDR_H		0x3e08
#define SENSOR_GAIN_REG_ADDR_L		0x3e09
#endif
#define SENSOR_EXPOSURE_REG_ADDR_H	0x3e01
#define SENSOR_EXPOSURE_REG_ADDR_L	0x3e02

#define SENSOR_MAX_FRAME_TIME       200000		// in us. used for delay. consider the lowest fps
#define SENSOR_FRAME_LENGTH_H		0x320e
#define SENSOR_FRAME_LENGTH_L		0x320f

#define SENSOR_EXPOSURE_DEFAULT		31111
#define DEFAULT_USE_AEC_AGC			0


static STF_U16 g_stRegisters_1080p_30fps[] = {
    0x0103,0x01,

    0x0100,0x00,

    0x3039,0x80,
    0x3621,0x28,

    0x3309,0x60,
    0x331f,0x4d,
    0x3321,0x4f,
    0x33b5,0x10,

    0x3303,0x20,
    0x331e,0x0d,
    0x3320,0x0f,

    0x3622,0x02,
    0x3633,0x42,
    0x3634,0x42,

    0x3306,0x66,
    0x330b,0xd1,

    0x3301,0x0e,

    0x320c,0x08,
    0x320d,0x98,

    0x3364,0x05,// [2] 1: write at sampling ending

    0x363c,0x28, //bypass nvdd
    0x363b,0x0a, //HVDD
    0x3635,0xa0, //TXVDD

    0x4500,0x59,
    0x3d08,0x00,
    0x3908,0x11,

    0x363c,0x08,

    0x3e03,0x03,
    0x3e01,0x46,


    //0x3e08,0x7f,
    //0x3e09,0x1f,
    //0x5000,0x00,
    //0x3908,0x31,

    //0703
    0x3381,0x0a,
    0x3348,0x09,
    0x3349,0x50,
    0x334a,0x02,
    0x334b,0x60,

    0x3380,0x04,
    0x3340,0x06,
    0x3341,0x50,
    0x3342,0x02,
    0x3343,0x60,

    //0707

    0x3632,0x88, //anti sm
    0x3309,0xa0,
    0x331f,0x8d,
    0x3321,0x8f,


    0x335e,0x01,  //ana dithering
    0x335f,0x03,
    0x337c,0x04,
    0x337d,0x06,
    0x33a0,0x05,
    0x3301,0x05,

    //atuo logic
    /*  ///raojing disable
    0x3670,0x08 , //[3]:3633 logic ctrl  real value in 3682
    0x367e,0x07,  //gain0
    0x367f,0x0f,  //gain1
    0x3677,0x2f,  //<gain0
    0x3678,0x22,  //gain0 - gain1
    0x3679,0x43,  //>gain1
    */
    0x337f,0x03, //new auto precharge  330e in 3372   [7:6] 11: close div_rst 00:open div_rst
    0x3368,0x02,
    0x3369,0x00,
    0x336a,0x00,
    0x336b,0x00,
    0x3367,0x08,
    0x330e,0x30,

    0x3366,0x7c, // div_rst gap

    0x3635,0xc1,
    0x363b,0x09,
    0x363c,0x07,

    0x391e,0x00,

    0x3637,0x14, //fullwell 7K

    0x3306,0x54,
    0x330b,0xd8,
    0x366e,0x08,  // ofs auto en [3]
    0x366f,0x2f,  // ofs+finegain  real ofs in 0x3687[4:0]

    0x3631,0x84,
    0x3630,0x48,
    0x3622,0x06,


    //ramp by sc
    0x3638,0x1f,
    0x3625,0x02,
    0x3636,0x24,

    //0714
    0x3348,0x08,
    0x3e03,0x0b,

    //7.17 fpn
    0x3342,0x03,
    0x3343,0xa0,
    0x334a,0x03,
    0x334b,0xa0,

    //0718
    0x3343,0xb0,
    0x334b,0xb0,

    //0720

    //digital ctrl
    0x3802,0x01,
    0x3235,0x04,
    0x3236,0x63, // vts-2

    //fpn
    0x3343,0xd0,
    0x334b,0xd0,
    0x3348,0x07,
    0x3349,0x80,

    //0724
    0x391b,0x4d,

    0x3342,0x04,
    0x3343,0x20,
    0x334a,0x04,
    0x334b,0x20,

    //0804
    0x3222,0x29,
    0x3901,0x02,

    //0808

    // auto blc
    0x3900,0xD5,    // Bit[0]: blc_enable
    0x3902,0x45,    // Bit[6]: blc_auto_en

    // blc target
#if 0
    0x3907,0x00,
    0x3908,0x11,
#elif 0
    0x3907,0x00,
    0x3908,0xd8,
#else
    0x3907,0x00,
    0x3908,0x00,
#endif

    // auto dpc
    //0x5000,0x06,    // Bit[2]: white dead pixel cancel enable, Bit[1]: black dead pixel cancel enable
    0x5000,0x00,    // Bit[2]: white dead pixel cancel enable, Bit[1]: black dead pixel cancel enable

    //digital ctrl
    0x3f00,0x07,  // bit[2] = 1
    0x3f04,0x08,
    0x3f05,0x74,  // hts - 0x24

    //0809
    0x330b,0xc8,

    //0817
    0x3306,0x4a,
    0x330b,0xca,
    0x3639,0x09,

    //manual DPC
    0x5780,0xff,
    0x5781,0x04,
    0x5785,0x18,

    //0822
    0x3039,0x35,    //fps
    0x303a,0x2e,
    0x3034,0x05,
    0x3035,0x2a,

    0x320c,0x08,
    0x320d,0xca,
    0x320e,0x04,
    0x320f,0xb0,

    0x3f04,0x08,
    0x3f05,0xa6, // hts - 0x24

    0x3235,0x04,
    0x3236,0xae, // vts-2

    //0825
    0x3313,0x05,
    0x3678,0x42,

    //for AE control per frame
    0x3670,0x00,
    0x3633,0x42,

    0x3802,0x00,

    //20180126
    0x3677,0x3f,
    0x3306,0x44,//20180126[3c,4a]
    0x330b,0xca,//20180126[c2,d3]

    //20180202
    0x3237,0x08,
    0x3238,0x9a, //hts-0x30

    //20180417
    0x3640,0x01,
    0x3641,0x02,


    0x3301,0x12,//[8,15]20180126
    0x3631,0x84,
    0x366f,0x2f,
    0x3622,0xc6,//20180117

    0x3e03,0x03,    // Bit[3]: AGC table mapping method, Bit[1]: AGC manual, BIt[0]: AEC manual

    /*[gain<2]
    0x3301,0x12,//[8,15]20180126
    0x3631,0x84,
    0x366f,0x2f,
    0x3622,0xc6,//20180117

    //[8>gain>=2]
    0x3301,0x15,//[11,16] 20180126
    0x3631,0x88,
    0x366f,0x2f,
    0x3622,0xc6,//20180117

    //[15.5>gain>=8]
    0x3301,0x1e,//[15,20]20180126
    0x3631,0x88,
    0x366f,0x2f,
    0x3622,0xc6,//20180117

    //[gain>=15.5]
    0x3301,0xff,
    0x3631,0x88,
    0x366f,0x3a,
    0x3622,0x06,//20180117
    //*/

    //0x0100,0x01,
    0x0100,0x00
};

static ST_SENSOR_MODE g_stSensorModes[] = {
    // This setting is for sensor clock 24MHz.
    // sensor clock = 24MHz, dvp pclk=81MHz, hsync=36KHz hblanking=4.08us, vsync=30Hz
#if 1
    // Max Frame Length = 3600 = 100ms / 27777.777ns
    {{10, 1920, 1080, 30, 12.345679, 2250, 3600, SENSOR_FLIP_BOTH, 29, 200000, 0, 4}, 1, 1, 27778,
        g_stRegisters_1080p_30fps, ARRAY_SIZE(g_stRegisters_1080p_30fps)},
#elif 0
    // Max Frame Length = 7200 = 100ms / 27777.777ns
    {{10, 1920, 1080, 30, 12.345679, 2250, 7200, SENSOR_FLIP_BOTH, 29, 200000, 0, 4}, 1, 1, 27778,
        g_stRegisters_1080p_30fps, ARRAY_SIZE(g_stRegisters_1080p_30fps)},
#endif
};
static STF_U8 g_u8SensorModeNum = 0;


// used before implementation therefore declared here
static STF_RESULT Sensor_ConfigRegister(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    STF_INT nI2c,
    STF_U8 u8ModeId
    );
static STF_RESULT Sensor_AecAgcCtrl(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    int nI2c,
    STF_BOOL8 bEnable
    );
static STF_RESULT Sensor_EnableCtrl(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    int nI2c,
    STF_BOOL8 bEnable
    );
static STF_BOOL8 IsGainInRange(
    STF_DOUBLE dGain,
    int bit
    );
static STF_U32 Sensor_ComputeGains(
    STF_DOUBLE dGain
    );
static STF_VOID Sensor_WriteGain(
    ST_SC2235_CAM *pstSensorCam,
    STF_U32 u32ValGain
    );
static STF_U32 Sensor_ComputeExposure(
    ST_SC2235_CAM *pstSensorCam,
    STF_U32 u32ExpoTime
    );
static void Sensor_WriteExposure(
    ST_SC2235_CAM *pstSensorCam,
    STF_U32 u32ExposureLines
    );
static STF_RESULT Sensor_FlipImage(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    int nI2c,
    STF_U8 u8HFlip,
    STF_U8 u8VFlip
    );
static STF_RESULT sSetFlipMirror(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8Flag
    );
static STF_RESULT sGetFixedFPS(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16FixedFps
    );
static STF_RESULT sSetFPS(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dFps
    );
static STF_RESULT sSetExposureAndGain(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    );


#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
static STF_RESULT Sensor_I2cRead(
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U8 *pu8Data
    )
{
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[2];
    STF_U8 u8Addr[2];

    STF_ASSERT(pu8Data);  // null pointer forbidden

    u8Addr[0] = (u16Reg >> 8) & 0xFF;
    u8Addr[1] = u16Reg & 0xFF;

    stMessages[0].addr = SC2235_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len = 2;
    stMessages[0].buf = u8Addr;

    stMessages[1].addr = SC2235_I2C_ADDR;
    stMessages[1].flags = I2C_M_RD;
    stMessages[1].len = 1;
    stMessages[1].buf = pu8Data;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 2;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to read reg 0x%04X.\n", u16Reg);
        return STF_ERROR_FATAL;
    }
    //printf("read  <-[0x%04x] = 0x%02x\n", u16Reg,  *pu8Data);

    return STF_SUCCESS;
}
#else
static STF_RESULT Sensor_I2cRead(
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U8 *pu8Data
    )
{
    ST_CAM_REG stCamReg;
    STF_RESULT Ret = STF_SUCCESS;

    STF_ASSERT(pCIConnection);  // null pointer forbidden
    STF_ASSERT(pu8Data);        // null pointer forbidden

    stCamReg.u8Idx = u8IspIdx;
    stCamReg.u16RegAddr = u16Reg;
    Ret = STFDRV_CD_SYS_CAM_OBJ_I2cRead(
#if defined(VIRTUAL_IO_MAPPING)
        pCIConnection,
#endif //VIRTUAL_IO_MAPPING
        &stCamReg
        );
    if (Ret) {
        LOG_ERROR("Unable to read reg 0x%04X.\n", stCamReg.u16RegAddr);
        return STF_ERROR_FATAL;
    }

    *pu8Data = (STF_U8)stCamReg.u16RegValue;

    return STF_SUCCESS;
}
#endif //USE_LINUX_SYSTEM_STARTAND_I2C

#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
static STF_RESULT Sensor_I2cWrite(
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U8 u8Data
    )
{
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[1];
    STF_U8 u8Buf[3] = { 0 };

    u8Buf[0] = (u16Reg >> 8) & 0xFF;
    u8Buf[1] = u16Reg & 0xFF;
    u8Buf[2] = u8Data;

    stMessages[0].addr  = SC2235_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len   = 3;
    stMessages[0].buf   = u8Buf;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 1;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to write reg 0x%04x with data 0x%02x.\n",
            u16Reg, u8Data);
        return STF_ERROR_FATAL;
    }
    //printf("write ->[0x%04x] = 0x%02x\n", u16Reg, u8Data);

    return STF_SUCCESS;
}
#else
static STF_RESULT Sensor_I2cWrite(
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U8 u8Data
    )
{
    ST_CAM_REG stCamReg;
    STF_RESULT Ret = STF_SUCCESS;

    stCamReg.u8Idx = u8IspIdx;
    stCamReg.u16RegAddr = u16Reg;
    stCamReg.u16RegValue = u8Data;
    Ret = STFDRV_CD_SYS_CAM_OBJ_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING)
        pCIConnection,
#endif //VIRTUAL_IO_MAPPING
        &stCamReg
        );

    if (Ret) {
        LOG_ERROR("Unable to write reg 0x%04x with data 0x%02x.\n",
            stCamReg.u16RegAddr, stCamReg.u16RegValue);
        return STF_ERROR_FATAL;
    }

    return STF_SUCCESS;
}
#endif //USE_LINUX_SYSTEM_STARTAND_I2C

static STF_RESULT Sensor_ConfigRegister(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    STF_INT nI2c,
    STF_U8 u8ModeId
    )
{
#if defined(V4L2_DRIVER)
#else
    STF_U16 *pu16Registers;
    STF_U32 u32RegNum;
    STF_U32 u32Idx;
    STF_RESULT Ret;

    ASSERT_MODE_RANGE(u8ModeId);

    pu16Registers = g_stSensorModes[u8ModeId].pu16Registers;
    u32RegNum = g_stSensorModes[u8ModeId].u32RegNum;

    V_LOG_INFO("**SC2235 DVP sensor mode_id=%d reg_num=%d**\n",
        u8ModeId, u32RegNum);

    for (u32Idx = 0; u32Idx < u32RegNum; u32Idx += 2) {
        Ret = Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            nI2c,
            pu16Registers[u32Idx],
            pu16Registers[u32Idx + 1]
            );
        if (Ret != STF_SUCCESS) {
            LOG_ERROR("sc2235 dvp sensor Set mode %d failed, Ret = %d(0x%08X)\n",
                u8ModeId,
                Ret,
                Ret
                );
            return Ret;
        }
    }
#endif //#if defined(V4L2_DRIVER)

    return STF_SUCCESS;
}

static STF_RESULT Sensor_AecAgcCtrl(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    int nI2c,
    STF_BOOL8 bEnable
    )
{
    STF_U8 u8RegValue = 0;
    STF_RESULT Ret = STF_SUCCESS;

    if (STF_SUCCESS != (Ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pCIConnection,
        u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        nI2c,
        SENSOR_AECAGC_CTRL_REG_ADDR,
        &u8RegValue))) {
        LOG_ERROR("sc2235 dvp sensor read 0x3E03 register failed, "\
            "Ret = %d(0x%08X)\n",
            Ret,
            Ret
            );
    } else {
        if (bEnable) {
            u8RegValue &= (~0x03);
            V_LOG_INFO("sc2235 dvp sensor AEC and AGC control by Sensor!\n");
        } else {
            u8RegValue |= 0x03;
            V_LOG_INFO("sc2235 dvp sensor AEC and AGC control by ISP!\n");
        }
        Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            nI2c,
            SENSOR_AECAGC_CTRL_REG_ADDR,
            u8RegValue
            );
    }

    return STF_SUCCESS;
}

static STF_S8 sGetModeIdx(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Width,
    STF_U16 u16Height
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_U8 u8Idx;
    STF_S8 s8ModeIdx= -1;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    V_LOG_INFO("SC2235 DVP sensor u16Width = %d, u16Height = %d\n",
        u16Width, u16Height);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    for (u8Idx = 0; u8Idx < g_u8SensorModeNum; u8Idx++) {
        if ((u16Width == g_stSensorModes[u8Idx].stMode.u16Width)
            && (u16Height == g_stSensorModes[u8Idx].stMode.u16Height)) {
            s8ModeIdx = u8Idx;
            break;
        }
    }

    return s8ModeIdx;
}

static STF_RESULT sGetMode(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    V_LOG_INFO("SC2235 DVP sensor u8ModeIdx = %d\n", u8ModeIdx);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pstModes);
    ASSERT_MODE_RANGE(u8ModeIdx);

    STF_MEMCPY(
        pstModes,
        &g_stSensorModes[u8ModeIdx].stMode,
        sizeof(SENSOR_MODE)
        );

    return STF_SUCCESS;
}

static STF_RESULT sGetState(
    SENSOR_HANDLE *pstHandle,
    SENSOR_STATUS *pstStatus
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    //LOG_INFO("**SC2235 DVP sensor sGetState** \n");
    TUNE_SLEEP(1);
    STF_ASSERT(pstStatus);

    if (!pstSensorCam->pstSensorPhy) {
        LOG_WARNING("sensor not initialised\n");
        pstStatus->enState = SENSOR_STATE_UNINITIALISED;
        pstStatus->u8CurrentMode = 0;
    } else {
        pstStatus->enState =
            (pstSensorCam->bEnabled ? SENSOR_STATE_RUNNING : SENSOR_STATE_IDLE);
        pstStatus->u8CurrentMode = pstSensorCam->u8ModeId;
        pstStatus->u8Flipping = pstSensorCam->u8Flipping;
        pstStatus->dCurrentFps = pstSensorCam->dCurFps;
    }

    return STF_SUCCESS;
}

static STF_RESULT sGetInterfaceInfo(
    SENSOR_HANDLE *pstHandle,
    SENSOR_INTFC *pstInterface
    )
{

    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    //LOG_INFO("**SC2235 DVP sensor sGetInterfaceInfo** \n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pstInterface);

    STF_MEMCPY(
        pstInterface,
        &g_stInterface,
        sizeof(SENSOR_INTFC)
        );

    return STF_SUCCESS;
}

static STF_RESULT sSetMode(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8ModeIdx,
    STF_U8 u8Flipping
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_RESULT Ret = STF_SUCCESS;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    V_LOG_INFO("**SC2235 DVP sensor mode=%d, flipping=0x%02X**\n",
        u8ModeIdx, u8Flipping);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    ASSERT_MODE_RANGE(u8ModeIdx);

    if (u8Flipping !=
        (u8Flipping & g_stSensorModes[u8ModeIdx].stMode.u8SupportFlipping)) {
        LOG_ERROR("Sensor mode %d does not support selected flipping 0x%x "\
            "(supports 0x%x)\n",
            u8ModeIdx,
            u8Flipping,
            g_stSensorModes[u8ModeIdx].stMode.u8SupportFlipping
            );
        return STF_ERROR_NOT_SUPPORTED;
    }

    /* Config registers */
    Ret = Sensor_ConfigRegister(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8ModeIdx
        );
    if (Ret) {
        goto sSetMode_failed;
    }
    usleep(2000);

    /* Config sensor AEC/AGC ctrl */
    Sensor_AecAgcCtrl(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        pstSensorCam->bUseSensorAecAgc
        );

    /* Config flipping */
    Ret = Sensor_FlipImage(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8Flipping & SENSOR_FLIP_HORIZONTAL,
        u8Flipping & SENSOR_FLIP_VERTICAL
        );
    if (Ret) {
        goto sSetMode_failed;
    }

    /* Init sensor status */
    pstSensorCam->bEnabled = STF_FALSE;
    pstSensorCam->u8ModeId = u8ModeIdx;
    pstSensorCam->u8Flipping = u8Flipping;
    pstSensorCam->u32Exposure = SENSOR_EXPOSURE_DEFAULT;
    pstSensorCam->dGain = 1;
    pstSensorCam->u16GainRegVal = 0;
    pstSensorCam->dCurFps = g_stSensorModes[u8ModeIdx].stMode.dFrameRate;

    /* Init ISP gasket params */
    pstSensorCam->pstSensorPhy->psGasket->u8Gasket = pstSensorCam->u8Imager;
    pstSensorCam->pstSensorPhy->psGasket->bParallel = STF_TRUE;
    pstSensorCam->pstSensorPhy->psGasket->uiWidth =
        g_stSensorModes[u8ModeIdx].stMode.u16Width - 1;
    pstSensorCam->pstSensorPhy->psGasket->uiHeight =
        g_stSensorModes[u8ModeIdx].stMode.u16Height - 1;
    pstSensorCam->pstSensorPhy->psGasket->bVSync =
        g_stSensorModes[u8ModeIdx].u8VSync;
    pstSensorCam->pstSensorPhy->psGasket->bHSync =
        g_stSensorModes[u8ModeIdx].u8HSync;
    pstSensorCam->pstSensorPhy->psGasket->u8ParallelBitdepth =
        g_stSensorModes[u8ModeIdx].stMode.u8BitDepth;
    V_LOG_DEBUG("gasket=%d, width=%d, height=%d, vsync=%d, hsync=%d, "\
        "bitdepth=%d, bUseSensorAecAgc=%d\n",
        pstSensorCam->pstSensorPhy->psGasket->u8Gasket,
        pstSensorCam->pstSensorPhy->psGasket->uiWidth,
        pstSensorCam->pstSensorPhy->psGasket->uiHeight,
        pstSensorCam->pstSensorPhy->psGasket->bVSync,
        pstSensorCam->pstSensorPhy->psGasket->bHSync,
        pstSensorCam->pstSensorPhy->psGasket->u8ParallelBitdepth,
        pstSensorCam->bUseSensorAecAgc
        );
    usleep(1000);

    return STF_SUCCESS;

sSetMode_failed:
    LOG_ERROR("Set mode %d failed, Ret = %d(0x%08X)\n", u8ModeIdx, Ret, Ret);

    return Ret;
}

static STF_RESULT Sensor_EnableCtrl(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    int nI2c,
    STF_BOOL8 bEnable
    )
{

    /* Sensor specific operation */
    if (bEnable) {
        Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            nI2c,
            0x0100,
            0x01
            );
        usleep(2000);
    } else {
        Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            nI2c,
            0x0100,
            0x00
            );
        usleep(2000);
    }

    return STF_SUCCESS;
}

static STF_RESULT sEnable(
    SENSOR_HANDLE *pstHandle
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    V_LOG_INFO("<===========\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    Ret = SensorPhyCtrl(
        pstSensorCam->pstSensorPhy,
        STF_TRUE,
        0,
        0
        );
    if (Ret) {
        LOG_ERROR("SensorPhyCtrl failed\n");
        return Ret;
    }

#if defined(V4L2_DRIVER)
#else
    Sensor_EnableCtrl(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        STF_ENABLE
        );
    //usleep(SENSOR_MAX_FRAME_TIME);
#endif //#if defined(V4L2_DRIVER)
    pstSensorCam->bEnabled = STF_TRUE;
    V_LOG_INFO("Sensor enabled!\n");

    return STF_SUCCESS;
}

static STF_RESULT sDisable(
    SENSOR_HANDLE *pstHandle
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    V_LOG_INFO("===========>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    if (pstSensorCam->bEnabled) {
        //LOG_INFO("Disabling SC2235 DVP camera\n");
        pstSensorCam->bEnabled = STF_FALSE;

#if defined(V4L2_DRIVER)
#else
        Sensor_EnableCtrl(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            STF_DISABLE
            );
#endif //#if defined(V4L2_DRIVER)
        // delay of a frame period to ensure sensor has stopped
        // flFrameRate in Hz, change to MHz to have micro seconds
        usleep(SENSOR_MAX_FRAME_TIME);
        SensorPhyCtrl(pstSensorCam->pstSensorPhy, STF_FALSE, 0, 0);
        V_LOG_INFO("Sensor disabled!\n");
    }

    return STF_SUCCESS;
}

static STF_RESULT sDestroy(
    SENSOR_HANDLE *pstHandle
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    LOG_DEBUG("Destroying SC2235 DVP camera\n");
    /* remember to free anything that might have been allocated dynamically
     * (like extended params...)*/
    if (pstSensorCam->bEnabled) {
        sDisable(pstHandle);
    }

    SensorPhyDeinit(pstSensorCam->pstSensorPhy);

    close(pstSensorCam->fdI2c);
    STF_FREE(pstSensorCam);

    return STF_SUCCESS;
}

static STF_RESULT sGetInfo(
    SENSOR_HANDLE *pstHandle,
    SENSOR_INFO *pstInfo
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    //V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pstInfo);

    STF_ASSERT(strlen(SC2235DVP_SENSOR_INFO_NAME) < SENSOR_INFO_NAME_MAX);
    STF_ASSERT(strlen(SC2235_SENSOR_VERSION) < SENSOR_INFO_VERSION_MAX);

    pstInfo->enBayerOriginal = SENSOR_BAYER_FORMAT;
    pstInfo->enBayerEnabled = pstInfo->enBayerOriginal;
    sprintf(pstInfo->pszSensorName, SC2235DVP_SENSOR_INFO_NAME);
    sprintf(pstInfo->pszSensorVersion, SC2235_SENSOR_VERSION);
    pstInfo->dNumber = 1.2;
    pstInfo->u16FocalLength = 30;
    pstInfo->u32WellDepth = 7500;
    // bitdepth is a mode information
    pstInfo->dReadNoise = 5.0;
    pstInfo->u8Imager = pstSensorCam->u8Imager;
    pstInfo->bBackFacing = STF_TRUE;
    // other information should be filled by sGetInfo()
    pstInfo->u32ModeCount = g_u8SensorModeNum;
    pstInfo->enExposureGainMethod = SC2235_EXPO_GAIN_METHOD;
    //LOG_DEBUG("Provided BayerOriginal = %d\n", pstInfo->enBayerOriginal);

    return STF_SUCCESS;
}

static STF_RESULT sGetRegister(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16RegAddr,
    STF_U16 *pu16RegVal,
    STF_U8 u8Context
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_U8 u8RegValue = 0;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu16RegVal);

    Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u16RegAddr,
        &u8RegValue
        );
    *pu16RegVal = u8RegValue;

    return STF_SUCCESS;
}

static STF_RESULT sSetRegister(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16RegAddr,
    STF_U16 u16RegVal,
    STF_U8 u8Context
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u16RegAddr,
        u16RegVal
        );

    return STF_SUCCESS;
}

static STF_BOOL8 IsGainInRange(
    STF_DOUBLE dGain,
    int bit
    )
{

    if (1 > bit) {
        LOG_ERROR("Invalid param bit %d\n", bit);
        return STF_FALSE;
    }

    if ((1 << bit) < dGain && (1 << (bit - 1)) <= dGain) {
        return STF_TRUE;
    } else {
        return STF_FALSE;
    }
}

/*
   typedef struct __BASE_GAIN_TABLE

   {
   0x10, 1,
   0x11, 1.0625,
   0x12, 1.125,
   0x13, 1.1875,
   0x14, 1.25,
   0x15, 1.3125,
   0x16, 1.375,
   0x17, 1.4375,
   0x18, 1.5,
   0x19, 1.5625,
   0x1a, 1.625,
   0x1b, 1.6875,
   0x1c, 1.75,
   0x1d, 1.8125,
   0x1e, 1.875,
   0x1f, 1.9375,
   };
   */
static STF_U32 Sensor_ComputeGains(
    STF_DOUBLE dGain
    )
{
    STF_U32 u32GainInteger;
    STF_U32 u32ValInteger;
    STF_U32 u32ValDecimal;
    STF_U32 u32ValGain;
    STF_DOUBLE dGainDecimal;

    if (dGain > SENSOR_MAX_GAIN) {
        dGain = SENSOR_MAX_GAIN;
    }

#if 1
    u32GainInteger = floor(dGain);
    dGainDecimal = dGain - u32GainInteger;
    //u32ValDecimal = round(gain_decimal * 16);
    u32ValDecimal = floor(dGainDecimal * 16);
    u32ValInteger = u32GainInteger * 16;
    u32ValGain = u32ValInteger+u32ValDecimal;
    //printf("u32ValGain=%x\n",u32ValGain);
#else
    u32ValGain = floor(dGain * 16)
#endif

    return u32ValGain;
}

static STF_VOID Sensor_WriteGain(
    ST_SC2235_CAM *pstSensorCam,
    STF_U32 u32ValGain
    )
{

    if (u32ValGain > pstSensorCam->u16GainRegValLimit) {
        u32ValGain = pstSensorCam->u16GainRegValLimit;
    }

    if (u32ValGain == pstSensorCam->u16GainRegVal) {
        return;
    }

    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_GAIN_REG_ADDR_H,
        (u32ValGain >> 8) & 0xFF
        );
    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_GAIN_REG_ADDR_L,
        u32ValGain & 0xFF
        );
    pstSensorCam->u16GainRegVal = u32ValGain;
}

static STF_RESULT sGetGainRange(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE *pdMin,
    STF_DOUBLE *pdMax,
    STF_U8 *pu8Contexts
    )
{

    //V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    STF_ASSERT(pdMin);
    STF_ASSERT(pdMax);
    STF_ASSERT(pu8Contexts);

    *pdMin = 1.0;
    *pdMax = SENSOR_MAX_GAIN;
    *pu8Contexts = 1;

    return STF_SUCCESS;
}

static STF_RESULT sGetCurrentGain(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE *pdCurrentGain,
    STF_U8 u8Context
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
#if defined(GET_CURRENT_FROM_SENSOR)
    STF_U8 u8GainHigh, u8GainLow;
#endif //GET_CURRENT_FROM_SENSOR

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    //V_LOG_DEBUG(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pdCurrentGain);

#if defined(GET_CURRENT_FROM_SENSOR)
    Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_GAIN_REG_ADDR_H,
        &u8GainHigh
        );
    u8GainHigh &= 0x0f;
    Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_GAIN_REG_ADDR_L,
        &u8GainLow
        );
    *pdCurrentGain = (((STF_U16)u8GainHigh << 8) + u8GainLow) / 16.0;
#else
    *pdCurrentGain = pstSensorCam->dGain;
#endif //GET_CURRENT_FROM_SENSOR

    return STF_SUCCESS;
}

static STF_RESULT sSetGain(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_U32 u32ValGain;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    if (SENSOR_MAX_GAIN < dGain) {
        dGain = SENSOR_MAX_GAIN;
    } else if (1 > dGain) {
        dGain = 1;
    }

    pstSensorCam->dGain = dGain;

    if (pstSensorCam->bUseSensorAecAgc) {
        V_LOG_DEBUG("Using sensor AEC & AGC function.\n");
        return STF_SUCCESS;
    }

    u32ValGain = Sensor_ComputeGains(dGain);
    Sensor_WriteGain(pstSensorCam, u32ValGain);
    V_LOG_DEBUG("dGain=%lf, register value=0x%02X, actual gain=%lf\n",
        dGain, u32ValGain, ((STF_DOUBLE)u32ValGain) / 16.0);

    return STF_SUCCESS;
}

static STF_U32 Sensor_ComputeExposure(
    ST_SC2235_CAM *pstSensorCam,
    STF_U32 u32ExpoTime
    )
{
    STF_U32 u32ExposureLines;

#if 1
    u32ExposureLines = u32ExpoTime * 1000
        / g_stSensorModes[pstSensorCam->u8ModeId].u32LineTime;
#else
    u32ExposureLines = u32ExpoTime
        / g_stSensorModes[pstSensorCam->u8ModeId].u32LineTime;
#endif
    if (1 > u32ExposureLines) {
        u32ExposureLines = 1;
#if 0
    } else if (u32ExposureLines > (pstSensorCam->u32FrameLength - 4)) {
        u32ExposureLines = pstSensorCam->u32FrameLength - 4;
#else
    } else if (u32ExposureLines > (pstSensorCam->u32FrameLength)) {
        u32ExposureLines = pstSensorCam->u32FrameLength;
#endif
    }

    return u32ExposureLines;
}

static void Sensor_WriteExposure(
    ST_SC2235_CAM *pstSensorCam,
    STF_U32 u32ExposureLines
    )
{

    if (u32ExposureLines == pstSensorCam->u32ExpoRegVal) {
        return;
    }

    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_EXPOSURE_REG_ADDR_H,
        (u32ExposureLines >> 4) & 0xFF
        );
    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_EXPOSURE_REG_ADDR_L,
        (u32ExposureLines << 4) & 0xFF
        );
    pstSensorCam->u32ExpoRegVal = u32ExposureLines;
}

static STF_RESULT sGetExposureRange(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Min,
    STF_U32 *pu32Max,
    STF_U8 *pu8Contexts
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    //V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu32Min);
    STF_ASSERT(pu32Max);
    STF_ASSERT(pu8Contexts);

    *pu32Min = g_stSensorModes[pstSensorCam->u8ModeId].stMode.u32ExposureMin;
    *pu32Max = g_stSensorModes[pstSensorCam->u8ModeId].stMode.u32ExposureMax;
    *pu8Contexts = 1;

    return STF_SUCCESS;
}

static STF_RESULT sGetExposure(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Exposure,
    STF_U8 u8Context
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
#if defined(GET_CURRENT_FROM_SENSOR)
    STF_U8 u8ExpoHigh, u8ExpoLow;
    STF_U16 u16ExpoLines;
#endif //GET_CURRENT_FROM_SENSOR

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    //V_LOG_DEBUG(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu32Exposure);

#if defined(GET_CURRENT_FROM_SENSOR)
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_EXPOSURE_REG_ADDR_H,
        &u8ExpoHigh
        );
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_EXPOSURE_REG_ADDR_L,
        &u8ExpoLow
        );
    u8ExpoLow = (u8ExpoLow >> 4) & 0xFF;
    u16ExpoLines = (u8ExpoHigh << 4) + u8ExpoLow;
  #if 1
    *pu32Exposure = u16ExpoLines
        * g_stSensorModes[pstSensorCam->u8ModeId].u32LineTime / 1000;
  #else
    *pu32Exposure = u16ExpoLines
        * g_stSensorModes[pstSensorCam->u8ModeId].u32LineTime;
  #endif
#else
    *pu32Exposure = pstSensorCam->u32Exposure;
#endif //GET_CURRENT_FROM_SENSOR

    return STF_SUCCESS;
}

static STF_RESULT sSetExposure(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_U8 u8Context
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_U32 u32ExposureLines;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    pstSensorCam->u32Exposure = u32Exposure;

    if (pstSensorCam->bUseSensorAecAgc) {
        V_LOG_DEBUG("Using sensor AEC & AGC function.\n");
        return STF_SUCCESS;
    }

    u32ExposureLines = Sensor_ComputeExposure(pstSensorCam, u32Exposure);
    Sensor_WriteExposure(pstSensorCam, u32ExposureLines);
    V_LOG_DEBUG("SetExposure. time=%d us, lines = %d\n",
        pstSensorCam->u32Exposure, u32ExposureLines);

    return STF_SUCCESS;
}

static STF_RESULT Sensor_FlipImage(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    int nI2c,
    STF_U8 u8HFlip,
    STF_U8 u8VFlip
    )
{
    STF_U8 u8Value = 0;

    V_LOG_INFO("u8HFlip=%d, u8VFlip=%d\n", u8HFlip, u8VFlip);

    if (u8HFlip) {
        u8Value |= 0x6;
    } else {
        u8Value |= 0x0;
    }

    if (u8VFlip) {
        u8Value |= 0x60;
    } else {
        u8Value |= 0x0;
    }

    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pCIConnection,
        u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        nI2c,
        0x3221,
        u8Value
        );

    return STF_SUCCESS;
}

static STF_RESULT sSetFlipMirror(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8Flag
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_U32 u32LineTime;    // in ns
    STF_U32 u32Delay;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    if (u8Flag != pstSensorCam->u8Flipping) {
        Sensor_FlipImage(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8Flag & SENSOR_FLIP_HORIZONTAL,
            u8Flag & SENSOR_FLIP_VERTICAL
            );
        pstSensorCam->u8Flipping = u8Flag;
    }

    return STF_SUCCESS;
}

static STF_RESULT sGetFixedFPS(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16FixedFps
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    //V_LOG_DEBUG(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    if (NULL != pu16FixedFps) {
        *pu16FixedFps =
            g_stSensorModes[pstSensorCam->u8ModeId].stMode.dFrameRate;
        V_LOG_DEBUG("Fixed FPS=%d\n", *pu16FixedFps);
    }

    return STF_SUCCESS;
}

static STF_RESULT sSetFPS(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dFps
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_U32 u32FixedFps;
    STF_U32 u32FixedFrameLength;
    STF_U32 u32FrameLength;
    STF_U32 u32DenoiseLength;
    STF_FLOAT fDownRatio = 0.0;
    STF_U16 u16FixedFps = 0;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);
	
    TUNE_SLEEP(1);

    u32FixedFps = g_stSensorModes[pstSensorCam->u8ModeId].stMode.dFrameRate;
    u32FixedFrameLength =
        g_stSensorModes[pstSensorCam->u8ModeId].stMode.u16VerticalTotal;

    sGetFixedFPS(pstHandle, &u16FixedFps);
    if (dFps > u16FixedFps) {
        dFps = u16FixedFps;
    } else if (dFps < 5) {
        dFps = 5;
    }
    fDownRatio = pstSensorCam->dCurFps / dFps;

    if (1 == fDownRatio && u32FixedFps == pstSensorCam->dCurFps) {
        V_LOG_INFO("fDownRatio=1, dCurFps = u32FixedFps, skip operation\n");
        return STF_SUCCESS;
    }

    if (1 > fDownRatio) {
        LOG_WARNING("Invalid fDownRatio=%d, force to 1, "\
            "fDownRatio check fps param in json file\n", fDownRatio);
        fDownRatio = 1;
    }
    if (12 < fDownRatio) {
        LOG_ERROR("Invalid fDownRatio %d, too big\n", fDownRatio);
        return STF_ERROR_INVALID_PARAMETERS;
    }

    u32FrameLength = round(u32FixedFrameLength * fDownRatio);
    pstSensorCam->u32FrameLength = u32FrameLength;
    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_FRAME_LENGTH_H,
        (u32FrameLength >> 8) & 0xFF
        );
    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        SENSOR_FRAME_LENGTH_L,
        u32FrameLength & 0xFF
        );

    pstSensorCam->dCurFps = round((STF_DOUBLE)u32FixedFps / fDownRatio);
    V_LOG_INFO("down_ratio = %lf, frame_length = %d, fixed_fps = %d, "\
        "cur_fps = %d >>>>>>>>>>>>\n",
        fDownRatio, u32FrameLength, u32FixedFps, pstSensorCam->dCurFps);

    return STF_SUCCESS;
}

static STF_RESULT sSetExposureAndGain(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_U32 u32ExposureLines;
    STF_U32 u32Gain;

    pstSensorCam = container_of(pstHandle, ST_SC2235_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    V_LOG_DEBUG("u32Exposure=%d, dGain=%lf >>>>>>>>>>>>\n",
        u32Exposure, dGain);

    //01. Exposure part
    //
    pstSensorCam->u32Exposure = u32Exposure;
    if (pstSensorCam->bUseSensorAecAgc) {
        V_LOG_DEBUG("Using sensor AEC & AGC function.\n");
        return STF_SUCCESS;
    }
    u32ExposureLines = Sensor_ComputeExposure(pstSensorCam, u32Exposure);
    //V_LOG_DEBUG("Exposure - time=%d us, lines = %d\n",
    //    pstSensorCam->u32Exposure,
    //    u32ExposureLines
    //    );

    //02. Gain part
    //
    if (SENSOR_MAX_GAIN < dGain) {
        dGain = SENSOR_MAX_GAIN;
    } else if (1 > dGain) {
        dGain = 1;
    }
    pstSensorCam->dGain = dGain;
    u32Gain = Sensor_ComputeGains(dGain);
    //V_LOG_DEBUG("dGain=%lf, register value=0x%02x, actual gain=%lf\n",
    //    dGain,
    //    u32Gain,
    //    u32Gain / 16.0
    //    );

    //03. Program register to sensor part
    //
    Sensor_WriteGain(pstSensorCam, u32Gain);
    Sensor_WriteExposure(pstSensorCam, u32ExposureLines);

#ifdef NOISE_LOGIC_ENABLE
    Sensor_NoiseLogicGain(pstSensorCam, u32Gain);
#endif

    return STF_SUCCESS;
}

static STF_RESULT sReset(
    SENSOR_HANDLE *pstHandle
    )
{
    int ret = 0;

    V_LOG_INFO("first: disable sc2235 dvp sensor.\n");
    sDisable(pstHandle);

    usleep(100000); // the delay time used for phy staus change deinit to init

    V_LOG_INFO("second: enable sc2235 dvp sensor.\n");
    sEnable(pstHandle);

    return ret;
}

STF_RESULT SC2235DVP_Create(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    SENSOR_HANDLE **ppstHandle,
    STF_U8 u8Index
    )
{
    ST_SC2235_CAM *pstSensorCam = NULL;
    STF_CHAR szI2cDevPath[NAME_MAX];
    STF_CHAR szExtraCfg[64];
    STF_CHAR szAdaptor[64];
    STF_U32 u32I2cAddr = SC2235_I2C_ADDR;
    STF_INT nChn = 0;
    STF_U8 u8Imager = 1;
    STF_RESULT Ret;

    V_LOG_INFO("<<<<<<<<<<<<\n");

#ifdef UNUSED_CODE_AND_VARIABLE
    STF_CHAR szDevName[64];
    STF_CHAR szPath[128];
    STF_INT fd = 0;

    STF_MEMSET((void *)szDevName, 0, sizeof(szDevName));
    STF_MEMSET((void *)szAdaptor, 0, sizeof(szAdaptor));
    STF_MEMSET((void *)szPath, 0, sizeof(szPath));
    sprintf(szDevName, "%s%d", DEV_PATH, u8Index);
    fd = open(szDevName, O_RDWR);
    if (0 > fd) {
        LOG_ERROR("open %s error\n", szDevName);
        return STF_ERROR_FATAL;
    }

    ioctl(fd, GETI2CADDR, &u32I2cAddr);
    ioctl(fd, GETI2CCHN, &nChn);
    ioctl(fd, GETIMAGER, &u8Imager);
    ioctl(fd, GETSENSORPATH, szPath);

    close(fd);
    printf("%s opened OK, i2c-addr=0x%x, chn = %d\n",
        szDevName, u32I2cAddr, nChn);
    sprintf(szAdaptor, "%s-%d", "i2c", nChn);
    STF_MEMSET((void *)szExtraCfg, 0, sizeof(szDevName));
    if (szPath[0] == 0) {
        sprintf(szExtraCfg, "%s%s%d-config.txt",
            EXTRA_CFG_PATH, "sensor" , u8Index);
    } else {
        strcpy(szExtraCfg, szPath);
    }
#else
    u32I2cAddr = SC2235_I2C_ADDR;
    nChn = SC2235_I2C_CHN;
    u8Imager = 1;
    V_LOG_INFO("i2c-addr=0x%x, chn = %d\n", u32I2cAddr, nChn);
    sprintf(szAdaptor, "%s-%d", "i2c", nChn);
    sprintf(szExtraCfg, "%s%s%d-config.txt", EXTRA_CFG_PATH, "sensor", u8Index);
#endif //UNUSED_CODE_AND_VARIABLE

    V_LOG_INFO("**SC2235DVP SENSOR**\n");
    TUNE_SLEEP(1);

    /* Init global variable */
    g_u8SensorModeNum = ARRAY_SIZE(g_stSensorModes);
    V_LOG_DEBUG("g_u8SensorModeNum=%d\n", g_u8SensorModeNum);

    pstSensorCam = (ST_SC2235_CAM *)STF_CALLOC(1, sizeof(ST_SC2235_CAM));
    if (!pstSensorCam)
        return STF_ERROR_MALLOC_FAILED;

    /* Init function handle */
    *ppstHandle = &pstSensorCam->stFuncs;
    pstSensorCam->stFuncs.GetModeIdx = sGetModeIdx;
    pstSensorCam->stFuncs.GetMode = sGetMode;
    pstSensorCam->stFuncs.GetState = sGetState;
    pstSensorCam->stFuncs.GetInterfaceInfo = sGetInterfaceInfo;
    pstSensorCam->stFuncs.SetMode = sSetMode;
    pstSensorCam->stFuncs.Enable = sEnable;
    pstSensorCam->stFuncs.Disable = sDisable;
    pstSensorCam->stFuncs.Destroy = sDestroy;
    pstSensorCam->stFuncs.GetInfo = sGetInfo;

    pstSensorCam->stFuncs.GetRegister = sGetRegister;
    pstSensorCam->stFuncs.SetRegister = sSetRegister;

    pstSensorCam->stFuncs.GetGainRange = sGetGainRange;
    pstSensorCam->stFuncs.GetCurrentGain = sGetCurrentGain;
    pstSensorCam->stFuncs.SetGain = sSetGain;

    pstSensorCam->stFuncs.GetExposureRange = sGetExposureRange;
    pstSensorCam->stFuncs.GetExposure = sGetExposure;
    pstSensorCam->stFuncs.SetExposure = sSetExposure;

    pstSensorCam->stFuncs.SetFlipMirror = sSetFlipMirror;

    pstSensorCam->stFuncs.GetFixedFPS = sGetFixedFPS;
    pstSensorCam->stFuncs.SetFPS = sSetFPS;

    pstSensorCam->stFuncs.SetExposureAndGain = sSetExposureAndGain;

    pstSensorCam->stFuncs.Reset = sReset;

    /* Init sensor config */
    pstSensorCam->u8Imager = u8Imager;
    pstSensorCam->bUseSensorAecAgc = DEFAULT_USE_AEC_AGC;

    /* Init sensor state */
    pstSensorCam->bEnabled = STF_FALSE;
    pstSensorCam->u8ModeId = 0;
    pstSensorCam->u8Flipping = SENSOR_FLIP_NONE;
    pstSensorCam->u32Exposure = SENSOR_EXPOSURE_DEFAULT;
    pstSensorCam->dGain = 1.0;
    pstSensorCam->u16GainRegVal = 0;
    pstSensorCam->u16GainRegValLimit = SENSOR_MAX_GAIN_REG_VAL;
    pstSensorCam->u32FrameLength =
        g_stSensorModes[pstSensorCam->u8ModeId].stMode.u16VerticalTotal;
    pstSensorCam->fdI2c = -1;

#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    /* Init i2c */
    V_LOG_INFO("u32I2cAddr = 0x%X\n", u32I2cAddr);
    Ret = FindI2cDev(
        szI2cDevPath,
        sizeof(szI2cDevPath),
        u32I2cAddr,
        szAdaptor
        );
    if (Ret) {
        LOG_ERROR("Failed to find I2C device! Ret=%d\n", Ret);
        STF_FREE(pstSensorCam);
        *ppstHandle = NULL;
        return STF_ERROR_DEVICE_NOT_FOUND;
    }
    pstSensorCam->fdI2c = open(szI2cDevPath, O_RDWR);
    if (0 > pstSensorCam->fdI2c) {
        LOG_ERROR("Failed to open I2C device: \"%s\", err = %d\n",
            szI2cDevPath, pstSensorCam->fdI2c);
        STF_FREE(pstSensorCam);
        *ppstHandle = NULL;
        return STF_ERROR_DEVICE_NOT_FOUND;
    }

#endif //USE_LINUX_SYSTEM_STARTAND_I2C
    /* Init ISP gasket phy */
    pstSensorCam->pstSensorPhy = SensorPhyInit(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pCIConnection,
        u8IspIdx
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        );
    if (!pstSensorCam->pstSensorPhy) {
        LOG_ERROR("Failed to create sensor phy!\n");
        close(pstSensorCam->fdI2c);
        STF_FREE(pstSensorCam);
        *ppstHandle = NULL;
        return STF_ERROR_DEVICE_NOT_FOUND;
    }

    return STF_SUCCESS;
}

