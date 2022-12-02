/**
  ******************************************************************************
  * @file ov4689mipi.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  09/03/2021
  * @brief OV4689 MIPI camera sensor implementation
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


#include "sensors/ov4689mipi.h"

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
#endif //1
#if 0
#define V_LOG_DEBUG LOG_DEBUG
#else
#define V_LOG_DEBUG(strings, arg...)
#endif //1


/* disabled as i2c drivers locks up if device is not present */
#define NO_DEV_CHECK
#if 0
#define ENABLE_1080P
#else
#define ENABLE_1088P
#endif //0
#define ENABLE_1920_1080_30_4lane
//#define ENABLE_1920_1088_60_4lane
//#define ENABLE_1920_1088_30_4lane
//#define ENABLE_1920_1088_60_2lane
//#define ENABLE_1920_1088_30_2lane
//#define ENABLE_1920_1088_20_2lane
//#define ENABLE_1920_1088_30_1lane
//#define ENABLE_2624_1520_30_2lane
//#define ENABLE_2688_1520_30_2lane

/* Sensor specific configuration */
#define SENSOR_NAME                 "OV4689_SENSOR"
#define LOG_TAG                     SENSOR_NAME
#include "stf_common/userlog.h"
#ifdef SIFIVE_ISP
#define DEV_PATH                    ("/dev/ddk_sensor")
#define EXTRA_CFG_PATH              ("/root/.ispddk/")
#endif //SIFIVE_ISP
#define OV4689_EXPO_GAIN_METHOD     (EN_EXPO_GAIN_METHOD_SAME_TIME)

/** @ the setup for mode does not work if enabled is not call just after
 * - temporarily do the whole of the setup in in enable function
 * - can remove that define and its checking once fixed
 */
// if defined writes all registers at enable time instead of configure
//#define DO_SETUP_IN_ENABLE

#if defined(V4L2_DRIVER)
//#define OV4689_I2C_CHN              (1)     // For JH7100 VisionFive
#define OV4689_I2C_CHN              (6)     // For JH7110
#else
#define OV4689_I2C_CHN              (1)
#endif //#if defined(V4L2_DRIVER)
#define OV4689_I2C_ADDR             (0x36)      // in 7-bits
#define OV4689_WRITE_ADDR           (0x6C >> 1)
#define OV4689_READ_ADDR            (0x6D >> 1)
#define AUTOFOCUS_WRITE_ADDR        (0x18 >> 1)
#define AUTOFOCUS_READ_ADDR         (0x19 >> 1)
#define MAX_I2C_BUF_SIZE            (256)

#define OV4689_SENSOR_VERSION       "not-verified"

#define OV4689_CHIP_VERSION         (0x4688)

// fake register value to interpret next value as delay in ms
#define DELAY_REG                   (0xFF)
// not a real register - marks the end of register array
#define STOP_REG                    (0xFE)

#define GET_CURRENT_FROM_SENSOR
#define OV4689_FLIP_MIRROR_FRIST
#define ENABLE_EXPOSURE_TIME_FRACTION
//#define USING_GAIN_TABLE
#define ENABLE_AUTO_EXTEND_VERTICAL_TOTAL
//#define ADD_USLEEP_FOR_I2C_READ


#define ARRAY_SIZE(n)		        (sizeof(n) / sizeof(n[0]))

#define SENSOR_TUNNING		        0
#define TUNE_SLEEP(n)	            { if (SENSOR_TUNNING) sleep(n); }

/* Assert */
#define ASSERT_INITIALIZED(p)		\
{ if (!p) { LOG_ERROR("Sensor not initialised\n"); return STF_ERROR_NOT_INITIALISED; } }
#define ASSERT_MODE_RANGE(n)		\
{ if (n >= g_u8SensorModeNum) { LOG_ERROR("Invalid mode_id %d, there is only %d modes\n", n, g_u8SensorModeNum); return STF_ERROR_INVALID_ID; } }


/*
 * Choice:
 * copy of the registers to apply because we want to apply them all at
 * once because some registers need 2 writes while others don't (may have
 * an effect on stability but was not showing in testing)
 * 
 * If not defined will apply values register by register and not need a copy
 * of the registers for the active mode.
 */
//#define COPY_REGS

// uncomment to write i2c commands to file
//#define CONFIG_REG_DEBUG "ov4689_write.txt"

#ifdef WIN32 // on windows we do not need to sleep to wait for the bus
static void usleep(int t)
{
    (void)t;
}
#endif //#ifdef WIN32

typedef struct _ST_OV4689_CAM {
    SENSOR_FUNCS stFuncs;
    SENSOR_PHY *pstSensorPhy;
    STF_INT fdI2c;

    /* Sensor status */
    STF_BOOL8 bEnabled;         // in using or not
    STF_U8 u8ModeId;            // index of current mode
    STF_U8 u8Flipping;

    SENSOR_MODE stSensorMode;
    /*
     * if COPY_REGS is defined:
     * copy of the registers to apply because we want to apply them all at
     * once because some registers need 2 writes while others don't
     * 
     * otherwise just a pointer to the registers
     */
    STF_U8 *pu8CurrentSensorModeReg;
    /* number of registers in the mode
     * (each registers uses 3 bytes: first 2 are address, last one is value)
     */
    STF_U32 u32Registers;
    STF_U32 u32Exposure;
    STF_U32 u32ExposureMax;
    STF_U32 u32ExposureMin;
    STF_DOUBLE dExposureMin;
    STF_DOUBLE dGain;
    STF_U16 u16CurrentFocus;
    STF_DOUBLE dCurFps;
    STF_U16 u16VerticalTotal;

    /* Sensor config params */
    STF_U8 u8Imager;            // 0: DVP, 1: MIPI
    STF_DOUBLE dRefClock;       // in MHz
    STF_U32 u32InitClk;
    STF_U32 u32FixedFps;
} ST_OV4689_CAM, *PST_OV4689_CAM;

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
    .u8DataInterface = 1,                       // MIPI.
    .u8I2cChannel = OV4689_I2C_CHN,
    .u8I2cSlaveAddrbit = 0,                     // 7 bits.
    .u8I2cRegAddrBit = 1,                       // 16 bits.
    .u8I2cRegDataBit = 1,                       // 8 bits.
    .u16I2cSlaveAddr = OV4689_I2C_ADDR,
    .s16PowerGpioId = 21,
    .u16PowerGpioInitLevel = 0,
    .s16ResetGpioId = 20,
    .u16ResetGpioInitLevel = 1,
    .u16ResetIntervalTime = 200,
    .u8SensorPolarity = 0, // 0:EN_DVP_SYNC_POL_H_NEG_V_NEG, 1: EN_DVP_SYNC_POL_H_NEG_V_POS, 2:EN_DVP_SYNC_POL_H_POS_V_NEG, 3:EN_DVP_SYNC_POL_H_POS_V_POS.
};


// from OmniVision data sheet
#define SENSOR_BAYER_FORMAT		    MOSAIC_BGGR

#define SPLIT0(R)                   ((R >> 8) & 0xFF)
#define SPLIT1(R)                   (R & 0xFF)
#define IS_REG(arr, off, name) \
    ((SPLIT0(name) == arr[off+0]) && (SPLIT1(name) == arr[off+1]))

#define REG_FORMAT1                 0x3820 // used for flipping
#define REG_FORMAT2                 0x3821 // used for flipping

#define REG_H_CROP_START_0          0x3800 // bits [4:0] address [12:8]
#define REG_H_CROP_START_1          0x3801 // bits [7:0] address [7:0]
#define REG_V_CROP_START_0          0x3802 // bits [3:0] address [11:8]
#define REG_V_CROP_START_1          0x3803 // bits [7:0] address [7:0]
#define REG_H_CROP_END_0            0x3804 // bits [4:0] address [12:8]
#define REG_H_CROP_END_1            0x3805 // bits [7:0] address [7:0]
#define REG_V_CROP_END_0            0x3806 // bits [3:0] address [11:8]
#define REG_V_CROP_END_1            0x3807 // bits [7:0] address [7:0]

#define REG_H_OUTPUT_SIZE_0         0x3808 // bits [4:0] size [12:8]
#define REG_H_OUTPUT_SIZE_1         0x3809 // bits [7:0] size [7:0]
#define REG_V_OUTPUT_SIZE_0         0x380A // bits [4:0] size [12:8]
#define REG_V_OUTPUT_SIZE_1         0x380B // bits [7:0] size [7:0]

#define REG_TIMING_HTS_0            0x380C // bits [6:0] size [14:8]
#define REG_TIMING_HTS_1            0x380D // bits [7:0] size [7:0]
#define REG_TIMING_VTS_0            0x380E // bits [6:0] size [14:8]
#define REG_TIMING_VTS_1            0x380F // bits [7:0] size [7:0]

#define REG_SC_CMMN_BIT_SEL         0x3031 // bits [4:0] for bitdepth
#define REG_SC_CMMN_MIPI_SC_CTRL    0x3018 // bits [7:5] for mipi lane mode

#define REG_PLL_CTRL_B              0x030B // PLL2_prediv (values see pll_ctrl_b_val)
#define REG_PLL_CTRL_C              0x030C // PLL2_mult [1:0] multiplier [9:8]
#define REG_PLL_CTRL_D              0x030D // PLL2_mult [7:0] multiplier [7:0]
#define REG_PLL_CTRL_E              0x030E // PLL2_divs (values see pll_ctrl_c_val)
#define REG_PLL_CTRL_F              0x030F // PLL2_divsp (values is 1/(r+1))
#define REG_PLL_CTRL_11             0x0311 // PLL2_predivp (values is 1/(r+1)

#define REG_SC_CMMN_CHIP_ID_0       0x300A // high bits for chip version
#define REG_SC_CMMN_CHIP_ID_1       0x300B // low bits for chip version

#define REG_EXPOSURE                0x3500
#define REG_GAIN                    0x3507


// values for PLL2_prediv 1/x, see sensor data-sheet
static const STF_DOUBLE pll_ctrl_b_val[] = {
    1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 6.0, 8.0
};

// values for PLL2_divs 1/x, see sensor data-sheet
static const STF_DOUBLE pll_ctrl_c_val[] = {
    1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0
};

static const STF_DOUBLE SENSOR_MIN_GAIN = 1.0;
#if 0
static const STF_DOUBLE SENSOR_MAX_GAIN = 15.9375;
#else
static const STF_DOUBLE SENSOR_MAX_GAIN = 31.9375;
#endif //0
static const STF_U32 SENSOR_MIN_EXPOSURE = 10000;
static const STF_U32 SENSOR_MAX_EXPOSURE = 1000000;

static STF_U32 g_u32VerticalTotal = 2000;

static STF_U8 g_OV4689_u8LinesValHiBackup = 0x00;
static STF_U8 g_OV4689_u8LinesValMidBackup = 0x2e;
static STF_U8 g_OV4689_u8LinesValLoBackup = 0x80;

static STF_U8 g_OV4689_u8GainValHiBackup = 0x00;
static STF_U8 g_OV4689_u8GainValMidBackup = 0x00;
static STF_U8 g_OV4689_u8GainValLoBackup = 0x00;

/*
 * from spreadsheet from omnivision (applied to registers 0x3507 0x3508)
 * formula is:
 * if gain < 2.0
 *    reg = gain*128
 * elif gain < 4.0
 *    reg = 256 + (gain*64 -8)
 * elif gain < 8.0
 *    reg = 256*3 + (gain*32 -12)
 * else
 *    reg = 256*7 + (gain*16 -8)
 */
static const STF_U16 g_OV4689_u16AecLongGainVal[] = {
    // 1.0
    0x0080, 0x0088, 0x0090, 0x0098, 0x00A0, 0x00A8, 0x00B0, 0x00B8,
    // 1.5
    0x00C0, 0x00C8, 0x00D0, 0x00D8, 0x00E0, 0x00E8, 0x00F0, 0x00F8,
    // 2.0
    0x0178, 0x017C, 0x0180, 0x0184, 0x0188, 0x018C, 0x0190, 0x0194,
    // 2.5
    0x0198, 0x019C, 0x01A0, 0x01A4, 0x01A8, 0x01AC, 0x01B0, 0x01B4,
    // 3.0
    0x01B8, 0x01BC, 0x01C0, 0x01C4, 0x01C8, 0x01CC, 0x01D0, 0x01D4,
    // 3.5
    0x01D8, 0x01DC, 0x01E0, 0x01E4, 0x01E8, 0x01EC, 0x01F0, 0x01F4,
    // 4.0
    0x0374, 0x0376, 0x0378, 0x037A, 0x037C, 0x037E, 0x0380, 0x0382,
    // 4.5
    0x0384, 0x0386, 0x0388, 0x038A, 0x038C, 0x038E, 0x0390, 0x0392,
    // 5.0
    0x0394, 0x0396, 0x0398, 0x039A, 0x039C, 0x039E, 0x03A0, 0x03A2,
    // 5.5
    0x03A4, 0x03A6, 0x03A8, 0x03AA, 0x03AC, 0x03AE, 0x03B0, 0x03B2,
    // 6.0
    0x03B4, 0x03B6, 0x03B8, 0x03BA, 0x03BC, 0x03BE, 0x03C0, 0x03C2,
    // 6.5
    0x03C4, 0x03C6, 0x03C8, 0x03CA, 0x03CC, 0x03CE, 0x03D0, 0x03D2,
    // 7.0
    0x03D4, 0x03D6, 0x03D8, 0x03DA, 0x03DC, 0x03DE, 0x03E0, 0x03E2,
    // 7.5
    0x03E4, 0x03E6, 0x03E8, 0x03EA, 0x03EC, 0x03EE, 0x03F0, 0x03F2,
    // 8.0
    0x0778, 0x0779, 0x077A, 0x077B, 0x077C, 0x077D, 0x077E, 0x077F,
    // 8.5
    0x0780, 0x0781, 0x0782, 0x0783, 0x0784, 0x0785, 0x0786, 0x0787,
    // 9.0
    0x0788, 0x0789, 0x078A, 0x078B, 0x078C, 0x078D, 0x078E, 0x078F,
    // 9.5
    0x0790, 0x0791, 0x0792, 0x0793, 0x0794, 0x0795, 0x0796, 0x0797,
    // 10.0
    0x0798, 0x0799, 0x079A, 0x079B, 0x079C, 0x079D, 0x079E, 0x079F,
    // 10.5
    0x07A0, 0x07A1, 0x07A2, 0x07A3, 0x07A4, 0x07A5, 0x07A6, 0x07A7,
    // 11.0
    0x07A8, 0x07A9, 0x07AA, 0x07AB, 0x07AC, 0x07AD, 0x07AE, 0x07AF,
    // 11.5
    0x07B0, 0x07B1, 0x07B2, 0x07B3, 0x07B4, 0x07B5, 0x07B6, 0x07B7,
    // 12.0
    0x07B8, 0x07B9, 0x07BA, 0x07BB, 0x07BC, 0x07BD, 0x07BE, 0x07BF,
    // 12.5
    0x07C0, 0x07C1, 0x07C2, 0x07C3, 0x07C4, 0x07C5, 0x07C6, 0x07C7,
    // 13.0
    0x07C8, 0x07C9, 0x07CA, 0x07CB, 0x07CC, 0x07CD, 0x07CE, 0x07CF,
    // 13.5
    0x07D0, 0x07D1, 0x07D2, 0x07D3, 0x07D4, 0x07D5, 0x07D6, 0x07D7,
    // 14.0
    0x07D8, 0x07D9, 0x07DA, 0x07DB, 0x07DC, 0x07DD, 0x07DE, 0x07DF,
    // 14.5
    0x07E0, 0x07E1, 0x07E2, 0x07E3, 0x07E4, 0x07E5, 0x07E6, 0x07E7,
    // 15.0
    0x07E8, 0x07E9, 0x07EA, 0x07EB, 0x07EC, 0x07ED, 0x07EE, 0x07EF,
    // 15.5
    0x07F0, 0x07F1, 0x07F2, 0x07F3, 0x07F4, 0x07F5, 0x07F6, 0x07F7,
    // 16.0
    0x07F8, 0x07F9, 0x07FA, 0x07FB, 0x07FC, 0x07FD, 0x07FE, 0x07FF,
    // 16.5
    0x0800, 0x0801, 0x0802, 0x0803, 0x0804, 0x0805, 0x0806, 0x0807,
    // 17.0
    0x0808, 0x0809, 0x080A, 0x080B, 0x080C, 0x080D, 0x080E, 0x080F,
    // 17.5
    0x0810, 0x0811, 0x0812, 0x0813, 0x0814, 0x0815, 0x0816, 0x0817,
    // 18.0
    0x0818, 0x0819, 0x081A, 0x081B, 0x081C, 0x081D, 0x081E, 0x081F,
    // 18.5
    0x0820, 0x0821, 0x0822, 0x0823, 0x0824, 0x0825, 0x0826, 0x0827,
    // 19.0
    0x0828, 0x0829, 0x082A, 0x082B, 0x082C, 0x082D, 0x082E, 0x082F,
    // 19.5
    0x0830, 0x0831, 0x0832, 0x0833, 0x0834, 0x0835, 0x0836, 0x0837,
    // 20.0
    0x0838, 0x0839, 0x083A, 0x083B, 0x083C, 0x083D, 0x083E, 0x083F,
    // 20.5
    0x0840, 0x0841, 0x0842, 0x0843, 0x0844, 0x0845, 0x0846, 0x0847,
    // 21.0
    0x0848, 0x0849, 0x084A, 0x084B, 0x084C, 0x084D, 0x084E, 0x084F,
    // 21.5
    0x0850, 0x0851, 0x0852, 0x0853, 0x0854, 0x0855, 0x0856, 0x0857,
    // 22.0
    0x0858, 0x0859, 0x085A, 0x085B, 0x085C, 0x085D, 0x085E, 0x085F,
    // 22.5
    0x0860, 0x0861, 0x0862, 0x0863, 0x0864, 0x0865, 0x0866, 0x0867,
    // 23.0
    0x0868, 0x0869, 0x086A, 0x086B, 0x086C, 0x086D, 0x086E, 0x086F,
    // 23.5
    0x0870, 0x0871, 0x0872, 0x0873, 0x0874, 0x0875, 0x0876, 0x0877,
    // 24.0
    0x0878, 0x0879, 0x087A, 0x087B, 0x087C, 0x087D, 0x087E, 0x087F,
    // 24.5
    0x0880, 0x0881, 0x0882, 0x0883, 0x0884, 0x0885, 0x0886, 0x0887,
    // 25.0
    0x0888, 0x0889, 0x088A, 0x088B, 0x088C, 0x088D, 0x088E, 0x088F,
    // 25.5
    0x0890, 0x0891, 0x0892, 0x0893, 0x0894, 0x0895, 0x0896, 0x0897,
    // 26.0
    0x0898, 0x0899, 0x089A, 0x089B, 0x089C, 0x089D, 0x089E, 0x089F,
    // 26.5
    0x08A0, 0x08A1, 0x08A2, 0x08A3, 0x08A4, 0x08A5, 0x08A6, 0x08A7,
    // 27.0
    0x08A8, 0x08A9, 0x08AA, 0x08AB, 0x08AC, 0x08AD, 0x08AE, 0x08AF,
    // 27.5
    0x08B0, 0x08B1, 0x08B2, 0x08B3, 0x08B4, 0x08B5, 0x08B6, 0x08B7,
    // 28.0
    0x08B8, 0x08B9, 0x08BA, 0x08BB, 0x08BC, 0x08BD, 0x08BE, 0x08BF,
    // 28.5
    0x08C0, 0x08C1, 0x08C2, 0x08C3, 0x08C4, 0x08C5, 0x08C6, 0x08C7,
    // 29.0
    0x08C8, 0x08C9, 0x08CA, 0x08CB, 0x08CC, 0x08CD, 0x08CE, 0x08CF,
    // 29.5
    0x08D0, 0x08D1, 0x08D2, 0x08D3, 0x08D4, 0x08D5, 0x08D6, 0x08D7,
    // 30.0
    0x08D8, 0x08D9, 0x08DA, 0x08DB, 0x08DC, 0x08DD, 0x08DE, 0x08DF,
    // 30.5
    0x08E0, 0x08E1, 0x08E2, 0x08E3, 0x08E4, 0x08E5, 0x08E6, 0x08E7,
    // 31.0
    0x08E8, 0x08E9, 0x08EA, 0x08EB, 0x08EC, 0x08ED, 0x08EE, 0x08EF,
    // 31.5
    0x08F0, 0x08F1, 0x08F2, 0x08F3, 0x08F4, 0x08F5, 0x08F6, 0x08F7,
#if 0
    // 32.0
    0x08F8, 0x08F9, 0x08FA, 0x08FB, 0x08FC, 0x08FD, 0x08FE, 0x08FF,
    // 32.5
    0x0900, 0x0901, 0x0902, 0x0903, 0x0904, 0x0905, 0x0906, 0x0907,
#endif
};


// minimum focus in millimetres
static const STF_U16 g_OV4689_u16FocusDacMin = 50;

// maximum focus in millimetres, if >= then focus is infinity
static const STF_U16 g_OV4689_u16FocusDacMax = 600;

// focus values for the g_OV4689_u16FocusDacDist
static const STF_U16 g_OV4689_u16FocusDacVal[] = {
    0x3ff, 0x180, 0x120, 0x100, 0x000
};

// distances in millimetres for the g_OV4689_u16FocusDacVal
static const STF_U16 g_OV4689_u16FocusDacDist[] = {
    50, 150, 300, 500, 600
};

//
// modes declaration
//

#if defined(ENABLE_1920_1080_30_4lane)
// 4lanes 30fps
static STF_U8 g_u8ModeRegs_1920_1080_30_4lane[] = {
    0x01, 0x03, 0x01,                                                   // 001
    0x36, 0x38, 0x00,                                                   // 002
#if 0
    0x03, 0x00, 0x00,                                                   // 003
    0x03, 0x02, 0x2a,                                                   // 004
    0x03, 0x03, 0x01/*0x01*/, // ??? use 0x01 for test pll1_divdm 1/2   // 005
#else
    // For WaveShare's sensor.
    0x03, 0x00, 0x02,                                                   // 003
    0x03, 0x02, 0x32,                                                   // 004
    0x03, 0x03, 0x00/*0x01*/, // ??? use 0x01 for test pll1_divdm 1/2   // 005
#endif //#if 0
    0x03, 0x04, 0x03,                                                   // 006
    0x03, 0x0a, 0x00, // pll1 prediv                                    // 007
    0x03, 0x0b, 0x00,                                                   // 008
    0x03, 0x0d, 0x1e,                                                   // 009
    0x03, 0x0e, 0x04,                                                   // 010
    0x03, 0x0f, 0x01,                                                   // 011
    0x03, 0x11, 0x00, // pll2 prediv                                    // 012
    0x03, 0x12, 0x01,                                                   // 013
    0x03, 0x1e, 0x00,                                                   // 014
    0x30, 0x00, 0x20,                                                   // 015
    0x30, 0x02, 0x00,                                                   // 016
    0x30, 0x18, 0x72,                                                   // 017
    0x30, 0x20, 0x93,                                                   // 018
    0x30, 0x21, 0x03,                                                   // 019
    0x30, 0x22, 0x01,                                                   // 020
    0x30, 0x31, 0x0a,                                                   // 021
    0x30, 0x3f, 0x0c,                                                   // 022
    0x33, 0x05, 0xf1,                                                   // 023
    0x33, 0x07, 0x04,                                                   // 024
    0x33, 0x09, 0x29,                                                   // 025
    0x35, 0x00, 0x00,                                                   // 026
    0x35, 0x01, 0x4c,                                                   // 027
    0x35, 0x02, 0x00,                                                   // 028
    0x35, 0x03, 0x04,                                                   // 029
    0x35, 0x04, 0x00,                                                   // 030
    0x35, 0x05, 0x00,                                                   // 031
    0x35, 0x06, 0x00,                                                   // 032
    0x35, 0x07, 0x00,                                                   // 033
    0x35, 0x08, 0x00/*0x01*/,                                           // 034
    0x35, 0x09, 0x80/*0xb8*/,                                           // 035
    0x35, 0x0a, 0x00,                                                   // 036
    0x35, 0x0b, 0x00,                                                   // 037
    0x35, 0x0c, 0x00,                                                   // 038
    0x35, 0x0d, 0x00,                                                   // 039
    0x35, 0x0e, 0x00,                                                   // 040
    0x35, 0x0f, 0x80,                                                   // 041
    0x35, 0x10, 0x00,                                                   // 042
    0x35, 0x11, 0x00,                                                   // 043
    0x35, 0x12, 0x00,                                                   // 044
    0x35, 0x13, 0x00,                                                   // 045
    0x35, 0x14, 0x00,                                                   // 046
    0x35, 0x15, 0x80,                                                   // 047
    0x35, 0x16, 0x00,                                                   // 048
    0x35, 0x17, 0x00,                                                   // 049
    0x35, 0x18, 0x00,                                                   // 050
    0x35, 0x19, 0x00,                                                   // 051
    0x35, 0x1a, 0x00,                                                   // 052
    0x35, 0x1b, 0x80,                                                   // 053
    0x35, 0x1c, 0x00,                                                   // 054
    0x35, 0x1d, 0x00,                                                   // 055
    0x35, 0x1e, 0x00,                                                   // 056
    0x35, 0x1f, 0x00,                                                   // 057
    0x35, 0x20, 0x00,                                                   // 058
    0x35, 0x21, 0x80,                                                   // 059
    0x35, 0x22, 0x08,                                                   // 060
    0x35, 0x24, 0x08,                                                   // 061
    0x35, 0x26, 0x08,                                                   // 062
    0x35, 0x28, 0x08,                                                   // 063
    0x35, 0x2a, 0x08,                                                   // 064
    0x36, 0x02, 0x00,                                                   // 065
    0x36, 0x03, 0x40,                                                   // 066
    0x36, 0x04, 0x02,                                                   // 067
    0x36, 0x05, 0x00,                                                   // 068
    0x36, 0x06, 0x00,                                                   // 069
    0x36, 0x07, 0x00,                                                   // 070
    0x36, 0x09, 0x12,                                                   // 071
    0x36, 0x0a, 0x40,                                                   // 072
    0x36, 0x0c, 0x08,                                                   // 073
    0x36, 0x0f, 0xe5,                                                   // 074
    0x36, 0x08, 0x8f,                                                   // 075
    0x36, 0x11, 0x00,                                                   // 076
    0x36, 0x13, 0xf7,                                                   // 077
    0x36, 0x16, 0x58,                                                   // 078
    0x36, 0x19, 0x99,                                                   // 079
    0x36, 0x1b, 0x60,                                                   // 080
    0x36, 0x1c, 0x7a,                                                   // 081
    0x36, 0x1e, 0x79,                                                   // 082
    0x36, 0x1f, 0x02,                                                   // 083
    0x36, 0x32, 0x00,                                                   // 084
    0x36, 0x33, 0x10,                                                   // 085
    0x36, 0x34, 0x10,                                                   // 086
    0x36, 0x35, 0x10,                                                   // 087
    0x36, 0x36, 0x15,                                                   // 088
    0x36, 0x46, 0x86,                                                   // 089
    0x36, 0x4a, 0x0b,                                                   // 090
    0x37, 0x00, 0x17,                                                   // 091
    0x37, 0x01, 0x22,                                                   // 092
    0x37, 0x03, 0x10,                                                   // 093
    0x37, 0x0a, 0x37,                                                   // 094
    0x37, 0x05, 0x00,                                                   // 095
    0x37, 0x06, 0x63,                                                   // 096
    0x37, 0x09, 0x3c,                                                   // 097
    0x37, 0x0b, 0x01,                                                   // 098
    0x37, 0x0c, 0x30,                                                   // 099
    0x37, 0x10, 0x24,                                                   // 100
    0x37, 0x11, 0x0c,                                                   // 101
    0x37, 0x16, 0x00,                                                   // 102
    0x37, 0x20, 0x28,                                                   // 103
    0x37, 0x29, 0x7b,                                                   // 104
    0x37, 0x2a, 0x84,                                                   // 105
    0x37, 0x2b, 0xbd,                                                   // 106
    0x37, 0x2c, 0xbc,                                                   // 107
    0x37, 0x2e, 0x52,                                                   // 108
    0x37, 0x3c, 0x0e,                                                   // 109
    0x37, 0x3e, 0x33,                                                   // 110
    0x37, 0x43, 0x10,                                                   // 111
    0x37, 0x44, 0x88,                                                   // 112
    0x37, 0x45, 0xc0,                                                   // 113
    0x37, 0x4a, 0x43,                                                   // 114
    0x37, 0x4c, 0x00,                                                   // 115
    0x37, 0x4e, 0x23,                                                   // 116
    0x37, 0x51, 0x7b,                                                   // 117
    0x37, 0x52, 0x84,                                                   // 118
    0x37, 0x53, 0xbd,                                                   // 119
    0x37, 0x54, 0xbc,                                                   // 120
    0x37, 0x56, 0x52,                                                   // 121
    0x37, 0x5c, 0x00,                                                   // 122
    0x37, 0x60, 0x00,                                                   // 123
    0x37, 0x61, 0x00,                                                   // 124
    0x37, 0x62, 0x00,                                                   // 125
    0x37, 0x63, 0x00,                                                   // 126
    0x37, 0x64, 0x00,                                                   // 127
    0x37, 0x67, 0x04,                                                   // 128
    0x37, 0x68, 0x04,                                                   // 129
    0x37, 0x69, 0x08,                                                   // 130
    0x37, 0x6a, 0x08,                                                   // 131
    0x37, 0x6b, 0x20,                                                   // 132
    0x37, 0x6c, 0x00,                                                   // 133
    0x37, 0x6d, 0x00,                                                   // 134
    0x37, 0x6e, 0x00,                                                   // 135
    0x37, 0x73, 0x00,                                                   // 136
    0x37, 0x74, 0x51,                                                   // 137
    0x37, 0x76, 0xbd,                                                   // 138
    0x37, 0x77, 0xbd,                                                   // 139
    0x37, 0x81, 0x18,                                                   // 140
    0x37, 0x83, 0x25,                                                   // 141
    0x37, 0x98, 0x1b,                                                   // 142
    0x38, 0x00, 0x01,                                                   // 143
    0x38, 0x01, 0x88,                                                   // 144
    0x38, 0x02, 0x00,                                                   // 145
    0x38, 0x03, 0xe0,                                                   // 146
    0x38, 0x04, 0x09,                                                   // 147
    0x38, 0x05, 0x17,                                                   // 148
    0x38, 0x06, 0x05,                                                   // 149
    0x38, 0x07, 0x1f,                                                   // 150
    0x38, 0x08, 0x07,                                                   // 151
    0x38, 0x09, 0x80,                                                   // 152
    0x38, 0x0a, 0x04,                                                   // 153
    0x38, 0x0b, 0x38,                                                   // 154
                                                                        //
    0x38, 0x0c, 0x0d, // 30fps                                          // 155
    0x38, 0x0d, 0x70,                                                   // 156
                                                                        //
    0x38, 0x0e, 0x04,                                                   // 157
    0x38, 0x0f, 0x8A,                                                   // 158
    0x38, 0x10, 0x00,                                                   // 159
    0x38, 0x11, 0x08,                                                   // 160
    0x38, 0x12, 0x00,                                                   // 161
    0x38, 0x13, 0x04,                                                   // 162
    0x38, 0x14, 0x01,                                                   // 163
    0x38, 0x15, 0x01,                                                   // 164
    0x38, 0x19, 0x01,                                                   // 165
#ifndef OV4689_FLIP_MIRROR_FRIST                                        //
    0x38, 0x20, 0x00,                                                   // 166
    0x38, 0x21, 0x06,                                                   // 167
#else                                                                   //
    0x38, 0x20, 0x06,                                                   // 166
    0x38, 0x21, 0x00,                                                   // 167
#endif //OV4689_FLIP_MIRROR_FRIST                                       //
    0x38, 0x29, 0x00,                                                   // 168
    0x38, 0x2a, 0x01,                                                   // 169
    0x38, 0x2b, 0x01,                                                   // 170
    0x38, 0x2d, 0x7f,                                                   // 171
    0x38, 0x30, 0x04,                                                   // 172
    0x38, 0x36, 0x01,                                                   // 173
    0x38, 0x37, 0x00,                                                   // 174
    0x38, 0x41, 0x02,                                                   // 175
    0x38, 0x46, 0x08,                                                   // 176
    0x38, 0x47, 0x07,                                                   // 177
    0x3d, 0x85, 0x36,                                                   // 178
    0x3d, 0x8c, 0x71,                                                   // 179
    0x3d, 0x8d, 0xcb,                                                   // 180
    0x3f, 0x0a, 0x00,                                                   // 181
    0x40, 0x00, 0xf1,                                                   // 182
    0x40, 0x01, 0x40,                                                   // 183
    0x40, 0x02, 0x04,                                                   // 184
    0x40, 0x03, 0x14,                                                   // 185
    0x40, 0x0e, 0x00,                                                   // 186
    0x40, 0x11, 0x00,                                                   // 187
    0x40, 0x1a, 0x00,                                                   // 188
    0x40, 0x1b, 0x00,                                                   // 189
    0x40, 0x1c, 0x00,                                                   // 190
    0x40, 0x1d, 0x00,                                                   // 191
    0x40, 0x1f, 0x00,                                                   // 192
    0x40, 0x20, 0x00,                                                   // 193
    0x40, 0x21, 0x10,                                                   // 194
    0x40, 0x22, 0x06,                                                   // 195
    0x40, 0x23, 0x13,                                                   // 196
    0x40, 0x24, 0x07,                                                   // 197
    0x40, 0x25, 0x40,                                                   // 198
    0x40, 0x26, 0x07,                                                   // 199
    0x40, 0x27, 0x50,                                                   // 200
    0x40, 0x28, 0x00,                                                   // 201
    0x40, 0x29, 0x02,                                                   // 202
    0x40, 0x2a, 0x06,                                                   // 203
    0x40, 0x2b, 0x04,                                                   // 204
    0x40, 0x2c, 0x02,                                                   // 205
    0x40, 0x2d, 0x02,                                                   // 206
    0x40, 0x2e, 0x0e,                                                   // 207
    0x40, 0x2f, 0x04,                                                   // 208
    0x43, 0x02, 0xff,                                                   // 209
    0x43, 0x03, 0xff,                                                   // 210
    0x43, 0x04, 0x00,                                                   // 211
    0x43, 0x05, 0x00,                                                   // 212
    0x43, 0x06, 0x00,                                                   // 213
    0x43, 0x08, 0x02,                                                   // 214
    0x45, 0x00, 0x6c,                                                   // 215
    0x45, 0x01, 0xc4,                                                   // 216
    0x45, 0x02, 0x40,                                                   // 217
    0x45, 0x03, 0x01,                                                   // 218
    0x46, 0x01, 0x77,                                                   // 219
    0x48, 0x00, 0x04, // timing                                         // 220
    0x48, 0x13, 0x08,                                                   // 221
    0x48, 0x1f, 0x40,                                                   // 222
    // HS-prepare//                                                     //
    //0x48, 0x26, 0x28,// - 0x10, // HS_PREPARE_MIN                     //
    //0x48, 0x27, 0x55,// + 0x10, // HS_PREPARE_MAX                     //
    //0x48, 0x31, 0x6C - 0x35, // UI_HS_PREPARE_MIN                     //
    ///////////////                                                     //
    0x48, 0x29, 0x78,                                                   // 223
    0x48, 0x37, 0x10,                                                   // 224
    0x4b, 0x00, 0x2a,                                                   // 225
    0x4b, 0x0d, 0x00,                                                   // 226
    0x4d, 0x00, 0x04,                                                   // 227
    0x4d, 0x01, 0x42,                                                   // 228
    0x4d, 0x02, 0xd1,                                                   // 229
    0x4d, 0x03, 0x93,                                                   // 230
    0x4d, 0x04, 0xf5,                                                   // 231
    0x4d, 0x05, 0xc1,                                                   // 232
    0x50, 0x00, 0xf3,                                                   // 233
    0x50, 0x01, 0x11,                                                   // 234
    0x50, 0x04, 0x00,                                                   // 235
    0x50, 0x0a, 0x00,                                                   // 236
    0x50, 0x0b, 0x00,                                                   // 237
    0x50, 0x32, 0x00,                                                   // 238
    0x50, 0x40, 0x00,                                                   // 239
    0x50, 0x50, 0x0c,                                                   // 240
    0x55, 0x00, 0x00,                                                   // 241
    0x55, 0x01, 0x10,                                                   // 242
    0x55, 0x02, 0x01,                                                   // 243
    0x55, 0x03, 0x0f,                                                   // 244
    0x80, 0x00, 0x00,                                                   // 245
    0x80, 0x01, 0x00,                                                   // 246
    0x80, 0x02, 0x00,                                                   // 247
    0x80, 0x03, 0x00,                                                   // 248
    0x80, 0x04, 0x00,                                                   // 249
    0x80, 0x05, 0x00,                                                   // 250
    0x80, 0x06, 0x00,                                                   // 251
    0x80, 0x07, 0x00,                                                   // 252
    0x80, 0x08, 0x00,                                                   // 253
    0x36, 0x38, 0x00,                                                   // 254
    //0x01, 0x00, 0x01,
    //0x03, 0x00, 0x60,
    STOP_REG, STOP_REG, STOP_REG,
};

