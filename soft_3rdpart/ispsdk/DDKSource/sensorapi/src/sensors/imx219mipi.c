/**
  ******************************************************************************
  * @file imx219mipi.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/16/2022
  * @brief IMX219 MIPI camera sensor implementation
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


#include "sensors/imx219mipi.h"

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

#define ENABLE_3280_2464_21fps_2lane
#define ENABLE_1920_1080_21fps_2lane
#define ENABLE_1640_1232_30fps_2lane
#define ENABLE_1280_720_30fps_2lane
#define ENABLE_640_480_30fps_2lane
//#define ENABLE_SENSOR_DIGITAL_GAIN

/* Sensor specific configuration */
#define SENSOR_NAME                 "IMX219_SENSOR"
#define LOG_TAG                     SENSOR_NAME
#include "stf_common/userlog.h"
#ifdef SIFIVE_ISP
#define DEV_PATH                    ("/dev/ddk_sensor")
#define EXTRA_CFG_PATH              ("/root/.ispddk/")
#endif //SIFIVE_ISP
#define IMX219_EXPO_GAIN_METHOD     (EN_EXPO_GAIN_METHOD_1ST_EXPO_2ND_GAIN)

/** @ the setup for mode does not work if enabled is not call just after
 * - temporarily do the whole of the setup in in enable function
 * - can remove that define and its checking once fixed
 */
// if defined writes all registers at enable time instead of configure
//#define DO_SETUP_IN_ENABLE

#if defined(V4L2_DRIVER)
//#define IMX219_I2C_CHN              (0)     // For JH7100 VisionFive
#define IMX219_I2C_CHN              (6)     // For JH7110
#else
#define IMX219_I2C_CHN              (0)
#endif //#if defined(V4L2_DRIVER)
#define IMX219_I2C_ADDR             (0x10)      // in 7-bits
#define IMX219_WRITE_ADDR           (0x20 >> 1)
#define IMX219_READ_ADDR            (0x21 >> 1)
#define MAX_I2C_BUF_SIZE            (256)

#define IMX219_SENSOR_VERSION       "not-verified"

#define IMX219_CHIP_VERSION         (0x0219)

// fake register value to interpret next value as delay in ms
#define DELAY_REG                   (0xFF)
// not a real register - marks the end of register array
#define STOP_REG                    (0xFE)

#define GET_CURRENT_FROM_SENSOR
#define IMX219_FLIP_MIRROR_FRIST
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
//#define CONFIG_REG_DEBUG "imx219_write.txt"

#ifdef WIN32 // on windows we do not need to sleep to wait for the bus
static void usleep(int t)
{
    (void)t;
}
#endif //#ifdef WIN32

typedef struct _ST_IMX219_CAM {
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
    STF_U32 u32FrameLength;
    STF_U16 u16VerticalTotal;

    /* Sensor config params */
    STF_U8 u8Imager;            // 0: DVP, 1: MIPI
    STF_DOUBLE dRefClock;       // in MHz
    STF_U32 u32InitClk;
    STF_U32 u32FixedFps;
} ST_IMX219_CAM, *PST_IMX219_CAM;

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
    .u8I2cChannel = IMX219_I2C_CHN,
    .u8I2cSlaveAddrbit = 0,                     // 7 bits.
    .u8I2cRegAddrBit = 1,                       // 16 bits.
    .u8I2cRegDataBit = 1,                       // 8 bits.
    .u16I2cSlaveAddr = IMX219_I2C_ADDR,
    .s16PowerGpioId = 21,
    .u16PowerGpioInitLevel = 0,
    .s16ResetGpioId = 20,
    .u16ResetGpioInitLevel = 1,
    .u16ResetIntervalTime = 200,
    .u8SensorPolarity = 0, // 0:EN_DVP_SYNC_POL_H_NEG_V_NEG, 1: EN_DVP_SYNC_POL_H_NEG_V_POS, 2:EN_DVP_SYNC_POL_H_POS_V_NEG, 3:EN_DVP_SYNC_POL_H_POS_V_POS.
};


// from OmniVision data sheet
#define SENSOR_BAYER_FORMAT		    MOSAIC_RGGB

#define SPLIT0(R)                   ((R >> 8) & 0xFF)
#define SPLIT1(R)                   (R & 0xFF)
#define IS_REG(arr, off, name) \
    ((SPLIT0(name) == arr[off+0]) && (SPLIT1(name) == arr[off+1]))

#define REG_IMG_ORIENTATION         0x0172 // used for mirrot and flipping

#define REG_H_CROP_START_0          0x3800 // bits [4:0] address [12:8]
#define REG_H_CROP_START_1          0x3801 // bits [7:0] address [7:0]
#define REG_V_CROP_START_0          0x3802 // bits [3:0] address [11:8]
#define REG_V_CROP_START_1          0x3803 // bits [7:0] address [7:0]
#define REG_H_CROP_END_0            0x3804 // bits [4:0] address [12:8]
#define REG_H_CROP_END_1            0x3805 // bits [7:0] address [7:0]
#define REG_V_CROP_END_0            0x3806 // bits [3:0] address [11:8]
#define REG_V_CROP_END_1            0x3807 // bits [7:0] address [7:0]

#define REG_X_OUTPUT_SIZE_0         0x016C // bits [3:0] size [11:8]
#define REG_X_OUTPUT_SIZE_1         0x016D // bits [7:0] size [7:0]
#define REG_Y_OUTPUT_SIZE_0         0x016E // bits [3:0] size [11:8]
#define REG_Y_OUTPUT_SIZE_1         0x016F // bits [7:0] size [7:0]

#define REG_LINE_LENGTH_0           0x0162 // bits [7:0] size [15:8]
#define REG_LINE_LENGTH_1           0x0163 // bits [7:0] size [7:0]
#define REG_FRM_LENGTH_0            0x0160 // bits [7:0] size [15:8]
#define REG_FRM_LENGTH_1            0x0161 // bits [7:0] size [7:0]

#define REG_BINNING_MODE_H          0x0174 // bits [1:0] defines binning mode (H-direction).
#define REG_BINNING_MODE_V          0x0175 // bits [1:0] defines binning mode (V-direction).

#define REG_CSI_DATA_FORMAT         0x018C // bits [7:0] for bitdepth
#define REG_CSI_LANE_MODE           0x0114 // bits [1:0] for CSI mipi lane mode

#define REG_EXCK_FREQ_0             0x012A // INCK frequency [MHz], bits [7:0] address [15:8]
#define REG_EXCK_FREQ_1             0x012B // INCK frequency [MHz], bits [7:0] address [7:0]
#define REG_PREPLLCK_VT_DIV         0x0304 // Pre PLL clock Video Timing System Divider Value, bits [7:0] Integer-N mode : prepllck_vt_div = 1, 2, 3. For example - 1: EXCK_FREQ 6 MHz to 12 MHz, 2: EXCK_FREQ 12 MHz to 24 MHz, 3: EXCK_FREQ 24 MHz to 27 MHz
#define REG_PREPLLCK_OP_DIV         0x0305 // Pre PLL clock Output System Divider Value, bits [7:0] Integer-N mode : prepllck_vt_div = 1, 2, 3. For example - 1: EXCK_FREQ 6 MHz to 12 MHz, 2: EXCK_FREQ 12 MHz to 24 MHz, 3: EXCK_FREQ 24 MHz to 27 MHz
#define REG_PLL_VT_MPY_0            0x0306 // PLL Video Timing System multiplier Value, bits [2:0] address [10:8]
#define REG_PLL_VT_MPY_1            0x0307 // PLL Video Timing System multiplier Value, bits [7:0] address [7:0]
#define REG_PLL_OP_MPY_0            0x030C // PLL Output System multiplier Value, bits [2:0] address [10:8]
#define REG_PLL_OP_MPY_1            0x030D // PLL Output System multiplier Value, bits [7:0] address [7:0]
#define REG_VTPXCK_DIV              0x0301 // Video Timing Pixel Clock Divider Value , bits [4:0]
#define REG_VTSYCK_DIV              0x0303 // Video Timing System Clock Divider Value, bits [1:0]
#define REG_OPPXCK_DIV              0x0309 // Output Pixel Clock Divider Value , bits [4:0]
#define REG_OPSYCK_DIV              0x030B // Output System Clock Divider Value, bits [1:0]

#define REG_FINE_INTEG_TIME_0       0x0388 // fine_integration_time, bits [7:0] address [15:8]
#define REG_FINE_INTEG_TIME_1       0x0389 // fine_integration_time, bits [7:0] address [7:0]

#define REG_MODEL_ID_0              0x0000 // high bits for model id
#define REG_MODEL_ID_1              0x0001 // low bits for model id

#define REG_EXPOSURE                0x015A
#define REG_ANA_GAIN                0x0157
#define REG_DIG_GAIN                0x0158


// values for PLL2_prediv 1/x, see sensor data-sheet
static const STF_DOUBLE pll_ctrl_b_val[] = {
    1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 6.0, 8.0
};

// values for PLL2_divs 1/x, see sensor data-sheet
static const STF_DOUBLE pll_ctrl_c_val[] = {
    1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0
};

static const STF_DOUBLE SENOSR_MIN_ANALOG_GAIN = 1.0;
static const STF_DOUBLE SENOSR_MAX_ANALOG_GAIN = 10.5;
static const STF_DOUBLE SENOSR_MIN_DIGITAL_GAIN = 1.0;
#if defined(ENABLE_SENSOR_DIGITAL_GAIN)
  #if 0
static const STF_DOUBLE SENOSR_MAX_DIGITAL_GAIN = 15.995;
  #else
static const STF_DOUBLE SENOSR_MAX_DIGITAL_GAIN = 2.0;
  #endif //#if 0