#endif //#if defined(ENABLE_1920_1080_30_4lane)
#if defined(ENABLE_1920_1088_60_4lane)
//// 4lane 60fps
//static STF_U8 g_u8ModeRegs_1920_1088_60_4lane[] = {
//#if 1
//	0x01, 0x03, 0x01,
//	0x36, 0x38, 0x00,
//	0x03, 0x00, 0x04,//0x02,
//	0x03, 0x02, 0x40,//0x32,
//	0x03, 0x03, 0x00,
//	0x03, 0x04, 0x03,
//
//	0x03, 0x05, 0x02,
//	0x03, 0x06, 0x02,
//
//	0x03, 0x0a, 0x00,  // pll1 prediv
//	0x03, 0x0b, 0x00,
//	0x03, 0x0c, 0x00,
//	0x03, 0x0d, 0x1e,
//	0x03, 0x0e, 0x04,
//	0x03, 0x0f, 0x01,
//	0x03, 0x11, 0x00,
//	0x03, 0x12, 0x01,
//	0x03, 0x1e, 0x00,
//	0x30, 0x00, 0x20,
//	0x30, 0x02, 0x00,
//	0x30, 0x18, 0x72, //0x72,
//	0x30, 0x20, 0x93,
//	0x30, 0x21, 0x03,
//	0x30, 0x22, 0x01,
//	0x30, 0x31, 0x0a,
//	0x30, 0x3f, 0x0c,
//	0x33, 0x05, 0xf1,
//	0x33, 0x07, 0x04,
//	0x33, 0x09, 0x29,
//	0x35, 0x00, 0x00,
//	0x35, 0x01, 0x20,
//	0x35, 0x02, 0x00,
//	0x35, 0x03, 0x04,
//	0x35, 0x04, 0x00,
//	0x35, 0x05, 0x00,
//	0x35, 0x06, 0x00,
//	0x35, 0x07, 0x00,
//	0x35, 0x08, 0x00,
//	0x35, 0x09, 0x80,
//	0x35, 0x0a, 0x00,
//	0x35, 0x0b, 0x00,
//	0x35, 0x0c, 0x00,
//	0x35, 0x0d, 0x00,
//	0x35, 0x0e, 0x00,
//	0x35, 0x0f, 0x80,
//	0x35, 0x10, 0x00,
//	0x35, 0x11, 0x00,
//	0x35, 0x12, 0x00,
//	0x35, 0x13, 0x00,
//	0x35, 0x14, 0x00,
//	0x35, 0x15, 0x80,
//	0x35, 0x16, 0x00,
//	0x35, 0x17, 0x00,
//	0x35, 0x18, 0x00,
//	0x35, 0x19, 0x00,
//	0x35, 0x1a, 0x00,
//	0x35, 0x1b, 0x80,
//	0x35, 0x1c, 0x00,
//	0x35, 0x1d, 0x00,
//	0x35, 0x1e, 0x00,
//	0x35, 0x1f, 0x00,
//	0x35, 0x20, 0x00,
//	0x35, 0x21, 0x80,
//	0x35, 0x22, 0x08,
//	0x35, 0x24, 0x08,
//	0x35, 0x26, 0x08,
//	0x35, 0x28, 0x08,
//	0x35, 0x2a, 0x08,
//	0x36, 0x02, 0x00,
//	0x36, 0x03, 0x40,
//	0x36, 0x04, 0x02,
//	0x36, 0x05, 0x00,
//	0x36, 0x06, 0x00,
//	0x36, 0x07, 0x00,
//	0x36, 0x09, 0x12,
//	0x36, 0x0a, 0x40,
//	0x36, 0x0c, 0x08,
//	0x36, 0x0f, 0xe5,
//	0x36, 0x08, 0x8f,
//	0x36, 0x11, 0x00,
//	0x36, 0x13, 0xf7,
//	0x36, 0x16, 0x58,
//	0x36, 0x19, 0x99,
//	0x36, 0x1b, 0x60,
//	0x36, 0x1c, 0x7a,
//	0x36, 0x1e, 0x79,
//	0x36, 0x1f, 0x02,
//	0x36, 0x32, 0x00,
//	0x36, 0x33, 0x10,
//	0x36, 0x34, 0x10,
//	0x36, 0x35, 0x10,
//	0x36, 0x36, 0x15,
//	0x36, 0x46, 0x86,
//	0x36, 0x4a, 0x0b,
//	0x37, 0x00, 0x17,
//	0x37, 0x01, 0x22,
//	0x37, 0x03, 0x10,
//	0x37, 0x0a, 0x37,
//	0x37, 0x05, 0x00,
//	0x37, 0x06, 0x63,
//	0x37, 0x09, 0x3c,
//	0x37, 0x0b, 0x01,
//	0x37, 0x0c, 0x30,
//	0x37, 0x10, 0x24,
//	0x37, 0x11, 0x0c,
//	0x37, 0x16, 0x00,
//	0x37, 0x20, 0x28,
//	0x37, 0x29, 0x7b,
//	0x37, 0x2a, 0x84,
//	0x37, 0x2b, 0xbd,
//	0x37, 0x2c, 0xbc,
//	0x37, 0x2e, 0x52,
//	0x37, 0x3c, 0x0e,
//	0x37, 0x3e, 0x33,
//	0x37, 0x43, 0x10,
//	0x37, 0x44, 0x88,
//	0x37, 0x45, 0xc0,
//	0x37, 0x4a, 0x43,
//	0x37, 0x4c, 0x00,
//	0x37, 0x4e, 0x23,
//	0x37, 0x51, 0x7b,
//	0x37, 0x52, 0x84,
//	0x37, 0x53, 0xbd,
//	0x37, 0x54, 0xbc,
//	0x37, 0x56, 0x52,
//	0x37, 0x5c, 0x00,
//	0x37, 0x60, 0x00,
//	0x37, 0x61, 0x00,
//	0x37, 0x62, 0x00,
//	0x37, 0x63, 0x00,
//	0x37, 0x64, 0x00,
//	0x37, 0x67, 0x04,
//	0x37, 0x68, 0x04,
//	0x37, 0x69, 0x08,
//	0x37, 0x6a, 0x08,
//	0x37, 0x6b, 0x20,
//	0x37, 0x6c, 0x00,
//	0x37, 0x6d, 0x00,
//	0x37, 0x6e, 0x00,
//	0x37, 0x73, 0x00,
//	0x37, 0x74, 0x51,
//	0x37, 0x76, 0xbd,
//	0x37, 0x77, 0xbd,
//	0x37, 0x81, 0x18,
//	0x37, 0x83, 0x25,
//	0x37, 0x98, 0x1b,
//	0x38, 0x00, 0x01,
//	0x38, 0x01, 0x88,
//	0x38, 0x02, 0x00,
//	0x38, 0x03, 0xdc,//e0
//	0x38, 0x04, 0x09,
//	0x38, 0x05, 0x17,
//	0x38, 0x06, 0x05,
//	0x38, 0x07, 0x23,//1f
//	0x38, 0x08, 0x07,
//	0x38, 0x09, 0x80,
//	0x38, 0x0a, 0x04,
//	0x38, 0x0b, 0x40,//38
//	0x38, 0x0c, 0x06,//03
//	0x38, 0x0d, 0xf3,//5c
//	0x38, 0x0e, 0x04,
//	0x38, 0x0f, 0x64,
//	0x38, 0x10, 0x00,
//	0x38, 0x11, 0x08,
//	0x38, 0x12, 0x00,
//	0x38, 0x13, 0x04,
//	0x38, 0x14, 0x01,
//	0x38, 0x15, 0x01,
//	0x38, 0x19, 0x01,
//	0x38, 0x20, 0x00,
//	0x38, 0x21, 0x06,
//	0x38, 0x29, 0x00,
//	0x38, 0x2a, 0x01,
//	0x38, 0x2b, 0x01,
//	0x38, 0x2d, 0x7f,
//	0x38, 0x30, 0x04,
//	0x38, 0x36, 0x01,
//	0x38, 0x37, 0x00,
//	0x38, 0x41, 0x02,
//	0x38, 0x46, 0x08,
//	0x38, 0x47, 0x07,
//	0x3d, 0x85, 0x36,
//	0x3d, 0x8c, 0x71,
//	0x3d, 0x8d, 0xcb,
//	0x3f, 0x0a, 0x00,
//	0x40, 0x00, 0xf1,
//	0x40, 0x01, 0x40,
//	0x40, 0x02, 0x04,
//	0x40, 0x03, 0x14,
//	0x40, 0x0e, 0x00,
//	0x40, 0x11, 0x00,
//	0x40, 0x1a, 0x00,
//	0x40, 0x1b, 0x00,
//	0x40, 0x1c, 0x00,
//	0x40, 0x1d, 0x00,
//	0x40, 0x1f, 0x00,
//	0x40, 0x20, 0x00,
//	0x40, 0x21, 0x10,
//	0x40, 0x22, 0x06,
//	0x40, 0x23, 0x13,
//	0x40, 0x24, 0x07,
//	0x40, 0x25, 0x40,
//	0x40, 0x26, 0x07,
//	0x40, 0x27, 0x50,
//	0x40, 0x28, 0x00,
//	0x40, 0x29, 0x02,
//	0x40, 0x2a, 0x06,
//	0x40, 0x2b, 0x04,
//	0x40, 0x2c, 0x02,
//	0x40, 0x2d, 0x02,
//	0x40, 0x2e, 0x0e,
//	0x40, 0x2f, 0x04,
//	0x43, 0x02, 0xff,
//	0x43, 0x03, 0xff,
//	0x43, 0x04, 0x00,
//	0x43, 0x05, 0x00,
//	0x43, 0x06, 0x00,
//	0x43, 0x08, 0x02,
//	0x45, 0x00, 0x6c,
//	0x45, 0x01, 0xc4,
//	0x45, 0x02, 0x40,
//	0x45, 0x03, 0x01,
//	0x46, 0x01, 0x77,
//	0x48, 0x00, 0x04,
//	0x48, 0x13, 0x08,
//	0x48, 0x1f, 0x40,
//	0x48, 0x29, 0x78,
//	0x48, 0x37, 0x1b,
//	0x4b, 0x00, 0x2a,
//	0x4b, 0x0d, 0x00,
//	0x4d, 0x00, 0x04,
//	0x4d, 0x01, 0x42,
//	0x4d, 0x02, 0xd1,
//	0x4d, 0x03, 0x93,
//	0x4d, 0x04, 0xf5,
//	0x4d, 0x05, 0xc1,
//	0x50, 0x00, 0xf3,
//	0x50, 0x01, 0x11,
//	0x50, 0x04, 0x00,
//	0x50, 0x0a, 0x00,
//	0x50, 0x0b, 0x00,
//	0x50, 0x32, 0x00,
//	0x50, 0x40, 0x00,
//	0x50, 0x50, 0x0c,
//	0x55, 0x00, 0x00,
//	0x55, 0x01, 0x10,
//	0x55, 0x02, 0x01,
//	0x55, 0x03, 0x0f,
//	0x80, 0x00, 0x00,
//	0x80, 0x01, 0x00,
//	0x80, 0x02, 0x00,
//	0x80, 0x03, 0x00,
//	0x80, 0x04, 0x00,
//	0x80, 0x05, 0x00,
//	0x80, 0x06, 0x00,
//	0x80, 0x07, 0x00,
//	0x80, 0x08, 0x00,
//	0x36, 0x38, 0x00,
//	//0x01, 0x00, 0x01,
//	STOP_REG, STOP_REG, STOP_REG,
//#else //for mode5 use mode0
//    0x01, 0x03, 0x01,
//    0x36, 0x38, 0x00,
//    0x03, 0x00, 0x00,
//    0x03, 0x02, 0x2a,  // 1_LANE
//    0x03, 0x03, 0x00,  // ??? use 0x01 for test pll1_divdm 1/2
//    0x03, 0x04, 0x03,
//    0x03, 0x0b, 0x00,
//    0x03, 0x0d, 0x1e,
//    0x03, 0x0e, 0x04,
//    0x03, 0x0f, 0x01,
//    0x03, 0x11, 0x00,  // pll2 prediv
//    0x03, 0x12, 0x01,
//    0x03, 0x1e, 0x00,
//    0x30, 0x00, 0x20,
//    0x30, 0x02, 0x00,
//    0x30, 0x18, 0x12,  // 1_LANE
//    0x30, 0x20, 0x93,
//    0x30, 0x21, 0x03,
//    0x30, 0x22, 0x01,
//    0x30, 0x31, 0x0a,
//    0x30, 0x3f, 0x0c,
//    0x33, 0x05, 0xf1,
//    0x33, 0x07, 0x04,
//    0x33, 0x09, 0x29,
//    0x35, 0x00, 0x00,
//    0x35, 0x01, 0x4c,
//    0x35, 0x02, 0x00,
//    0x35, 0x03, 0x04,
//    0x35, 0x04, 0x00,
//    0x35, 0x05, 0x00,
//    0x35, 0x06, 0x00,
//    0x35, 0x07, 0x00,
//    0x35, 0x08, 0x00,
//    0x35, 0x09, 0x80,
//    0x35, 0x0a, 0x00,
//    0x35, 0x0b, 0x00,
//    0x35, 0x0c, 0x00,
//    0x35, 0x0d, 0x00,
//    0x35, 0x0e, 0x00,
//    0x35, 0x0f, 0x80,
//    0x35, 0x10, 0x00,
//    0x35, 0x11, 0x00,
//    0x35, 0x12, 0x00,
//    0x35, 0x13, 0x00,
//    0x35, 0x14, 0x00,
//    0x35, 0x15, 0x80,
//    0x35, 0x16, 0x00,
//    0x35, 0x17, 0x00,
//    0x35, 0x18, 0x00,
//    0x35, 0x19, 0x00,
//    0x35, 0x1a, 0x00,
//    0x35, 0x1b, 0x80,
//    0x35, 0x1c, 0x00,
//    0x35, 0x1d, 0x00,
//    0x35, 0x1e, 0x00,
//    0x35, 0x1f, 0x00,
//    0x35, 0x20, 0x00,
//    0x35, 0x21, 0x80,
//    0x35, 0x22, 0x08,
//    0x35, 0x24, 0x08,
//    0x35, 0x26, 0x08,
//    0x35, 0x28, 0x08,
//    0x35, 0x2a, 0x08,
//    0x36, 0x02, 0x00,
//    0x36, 0x03, 0x40,
//    0x36, 0x04, 0x02,
//    0x36, 0x05, 0x00,
//    0x36, 0x06, 0x00,
//    0x36, 0x07, 0x00,
//    0x36, 0x09, 0x12,
//    0x36, 0x0a, 0x40,
//    0x36, 0x0c, 0x08,
//    0x36, 0x0f, 0xe5,
//    0x36, 0x08, 0x8f,
//    0x36, 0x11, 0x00,
//    0x36, 0x13, 0xf7,
//    0x36, 0x16, 0x58,
//    0x36, 0x19, 0x99,
//    0x36, 0x1b, 0x60,
//    0x36, 0x1c, 0x7a,
//    0x36, 0x1e, 0x79,
//    0x36, 0x1f, 0x02,
//    0x36, 0x32, 0x00,
//    0x36, 0x33, 0x10,
//    0x36, 0x34, 0x10,
//    0x36, 0x35, 0x10,
//    0x36, 0x36, 0x15,
//    0x36, 0x46, 0x86,
//    0x36, 0x4a, 0x0b,
//    0x37, 0x00, 0x17,
//    0x37, 0x01, 0x22,
//    0x37, 0x03, 0x10,
//    0x37, 0x0a, 0x37,
//    0x37, 0x05, 0x00,
//    0x37, 0x06, 0x63,
//    0x37, 0x09, 0x3c,
//    0x37, 0x0b, 0x01,
//    0x37, 0x0c, 0x30,
//    0x37, 0x10, 0x24,
//    0x37, 0x11, 0x0c,
//    0x37, 0x16, 0x00,
//    0x37, 0x20, 0x28,
//    0x37, 0x29, 0x7b,
//    0x37, 0x2a, 0x84,
//    0x37, 0x2b, 0xbd,
//    0x37, 0x2c, 0xbc,
//    0x37, 0x2e, 0x52,
//    0x37, 0x3c, 0x0e,
//    0x37, 0x3e, 0x33,
//    0x37, 0x43, 0x10,
//    0x37, 0x44, 0x88,
//    0x37, 0x45, 0xc0,
//    0x37, 0x4a, 0x43,
//    0x37, 0x4c, 0x00,
//    0x37, 0x4e, 0x23,
//    0x37, 0x51, 0x7b,
//    0x37, 0x52, 0x84,
//    0x37, 0x53, 0xbd,
//    0x37, 0x54, 0xbc,
//    0x37, 0x56, 0x52,
//    0x37, 0x5c, 0x00,
//    0x37, 0x60, 0x00,
//    0x37, 0x61, 0x00,
//    0x37, 0x62, 0x00,
//    0x37, 0x63, 0x00,
//    0x37, 0x64, 0x00,
//    0x37, 0x67, 0x04,
//    0x37, 0x68, 0x04,
//    0x37, 0x69, 0x08,
//    0x37, 0x6a, 0x08,
//    0x37, 0x6b, 0x20,
//    0x37, 0x6c, 0x00,
//    0x37, 0x6d, 0x00,
//    0x37, 0x6e, 0x00,
//    0x37, 0x73, 0x00,
//    0x37, 0x74, 0x51,
//    0x37, 0x76, 0xbd,
//    0x37, 0x77, 0xbd,
//    0x37, 0x81, 0x18,
//    0x37, 0x83, 0x25,
//    0x37, 0x98, 0x1b,
//    0x38, 0x00, 0x01,
//    0x38, 0x01, 0x88,
//    0x38, 0x02, 0x00,
//    0x38, 0x03, 0xdc, //0xe0,
//    0x38, 0x04, 0x09,
//    0x38, 0x05, 0x17,
//    0x38, 0x06, 0x05,
//    0x38, 0x07, 0x23, //0x1f,
//    0x38, 0x08, 0x07,
//    0x38, 0x09, 0x80,
//    0x38, 0x0a, 0x04,
//    0x38, 0x0b, 0x40, //0x38,
//    ///////////////
//    0x38, 0x0c, 0x0c,  // 30fps
//    0x38, 0x0d, 0xb4,
//    ///////////////
//    0x38, 0x0e, 0x04,
//    0x38, 0x0f, 0x64, //0x8A,
//    0x38, 0x10, 0x00,
//    0x38, 0x11, 0x08,
//    0x38, 0x12, 0x00,
//    0x38, 0x13, 0x04,
//    0x38, 0x14, 0x01,
//    0x38, 0x15, 0x01,
//    0x38, 0x19, 0x01,
//#ifndef OV4689_FLIP_MIRROR_FRIST
//    0x38, 0x20, 0x00,
//    0x38, 0x21, 0x06,
//#else
//    0x38, 0x20, 0x06,
//    0x38, 0x21, 0x00,
//#endif
//    0x38, 0x29, 0x00,
//    0x38, 0x2a, 0x01,
//    0x38, 0x2b, 0x01,
//    0x38, 0x2d, 0x7f,
//    0x38, 0x30, 0x04,
//    0x38, 0x36, 0x01,
//    0x38, 0x37, 0x00,
//    0x38, 0x41, 0x02,
//    0x38, 0x46, 0x08,
//    0x38, 0x47, 0x07,
//    0x3d, 0x85, 0x36,
//    0x3d, 0x8c, 0x71,
//    0x3d, 0x8d, 0xcb,
//    0x3f, 0x0a, 0x00,
//    0x40, 0x00, 0xf1,
//    0x40, 0x01, 0x40,
//    0x40, 0x02, 0x04,
//    0x40, 0x03, 0x14,
//    0x40, 0x0e, 0x00,
//    0x40, 0x11, 0x00,
//    0x40, 0x1a, 0x00,
//    0x40, 0x1b, 0x00,
//    0x40, 0x1c, 0x00,
//    0x40, 0x1d, 0x00,
//    0x40, 0x1f, 0x00,
//    0x40, 0x20, 0x00,
//    0x40, 0x21, 0x10,
//    0x40, 0x22, 0x06,
//    0x40, 0x23, 0x13,
//    0x40, 0x24, 0x07,
//    0x40, 0x25, 0x40,
//    0x40, 0x26, 0x07,
//    0x40, 0x27, 0x50,
//    0x40, 0x28, 0x00,
//    0x40, 0x29, 0x02,
//    0x40, 0x2a, 0x06,
//    0x40, 0x2b, 0x04,
//    0x40, 0x2c, 0x02,
//    0x40, 0x2d, 0x02,
//    0x40, 0x2e, 0x0e,
//    0x40, 0x2f, 0x04,
//    0x43, 0x02, 0xff,
//    0x43, 0x03, 0xff,
//    0x43, 0x04, 0x00,
//    0x43, 0x05, 0x00,
//    0x43, 0x06, 0x00,
//    0x43, 0x08, 0x02,
//    0x45, 0x00, 0x6c,
//    0x45, 0x01, 0xc4,
//    0x45, 0x02, 0x40,
//    0x45, 0x03, 0x01,
//    0x46, 0x01, 0x77,
//    0x48, 0x00, 0x04,  // timing
//    0x48, 0x13, 0x08,
//    0x48, 0x1f, 0x40,
//    // HS-prepare//
//    //0x48, 0x26, 0x28, // - 0x10,  //HS_PREPARE_MIN
//    //0x48, 0x27, 0x55, // + 0x10,      //HS_PREPARE_MAX
//    //0x48, 0x31, 0x6C - 0x35,  //UI_HS_PREPARE_MIN
//    ///////////////
//    0x48, 0x29, 0x78,
//    0x48, 0x37, 0x10,
//    0x4b, 0x00, 0x2a,
//    0x4b, 0x0d, 0x00,
//    0x4d, 0x00, 0x04,
//    0x4d, 0x01, 0x42,
//    0x4d, 0x02, 0xd1,
//    0x4d, 0x03, 0x93,
//    0x4d, 0x04, 0xf5,
//    0x4d, 0x05, 0xc1,
//    0x50, 0x00, 0xf3,
//    0x50, 0x01, 0x11,
//    0x50, 0x04, 0x00,
//    0x50, 0x0a, 0x00,
//    0x50, 0x0b, 0x00,
//    0x50, 0x32, 0x00,
//    0x50, 0x40, 0x00,
//    0x50, 0x50, 0x0c,
//    0x55, 0x00, 0x00,
//    0x55, 0x01, 0x10,
//    0x55, 0x02, 0x01,
//    0x55, 0x03, 0x0f,
//    0x80, 0x00, 0x00,
//    0x80, 0x01, 0x00,
//    0x80, 0x02, 0x00,
//    0x80, 0x03, 0x00,
//    0x80, 0x04, 0x00,
//    0x80, 0x05, 0x00,
//    0x80, 0x06, 0x00,
//    0x80, 0x07, 0x00,
//    0x80, 0x08, 0x00,
//    0x36, 0x38, 0x00,
//    //0x01, 0x00, 0x01,
//    //0x03, 0x00, 0x60,
//    STOP_REG, STOP_REG, STOP_REG,
//#endif
//};
//
#endif //#if defined(ENABLE_1920_1088_60_4lane)
#if defined(ENABLE_1920_1088_30_4lane)
// 4lanes 30fps
static STF_U8 g_u8ModeRegs_1920_1088_30_4lane[] = {
	0x01, 0x03, 0x01,                                                   // 001
	0x36, 0x38, 0x00,                                                   // 002
	0x03, 0x00, 0x00,                                                   // 003
	0x03, 0x02, 0x2a,                                                   // 004
	0x03, 0x03, 0x01/*0x01*/, // ??? use 0x01 for test pll1_divdm 1/2   // 005
	0x03, 0x04, 0x03,                                                   // 006
	0x03, 0x0a, 0x00, // pll1 prediv                                    // 007
	0x03, 0x0b, 0x00,                                                   // 008
	0x03, 0x0d, 0x1e,                                                   // 009
	0x03, 0x0e, 0x04,                                                   // 010
	0x03, 0x0f, 0x01,                                                   // 011
	0x03, 0x11, 0x00, // pll2 prediv                                    // 012
	0x03, 0x12, 0x01,                                                   // 013
	0x03, 0x1e, 0x00,                                                   // 014
	0x30, 0x00, 0x20,                                                   // 015
	0x30, 0x02, 0x00,                                                   // 016
	0x30, 0x18, 0x72,                                                   // 017
	0x30, 0x20, 0x93,                                                   // 018
	0x30, 0x21, 0x03,                                                   // 019
	0x30, 0x22, 0x01,                                                   // 020
	0x30, 0x31, 0x0a,                                                   // 021
	0x30, 0x3f, 0x0c,                                                   // 022
	0x33, 0x05, 0xf1,                                                   // 023
	0x33, 0x07, 0x04,                                                   // 024
	0x33, 0x09, 0x29,                                                   // 025
	0x35, 0x00, 0x00,                                                   // 026
	0x35, 0x01, 0x4c,                                                   // 027
	0x35, 0x02, 0x00,                                                   // 028
	0x35, 0x03, 0x04,                                                   // 029
	0x35, 0x04, 0x00,                                                   // 030
	0x35, 0x05, 0x00,                                                   // 031
	0x35, 0x06, 0x00,                                                   // 032
	0x35, 0x07, 0x00,                                                   // 033
	0x35, 0x08, 0x00/*0x01*/,                                           // 034
	0x35, 0x09, 0x80/*0xb8*/,                                           // 035
	0x35, 0x0a, 0x00,                                                   // 036
	0x35, 0x0b, 0x00,                                                   // 037
	0x35, 0x0c, 0x00,                                                   // 038
	0x35, 0x0d, 0x00,                                                   // 039
	0x35, 0x0e, 0x00,                                                   // 040
	0x35, 0x0f, 0x80,                                                   // 041
	0x35, 0x10, 0x00,                                                   // 042
	0x35, 0x11, 0x00,                                                   // 043
	0x35, 0x12, 0x00,                                                   // 044
	0x35, 0x13, 0x00,                                                   // 045
	0x35, 0x14, 0x00,                                                   // 046
	0x35, 0x15, 0x80,                                                   // 047
	0x35, 0x16, 0x00,                                                   // 048
	0x35, 0x17, 0x00,                                                   // 049
	0x35, 0x18, 0x00,                                                   // 050
	0x35, 0x19, 0x00,                                                   // 051
	0x35, 0x1a, 0x00,                                                   // 052
	0x35, 0x1b, 0x80,                                                   // 053
	0x35, 0x1c, 0x00,                                                   // 054
	0x35, 0x1d, 0x00,                                                   // 055
	0x35, 0x1e, 0x00,                                                   // 056
	0x35, 0x1f, 0x00,                                                   // 057
	0x35, 0x20, 0x00,                                                   // 058
	0x35, 0x21, 0x80,                                                   // 059
	0x35, 0x22, 0x08,                                                   // 060
	0x35, 0x24, 0x08,                                                   // 061
	0x35, 0x26, 0x08,                                                   // 062
	0x35, 0x28, 0x08,                                                   // 063
	0x35, 0x2a, 0x08,                                                   // 064
	0x36, 0x02, 0x00,                                                   // 065
	0x36, 0x03, 0x40,                                                   // 066
	0x36, 0x04, 0x02,                                                   // 067
	0x36, 0x05, 0x00,                                                   // 068
	0x36, 0x06, 0x00,                                                   // 069
	0x36, 0x07, 0x00,                                                   // 070
	0x36, 0x09, 0x12,                                                   // 071
	0x36, 0x0a, 0x40,                                                   // 072
	0x36, 0x0c, 0x08,                                                   // 073
	0x36, 0x0f, 0xe5,                                                   // 074
	0x36, 0x08, 0x8f,                                                   // 075
	0x36, 0x11, 0x00,                                                   // 076
	0x36, 0x13, 0xf7,                                                   // 077
	0x36, 0x16, 0x58,                                                   // 078
	0x36, 0x19, 0x99,                                                   // 079
	0x36, 0x1b, 0x60,                                                   // 080
	0x36, 0x1c, 0x7a,                                                   // 081
	0x36, 0x1e, 0x79,                                                   // 082
	0x36, 0x1f, 0x02,                                                   // 083
	0x36, 0x32, 0x00,                                                   // 084
	0x36, 0x33, 0x10,                                                   // 085
	0x36, 0x34, 0x10,                                                   // 086
	0x36, 0x35, 0x10,                                                   // 087
	0x36, 0x36, 0x15,                                                   // 088
	0x36, 0x46, 0x86,                                                   // 089
	0x36, 0x4a, 0x0b,                                                   // 090
	0x37, 0x00, 0x17,                                                   // 091
	0x37, 0x01, 0x22,                                                   // 092
	0x37, 0x03, 0x10,                                                   // 093
	0x37, 0x0a, 0x37,                                                   // 094
	0x37, 0x05, 0x00,                                                   // 095
	0x37, 0x06, 0x63,                                                   // 096
	0x37, 0x09, 0x3c,                                                   // 097
	0x37, 0x0b, 0x01,                                                   // 098
	0x37, 0x0c, 0x30,                                                   // 099
	0x37, 0x10, 0x24,                                                   // 100
	0x37, 0x11, 0x0c,                                                   // 101
	0x37, 0x16, 0x00,                                                   // 102
	0x37, 0x20, 0x28,                                                   // 103
	0x37, 0x29, 0x7b,                                                   // 104
	0x37, 0x2a, 0x84,                                                   // 105
	0x37, 0x2b, 0xbd,                                                   // 106
	0x37, 0x2c, 0xbc,                                                   // 107
	0x37, 0x2e, 0x52,                                                   // 108
	0x37, 0x3c, 0x0e,                                                   // 109
	0x37, 0x3e, 0x33,                                                   // 110
	0x37, 0x43, 0x10,                                                   // 111
	0x37, 0x44, 0x88,                                                   // 112
	0x37, 0x45, 0xc0,                                                   // 113
	0x37, 0x4a, 0x43,                                                   // 114
	0x37, 0x4c, 0x00,                                                   // 115
	0x37, 0x4e, 0x23,                                                   // 116
	0x37, 0x51, 0x7b,                                                   // 117
	0x37, 0x52, 0x84,                                                   // 118
	0x37, 0x53, 0xbd,                                                   // 119
	0x37, 0x54, 0xbc,                                                   // 120
	0x37, 0x56, 0x52,                                                   // 121
	0x37, 0x5c, 0x00,                                                   // 122
	0x37, 0x60, 0x00,                                                   // 123
	0x37, 0x61, 0x00,                                                   // 124
	0x37, 0x62, 0x00,                                                   // 125
	0x37, 0x63, 0x00,                                                   // 126
	0x37, 0x64, 0x00,                                                   // 127
	0x37, 0x67, 0x04,                                                   // 128
	0x37, 0x68, 0x04,                                                   // 129
	0x37, 0x69, 0x08,                                                   // 130
	0x37, 0x6a, 0x08,                                                   // 131
	0x37, 0x6b, 0x20,                                                   // 132
	0x37, 0x6c, 0x00,                                                   // 133
	0x37, 0x6d, 0x00,                                                   // 134
	0x37, 0x6e, 0x00,                                                   // 135
	0x37, 0x73, 0x00,                                                   // 136
	0x37, 0x74, 0x51,                                                   // 137
	0x37, 0x76, 0xbd,                                                   // 138
	0x37, 0x77, 0xbd,                                                   // 139
	0x37, 0x81, 0x18,                                                   // 140
	0x37, 0x83, 0x25,                                                   // 141
	0x37, 0x98, 0x1b,                                                   // 142
	0x38, 0x00, 0x01,                                                   // 143
	0x38, 0x01, 0x88,                                                   // 144
	0x38, 0x02, 0x00,                                                   // 145
	0x38, 0x03, 0xdc/*0xe0*/,                                           // 146
	0x38, 0x04, 0x09,                                                   // 147
	0x38, 0x05, 0x17,                                                   // 148
	0x38, 0x06, 0x05,                                                   // 149
	0x38, 0x07, 0x23/*0x1f*/,                                           // 150
	0x38, 0x08, 0x07,                                                   // 151
	0x38, 0x09, 0x80,                                                   // 152
#if defined(ENABLE_1088P)
	0x38, 0x0a, 0x04,                                                   // 153
	0x38, 0x0b, 0x40,                                                   // 154
#elif defined(ENABLE_1080P)
	0x38, 0x0a, 0x04,                                                   // 153
	0x38, 0x0b, 0x38,                                                   // 154
#else
    0x38, 0x0a, 0x04,                                                   // 153
    0x38, 0x0b, 0x38,                                                   // 154
#endif //#if defined(ENABLE_1080P)
                                                                        //
	0x38, 0x0c, 0x0d, // 30fps                                          // 155
	0x38, 0x0d, 0xf0,                                                   // 156
                                                                        //
	0x38, 0x0e, 0x04,                                                   // 157
	0x38, 0x0f, 0x64/*0x8A*/,                                           // 158
	0x38, 0x10, 0x00,                                                   // 159
	0x38, 0x11, 0x08,                                                   // 160
	0x38, 0x12, 0x00,                                                   // 161
	0x38, 0x13, 0x04,                                                   // 162
	0x38, 0x14, 0x01,                                                   // 163
	0x38, 0x15, 0x01,                                                   // 164
	0x38, 0x19, 0x01,                                                   // 165
#ifndef OV4689_FLIP_MIRROR_FRIST                                        //
	0x38, 0x20, 0x00,                                                   // 166
	0x38, 0x21, 0x06,                                                   // 167
#else                                                                   //
	0x38, 0x20, 0x06,                                                   // 166
	0x38, 0x21, 0x00,                                                   // 167
#endif //OV4689_FLIP_MIRROR_FRIST                                       //
	0x38, 0x29, 0x00,                                                   // 168
	0x38, 0x2a, 0x01,                                                   // 169
	0x38, 0x2b, 0x01,                                                   // 170
	0x38, 0x2d, 0x7f,                                                   // 171
	0x38, 0x30, 0x04,                                                   // 172
	0x38, 0x36, 0x01,                                                   // 173
	0x38, 0x37, 0x00,                                                   // 174
	0x38, 0x41, 0x02,                                                   // 175
	0x38, 0x46, 0x08,                                                   // 176
	0x38, 0x47, 0x07,                                                   // 177
	0x3d, 0x85, 0x36,                                                   // 178
	0x3d, 0x8c, 0x71,                                                   // 179
	0x3d, 0x8d, 0xcb,                                                   // 180
	0x3f, 0x0a, 0x00,                                                   // 181
	0x40, 0x00, 0xf1,                                                   // 182
	0x40, 0x01, 0x40,                                                   // 183
	0x40, 0x02, 0x04,                                                   // 184
	0x40, 0x03, 0x14,                                                   // 185
	0x40, 0x0e, 0x00,                                                   // 186
	0x40, 0x11, 0x00,                                                   // 187
	0x40, 0x1a, 0x00,                                                   // 188
	0x40, 0x1b, 0x00,                                                   // 189
	0x40, 0x1c, 0x00,                                                   // 190
	0x40, 0x1d, 0x00,                                                   // 191
	0x40, 0x1f, 0x00,                                                   // 192
	0x40, 0x20, 0x00,                                                   // 193
	0x40, 0x21, 0x10,                                                   // 194
	0x40, 0x22, 0x06,                                                   // 195
	0x40, 0x23, 0x13,                                                   // 196
	0x40, 0x24, 0x07,                                                   // 197
	0x40, 0x25, 0x40,                                                   // 198
	0x40, 0x26, 0x07,                                                   // 199
	0x40, 0x27, 0x50,                                                   // 200
	0x40, 0x28, 0x00,                                                   // 201
	0x40, 0x29, 0x02,                                                   // 202
	0x40, 0x2a, 0x06,                                                   // 203
	0x40, 0x2b, 0x04,                                                   // 204
	0x40, 0x2c, 0x02,                                                   // 205
	0x40, 0x2d, 0x02,                                                   // 206
	0x40, 0x2e, 0x0e,                                                   // 207
	0x40, 0x2f, 0x04,                                                   // 208
	0x43, 0x02, 0xff,                                                   // 209
	0x43, 0x03, 0xff,                                                   // 210
	0x43, 0x04, 0x00,                                                   // 211
	0x43, 0x05, 0x00,                                                   // 212
	0x43, 0x06, 0x00,                                                   // 213
	0x43, 0x08, 0x02,                                                   // 214
	0x45, 0x00, 0x6c,                                                   // 215
	0x45, 0x01, 0xc4,                                                   // 216
	0x45, 0x02, 0x40,                                                   // 217
	0x45, 0x03, 0x01,                                                   // 218
	0x46, 0x01, 0x77,                                                   // 219
	0x48, 0x00, 0x04, // timing                                         // 220
	0x48, 0x13, 0x08,                                                   // 221
	0x48, 0x1f, 0x40,                                                   // 222
	// HS-prepare//                                                     //
	//0x48, 0x26, 0x28,// - 0x10, // HS_PREPARE_MIN                     //
	//0x48, 0x27, 0x55,// + 0x10, // HS_PREPARE_MAX                     //
	//0x48, 0x31, 0x6C - 0x35, // UI_HS_PREPARE_MIN                     //
	///////////////                                                     //
	0x48, 0x29, 0x78,                                                   // 223
	0x48, 0x37, 0x10,                                                   // 224
	0x4b, 0x00, 0x2a,                                                   // 225
	0x4b, 0x0d, 0x00,                                                   // 226
	0x4d, 0x00, 0x04,                                                   // 227
	0x4d, 0x01, 0x42,                                                   // 228
	0x4d, 0x02, 0xd1,                                                   // 229
	0x4d, 0x03, 0x93,                                                   // 230
	0x4d, 0x04, 0xf5,                                                   // 231
	0x4d, 0x05, 0xc1,                                                   // 232
	0x50, 0x00, 0xf3,                                                   // 233
	0x50, 0x01, 0x11,                                                   // 234
	0x50, 0x04, 0x00,                                                   // 235
	0x50, 0x0a, 0x00,                                                   // 236
	0x50, 0x0b, 0x00,                                                   // 237
	0x50, 0x32, 0x00,                                                   // 238
	0x50, 0x40, 0x00,                                                   // 239
	0x50, 0x50, 0x0c,                                                   // 240
	0x55, 0x00, 0x00,                                                   // 241
	0x55, 0x01, 0x10,                                                   // 242
	0x55, 0x02, 0x01,                                                   // 243
	0x55, 0x03, 0x0f,                                                   // 244
	0x80, 0x00, 0x00,                                                   // 245
	0x80, 0x01, 0x00,                                                   // 246
	0x80, 0x02, 0x00,                                                   // 247
	0x80, 0x03, 0x00,                                                   // 248
	0x80, 0x04, 0x00,                                                   // 249
	0x80, 0x05, 0x00,                                                   // 250
	0x80, 0x06, 0x00,                                                   // 251
	0x80, 0x07, 0x00,                                                   // 252
	0x80, 0x08, 0x00,                                                   // 253
	0x36, 0x38, 0x00,                                                   // 254
	//0x01, 0x00, 0x01,
	//0x03, 0x00, 0x60,
	STOP_REG, STOP_REG, STOP_REG,
};

#endif //#if defined(ENABLE_1920_1088_30_4lane)
#if defined(ENABLE_1920_1088_60_2lane)
//// 2lane 60fps
//static STF_U8 g_u8ModeRegs_1920_1088_60_2lane[] = {
//	0x01, 0x03, 0x01,
//	0x36, 0x38, 0x00,
//	0x03, 0x00, 0x04/*0x02*/,
//	0x03, 0x02, 0x40/*0x32*/,
//	0x03, 0x03, 0x00,
//	0x03, 0x04, 0x03,
//
//	0x03, 0x05, 0x02,
//	0x03, 0x06, 0x02,
//
//	0x03, 0x0a, 0x00, // pll1 prediv
//	0x03, 0x0b, 0x00,
//	0x03, 0x0c, 0x00,
//	0x03, 0x0d, 0x1e,
//	0x03, 0x0e, 0x04,
//	0x03, 0x0f, 0x01,
//	0x03, 0x11, 0x00,
//	0x03, 0x12, 0x01,
//	0x03, 0x1e, 0x00,
//	0x30, 0x00, 0x20,
//	0x30, 0x02, 0x00,
//	0x30, 0x18, 0x32/*0x72*/,
//	0x30, 0x20, 0x93,
//	0x30, 0x21, 0x03,
//	0x30, 0x22, 0x01,
//	0x30, 0x31, 0x0a,
//	0x30, 0x3f, 0x0c,
//	0x33, 0x05, 0xf1,
//	0x33, 0x07, 0x04,
//	0x33, 0x09, 0x29,
//	0x35, 0x00, 0x00,
//	0x35, 0x01, 0x20,
//	0x35, 0x02, 0x00,
//	0x35, 0x03, 0x04,
//	0x35, 0x04, 0x00,
//	0x35, 0x05, 0x00,
//	0x35, 0x06, 0x00,
//	0x35, 0x07, 0x00,
//	0x35, 0x08, 0x00,
//	0x35, 0x09, 0x80,
//	0x35, 0x0a, 0x00,
//	0x35, 0x0b, 0x00,
//	0x35, 0x0c, 0x00,
//	0x35, 0x0d, 0x00,
//	0x35, 0x0e, 0x00,
//	0x35, 0x0f, 0x80,
//	0x35, 0x10, 0x00,
//	0x35, 0x11, 0x00,
//	0x35, 0x12, 0x00,
//	0x35, 0x13, 0x00,
//	0x35, 0x14, 0x00,
//	0x35, 0x15, 0x80,
//	0x35, 0x16, 0x00,
//	0x35, 0x17, 0x00,
//	0x35, 0x18, 0x00,
//	0x35, 0x19, 0x00,
//	0x35, 0x1a, 0x00,
//	0x35, 0x1b, 0x80,
//	0x35, 0x1c, 0x00,
//	0x35, 0x1d, 0x00,
//	0x35, 0x1e, 0x00,
//	0x35, 0x1f, 0x00,
//	0x35, 0x20, 0x00,
//	0x35, 0x21, 0x80,
//	0x35, 0x22, 0x08,
//	0x35, 0x24, 0x08,
//	0x35, 0x26, 0x08,
//	0x35, 0x28, 0x08,
//	0x35, 0x2a, 0x08,
//	0x36, 0x02, 0x00,
//	0x36, 0x03, 0x40,
//	0x36, 0x04, 0x02,
//	0x36, 0x05, 0x00,
//	0x36, 0x06, 0x00,
//	0x36, 0x07, 0x00,
//	0x36, 0x09, 0x12,
//	0x36, 0x0a, 0x40,
//	0x36, 0x0c, 0x08,
//	0x36, 0x0f, 0xe5,
//	0x36, 0x08, 0x8f,
//	0x36, 0x11, 0x00,
//	0x36, 0x13, 0xf7,
//	0x36, 0x16, 0x58,
//	0x36, 0x19, 0x99,
//	0x36, 0x1b, 0x60,
//	0x36, 0x1c, 0x7a,
//	0x36, 0x1e, 0x79,
//	0x36, 0x1f, 0x02,
//	0x36, 0x32, 0x00,
//	0x36, 0x33, 0x10,
//	0x36, 0x34, 0x10,
//	0x36, 0x35, 0x10,
//	0x36, 0x36, 0x15,
//	0x36, 0x46, 0x86,
//	0x36, 0x4a, 0x0b,
//	0x37, 0x00, 0x17,
//	0x37, 0x01, 0x22,
//	0x37, 0x03, 0x10,
//	0x37, 0x0a, 0x37,
//	0x37, 0x05, 0x00,
//	0x37, 0x06, 0x63,
//	0x37, 0x09, 0x3c,
//	0x37, 0x0b, 0x01,
//	0x37, 0x0c, 0x30,
//	0x37, 0x10, 0x24,
//	0x37, 0x11, 0x0c,
//	0x37, 0x16, 0x00,
//	0x37, 0x20, 0x28,
//	0x37, 0x29, 0x7b,
//	0x37, 0x2a, 0x84,
//	0x37, 0x2b, 0xbd,
//	0x37, 0x2c, 0xbc,
//	0x37, 0x2e, 0x52,
//	0x37, 0x3c, 0x0e,
//	0x37, 0x3e, 0x33,
//	0x37, 0x43, 0x10,
//	0x37, 0x44, 0x88,
//	0x37, 0x45, 0xc0,
//	0x37, 0x4a, 0x43,
//	0x37, 0x4c, 0x00,
//	0x37, 0x4e, 0x23,
//	0x37, 0x51, 0x7b,
//	0x37, 0x52, 0x84,
//	0x37, 0x53, 0xbd,
//	0x37, 0x54, 0xbc,
//	0x37, 0x56, 0x52,
//	0x37, 0x5c, 0x00,
//	0x37, 0x60, 0x00,
//	0x37, 0x61, 0x00,
//	0x37, 0x62, 0x00,
//	0x37, 0x63, 0x00,
//	0x37, 0x64, 0x00,
//	0x37, 0x67, 0x04,
//	0x37, 0x68, 0x04,
//	0x37, 0x69, 0x08,
//	0x37, 0x6a, 0x08,
//	0x37, 0x6b, 0x20,
//	0x37, 0x6c, 0x00,
//	0x37, 0x6d, 0x00,
//	0x37, 0x6e, 0x00,
//	0x37, 0x73, 0x00,
//	0x37, 0x74, 0x51,
//	0x37, 0x76, 0xbd,
//	0x37, 0x77, 0xbd,
//	0x37, 0x81, 0x18,
//	0x37, 0x83, 0x25,
//	0x37, 0x98, 0x1b,
//	0x38, 0x00, 0x01,
//	0x38, 0x01, 0x88,
//	0x38, 0x02, 0x00,
//	0x38, 0x03, 0xdc/*0xe0*/,
//	0x38, 0x04, 0x09,
//	0x38, 0x05, 0x17,
//	0x38, 0x06, 0x05,
//	0x38, 0x07, 0x23/*0x1f*/,
//	0x38, 0x08, 0x07,
//	0x38, 0x09, 0x80,
//	0x38, 0x0a, 0x04,
//	0x38, 0x0b, 0x40/*0x38*/,
//	0x38, 0x0c, 0x06/*0x03*/,
//	0x38, 0x0d, 0xf3/*0x5c*/,
//	0x38, 0x0e, 0x04,
//	0x38, 0x0f, 0x64,
//	0x38, 0x10, 0x00,
//	0x38, 0x11, 0x08,
//	0x38, 0x12, 0x00,
//	0x38, 0x13, 0x04,
//	0x38, 0x14, 0x01,
//	0x38, 0x15, 0x01,
//	0x38, 0x19, 0x01,
//	0x38, 0x20, 0x00,
//	0x38, 0x21, 0x06,
//	0x38, 0x29, 0x00,
//	0x38, 0x2a, 0x01,
//	0x38, 0x2b, 0x01,
//	0x38, 0x2d, 0x7f,
//	0x38, 0x30, 0x04,
//	0x38, 0x36, 0x01,
//	0x38, 0x37, 0x00,
//	0x38, 0x41, 0x02,
//	0x38, 0x46, 0x08,
//	0x38, 0x47, 0x07,
//	0x3d, 0x85, 0x36,
//	0x3d, 0x8c, 0x71,
//	0x3d, 0x8d, 0xcb,
//	0x3f, 0x0a, 0x00,
//	0x40, 0x00, 0xf1,
//	0x40, 0x01, 0x40,
//	0x40, 0x02, 0x04,
//	0x40, 0x03, 0x14,
//	0x40, 0x0e, 0x00,
//	0x40, 0x11, 0x00,
//	0x40, 0x1a, 0x00,
//	0x40, 0x1b, 0x00,
//	0x40, 0x1c, 0x00,
//	0x40, 0x1d, 0x00,
//	0x40, 0x1f, 0x00,
//	0x40, 0x20, 0x00,
//	0x40, 0x21, 0x10,
//	0x40, 0x22, 0x06,
//	0x40, 0x23, 0x13,
//	0x40, 0x24, 0x07,
//	0x40, 0x25, 0x40,
//	0x40, 0x26, 0x07,
//	0x40, 0x27, 0x50,
//	0x40, 0x28, 0x00,
//	0x40, 0x29, 0x02,
//	0x40, 0x2a, 0x06,
//	0x40, 0x2b, 0x04,
//	0x40, 0x2c, 0x02,
//	0x40, 0x2d, 0x02,
//	0x40, 0x2e, 0x0e,
//	0x40, 0x2f, 0x04,
//	0x43, 0x02, 0xff,
//	0x43, 0x03, 0xff,
//	0x43, 0x04, 0x00,
//	0x43, 0x05, 0x00,
//	0x43, 0x06, 0x00,
//	0x43, 0x08, 0x02,
//	0x45, 0x00, 0x6c,
//	0x45, 0x01, 0xc4,
//	0x45, 0x02, 0x40,
//	0x45, 0x03, 0x01,
//	0x46, 0x01, 0x77,
//	0x48, 0x00, 0x04,
//	0x48, 0x13, 0x08,
//	0x48, 0x1f, 0x40,
//	0x48, 0x29, 0x78,
//	0x48, 0x37, 0x1b,
//	0x4b, 0x00, 0x2a,
//	0x4b, 0x0d, 0x00,
//	0x4d, 0x00, 0x04,
//	0x4d, 0x01, 0x42,
//	0x4d, 0x02, 0xd1,
//	0x4d, 0x03, 0x93,
//	0x4d, 0x04, 0xf5,
//	0x4d, 0x05, 0xc1,
//	0x50, 0x00, 0xf3,
//	0x50, 0x01, 0x11,
//	0x50, 0x04, 0x00,
//	0x50, 0x0a, 0x00,
//	0x50, 0x0b, 0x00,
//	0x50, 0x32, 0x00,
//	0x50, 0x40, 0x00,
//	0x50, 0x50, 0x0c,
//	0x55, 0x00, 0x00,
//	0x55, 0x01, 0x10,
//	0x55, 0x02, 0x01,
//	0x55, 0x03, 0x0f,
//	0x80, 0x00, 0x00,
//	0x80, 0x01, 0x00,
//	0x80, 0x02, 0x00,
//	0x80, 0x03, 0x00,
//	0x80, 0x04, 0x00,
//	0x80, 0x05, 0x00,
//	0x80, 0x06, 0x00,
//	0x80, 0x07, 0x00,
//	0x80, 0x08, 0x00,
//	0x36, 0x38, 0x00,
//	//0x01, 0x00, 0x01,
//	STOP_REG, STOP_REG, STOP_REG,
//};
//
#endif //#if defined(ENABLE_1920_1088_60_2lane)
#if defined(ENABLE_1920_1088_30_2lane)
//// 2lane 30fps
//static STF_U8 g_u8ModeRegs_1920_1088_30_2lane[] = {
//	0x01, 0x03, 0x01,
//	0x36, 0x38, 0x00,
//	0x03, 0x00, 0x00,
//	0x03, 0x02, 0x2a, // 2_LANE
//	0x03, 0x03, 0x00, // ??? use 0x01 for test pll1_divdm 1/2
//	0x03, 0x04, 0x03,
//	0x03, 0x0b, 0x00,
//	0x03, 0x0d, 0x1e,
//	0x03, 0x0e, 0x04,
//	0x03, 0x0f, 0x01,
//	0x03, 0x12, 0x01,
//	0x03, 0x1e, 0x00,
//	0x30, 0x00, 0x20,
//	0x30, 0x02, 0x00,
//	0x30, 0x18, 0x32, // 2_LANE
//	0x30, 0x20, 0x93,
//	0x30, 0x21, 0x03,
//	0x30, 0x22, 0x01,
//	0x30, 0x31, 0x0a,
//	0x30, 0x3f, 0x0c,
//	0x33, 0x05, 0xf1,
//	0x33, 0x07, 0x04,
//	0x33, 0x09, 0x29,
//	0x35, 0x00, 0x00,
//	0x35, 0x01, 0x4c,
//	0x35, 0x02, 0x00,
//	0x35, 0x03, 0x04,
//	0x35, 0x04, 0x00,
//	0x35, 0x05, 0x00,
//	0x35, 0x06, 0x00,
//	0x35, 0x07, 0x00,
//	0x35, 0x08, 0x00,
//	0x35, 0x09, 0x80,
//	0x35, 0x0a, 0x00,
//	0x35, 0x0b, 0x00,
//	0x35, 0x0c, 0x00,
//	0x35, 0x0d, 0x00,
//	0x35, 0x0e, 0x00,
//	0x35, 0x0f, 0x80,
//	0x35, 0x10, 0x00,
//	0x35, 0x11, 0x00,
//	0x35, 0x12, 0x00,
//	0x35, 0x13, 0x00,
//	0x35, 0x14, 0x00,
//	0x35, 0x15, 0x80,
//	0x35, 0x16, 0x00,
//	0x35, 0x17, 0x00,
//	0x35, 0x18, 0x00,
//	0x35, 0x19, 0x00,
//	0x35, 0x1a, 0x00,
//	0x35, 0x1b, 0x80,
//	0x35, 0x1c, 0x00,
//	0x35, 0x1d, 0x00,
//	0x35, 0x1e, 0x00,
//	0x35, 0x1f, 0x00,
//	0x35, 0x20, 0x00,
//	0x35, 0x21, 0x80,
//	0x35, 0x22, 0x08,
//	0x35, 0x24, 0x08,
//	0x35, 0x26, 0x08,
//	0x35, 0x28, 0x08,
//	0x35, 0x2a, 0x08,
//	0x36, 0x02, 0x00,
//	0x36, 0x03, 0x40,
//	0x36, 0x04, 0x02,
//	0x36, 0x05, 0x00,
//	0x36, 0x06, 0x00,
//	0x36, 0x07, 0x00,
//	0x36, 0x09, 0x12,
//	0x36, 0x0a, 0x40,
//	0x36, 0x0c, 0x08,
//	0x36, 0x0f, 0xe5,
//	0x36, 0x08, 0x8f,
//	0x36, 0x11, 0x00,
//	0x36, 0x13, 0xf7,
//	0x36, 0x16, 0x58,
//	0x36, 0x19, 0x99,
//	0x36, 0x1b, 0x60,
//	0x36, 0x1c, 0x7a,
//	0x36, 0x1e, 0x79,
//	0x36, 0x1f, 0x02,
//	0x36, 0x32, 0x00,
//	0x36, 0x33, 0x10,
//	0x36, 0x34, 0x10,
//	0x36, 0x35, 0x10,
//	0x36, 0x36, 0x15,
//	0x36, 0x46, 0x86,
//	0x36, 0x4a, 0x0b,
//	0x37, 0x00, 0x17,
//	0x37, 0x01, 0x22,
//	0x37, 0x03, 0x10,
//	0x37, 0x0a, 0x37,
//	0x37, 0x05, 0x00,
//	0x37, 0x06, 0x63,
//	0x37, 0x09, 0x3c,
//	0x37, 0x0b, 0x01,
//	0x37, 0x0c, 0x30,
//	0x37, 0x10, 0x24,
//	0x37, 0x11, 0x0c,
//	0x37, 0x16, 0x00,
//	0x37, 0x20, 0x28,
//	0x37, 0x29, 0x7b,
//	0x37, 0x2a, 0x84,
//	0x37, 0x2b, 0xbd,
//	0x37, 0x2c, 0xbc,
//	0x37, 0x2e, 0x52,
//	0x37, 0x3c, 0x0e,
//	0x37, 0x3e, 0x33,
//	0x37, 0x43, 0x10,
//	0x37, 0x44, 0x88,
//	0x37, 0x45, 0xc0,
//	0x37, 0x4a, 0x43,
//	0x37, 0x4c, 0x00,
//	0x37, 0x4e, 0x23,
//	0x37, 0x51, 0x7b,
//	0x37, 0x52, 0x84,
//	0x37, 0x53, 0xbd,
//	0x37, 0x54, 0xbc,
//	0x37, 0x56, 0x52,
//	0x37, 0x5c, 0x00,
//	0x37, 0x60, 0x00,
//	0x37, 0x61, 0x00,
//	0x37, 0x62, 0x00,
//	0x37, 0x63, 0x00,
//	0x37, 0x64, 0x00,
//	0x37, 0x67, 0x04,
//	0x37, 0x68, 0x04,
//	0x37, 0x69, 0x08,
//	0x37, 0x6a, 0x08,
//	0x37, 0x6b, 0x20,
//	0x37, 0x6c, 0x00,
//	0x37, 0x6d, 0x00,
//	0x37, 0x6e, 0x00,
//	0x37, 0x73, 0x00,
//	0x37, 0x74, 0x51,
//	0x37, 0x76, 0xbd,
//	0x37, 0x77, 0xbd,
//	0x37, 0x81, 0x18,
//	0x37, 0x83, 0x25,
//	0x37, 0x98, 0x1b,
//	0x38, 0x00, 0x01,
//	0x38, 0x01, 0x88,
//	0x38, 0x02, 0x00,
//	0x38, 0x03, 0xdc/*0xe0*/,
//	0x38, 0x04, 0x09,
//	0x38, 0x05, 0x17,
//	0x38, 0x06, 0x05,
//	0x38, 0x07, 0x23/*0x1f*/,
//	0x38, 0x08, 0x07,
//	0x38, 0x09, 0x80,
//	0x38, 0x0a, 0x04,
//	0x38, 0x0b, 0x40/*0x38*/,
//	///////////////
//	0x38, 0x0c, 0x0c, // 30fps
//	0x38, 0x0d, 0xb4,
//	///////////////
//	0x38, 0x0e, 0x04,
//	0x38, 0x0f, 0x64/*0x8A*/,
//	0x38, 0x10, 0x00,
//	0x38, 0x11, 0x08,
//	0x38, 0x12, 0x00,
//	0x38, 0x13, 0x04,
//	0x38, 0x14, 0x01,
//	0x38, 0x15, 0x01,
//	0x38, 0x19, 0x01,
//#ifndef OV4689_FLIP_MIRROR_FRIST
//	0x38, 0x20, 0x00,
//	0x38, 0x21, 0x06,
//#else
//	0x38, 0x20, 0x06,
//	0x38, 0x21, 0x00,
//#endif //OV4689_FLIP_MIRROR_FRIST
//	0x38, 0x29, 0x00,
//	0x38, 0x2a, 0x01,
//	0x38, 0x2b, 0x01,
//	0x38, 0x2d, 0x7f,
//	0x38, 0x30, 0x04,
//	0x38, 0x36, 0x01,
//	0x38, 0x37, 0x00,
//	0x38, 0x41, 0x02,
//	0x38, 0x46, 0x08,
//	0x38, 0x47, 0x07,
//	0x3d, 0x85, 0x36,
//	0x3d, 0x8c, 0x71,
//	0x3d, 0x8d, 0xcb,
//	0x3f, 0x0a, 0x00,
//	0x40, 0x00, 0xf1,
//	0x40, 0x01, 0x40,
//	0x40, 0x02, 0x04,
//	0x40, 0x03, 0x14,
//	0x40, 0x0e, 0x00,
//	0x40, 0x11, 0x00,
//	0x40, 0x1a, 0x00,
//	0x40, 0x1b, 0x00,
//	0x40, 0x1c, 0x00,
//	0x40, 0x1d, 0x00,
//	0x40, 0x1f, 0x00,
//	0x40, 0x20, 0x00,
//	0x40, 0x21, 0x10,
//	0x40, 0x22, 0x06,
//	0x40, 0x23, 0x13,
//	0x40, 0x24, 0x07,
//	0x40, 0x25, 0x40,
//	0x40, 0x26, 0x07,
//	0x40, 0x27, 0x50,
//	0x40, 0x28, 0x00,
//	0x40, 0x29, 0x02,
//	0x40, 0x2a, 0x06,
//	0x40, 0x2b, 0x04,
//	0x40, 0x2c, 0x02,
//	0x40, 0x2d, 0x02,
//	0x40, 0x2e, 0x0e,
//	0x40, 0x2f, 0x04,
//	0x43, 0x02, 0xff,
//	0x43, 0x03, 0xff,
//	0x43, 0x04, 0x00,
//	0x43, 0x05, 0x00,
//	0x43, 0x06, 0x00,
//	0x43, 0x08, 0x02,
//	0x45, 0x00, 0x6c,
//	0x45, 0x01, 0xc4,
//	0x45, 0x02, 0x40,
//	0x45, 0x03, 0x01,
//	0x46, 0x01, 0x77,
//	0x48, 0x00, 0x04, // timing
//	0x48, 0x13, 0x08,
//	0x48, 0x1f, 0x40,
//	// HS-prepare//
//	//0x48, 0x26, 0x28,// - 0x10, // HS_PREPARE_MIN
//	//0x48, 0x27, 0x55,// + 0x10, // HS_PREPARE_MAX
//	//0x48, 0x31, 0x6C - 0x35, // UI_HS_PREPARE_MIN
//	///////////////
//	0x48, 0x29, 0x78,
//	0x48, 0x37, 0x10,
//	0x4b, 0x00, 0x2a,
//	0x4b, 0x0d, 0x00,
//	0x4d, 0x00, 0x04,
//	0x4d, 0x01, 0x42,
//	0x4d, 0x02, 0xd1,
//	0x4d, 0x03, 0x93,
//	0x4d, 0x04, 0xf5,
//	0x4d, 0x05, 0xc1,
//	0x50, 0x00, 0xf3,
//	0x50, 0x01, 0x11,
//	0x50, 0x04, 0x00,
//	0x50, 0x0a, 0x00,
//	0x50, 0x0b, 0x00,
//	0x50, 0x32, 0x00,
//	0x50, 0x40, 0x00,
//	0x50, 0x50, 0x0c,
//	0x55, 0x00, 0x00,
//	0x55, 0x01, 0x10,
//	0x55, 0x02, 0x01,
//	0x55, 0x03, 0x0f,
//	0x80, 0x00, 0x00,
//	0x80, 0x01, 0x00,
//	0x80, 0x02, 0x00,
//	0x80, 0x03, 0x00,
//	0x80, 0x04, 0x00,
//	0x80, 0x05, 0x00,
//	0x80, 0x06, 0x00,
//	0x80, 0x07, 0x00,
//	0x80, 0x08, 0x00,
//	0x36, 0x38, 0x00,
//	//0x01, 0x00, 0x01,
//	//0x03, 0x00, 0x60,
//	STOP_REG, STOP_REG, STOP_REG,
//};
//
#endif //#if defined(ENABLE_1920_1088_30_2lane)
#if defined(ENABLE_1920_1088_20_2lane)
//// 2lane 20fps
//static STF_U8 g_u8ModeRegs_1920_1088_20_2lane[] = {
//	0x01, 0x03, 0x01,
//	0x36, 0x38, 0x00,
//	0x03, 0x00, 0x00,
//	0x03, 0x02, 0x2a, // 2_LANE
//	0x03, 0x03, 0x00, // ??? use 0x01 for test pll1_divdm 1/2
//	0x03, 0x04, 0x03,
//	0x03, 0x0b, 0x00,
//	0x03, 0x0d, 0x1e,
//	0x03, 0x0e, 0x04,
//	0x03, 0x0f, 0x01,
//	0x03, 0x12, 0x01,
//	0x03, 0x1e, 0x00,
//	0x30, 0x00, 0x20,
//	0x30, 0x02, 0x00,
//	0x30, 0x18, 0x32, // 2_LANE
//	0x30, 0x20, 0x93,
//	0x30, 0x21, 0x03,
//	0x30, 0x22, 0x01,
//	0x30, 0x31, 0x0a,
//	0x30, 0x3f, 0x0c,
//	0x33, 0x05, 0xf1,
//	0x33, 0x07, 0x04,
//	0x33, 0x09, 0x29,
//	0x35, 0x00, 0x00,
//	0x35, 0x01, 0x4c,
//	0x35, 0x02, 0x00,
//	0x35, 0x03, 0x04,
//	0x35, 0x04, 0x00,
//	0x35, 0x05, 0x00,
//	0x35, 0x06, 0x00,
//	0x35, 0x07, 0x00,
//	0x35, 0x08, 0x00,
//	0x35, 0x09, 0x80,
//	0x35, 0x0a, 0x00,
//	0x35, 0x0b, 0x00,
//	0x35, 0x0c, 0x00,
//	0x35, 0x0d, 0x00,
//	0x35, 0x0e, 0x00,
//	0x35, 0x0f, 0x80,
//	0x35, 0x10, 0x00,
//	0x35, 0x11, 0x00,
//	0x35, 0x12, 0x00,
//	0x35, 0x13, 0x00,
//	0x35, 0x14, 0x00,
//	0x35, 0x15, 0x80,
//	0x35, 0x16, 0x00,
//	0x35, 0x17, 0x00,
//	0x35, 0x18, 0x00,
//	0x35, 0x19, 0x00,
//	0x35, 0x1a, 0x00,
//	0x35, 0x1b, 0x80,
//	0x35, 0x1c, 0x00,
//	0x35, 0x1d, 0x00,
//	0x35, 0x1e, 0x00,
//	0x35, 0x1f, 0x00,
//	0x35, 0x20, 0x00,
//	0x35, 0x21, 0x80,
//	0x35, 0x22, 0x08,
//	0x35, 0x24, 0x08,
//	0x35, 0x26, 0x08,
//	0x35, 0x28, 0x08,
//	0x35, 0x2a, 0x08,
//	0x36, 0x02, 0x00,
//	0x36, 0x03, 0x40,
//	0x36, 0x04, 0x02,
//	0x36, 0x05, 0x00,
//	0x36, 0x06, 0x00,
//	0x36, 0x07, 0x00,
//	0x36, 0x09, 0x12,
//	0x36, 0x0a, 0x40,
//	0x36, 0x0c, 0x08,
//	0x36, 0x0f, 0xe5,
//	0x36, 0x08, 0x8f,
//	0x36, 0x11, 0x00,
//	0x36, 0x13, 0xf7,
//	0x36, 0x16, 0x58,
//	0x36, 0x19, 0x99,
//	0x36, 0x1b, 0x60,
//	0x36, 0x1c, 0x7a,
//	0x36, 0x1e, 0x79,
//	0x36, 0x1f, 0x02,
//	0x36, 0x32, 0x00,
//	0x36, 0x33, 0x10,
//	0x36, 0x34, 0x10,
//	0x36, 0x35, 0x10,
//	0x36, 0x36, 0x15,
//	0x36, 0x46, 0x86,
//	0x36, 0x4a, 0x0b,
//	0x37, 0x00, 0x17,
//	0x37, 0x01, 0x22,
//	0x37, 0x03, 0x10,
//	0x37, 0x0a, 0x37,
//	0x37, 0x05, 0x00,
//	0x37, 0x06, 0x63,
//	0x37, 0x09, 0x3c,
//	0x37, 0x0b, 0x01,
//	0x37, 0x0c, 0x30,
//	0x37, 0x10, 0x24,
//	0x37, 0x11, 0x0c,
//	0x37, 0x16, 0x00,
//	0x37, 0x20, 0x28,
//	0x37, 0x29, 0x7b,
//	0x37, 0x2a, 0x84,
//	0x37, 0x2b, 0xbd,
//	0x37, 0x2c, 0xbc,
//	0x37, 0x2e, 0x52,
//	0x37, 0x3c, 0x0e,
//	0x37, 0x3e, 0x33,
//	0x37, 0x43, 0x10,
//	0x37, 0x44, 0x88,
//	0x37, 0x45, 0xc0,
//	0x37, 0x4a, 0x43,
//	0x37, 0x4c, 0x00,
//	0x37, 0x4e, 0x23,
//	0x37, 0x51, 0x7b,
//	0x37, 0x52, 0x84,
//	0x37, 0x53, 0xbd,
//	0x37, 0x54, 0xbc,
//	0x37, 0x56, 0x52,
//	0x37, 0x5c, 0x00,
//	0x37, 0x60, 0x00,
//	0x37, 0x61, 0x00,
//	0x37, 0x62, 0x00,
//	0x37, 0x63, 0x00,
//	0x37, 0x64, 0x00,
//	0x37, 0x67, 0x04,
//	0x37, 0x68, 0x04,
//	0x37, 0x69, 0x08,
//	0x37, 0x6a, 0x08,
//	0x37, 0x6b, 0x20,
//	0x37, 0x6c, 0x00,
//	0x37, 0x6d, 0x00,
//	0x37, 0x6e, 0x00,
//	0x37, 0x73, 0x00,
//	0x37, 0x74, 0x51,
//	0x37, 0x76, 0xbd,
//	0x37, 0x77, 0xbd,
//	0x37, 0x81, 0x18,
//	0x37, 0x83, 0x25,
//	0x37, 0x98, 0x1b,
//	0x38, 0x00, 0x01,
//	0x38, 0x01, 0x88,
//	0x38, 0x02, 0x00,
//	0x38, 0x03, 0xdc/*0xe0*/,
//	0x38, 0x04, 0x09,
//	0x38, 0x05, 0x17,
//	0x38, 0x06, 0x05,
//	0x38, 0x07, 0x23/*0x1f*/,
//	0x38, 0x08, 0x07,
//	0x38, 0x09, 0x80,
//	0x38, 0x0a, 0x04,
//	0x38, 0x0b, 0x40/*0x38*/,
//	///////////////
//	0x38, 0x0c, 0x11, // 20fps
//	0x38, 0x0d, 0x00,
//	///////////////
//	0x38, 0x0e, 0x04,
//	0x38, 0x0f, 0x64/*0x8A*/,
//	0x38, 0x10, 0x00,
//	0x38, 0x11, 0x08,
//	0x38, 0x12, 0x00,
//	0x38, 0x13, 0x04,
//	0x38, 0x14, 0x01,
//	0x38, 0x15, 0x01,
//	0x38, 0x19, 0x01,
//#ifndef OV4689_FLIP_MIRROR_FRIST
//	0x38, 0x20, 0x00,
//	0x38, 0x21, 0x06,
//#else
//	0x38, 0x20, 0x06,
//	0x38, 0x21, 0x00,
//#endif //OV4689_FLIP_MIRROR_FRIST
//	0x38, 0x29, 0x00,
//	0x38, 0x2a, 0x01,
//	0x38, 0x2b, 0x01,
//	0x38, 0x2d, 0x7f,
//	0x38, 0x30, 0x04,
//	0x38, 0x36, 0x01,
//	0x38, 0x37, 0x00,
//	0x38, 0x41, 0x02,
//	0x38, 0x46, 0x08,
//	0x38, 0x47, 0x07,
//	0x3d, 0x85, 0x36,
//	0x3d, 0x8c, 0x71,
//	0x3d, 0x8d, 0xcb,
//	0x3f, 0x0a, 0x00,
//	0x40, 0x00, 0xf1,
//	0x40, 0x01, 0x40,
//	0x40, 0x02, 0x04,
//	0x40, 0x03, 0x14,
//	0x40, 0x0e, 0x00,
//	0x40, 0x11, 0x00,
//	0x40, 0x1a, 0x00,
//	0x40, 0x1b, 0x00,
//	0x40, 0x1c, 0x00,
//	0x40, 0x1d, 0x00,
//	0x40, 0x1f, 0x00,
//	0x40, 0x20, 0x00,
//	0x40, 0x21, 0x10,
//	0x40, 0x22, 0x06,
//	0x40, 0x23, 0x13,
//	0x40, 0x24, 0x07,
//	0x40, 0x25, 0x40,
//	0x40, 0x26, 0x07,
//	0x40, 0x27, 0x50,
//	0x40, 0x28, 0x00,
//	0x40, 0x29, 0x02,
//	0x40, 0x2a, 0x06,
//	0x40, 0x2b, 0x04,
//	0x40, 0x2c, 0x02,
//	0x40, 0x2d, 0x02,
//	0x40, 0x2e, 0x0e,
//	0x40, 0x2f, 0x04,
//	0x43, 0x02, 0xff,
//	0x43, 0x03, 0xff,
//	0x43, 0x04, 0x00,
//	0x43, 0x05, 0x00,
//	0x43, 0x06, 0x00,
//	0x43, 0x08, 0x02,
//	0x45, 0x00, 0x6c,
//	0x45, 0x01, 0xc4,
//	0x45, 0x02, 0x40,
//	0x45, 0x03, 0x01,
//	0x46, 0x01, 0x77,
//	0x48, 0x00, 0x04, // timing
//	0x48, 0x13, 0x08,
//	0x48, 0x1f, 0x40,
//	// HS-prepare//
//	//0x48, 0x26, 0x28,// - 0x10, // HS_PREPARE_MIN
//	//0x48, 0x27, 0x55,// + 0x10, // HS_PREPARE_MAX
//	//0x48, 0x31, 0x6C - 0x35, // UI_HS_PREPARE_MIN
//	///////////////
//	0x48, 0x29, 0x78,
//	0x48, 0x37, 0x10,
//	0x4b, 0x00, 0x2a,
//	0x4b, 0x0d, 0x00,
//	0x4d, 0x00, 0x04,
//	0x4d, 0x01, 0x42,
//	0x4d, 0x02, 0xd1,
//	0x4d, 0x03, 0x93,
//	0x4d, 0x04, 0xf5,
//	0x4d, 0x05, 0xc1,
//	0x50, 0x00, 0xf3,
//	0x50, 0x01, 0x11,
//	0x50, 0x04, 0x00,
//	0x50, 0x0a, 0x00,
//	0x50, 0x0b, 0x00,
//	0x50, 0x32, 0x00,
//	0x50, 0x40, 0x00,
//	0x50, 0x50, 0x0c,
//	0x55, 0x00, 0x00,
//	0x55, 0x01, 0x10,
//	0x55, 0x02, 0x01,
//	0x55, 0x03, 0x0f,
//	0x80, 0x00, 0x00,
//	0x80, 0x01, 0x00,
//	0x80, 0x02, 0x00,
//	0x80, 0x03, 0x00,
//	0x80, 0x04, 0x00,
//	0x80, 0x05, 0x00,
//	0x80, 0x06, 0x00,
//	0x80, 0x07, 0x00,
//	0x80, 0x08, 0x00,
//	0x36, 0x38, 0x00,
//	//0x01, 0x00, 0x01,
//	//0x03, 0x00, 0x60,
//	STOP_REG, STOP_REG, STOP_REG,
//};
//
#endif //#if defined(ENABLE_1920_1088_20_2lane)
#if defined(ENABLE_1920_1088_30_1lane)
//// 1lane 30fps
//static STF_U8 g_u8ModeRegs_1920_1088_30_1lane[] = {
//	0x01, 0x03, 0x01,
//	0x36, 0x38, 0x00,
//	0x03, 0x00, 0x00,
//	0x03, 0x02, 0x2a, // 1_LANE
//	0x03, 0x03, 0x00, // ??? use 0x01 for test pll1_divdm 1/2
//	0x03, 0x04, 0x03,
//	0x03, 0x0b, 0x00,
//	0x03, 0x0d, 0x1e,
//	0x03, 0x0e, 0x04,
//	0x03, 0x0f, 0x01,
//	0x03, 0x11, 0x00, // pll2 prediv
//	0x03, 0x12, 0x01,
//	0x03, 0x1e, 0x00,
//	0x30, 0x00, 0x20,
//	0x30, 0x02, 0x00,
//	0x30, 0x18, 0x12, // 1_LANE
//	0x30, 0x20, 0x93,
//	0x30, 0x21, 0x03,
//	0x30, 0x22, 0x01,
//	0x30, 0x31, 0x0a,
//	0x30, 0x3f, 0x0c,
//	0x33, 0x05, 0xf1,
//	0x33, 0x07, 0x04,
//	0x33, 0x09, 0x29,
//	0x35, 0x00, 0x00,
//	0x35, 0x01, 0x4c,
//	0x35, 0x02, 0x00,
//	0x35, 0x03, 0x04,
//	0x35, 0x04, 0x00,
//	0x35, 0x05, 0x00,
//	0x35, 0x06, 0x00,
//	0x35, 0x07, 0x00,
//	0x35, 0x08, 0x00,
//	0x35, 0x09, 0x80,
//	0x35, 0x0a, 0x00,
//	0x35, 0x0b, 0x00,
//	0x35, 0x0c, 0x00,
//	0x35, 0x0d, 0x00,
//	0x35, 0x0e, 0x00,
//	0x35, 0x0f, 0x80,
//	0x35, 0x10, 0x00,
//	0x35, 0x11, 0x00,
//	0x35, 0x12, 0x00,
//	0x35, 0x13, 0x00,
//	0x35, 0x14, 0x00,
//	0x35, 0x15, 0x80,
//	0x35, 0x16, 0x00,
//	0x35, 0x17, 0x00,
//	0x35, 0x18, 0x00,
//	0x35, 0x19, 0x00,
//	0x35, 0x1a, 0x00,
//	0x35, 0x1b, 0x80,
//	0x35, 0x1c, 0x00,
//	0x35, 0x1d, 0x00,
//	0x35, 0x1e, 0x00,
//	0x35, 0x1f, 0x00,
//	0x35, 0x20, 0x00,
//	0x35, 0x21, 0x80,
//	0x35, 0x22, 0x08,
//	0x35, 0x24, 0x08,
//	0x35, 0x26, 0x08,
//	0x35, 0x28, 0x08,
//	0x35, 0x2a, 0x08,
//	0x36, 0x02, 0x00,
//	0x36, 0x03, 0x40,
//	0x36, 0x04, 0x02,
//	0x36, 0x05, 0x00,
//	0x36, 0x06, 0x00,
//	0x36, 0x07, 0x00,
//	0x36, 0x09, 0x12,
//	0x36, 0x0a, 0x40,
//	0x36, 0x0c, 0x08,
//	0x36, 0x0f, 0xe5,
//	0x36, 0x08, 0x8f,
//	0x36, 0x11, 0x00,
//	0x36, 0x13, 0xf7,
//	0x36, 0x16, 0x58,
//	0x36, 0x19, 0x99,
//	0x36, 0x1b, 0x60,
//	0x36, 0x1c, 0x7a,
//	0x36, 0x1e, 0x79,
//	0x36, 0x1f, 0x02,
//	0x36, 0x32, 0x00,
//	0x36, 0x33, 0x10,
//	0x36, 0x34, 0x10,
//	0x36, 0x35, 0x10,
//	0x36, 0x36, 0x15,
//	0x36, 0x46, 0x86,
//	0x36, 0x4a, 0x0b,
//	0x37, 0x00, 0x17,
//	0x37, 0x01, 0x22,
//	0x37, 0x03, 0x10,
//	0x37, 0x0a, 0x37,
//	0x37, 0x05, 0x00,
//	0x37, 0x06, 0x63,
//	0x37, 0x09, 0x3c,
//	0x37, 0x0b, 0x01,
//	0x37, 0x0c, 0x30,
//	0x37, 0x10, 0x24,
//	0x37, 0x11, 0x0c,
//	0x37, 0x16, 0x00,
//	0x37, 0x20, 0x28,
//	0x37, 0x29, 0x7b,
//	0x37, 0x2a, 0x84,
//	0x37, 0x2b, 0xbd,
//	0x37, 0x2c, 0xbc,
//	0x37, 0x2e, 0x52,
//	0x37, 0x3c, 0x0e,
//	0x37, 0x3e, 0x33,
//	0x37, 0x43, 0x10,
//	0x37, 0x44, 0x88,
//	0x37, 0x45, 0xc0,
//	0x37, 0x4a, 0x43,
//	0x37, 0x4c, 0x00,
//	0x37, 0x4e, 0x23,
//	0x37, 0x51, 0x7b,
//	0x37, 0x52, 0x84,
//	0x37, 0x53, 0xbd,
//	0x37, 0x54, 0xbc,
//	0x37, 0x56, 0x52,
//	0x37, 0x5c, 0x00,
//	0x37, 0x60, 0x00,
//	0x37, 0x61, 0x00,
//	0x37, 0x62, 0x00,
//	0x37, 0x63, 0x00,
//	0x37, 0x64, 0x00,
//	0x37, 0x67, 0x04,
//	0x37, 0x68, 0x04,
//	0x37, 0x69, 0x08,
//	0x37, 0x6a, 0x08,
//	0x37, 0x6b, 0x20,
//	0x37, 0x6c, 0x00,
//	0x37, 0x6d, 0x00,
//	0x37, 0x6e, 0x00,
//	0x37, 0x73, 0x00,
//	0x37, 0x74, 0x51,
//	0x37, 0x76, 0xbd,
//	0x37, 0x77, 0xbd,
//	0x37, 0x81, 0x18,
//	0x37, 0x83, 0x25,
//	0x37, 0x98, 0x1b,
//	0x38, 0x00, 0x01,
//	0x38, 0x01, 0x88,
//	0x38, 0x02, 0x00,
//	0x38, 0x03, 0xdc/*0xe0*/,
//	0x38, 0x04, 0x09,
//	0x38, 0x05, 0x17,
//	0x38, 0x06, 0x05,
//	0x38, 0x07, 0x23/*0x1f*/,
//	0x38, 0x08, 0x07,
//	0x38, 0x09, 0x80,
//	0x38, 0x0a, 0x04,
//	0x38, 0x0b, 0x40/*0x38*/,
//	///////////////
//	0x38, 0x0c, 0x0c, // 30fps
//	0x38, 0x0d, 0xb4,
//	///////////////
//	0x38, 0x0e, 0x04,
//	0x38, 0x0f, 0x64/*0x8A*/,
//	0x38, 0x10, 0x00,
//	0x38, 0x11, 0x08,
//	0x38, 0x12, 0x00,
//	0x38, 0x13, 0x04,
//	0x38, 0x14, 0x01,
//	0x38, 0x15, 0x01,
//	0x38, 0x19, 0x01,
//#ifndef OV4689_FLIP_MIRROR_FRIST
//	0x38, 0x20, 0x00,
//	0x38, 0x21, 0x06,
//#else
//	0x38, 0x20, 0x06,
//	0x38, 0x21, 0x00,
//#endif
//	0x38, 0x29, 0x00,
//	0x38, 0x2a, 0x01,
//	0x38, 0x2b, 0x01,
//	0x38, 0x2d, 0x7f,
//	0x38, 0x30, 0x04,
//	0x38, 0x36, 0x01,
//	0x38, 0x37, 0x00,
//	0x38, 0x41, 0x02,
//	0x38, 0x46, 0x08,
//	0x38, 0x47, 0x07,
//	0x3d, 0x85, 0x36,
//	0x3d, 0x8c, 0x71,
//	0x3d, 0x8d, 0xcb,
//	0x3f, 0x0a, 0x00,
//	0x40, 0x00, 0xf1,
//	0x40, 0x01, 0x40,
//	0x40, 0x02, 0x04,
//	0x40, 0x03, 0x14,
//	0x40, 0x0e, 0x00,
//	0x40, 0x11, 0x00,
//	0x40, 0x1a, 0x00,
//	0x40, 0x1b, 0x00,
//	0x40, 0x1c, 0x00,
//	0x40, 0x1d, 0x00,
//	0x40, 0x1f, 0x00,
//	0x40, 0x20, 0x00,
//	0x40, 0x21, 0x10,
//	0x40, 0x22, 0x06,
//	0x40, 0x23, 0x13,
//	0x40, 0x24, 0x07,
//	0x40, 0x25, 0x40,
//	0x40, 0x26, 0x07,
//	0x40, 0x27, 0x50,
//	0x40, 0x28, 0x00,
//	0x40, 0x29, 0x02,
//	0x40, 0x2a, 0x06,
//	0x40, 0x2b, 0x04,
//	0x40, 0x2c, 0x02,
//	0x40, 0x2d, 0x02,
//	0x40, 0x2e, 0x0e,
//	0x40, 0x2f, 0x04,
//	0x43, 0x02, 0xff,
//	0x43, 0x03, 0xff,
//	0x43, 0x04, 0x00,
//	0x43, 0x05, 0x00,
//	0x43, 0x06, 0x00,
//	0x43, 0x08, 0x02,
//	0x45, 0x00, 0x6c,
//	0x45, 0x01, 0xc4,
//	0x45, 0x02, 0x40,
//	0x45, 0x03, 0x01,
//	0x46, 0x01, 0x77,
//	0x48, 0x00, 0x04, // timing
//	0x48, 0x13, 0x08,
//	0x48, 0x1f, 0x40,
//	// HS-prepare//
//	//0x48, 0x26, 0x28, // - 0x10,  // HS_PREPARE_MIN
//	//0x48, 0x27, 0x55, // + 0x10,  // HS_PREPARE_MAX
//	//0x48, 0x31, 0x6C - 0x35,  // UI_HS_PREPARE_MIN
//	///////////////
//	0x48, 0x29, 0x78,
//	0x48, 0x37, 0x10,
//	0x4b, 0x00, 0x2a,
//	0x4b, 0x0d, 0x00,
//	0x4d, 0x00, 0x04,
//	0x4d, 0x01, 0x42,
//	0x4d, 0x02, 0xd1,
//	0x4d, 0x03, 0x93,
//	0x4d, 0x04, 0xf5,
//	0x4d, 0x05, 0xc1,
//	0x50, 0x00, 0xf3,
//	0x50, 0x01, 0x11,
//	0x50, 0x04, 0x00,
//	0x50, 0x0a, 0x00,
//	0x50, 0x0b, 0x00,
//	0x50, 0x32, 0x00,
//	0x50, 0x40, 0x00,
//	0x50, 0x50, 0x0c,
//	0x55, 0x00, 0x00,
//	0x55, 0x01, 0x10,
//	0x55, 0x02, 0x01,
//	0x55, 0x03, 0x0f,
//	0x80, 0x00, 0x00,
//	0x80, 0x01, 0x00,
//	0x80, 0x02, 0x00,
//	0x80, 0x03, 0x00,
//	0x80, 0x04, 0x00,
//	0x80, 0x05, 0x00,
//	0x80, 0x06, 0x00,
//	0x80, 0x07, 0x00,
//	0x80, 0x08, 0x00,
//	0x36, 0x38, 0x00,
//	//0x01, 0x00, 0x01,
//	//0x03, 0x00, 0x60,
//	STOP_REG, STOP_REG, STOP_REG,
//};
//
#endif //#if defined(ENABLE_1920_1088_30_1lane)
#if defined(ENABLE_2624_1520_30_2lane)
//static STF_U8 g_u8ModeRegs_2624_1520_30_2lane[] = {
//    0x01, 0x03, 0x01,
//    0x36, 0x38, 0x00,
//    0x03, 0x00, 0x00, //0x01 eric
//    0x03, 0x01, 0x00,
//    0x03, 0x02, 0x23, //0x28 eric
//    0x03, 0x03, 0x00,
//    0x03, 0x04, 0x03,
//    0x03, 0x05, 0x01,
//    0x03, 0x06, 0x01,
//    0x03, 0x0A, 0x00,
//    0x03, 0x0B, 0x00,
//    0x03, 0x0C, 0x00,
//    0x03, 0x0D, 0x1E,
//    0x03, 0x0E, 0x04,
//    0x03, 0x0F, 0x01,
//    0x03, 0x11, 0x00,
//    0x03, 0x12, 0x01,
//    0x03, 0x1e, 0x00,
//    0x30, 0x00, 0x20,
//    0x30, 0x02, 0x00,
//    0x30, 0x18, 0x32,
//    0x30, 0x19, 0x0C,
//    0x30, 0x20, 0x93,
//    0x30, 0x21, 0x03,
//    0x30, 0x22, 0x01,
//    0x30, 0x31, 0x0A,
//    0x30, 0x3f, 0x0c,
//    0x33, 0x05, 0xf1,
//    0x33, 0x07, 0x04,
//    0x33, 0x09, 0x29,
//    0x35, 0x00, 0x00,
//    0x35, 0x01, 0x7C,
//    0x35, 0x02, 0xC0,
//    0x35, 0x03, 0x04,
//    0x35, 0x04, 0x00,
//    0x35, 0x05, 0x00,
//    0x35, 0x06, 0x00,
//    0x35, 0x07, 0x00,
//    0x35, 0x08, 0x00,
//    0x35, 0x09, 0x80,
//    0x35, 0x0a, 0x00,
//    0x35, 0x0b, 0x00,
//    0x35, 0x0c, 0x00,
//    0x35, 0x0d, 0x00,
//    0x35, 0x0e, 0x00,
//    0x35, 0x0f, 0x80,
//    0x35, 0x10, 0x00,
//    0x35, 0x11, 0x00,
//    0x35, 0x12, 0x00,
//    0x35, 0x13, 0x00,
//    0x35, 0x14, 0x00,
//    0x35, 0x15, 0x80,
//    0x35, 0x16, 0x00,
//    0x35, 0x17, 0x00,
//    0x35, 0x18, 0x00,
//    0x35, 0x19, 0x00,
//    0x35, 0x1a, 0x00,
//    0x35, 0x1b, 0x80,
//    0x35, 0x1c, 0x00,
//    0x35, 0x1d, 0x00,
//    0x35, 0x1e, 0x00,
//    0x35, 0x1f, 0x00,
//    0x35, 0x20, 0x00,
//    0x35, 0x21, 0x80,
//    0x35, 0x22, 0x08,
//    0x35, 0x24, 0x08,
//    0x35, 0x26, 0x08,
//    0x35, 0x28, 0x08,
//    0x35, 0x2a, 0x08,
//    0x36, 0x02, 0x00,
//    0x36, 0x03, 0x40,
//    0x36, 0x04, 0x02,
//    0x36, 0x05, 0x00,
//    0x36, 0x06, 0x00,
//    0x36, 0x07, 0x00,
//    0x36, 0x09, 0x12,
//    0x36, 0x0a, 0x40,
//    0x36, 0x0c, 0x08,
//    0x36, 0x0f, 0xe5,
//    0x36, 0x08, 0x8f,
//    0x36, 0x11, 0x00,
//    0x36, 0x13, 0xf7,
//    0x36, 0x16, 0x58,
//    0x36, 0x19, 0x99,
//    0x36, 0x1b, 0x60,
//    0x36, 0x1c, 0x7a,
//    0x36, 0x1e, 0x79,
//    0x36, 0x1f, 0x02,
//    0x36, 0x32, 0x00,
//    0x36, 0x33, 0x10,
//    0x36, 0x34, 0x10,
//    0x36, 0x35, 0x10,
//    0x36, 0x36, 0x15,
//    0x36, 0x46, 0x86,
//    0x36, 0x4a, 0x0b,
//    0x37, 0x00, 0x17,
//    0x37, 0x01, 0x22,
//    0x37, 0x03, 0x10,
//    0x37, 0x0a, 0x37,
//    0x37, 0x05, 0x00,
//    0x37, 0x06, 0x63,
//    0x37, 0x09, 0x3c,
//    0x37, 0x0b, 0x01,
//    0x37, 0x0c, 0x30,
//    0x37, 0x10, 0x24,
//    0x37, 0x11, 0x0c,
//    0x37, 0x16, 0x00,
//    0x37, 0x20, 0x28,
//    0x37, 0x29, 0x7b,
//    0x37, 0x2a, 0x84,
//    0x37, 0x2b, 0xbd,
//    0x37, 0x2c, 0xbc,
//    0x37, 0x2e, 0x52,
//    0x37, 0x3c, 0x0e,
//    0x37, 0x3e, 0x33,
//    0x37, 0x43, 0x10,
//    0x37, 0x44, 0x88,
//    0x37, 0x45, 0xc0,
//    0x37, 0x4a, 0x43,
//    0x37, 0x4c, 0x00,
//    0x37, 0x4e, 0x23,
//    0x37, 0x51, 0x7b,
//    0x37, 0x52, 0x84,
//    0x37, 0x53, 0xbd,
//    0x37, 0x54, 0xbc,
//    0x37, 0x56, 0x52,
//    0x37, 0x5c, 0x00,
//    0x37, 0x60, 0x00,
//    0x37, 0x61, 0x00,
//    0x37, 0x62, 0x00,
//    0x37, 0x63, 0x00,
//    0x37, 0x64, 0x00,
//    0x37, 0x67, 0x04,
//    0x37, 0x68, 0x04,
//    0x37, 0x69, 0x08,
//    0x37, 0x6a, 0x08,
//    0x37, 0x6b, 0x20,
//    0x37, 0x6c, 0x00,
//    0x37, 0x6d, 0x00,
//    0x37, 0x6e, 0x00,
//    0x37, 0x73, 0x00,
//    0x37, 0x74, 0x51,
//    0x37, 0x76, 0xbd,
//    0x37, 0x77, 0xbd,
//    0x37, 0x81, 0x18,
//    0x37, 0x83, 0x25,
//    0x37, 0x98, 0x1b,
//    0x38, 0x00, 0x00,
//    0x38, 0x01, 0x28,
//    0x38, 0x02, 0x00,
//    0x38, 0x03, 0x04,
//    0x38, 0x04, 0x0A,
//    0x38, 0x05, 0x77,
//    0x38, 0x06, 0x05,
//    0x38, 0x07, 0xFB,
//    0x38, 0x08, 0x0A,
//    0x38, 0x09, 0x40, //0x40  eric
//    0x38, 0x0A, 0x05,
//    0x38, 0x0B, 0xF0,
//    0x38, 0x0C, 0x07, //0x0b eric
//    0x38, 0x0D, 0xd0, //0xb8 eric
//    0x38, 0x0E, 0x07, //0x07 eric
//    0x38, 0x0F, 0xd0, //0xd0 eric
//    0x38, 0x10, 0x00,
//    0x38, 0x11, 0x08,
//    0x38, 0x12, 0x00,
//    0x38, 0x13, 0x04,
//    0x38, 0x14, 0x01,
//    0x38, 0x15, 0x01,
//    0x38, 0x19, 0x01,
//    0x38, 0x20, 0x00,
//    0x38, 0x21, 0x06,
//    0x38, 0x23, 0x00,
//    0x38, 0x24, 0x00,
//    0x38, 0x25, 0x20,
//    0x38, 0x26, 0x00,
//    0x38, 0x27, 0x04,
//    0x38, 0x29, 0x00,
//    0x38, 0x2a, 0x01,
//    0x38, 0x2b, 0x01,
//    0x38, 0x2d, 0x7f,
//    0x38, 0x30, 0x04,
//    0x38, 0x36, 0x01,
//    0x38, 0x37, 0x00,
//    0x38, 0x41, 0x02,
//    0x38, 0x46, 0x08,
//    0x38, 0x47, 0x07,
//    0x3d, 0x85, 0x36,
//    0x3d, 0x8c, 0x71,
//    0x3d, 0x8d, 0xcb,
//    0x3f, 0x0a, 0x00,
//    0x40, 0x00, 0xf1,
//    0x40, 0x01, 0x40,
//    0x40, 0x02, 0x04,
//    0x40, 0x03, 0x14,
//    0x40, 0x0e, 0x00,
//    0x40, 0x11, 0x00,
//    0x40, 0x1a, 0x00,
//    0x40, 0x1b, 0x00,
//    0x40, 0x1c, 0x00,
//    0x40, 0x1d, 0x00,
//    0x40, 0x1f, 0x00,
//    0x40, 0x20, 0x00,
//    0x40, 0x21, 0x10,
//    0x40, 0x22, 0x08,
//    0x40, 0x23, 0xd3,
//    0x40, 0x24, 0x0a,
//    0x40, 0x25, 0x00,
//    0x40, 0x26, 0x0a,
//    0x40, 0x27, 0x10,
//    0x40, 0x28, 0x00,
//    0x40, 0x29, 0x02,
//    0x40, 0x2a, 0x06,
//    0x40, 0x2b, 0x04,
//    0x40, 0x2c, 0x02,
//    0x40, 0x2d, 0x02,
//    0x40, 0x2e, 0x0e,
//    0x40, 0x2f, 0x04,
//    0x43, 0x02, 0xff,
//    0x43, 0x03, 0xff,
//    0x43, 0x04, 0x00,
//    0x43, 0x05, 0x00,
//    0x43, 0x06, 0x00,
//    0x43, 0x08, 0x02,
//    0x45, 0x00, 0x6c,
//    0x45, 0x01, 0xc4,
//    0x45, 0x02, 0x40,
//    0x45, 0x03, 0x01,
//    0x46, 0x00, 0x00,
//    0x46, 0x01, 0xA3,
//    0x48, 0x00, 0x04,
//    0x48, 0x13, 0x08,
//    0x48, 0x1f, 0x40,
//    0x48, 0x29, 0x78,
//    0x48, 0x37, 0x14, //0x1A eric
//    0x4b, 0x00, 0x2a,
//    0x4b, 0x0d, 0x00,
//    0x4d, 0x00, 0x04,
//    0x4d, 0x01, 0x42,
//    0x4d, 0x02, 0xd1,
//    0x4d, 0x03, 0x93,
//    0x4d, 0x04, 0xf5,
//    0x4d, 0x05, 0xc1,
//    0x50, 0x00, 0xf3,
//    0x50, 0x01, 0x11,
//    0x50, 0x04, 0x00,
//    0x50, 0x0a, 0x00,
//    0x50, 0x0b, 0x00,
//    0x50, 0x32, 0x00,
//    0x50, 0x40, 0x00,
//    0x50, 0x50, 0x0c,
//    0x55, 0x00, 0x00,
//    0x55, 0x01, 0x10,
//    0x55, 0x02, 0x01,
//    0x55, 0x03, 0x0f,
//    0x80, 0x00, 0x00,
//    0x80, 0x01, 0x00,
//    0x80, 0x02, 0x00,
//    0x80, 0x03, 0x00,
//    0x80, 0x04, 0x00,
//    0x80, 0x05, 0x00,
//    0x80, 0x06, 0x00,
//    0x80, 0x07, 0x00,
//    0x80, 0x08, 0x00,
//    0x36, 0x38, 0x00,
//	STOP_REG, STOP_REG, STOP_REG,
//};
//
#endif //#if defined(ENABLE_2624_1520_30_2lane)
#if defined(ENABLE_2688_1520_30_2lane)
//// 2688X1520  2lanes 30fps
//static STF_U8 g_u8ModeRegs_2688_1520_30_2lane[] = {
//    0x01, 0x03, 0x01,
//    0x36, 0x38, 0x00,
//    0x03, 0x00, 0x00, //0x01 eric
//    0x03, 0x01, 0x00,
//    0x03, 0x02, 0x23, //0x28 eric
//    0x03, 0x03, 0x00,
//    0x03, 0x04, 0x03,
//    0x03, 0x05, 0x01,
//    0x03, 0x06, 0x01,
//    0x03, 0x0A, 0x00,
//    0x03, 0x0B, 0x00,
//    0x03, 0x0C, 0x00,
//    0x03, 0x0e, 0x04,
//    0x03, 0x0f, 0x01,
//    0x03, 0x11, 0x00, // pll2 prediv
//    0x03, 0x12, 0x01,
//    0x03, 0x1e, 0x00,
//    0x30, 0x00, 0x20,
//    0x30, 0x02, 0x00,
//    0x30, 0x18, 0x32,
//    0x30, 0x19, 0x0C,
//    0x30, 0x20, 0x93,
//    0x30, 0x21, 0x03,
//    0x30, 0x22, 0x01,
//    0x30, 0x31, 0x0a,
//    0x30, 0x3f, 0x0c,
//    0x33, 0x05, 0xf1,
//    0x33, 0x07, 0x04,
//    0x33, 0x09, 0x29,
//    0x35, 0x00, 0x00,
//    0x35, 0x01, 0x7c,
//    0x35, 0x02, 0xc0,
//    0x35, 0x03, 0x04,
//    0x35, 0x04, 0x00,
//    0x35, 0x05, 0x00,
//    0x35, 0x06, 0x00,
//    0x35, 0x07, 0x00,
//    0x35, 0x08, 0x00/*0x01*/,
//    0x35, 0x09, 0x80/*0xb8*/,
//    0x35, 0x0a, 0x00,
//    0x35, 0x0b, 0x00,
//    0x35, 0x0c, 0x00,
//    0x35, 0x0d, 0x00,
//    0x35, 0x0e, 0x00,
//    0x35, 0x0f, 0x80,
//    0x35, 0x10, 0x00,
//    0x35, 0x11, 0x00,
//    0x35, 0x12, 0x00,
//    0x35, 0x13, 0x00,
//    0x35, 0x14, 0x00,
//    0x35, 0x15, 0x80,
//    0x35, 0x16, 0x00,
//    0x35, 0x17, 0x00,
//    0x35, 0x18, 0x00,
//    0x35, 0x19, 0x00,
//    0x35, 0x1a, 0x00,
//    0x35, 0x1b, 0x80,
//    0x35, 0x1c, 0x00,
//    0x35, 0x1d, 0x00,
//    0x35, 0x1e, 0x00,
//    0x35, 0x1f, 0x00,
//    0x35, 0x20, 0x00,
//    0x35, 0x21, 0x80,
//    0x35, 0x22, 0x08,
//    0x35, 0x24, 0x08,
//    0x35, 0x26, 0x08,
//    0x35, 0x28, 0x08,
//    0x35, 0x2a, 0x08,
//    0x36, 0x02, 0x00,
//    0x36, 0x03, 0x40,
//    0x36, 0x04, 0x02,
//    0x36, 0x05, 0x00,
//    0x36, 0x06, 0x00,
//    0x36, 0x07, 0x00,
//    0x36, 0x09, 0x12,
//    0x36, 0x0a, 0x40,
//    0x36, 0x0c, 0x08,
//    0x36, 0x0f, 0xe5,
//    0x36, 0x08, 0x8f,
//    0x36, 0x11, 0x00,
//    0x36, 0x13, 0xf7,
//    0x36, 0x16, 0x58,
//    0x36, 0x19, 0x99,
//    0x36, 0x1b, 0x60,
//    0x36, 0x1c, 0x7a,
//    0x36, 0x1e, 0x79,
//    0x36, 0x1f, 0x02,
//    0x36, 0x32, 0x00,
//    0x36, 0x33, 0x10,
//    0x36, 0x34, 0x10,
//    0x36, 0x35, 0x10,
//    0x36, 0x36, 0x15,
//    0x36, 0x46, 0x86,
//    0x36, 0x4a, 0x0b,
//    0x37, 0x00, 0x17,
//    0x37, 0x01, 0x22,
//    0x37, 0x03, 0x10,
//    0x37, 0x0a, 0x37,
//    0x37, 0x05, 0x00,
//    0x37, 0x06, 0x63,
//    0x37, 0x09, 0x3c,
//    0x37, 0x0b, 0x01,
//    0x37, 0x0c, 0x30,
//    0x37, 0x10, 0x24,
//    0x37, 0x11, 0x0c,
//    0x37, 0x16, 0x00,
//    0x37, 0x20, 0x28,
//    0x37, 0x29, 0x7b,
//    0x37, 0x2a, 0x84,
//    0x37, 0x2b, 0xbd,
//    0x37, 0x2c, 0xbc,
//    0x37, 0x2e, 0x52,
//    0x37, 0x3c, 0x0e,
//    0x37, 0x3e, 0x33,
//    0x37, 0x43, 0x10,
//    0x37, 0x44, 0x88,
//    0x37, 0x45, 0xc0,
//    0x37, 0x4a, 0x43,
//    0x37, 0x4c, 0x00,
//    0x37, 0x4e, 0x23,
//    0x37, 0x51, 0x7b,
//    0x37, 0x52, 0x84,
//    0x37, 0x53, 0xbd,
//    0x37, 0x54, 0xbc,
//    0x37, 0x56, 0x52,
//    0x37, 0x5c, 0x00,
//    0x37, 0x60, 0x00,
//    0x37, 0x61, 0x00,
//    0x37, 0x62, 0x00,
//    0x37, 0x63, 0x00,
//    0x37, 0x64, 0x00,
//    0x37, 0x67, 0x04,
//    0x37, 0x68, 0x04,
//    0x37, 0x69, 0x08,
//    0x37, 0x6a, 0x08,
//    0x37, 0x6b, 0x20,
//    0x37, 0x6c, 0x00,
//    0x37, 0x6d, 0x00,
//    0x37, 0x6e, 0x00,
//    0x37, 0x73, 0x00,
//    0x37, 0x74, 0x51,
//    0x37, 0x76, 0xbd,
//    0x37, 0x77, 0xbd,
//    0x37, 0x81, 0x18,
//    0x37, 0x83, 0x25,
//    0x37, 0x98, 0x1b,
//    0x38, 0x00, 0x00,
//    0x38, 0x01, 0x08,
//    0x38, 0x02, 0x00,
//    0x38, 0x03, 0x04,
//    0x38, 0x04, 0x0A,
//    0x38, 0x05, 0x97,
//    0x38, 0x06, 0x05,
//    0x38, 0x07, 0xFB,
//    0x38, 0x08, 0x0A,
//    0x38, 0x09, 0x80,
//    0x38, 0x0A, 0x05,
//    0x38, 0x0B, 0xF0,
//    0x38, 0x0C, 0x0a, //fps
//    0x38, 0x0D, 0x0a,
//    0x38, 0x0E, 0x06,
//    0x38, 0x0F, 0x14,
//    0x38, 0x10, 0x00,
//    0x38, 0x11, 0x08,
//    0x38, 0x12, 0x00,
//    0x38, 0x13, 0x04,
//    0x38, 0x14, 0x01,
//    0x38, 0x15, 0x01,
//    0x38, 0x19, 0x01,
//    0x38, 0x20, 0x00,
//    0x38, 0x21, 0x06,
//    0x38, 0x23, 0x00,
//    0x38, 0x24, 0x00,
//    0x38, 0x25, 0x20,
//    0x38, 0x26, 0x00,
//    0x38, 0x27, 0x04,
//    0x38, 0x29, 0x00,
//    0x38, 0x2a, 0x01,
//    0x38, 0x2b, 0x01,
//    0x38, 0x2d, 0x7f,
//    0x38, 0x30, 0x04,
//    0x38, 0x36, 0x01,
//    0x38, 0x37, 0x00,
//    0x38, 0x41, 0x02,
//    0x38, 0x46, 0x08,
//    0x38, 0x47, 0x07,
//    0x3d, 0x85, 0x36,
//    0x3d, 0x8c, 0x71,
//    0x3d, 0x8d, 0xcb,
//    0x3f, 0x0a, 0x00,
//    0x40, 0x00, 0xf1,
//    0x40, 0x01, 0x40,
//    0x40, 0x02, 0x04,
//    0x40, 0x03, 0x14,
//    0x40, 0x0e, 0x00,
//    0x40, 0x11, 0x00,
//    0x40, 0x1a, 0x00,
//    0x40, 0x1b, 0x00,
//    0x40, 0x1c, 0x00,
//    0x40, 0x1d, 0x00,
//    0x40, 0x1f, 0x00,
//    0x40, 0x20, 0x00,
//    0x40, 0x21, 0x10,
//    0x40, 0x22, 0x08,
//    0x40, 0x23, 0xd3,
//    0x40, 0x24, 0x0A,
//    0x40, 0x25, 0x00,
//    0x40, 0x26, 0x0A,
//    0x40, 0x27, 0x10,
//    0x40, 0x28, 0x00,
//    0x40, 0x29, 0x02,
//    0x40, 0x2a, 0x06,
//    0x40, 0x2b, 0x04,
//    0x40, 0x2c, 0x02,
//    0x40, 0x2d, 0x02,
//    0x40, 0x2e, 0x0e,
//    0x40, 0x2f, 0x04,
//    0x43, 0x02, 0xff,
//    0x43, 0x03, 0xff,
//    0x43, 0x04, 0x00,
//    0x43, 0x05, 0x00,
//    0x43, 0x06, 0x00,
//    0x43, 0x08, 0x02,
//    0x45, 0x00, 0x6c,
//    0x45, 0x01, 0xc4,
//    0x45, 0x02, 0x40,
//    0x45, 0x03, 0x01,
//    0x46, 0x00, 0x00,
//    0x46, 0x01, 0xA3,
//    0x48, 0x00, 0x04, // timing
//    0x48, 0x13, 0x08,
//    0x48, 0x1f, 0x40,
//    0x48, 0x29, 0x78,
//    0x48, 0x37, 0x14,
//    0x4b, 0x00, 0x2a,
//    0x4b, 0x0d, 0x00,
//    0x4d, 0x00, 0x04,
//    0x4d, 0x01, 0x42,
//    0x4d, 0x02, 0xd1,
//    0x4d, 0x03, 0x93,
//    0x4d, 0x04, 0xf5,
//    0x4d, 0x05, 0xc1,
//    0x50, 0x00, 0xf3,
//    0x50, 0x01, 0x11,
//    0x50, 0x04, 0x00,
//    0x50, 0x0a, 0x00,
//    0x50, 0x0b, 0x00,
//    0x50, 0x32, 0x00,
//    0x50, 0x40, 0x00,
//    0x50, 0x50, 0x0c,
//    0x55, 0x00, 0x00,
//    0x55, 0x01, 0x10,
//    0x55, 0x02, 0x01,
//    0x55, 0x03, 0x0f,
//    0x80, 0x00, 0x00,
//    0x80, 0x01, 0x00,
//    0x80, 0x02, 0x00,
//    0x80, 0x03, 0x00,
//    0x80, 0x04, 0x00,
//    0x80, 0x05, 0x00,
//    0x80, 0x06, 0x00,
//    0x80, 0x07, 0x00,
//    0x80, 0x08, 0x00,
//    0x36, 0x38, 0x00,
//    STOP_REG, STOP_REG, STOP_REG,
//};
//
#endif //#if defined(ENABLE_2688_1520_30_2lane)
struct ST_OV4689_MODE {
    STF_U16 u16Width;
    STF_U16 u16Height;
    STF_U8 u8Flipping;
    STF_U8 *pu8ModeRegisters;
    STF_U32 u32Registers;           // initially 0 then computed the 1st time
};