#else
static const STF_DOUBLE SENOSR_MAX_DIGITAL_GAIN = 1.0;
#endif //#if defined(ENABLE_SENSOR_DIGITAL_GAIN)
static const STF_DOUBLE SENSOR_MIN_GAIN = (SENOSR_MIN_ANALOG_GAIN * SENOSR_MIN_DIGITAL_GAIN);
static const STF_DOUBLE SENSOR_MAX_GAIN = (SENOSR_MAX_ANALOG_GAIN * SENOSR_MAX_DIGITAL_GAIN);
static const STF_U32 SENSOR_MIN_EXPOSURE = 1 * 1000;    // 1ms
#if 0
static const STF_U32 SENSOR_MAX_EXPOSURE = 200 * 1000;  // 200ms, 5fps.
#else
static const STF_U32 SENSOR_MAX_EXPOSURE = 100 * 1000;  // 100ms, 10fps.
#endif //#if 0

static STF_U32 g_u32VerticalTotal = 1000;

static STF_U32 g_IMX219_u32ExposureLinesBackup = 0x03E8;
static STF_U32 g_IMX219_u32AnalogGainBackup = 0x00;
static STF_U32 g_IMX219_u32DigitalGainBackup = 0x0100;



// minimum focus in millimetres
static const STF_U16 g_IMX219_u16FocusDacMin = 50;

// maximum focus in millimetres, if >= then focus is infinity
static const STF_U16 g_IMX219_u16FocusDacMax = 600;

// focus values for the g_IMX219_u16FocusDacDist
static const STF_U16 g_IMX219_u16FocusDacVal[] = {
    0x3ff, 0x180, 0x120, 0x100, 0x000
};

// distances in millimetres for the g_IMX219_u16FocusDacVal
static const STF_U16 g_IMX219_u16FocusDacDist[] = {
    50, 150, 300, 500, 600
};

//
// modes declaration
//

#if defined(ENABLE_3280_2464_21fps_2lane)
static STF_U8 g_u8ModeRegs_3280_2464_21fps_2lane[] = {
    //DELAY_REG, 0x0050, DELAY_REG,
    0x01, 0x00, 0x00,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x0c,
    0x30, 0x0a, 0xff,
    0x30, 0x0b, 0xff,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x09,
    0x01, 0x14, 0x01,
    0x01, 0x28, 0x00,
    0x01, 0x2a, 0x18,
    0x01, 0x2b, 0x00,
    0x01, 0x62, 0x0d,       // Line_Length_A = 0x0D78 = 3448
    0x01, 0x63, 0x78,
    0x01, 0x64, 0x00,       // X_ADD_STA_A = 0x0000 = 0
    0x01, 0x65, 0x00,
    0x01, 0x66, 0x0c,       // X_ADD_END_A = 0x0CCF = 3279
    0x01, 0x67, 0xcf,
    0x01, 0x68, 0x00,       // Y_ADD_STA_A = 0x0000 = 0
    0x01, 0x69, 0x00,
    0x01, 0x6a, 0x09,       // Y_ADD_END_A = 0x099F = 2463
    0x01, 0x6b, 0x9f,
    0x01, 0x6c, 0x0c,       // x_output_size = 0x0CD0 = 3280
    0x01, 0x6d, 0xd0,
    0x01, 0x6e, 0x09,       // y_output_size = 0x09A0 = 2464
    0x01, 0x6f, 0xa0,
    0x01, 0x70, 0x01,
    0x01, 0x71, 0x01,
    0x01, 0x74, 0x00,       // BINNING_MODE_H_A = 0x00 = No Binning
    0x01, 0x75, 0x00,       // BINNING_MODE_V_A = 0x00 = No Binning
    0x03, 0x01, 0x05,
    0x03, 0x03, 0x01,
    0x03, 0x04, 0x03,
    0x03, 0x05, 0x03,
    0x03, 0x06, 0x00,
    0x03, 0x07, 0x39,
    0x03, 0x0b, 0x01,
    0x03, 0x0c, 0x00,
    0x03, 0x0d, 0x72,
    0x06, 0x24, 0x0c,       // TP_WINDOW_WIDTH = 0x0CD0 = 3280
    0x06, 0x25, 0xd0,
    0x06, 0x26, 0x09,       // TP_WINDOW_HEIGHT = 0x09A0 = 2464
    0x06, 0x27, 0xa0,
    0x45, 0x5e, 0x00,
    0x47, 0x1e, 0x4b,
    0x47, 0x67, 0x0f,
    0x47, 0x50, 0x14,
    0x45, 0x40, 0x00,
    0x47, 0xb4, 0x14,
    0x47, 0x13, 0x30,
    0x47, 0x8b, 0x10,
    0x47, 0x8f, 0x10,
    0x47, 0x93, 0x10,
    0x47, 0x97, 0x0e,
    0x47, 0x9b, 0x0e,

    0x01, 0x57, 0x00,       // ANA_GAIN_GLOBAL_A
    0x01, 0x58, 0x01,       // DIG_GAIN_GLOBAL_A [11:8]
    0x01, 0x59, 0x00,       // DIG_GAIN_GLOBAL_A [7:0]
    0x01, 0x5a, 0x03,       // COARSE_INTEGRATION_TIME_A[15:8]
    0x01, 0x5b, 0xe8,       // COARSE_INTEGRATION_TIME_A[7:0]

    0x01, 0x72, 0x00,       // IMG_ORIENTATION_A

    0x01, 0x8c, 0x0a,       // CSI_DATA_FORMAT_A [15:8]
    0x01, 0x8d, 0x0a,       // CSI_DATA_FORMAT_A [7:0]
    0x03, 0x09, 0x0a,       // OPPXCK_DIV

    STOP_REG, STOP_REG, STOP_REG,
};

#endif //#if defined(ENABLE_3280_2464_21fps_2lane)
#if defined(ENABLE_1920_1080_21fps_2lane)
static STF_U8 g_u8ModeRegs_1920_1080_21fps_2lane[] = {
    //DELAY_REG, 0x0050, DELAY_REG,
    0x01, 0x00, 0x00,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x0c,
    0x30, 0x0a, 0xff,
    0x30, 0x0b, 0xff,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x09,
    0x01, 0x14, 0x01,
    0x01, 0x28, 0x00,
    0x01, 0x2a, 0x18,
    0x01, 0x2b, 0x00,
    0x01, 0x62, 0x0d,       // Line_Length_A = 0x0D78 = 3448
    0x01, 0x63, 0x78,
    0x01, 0x64, 0x02,       // X_ADD_STA_A = 0x02A8 = 680
    0x01, 0x65, 0xa8,
    0x01, 0x66, 0x0a,       // X_ADD_END_A = 0x0A27 = 2599
    0x01, 0x67, 0x27,
    0x01, 0x68, 0x02,       // Y_ADD_STA_A = 0x02B4 = 692
    0x01, 0x69, 0xb4,
    0x01, 0x6a, 0x06,       // Y_ADD_END_A = 0x06EB = 1771
    0x01, 0x6b, 0xeb,
    0x01, 0x6c, 0x07,       // x_output_size = 0x0780 = 1920
    0x01, 0x6d, 0x80,
    0x01, 0x6e, 0x04,       // y_output_size = 0x0438 = 1080
    0x01, 0x6f, 0x38,
    0x01, 0x70, 0x01,
    0x01, 0x71, 0x01,
    0x01, 0x74, 0x00,       // BINNING_MODE_H_A = 0x00 = No Binning
    0x01, 0x75, 0x00,       // BINNING_MODE_V_A = 0x00 = No Binning
    0x03, 0x01, 0x05,
    0x03, 0x03, 0x01,
    0x03, 0x04, 0x03,
    0x03, 0x05, 0x03,
    0x03, 0x06, 0x00,
    0x03, 0x07, 0x39,
    0x03, 0x0b, 0x01,
    0x03, 0x0c, 0x00,
    0x03, 0x0d, 0x72,
    0x06, 0x24, 0x07,       // TP_WINDOW_WIDTH = 0x0780 = 1920
    0x06, 0x25, 0x80,
    0x06, 0x26, 0x04,       // TP_WINDOW_HEIGHT = 0x0438 = 1080
    0x06, 0x27, 0x38,
    0x45, 0x5e, 0x00,
    0x47, 0x1e, 0x4b,
    0x47, 0x67, 0x0f,
    0x47, 0x50, 0x14,
    0x45, 0x40, 0x00,
    0x47, 0xb4, 0x14,
    0x47, 0x13, 0x30,
    0x47, 0x8b, 0x10,
    0x47, 0x8f, 0x10,
    0x47, 0x93, 0x10,
    0x47, 0x97, 0x0e,
    0x47, 0x9b, 0x0e,

    0x01, 0x57, 0x00,       // ANA_GAIN_GLOBAL_A
    0x01, 0x58, 0x01,       // DIG_GAIN_GLOBAL_A [11:8]
    0x01, 0x59, 0x00,       // DIG_GAIN_GLOBAL_A [7:0]
    0x01, 0x5a, 0x03,       // COARSE_INTEGRATION_TIME_A[15:8]
    0x01, 0x5b, 0xe8,       // COARSE_INTEGRATION_TIME_A[7:0]

    0x01, 0x72, 0x00,       // IMG_ORIENTATION_A

    0x01, 0x8c, 0x0a,       // CSI_DATA_FORMAT_A [15:8]
    0x01, 0x8d, 0x0a,       // CSI_DATA_FORMAT_A [7:0]
    0x03, 0x09, 0x0a,       // OPPXCK_DIV

    STOP_REG, STOP_REG, STOP_REG,
};