struct ST_OV4689_MODE g_stOv4689Modes[] =  {
#if defined(ENABLE_1920_1080_30_4lane)
    { 1920, 1080, SENSOR_FLIP_BOTH, g_u8ModeRegs_1920_1080_30_4lane, 0 },   // 0
#endif //#if defined(ENABLE_1920_1088_30_4lane)
//#if defined(ENABLE_1920_1088_60_4lane)
//    { 1920, 1088, SENSOR_FLIP_BOTH, g_u8ModeRegs_1920_1088_60_4lane, 0 },   // 1
//#endif //#if defined(ENABLE_1920_1088_60_4lane)
//#if defined(ENABLE_1920_1088_30_4lane)
//    { 1920, 1088, SENSOR_FLIP_BOTH, g_u8ModeRegs_1920_1088_30_4lane, 0 },   // 2
//#endif //#if defined(ENABLE_1920_1088_30_4lane)
//#if defined(ENABLE_1920_1088_60_2lane)
//    { 1920, 1088, SENSOR_FLIP_BOTH, g_u8ModeRegs_1920_1088_60_2lane, 0 },   // 3
//#endif //#if defined(ENABLE_1920_1088_60_2lane)
//#if defined(ENABLE_1920_1088_30_2lane)
//    { 1920, 1088, SENSOR_FLIP_BOTH, g_u8ModeRegs_1920_1088_30_2lane, 0 },   // 4
//#endif //#if defined(ENABLE_1920_1088_30_2lane)
//#if defined(ENABLE_1920_1088_20_2lane)
//	  { 1920, 1088, SENSOR_FLIP_BOTH, g_u8ModeRegs_1920_1088_20_2lane, 0 },   // 5
//#endif //#if defined(ENABLE_1920_1088_20_2lane)
//#if defined(ENABLE_1920_1088_30_1lane)
//    { 1920, 1088, SENSOR_FLIP_BOTH, g_u8ModeRegs_1920_1088_30_1lane, 0 },   // 6
//#endif //#if defined(ENABLE_1920_1088_30_1lane)
//#if defined(ENABLE_2624_1520_30_2lane)
//    { 2624, 1520, SENSOR_FLIP_BOTH, g_u8ModeRegs_2624_1520_30_2lane, 0 },   // 7
//#endif //#if defined(ENABLE_2624_1520_30_2lane)
//#if defined(ENABLE_2688_1520_30_2lane)
//    { 2688, 1520, SENSOR_FLIP_BOTH, g_u8ModeRegs_2688_1520_30_2lane, 0 },   // 8
//#endif //#if defined(ENABLE_2688_1520_30_2lane)
};
static STF_U8 g_u8SensorModeNum = 0;