#endif //#if defined(ENABLE_1920_1080_21fps_2lane)
#if defined(ENABLE_1640_1232_30fps_2lane)
static STF_U8 g_u8ModeRegs_1640_1232_30fps_2lane[] = {
    //DELAY_REG, 0x0050, DELAY_REG,
    0x01, 0x00, 0x00,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x0c,
    0x30, 0x0a, 0xff,
    0x30, 0x0b, 0xff,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x09,
    0x01, 0x14, 0x01,
    0x01, 0x28, 0x00,
    0x01, 0x2a, 0x18,
    0x01, 0x2b, 0x00,
    0x01, 0x62, 0x0d,       // Line_Length_A = 0x0D78 = 3448
    0x01, 0x63, 0x78,
    0x01, 0x64, 0x00,       // X_ADD_STA_A = 0x0000 = 0
    0x01, 0x65, 0x00,
    0x01, 0x66, 0x0c,       // X_ADD_END_A = 0x0CCF = 3279
    0x01, 0x67, 0xcf,
    0x01, 0x68, 0x00,       // Y_ADD_STA_A = 0x0000 = 0
    0x01, 0x69, 0x00,
    0x01, 0x6a, 0x09,       // Y_ADD_END_A = 0x099F = 2463
    0x01, 0x6b, 0x9f,
    0x01, 0x6c, 0x06,       // x_output_size = 0x0668 = 1640
    0x01, 0x6d, 0x68,
    0x01, 0x6e, 0x04,       // y_output_size = 0x04D0 = 1232
    0x01, 0x6f, 0xd0,
    0x01, 0x70, 0x01,
    0x01, 0x71, 0x01,
    0x01, 0x74, 0x01,       // BINNING_MODE_H_A = 0x01 = X2 Binning
    0x01, 0x75, 0x01,       // BINNING_MODE_V_A = 0x01 = X2 Binning
    0x03, 0x01, 0x05,
    0x03, 0x03, 0x01,
    0x03, 0x04, 0x03,
    0x03, 0x05, 0x03,
    0x03, 0x06, 0x00,
    0x03, 0x07, 0x39,
    0x03, 0x0b, 0x01,
    0x03, 0x0c, 0x00,
    0x03, 0x0d, 0x72,
    0x06, 0x24, 0x06,       // TP_WINDOW_WIDTH = 0x0668 = 1640
    0x06, 0x25, 0x68,
    0x06, 0x26, 0x04,       // TP_WINDOW_HEIGHT = 0x04D0 = 1232
    0x06, 0x27, 0xd0,
    0x45, 0x5e, 0x00,
    0x47, 0x1e, 0x4b,
    0x47, 0x67, 0x0f,
    0x47, 0x50, 0x14,
    0x45, 0x40, 0x00,
    0x47, 0xb4, 0x14,
    0x47, 0x13, 0x30,
    0x47, 0x8b, 0x10,
    0x47, 0x8f, 0x10,
    0x47, 0x93, 0x10,
    0x47, 0x97, 0x0e,
    0x47, 0x9b, 0x0e,

    0x01, 0x57, 0x00,       // ANA_GAIN_GLOBAL_A
    0x01, 0x58, 0x01,       // DIG_GAIN_GLOBAL_A [11:8]
    0x01, 0x59, 0x00,       // DIG_GAIN_GLOBAL_A [7:0]
    0x01, 0x5a, 0x03,       // COARSE_INTEGRATION_TIME_A[15:8]
    0x01, 0x5b, 0xe8,       // COARSE_INTEGRATION_TIME_A[7:0]

    0x01, 0x72, 0x00,       // IMG_ORIENTATION_A

    0x01, 0x8c, 0x0a,       // CSI_DATA_FORMAT_A [15:8]
    0x01, 0x8d, 0x0a,       // CSI_DATA_FORMAT_A [7:0]
    0x03, 0x09, 0x0a,       // OPPXCK_DIV

    STOP_REG, STOP_REG, STOP_REG,
};

#endif //#if defined(ENABLE_1640_1232_30fps_2lane)
#if defined(ENABLE_1280_720_30fps_2lane)
static STF_U8 g_u8ModeRegs_1280_720_30fps_2lane[] = {
    //DELAY_REG, 0x0050, DELAY_REG,
    0x01, 0x00, 0x00,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x0c,
    0x30, 0x0a, 0xff,
    0x30, 0x0b, 0xff,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x09,
    0x01, 0x14, 0x01,
    0x01, 0x28, 0x00,
    0x01, 0x2a, 0x18,
    0x01, 0x2b, 0x00,
    0x01, 0x62, 0x0d,       // Line_Length_A = 0x0D78 = 3448
    0x01, 0x63, 0x78,
    0x01, 0x64, 0x01,       // X_ADD_STA_A = 0x0168 = 360
    0x01, 0x65, 0x68,
    0x01, 0x66, 0x0b,       // X_ADD_END_A = 0x0B67 = 2919
    0x01, 0x67, 0x67,
    0x01, 0x68, 0x02,       // Y_ADD_STA_A = 0x0200 = 512
    0x01, 0x69, 0x00,
    0x01, 0x6a, 0x07,       // Y_ADD_END_A = 0x079F = 1951
    0x01, 0x6b, 0x9f,
    0x01, 0x6c, 0x05,       // x_output_size = 0x0500 = 1280
    0x01, 0x6d, 0x00,
    0x01, 0x6e, 0x02,       // y_output_size = 0x02D0 = 720
    0x01, 0x6f, 0xd0,
    0x01, 0x70, 0x01,
    0x01, 0x71, 0x01,
    0x01, 0x74, 0x01,       // BINNING_MODE_H_A = 0x01 = X2 Binning
    0x01, 0x75, 0x01,       // BINNING_MODE_V_A = 0x01 = X2 Binning
    0x03, 0x01, 0x05,
    0x03, 0x03, 0x01,
    0x03, 0x04, 0x03,
    0x03, 0x05, 0x03,
    0x03, 0x06, 0x00,
    0x03, 0x07, 0x39,
    0x03, 0x0b, 0x01,
    0x03, 0x0c, 0x00,
    0x03, 0x0d, 0x72,
    0x06, 0x24, 0x06,       // TP_WINDOW_WIDTH = 0x0668 = 1640
    0x06, 0x25, 0x68,
    0x06, 0x26, 0x04,       // TP_WINDOW_HEIGHT = 0x04D0 = 1232
    0x06, 0x27, 0xd0,
    0x45, 0x5e, 0x00,
    0x47, 0x1e, 0x4b,
    0x47, 0x67, 0x0f,
    0x47, 0x50, 0x14,
    0x45, 0x40, 0x00,
    0x47, 0xb4, 0x14,
    0x47, 0x13, 0x30,
    0x47, 0x8b, 0x10,
    0x47, 0x8f, 0x10,
    0x47, 0x93, 0x10,
    0x47, 0x97, 0x0e,
    0x47, 0x9b, 0x0e,

    0x01, 0x57, 0x00,       // ANA_GAIN_GLOBAL_A
    0x01, 0x58, 0x01,       // DIG_GAIN_GLOBAL_A [11:8]
    0x01, 0x59, 0x00,       // DIG_GAIN_GLOBAL_A [7:0]
    0x01, 0x5a, 0x03,       // COARSE_INTEGRATION_TIME_A[15:8]
    0x01, 0x5b, 0xe8,       // COARSE_INTEGRATION_TIME_A[7:0]

    0x01, 0x72, 0x00,       // IMG_ORIENTATION_A

    0x01, 0x8c, 0x0a,       // CSI_DATA_FORMAT_A [15:8]
    0x01, 0x8d, 0x0a,       // CSI_DATA_FORMAT_A [7:0]
    0x03, 0x09, 0x0a,       // OPPXCK_DIV

    STOP_REG, STOP_REG, STOP_REG,
};

#endif //#if defined(ENABLE_1280_720_30fps_2lane)
#if defined(ENABLE_640_480_30fps_2lane)
static STF_U8 g_u8ModeRegs_640_480_30fps_2lane[] = {
    //DELAY_REG, 0x0050, DELAY_REG,
    0x01, 0x00, 0x00,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x0c,
    0x30, 0x0a, 0xff,
    0x30, 0x0b, 0xff,
    0x30, 0xeb, 0x05,
    0x30, 0xeb, 0x09,
    0x01, 0x14, 0x01,
    0x01, 0x28, 0x00,
    0x01, 0x2a, 0x18,
    0x01, 0x2b, 0x00,
    0x01, 0x62, 0x0d,       // Line_Length_A = 0x0D78 = 3448
    0x01, 0x63, 0x78,
    0x01, 0x64, 0x03,       // X_ADD_STA_A = 0x03E8 = 1000
    0x01, 0x65, 0xe8,
    0x01, 0x66, 0x08,       // X_ADD_END_A = 0x08E7 = 2279
    0x01, 0x67, 0xe7,
    0x01, 0x68, 0x02,       // Y_ADD_STA_A = 0x02F0 = 752
    0x01, 0x69, 0xf0,
    0x01, 0x6a, 0x06,       // Y_ADD_END_A = 0x06AF = 1711
    0x01, 0x6b, 0xaf,
    0x01, 0x6c, 0x02,       // x_output_size = 0x0280 = 640
    0x01, 0x6d, 0x80,
    0x01, 0x6e, 0x01,       // y_output_size = 0x01E0 = 480
    0x01, 0x6f, 0xe0,
    0x01, 0x70, 0x01,
    0x01, 0x71, 0x01,
    0x01, 0x74, 0x03,       // BINNING_MODE_H_A = 0x03 = X2 Analog (Special) Binning
    0x01, 0x75, 0x03,       // BINNING_MODE_V_A = 0x03 = X2 Analog (Special) Binning
    0x03, 0x01, 0x05,
    0x03, 0x03, 0x01,
    0x03, 0x04, 0x03,
    0x03, 0x05, 0x03,
    0x03, 0x06, 0x00,
    0x03, 0x07, 0x39,
    0x03, 0x0b, 0x01,
    0x03, 0x0c, 0x00,
    0x03, 0x0d, 0x72,
    0x06, 0x24, 0x06,       // TP_WINDOW_WIDTH = 0x0668 = 1640
    0x06, 0x25, 0x68,
    0x06, 0x26, 0x04,       // TP_WINDOW_HEIGHT = 0x04D0 = 1232
    0x06, 0x27, 0xd0,
    0x45, 0x5e, 0x00,
    0x47, 0x1e, 0x4b,
    0x47, 0x67, 0x0f,
    0x47, 0x50, 0x14,
    0x45, 0x40, 0x00,
    0x47, 0xb4, 0x14,
    0x47, 0x13, 0x30,
    0x47, 0x8b, 0x10,
    0x47, 0x8f, 0x10,
    0x47, 0x93, 0x10,
    0x47, 0x97, 0x0e,
    0x47, 0x9b, 0x0e,

    0x01, 0x57, 0x00,       // ANA_GAIN_GLOBAL_A
    0x01, 0x58, 0x01,       // DIG_GAIN_GLOBAL_A [11:8]
    0x01, 0x59, 0x00,       // DIG_GAIN_GLOBAL_A [7:0]
    0x01, 0x5a, 0x03,       // COARSE_INTEGRATION_TIME_A[15:8]
    0x01, 0x5b, 0xe8,       // COARSE_INTEGRATION_TIME_A[7:0]

    0x01, 0x72, 0x00,       // IMG_ORIENTATION_A

    0x01, 0x8c, 0x0a,       // CSI_DATA_FORMAT_A [15:8]
    0x01, 0x8d, 0x0a,       // CSI_DATA_FORMAT_A [7:0]
    0x03, 0x09, 0x0a,       // OPPXCK_DIV

    STOP_REG, STOP_REG, STOP_REG,
};

#endif //#if defined(ENABLE_640_480_30fps_2lane)
static STF_U8 g_u8Raw8_FrameFmt[] = {
    0x01, 0x8c, 0x08,       // CSI_DATA_FORMAT_A [15:8]
    0x01, 0x8d, 0x08,       // CSI_DATA_FORMAT_A [7:0]
    0x03, 0x09, 0x08,       // OPPXCK_DIV
};

static STF_U8 g_u8Raw10_FrameFmt[] = {
    0x01, 0x8c, 0x0a,       // CSI_DATA_FORMAT_A [15:8]
    0x01, 0x8d, 0x0a,       // CSI_DATA_FORMAT_A [7:0]
    0x03, 0x09, 0x0a,       // OPPXCK_DIV
};

struct ST_IMX219_MODE {
    STF_U16 u16Width;
    STF_U16 u16Height;
    STF_U8 u8Flipping;
    STF_U8 *pu8ModeRegisters;
    STF_U32 u32Registers;           // initially 0 then computed the 1st time
};

struct ST_IMX219_MODE g_stImx219Modes[] = {
#if defined(ENABLE_1920_1080_21fps_2lane)
    { 1920, 1080, SENSOR_FLIP_NONE, g_u8ModeRegs_1920_1080_21fps_2lane, 0 },    // 0
#endif //#if defined(ENABLE_1920_1080_21fps_2lane)
#if defined(ENABLE_3280_2464_21fps_2lane)
    { 3280, 2464, SENSOR_FLIP_NONE, g_u8ModeRegs_3280_2464_21fps_2lane, 0 },    // 1
#endif //#if defined(ENABLE_3280_2464_21fps_2lane)
#if defined(ENABLE_1640_1232_30fps_2lane)
    { 1640, 1232, SENSOR_FLIP_NONE, g_u8ModeRegs_1640_1232_30fps_2lane, 0 },    // 2
#endif //#if defined(ENABLE_1640_1232_30fps_2lane)
#if defined(ENABLE_1280_720_30fps_2lane)
    { 1280, 720, SENSOR_FLIP_NONE, g_u8ModeRegs_1280_720_30fps_2lane, 0 },    // 3
#endif //#if defined(ENABLE_1280_720_30fps_2lane)
#if defined(ENABLE_640_480_30fps_2lane)
    { 640, 480, SENSOR_FLIP_NONE, g_u8ModeRegs_640_480_30fps_2lane, 0 },        // 4
#endif //#if defined(ENABLE_640_480_30fps_2lane)
};
static STF_U8 g_u8SensorModeNum = 0;


// used before implementation therefore declared here
static STF_VOID Sensor_ConfigRegister(
    ST_IMX219_CAM *pstSensorCam
    );
static STF_RESULT Sensor_GetModeInfo(
    ST_IMX219_CAM *pstSensorCam,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    );
static STF_DOUBLE Sensor_ComputeGains(
    STF_DOUBLE dGain,
    STF_U32 *pu32AnalogGain,
    STF_U32 *pu32DigitalGain
    );
static STF_DOUBLE Sensor_ComputeRegToGains(
    STF_U32 u32AnalogGain,
    STF_U32 u32DigitalGain
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
    ST_IMX219_CAM *pstSensorCam,
    STF_U8 u8ModeIdx,
    STF_U32 *pu32Registers
    )
{

    STF_ASSERT(pu32Registers);  // null pointer forbidden

    if (u8ModeIdx < ARRAY_SIZE(g_stImx219Modes)) {
        STF_U8 *registerArray = g_stImx219Modes[u8ModeIdx].pu8ModeRegisters;

        if (0 == g_stImx219Modes[u8ModeIdx].u32Registers) {
            // should be done only once per mode
            int i = 0;
            while (STOP_REG != registerArray[3 * i]) {
                i++;
            }
            g_stImx219Modes[u8ModeIdx].u32Registers = i;
        }
        *pu32Registers = g_stImx219Modes[u8ModeIdx].u32Registers;
        
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
	if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
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
        LOG_ERROR("Failed to read I2C at 0x%X\n", u16Reg);
        return STF_ERROR_FATAL;
    }
  #else
    struct i2c_rdwr_ioctl_data stPackets;
    struct i2c_msg stMessages[2];
    STF_U8 u8Addr[2];

    STF_ASSERT(pu8Data);  // null pointer forbidden

    u8Addr[0] = (u16Reg >> 8) & 0xFF;
    u8Addr[1] = u16Reg & 0xFF;

    stMessages[0].addr = IMX219_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len = 2;
    stMessages[0].buf = u8Addr;

    stMessages[1].addr = IMX219_I2C_ADDR;
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
	if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
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

    stMessages[0].addr = IMX219_I2C_ADDR;
    stMessages[0].flags = 0;
    stMessages[0].len = 2;
    stMessages[0].buf = u8Addr;

    stMessages[1].addr = IMX219_I2C_ADDR;
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
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
        LOG_ERROR("Failed to write I2C slave address!\n");
        return STF_ERROR_BUSY;
    }

  #ifdef CONFIG_REG_DEBUG
    fpLog = fopen("/tmp/imx219_write.txt", "a");
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
        LOG_ERROR("Wrong len of data array, u16Len = %d\n", u16Len);
        return STF_ERROR_INVALID_PARAMETERS;
    }

  #ifdef CONFIG_REG_DEBUG
    fpLog = fopen("/tmp/imx219_write.txt", "a");
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

        stMessages[0].addr  = IMX219_I2C_ADDR;
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
    fpLog = fopen("/tmp/imx219_write.txt", "a");
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
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
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

    stMessages[0].addr  = IMX219_I2C_ADDR;
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
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
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

    stMessages[0].addr  = IMX219_I2C_ADDR;
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
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
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

    stMessages[0].addr  = IMX219_I2C_ADDR;
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
    if (ioctl(nI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
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

    stMessages[0].addr  = IMX219_I2C_ADDR;
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
    ST_IMX219_CAM *pstSensorCam
    )
{
    STF_U32 nIdx;
    // index for flipping register
    STF_U32 u32FlipIdx = pstSensorCam->u32Registers * 3;
    // index for exposure register
    STF_U32 u32ExposureHighIdx = pstSensorCam->u32Registers * 3;
    STF_U32 u32ExposureLowIdx = pstSensorCam->u32Registers * 3;
    // index for gain register
    STF_U32 u32AnalogGainIdx = pstSensorCam->u32Registers * 3;
    STF_U32 u32DigitalGainHighIdx = pstSensorCam->u32Registers * 3;
    STF_U32 u32DigitalGainLowIdx = pstSensorCam->u32Registers * 3;
#if !defined(COPY_REGS)
    STF_U8 u8Values[3];
#endif //COPY_REGS
    STF_U8 u8Regs[] = {
        0x01, 0x57,         // Offset of ANA_GAIN_GLOBAL_A

        0x00,               // Value of ANA_GAIN_GLOBAL_A
        0x01, 0x00,         // Value of DIG_GAIN_GLOBAL_A

        0x03, 0xE8,         // Value of COARSE_INTEGRATION_TIME_A
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

        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_IMG_ORIENTATION)) {
            u32FlipIdx = nIdx;
            if (pstSensorCam->u8Flipping & SENSOR_FLIP_HORIZONTAL) {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] |= 0x01;
#else
                u8Values[2] |= 0x01;
#endif //COPY_REGS
            } else {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] &= 0xFE;
#else
                u8Values[2] = 0xFE;
#endif //COPY_REGS
            }
            if (pstSensorCam->u8Flipping & SENSOR_FLIP_VERTICAL) {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] |= 0x02;
#else
                u8Values[2] |= 0x02;