// used before implementation therefore declared here
static STF_VOID Sensor_ConfigRegister(
    ST_OV4689_CAM *pstSensorCam
    );
static STF_RESULT Sensor_GetModeInfo(
    ST_OV4689_CAM *pstSensorCam,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    );
static STF_U32 Sensor_ComputeGains(
    STF_DOUBLE dGain
    );
static STF_DOUBLE Sensor_ComputeRegToGains(
    STF_U32 u32Gain
    );
static STF_U32 Sensor_ComputeExposure(
    STF_U32 u32Exposure,
    STF_DOUBLE dExposureMin
    );
static STF_U16 Sensor_ComputeFocus(
    const STF_U16 *pu16DACDist,
    const STF_U16 *pu16DACValues,
    STF_U8 u8Entries,
    STF_U16 u16Requested
    );
static STF_RESULT sSetFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Focus
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


static STF_U8* Sensor_GetRegisters(
    ST_OV4689_CAM *pstSensorCam,
    STF_U8 u8ModeIdx,
    STF_U32 *pu32Registers
    )
{

    STF_ASSERT(pu32Registers);  // null pointer forbidden

    if (u8ModeIdx < ARRAY_SIZE(g_stOv4689Modes)) {
        STF_U8 *registerArray = g_stOv4689Modes[u8ModeIdx].pu8ModeRegisters;

        if (0 == g_stOv4689Modes[u8ModeIdx].u32Registers) {
            // should be done only once per mode
            int i = 0;
            while (STOP_REG != registerArray[3 * i]) {
                i++;
            }
            g_stOv4689Modes[u8ModeIdx].u32Registers = i;
        }
        *pu32Registers = g_stOv4689Modes[u8ModeIdx].u32Registers;
        
        return registerArray;
    }
    // if it is an invalid mode returns NULL

    return NULL;
}