#endif //COPY_REGS
            } else {
#if defined(COPY_REGS)
                pstSensorCam->pu8CurrentSensorModeReg[nIdx + 2] &= 0xFD;
#else
                u8Values[2] &= 0xFD;
#endif //COPY_REGS
            }
        }

        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_EXPOSURE)) {
            u32ExposureHighIdx = nIdx;
        }
        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_EXPOSURE + 1)) {
            u32ExposureLowIdx = nIdx;
        }

        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_ANA_GAIN)) {
            u32AnalogGainIdx = nIdx;
        }
        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_DIG_GAIN)) {
            u32DigitalGainHighIdx = nIdx;
        }
        if (IS_REG(pstSensorCam->pu8CurrentSensorModeReg, nIdx, REG_DIG_GAIN + 1)) {
            u32DigitalGainLowIdx = nIdx;
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
    if (u32FlipIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32FlipIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }

    if (u32AnalogGainIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32AnalogGainIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }
    if (u32DigitalGainHighIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32DigitalGainHighIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }
    if (u32DigitalGainLowIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32DigitalGainLowIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }
    if (u32ExposureHighIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32ExposureHighIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }
    if (u32ExposureLowIdx >= pstSensorCam->u32Registers * 3) {
        LOG_WARNING("Did not find u32ExposureLowIdx in registers for mode %d\n",
            pstSensorCam->u8ModeId);
    }

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

    u8Regs[5] = ((g_IMX219_u32ExposureLinesBackup >> 8) & 0xFF);
    u8Regs[6] = (g_IMX219_u32ExposureLinesBackup & 0xFF);
    u8Regs[2] = (g_IMX219_u32AnalogGainBackup & 0xFF);
    u8Regs[3] = ((g_IMX219_u32DigitalGainBackup >> 8) & 0xFF);
    u8Regs[4] = (g_IMX219_u32DigitalGainBackup & 0xFF);

    Ret = Sensor_I2cWriteRegs_3(
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
    ST_IMX219_CAM *pstSensorCam,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    )
{
    const STF_U8 u8BinMode[] = { 1, 2, 4, 4 };
    STF_U32 u32Registers = 0;
    STF_U8 *pu8ModeReg = NULL;

    STF_U16 csi_data_format = 0;
    STF_U16 csi_lane_mode = 0;
    STF_U16 binning_mode_h = 0;
    STF_U16 binning_mode_v = 0;
    STF_U16 line_length[2] = { 0, 0 };
    STF_U16 line_length_v = 0;
    STF_U16 frm_length[2] = { 0, 0 };
    STF_U16 frm_length_v = 0;
    STF_U32 frame_t = 0;
    STF_U16 x_output[2] = { 0, 0 };
    STF_U16 y_output[2] = { 0, 0 };
    STF_U16 exck_freq[2] = { 0, 0 };
    STF_U16 pll_pre_div_1 = 0;
    STF_U16 pll_pre_div_2 = 0;
    STF_U16 pll_m_1[2] = { 0, 0 };
    STF_U16 pll_m_2[2] = { 0, 0 };
    STF_U16 pll_div_1[2] = { 0, 0 };
    STF_U16 pll_div_2[2] = { 0, 0 };
    STF_U16 fine_integration_time[2] = { 0, 0 };
    STF_U16 fine_integration_time_v = 0;

    STF_DOUBLE ext_clk = 0;
    STF_DOUBLE pll_video_pre_div = 0;
    STF_DOUBLE pll_video_mult = 0;
    STF_DOUBLE pll_video_pxl_div = 0;
    STF_DOUBLE pll_video_sys_div = 0;
    STF_DOUBLE pll_video_pxl_clk = 0;
    STF_DOUBLE pll_output_pre_div = 0;
    STF_DOUBLE pll_output_mult = 0;
    STF_DOUBLE pll_output_pxl_div = 0;
    STF_DOUBLE pll_output_sys_div = 0;
    STF_DOUBLE pll_output_clk = 0;
    STF_DOUBLE pll_sclk = 0;
    STF_DOUBLE pll_mipi_clk = 0;

    STF_DOUBLE h_bin_mode = 1.0;
    STF_DOUBLE v_bin_mode = 1.0;

    STF_DOUBLE trow = 0; // in micro seconds

    STF_U16 nIdx;
    STF_RESULT Ret = STF_SUCCESS;

    pu8ModeReg = Sensor_GetRegisters(
        pstSensorCam,
        u8ModeIdx,
        &u32Registers
        );
    if (!pu8ModeReg) {
        LOG_ERROR("invalid mode %d\n", pu8ModeReg);
        return STF_ERROR_NOT_SUPPORTED;
    }
    u32Registers = g_stImx219Modes[u8ModeIdx].u32Registers;

#if defined(V4L2_DRIVER)
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_CSI_DATA_FORMAT,
        (STF_U8 *)&csi_data_format
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_CSI_LANE_MODE,
        (STF_U8 *)&csi_lane_mode
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_BINNING_MODE_H,
        (STF_U8 *)&binning_mode_h
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_BINNING_MODE_V,
        (STF_U8 *)&binning_mode_v
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_FRM_LENGTH_0,
        (STF_U8 *)&frm_length[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_FRM_LENGTH_1,
        (STF_U8 *)&frm_length[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_LINE_LENGTH_0,
        (STF_U8 *)&line_length[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_LINE_LENGTH_1,
        (STF_U8 *)&line_length[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_X_OUTPUT_SIZE_0,
        (STF_U8 *)&x_output[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_X_OUTPUT_SIZE_1,
        (STF_U8 *)&x_output[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_Y_OUTPUT_SIZE_0,
        (STF_U8 *)&y_output[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_Y_OUTPUT_SIZE_1,
        (STF_U8 *)&y_output[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_EXCK_FREQ_0,
        (STF_U8 *)&exck_freq[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_EXCK_FREQ_1,
        (STF_U8 *)&exck_freq[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_PREPLLCK_VT_DIV,
        (STF_U8 *)&pll_pre_div_1
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_PREPLLCK_OP_DIV,
        (STF_U8 *)&pll_pre_div_2
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_PLL_VT_MPY_0,
        (STF_U8 *)&pll_m_1[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_PLL_VT_MPY_1,
        (STF_U8 *)&pll_m_1[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_PLL_OP_MPY_0,
        (STF_U8 *)&pll_m_2[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_PLL_OP_MPY_1,
        (STF_U8 *)&pll_m_2[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_VTPXCK_DIV,
        (STF_U8 *)&pll_div_1[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_VTSYCK_DIV,
        (STF_U8 *)&pll_div_1[1]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_OPPXCK_DIV,
        (STF_U8 *)&pll_div_2[0]
        );
    Ret = Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_OPSYCK_DIV,
        (STF_U8 *)&pll_div_2[1]
        );
#else
    for (nIdx = 0 ; nIdx < u32Registers * 3 ; nIdx += 3) {
        if (IS_REG(pu8ModeReg, nIdx, REG_CSI_DATA_FORMAT)) {
            csi_data_format = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("csi_data_format = %d bits\n", csi_data_format);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_CSI_LANE_MODE)) {
            csi_lane_mode = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("csi_lane_mode = %d lane\n", (csi_lane_mode + 1));
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_BINNING_MODE_H)) {
            binning_mode_h = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("binning_mode_h = 0x%02x(%d)\n",
                binning_mode_h, u8BinMode[binning_mode_h & 0x03]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_BINNING_MODE_V)) {
            binning_mode_v = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("binning_mode_v = 0x%02x(%d)\n",
                binning_mode_v, u8BinMode[binning_mode_v & 0x03]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_FRM_LENGTH_0)) {
            frm_length[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("frm_length[0] = 0x%02X\n", frm_length[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_FRM_LENGTH_1)) {
            frm_length[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("frm_length[1] = 0x%02X\n", frm_length[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_LINE_LENGTH_0)) {
            line_length[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("line_length[0]=0x%02X\n", line_length[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_LINE_LENGTH_1)) {
            line_length[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("line_length[1]=0x%02X\n", line_length[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_X_OUTPUT_SIZE_0)) {
            x_output[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("x_output[0] = 0x%02X\n", x_output[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_X_OUTPUT_SIZE_1)) {
            x_output[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("x_output[1] = 0x%02X\n", x_output[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_Y_OUTPUT_SIZE_0)) {
            y_output[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("y_output[0] = 0x%02X\n", y_output[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_Y_OUTPUT_SIZE_1)) {
            y_output[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("y_output[1] = 0x%02X\n", y_output[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_EXCK_FREQ_0)) {
            exck_freq[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("exck_freq[0] = 0x%02X\n", exck_freq[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_EXCK_FREQ_1)) {
            exck_freq[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("exck_freq[1] = 0x%02X\n", exck_freq[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PREPLLCK_VT_DIV)) {
            pll_pre_div_1 = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_pre_div_1 = 0x%02X\n", pll_pre_div_1);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_PREPLLCK_OP_DIV)) {
            pll_pre_div_2 = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_pre_div_2 = 0x%02X\n", pll_pre_div_2);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_VT_MPY_0)) {
            pll_m_1[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_m_1[0] = 0x%02X\n", pll_m_1[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_VT_MPY_1)) {
            pll_m_1[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_m_1[1] = 0x%02X\n", pll_m_1[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_OP_MPY_0)) {
            pll_m_2[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_m_2[0] = 0x%02X\n", pll_m_2[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_PLL_OP_MPY_1)) {
            pll_m_2[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_m_2[1] = 0x%02X\n", pll_m_2[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_VTPXCK_DIV)) {
            pll_div_1[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_div_1[0] = 0x%02X\n", pll_div_1[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_VTSYCK_DIV)) {
            pll_div_1[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_div_1[1] = 0x%02X\n", pll_div_1[1]);
        }

        if (IS_REG(pu8ModeReg, nIdx, REG_OPPXCK_DIV)) {
            pll_div_2[0] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_div_2[0] = 0x%02X\n", pll_div_2[0]);
        }
        if (IS_REG(pu8ModeReg, nIdx, REG_OPSYCK_DIV)) {
            pll_div_2[1] = pu8ModeReg[nIdx + 2];
            LOG_DEBUG("pll_div_2[1] = 0x%02X\n", pll_div_2[1]);
        }
    }
#endif //#if defined(V4L2_DRIVER)
    Ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_FINE_INTEG_TIME_0,
        (STF_U8 *)&fine_integration_time[0]
        );
    Ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_FINE_INTEG_TIME_1,
        (STF_U8 *)&fine_integration_time[1]
        );

    ext_clk = (((exck_freq[0] << 8) | exck_freq[1]) / 256.0)
        * 1000000.0;                                    // 0x1800 = 24Mhz
    pll_video_pre_div = pll_pre_div_1;                  // 3
    pll_video_mult = ((pll_m_1[0] << 8) | pll_m_1[1]);  // 0x0039 = 57
    pll_video_pxl_div = (pll_div_1[0]);                 // 0x05 = 5
    pll_video_sys_div = (pll_div_1[1]);                 // 1
    pll_output_pre_div = pll_pre_div_2;                 // 3
    pll_output_mult = ((pll_m_2[0] << 8) | pll_m_2[1]); // 0x0072 = 114
    pll_output_pxl_div = (pll_div_2[0]);                // 0x0A = 10
    pll_output_sys_div = (pll_div_2[1]);                // 1

    if (0.0 == ext_clk
        || 0.0 == pll_video_pre_div
        || 0.0 == pll_video_mult
        || 0.0 == pll_video_pxl_div
        || 0.0 == pll_video_sys_div
        ) {
        pll_video_pxl_clk = (12.0 * 1000000 * 117 * 2) / (2 * 5 * 1);
        pll_sclk = (pll_video_pxl_clk / 2);
        LOG_WARNING("Did not find all PLL registers - assumes "\
            "pxl_clk of %.2lfMHz\n", (pll_video_pxl_clk / 1000000.0));
        LOG_WARNING("ext_clk=%.2lfMHz, pll_video_pre_div=%f, pll_video_mult=%f, "\
            "pll_video_pxl_div=%f, pll_video_sys_divs=%f\n",
            (ext_clk / 1000000.0), pll_video_pre_div, pll_video_mult, pll_video_pxl_div,
            pll_video_sys_div);
    } else {
        pll_video_pxl_clk = (ext_clk * pll_video_mult * 2)
            / (pll_video_pre_div * pll_video_pxl_div * pll_video_sys_div);
        pll_sclk = (pll_video_pxl_clk / 2);
        pll_output_clk = (ext_clk * pll_output_mult * 2)
            / (pll_output_pre_div * pll_output_pxl_div
                * (pll_output_sys_div + 1));
        pll_mipi_clk = (ext_clk * pll_output_mult) / (pll_output_pre_div);

        LOG_DEBUG("ext_clk=%.2lfMHz * pll_video_mult=%.2lf * 2 / "\
            "pll_video_pre_div=%.2lf / pll_video_pxl_div=%.2lf / "\
            "pll_video_sys_div=%.2lf\n",
            (ext_clk / 1000000.0), pll_video_mult, pll_video_pre_div,
            pll_video_pxl_div, pll_video_sys_div);
        LOG_DEBUG("pll_video_pxl_clk=%.2lfMHz / 2 = pll_sclk=%.2lfMHz\n",
            (pll_video_pxl_clk / 1000000.0), (pll_sclk / 1000000.0));
        LOG_DEBUG("pll_output_clk=%.2lfMHz, pll_mipi_clk=%.2lfMHz\n",
            (pll_output_clk / 1000000.0), (pll_mipi_clk / 1000000.0));
    }

    line_length_v = ((line_length[0] << 8) | line_length[1]);
    frm_length_v = ((frm_length[0] << 8) | frm_length[1]);
    fine_integration_time_v = ((fine_integration_time[0] << 8)
        | fine_integration_time[1]);
#if 0
    trow = ((line_length_v * 1000000.0) / (2.0 * pll_video_pxl_clk));
#else
    trow = ((line_length_v * 1000000.0) / (1.0 * pll_video_pxl_clk));
#endif //#if 0
    frame_t = trow * frm_length_v;

    LOG_DEBUG("line_length_v=%u frm_length_v=%u\n", line_length_v, frm_length_v);
    LOG_DEBUG("trow=%lf frame_t=%u\n", trow, frame_t);

    pstModes->u16Width = x_output[1] | (x_output[0] << 8);
    pstModes->u16Height = y_output[1] | (y_output[0] << 8);
    pstModes->u16VerticalTotal = frm_length_v;
    g_u32VerticalTotal = frm_length_v;
    //LOG_DEBUG("************ hts=%u vts=%u ************\n", hts_v, vts_v);

    pstModes->u8SupportFlipping = SENSOR_FLIP_NONE;
#if 0
    pstModes->u32ExposureMax = (STF_U32)floor(trow * frm_length_v);
#else
    pstModes->u32ExposureMax = SENSOR_MAX_EXPOSURE;
#endif //#if 0
    pstModes->u32ExposureMin = (STF_U32)floor(trow);

    pstModes->dExposureMin = trow;
    pstSensorCam->u32Exposure = trow * pstModes->u16VerticalTotal; // set initial exposure.

    h_bin_mode = u8BinMode[binning_mode_h & 0x03];
    v_bin_mode = u8BinMode[binning_mode_v & 0x03];

    //FrameRate = ((1 * h_bin_mode * v_bin_mode)
    //    / (((line_length_v) / (1 * pll_video_pxl_clk)) * frm_length_v) + 0.5);
    pstModes->dFrameRate = ((1 * h_bin_mode * v_bin_mode)
        / (((line_length_v) / (1 * pll_video_pxl_clk)) * frm_length_v) + 0.5);

    pstModes->u8MipiLanes = (csi_lane_mode + 1);
    //printf("pstModes->u8MipiLanes = %d\n", pstModes->u8MipiLanes);

    pstModes->u8BitDepth = csi_data_format;
    pstSensorCam->dCurFps = pstModes->dFrameRate;
    //printf("the dFrameRate = %f\n", pstModes->dFrameRate);
    pstSensorCam->u32FixedFps = (STF_U32)(pstSensorCam->dCurFps + 0.5);
    if (0.0 == ext_clk) {
        pstSensorCam->u32InitClk = (12 * 1000000);
    } else {
        pstSensorCam->u32InitClk = ext_clk;
    }
    pstSensorCam->dRefClock = ext_clk;
    V_LOG_DEBUG("ext_clk = %lf\n", ext_clk);
    V_LOG_DEBUG("pll_video_pxl_clk = %lf\n", pll_video_pxl_clk);
    V_LOG_DEBUG("pll_mipi_clk = %lf\n", pll_mipi_clk);
    V_LOG_DEBUG("line_length_v = %d\n", line_length_v);
    V_LOG_DEBUG("frm_length_v = %d\n", frm_length_v);
    V_LOG_DEBUG("fine_integration_time_v = %d\n", fine_integration_time_v);
    V_LOG_DEBUG("trow = %lf\n", trow);
    V_LOG_DEBUG("frame_t = %d\n", frame_t);
    V_LOG_DEBUG("dFrameRate = %lf\n", pstModes->dFrameRate);
    V_LOG_DEBUG("h_bin_mode = %lf(%d), v_bin_mode = %lf(%d)\n",
        h_bin_mode, binning_mode_h, v_bin_mode, binning_mode_v);

    return STF_SUCCESS;
}

static STF_S8 sGetModeIdx(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Width,
    STF_U16 u16Height
    )
{
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U8 u8Idx;
    STF_S8 s8ModeIdx= -1;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    V_LOG_INFO("IMX219 MIPI sensor u16Width = %d, u16Height = %d\n",
        u16Width, u16Height);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    for (u8Idx = 0; u8Idx < g_u8SensorModeNum; u8Idx++) {
        if ((u16Width == g_stImx219Modes[u8Idx].u16Width)
            && (u16Height == g_stImx219Modes[u8Idx].u16Height)) {
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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    V_LOG_INFO("IMX219 MIPI sensor u8ModeIdx = %d\n", u8ModeIdx);
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
    ST_IMX219_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    //LOG_INFO("**IMX219 MIPI sensor sGetState** \n");
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

    ST_IMX219_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    //LOG_INFO("**IMX219 MIPI sensor sGetInterfaceInfo** \n");
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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U8 *pu8ModeReg = NULL;
    STF_U32 u32Registers = 0;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    V_LOG_INFO("**IMX219 MIPI sensor mode=%d, flipping=0x%02X**\n",
        u8ModeIdx, u8Flipping);
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    ASSERT_MODE_RANGE(u8ModeIdx);

    pu8ModeReg = Sensor_GetRegisters(pstSensorCam, u8ModeIdx, &u32Registers);
    if (NULL == pu8ModeReg) {
        return STF_ERROR_INVALID_PARAMETERS;
    }

    if (u8Flipping !=
        (u8Flipping & g_stImx219Modes[u8ModeIdx].u8Flipping)) {
        LOG_ERROR("Sensor mode %d does not support selected flipping 0x%x "\
            "(supports 0x%x)\n",
            u8ModeIdx,
            u8Flipping,
            g_stImx219Modes[u8ModeIdx].u8Flipping
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
    pstSensorCam->u16CurrentFocus = g_IMX219_u16FocusDacMin; // minimum focus

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
        0x01, 0x00, 0x01 // streaming enable
    };
    static const STF_U8 au8DisableRegs[] = {
        0x01, 0x00, 0x00
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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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
#if !defined(SIFIVE_ISP)
    sSetFocus(pstHandle, pstSensorCam->u16CurrentFocus);
#endif //SIFIVE_ISP
    sSetFlipMirror(pstHandle, pstSensorCam->u8Flipping);

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
    //LOG_DEBUG("camera enabled\n");

    return STF_SUCCESS;
}

static STF_RESULT sDisable(
    SENSOR_HANDLE *pstHandle
    )
{
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    V_LOG_INFO("===========>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    if (pstSensorCam->bEnabled) {
        int delay = 0;

        //LOG_INFO("Disabling IMX219 MIPI camera\n");
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
    ST_IMX219_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    LOG_DEBUG("Destroying IMX219 MIPI camera\n");
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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U16 u16Id = 0;
    STF_RESULT Ret;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    //V_LOG_INFO(">>>>>>>>>>>>\n");
    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pstInfo);

    STF_ASSERT(strlen(IMX219MIPI_SENSOR_INFO_NAME) < SENSOR_INFO_NAME_MAX);
    STF_ASSERT(strlen(IMX219_SENSOR_VERSION) < SENSOR_INFO_VERSION_MAX);

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
    sprintf(pstInfo->pszSensorName, IMX219MIPI_SENSOR_INFO_NAME);
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
        REG_MODEL_ID_0,
        &u16Id
        );
#else
    Ret = 1;
#endif //NO_DEV_CHECK
    if (!Ret) {
        sprintf(pstInfo->pszSensorVersion, "0x%x", u16Id);
    } else {
        sprintf(pstInfo->pszSensorVersion, IMX219_SENSOR_VERSION);
    }
    pstInfo->dNumber = 1.2;
    pstInfo->u16FocalLength = 30;
    pstInfo->u32WellDepth = 6040;
    // bitdepth is a mode information
    pstInfo->dReadNoise = 5.0;
    pstInfo->u8Imager = pstSensorCam->u8Imager;
    pstInfo->bBackFacing = STF_TRUE;
    // other information should be filled by sGetInfo()
    pstInfo->u32ModeCount = ARRAY_SIZE(g_stImx219Modes);
    pstInfo->enExposureGainMethod = IMX219_EXPO_GAIN_METHOD;
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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U8 u8RegValue = 0;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U8 u8Regs[] = {
        0x00, 0x00, 0x00
    };

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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

static STF_DOUBLE Sensor_ComputeGains(
    STF_DOUBLE dGain,
    STF_U32 *pu32AnalogGain,
    STF_U32 *pu32DigitalGain
    )
{
    STF_DOUBLE dActualGain = 1.0;
    STF_DOUBLE dAnalogGain = 1.0;
    STF_DOUBLE dDigitalGain = 1.0;

    STF_ASSERT(pu32AnalogGain);
    STF_ASSERT(pu32DigitalGain);

    // Analog gain formula
    // rounddown(256.0 - (256.0 / dGain) + 0.5)
#if 0
    *pu32AnalogGain = (STF_U32)(256.0 - (256.0 / dGain) + 0.5);
#else
    *pu32AnalogGain = (STF_U32)(256.0 - (256.0 / dGain));
#endif //#if 0
    dAnalogGain = (256.0 / (256.0 - *pu32AnalogGain));

    // Digital gain formula
    // rounddown(dGain * 256.0 + 0.5)
#if defined(ENABLE_SENSOR_DIGITAL_GAIN)
    dDigitalGain = dGain / dAnalogGain;
    if (1.0 > dDigitalGain) {
        dDigitalGain = 1.0;
    }
#else
    dDigitalGain = 1.0;
#endif //#if defined(ENABLE_SENSOR_DIGITAL_GAIN)
#if 0
    *pu32DigitalGain = (STF_U32)(dDigitalGain * 256.0 + 0.5);
#else
    *pu32DigitalGain = (STF_U32)(dDigitalGain * 256.0);
#endif //#if 0
    dDigitalGain = (*pu32DigitalGain / 256.0);

    dActualGain = dAnalogGain * dDigitalGain;

    return dActualGain;
}

static STF_DOUBLE Sensor_ComputeRegToGains(
    STF_U32 u32AnalogGain,
    STF_U32 u32DigitalGain
    )
{
    STF_DOUBLE dActualGain = 1.0;
    STF_DOUBLE dAnalogGain = 1.0;
    STF_DOUBLE dDigitalGain = 1.0;

    dAnalogGain = (256.0 / (256.0 - u32AnalogGain));
    dDigitalGain = (u32DigitalGain / 256.0);
    dActualGain = dAnalogGain * dDigitalGain;

    return dActualGain;
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
    ST_IMX219_CAM *pstSensorCam = NULL;
#if defined(GET_CURRENT_FROM_SENSOR)
    STF_U32 u32AnalogGain;
    STF_U32 u32DigitalGain;
    STF_DOUBLE dActualGain = 1.0;
#endif //GET_CURRENT_FROM_SENSOR

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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
        REG_ANA_GAIN,
        (STF_U8 *)&u32AnalogGain
        );
    u32AnalogGain &= 0xFF;
    Sensor_I2cRead16(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_DIG_GAIN,
        (STF_U16 *)&u32DigitalGain
        );
    u32DigitalGain &= 0xFFFF;
    *pdCurrentGain = Sensor_ComputeRegToGains(u32AnalogGain, u32DigitalGain);
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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U8 u8GainRegs[] = {
        0x01, 0x57,         // Offset of ANA_GAIN_GLOBAL_A

        0x00,               // Value of ANA_GAIN_GLOBAL_A
        0x01, 0x00,         // Value of DIG_GAIN_GLOBAL_A
    };
    STF_DOUBLE dActualGain;
    STF_U32 u32AnalogGain;
    STF_U32 u32DigitalGain;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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

    dActualGain = Sensor_ComputeGains(dGain, &u32AnalogGain, &u32DigitalGain);
    u8GainRegs[2] = (u32AnalogGain & 0xFF);
    u8GainRegs[3] = ((u32DigitalGain >> 8) & 0xFF);
    u8GainRegs[4] = (u32DigitalGain & 0xFF);
    //dActualGain = Sensor_ComputeRegToGains(u32AnalogGain, u32DigitalGain);
    Sensor_I2cWriteRegs_3(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8GainRegs,
        sizeof(u8GainRegs)
        );
    V_LOG_DEBUG("dGain=%lf, reg again=0x%02X, dgain=0x%04X, actual gain=%lf\n",
        dGain, u32AnalogGain, u32DigitalGain, dActualGain);
    g_IMX219_u32AnalogGainBackup = u32AnalogGain;
    g_IMX219_u32DigitalGainBackup = u32DigitalGain;

    return STF_SUCCESS;
}

static STF_U32 Sensor_ComputeExposure(
    STF_U32 u32Exposure,
    STF_DOUBLE dExposureMin
    )
{
    STF_U32 u32ExposureLines;

    u32ExposureLines = (STF_U32)(u32Exposure / dExposureMin);
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
    ST_IMX219_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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
    ST_IMX219_CAM *pstSensorCam = NULL;
#if defined(GET_CURRENT_FROM_SENSOR)
    STF_U8 u8Exposure;
    STF_U32 u32ExposureLines;
#endif //GET_CURRENT_FROM_SENSOR

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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
    u32ExposureLines = u8Exposure;
    u32ExposureLines <<= 8;
    Sensor_I2cRead(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_EXPOSURE + 0x01,
        &u8Exposure
        );
    u32ExposureLines |= u8Exposure;
    *pu32Exposure = u32ExposureLines * pstSensorCam->stSensorMode.dExposureMin;
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
    STF_U8 u8FrameLengthRegs[] = {
        0x01, 0x60,         // Offset of FRM_LENGTH_A

        0x0A, 0xA8,         // Value of FRM_LENGTH_A
    };
    STF_U8 u8ExposureRegs[] = {
        0x01, 0x5A,         // Offset of COARSE_INTEGRATION_TIME_A

        0x03, 0xE8,         // Value of COARSE_INTEGRATION_TIME_A
    };
	ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U16 u16VerticalTotal;
    STF_U32 u32ExposureLines;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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

#if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)
    if (u32ExposureLines > (g_u32VerticalTotal - 4)) {
        u16VerticalTotal = u32ExposureLines + 4;
        //u16VerticalTotal = pstSensorCam->stSensorMode.u16VerticalTotal
        //    * pstSensorCam->u32FixedFps / fps;
        u8FrameLengthRegs[2] = (u16VerticalTotal >> 8);
        u8FrameLengthRegs[3] = (u16VerticalTotal & 0xff);
        Sensor_I2cWriteRegs_3(
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8FrameLengthRegs,
            sizeof(u8FrameLengthRegs)
            );
        pstSensorCam->dCurFps = pstSensorCam->stSensorMode.u16VerticalTotal
            * pstSensorCam->u32FixedFps / u16VerticalTotal;
    }
    u8ExposureRegs[2] = (u32ExposureLines >> 8);
    u8ExposureRegs[3] = (u32ExposureLines & 0xff);
#else
    if (u32ExposureLines > (g_u32VerticalTotal - 4)) {
        u32ExposureLines = g_u32VerticalTotal - 4;
    }
    u8ExposureRegs[2] = (u32ExposureLines >> 8);
    u8ExposureRegs[3] = (u32ExposureLines & 0xff);
#endif //#if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)

    Sensor_I2cWriteRegs_3(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8ExposureRegs,
        sizeof(u8ExposureRegs)
        );
    V_LOG_DEBUG("SetExposure. time=%d us, lines = %d\n",
        pstSensorCam->u32Exposure, u32ExposureLines);

    g_IMX219_u32ExposureLinesBackup = u32ExposureLines;

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
//    ST_IMX219_CAM *pstSensorCam = NULL;

//    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);
//
//    TUNE_SLEEP(1);
//    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);
    STF_ASSERT(pu16Min);
    STF_ASSERT(pu16Max);

    *pu16Min = g_IMX219_u16FocusDacMin;
    *pu16Max = g_IMX219_u16FocusDacMax;

    return STF_SUCCESS;
}

static STF_RESULT sGetCurrentFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16Current
    )
{
    ST_IMX219_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U8 ui8Regs[4];
    STF_U16 ui16DACVal;
    STF_U16 nIdx;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

    pstSensorCam->u16CurrentFocus = u16Focus;
    if (pstSensorCam->u16CurrentFocus >= g_IMX219_u16FocusDacMax) {
        // special case the infinity as it doesn't fit in with the rest
        ui16DACVal = 0;
    } else {
        ui16DACVal = Sensor_ComputeFocus(
            g_IMX219_u16FocusDacDist,
            g_IMX219_u16FocusDacVal,
            sizeof(g_IMX219_u16FocusDacDist) / sizeof(STF_U16),
            u16Focus
            );
    }

    ui8Regs[0] = 4;
    ui8Regs[1] = ui16DACVal >> 8;
    ui8Regs[2] = 5;
    ui8Regs[3] = ui16DACVal & 0xff;

    if (ioctl(pstSensorCam->fdI2c, /*I2C_SLAVE*/I2C_SLAVE_FORCE, IMX219_I2C_ADDR)) {
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
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U8 u8FlipMirror = 0;
    STF_RESULT ret;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

    TUNE_SLEEP(1);
    ASSERT_INITIALIZED(pstSensorCam->pstSensorPhy);

#if defined(ADD_USLEEP_FOR_I2C_READ)
    usleep(1);
#endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
    ret = Sensor_I2cRead(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_IMG_ORIENTATION,
        &u8FlipMirror
        );
    u8FlipMirror &= 0x03;                   //clear flip and mirror bit.
    switch (u8Flag) {
        default:
        case SENSOR_FLIP_NONE:
            u8FlipMirror &= 0x03;           //clear flip and mirror bit.
            pstSensorCam->u8Flipping = SENSOR_FLIP_NONE;
            break;

        case SENSOR_FLIP_HORIZONTAL:
            u8FlipMirror |= 0x01;           //set mirror bit and clean flip bit.
            pstSensorCam->u8Flipping = SENSOR_FLIP_HORIZONTAL;
            break;

        case SENSOR_FLIP_VERTICAL:
            u8FlipMirror |= 0x02;           //set flip bit and clean mirror bit.
            pstSensorCam->u8Flipping = SENSOR_FLIP_VERTICAL;
            break;

        case SENSOR_FLIP_BOTH:
            u8FlipMirror |= 0x03;           //set flip and mirror bit.
            pstSensorCam->u8Flipping = SENSOR_FLIP_BOTH;
            break;
    }
    Sensor_I2cWrite(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        REG_IMG_ORIENTATION,
        u8FlipMirror
        );

    return STF_SUCCESS;
}

static STF_RESULT sGetFixedFPS(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16FixedFps
    )
{
    ST_IMX219_CAM *pstSensorCam = NULL;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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
    STF_U8 u8FrameLengthRegs[] = {
        0x01, 0x60,         // Offset of FRM_LENGTH_A

        0x0A, 0xA8,         // Value of FRM_LENGTH_A
    };
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_DOUBLE dRealFps = 0.0;
    STF_U32 u32Framlines = 0;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);
	
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

    u8FrameLengthRegs[2] = ((u32Framlines >> 8) & 0xFF);
    u8FrameLengthRegs[3] = (u32Framlines & 0xFF);

    Sensor_I2cWriteRegs_3(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->pstSensorPhy->psConnection,
        pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstSensorCam->fdI2c,
        u8FrameLengthRegs,
        sizeof(u8FrameLengthRegs)
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
//        REG_FRM_LENGTH_0,
//        &u8FrameLengthRegs[2]
//        );
//#if defined(ADD_USLEEP_FOR_I2C_READ)
//    usleep(1);
//#endif //#if defined(ADD_USLEEP_FOR_I2C_READ)
//    Sensor_I2cRead(
//#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
//        pstSensorCam->pstSensorPhy->psConnection,
//        pstSensorCam->pstSensorPhy->u8IspIdx,
//#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
//        pstSensorCam->fdI2c,
//        REG_FRM_LENGTH_1,
//        &u8FrameLengthRegs[3]
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
    STF_U8 u8FrameLengthRegs[] = {
        0x01, 0x60,         // Offset of FRM_LENGTH_A

        0x0A, 0xA8,         // Value of FRM_LENGTH_A
    };
    STF_U8 u8Regs[] = {
        0x01, 0x57,         // Offset of ANA_GAIN_GLOBAL_A

        0x00,               // Value of ANA_GAIN_GLOBAL_A
        0x01, 0x00,         // Value of DIG_GAIN_GLOBAL_A

        0x03, 0xE8,         // Value of COARSE_INTEGRATION_TIME_A
    };
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_U16 u16VerticalTotal;
    STF_U32 u32ExposureLines;
    STF_DOUBLE dActualGain;
    STF_U32 u32AnalogGain;
    STF_U32 u32DigitalGain;

    pstSensorCam = container_of(pstHandle, ST_IMX219_CAM, stFuncs);

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

  #if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)
    if (u32ExposureLines > (g_u32VerticalTotal - 4)) {
        u16VerticalTotal = u32ExposureLines + 4;
        //u16VerticalTotal = pstSensorCam->stSensorMode.u16VerticalTotal
        //    * pstSensorCam->u32FixedFps / fps;
        u8FrameLengthRegs[2] = (u16VerticalTotal >> 8);
        u8FrameLengthRegs[3] = (u16VerticalTotal & 0xff);
        Sensor_I2cWriteRegs_3(
    #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
    #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8FrameLengthRegs,
            sizeof(u8FrameLengthRegs)
            );
        pstSensorCam->dCurFps = pstSensorCam->stSensorMode.u16VerticalTotal
            * pstSensorCam->u32FixedFps / u16VerticalTotal;
    }
    u8Regs[5] = (u32ExposureLines >> 8);
    u8Regs[6] = (u32ExposureLines & 0xff);
  #else
    if (u32ExposureLines > (g_u32VerticalTotal - 4)) {
        u32ExposureLines = g_u32VerticalTotal - 4;
    }
    u8Regs[5] = (u32ExposureLines >> 8);
    u8Regs[6] = (u32ExposureLines & 0xff);
  #endif //#if defined(ENABLE_AUTO_EXTEND_VERTICAL_TOTAL)
    //V_LOG_DEBUG("Exposure - time=%d us, reg value=0x%04X, lines = %d\n",
    //    pstSensorCam->u32Exposure,
    //    u32ExposureLines,
    //    u32ExposureLines
    //    );
    //LOG_INFO("g_u32VerticalTotal = %d, u32Exposure = %d, dExposureMin = %lf, u32ExposureLines = %d\n",
    //    g_u32VerticalTotal, u32Exposure, pstSensorCam->stSensorMode.dExposureMin, u32ExposureLines);
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
    dActualGain = Sensor_ComputeGains(dGain, &u32AnalogGain, &u32DigitalGain);
    u8Regs[2] = (u32AnalogGain & 0xFF);
    u8Regs[3] = ((u32DigitalGain >> 8) & 0xFF);
    u8Regs[4] = (u32DigitalGain & 0xFF);
    //dActualGain = Sensor_ComputeRegToGains(u32AnalogGain, u32DigitalGain);
    //V_LOG_DEBUG("Gain - dGain=%lf, reg again=0x%02X, dgain=0x%04X, "\
    //    "actual gain=%lf\n",
    //    dGain,
    //    u32AnalogGain,
    //    u32DigitalGain,
    //    dActualGain
    //    );
    //LOG_INFO("dGain = %lf, u32Gain = 0x%08X, dActualGain = %lf\n",
    //    dGain, u32Gain, dActualGain);

    //03. Program register to sensor part
    //
    Sensor_I2cWriteRegs_3(
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
    ST_IMX219_CAM *pstSensorCam
    )
{
    STF_U8 u8FrameLengthRegs[] = {
        0x01, 0x60,         // Offset of FRM_LENGTH_A

        0x0A, 0xA8,         // Value of FRM_LENGTH_A
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
        u8FrameLengthRegs[2] = (u16VerticalTotal >> 8);
        u8FrameLengthRegs[3] = (u16VerticalTotal & 0xff);
        Sensor_I2cWriteRegs_3(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->pstSensorPhy->psConnection,
            pstSensorCam->pstSensorPhy->u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pstSensorCam->fdI2c,
            u8FrameLengthRegs,
            sizeof(u8FrameLengthRegs)
            );
        //LOG_DEBUG("^^^^^^^^^^^^^^^^ 0x0160 = %x, 0x0161 = %x^^^^^^^^^^^^^^^^\n",
        //    u8Regs[2], u8Regs[3]);
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

STF_RESULT IMX219MIPI_Create(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    SENSOR_HANDLE **ppstHandle,
    STF_U8 u8Index
    )
{
    ST_IMX219_CAM *pstSensorCam = NULL;
    STF_CHAR szI2cDevPath[NAME_MAX];
    STF_CHAR szExtraCfg[64];
    STF_CHAR szAdaptor[64];
    STF_U32 u32I2cAddr = IMX219_I2C_ADDR;
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
    u32I2cAddr = IMX219_I2C_ADDR;
    nChn = IMX219_I2C_CHN;
    u8Imager = 1;
    V_LOG_INFO("i2c-addr=0x%x, chn = %d\n", u32I2cAddr, nChn);
    sprintf(szAdaptor, "%s-%d", "i2c", nChn);
    sprintf(szExtraCfg, "%s%s%d-config.txt", EXTRA_CFG_PATH, "sensor", u8Index);
#endif //UNUSED_CODE_AND_VARIABLE

    V_LOG_INFO("**IMX219MIPI SENSOR**\n");
    TUNE_SLEEP(1);

    /* Init global variable */
    g_u8SensorModeNum = ARRAY_SIZE(g_stImx219Modes);
    V_LOG_DEBUG("g_u8SensorModeNum=%d\n", g_u8SensorModeNum);

    pstSensorCam = (ST_IMX219_CAM *)STF_CALLOC(1, sizeof(ST_IMX219_CAM));
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
        REG_MODEL_ID_0,
        &u16ChipVersion
        );
    if (Ret || (IMX219_CHIP_VERSION != u16ChipVersion)) {
        LOG_ERROR("Failed to ensure that the i2c device has a compatible "\
            "IMX219 sensor! Ret=%d chip_version=0x%x (expect chip 0x%x)\n",
            Ret, u16ChipVersion, IMX219_CHIP_VERSION);
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
	    REG_MODEL_ID_0,
	    &u16ChipVersion
	    );
	V_LOG_INFO("Check imx219 chip version: 0x%x\n", u16ChipVersion);

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