#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
static STF_RESULT Sensor_I2cRead(
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U8 *pu8Data
    )
{
  #if 0
    STF_INT nRet;
    STF_U8 u8Buf[2];
    
    STF_ASSERT(pu8Data);  // null pointer forbidden
    
    /* Set I2C slave address */
	if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C read address!\n");
        return STF_ERROR_BUSY;
    }
    
	u8Buf[0] = (u16Reg >> 8) & 0xFF;
	u8Buf[1] = u16Reg & 0xFF;
    
    nRet = write(nI2c, u8Buf, sizeof(u8Buf));
    if (sizeof(u8Buf) != nRet) {
        LOG_WARNING("Wrote %dB instead of %luB before reading\n",
            nRet, sizeof(u8Buf));
    }
    
    nRet = read(nI2c, pu8Data, 1);
    if (1 != nRet) {
        LOG_ERROR("Failed to read I2C at 0x%04X\n", u16Reg);
        return STF_ERROR_FATAL;
    }
  #else
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[2];
    STF_U8 u8Addr[2];

    STF_ASSERT(pu8Data);  // null pointer forbidden

    u8Addr[0] = (u16Reg >> 8) & 0xFF;
    u8Addr[1] = u16Reg & 0xFF;

    stMessages[0].addr = OV4689_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len = 2;
    stMessages[0].buf = u8Addr;

    stMessages[1].addr = OV4689_I2C_ADDR;
    stMessages[1].flags = I2C_M_RD;
    stMessages[1].len = 1;
    stMessages[1].buf = pu8Data;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 2;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to read reg 0x%04X.\n", u16Reg);
        return STF_ERROR_FATAL;
    }
    //printf("read  <-[0x%04X] = 0x%02X\n", u16Reg,  *pu8Data);
  #endif

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
static STF_RESULT Sensor_I2cRead16(
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U16 *pu16Data
    )
{
#if 0
    STF_INT nRet;
    STF_U8 u8Buf[2];
    
    STF_ASSERT(pu16Data);  // null pointer forbidden
    
    /* Set I2C slave address */
	if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C read address!\n");
        return STF_ERROR_BUSY;
    }
    
	u8Buf[0] = (u16Reg >> 8) & 0xFF;
	u8Buf[1] = u16Reg & 0xFF;
    
    nRet = write(nI2c, u8Buf, sizeof(u8Buf));
    if (sizeof(u8Buf) != nRet) {
        LOG_WARNING("Wrote %dB instead of %luB before reading\n",
            nRet, sizeof(u8Buf));
    }
    
    nRet = read(nI2c, u8Buf, sizeof(u8Buf));
    if (sizeof(u8Buf) != nRet) {
        LOG_ERROR("Failed to read I2C at 0x%04X\n", u16Reg);
        return STF_ERROR_FATAL;
    }

    *pu16Data = ((STF_U16)u8Buf[0] << 8) | u8Buf[1];
#else
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[2];
    STF_U8 u8Addr[2];
    STF_U8 u8Data[2];

    STF_ASSERT(pu16Data);  // null pointer forbidden

    u8Addr[0] = (u16Reg >> 8) & 0xFF;
    u8Addr[1] = u16Reg & 0xFF;

    stMessages[0].addr = OV4689_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len = 2;
    stMessages[0].buf = u8Addr;

    stMessages[1].addr = OV4689_I2C_ADDR;
    stMessages[1].flags = I2C_M_RD;
    stMessages[1].len = 2;
    stMessages[1].buf = u8Data;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 2;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to read reg 0x%04X.\n", u16Reg);
        return STF_ERROR_FATAL;
    }
    *pu16Data = ((STF_U16)u8Data[0] << 8) | u8Data[1];
    //printf("read  <-[0x%04X] = 0x%04X\n", u16Reg,  *pu16Data);
#endif

    return STF_SUCCESS;
}
#else
static STF_RESULT Sensor_I2cRead16(
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U16 *pu16Data
    )
{
    ST_CAM_REG stCamReg;
    STF_RESULT Ret = STF_SUCCESS;

    STF_ASSERT(pCIConnection);  // null pointer forbidden
    STF_ASSERT(pu16Data);       // null pointer forbidden

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

    *pu16Data = (stCamReg.u16RegValue << 8);

    stCamReg.u16RegAddr++;
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

    *pu16Data |= (STF_U8)stCamReg.u16RegValue;

    return STF_SUCCESS;
}
#endif //USE_LINUX_SYSTEM_STARTAND_I2C

#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
static STF_RESULT Sensor_I2cWriteRegs(
    STF_INT nI2c,
    const STF_U8 *pu8Data,
    STF_U16 u16Len
    )
{
#if 0
    STF_U16 u16Idx;
  #ifdef CONFIG_REG_DEBUG
    FILE *fpLog = NULL;
    static STF_U16 u16Write = 0;
  #endif //CONFIG_REG_DEBUG

    STF_ASSERT(pu8Data);        // null pointer forbidden

    /* Every write sequence needs to have 3 elements:
     * 1) slave address high bits [15:8]
     * 2) slave address low bits [7:0]
     * 3) data
     */
    if (u16Len % 3) {
        LOG_ERROR("Wrong len of data array, u16Len = %d", u16Len);
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Set I2C slave address */
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C slave address!\n");
        return STF_ERROR_BUSY;
    }

  #ifdef CONFIG_REG_DEBUG
    fpLog = fopen("/tmp/ov4689_write.txt", "a");
    fprintf(fpLog, "write %d\n", u16Write++);
  #endif //CONFIG_REG_DEBUG
    for (u16Idx = 0; u16Idx < u16Len; pu8Data += 3, u16Idx += 3) {
        STF_INT nWriteLen = 0;

        if (DELAY_REG == pu8Data[0]) {
            usleep((int)pu8Data[1] * 1000);
  #ifdef CONFIG_REG_DEBUG
            fprintf(fpLog, "delay %dms\n", pu8Data[1]);
  #endif //CONFIG_REG_DEBUG
            continue;
        }

        nWriteLen = write(nI2c, pu8Data, 3);
  #ifdef CONFIG_REG_DEBUG
        fprintf(fpLog, "0x%02X%02X 0x%02X\n", pu8Data[0], pu8Data[1], pu8Data[2]);
  #endif //CONFIG_REG_DEBUG

        if (3 != nWriteLen) {
            LOG_ERROR("Failed to write I2C data! write_len = %d, index = %d\n",
                nWriteLen, u16Idx);
  #ifdef CONFIG_REG_DEBUG
            fclose(fpLog);
  #endif //CONFIG_REG_DEBUG
            return STF_ERROR_BUSY;
        }
    }
  #ifdef CONFIG_REG_DEBUG
    fclose(fpLog);
  #endif //CONFIG_REG_DEBUG
#else
    STF_U16 u16Idx;
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[1];
  #ifdef CONFIG_REG_DEBUG
    FILE *fpLog = NULL;
    static STF_U16 u16Write = 0;
  #endif //CONFIG_REG_DEBUG

    STF_ASSERT(pu8Data);        // null pointer forbidden

    /* Every write sequence needs to have 3 elements:
     * 1) slave address high bits [15:8]
     * 2) slave address low bits [7:0]
     * 3) data
     */
    if (u16Len % 3) {
        LOG_ERROR("Wrong len of data array, u16Len = %d", u16Len);
        return STF_ERROR_INVALID_PARAMETERS;
    }

  #ifdef CONFIG_REG_DEBUG
    fpLog = fopen("/tmp/ov4689_write.txt", "a");
    fprintf(fpLog, "write %d\n", u16Write++);
  #endif //CONFIG_REG_DEBUG
    for (u16Idx = 0; u16Idx < u16Len; pu8Data += 3, u16Idx += 3) {
        if (DELAY_REG == pu8Data[0]) {
            usleep((int)pu8Data[1] * 1000);
  #ifdef CONFIG_REG_DEBUG
            fprintf(fpLog, "delay %dms\n", pu8Data[1]);
  #endif //CONFIG_REG_DEBUG
            continue;
        }

        stMessages[0].addr  = OV4689_I2C_ADDR;
        stMessages[0].flags = 0;
        stMessages[0].len   = 3;
        stMessages[0].buf   = (STF_U8 *)pu8Data;

        stPackets.msgs = stMessages;
        stPackets.nmsgs = 1;

  #ifdef CONFIG_REG_DEBUG
        fprintf(fpLog, "0x%02X%02X 0x%02X\n", pu8Data[0], pu8Data[1], pu8Data[2]);
  #endif //CONFIG_REG_DEBUG
        if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
            LOG_ERROR("Unable to write reg 0x%02X%02X with data 0x%02X.\n",
                pu8Data[0], pu8Data[1], pu8Data[2]);
  #ifdef CONFIG_REG_DEBUG
            fclose(fpLog);
  #endif //CONFIG_REG_DEBUG
            return STF_ERROR_FATAL;
        }
        //printf("write ->[0x%02X%02X] = 0x%02X\n", pu8Data[0], pu8Data[1], pu8Data[2]);
    }
  #ifdef CONFIG_REG_DEBUG
    fclose(fpLog);
  #endif //CONFIG_REG_DEBUG
#endif

    return STF_SUCCESS;
}
#else
static STF_RESULT Sensor_I2cWriteRegs(
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
    STF_INT nI2c,
    const STF_U8 *pu8Data,
    STF_U16 u16Len
    )
{
    ST_CAM_REG stCamReg;
    STF_U16 u16Idx;
    STF_RESULT Ret = STF_SUCCESS;
  #ifdef CONFIG_REG_DEBUG
    FILE *fpLog = NULL;
    static STF_U16 u16Write = 0;
  #endif //CONFIG_REG_DEBUG

    STF_ASSERT(pCIConnection);  // null pointer forbidden
    STF_ASSERT(pu8Data);        // null pointer forbidden

    /* Every write sequence needs to have 3 elements:
     * 1) slave address high bits [15:8]
     * 2) slave address low bits [7:0]
     * 3) data
     */
    if (u16Len % 3) {
        LOG_ERROR("Wrong len of data array, u16Len = %d", u16Len);
        return STF_ERROR_INVALID_PARAMETERS;
    }

  #ifdef CONFIG_REG_DEBUG
    fpLog = fopen("/tmp/ov4689_write.txt", "a");
    fprintf(fpLog, "write %d\n", u16Write++);
  #endif //CONFIG_REG_DEBUG
    for (u16Idx = 0; u16Idx < u16Len; pu8Data += 3, u16Idx += 3) {
        if (DELAY_REG == pu8Data[0]) {
            usleep((int)pu8Data[1] * 1000);
  #ifdef CONFIG_REG_DEBUG
            fprintf(fpLog, "delay %dms\n", pu8Data[1]);
  #endif //CONFIG_REG_DEBUG
            continue;
        }

        stCamReg.u8Idx = u8IspIdx;
        stCamReg.u16RegAddr = pu8Data[0];
        stCamReg.u16RegAddr = (stCamReg.u16RegAddr << 8) | pu8Data[1];
        stCamReg.u16RegValue = pu8Data[2];
        Ret = STFDRV_CD_SYS_CAM_OBJ_I2cWrite(
  #if defined(VIRTUAL_IO_MAPPING)
            pCIConnection,
  #endif //VIRTUAL_IO_MAPPING
            &stCamReg
            );
  #ifdef CONFIG_REG_DEBUG
        fprintf(fpLog, "0x%02X%02X 0x%02X\n", pu8Data[0], pu8Data[1], pu8Data[2]);
  #endif //CONFIG_REG_DEBUG

        if (Ret) {
            LOG_ERROR("Unable to write reg 0x%04X with data 0x%02X.\n",
                stCamReg.u16RegAddr, stCamReg.u16RegValue);
  #ifdef CONFIG_REG_DEBUG
            fclose(fpLog);
  #endif //CONFIG_REG_DEBUG
            return STF_ERROR_FATAL;
        }
        //printf("write ->[0x%02X%02X] = 0x%02X\n", pu8Data[0], pu8Data[1], pu8Data[2]);
    }
  #ifdef CONFIG_REG_DEBUG
    fclose(fpLog);
  #endif //CONFIG_REG_DEBUG

    return STF_SUCCESS;
}
#endif //USE_LINUX_SYSTEM_STARTAND_I2C

#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
static STF_RESULT Sensor_I2cWriteRegs_2(
    STF_INT nI2c,
    STF_U16 u16Addr,
    STF_U8 *pu8Data,
    STF_U16 u16Len
    )
{
#if 0
    STF_INT nRet;
    STF_U8 u8Buf[MAX_I2C_BUF_SIZE];

    STF_ASSERT(pu8Data);        // null pointer forbidden

    if ((MAX_I2C_BUF_SIZE - 2) < u16Len) {
        LOG_ERROR("Wrong len of data array, u16Len(%d) > %d\n",
            u16Len, (MAX_I2C_BUF_SIZE - 2));
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Set I2C slave address */
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C slave address!\n");
        return STF_ERROR_BUSY;
    }

    u8Buf[0] = (u16Addr >> 8) & 0xFF;
    u8Buf[1] = u16Addr & 0xFF;
    STF_MEMCPY(&u8Buf[2], pu8Data, u16Len);

    nRet = write(nI2c, u8Buf, (u16Len + 2));
    if ((u16Len + 2) != nRet) {
        LOG_ERROR("Unable to write reg 0x%04X with data length %d.\n",
            u16Addr, u16Len);
        return STF_ERROR_FATAL;
    }
#else
    STF_U16 u16Idx;
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[1];
    STF_U8 u8Buf[MAX_I2C_BUF_SIZE];

    STF_ASSERT(pu8Data);        // null pointer forbidden

    if ((MAX_I2C_BUF_SIZE - 2) < u16Len) {
        LOG_ERROR("Wrong len of data array, u16Len(%d) > %d\n",
            u16Len, (MAX_I2C_BUF_SIZE - 2));
        return STF_ERROR_INVALID_PARAMETERS;
    }

    u8Buf[0] = (u16Addr >> 8) & 0xFF;
    u8Buf[1] = u16Addr & 0xFF;
    STF_MEMCPY(&u8Buf[2], pu8Data, u16Len);

    stMessages[0].addr  = OV4689_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len   = (u16Len + 2);
    stMessages[0].buf   = u8Buf;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 1;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to write reg 0x%04X with data length %d.\n",
            u16Addr, u16Len);
        return STF_ERROR_FATAL;
    }
#endif

    return STF_SUCCESS;
}
#else
static STF_RESULT Sensor_I2cWriteRegs_2(
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
    STF_INT nI2c,
    STF_U16 u16Addr,
    STF_U8 *pu8Data,
    STF_U16 u16Len
    )
{
    ST_CAM_REG stCamReg;
    STF_U16 u16RegAddr;
    STF_U16 u16Idx;
    STF_RESULT Ret = STF_SUCCESS;

    STF_ASSERT(pCIConnection);  // null pointer forbidden
    STF_ASSERT(pu8Data);        // null pointer forbidden

    u16RegAddr = u16Addr;
    for (u16Idx = 0; u16Idx < u16Len; u16RegAddr++, pu8Data++, u16Idx++) {
        stCamReg.u8Idx = u8IspIdx;
        stCamReg.u16RegAddr = u16RegAddr;
        stCamReg.u16RegValue = *pu8Data;
        Ret = STFDRV_CD_SYS_CAM_OBJ_I2cWrite(
            &stCamReg
            );

        if (Ret) {
            LOG_ERROR("Unable to write reg 0x%04X with data 0x%02X.\n",
                stCamReg.u16RegAddr, stCamReg.u16RegValue);
            return STF_ERROR_FATAL;
        }
        //printf("write ->[0x%02X%02X] = 0x%02X\n", pu8Data[0], pu8Data[1], pu8Data[2]);
    }

    return STF_SUCCESS;
}
#endif //USE_LINUX_SYSTEM_STARTAND_I2C

#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
static STF_RESULT Sensor_I2cWriteRegs_3(
    STF_INT nI2c,
    STF_U8 *pu8Data,
    STF_U16 u16Len
    )
{
#if 0
    STF_INT nRet;

    STF_ASSERT(pu8Data);        // null pointer forbidden

    /* Set I2C slave address */
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C slave address!\n");
        return STF_ERROR_BUSY;
    }

    nRet = write(nI2c, pu8Data, u16Len);
    if (u16Len != nRet) {
        LOG_ERROR("Unable to write reg 0x%02X%02X with data length %d.\n",
            pu8Data[0], pu8Data[1], (u16Len - 2));
        return STF_ERROR_FATAL;
    }
#else
    STF_U16 u16Idx;
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[1];

    STF_ASSERT(pu8Data);        // null pointer forbidden

    stMessages[0].addr  = OV4689_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len   = u16Len;
    stMessages[0].buf   = pu8Data;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 1;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to write reg 0x%02X%02X with data length %d.\n",
            pu8Data[0], pu8Data[1], (u16Len - 2));
        return STF_ERROR_FATAL;
    }
#endif

    return STF_SUCCESS;
}
#else
static STF_RESULT Sensor_I2cWriteRegs_3(
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
    STF_INT nI2c,
    STF_U8 *pu8Data,
    STF_U16 u16Len
    )
{
    ST_CAM_REG stCamReg;
    STF_U16 u16RegAddr;
    STF_U16 u16Idx;
    STF_RESULT Ret = STF_SUCCESS;

    STF_ASSERT(pCIConnection);  // null pointer forbidden
    STF_ASSERT(pu8Data);        // null pointer forbidden

    u16RegAddr = pu8Data[0];
    u16RegAddr = (u16RegAddr << 8) | pu8Data[1];
    pu8Data += 2;
    for (u16Idx = 0; u16Idx < (u16Len - 2); u16RegAddr++, pu8Data++, u16Idx++) {
        stCamReg.u8Idx = u8IspIdx;
        stCamReg.u16RegAddr = u16RegAddr;
        stCamReg.u16RegValue = *pu8Data;
        Ret = STFDRV_CD_SYS_CAM_OBJ_I2cWrite(
            &stCamReg
            );

        if (Ret) {
            LOG_ERROR("Unable to write reg 0x%04X with data 0x%02X.\n",
                stCamReg.u16RegAddr, stCamReg.u16RegValue);
            return STF_ERROR_FATAL;
        }
        //printf("write ->[0x%02X%02X] = 0x%02X\n", pu8Data[0], pu8Data[1], pu8Data[2]);
    }

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
#if 0
    STF_INT nRet;
    STF_U8 u8Buf[3];

    /* Set I2C slave address */
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C slave address!\n");
        return STF_ERROR_BUSY;
    }

    u8Buf[0] = (u16Reg >> 8) & 0xFF;
    u8Buf[1] = u16Reg & 0xFF;
    u8Buf[2] = u8Data;

    nRet = write(nI2c, u8Buf, sizeof(u8Buf));
    if (sizeof(u8Buf) != nRet) {
        LOG_ERROR("Unable to write reg 0x%04X with data 0x%02X.\n",
            u16Reg, u8Data);
        return STF_ERROR_FATAL;
    }
#else
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[1];
    STF_U8 u8Buf[3];

    u8Buf[0] = (u16Reg >> 8) & 0xFF;
    u8Buf[1] = u16Reg & 0xFF;
    u8Buf[2] = u8Data;

    stMessages[0].addr  = OV4689_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len   = sizeof(u8Buf);
    stMessages[0].buf   = u8Buf;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 1;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to write reg 0x%04X with data 0x%02X.\n",
            u16Reg, u8Data);
        return STF_ERROR_FATAL;
    }
#endif
    //printf("write ->[0x%04X] = 0x%02X\n", u16Reg, u8Data);

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
    STF_U16 u16Idx;
    STF_RESULT Ret = STF_SUCCESS;

    STF_ASSERT(pCIConnection);  // null pointer forbidden

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
        LOG_ERROR("Unable to write reg 0x%04X with data 0x%02X.\n",
            stCamReg.u16RegAddr, stCamReg.u16RegValue);
        return STF_ERROR_FATAL;
    }
    //printf("write ->[0x%04X] = 0x%02X\n",
    //    stCamReg.u16RegAddr, stCamReg.u16RegValue);

    return STF_SUCCESS;
}
#endif //USE_LINUX_SYSTEM_STARTAND_I2C

#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
static STF_RESULT Sensor_I2cWrite16(
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U16 u16Data
    )
{
#if 0
    STF_INT nRet;
    STF_U8 u8Buf[4];

    /* Set I2C slave address */
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C slave address!\n");
        return STF_ERROR_BUSY;
    }

    u8Buf[0] = (u16Reg >> 8) & 0xFF;
    u8Buf[1] = u16Reg & 0xFF;
    u8Buf[2] = (u16Data >> 8) & 0xFF;
    u8Buf[3] = u16Data & 0xFF;

    nRet = write(nI2c, u8Buf, sizeof(u8Buf));
    if (sizeof(u8Buf) != nRet) {
        LOG_ERROR("Unable to write reg 0x%04X with data 0x%04X.\n",
            u16Reg, u16Data);
        return STF_ERROR_FATAL;
    }
#else
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[1];
    STF_U8 u8Buf[4] = { 0 };

    u8Buf[0] = (u16Reg >> 8) & 0xFF;
    u8Buf[1] = u16Reg & 0xFF;
    u8Buf[2] = (u16Data >> 8) & 0xFF;
    u8Buf[3] = u16Data & 0xFF;

    stMessages[0].addr  = OV4689_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len   = sizeof(u8Buf);
    stMessages[0].buf   = u8Buf;

    stPackets.msgs = stMessages;
    stPackets.nmsgs = 1;

    if (0 > ioctl(nI2c, I2C_RDWR, &stPackets)) {
        LOG_ERROR("Unable to write reg 0x%04X with data 0x%04X.\n",
            u16Reg, u16Data);
        return STF_ERROR_FATAL;
    }
    //printf("write ->[0x%04X] = 0x%04X\n", u16Reg, u16Data);
#endif

    return STF_SUCCESS;
}
#else
static STF_RESULT Sensor_I2cWrite16(
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
    STF_INT nI2c,
    STF_U16 u16Reg,
    STF_U16 u16Data
    )
{
    ST_CAM_REG stCamReg;
    STF_U16 u16Idx;
    STF_RESULT Ret = STF_SUCCESS;

    STF_ASSERT(pCIConnection);  // null pointer forbidden

    stCamReg.u8Idx = u8IspIdx;
    stCamReg.u16RegAddr = u16Reg;
    stCamReg.u16RegValue = u16Data;
    Ret = STFDRV_CD_SYS_CAM_OBJ_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING)
        pCIConnection,
#endif //VIRTUAL_IO_MAPPING
        &stCamReg
        );

    if (Ret) {
        LOG_ERROR("Unable to write reg 0x%04X with data 0x%04X.\n",
            stCamReg.u16RegAddr, stCamReg.u16RegValue);
        return STF_ERROR_FATAL;
    }
    //printf("write ->[0x%04X] = 0x%04X\n",
    //    stCamReg.u16RegAddr, stCamReg.u16RegValue);

    return STF_SUCCESS;
}
#endif //USE_LINUX_SYSTEM_STARTAND_I2C

static STF_VOID Sensor_ConfigRegister(
    ST_OV4689_CAM *pstSensorCam
    )
{
    STF_U32 nIdx;
    // index for flipping register
    STF_U32 u32FlipIdx0 = pstSensorCam->u32Registers * 3;
    // index for flipping register
    STF_U32 u32FlipIdx1 = pstSensorCam->u32Registers * 3;
    // index for exposure register
    STF_U32 u32ExposureIdx = pstSensorCam->u32Registers * 3;
    // index for gain register
    STF_U32 u32GainIdx = pstSensorCam->u32Registers * 3;
#if !defined(COPY_REGS)
    STF_U8 u8Values[3];
#endif //COPY_REGS
    STF_U8 u8Regs[] = {
        0x32, 0x08, 0x00, // GROUP_ACCESS start hold of group 0

        0x35, 0x00, 0x00, // Exposure register
        0x35, 0x01, 0x2E, //
        0x35, 0x02, 0x80, //

        0x35, 0x07, 0x00, // Gain register
        0x35, 0x08, 0x00, //
        0x35, 0x09, 0x00, //

        0x32, 0x08, 0x10, // end hold of group 0 register writes
        0x32, 0x0b, 0x00, // set quick manual mode
        0x32, 0x08, 0xe0, // quick launch group 0
    };
    STF_RESULT Ret = 0;

    if (!pstSensorCam->pu8CurrentSensorModeReg) {
        LOG_ERROR("current register modes not available!\n");
        return;
    }

    LOG_DEBUG("Writing I2C\n");

    for (nIdx = 0; nIdx < pstSensorCam->u32Registers * 3; nIdx += 3) {
#if !defined(COPY_REGS)
        u8Values[0] = pstSensorCam->pu8CurrentSensorModeReg[nIdx + 0];
        u8Values[1] = pstSensorCam->pu8CurrentSensorModeReg[nIdx + 1];
        u8Values[2] = pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2];
#endif //COPY_REGS

        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_FORMAT1)) {
            u32FlipIdx0 = nIdx;
            if (pstSensorCam->u8Flipping & SENSOR_FLIP_VERTICAL) {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] = 6;
#else
                u8Values[2] = 6;
#endif //COPY_REGS
            } else {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] = 0;
#else
                u8Values[2] = 0;
#endif //COPY_REGS
            }
        }
        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_FORMAT2)) {
            u32FlipIdx1 = nIdx;
            if (pstSensorCam->u8Flipping & SENSOR_FLIP_HORIZONTAL) {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] = 6;
#else
                u8Values[2] = 6;
#endif //COPY_REGS
            } else {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] = 0;
#else
                u8Values[2] = 0;
#endif //COPY_REGS
            }
        }

        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_EXPOSURE)) {
            u32ExposureIdx = nIdx;
        }

        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_GAIN)) {
            u32GainIdx = nIdx;
        }

#if defined(V4L2_DRIVER)
#else
#if !defined(COPY_REGS)
        Ret = Sensor_I2cWriteRegs(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8Values,
            3
            );
        if (STF_SUCCESS != Ret) {
            break;
        }
#endif //COPY_REGS
#endif //#if defined(V4L2_DRIVER)
    }

    // if it was parallel
    //pstSensorCam->pstSensorPhy->psGasket->uiWidth =
    //    pstSensorCam->stSensorMode.u16Width;
    //pstSensorCam->pstSensorPhy->psGasket->uiHeight =
    //    pstSensorCam->stSensorMode.u16Height;
    //pstSensorCam->pstSensorPhy->psGasket->bVSync = STF_TRUE;
    //pstSensorCam->pstSensorPhy->psGasket->bHSync = STF_TRUE;
    //pstSensorCam->pstSensorPhy->psGasket->u8ParallelBitdepth = 10;

    pstSensorCam->u32Exposure = pstSensorCam->stSensorMode.u32ExposureMin * 302;
    pstSensorCam->u32Exposure = STF_MIN_INT(
        pstSensorCam->stSensorMode.u32ExposureMax, pstSensorCam->u32Exposure);

    //
    // before writing the registers, patch the flip horizontal/flipvertical
    // fields to match the required values
    //
    // should be found and fit in array
    if (u32FlipIdx0 >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32FlipIdx0 in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }
    if (u32FlipIdx1 >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32FlipIdx1 in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }

    if (u32GainIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32GainIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }
    if (u32ExposureIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32ExposureIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }
#if 0
    // should be found and fit in array
    STF_ASSERT(u32GainIdx+8 < pstSensorCam->u32Registers * 3);
    {
        STF_U32 u32Exposure;

        u32Exposure = Sensor_ComputeExposure(
            pstSensorCam->u32Exposure,
            pstSensorCam->stSensorMode.dExposureMin
            );
        u8Regs[5] = ((u32Exposure) >> 12) & 0xf;
        u8Regs[8] = ((u32Exposure) >> 4) & 0xff;
        u8Regs[11] = ((u32Exposure) << 4) & 0xff;
    }

    // should be found and fit in array
    STF_ASSERT(u32ExposureIdx+8 < pstSensorCam->u32Registers * 3);
    {
        STF_U32 u32Gain;

        u32Gain = Sensor_ComputeGains(pstSensorCam->dGain);
        u8Regs[14] = (u32Gain >> 16) & 0xff;
        u8Regs[17] = (u32Gain >> 8) & 0xff;
        u8Regs[20] = u32Gain & 0xff;
    }
#endif //0

#if defined(V4L2_DRIVER)
#else
#if defined(COPY_REGS)
    Sensor_I2cWriteRegs(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        pstSensorCam->pu8CurrentSensorModeReg,
        pstSensorCam->u32Registers * 3
        );
#endif //COPY_REGS
#endif //#if defined(V4L2_DRIVER)

    u8Regs[5] = g_OV4689_u8LinesValHiBackup ;
    u8Regs[8] = g_OV4689_u8LinesValMidBackup;
    u8Regs[11] = g_OV4689_u8LinesValLoBackup ;
    u8Regs[14] = g_OV4689_u8GainValHiBackup;
    u8Regs[17] = g_OV4689_u8GainValMidBackup ;
    u8Regs[20] = g_OV4689_u8GainValLoBackup;

    Ret = Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8Regs,
        sizeof(u8Regs)
        );

    LOG_DEBUG("Sensor ready to go\n");
    // setup the exposure setting information
}

static STF_RESULT Sensor_GetModeInfo(
    ST_OV4689_CAM *pstSensorCam,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    )
{
    STF_U32 u32Registers = 0;
    STF_U8 *pu8ModeReg = NULL;

    STF_U16 sc_cmmm_bit_sel = 0;
    STF_U16 sc_cmmn_mipi_sc_ctrl = 0;
    STF_U16 hts[2] = { 0, 0 };
    STF_U16 hts_v = 0;
    STF_U16 vts[2] = { 0, 0 };
    STF_U16 vts_v = 0;
    STF_U32 frame_t = 0;
    STF_U16 h_output[2] = { 0, 0 };
    STF_U16 v_output[2] = { 0, 0 };
    STF_U16 pll_ctrl_d[2] = { 0, 0 };

    STF_DOUBLE sclk = 0;
    STF_DOUBLE pll2_predivp = 0;
    STF_DOUBLE pll2_mult = 0;
    STF_DOUBLE pll2_prediv = 0;
    STF_DOUBLE pll2_divsp = 0;
    STF_DOUBLE pll2_divs = 0;

    STF_DOUBLE trow = 0; // in micro seconds

    STF_U16 nIdx;
    STF_RESULT ret = STF_SUCCESS;

    pu8ModeReg = Sensor_GetRegisters(
        pstSensorCam,
        u8ModeIdx,
        &u32Registers
        );
    if (!pu8ModeReg) {
        LOG_ERROR("invalid mode %d\n", pu8ModeReg);
        return STF_ERROR_NOT_SUPPORTED;
    }
    u32Registers = g_stOv4689Modes[u8ModeIdx].u32Registers;

    for (nIdx = 0 ; nIdx < u32Registers * 3 ; nIdx += 3) {
        if (IS_REG(pu8ModeReg, nIdx, REG_SC_CMMN_BIT_SEL)) {
            sc_cmmm_bit_sel = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("sc_cmmm_bit_sel = 0x%x\n", sc_cmmm_bit_sel);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_SC_CMMN_MIPI_SC_CTRL)) {
            sc_cmmn_mipi_sc_ctrl = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("sc_cmmn_mipi_sc_ctrl = 0x%x\n", sc_cmmn_mipi_sc_ctrl);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_TIMING_VTS_0)) {
            vts[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("vts[0] = 0x%x\n", vts[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_TIMING_VTS_1)) {
            vts[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("vts[1] = 0x%x\n", vts[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_TIMING_HTS_0)) {
            hts[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("hts[0]=0x%x\n", hts[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_TIMING_HTS_1)) {
            hts[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("hts[1]=0x%x\n", hts[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_H_OUTPUT_SIZE_0)) {
            h_output[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("h_output[0] = 0x%x, n=%d\n", h_output[0], nIdx);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_H_OUTPUT_SIZE_1)) {
            h_output[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("h_output[1] = 0x%x\n", h_output[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_V_OUTPUT_SIZE_0)) {
            v_output[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("v_output[0] = 0x%x\n", v_output[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_V_OUTPUT_SIZE_1)) {
            v_output[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("v_output[1] = 0x%x\n", v_output[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_CTRL_11)) {
            pll2_predivp = pu8ModeReg[nIdx + 2] + 1.0;
            LOG_DEBUG("pll2_predivp = 0x%x -> %lf\n",
                pu8ModeReg[nIdx + 2], pll2_predivp);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_CTRL_B)) {
            pll2_prediv = pll_ctrl_b_val[pu8ModeReg[nIdx + 2]];
            LOG_DEBUG("pll2_prediv = 0x%x -> %lf\n",
                pu8ModeReg[nIdx + 2], pll2_prediv);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_CTRL_C)) {
            pll_ctrl_d[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_ctrl_d[0] = 0x%x\n", pll_ctrl_d[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_CTRL_D)) {
            pll_ctrl_d[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_ctrl_d[1] = 0x%x\n", pll_ctrl_d[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_CTRL_F)) {
            pll2_divsp = pu8ModeReg[nIdx + 2] + 1.0;
            LOG_DEBUG("pll2_divsp = 0x%x -> %lf\n",
                pu8ModeReg[nIdx + 2], pll2_divsp);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_CTRL_E)) {
            pll2_divs = pll_ctrl_c_val[pu8ModeReg[nIdx + 2]];
            LOG_DEBUG("pll2_divs = 0x%x -> %lf\n",
                pu8ModeReg[nIdx + 2], pll2_divs);
        }
    }

    pll2_mult = (STF_DOUBLE)((pll_ctrl_d[0] << 8) | pll_ctrl_d[1]);

    if (0.0 == pll2_predivp
        || 0.0 == pll2_mult
        || 0.0 == pll2_prediv
        || 0.0 == pll2_divsp
        || 0.0 == pll2_divs
        ) {
        sclk = 120.0 * 1000 * 1000;
        LOG_WARNING("Did not find all PLL registers - assumes "\
            "sclk of %ld MHz\n", sclk);
        LOG_WARNING("pll2_predivp=%f, pll2_mult=%f, pll2_prediv=%f, "\
            "pll2_divsp=%f, pll2_divs=%f\n",
            pll2_predivp, pll2_mult, pll2_prediv, pll2_divsp, pll2_divs);
    } else {
        // xclk is the dRefClock
        // vco = xclk/PLL2_predivp * PLL2_mult/PLL2_prediv
        STF_DOUBLE vco = 0;
        // sclk = vco/PLL2_divsp/PLL2_divs

        vco = ((pstSensorCam->dRefClock / pll2_predivp) / pll2_prediv) * pll2_mult;
        sclk = vco / pll2_divsp / pll2_divs;

        LOG_DEBUG("xclk=%.2lfMHz / pll2_predivp=%.2lf / pll2_prediv=%.2lf "\
            "* pll2_mult=%.2lf\n",
            pstSensorCam->dRefClock/1000000, pll2_predivp, pll2_prediv, pll2_mult);
        LOG_DEBUG("vco=%.2lfMHz / pll2_divsp=%.2lf / pll2_divs=%.2lf "\
            "= sclk=%.2lfMHz\n",
            vco/1000000, pll2_divsp, pll2_divs, sclk/1000000);
    }

    hts_v = (hts[1] | (hts[0] << 8));
    vts_v = (vts[1] | (vts[0] << 8));
    trow = (hts_v / sclk) * 1000 * 1000; // sclk in Hz, trow in micro seconds
    frame_t = vts_v * hts_v;

    LOG_DEBUG("hts=%u vts=%u\n", hts_v, vts_v);
    LOG_DEBUG("trow=%lf frame_t=%u\n", trow, frame_t);

    pstModes->u16Width = h_output[1] | (h_output[0] << 8);
    pstModes->u16Height = v_output[1] | (v_output[0] << 8);
    pstModes->u16VerticalTotal = vts_v;
    g_u32VerticalTotal = vts_v;
    //LOG_DEBUG("************ hts=%u vts=%u ************\n", hts_v, vts_v);

    pstModes->u8SupportFlipping = SENSOR_FLIP_BOTH;
    pstModes->u32ExposureMax = (STF_U32)floor(trow * vts_v);
    pstModes->u32ExposureMin = (STF_U32)floor(trow);

    pstModes->dExposureMin = trow;
    pstSensorCam->u32Exposure = trow * pstModes->u16VerticalTotal; // set initial exposure.

    pstModes->dFrameRate = sclk / frame_t;

    pstModes->u8MipiLanes = ((sc_cmmn_mipi_sc_ctrl >> 5) & 0x3) + 1;
    //printf("pstModes->u8MipiLanes = %d\n", pstModes->u8MipiLanes);

    pstModes->u8BitDepth = (sc_cmmm_bit_sel & 0x1F);
    pstSensorCam->dCurFps = pstModes->dFrameRate;
    //printf("the dFrameRate = %f\n", pstModes->dFrameRate);
    pstSensorCam->u32FixedFps = (STF_U32)(pstSensorCam->dCurFps + 0.5);
    pstSensorCam->u32InitClk = pll2_divsp - 1;

    return STF_SUCCESS;
}

static STF_S8 sGetModeIdx(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Width,
    STF_U16 u16Height
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U8 u8Idx;
    STF_S8 s8ModeIdx= -1;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    V_LOG_INFO("OV4689 MIPI sensor u16Width = %d, u16Height = %d\n",
        u16Width, u16Height);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    for (u8Idx = 0; u8Idx < g_u8SensorModeNum; u8Idx++) {
        if ((u16Width == g_stOv4689Modes[u8Idx].u16Width)
            && (u16Height == g_stOv4689Modes[u8Idx].u16Height)) {
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
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    V_LOG_INFO("OV4689 MIPI sensor u8ModeIdx = %d\n", u8ModeIdx);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pstModes);
    ASSERT_MODE_RANGE(u8ModeIdx);

    Ret = Sensor_GetModeInfo(pstSensorCam, u8ModeIdx, pstModes);

    return Ret;
}

static STF_RESULT sGetState(
    SENSOR_HANDLE *pstHandle,
    SENSOR_STATUS *pstStatus
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //LOG_INFO("**OV4689 MIPI sensor sGetState** \n");
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

    ST_OV4689_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //LOG_INFO("**OV4689 MIPI sensor sGetInterfaceInfo** \n");
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
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U8 *pu8ModeReg = NULL;
    STF_U32 u32Registers = 0;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    V_LOG_INFO("**OV4689 MIPI sensor mode=%d, flipping=0x%02X**\n",
        u8ModeIdx, u8Flipping);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    ASSERT_MODE_RANGE(u8ModeIdx);

    pu8ModeReg = Sensor_GetRegisters(pstSensorCam, u8ModeIdx, &u32Registers);
    if (NULL == pu8ModeReg) {
        return STF_ERROR_INVALID_PARAMETERS;
    }

    if (u8Flipping !=
        (u8Flipping & g_stOv4689Modes[u8ModeIdx].u8Flipping)) {
        LOG_ERROR("Sensor mode %d does not support selected flipping 0x%x "\
            "(supports 0x%x)\n",
            u8ModeIdx,
            u8Flipping,
            g_stOv4689Modes[u8ModeIdx].u8Flipping
            );
        return STF_ERROR_NOT_SUPPORTED;
    }

#if defined(COPY_REGS)
    if (pstSensorCam->pu8CurrentSensorModeReg) {
        STF_FREE(pstSensorCam->pu8CurrentSensorModeReg);
    }
#endif //COPY_REGS
    pstSensorCam->u32Registers = u32Registers;

#if defined(COPY_REGS)
    /* we want to apply all the registers at once - need to copy them */
    pstSensorCam->pu8CurrentSensorModeReg =
        (STF_U8 *)STF_MALLOC(pstSensorCam->u32Registers * 3);
    STF_MEMCPY(
        pstSensorCam->pu8CurrentSensorModeReg,
        pu8ModeReg,
        pstSensorCam->u32Registers * 3
        );
#else
    /* no need to copy the register as we will apply them one by one */
    pstSensorCam->pu8CurrentSensorModeReg = (STF_U8 *)pu8ModeReg;
#endif //COPY_REGS

    Ret = Sensor_GetModeInfo(
        pstSensorCam,
        u8ModeIdx,
        &pstSensorCam->stSensorMode
        );
    if (Ret) {
        LOG_ERROR("failed to get mode %d initial information!\n", u8ModeIdx);
        return STF_ERROR_FATAL;
    }

#ifdef DO_SETUP_IN_ENABLE
    /* because we don't do the actual setup we do not have real
     * information about the sensor - fake it */
    pstSensorCam->u32Exposure = pstSensorCam->stSensorMode.u32ExposureMin * 302;
    pstSensorCam->u32Exposure = STF_MIN_INT(
        pstSensorCam->stSensorMode.u32ExposureMax, STF_MAX_INT(
            pstSensorCam->u32Exposure, pstSensorCam->stSensorMode.u32ExposureMin
            )
        );
#else
    Sensor_ConfigRegister(pstSensorCam);
#endif //DO_SETUP_IN_ENABLE

    /* Init sensor status */
    pstSensorCam->bEnabled = STF_FALSE;
    pstSensorCam->u8ModeId = u8ModeIdx;
    pstSensorCam->u8Flipping = u8Flipping;
    pstSensorCam->u16CurrentFocus = g_OV4689_u16FocusDacMin; // minimum focus

    /* Init ISP gasket params */
    pstSensorCam->pstSensorPhy->psGasket->u8Gasket = pstSensorCam->u8Imager;
    pstSensorCam->pstSensorPhy->psGasket->bParallel = STF_TRUE;
    pstSensorCam->pstSensorPhy->psGasket->uiWidth =
        pstSensorCam->stSensorMode.u16Width - 1;
    pstSensorCam->pstSensorPhy->psGasket->uiHeight =
        pstSensorCam->stSensorMode.u16Height - 1;
#if 0
    pstSensorCam->pstSensorPhy->psGasket->bVSync =
        g_stSensorModes[u8ModeIdx].u8VSync;
    pstSensorCam->pstSensorPhy->psGasket->bHSync =
        g_stSensorModes[u8ModeIdx].u8HSync;
#else
    pstSensorCam->pstSensorPhy->psGasket->bVSync =
        (g_stInterface.u8SensorPolarity & 0x01) ? (STF_TRUE) : (STF_FALSE);
    pstSensorCam->pstSensorPhy->psGasket->bHSync =
        (g_stInterface.u8SensorPolarity & 0x02) ? (STF_TRUE) : (STF_FALSE);
#endif //0
    pstSensorCam->pstSensorPhy->psGasket->u8ParallelBitdepth =
        pstSensorCam->stSensorMode.u8BitDepth;
    V_LOG_DEBUG("gasket=%d, width=%d, height=%d, vsync=%d, hsync=%d, "\
        "bitdepth=%d\n",
        pstSensorCam->pstSensorPhy->psGasket->u8Gasket,
        pstSensorCam->pstSensorPhy->psGasket->uiWidth,
        pstSensorCam->pstSensorPhy->psGasket->uiHeight,
        pstSensorCam->pstSensorPhy->psGasket->bVSync,
        pstSensorCam->pstSensorPhy->psGasket->bHSync,
        pstSensorCam->pstSensorPhy->psGasket->u8ParallelBitdepth
        );
    usleep(1000);

    return STF_SUCCESS;
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
    static const STF_U8 au8EnableRegs[] = {
#if 0//ndef DO_SETUP_IN_ENABLE
        //set sensor to streaming mode
        0x01, 0x00, 0x01, // SC_CTRL0100 streaming
        0x30, 0x1A, 0xF9, // SC_CMMN_CLKRST0 as recommended by OmniVision
        DELAY_REG, 15, DELAY_REG,// 15 ms
        0x30, 0x1A, 0xF1, // SC_CMMN_CLKRST0
        0x48, 0x05, 0x00, // MIPI_CTRL_05
        0x30, 0x1A, 0xF0, // SC_CMMN_CLKRST0
#endif //DO_SETUP_IN_ENABLE
        //0x32, 0x0b, 0x0, // GRP_SWCTRL select group 0
        //0x32, 0x08, 0xe0 // GROUP_ACCESS quick launch group 0
        0x01, 0x00, 0x01 // streaming enable
    };
    static const STF_U8 au8DisableRegs[] = {
#if 0
        0x01, 0x00, 0x00, // SC_CTRL0100 set to sw standby
        DELAY_REG, 200, DELAY_REG, // 200ms delay
        0x30, 0x1A, 0xF9, // SC_CMMN_CLKRST0 as recommended by omnivision
        0x48, 0x05, 0x03, // MIPI_CTRL_05 retime manu/sel to manual
        //DELAY_REG, 10, DELAY_REG // 10ms delay
#else
        0x01, 0x00, 0x00
#endif //0
    };

    /* Sensor specific operation */
    if (bEnable) {
        Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            nI2c,
            au8EnableRegs,
            sizeof(au8EnableRegs)
            );
        usleep(2000);
    } else {
        Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            nI2c,
            au8DisableRegs,
            sizeof(au8DisableRegs)
            );
        usleep(2000);
    }

    return STF_SUCCESS;
}

static STF_RESULT sEnable(
    SENSOR_HANDLE *pstHandle
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    V_LOG_INFO("<===========\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    pstSensorCam->pstSensorPhy->psGasket->bParallel = STF_FALSE;
    pstSensorCam->pstSensorPhy->psGasket->u8Gasket = pstSensorCam->u8Imager;
    //LOG_INFO("u8MipiLanes=%d\n", pstSensorCam->stSensorMode.u8MipiLanes);
    Ret = SensorPhyCtrl(
        pstSensorCam->pstSensorPhy,
        STF_TRUE,
        pstSensorCam->stSensorMode.u8MipiLanes,
        0
        );
    if (Ret) {
        LOG_ERROR("SensorPhyCtrl failed\n");
        return Ret;
    }

#ifdef DO_SETUP_IN_ENABLE
    // because we didn't do the actual setup in setMode we have to do it now
    // it may start the sensor so we have to do it after the gasket enable
    /// @ fix conversion from float to uint
    Sensor_ConfigRegister(pstSensorCam);
#endif //DO_SETUP_IN_ENABLE

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
#if !defined(SIFIVE_ISP)
    sSetFocus(pstHandle, pstSensorCam->u16CurrentFocus);
#endif //SIFIVE_ISP
    sSetFlipMirror(pstHandle, pstSensorCam->u8Flipping);
    //LOG_DEBUG("camera enabled\n");

    return STF_SUCCESS;
}

static STF_RESULT sDisable(
    SENSOR_HANDLE *pstHandle
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    V_LOG_INFO("===========>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    if (pstSensorCam->bEnabled) {
        int delay = 0;

        //LOG_INFO("Disabling OV4689 MIPI camera\n");
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
        delay = (int)floor((1.0 / pstSensorCam->stSensorMode.dFrameRate)
            * 1000 * 1000);
        LOG_DEBUG("delay of %uus between disabling sensor/phy\n", delay);
        usleep(delay);
        SensorPhyCtrl(pstSensorCam->pstSensorPhy, STF_FALSE, 0, 0);
        V_LOG_INFO("Sensor disabled!\n");
    }

    return STF_SUCCESS;
}

static STF_RESULT sDestroy(
    SENSOR_HANDLE *pstHandle
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    LOG_DEBUG("Destroying OV4689 MIPI camera\n");
    /* remember to free anything that might have been allocated dynamically
     * (like extended params...)*/
    if (pstSensorCam->bEnabled) {
        sDisable(pstHandle);
    }

    SensorPhyDeinit(pstSensorCam->pstSensorPhy);

    if (pstSensorCam->pu8CurrentSensorModeReg) {
#if defined(COPY_REGS)
        STF_FREE(pstSensorCam->pu8CurrentSensorModeReg);
#endif //COPY_REGS
        pstSensorCam->pu8CurrentSensorModeReg = NULL;
    }
    close(pstSensorCam->fdI2c);
    STF_FREE(pstSensorCam);

    return STF_SUCCESS;
}

static STF_RESULT sGetInfo(
    SENSOR_HANDLE *pstHandle,
    SENSOR_INFO *pstInfo
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U16 u16Id = 0;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pstInfo);

    STF_ASSERT(strlen(OV4689MIPI_SENSOR_INFO_NAME) < SENSOR_INFO_NAME_MAX);
    STF_ASSERT(strlen(OV4689_SENSOR_VERSION) < SENSOR_INFO_VERSION_MAX);

    pstInfo->enBayerOriginal = SENSOR_BAYER_FORMAT;
    pstInfo->enBayerEnabled = pstInfo->enBayerOriginal;
//    // assumes that when flipping changes the bayer pattern
//    if (SENSOR_FLIP_NONE != pstInfo->stStatus.u8Flipping) {
//        pstInfo->enBayerEnabled = MosaicFlip(
//            pstInfo->enBayerOriginal,
//#if 1
//            0,
//#else
//            (pstInfo->stStatus.u8Flipping & SENSOR_FLIP_HORIZONTAL) ? (1) : (0),
//#endif
//            (pstInfo->stStatus.u8Flipping & SENSOR_FLIP_VERTICAL) ? (1) : (0)
//            );
//        // bayer format is not affected by flipping since we change
//        // registers to 6 instead of 2
//    }
    sprintf(pstInfo->pszSensorName, OV4689MIPI_SENSOR_INFO_NAME);
#ifndef NO_DEV_CHECK
  #if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
  #endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
    Ret = Sensor_I2cRead16(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_SC_CMMN_CHIP_ID_0,
        &u16Id
        );
#else
    Ret = 1;
#endif //NO_DEV_CHECK
    if (!Ret) {
        sprintf(pstInfo->pszSensorVersion, "0x%x", u16Id);
    } else {
        sprintf(pstInfo->pszSensorVersion, OV4689_SENSOR_VERSION);
    }
    pstInfo->dNumber = 1.2;
    pstInfo->u16FocalLength = 30;
    pstInfo->u32WellDepth = 6040;
    // bitdepth is a mode information
    pstInfo->dReadNoise = 5.0;
    pstInfo->u8Imager = pstSensorCam->u8Imager;
    pstInfo->bBackFacing = STF_TRUE;
    // other information should be filled by sGetInfo()
    pstInfo->u32ModeCount = ARRAY_SIZE(g_stOv4689Modes);
    pstInfo->enExposureGainMethod = OV4689_EXPO_GAIN_METHOD;
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
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U8 u8RegValue = 0;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //V_LOG_DEBUG(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu16RegVal);

#if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
#endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
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
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U8 u8Regs[] = {
        0x32, 0x08, 0x00
    };

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    u8Regs[0] = (u16RegAddr >> 8);
    u8Regs[1] = u16RegAddr & 0xFF;
    u8Regs[2] = u16RegVal & 0xFF;
    Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8Regs,
        sizeof(u8Regs)
        );

    return STF_SUCCESS;
}

static STF_U32 Sensor_ComputeGains(
    STF_DOUBLE dGain
    )
{
    STF_U32 u32Gain = 0x0080;
#if defined(USING_GAIN_TABLE)
    STF_U16 nIdx = 0; // if dGain <= 1.0
#endif //#if defined(USING_GAIN_TABLE)

#if defined(USING_GAIN_TABLE)
    if (SENSOR_MIN_GAIN < dGain) {
        nIdx = (STF_U16)floor((dGain - SENSOR_MIN_GAIN) * 16.0);
    }
    nIdx = STF_MIN_INT(
        nIdx,
        sizeof(g_OV4689_u16AecLongGainVal) / sizeof(STF_U16) - 1
        );

    LOG_DEBUG("gain nIdx=%u from %lf\n",
        nIdx,
        ((dGain - SENSOR_MIN_GAIN) * SENSOR_MAX_GAIN)
        );
    u32Gain = g_OV4689_u16AecLongGainVal[nIdx];
#else
    if (dGain < 2.0) {
        u32Gain = dGain * 128;
    } else if (dGain < 4.0) {
        u32Gain = 256 + (dGain * 64 - 8);
    } else if (dGain < 8.0) {
        u32Gain = 256 * 3 + (dGain * 32 - 12);
    } else {
        u32Gain = 256 * 7 + (dGain * 16 - 8);
    }
#endif //#if defined(USING_GAIN_TABLE)

    return u32Gain;
}

static STF_DOUBLE Sensor_ComputeRegToGains(
    STF_U32 u32Gain
    )
{
    STF_DOUBLE dGain = 1.0;

    if (0x0178 > u32Gain) {
        dGain = u32Gain / 128.0;
    } else if (0x0374 > u32Gain) {
        dGain = (u32Gain - 256 + 8) / 64.0;
    } else if (0x0778 > u32Gain) {
        dGain = (u32Gain - (256 * 3) + 12) / 32.0;
    } else {
        dGain = (u32Gain - (256 * 7) + 8) / 16.0;
    }

    return dGain;
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

    *pdMin = SENSOR_MIN_GAIN;
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
    ST_OV4689_CAM *pstSensorCam = NULL;
#if defined(GET_CURRENT_FROM_SENSOR)
    STF_U8 u8Gain;
    STF_U32 u32Gain;
#endif //GET_CURRENT_FROM_SENSOR

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //V_LOG_DEBUG(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pdCurrentGain);

#if defined(GET_CURRENT_FROM_SENSOR)
  #if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
  #endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_GAIN + 0x00,
        &u8Gain
        );
    u32Gain = u8Gain & 0x03;
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_GAIN + 0x01,
        &u8Gain
        );
    u32Gain = (u32Gain << 8) | u8Gain;
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_GAIN + 0x02,
        &u8Gain
        );
    u32Gain = (u32Gain << 8) | u8Gain;
    *pdCurrentGain = Sensor_ComputeRegToGains(u32Gain);
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
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U8 u8GainRegs[] = {
        0x32, 0x08, 0x00,  // start hold of group 0

        0x35, 0x07, 0x00,  // Gain register
        0x35, 0x08, 0x00,  //
        0x35, 0x09, 0x00,  //

        0x32, 0x08, 0x10,  // end hold of group 1 register writes
        0x32, 0x0b, 0x00,  // set quick manual mode
        0x32, 0x08, 0xe0,  // quick launch group 1
    };
    STF_U32 u32Gain;
    STF_DOUBLE dActualGain;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    if (SENSOR_MAX_GAIN < dGain) {
        dGain = SENSOR_MAX_GAIN;
    } else if (SENSOR_MIN_GAIN > dGain) {
        dGain = SENSOR_MIN_GAIN;
    }

    if (dGain == pstSensorCam->dGain) {
        return STF_SUCCESS;
    }

    pstSensorCam->dGain = dGain;

    u32Gain = Sensor_ComputeGains(dGain);
    u8GainRegs[5] = (u32Gain >> 16) & 0x03;
    u8GainRegs[8] = (u32Gain >> 8) & 0xff;
    u8GainRegs[11] = u32Gain & 0xff;
    dActualGain = Sensor_ComputeRegToGains(u32Gain);
    Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8GainRegs,
        sizeof(u8GainRegs)
        );
    V_LOG_DEBUG("dGain=%lf, register value=0x%02X%02X%02X, actual gain=%lf\n",
        dGain, u8GainRegs[5], u8GainRegs[8], u8GainRegs[11], dActualGain);
    g_OV4689_u8GainValHiBackup = u8GainRegs[5];
    g_OV4689_u8GainValMidBackup = u8GainRegs[8];
    g_OV4689_u8GainValLoBackup = u8GainRegs[11];

    return STF_SUCCESS;
}

static STF_U32 Sensor_ComputeExposure(
    STF_U32 u32Exposure,
    STF_DOUBLE dExposureMin
    )
{
    STF_U32 u32ExposureLines;

#if defined(ENABLE_EXPOSURE_TIME_FRACTION)
    u32ExposureLines = (STF_U32)((u32Exposure << 4) / dExposureMin);
#else
    u32ExposureLines = (STF_U32)(u32Exposure / dExposureMin);
#endif //#if defined(ENABLE_EXPOSURE_TIME_FRACTION)
    if (1 > u32ExposureLines) {
        u32ExposureLines = 1;
    }
    LOG_DEBUG("Exposure Val 0x%x\n", u32ExposureLines);

    return u32ExposureLines;
}

static STF_RESULT sGetExposureRange(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Min,
    STF_U32 *pu32Max,
    STF_U8 *pu8Contexts
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu32Min);
    STF_ASSERT(pu32Max);
    STF_ASSERT(pu8Contexts);

    *pu32Min = pstSensorCam->stSensorMode.u32ExposureMin;
    *pu32Max = pstSensorCam->stSensorMode.u32ExposureMax;
    *pu8Contexts = 1;

    return STF_SUCCESS;
}

static STF_RESULT sGetExposure(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Exposure,
    STF_U8 u8Context
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;
#if defined(GET_CURRENT_FROM_SENSOR)
    STF_U8 u8Exposure;
    STF_U32 u32ExposureLines;
#endif //GET_CURRENT_FROM_SENSOR

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //V_LOG_DEBUG(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu32Exposure);

#if defined(GET_CURRENT_FROM_SENSOR)
  #if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
  #endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_EXPOSURE + 0x00,
        &u8Exposure
        );
    u32ExposureLines = u8Exposure & 0x0f;
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_EXPOSURE + 0x01,
        &u8Exposure
        );
    u32ExposureLines = (u32ExposureLines << 8) | u8Exposure;
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_EXPOSURE + 0x02,
        &u8Exposure
        );
    u32ExposureLines = (u32ExposureLines << 8) | u8Exposure;
    *pu32Exposure =
        (STF_U32) (u32ExposureLines * pstSensorCam->stSensorMode.dExposureMin
            / 16.0);
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
    STF_U8 u8FpsRegs[] = {
        0x32, 0x08, 0x00, // GROUP_ACCESS start hold of group 0

        0x38, 0x0E, 0x00, // Vertical total size register
        0x38, 0x0F, 0x00, //

        0x32, 0x08, 0x10, // end hold of group 0 register writes
        0x32, 0x0b, 0x00, // set quick manual mode
        0x32, 0x08, 0xe0, // quick launch group 0
    };
    STF_U8 u8ExposureRegs[] = {
        0x32, 0x08, 0x00, // GROUP_ACCESS start hold of group 0

        0x35, 0x00, 0x00, // Exposure register
        0x35, 0x01, 0x2E, //
        0x35, 0x02, 0x80, //

        0x32, 0x08, 0x10, // end hold of group 0 register writes
        0x32, 0x0b, 0x00, // set quick manual mode
        0x32, 0x08, 0xe0, // quick launch group 0
    };
	ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U16 u16VerticalTotal;
    STF_U32 u32ExposureLines;
    STF_U32 u32ExposureLinesInt;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

#if 1
    if (SENSOR_MAX_EXPOSURE < u32Exposure) {
        u32Exposure = SENSOR_MAX_EXPOSURE;
    }
#else
    if (pstSensorCam->stSensorMode.u32ExposureMax <= u32Exposure) {
        u32Exposure = pstSensorCam->stSensorMode.u32ExposureMax;
    }
#endif //1

    pstSensorCam->u32Exposure = u32Exposure;

    u32ExposureLines = Sensor_ComputeExposure(
        pstSensorCam->u32Exposure,
        pstSensorCam->stSensorMode.dExposureMin
        );
#if defined(ENABLE_EXPOSURE_TIME_FRACTION)
    u32ExposureLinesInt = u32ExposureLines >> 4;
    if (u32ExposureLines & 0x0F) {
        u32ExposureLinesInt + 1;
    }
#else
    u32ExposureLinesInt = u32ExposureLines;
#endif //#if defined(ENABLE_EXPOSURE_TIME_FRACTION)

#if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)
    if (u32ExposureLinesInt > g_u32VerticalTotal - 4) {
        u16VerticalTotal = u32ExposureLinesInt + 4;
        //u16VerticalTotal = pstSensorCam->stSensorMode.u16VerticalTotal
        //    * pstSensorCam->u32FixedFps / fps;
        u8FpsRegs[5] = (u16VerticalTotal >> 8);
        u8FpsRegs[8] = (u16VerticalTotal & 0xff);
        Sensor_I2cWriteRegs(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8FpsRegs,
            sizeof(u8FpsRegs)
            );
        pstSensorCam->dCurFps = pstSensorCam->stSensorMode.u16VerticalTotal
            * pstSensorCam->u32FixedFps / u16VerticalTotal;
    }
  #if defined(ENABLE_EXPOSURE_TIME_FRACTION)
    u8ExposureRegs[5] = (u32ExposureLines >> 16) & 0xf;
    u8ExposureRegs[8] = (u32ExposureLines >> 8) & 0xff;
    u8ExposureRegs[11] = u32ExposureLines & 0xff;
  #else
    u8ExposureRegs[5] = (u32ExposureLinesInt >> 12) & 0xf;
    u8ExposureRegs[8] = (u32ExposureLinesInt >> 4) & 0xff;
    u8ExposureRegs[11] = (u32ExposureLinesInt << 4) & 0xff;
  #endif //#if defined(ENABLE_EXPOSURE_TIME_FRACTION)
#else
    if (u32ExposureLinesInt > g_u32VerticalTotal - 4) {
        u32ExposureLinesInt = g_u32VerticalTotal - 4;
    }
    u8ExposureRegs[5] = (u32ExposureLinesInt >> 12) & 0xf;
    u8ExposureRegs[8] = (u32ExposureLinesInt >> 4) & 0xff;
    u8ExposureRegs[11] = (u32ExposureLinesInt << 4) & 0xff;
#endif //#if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)

    Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8ExposureRegs,
        sizeof(u8ExposureRegs)
        );
    V_LOG_DEBUG("SetExposure. time=%d us, lines = %d, lines_int = %d\n",
        pstSensorCam->u32Exposure, u32ExposureLines, u32ExposureLinesInt);

    g_OV4689_u8LinesValHiBackup = u8ExposureRegs[5];
    g_OV4689_u8LinesValMidBackup = u8ExposureRegs[8];
    g_OV4689_u8LinesValLoBackup = u8ExposureRegs[11];

    return STF_SUCCESS;
}

/*
 * calculate focus position from points and return DAC value
 */
STF_U16 Sensor_ComputeFocus(
    const STF_U16 *pu16DACDist,
    const STF_U16 *pu16DACValues,
    STF_U8 u8Entries,
    STF_U16 u16Requested
    )
{
    STF_DOUBLE dMinDistRcp;
    STF_DOUBLE dMaxDistRcp;
    STF_DOUBLE dRequestedRcp;
    STF_DOUBLE dM;
    STF_DOUBLE dC;
    int i;

    /* find the entries between which the requested distance fits
     * calculate the equation of the line between those point for the
     * reciprocal of the distance
     * special casing the case where the requested point lies on one of the
     * points of the curve*/
    if (u16Requested == pu16DACDist[0]) {
        return pu16DACValues[0];
    }
    for (i = 0; i < u8Entries - 1; i++) {
        if (u16Requested == pu16DACDist[i + 1]) {
            return pu16DACValues[i + 1];
        }
        if (u16Requested >= pu16DACDist[i]
            && u16Requested <= pu16DACDist[i + 1]) {
            break;
        }
    }
    dMinDistRcp = 1.0 / pu16DACDist[i];
    dMaxDistRcp = 1.0 / pu16DACDist[i + 1];
    dRequestedRcp = 1.0 / u16Requested;

    dM = (pu16DACValues[i] - pu16DACValues[i + 1])
        / (dMinDistRcp - dMaxDistRcp);
    dC = -(dM * dMinDistRcp - pu16DACValues[i]);

    return (STF_U16)(dM * dRequestedRcp + dC);
}

static STF_RESULT sGetFocusRange(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16Min,
    STF_U16 *pu16Max
    )
{
//    ST_OV4689_CAM *pstSensorCam = NULL;

//    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);
//
//    TUNE_SLEEP(1);
//    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu16Min);
    STF_ASSERT(pu16Max);

    *pu16Min = g_OV4689_u16FocusDacMin;
    *pu16Max = g_OV4689_u16FocusDacMax;

    return STF_SUCCESS;
}

static STF_RESULT sGetCurrentFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16Current
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu16Current);

    *pu16Current = pstSensorCam->u16CurrentFocus;

    return STF_SUCCESS;
}

// declared beforehand
STF_RESULT sSetFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Focus
    )
{
#if 0
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U8 ui8Regs[4];
    STF_U16 ui16DACVal;
    STF_U16 nIdx;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    pstSensorCam->u16CurrentFocus = u16Focus;
    if (pstSensorCam->u16CurrentFocus >= g_OV4689_u16FocusDacMax) {
        // special case the infinity as it doesn't fit in with the rest
        ui16DACVal = 0;
    } else {
        ui16DACVal = Sensor_ComputeFocus(
            g_OV4689_u16FocusDacDist,
            g_OV4689_u16FocusDacVal,
            sizeof(g_OV4689_u16FocusDacDist) / sizeof(STF_U16),
            u16Focus
            );
    }

    ui8Regs[0] = 4;
    ui8Regs[1] = ui16DACVal >> 8;
    ui8Regs[2] = 5;
    ui8Regs[3] = ui16DACVal & 0xff;

    if (ioctl(pstSensorCam->fdI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, OV4689_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C slave address!\n");
        return STF_ERROR_BUSY;
    }

    for (nIdx = 0; nIdx < sizeof(ui8Regs); nIdx += 2) {
        STF_U8 *pu8Data = NULL;

        pu8Data = ui8Regs + nIdx;
        if (2 != write(pstSensorCam->fdI2c, pu8Data, 2)) {
            LOG_ERROR("Failed to write I2C data!\n");
            return STF_ERROR_BUSY;
        }
    }
#endif //0

    return STF_SUCCESS;
}

static STF_RESULT sSetFlipMirror(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8Flag
    )
{
    STF_U8 au8FlipMirrorRegs[] = {
        0x38, 0x21, 0x00,
        0x38, 0x20, 0x00
    };
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U16 ui16Regs;
    STF_RESULT ret;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

#if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
#endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
    switch (u8Flag) {
        default:
        case SENSOR_FLIP_NONE:
            ui16Regs = 0x3821;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[2]
                );
            au8FlipMirrorRegs[2] &= ~0x06; //clear mirror bit.

            ui16Regs = 0x3820;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[5]
                );
            au8FlipMirrorRegs[5] &= ~0x06; //clear flip bit.

            Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                au8FlipMirrorRegs,
                sizeof(au8FlipMirrorRegs)
                );
            pstSensorCam->u8Flipping = SENSOR_FLIP_NONE;
            break;

        case SENSOR_FLIP_HORIZONTAL:
            ui16Regs = 0x3821;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[2]
                );
            au8FlipMirrorRegs[2] |= 0x06; //set mirror bit.

            ui16Regs = 0x3820;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[5]
                );
            au8FlipMirrorRegs[5] &= ~0x06; //clear flip bit.

            Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                au8FlipMirrorRegs,
                sizeof(au8FlipMirrorRegs)
                );
            pstSensorCam->u8Flipping = SENSOR_FLIP_HORIZONTAL;
            break;

        case SENSOR_FLIP_VERTICAL:
            ui16Regs = 0x3821;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[2]
                );
            au8FlipMirrorRegs[2] &= ~0x06; //clear mirror bit.

            ui16Regs = 0x3820;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[5]
                );
            au8FlipMirrorRegs[5] |= 0x06; //set flip bit.

            Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                au8FlipMirrorRegs,
                sizeof(au8FlipMirrorRegs)
                );
            pstSensorCam->u8Flipping = SENSOR_FLIP_VERTICAL;
            break;

        case SENSOR_FLIP_BOTH:
            ui16Regs = 0x3821;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[2]
                );
            au8FlipMirrorRegs[2] |= 0x06; //set mirror bit.

            ui16Regs = 0x3820;
            ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                ui16Regs,
                &au8FlipMirrorRegs[5]
                );
            au8FlipMirrorRegs[5] |= 0x06; //set flip bit.

            Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->pstSensorPhy->psConnection,
                pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                pstSensorCam->fdI2c,
                au8FlipMirrorRegs,
                sizeof(au8FlipMirrorRegs)
                );
            pstSensorCam->u8Flipping = SENSOR_FLIP_BOTH;
            break;
    }

    return STF_SUCCESS;
}

static STF_RESULT sGetFixedFPS(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16FixedFps
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    //V_LOG_DEBUG(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    if (NULL != pu16FixedFps) {
        *pu16FixedFps = (STF_U16)pstSensorCam->u32FixedFps;
        V_LOG_DEBUG("Fixed FPS=%d\n", *pu16FixedFps);
    }

    return STF_SUCCESS;
}

static STF_RESULT sSetFPS(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dFps
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U8 au8Regs[3];
    STF_DOUBLE  dRealFps = 0.0;
    STF_U32 u32Framlines = 0;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);
	
    TUNE_SLEEP(1);

    if (dFps > pstSensorCam->u32FixedFps) {
        dRealFps = pstSensorCam->u32FixedFps;
    } else {
        dRealFps = dFps;
    }

    if (dFps == pstSensorCam->dCurFps) {
        return STF_SUCCESS;
    }
    u32Framlines = (1 * 1000 * 1000)
        / (dRealFps * pstSensorCam->stSensorMode.u32ExposureMin);

    au8Regs[0] = 0x38;
    au8Regs[1] = 0x0e;
    au8Regs[2] = ((u32Framlines >> 8) & 0x7f);

    Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        au8Regs,
        3
        );
//#if defined(ADD_USLEEP_FOR_I2C_READ)
//    usleep(1);
//#endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
//    Sensor_I2cRead(
//#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
//        pstSensorCam->pstSensorPhy->psConnection,
//        pstSensorCam->pstSensorPhy->u8IspIdx,
//#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
//        pstSensorCam->fdI2c,
//        0x380e,
//        &au8Regs[2]
//        );

    au8Regs[0] = 0x38;
    au8Regs[1] = 0x0f;
    au8Regs[2] = u32Framlines & 0xff;

    Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        au8Regs,
        3
        );
//#if defined(ADD_USLEEP_FOR_I2C_READ)
//    usleep(1);
//#endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
//    Sensor_I2cRead(
//#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
//        pstSensorCam->pstSensorPhy->psConnection,
//        pstSensorCam->pstSensorPhy->u8IspIdx,
//#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
//        pstSensorCam->fdI2c,
//        0x380f,
//        &au8Regs[2]
//        );

    pstSensorCam->dCurFps = dRealFps;
    g_u32VerticalTotal = u32Framlines;

    return STF_SUCCESS;
}

static STF_RESULT sSetExposureAndGain(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    )
{
#if 0
    sSetExposure(pstHandle, u32Exposure, u8Context);
    sSetGain(pstHandle, dGain, u8Context);
#else
    STF_U8 u8FpsRegs[] = {
        0x32, 0x08, 0x00, // GROUP_ACCESS start hold of group 0

        0x38, 0x0E, 0x00, // Vertical total size register
        0x38, 0x0F, 0x00, //

        0x32, 0x08, 0x10, // end hold of group 0 register writes
        0x32, 0x0b, 0x00, // set quick manual mode
        0x32, 0x08, 0xe0, // quick launch group 0
    };
    STF_U8 u8Regs[] = {
        0x32, 0x08, 0x00, // GROUP_ACCESS start hold of group 0

        0x35, 0x00, 0x00, // Exposure register
        0x35, 0x01, 0x2E, //
        0x35, 0x02, 0x80, //

        0x35, 0x07, 0x00, // Gain register
        0x35, 0x08, 0x00, //
        0x35, 0x09, 0x00, //

        0x32, 0x08, 0x10, // end hold of group 0 register writes
        0x32, 0x0b, 0x00, // set quick manual mode
        0x32, 0x08, 0xe0, // quick launch group 0
    };
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_U16 u16VerticalTotal;
    STF_U32 u32ExposureLines;
    STF_U32 u32ExposureLinesInt;
    STF_U32 u32Gain;
    STF_DOUBLE dActualGain;

    pstSensorCam = container_of(pstHandle, ST_OV4689_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    V_LOG_DEBUG("u32Exposure=%d, dGain=%lf >>>>>>>>>>>>\n",
        u32Exposure, dGain);

    //01. Exposure part
    //
  #if 1
    if (SENSOR_MAX_EXPOSURE < u32Exposure) {
        u32Exposure = SENSOR_MAX_EXPOSURE;
    }
  #else
    if (pstSensorCam->stSensorMode.u32ExposureMax <= u32Exposure) {
        u32Exposure = pstSensorCam->stSensorMode.u32ExposureMax;
    }
  #endif //1

    pstSensorCam->u32Exposure = u32Exposure;

    u32ExposureLines = Sensor_ComputeExposure(
        pstSensorCam->u32Exposure,
        pstSensorCam->stSensorMode.dExposureMin
        );
  #if defined(ENABLE_EXPOSURE_TIME_FRACTION)
    u32ExposureLinesInt = u32ExposureLines >> 4;
    if (u32ExposureLines & 0x0F) {
        u32ExposureLinesInt + 1;
    }
  #else
    u32ExposureLinesInt = u32ExposureLines;
  #endif //#if defined(ENABLE_EXPOSURE_TIME_FRACTION)

  #if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)
    if (u32ExposureLinesInt > (g_u32VerticalTotal - 4)) {
        u16VerticalTotal = u32ExposureLinesInt + 4;
        //u16VerticalTotal = pstSensorCam->stSensorMode.u16VerticalTotal
        //    * pstSensorCam->u32FixedFps / fps;
        u8FpsRegs[5] = (u16VerticalTotal >> 8);
        u8FpsRegs[8] = (u16VerticalTotal & 0xff);
        Sensor_I2cWriteRegs(
    #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
    #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8FpsRegs,
            sizeof(u8FpsRegs)
            );
        pstSensorCam->dCurFps = pstSensorCam->stSensorMode.u16VerticalTotal
            * pstSensorCam->u32FixedFps / u16VerticalTotal;
    }
    #if defined(ENABLE_EXPOSURE_TIME_FRACTION)
    u8Regs[5] = (u32ExposureLines >> 16) & 0xf;
    u8Regs[8] = (u32ExposureLines >> 8) & 0xff;
    u8Regs[11] = u32ExposureLines & 0xff;
    #else
    u8Regs[5] = (u32ExposureLinesInt >> 12) & 0xf;
    u8Regs[8] = (u32ExposureLinesInt >> 4) & 0xff;
    u8Regs[11] = (u32ExposureLinesInt << 4) & 0xff;
    #endif //#if defined(ENABLE_EXPOSURE_TIME_FRACTION)
  #else
    if (u32ExposureLinesInt > (g_u32VerticalTotal - 4)) {
        u32ExposureLinesInt = g_u32VerticalTotal - 4;
    }
    u8Regs[5] = (u32ExposureLinesInt >> 12) & 0xf;
    u8Regs[8] = (u32ExposureLinesInt >> 4) & 0xff;
    u8Regs[11] = (u32ExposureLinesInt << 4) & 0xff;
  #endif //#if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)
    //V_LOG_DEBUG("Exposure - time=%d us, reg value=0x%02X%02X%02X, "\
    //    "lines = %d, line_int = %d\n",
    //    pstSensorCam->u32Exposure,
    //    u8Regs[5],
    //    u8Regs[8],
    //    u8Regs[11],
    //    u32ExposureLines,
    //    u32ExposureLinesInt
    //    );
    //LOG_INFO("g_u32VerticalTotal = %d, u32Exposure = %d, dExposureMin = %lf, u32ExposureLines = %d, u32ExposureLinesInt = %d\n",
    //    g_u32VerticalTotal, u32Exposure, pstSensorCam->stSensorMode.dExposureMin, u32ExposureLines, u32ExposureLinesInt);
    //LOG_INFO("stSensorMode.u16VerticalTotal = %d, u32FixedFps = %d, u16VerticalTotal = %d, dCurFps = %lf\n",
    //    pstSensorCam->stSensorMode.u16VerticalTotal, pstSensorCam->u32FixedFps, u16VerticalTotal, pstSensorCam->dCurFps);

    //02. Gain part
    //
    if (SENSOR_MAX_GAIN < dGain) {
        dGain = SENSOR_MAX_GAIN;
    } else if (SENSOR_MIN_GAIN > dGain) {
        dGain = SENSOR_MIN_GAIN;
    }
    pstSensorCam->dGain = dGain;
    u32Gain = Sensor_ComputeGains(dGain);
    u8Regs[14] = (u32Gain >> 16) & 0x03;
    u8Regs[17] = (u32Gain >> 8) & 0xff;
    u8Regs[20] = u32Gain & 0xff;
    dActualGain = Sensor_ComputeRegToGains(u32Gain);
    //V_LOG_DEBUG("Gain - dGain=%lf, reg value=0x%02X%02X%02X, "\
    //    "actual gain=%lf\n",
    //    dGain,
    //    u8Regs[14],
    //    u8Regs[17],
    //    u8Regs[20],
    //    dActualGain
    //    );
    //LOG_INFO("dGain = %lf, u32Gain = 0x%08X, dActualGain = %lf\n",
    //    dGain, u32Gain, dActualGain);

    //03. Program register to sensor part
    //
    Sensor_I2cWriteRegs(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8Regs,
        sizeof(u8Regs)
        );
#endif //0

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

static STF_VOID *Test(
    ST_OV4689_CAM *pstSensorCam
    )
{
    STF_U8 u8Regs[] = {
        0x32, 0x08, 0x00, // GROUP_ACCESS start hold of group 0

        0x38, 0x0E, 0x00, // Exposure register
        0x38, 0x0F, 0x00, //

        0x32, 0x08, 0x10, // end hold of group 0 register writes
        0x32, 0x0b, 0x00, // set quick manual mode
        0x32, 0x08, 0xe0, // quick launch group 0
    };
    STF_U32 u32FixedFps = 0;
    STF_U32 u32Fps = 20;
    STF_U16 u16VerticalTotal;
    STF_U16 u16TmpVal;
    int cnt, i;
    static STF_U16 BK_u16VerticalTotal = 0xFFFF;

    sleep(10);
    while (1) {
        //LOG_DEBUG("################ u32Fps = %d ################\n", u32Fps);
        u32FixedFps = pstSensorCam->u32FixedFps;
        if (u32FixedFps < u32Fps) {
            u32Fps = u32FixedFps;
        } else if (5 > u32Fps) {
            u32Fps = 5;
        }

        if (pstSensorCam->dCurFps == u32Fps) {
            //return STF_SUCCESS;
        }

        u16VerticalTotal = pstSensorCam->stSensorMode.u16VerticalTotal
            * pstSensorCam->u32FixedFps / u32Fps;
        u8Regs[5] = (u16VerticalTotal >> 8);
        u8Regs[8] = (u16VerticalTotal & 0xff);
        Sensor_I2cWriteRegs(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8Regs,
            sizeof(u8Regs)
            );
        //LOG_DEBUG("^^^^^^^^^^^^^^^^ 0x380e = %x, 0x380f = %x^^^^^^^^^^^^^^^^\n",
        //    u8Regs[5], u8Regs[8]);
        //LOG_DEBUG("^^^^^^^^^^^^^^^^ pstSensorCam->u32FixedFps = %d, u32Fps=%d"\
        //    "^^^^^^^^^^^^^^^^\n", pstSensorCam->u32FixedFps, u32Fps);
        //LOG_DEBUG("^^^^^^^^^^^^^^^^ u16VerticalTotal = %d, "\
        //    "pstSensorCam->stSensorMode->u16VerticalTotal = %d "\
        //    "^^^^^^^^^^^^^^^^\n", u16VerticalTotal,
        //    pstSensorCam->stSensorMode.u16VerticalTotal);
        pstSensorCam->dCurFps = u32Fps;

        sleep(10);
        u32Fps--;
        if (u32Fps < 8) {
            u32Fps = 20;
        }
    }

    return STF_SUCCESS;
}

STF_RESULT OV4689MIPI_Create(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    SENSOR_HANDLE **ppstHandle,
    STF_U8 u8Index
    )
{
    ST_OV4689_CAM *pstSensorCam = NULL;
    STF_CHAR szI2cDevPath[NAME_MAX];
    STF_CHAR szExtraCfg[64];
    STF_CHAR szAdaptor[64];
    STF_U32 u32I2cAddr = OV4689_I2C_ADDR;
    STF_INT nChn = 0;
    STF_U8 u8Imager = 1;
    STF_U16 u16ChipVersion;
    //pthread_t tid;
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
    u32I2cAddr = OV4689_I2C_ADDR;
    nChn = OV4689_I2C_CHN;
    u8Imager = 1;
    V_LOG_INFO("i2c-addr=0x%x, chn = %d\n", u32I2cAddr, nChn);
    sprintf(szAdaptor, "%s-%d", "i2c", nChn);
    sprintf(szExtraCfg, "%s%s%d-config.txt", EXTRA_CFG_PATH, "sensor", u8Index);
#endif //UNUSED_CODE_AND_VARIABLE

    V_LOG_INFO("**OV4689MIPI SENSOR**\n");
    TUNE_SLEEP(1);

    /* Init global variable */
    g_u8SensorModeNum = ARRAY_SIZE(g_stOv4689Modes);
    V_LOG_DEBUG("g_u8SensorModeNum=%d\n", g_u8SensorModeNum);

    pstSensorCam = (ST_OV4689_CAM *)STF_CALLOC(1, sizeof(ST_OV4689_CAM));
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

    pstSensorCam->stFuncs.GetCurrentFocus = sGetCurrentFocus;
    pstSensorCam->stFuncs.GetFocusRange = sGetFocusRange;
    pstSensorCam->stFuncs.SetFocus = sSetFocus;

    pstSensorCam->stFuncs.SetFlipMirror = sSetFlipMirror;

    pstSensorCam->stFuncs.GetFixedFPS = sGetFixedFPS;
    pstSensorCam->stFuncs.SetFPS = sSetFPS;

    pstSensorCam->stFuncs.SetExposureAndGain = sSetExposureAndGain;

    pstSensorCam->stFuncs.Reset = sReset;

    /* Init sensor config */
    pstSensorCam->u8Imager = u8Imager;
    //pstSensorCam->bUseSensorAecAgc = DEFAULT_USE_AEC_AGC;

    // customers should change the clock!
    pstSensorCam->dRefClock = 24 * 1000 * 1000;

    /* Init sensor state */
    pstSensorCam->bEnabled = STF_FALSE;
    pstSensorCam->u8ModeId = 0;
    pstSensorCam->u8Flipping = SENSOR_FLIP_HORIZONTAL;
    pstSensorCam->u32Exposure = 89 * 302;
    pstSensorCam->dGain = SENSOR_MIN_GAIN;
    pstSensorCam->fdI2c = -1;

    pstSensorCam->pu8CurrentSensorModeReg = NULL;
    pstSensorCam->u32Registers = 0;

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
#ifndef NO_DEV_CHECK
  #if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
  #endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
    Ret = Sensor_I2cRead16(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_SC_CMMN_CHIP_ID_0,
        &u16ChipVersion
        );
    if (Ret || (OV4689_CHIP_VERSION != u16ChipVersion)) {
        LOG_ERROR("Failed to ensure that the i2c device has a compatible "\
            "OV4689 sensor! Ret=%d chip_version=0x%x (expect chip 0x%x)\n",
            Ret, u16ChipVersion, OV4689_CHIP_VERSION);
        close(pstSensorCam->fdI2c);
        STF_FREE(pstSensorCam);
        *ppstHandle = NULL;
        return STF_ERROR_DEVICE_NOT_FOUND;
    }
#endif //NO_DEV_CHECK

#ifdef CONFIG_REG_DEBUG
    // clean file
    {
        FILE *f = fopen(CONFIG_REG_DEBUG, "w");
        fclose(f);
    }

#endif //CONFIG_REG_DEBUG
#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
  #if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
  #endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
	Ret = Sensor_I2cRead16(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
	    pstSensorCam->fdI2c,
	    REG_SC_CMMN_CHIP_ID_0,
	    &u16ChipVersion
	    );
	V_LOG_INFO("Check ov4689 chip version: 0x%x\n", u16ChipVersion);

#endif //#if defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    // need i2c to have been found to read mode
    Sensor_GetModeInfo(
        pstSensorCam,
        pstSensorCam->u8ModeId,
        &pstSensorCam->stSensorMode
        );

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

    //pthread_create(&tid, NULL, Test, pstSensorCam);

    return STF_SUCCESS;
}

