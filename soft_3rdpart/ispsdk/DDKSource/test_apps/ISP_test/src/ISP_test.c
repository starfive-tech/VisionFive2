/**
  ******************************************************************************
  * @file  ISP_test.c
  * @author  SiFive Isp Team
  * @version  V1.0
  * @date  03/24/2020
  * @brief
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, SIFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai SiFive</center></h2>
  */


#include <stdio.h>
#include <errno.h>
//#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stf_include.h"

#include "ci/ci_api.h"
#include "ci/ci_version.h"

//=== Godspeed === remove below include after include Camera.h.
#include "ISPC/stflib_isp_device.h"
#include "ISPC/stflib_isp_base.h"
//#include "ISPC/Pipeline.h"

#define LOG_TAG "ISP_Test"
#include "stf_common/userlog.h"

#include "ISP_test.h"
#include "stf_isp_api.h"

#include "test/ISP_reg_test.h"
#include "test/ISP_mem_test.h"
#include "test/ISP_isp_base_test.h"
#include "test/ISP_mod_sud_test.h"
#include "test/ISP_mod_csi_in_test.h"
#include "test/ISP_mod_cbar_test.h"
#include "test/ISP_mod_crop_test.h"
#include "test/ISP_mod_dc_test.h"
#include "test/ISP_mod_dec_test.h"
#include "test/ISP_mod_oba_test.h"
#include "test/ISP_mod_obc_test.h"
#include "test/ISP_mod_lcbq_test.h"
#include "test/ISP_mod_sc_test.h"
#if defined(SUPPORT_DUMP_MODULE)
#include "test/ISP_mod_dump_test.h"
#endif //#if defined(SUPPORT_DUMP_MODULE)
#include "test/ISP_mod_isp_in_test.h"
#include "test/ISP_mod_dbc_test.h"
#include "test/ISP_mod_ctc_test.h"
#include "test/ISP_mod_oecf_test.h"
#include "test/ISP_mod_oecfhm_test.h"
#include "test/ISP_mod_lccf_test.h"
#include "test/ISP_mod_awb_test.h"
#include "test/ISP_mod_cfa_ahd_test.h"
#include "test/ISP_mod_car_test.h"
#include "test/ISP_mod_ccm_test.h"
#include "test/ISP_mod_gmargb_test.h"
#include "test/ISP_mod_r2y_test.h"
#include "test/ISP_mod_yhist_test.h"
#include "test/ISP_mod_ycrv_test.h"
#include "test/ISP_mod_shrp_test.h"
#include "test/ISP_mod_dnyuv_test.h"
#include "test/ISP_mod_sat_test.h"
#include "test/ISP_mod_out_test.h"
#include "test/ISP_mod_til_test.h"
#include "test/ISP_mod_buf_test.h"
#if defined(V4L2_DRIVER)

#include <linux/media.h>
#include <linux/types.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#endif //#if defined(V4L2_DRIVER)


#if 0
#define debug printf
#else
#define debug
#endif


#define TEST_DRV_CONNECT_KEY        ('0')
#define TEST_DRV_DISCONNECT_KEY     ('1')
#ifdef ISP_MAIN_FUNCTION_ENABLE
#define TEST_ISP_MAIN_KEY           ('2')
#endif //ISP_MAIN_FUNCTION_ENABLE
#define TEST_REG_KEY                ('3')
#define TEST_MEM_KEY                ('4')
#define TEST_ISP_BASE_KEY           ('5')
//---------------------------------------
#define TEST_MOD_SUD_KEY            ('A')                       // 'A'
#define TEST_MOD_CSI_IN_KEY         (TEST_MOD_SUD_KEY + 1)      // 'B'
#define TEST_MOD_CBAR_KEY           (TEST_MOD_CSI_IN_KEY + 1)   // 'C'
#define TEST_MOD_CROP_KEY           (TEST_MOD_CBAR_KEY + 1)     // 'D'
#define TEST_MOD_DC_KEY             (TEST_MOD_CROP_KEY + 1)     // 'E'
#define TEST_MOD_DEC_KEY            (TEST_MOD_DC_KEY + 1)       // 'F'
#define TEST_MOD_OBA_KEY            (TEST_MOD_DEC_KEY + 1)      // 'G'
#define TEST_MOD_OBC_KEY            (TEST_MOD_OBA_KEY + 1)      // 'H'
#define TEST_MOD_LCBQ_KEY           (TEST_MOD_OBC_KEY + 1)      // 'I'
#define TEST_MOD_SC_KEY             (TEST_MOD_LCBQ_KEY + 1)     // 'J'
#define TEST_MOD_DUMP_KEY           (TEST_MOD_SC_KEY + 1)       // 'K'
#define TEST_MOD_ISP_IN_KEY         (TEST_MOD_DUMP_KEY + 1)     // 'L'
#define TEST_MOD_DBC_KEY            (TEST_MOD_ISP_IN_KEY + 1)   // 'M'
#define TEST_MOD_CTC_KEY            (TEST_MOD_DBC_KEY + 1)      // 'N'
#define TEST_MOD_OECF_KEY           (TEST_MOD_CTC_KEY + 1)      // 'O'
#define TEST_MOD_OECFHM_KEY         (TEST_MOD_OECF_KEY + 1)     // 'P'
#define TEST_MOD_LCCF_KEY           (TEST_MOD_OECFHM_KEY + 1)   // 'Q'
#define TEST_MOD_AWB_KEY            (TEST_MOD_LCCF_KEY + 1)     // 'R'
#define TEST_MOD_CFA_KEY            (TEST_MOD_AWB_KEY + 1)      // 'S'
#define TEST_MOD_CAR_KEY            (TEST_MOD_CFA_KEY + 1)      // 'T'
#define TEST_MOD_CCM_KEY            (TEST_MOD_CAR_KEY + 1)      // 'U'
#define TEST_MOD_GMARGB_KEY         (TEST_MOD_CCM_KEY + 1)      // 'V'
#define TEST_MOD_R2Y_KEY            (TEST_MOD_GMARGB_KEY + 1)   // 'W'
#define TEST_MOD_YHIST_KEY          (TEST_MOD_R2Y_KEY + 1)      // 'X'
#define TEST_MOD_YCRV_KEY           (TEST_MOD_YHIST_KEY + 1)    // 'Y'
#define TEST_MOD_SHRP_KEY           (TEST_MOD_YCRV_KEY + 1)     // 'Z'
#define TEST_MOD_DNYUV_KEY          (TEST_MOD_SHRP_KEY + 1)     // '['
#define TEST_MOD_SAT_KEY            (TEST_MOD_DNYUV_KEY + 1)    // '\'
#define TEST_MOD_OUT_KEY            (TEST_MOD_SAT_KEY + 1)      // ']'
#define TEST_MOD_TIL_KEY            (TEST_MOD_OUT_KEY + 1)      // '^'
#define TEST_MOD_BUF_KEY            (TEST_MOD_TIL_KEY + 1)      // '_'
#define TEST_MOD_NEXT_KEY           (TEST_MOD_BUF_KEY + 1)      // '`'
//---------------------------------------
#define SET_ISP_AWB_NORMAL_KEY      ('j')
#define SET_ISP_AWB_R_GAIN_X2_KEY   ('k')
#define SET_ISP_AWB_G_GAIN_X2_KEY   ('l')
#define SET_ISP_AWB_B_GAIN_X2_KEY   ('m')
//---------------------------------------
#define SET_ISP_R2Y_NORMAL_KEY      ('n')
#define SET_ISP_R2Y_REMOVE_R_KEY    ('o')
#define SET_ISP_R2Y_REMOVE_G_KEY    ('p')
#define SET_ISP_R2Y_REMOVE_B_KEY    ('q')
//---------------------------------------
#define SET_CSI_SHADOW_ENABLE_KEY   ('r')
#define SET_CSI_SHADOW_DISABLE_KEY  ('s')
#define SET_ISP_SHADOW_ENABLE_KEY   ('t')
#define SET_ISP_SHADOW_DISABLE_KEY  ('u')
//---------------------------------------
#define PRINT_SC_DUMP_BUF_KEY       ('v')
#define PRINT_Y_HIST_BUF_KEY        ('w')
//---------------------------------------
#define CHECK_ISP_PARAM_KEY         ('x')
#define CHECK_SENSOR_PARAM_KEY      ('y')
#define CHECK_REG_KEY               ('z')
//---------------------------------------
#define TEST_API_REG_READ_WRITE_KEY ('!')
#define TEST_API_MEM_GET_BUF_KEY    ('@')
#define TEST_API_START_ISP_CAP_KEY  ('#')
#define TEST_API_STOP_ISP_CAP_KEY   ('$')
//---------------------------------------
#define TEST_HELP_KEY               ('?')
#define EXIT_KEY                    ('.')


const STF_U16 g_au16SensorParam[] = {
    0x0103,0x01,
    0x0100,0x00,
    0x3039,0x80,
    0x3621,0x28,

    0x3309,0x60,
    0x331f,0x4d,
    0x3321,0x4f,
    0x33b5,0x10,

    0x3303,0x20,
    0x331e,0xd,
    0x3320,0xf,

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
    //0x0100,0x01,
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
    */
    };


/*
 * other functions
 */

void printUsage()
{

//    DYNCMD_PrintUsage();

    //printf("\n\nRuntime key controls:\n");
    printf("\nRuntime key controls:\n");
    printf("\t=== Test Program ===\n");
    printf("%c Test ISP driver connection\n", TEST_DRV_CONNECT_KEY);
    printf("%c Test ISP driver disconnection\n", TEST_DRV_DISCONNECT_KEY);
#ifdef ISP_MAIN_FUNCTION_ENABLE
    printf("%c Enter ISP main\n", TEST_ISP_MAIN_KEY);
#endif //ISP_MAIN_FUNCTION_ENABLE
    printf("%c Test Register Access driver\n", TEST_REG_KEY);
    printf("%c Test Memory Alloc/Free driver\n", TEST_MEM_KEY);
    printf("%c Test ISP Base driver\n", TEST_ISP_BASE_KEY);
    printf("------------------------------\n");
    printf("%c Test SUD module driver\n", TEST_MOD_SUD_KEY);
    printf("%c Test CSI_IN module driver\n", TEST_MOD_CSI_IN_KEY);
    printf("%c Test CBAR module driver\n", TEST_MOD_CBAR_KEY);
    printf("%c Test CROP module driver\n", TEST_MOD_CROP_KEY);
    printf("%c Test DC module driver\n", TEST_MOD_DC_KEY);
    printf("%c Test DEC module driver\n", TEST_MOD_DEC_KEY);
    printf("%c Test OBA module driver\n", TEST_MOD_OBA_KEY);
    printf("%c Test OBC module driver\n", TEST_MOD_OBC_KEY);
    printf("%c Test LCBQ module driver\n", TEST_MOD_LCBQ_KEY);
    printf("%c Test SC module driver\n", TEST_MOD_SC_KEY);
#if defined(SUPPORT_DUMP_MODULE)
    printf("%c Test DUMP module driver\n", TEST_MOD_DUMP_KEY);
#endif //#if defined(SUPPORT_DUMP_MODULE)
    printf("%c Test ISP_IN module driver\n", TEST_MOD_ISP_IN_KEY);
    printf("%c Test DBC module driver\n", TEST_MOD_DBC_KEY);
    printf("%c Test CTC module driver\n", TEST_MOD_CTC_KEY);
    printf("%c Test OECF module driver\n", TEST_MOD_OECF_KEY);
    printf("%c Test OECFHM module driver\n", TEST_MOD_OECFHM_KEY);
    printf("%c Test LCCF module driver\n", TEST_MOD_LCCF_KEY);
    printf("%c Test AWB module driver\n", TEST_MOD_AWB_KEY);
    printf("%c Test CFA module driver\n", TEST_MOD_CFA_KEY);
    printf("%c Test CAR module driver\n", TEST_MOD_CAR_KEY);
    printf("%c Test CCM module driver\n", TEST_MOD_CCM_KEY);
    printf("%c Test GMARGB module driver\n", TEST_MOD_GMARGB_KEY);
    printf("%c Test R2Y module driver\n", TEST_MOD_R2Y_KEY);
    printf("%c Test YHIST module driver\n", TEST_MOD_YHIST_KEY);
    printf("%c Test YCRV module driver\n", TEST_MOD_YCRV_KEY);
    printf("%c Test SHRP module driver\n", TEST_MOD_SHRP_KEY);
    printf("%c Test DNYUV module driver\n", TEST_MOD_DNYUV_KEY);
    printf("%c Test SAT module driver\n", TEST_MOD_SAT_KEY);
    printf("%c Test OUT module driver\n", TEST_MOD_OUT_KEY);
    printf("%c Test TIL module driver\n", TEST_MOD_TIL_KEY);
    printf("%c Test BUF module driver\n", TEST_MOD_BUF_KEY);
    printf("------------------------------\n");
    printf("%c Set ISP AWB gain normal\n", SET_ISP_AWB_NORMAL_KEY);
    printf("%c Set ISP AWB gain Red gain x2\n", SET_ISP_AWB_R_GAIN_X2_KEY);
    printf("%c Set ISP AWB gain Green gain x2\n", SET_ISP_AWB_G_GAIN_X2_KEY);
    printf("%c Set ISP AWB gain Blue gain x2\n", SET_ISP_AWB_B_GAIN_X2_KEY);
    printf("------------------------------\n");
    printf("%c Set ISP R2Y matrix normal\n", SET_ISP_R2Y_NORMAL_KEY);
    printf("%c Set ISP R2Y matrix remove Red\n", SET_ISP_R2Y_REMOVE_R_KEY);
    printf("%c Set ISP R2Y matrix remove Green\n", SET_ISP_R2Y_REMOVE_G_KEY);
    printf("%c Set ISP R2Y matrix remove Blue\n", SET_ISP_R2Y_REMOVE_B_KEY);
    printf("------------------------------\n");
    printf("%c Set CSI shadow update enable\n", SET_CSI_SHADOW_ENABLE_KEY);
    printf("%c Set CSI shadow update disable\n", SET_CSI_SHADOW_DISABLE_KEY);
    printf("%c Set ISP shadow update enable\n", SET_ISP_SHADOW_ENABLE_KEY);
    printf("%c Set ISP shadow update disable\n", SET_ISP_SHADOW_DISABLE_KEY);
    printf("------------------------------\n");
    printf("%c Print SC dumping buffer\n", PRINT_SC_DUMP_BUF_KEY);
    printf("%c Print Y Histogram dumping buffer\n", PRINT_Y_HIST_BUF_KEY);
    printf("------------------------------\n");
    printf("%c Check ISP Param value\n", CHECK_ISP_PARAM_KEY);
    printf("%c Check Sensor Param value\n", CHECK_SENSOR_PARAM_KEY);
    printf("%c Check register value\n", CHECK_REG_KEY);
    printf("------------------------------\n");
    printf("%c Test Reg read and write API\n", TEST_API_REG_READ_WRITE_KEY);
    printf("%c Test Mem buf get API\n", TEST_API_MEM_GET_BUF_KEY);
    printf("%c Test start ISP capture API\n", TEST_API_START_ISP_CAP_KEY);
    printf("%c Test stop ISP capture API\n", TEST_API_STOP_ISP_CAP_KEY);
    printf("------------------------------\n");
    printf("%c Print the key controls menu\n", TEST_HELP_KEY);
    printf("%c Exit\n", EXIT_KEY);

    //printf("\n\n");
    printf("\n");
}

void enableRawMode(struct termios *p_orig_term_attr)
{
    struct termios new_term_attr;

    debug("Enable Console RAW mode\n");

    /* set the terminal to raw mode */
    //tcgetattr(STDIN_FILENO, p_orig_term_attr);
    tcgetattr(fileno(stdin), p_orig_term_attr);
    STF_MEMCPY(&new_term_attr, p_orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;

    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
}

char getch(void)
{
    static char line[2];

    if (read(0, line, 1)) {
        return line[0];
    }

    return -1;
}

void disableRawMode(struct termios *p_orig_term_attr)
{

    debug("Disable Console RAW mode\n");
    tcsetattr(fileno(stdin), TCSANOW, p_orig_term_attr);
}


int run(int argc, char *argv[])
{
#if defined(V4L2_DRIVER)
    ST_CI_DEVICE stDevUo0;
    ST_SENSOR stSensor;
    STF_CHAR szSensorName[256];
    STF_U16 u16SensorId = 0;
    STF_U8 u8SensorIdx = 0;
    STF_U8 u8SensorMode = 0;
    STF_U8 u8SensorFlipping = SENSOR_FLIP_NONE;
    ST_SIZE stCaptureSize;
#endif //#if defined(V4L2_DRIVER)
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT Ret = STF_SUCCESS;
    int iSysRet;

    Ret = STFLIB_ISP_BASE_Connection(TEST_ISP_DEVICE);
    if (Ret) {
        LOG_ERROR("Failed to connected to the ISP driver\n");
        return -1;
    } else {
        LOG_INFO("ISP driver is connected, connections = %d\n",
            STFLIB_ISP_BASE_GetConnections(TEST_ISP_DEVICE)
            );
    }

#if defined(V4L2_DRIVER)
    // Open video 1 to configure ISP 0 clock and power on.
    Ret = STFLIB_ISP_DEVICE_StructInitialize(&stDevUo0, EN_DEV_ID_ISP_0_UO,
        STF_FALSE, EN_MEM_TYPE_NONE, 0, 0);
    // check if opening was successfull
    if (STF_SUCCESS != Ret) {
        LOG_ERROR("Cannot open '%s': %d, %s\n",
            stDevUo0.szDeviceName, errno, strerror(errno));
        return -1;
    }

#endif //#if defined(V4L2_DRIVER)
    iSysRet = STFAPI_ISP_StartIspMainThread();

    srand(time(0));

    sleep(1);
    printUsage();

    bool loopActive = true;

#if 0//defined(V4L2_DRIVER)
    {
        CI_CONNECTION *pstCiConnection = NULL;
        struct v4l2_capability cap;

        LOG_INFO("-------------------------------------\n");
        pstCiConnection = stDevUo0.GetConnection(&stDevUo0);
        if (STF_SUCCESS != STFDRV_V4L_BASE_IOCTL(
            pstCiConnection,
            VIDIOC_QUERYCAP,
            &cap
            )) {
            if (EINVAL == errno) {
                LOG_ERROR("%s is not a V4L2 device!\n", stDevUo0.szDeviceName);
                return -1;
            } else {
                LOG_ERROR("Call VIDIOC_QUERYCAP failed, Device = %s!\n",
                    stDevUo0.szDeviceName);
                return -1;
            }
        } else {
            LOG_INFO("%s is a V4L2 device!\n", stDevUo0.szDeviceName);
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            LOG_ERROR("%s is not a video capture device!\n",
                stDevUo0.szDeviceName);
            return -1;
        } else {
            LOG_INFO("%s is a video capture device!\n", stDevUo0.szDeviceName);
        }
        LOG_INFO("-------------------------------------\n");
        pstCiConnection = (CI_CONNECTION *)STFLIB_ISP_BASE_GetConnection(
            TEST_ISP_DEVICE);
        if (STF_SUCCESS != STFDRV_V4L_BASE_IOCTL(
            pstCiConnection,
            VIDIOC_QUERYCAP,
            &cap
            )) {
            if (EINVAL == errno) {
                LOG_ERROR("%s is not a V4L2 device!\n",
                    (TEST_ISP_DEVICE == 0) ? ("ISP 0") : ("ISP 1"));
                return -1;
            } else {
                LOG_ERROR("Call VIDIOC_QUERYCAP failed, Device = %s!\n",
                    (TEST_ISP_DEVICE == 0) ? ("ISP 0") : ("ISP 1"));
                return -1;
            }
        } else {
            LOG_INFO("%s is a V4L2 device!\n",
                (TEST_ISP_DEVICE == 0) ? ("ISP 0") : ("ISP 1"));
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            LOG_ERROR("%s is not a video capture device!\n",
                (TEST_ISP_DEVICE == 0) ? ("ISP 0") : ("ISP 1"));
            return -1;
        } else {
            LOG_INFO("%s is a video capture device!\n",
                (TEST_ISP_DEVICE == 0) ? ("ISP 0") : ("ISP 1"));
        }
        LOG_INFO("-------------------------------------\n");
    }
#endif //#if defined(V4L2_DRIVER)
    while (loopActive) {    //Capture loop
        //Deal with the keyboard input
        //char read = fgetc(stdin);
        char read = getch();
        switch (read) {
            //-----------------------------------------------------------------
            case TEST_DRV_CONNECT_KEY:
                Ret = STFLIB_ISP_BASE_Connection(TEST_ISP_DEVICE);
                pstCiConnection = (CI_CONNECTION *)
                    STFLIB_ISP_BASE_GetConnection(TEST_ISP_DEVICE);
                if (!pstCiConnection) {
                    printf("Does no connected to the ISP driver!\n");
                } else {
                    printf("ISP driver is connected, connections = %d\n",
                        STFLIB_ISP_BASE_GetConnections(TEST_ISP_DEVICE)
                        );
                }
                break;

            case TEST_DRV_DISCONNECT_KEY:
                Ret = STFLIB_ISP_BASE_Disconnection(TEST_ISP_DEVICE);
                printf("ISP driver disconnected, connections = %d\n",
                    STFLIB_ISP_BASE_GetConnections(TEST_ISP_DEVICE)
                    );
                break;

#ifdef ISP_MAIN_FUNCTION_ENABLE
            case TEST_ISP_MAIN_KEY:
                printf("Enter ISP main key pressed\n");
                Ret = Isp_Main();
                break;

#endif //ISP_MAIN_FUNCTION_ENABLE
            case TEST_REG_KEY:
                Ret = ISP_Reg_Test();
                break;

            case TEST_MEM_KEY:
                Ret = ISP_Mem_Test();
                break;

            case TEST_ISP_BASE_KEY:
                Ret = ISP_Isp_Base_Test();
                break;

            //-----------------------------------------------------------------
            case TEST_MOD_SUD_KEY:
                Ret = ISP_Mod_Sud_Test();
                break;

            case TEST_MOD_CSI_IN_KEY:
                Ret = ISP_Mod_Csi_In_Test();
                break;

            case TEST_MOD_CBAR_KEY:
                Ret = ISP_Mod_Cbar_Test();
                break;

            case TEST_MOD_CROP_KEY:
                Ret = ISP_Mod_Crop_Test();
                break;

            case TEST_MOD_DC_KEY:
                Ret = ISP_Mod_Dc_Test();
                break;

            case TEST_MOD_DEC_KEY:
                Ret = ISP_Mod_Dec_Test();
                break;

            case TEST_MOD_OBA_KEY:
                Ret = ISP_Mod_Oba_Test();
                break;

            case TEST_MOD_OBC_KEY:
                Ret = ISP_Mod_Obc_Test();
                break;

            case TEST_MOD_LCBQ_KEY:
                Ret = ISP_Mod_Lcbq_Test();
                break;

            case TEST_MOD_SC_KEY:
                Ret = ISP_Mod_Sc_Test();
                break;

#if defined(SUPPORT_DUMP_MODULE)
            case TEST_MOD_DUMP_KEY:
                Ret = ISP_Mod_Dump_Test();
                break;

#endif //#if defined(SUPPORT_DUMP_MODULE)
            case TEST_MOD_ISP_IN_KEY:
                Ret = ISP_Mod_Isp_In_Test();
                break;

            case TEST_MOD_DBC_KEY:
                Ret = ISP_Mod_Dbc_Test();
                break;

            case TEST_MOD_CTC_KEY:
                Ret = ISP_Mod_Ctc_Test();
                break;

            case TEST_MOD_OECF_KEY:
                Ret = ISP_Mod_Oecf_Test();
                break;

            case TEST_MOD_OECFHM_KEY:
                Ret = ISP_Mod_OecfHm_Test();
                break;

            case TEST_MOD_LCCF_KEY:
                Ret = ISP_Mod_Lccf_Test();
                break;

            case TEST_MOD_AWB_KEY:
                Ret = ISP_Mod_Awb_Test();
                break;

            case TEST_MOD_CFA_KEY:
                Ret = ISP_Mod_Cfa_Test();
                break;

            case TEST_MOD_CAR_KEY:
                Ret = ISP_Mod_Car_Test();
                break;

            case TEST_MOD_CCM_KEY:
                Ret = ISP_Mod_Ccm_Test();
                break;

            case TEST_MOD_GMARGB_KEY:
                Ret = ISP_Mod_GmaRgb_Test();
                break;

            case TEST_MOD_R2Y_KEY:
                Ret = ISP_Mod_R2y_Test();
                break;

            case TEST_MOD_YHIST_KEY:
                Ret = ISP_Mod_YHist_Test();
                break;

            case TEST_MOD_YCRV_KEY:
                Ret = ISP_Mod_YCrv_Test();
                break;

            case TEST_MOD_SHRP_KEY:
                Ret = ISP_Mod_Shrp_Test();
                break;

            case TEST_MOD_DNYUV_KEY:
                Ret = ISP_Mod_Dnyuv_Test();
                break;

            case TEST_MOD_SAT_KEY:
                Ret = ISP_Mod_Sat_Test();
                break;

            case TEST_MOD_OUT_KEY:
                Ret = ISP_Mod_Out_Test();
                break;

            case TEST_MOD_TIL_KEY:
                Ret = ISP_Mod_Til_Test();
                break;

            case TEST_MOD_BUF_KEY:
                Ret = ISP_Mod_Buf_Test();
                break;

            //-----------------------------------------------------------------
            case SET_ISP_AWB_NORMAL_KEY:
                LOG_INFO("Set the AWB gain as normal.\n");
                Ret = AWB_SetWbGain(0, 0);
                if (Ret) {
                    LOG_INFO("Failed to call AWB_SetWbGain(0, 0) function!\n");
                }
                break;

            case SET_ISP_AWB_R_GAIN_X2_KEY:
                LOG_INFO("Set red gain x 2 to the AWB gain.\n");
                Ret = AWB_SetWbGain(0, 1);
                if (Ret) {
                    LOG_INFO("Failed to call AWB_SetWbGain(0, 1) function!\n");
                }
                break;

            case SET_ISP_AWB_G_GAIN_X2_KEY:
                LOG_INFO("Set green gain x 2 to the AWB gain.\n");
                Ret = AWB_SetWbGain(0, 2);
                if (Ret) {
                    LOG_INFO("Failed to call AWB_SetWbGain(0, 2) function!\n");
                }
                break;

            case SET_ISP_AWB_B_GAIN_X2_KEY:
                LOG_INFO("Set blue gain x 2 to the AWB gain.\n");
                Ret = AWB_SetWbGain(0, 3);
                if (Ret) {
                    LOG_INFO("Failed to call AWB_SetWbGain(0, 3) function!\n");
                }
                break;

            //-----------------------------------------------------------------
            case SET_ISP_R2Y_NORMAL_KEY:
                LOG_INFO("Set the R2Y matrix as normal.\n");
                Ret = R2Y_SetMatrix(0, 0);
                if (Ret) {
                    LOG_INFO("Failed to call R2Y_SetMatrix(0, 0) function!\n");
                }
                break;


            case SET_ISP_R2Y_REMOVE_R_KEY:
                LOG_INFO("Remove red component from the R2Y matrix.\n");
                Ret = R2Y_SetMatrix(0, 1);
                if (Ret) {
                    LOG_INFO("Failed to call R2Y_SetMatrix(0, 1) function!\n");
                }
                break;

            case SET_ISP_R2Y_REMOVE_G_KEY:
                LOG_INFO("Remove green component from the R2Y matrix.\n");
                Ret = R2Y_SetMatrix(0, 2);
                if (Ret) {
                    LOG_INFO("Failed to call R2Y_SetMatrix(0, 2) function!\n");
                }
                break;

            case SET_ISP_R2Y_REMOVE_B_KEY:
                LOG_INFO("Remove blue component from the R2Y matrix.\n");
                Ret = R2Y_SetMatrix(0, 3);
                if (Ret) {
                    LOG_INFO("Failed to call R2Y_SetMatrix(0, 3) function!\n");
                }
                break;

            //-----------------------------------------------------------------
            case SET_CSI_SHADOW_ENABLE_KEY:
                LOG_INFO("CSI shadow update enable.\n");
                Ret = SUD_SetCsiShadowEnable(TEST_ISP_DEVICE);
                if (Ret) {
                    LOG_INFO("Failed to call SUD_SetCsiShadowEnable "\
                        "function!\n");
                }
                break;

            case SET_CSI_SHADOW_DISABLE_KEY:
                LOG_INFO("CSI shadow update disable.\n");
                Ret = SUD_SetCsiShadowDisable(TEST_ISP_DEVICE);
                if (Ret) {
                    LOG_INFO("Failed to call SUD_SetCsiShadowDisable "\
                        "function!\n");
                }
                break;

            case SET_ISP_SHADOW_ENABLE_KEY:
                LOG_INFO("ISP shadow update enable.\n");
                Ret = SUD_SetIspShadowEnable(TEST_ISP_DEVICE);
                if (Ret) {
                    LOG_INFO("Failed to call SUD_SetIspShadowEnable "\
                        "function!\n");
                }
                break;

            case SET_ISP_SHADOW_DISABLE_KEY:
                LOG_INFO("ISP shadow update disable.\n");
                Ret = SUD_SetIspShadowDisable(TEST_ISP_DEVICE);
                if (Ret) {
                    LOG_INFO("Failed to call SUD_SetIspShadowDisable "\
                        "function!\n");
                }
                break;

            //-----------------------------------------------------------------
            //=== Godspeed === Add new memory/buffer type support here.
            case PRINT_SC_DUMP_BUF_KEY:
                {
                    ST_MEM_INFO stMemInfo;

                    Ret = STFAPI_ISP_ShotMemBufGet(
                        EN_SHOT_KIND_COMPLETED,
                        EN_MEM_KIND_SC,
                        &stMemInfo,
                        NULL,
                        0
                        );
                    if (STF_SUCCESS != Ret) {
                        LOG_INFO("Failed to get SC dumping buffer!\n");
                    } else {
                        LOG_INFO("ScDumpBuf = %p!\n", stMemInfo.pvMemBuf);
                        Ret = SC_PrintScDumpBuffer(stMemInfo.pvMemBuf);
                    }
                }
                break;

            //=== Godspeed === Add new memory/buffer type support here.
            case PRINT_Y_HIST_BUF_KEY:
                {
                    ST_MEM_INFO stMemInfo;

                    Ret = STFAPI_ISP_ShotMemBufGet(
                        EN_SHOT_KIND_COMPLETED,
                        EN_MEM_KIND_Y_HIST,
                        &stMemInfo,
                        NULL,
                        0
                        );
                    if (STF_SUCCESS != Ret) {
                        LOG_INFO("Failed to get Y histogram buffer!\n");
                    } else {
                        LOG_INFO("YHistBuf = %p!\n", stMemInfo.pvMemBuf);
                        Ret = YHist_PrintYHistBuffer(stMemInfo.pvMemBuf);
                    }
                }
                break;

            //-----------------------------------------------------------------
            case CHECK_ISP_PARAM_KEY:
                Ret = ISP_Check_Isp_Param_Value(TEST_ISP_DEVICE);
                break;

            case CHECK_SENSOR_PARAM_KEY:
                {
#if defined(V4L2_DRIVER)
                    u8SensorIdx = 0;
                    u8SensorMode = 0;
                    u8SensorFlipping = SENSOR_FLIP_NONE;
                    strncpy(szSensorName, "sc2235dvp", sizeof(szSensorName));
                    //strncpy(szSensorName, "ov4689mipi", sizeof(szSensorName));
                    //strncpy(szSensorName, "imx219mipi", sizeof(szSensorName));
                    if (-1 == (u16SensorId = STFLIB_ISP_SENSOR_GetSensorId(
                        szSensorName
                        ))) {
                        LOG_ERROR("Failed to get the sensor Id of \"%s\"!!!\n",
                            szSensorName);
                        break;
                    }

                    if (STF_SUCCESS != (Ret = STFLIB_ISP_SENSOR_SturctInitialize(
                        &stSensor,
  #if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                        STFLIB_ISP_BASE_GetConnections(TEST_ISP_DEVICE),
                        TEST_ISP_DEVICE,
  #endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
                        u16SensorId,
                        u8SensorIdx
                        ))) {
                        LOG_ERROR(
                            "Failed to initialize ST_SENSOR structure!!!\n");
                        break;
                    }

                    Ret = ISP_Check_Sensor_Param_Value(
                        &stSensor,
                        g_au16SensorParam,
                        sizeof(g_au16SensorParam) / sizeof(STF_U16)
                        );

                    if (STF_SUCCESS != (Ret = stSensor.Destroy(&stSensor))) {
                        LOG_ERROR(
                            "Failed to uninitialize ST_SENSOR structure!!!\n");
                        break;
                    }
                }
#else
                Ret = ISP_Check_Sensor_Param_Value(
                    g_au16SensorParam,
                    sizeof(g_au16SensorParam) / sizeof(STF_U16)
                    );
#endif //#if defined(V4L2_DRIVER)
                break;

            case CHECK_REG_KEY:
                Ret = ISP_Check_Register_Value(TEST_ISP_DEVICE);
                break;

            //-----------------------------------------------------------------
            case TEST_API_REG_READ_WRITE_KEY:
                {
                    int i;
                    int j;
                    STF_BOOL8 bFound;
                    STF_U32 u32Offset;
                    STF_U32 u32Length;
                    STF_U32 u32ReadBuffer[1024];
                    STF_U32 u32WriteBuffer[1024];

                    //STFLIB_ISP_RegDebugInfoEnable(STF_ENABLE);
                    LOG_INFO("=================================\n");
                    {
#if 0
                        u32Offset = (rand() & 0x0FFC);
                        u32Length = 500;
                        if (PESUDO_REG_SIZE < (u32Offset + (u32Length * 4))) {
                            u32Offset = PESUDO_REG_SIZE - (u32Length * 4);
                        }
#else
                        u32Offset = (rand() & 0x07FC);
                        u32Length = 500;
#endif
                        for (i = 0; i < u32Length; i++) {
                            u32WriteBuffer[i] = (rand() & 0xFFFFFFFF);
                        }
                        Ret = STFAPI_ISP_RegWriteSeries(
                            TEST_ISP_DEVICE,
                            u32Offset,
                            u32Length,
                            &u32WriteBuffer[0]
                            );
                        if (Ret) {
                            LOG_ERROR("Failed to call "\
                                "\"STFAPI_ISP_RegWriteSeries\" API function\n");
                        } else {
                            LOG_INFO("Call \"STFAPI_ISP_RegWriteSeries\" "\
                                "API function is successful\n");
                        }
#if 0
                        for (i = 0; i < u32Length; i++) {
                            LOG_INFO("%03d-Reg[0x%08X] = 0x%08X\n",
                                i,
                                u32Offset + i * 4,
                                u32WriteBuffer[i]
                                );
                        }
#endif
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        u32Length = 500;
                        //for (i = 0; i < u32Length; i++)
                        //{
                        //    u32ReadBuffer[i] = 0;
                        //}
                        Ret = STFAPI_ISP_RegReadSeries(
                            TEST_ISP_DEVICE,
                            u32Offset,
                            u32Length,
                            &u32ReadBuffer[0]
                            );
                        if (Ret) {
                            LOG_ERROR("Failed to call "\
                                "\"STFAPI_ISP_RegReadSeries\" API function\n");
                        } else {
                            LOG_INFO("Call \"STFAPI_ISP_RegReadSeries\" "\
                                "API function is successful\n");
                        }
#if 0
                        for (i = 0; i < u32Length; i++) {
                            LOG_INFO("%03d-Reg[0x%08X] = 0x%08X\n",
                                i,
                                u32Offset + i * 4,
                                u32ReadBuffer[i]
                                );
                        }
#endif
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        u32Length = 500;
                        for (i = 0; i < u32Length; i++) {
                            if (u32ReadBuffer[i] != u32WriteBuffer[i]) {
                                Ret = STF_ERROR_FAILURE;
                                LOG_ERROR("%03d-ReadBuffer[0x%08X]=0x%08X "\
                                    "!= WriteBuffer[0x%08X]=0x%08X\n",
                                    i,
                                    u32Offset + 1 * 4,
                                    u32ReadBuffer[i],
                                    u32Offset + 1 * 4,
                                    u32WriteBuffer[i]
                                    );
                                break;
                            }
                        }
                        if (i == u32Length) {
                            LOG_INFO("The read buffer content compared with "\
                                "the write buffer is the same.\n");
                            LOG_INFO("Register read/write series method "\
                                "is successful!\n");
                        }
                    }
                    LOG_INFO("=================================\n");

                    LOG_INFO("=================================\n");
                    {
                        u32Length = 500;
                        for (i = 0; i < u32Length; i++) {
                            do {
                                u32Offset = (rand() & 0x0FFC);
                                bFound = STF_FALSE;
                                for (j = 0; j < i; j++) {
                                    if (u32Offset == u32WriteBuffer[j * 2]) {
                                        bFound = STF_TRUE;
                                        break;
                                    }
                                }
                            } while (STF_TRUE == bFound);
                            u32WriteBuffer[i * 2] = u32Offset;
                            u32WriteBuffer[i * 2 + 1] = (rand() & 0xFFFFFFFF);
                        }
                        Ret = STFAPI_ISP_RegWriteByTable(
                            TEST_ISP_DEVICE,
                            u32Length,
                            (ST_REG_TBL *)&u32WriteBuffer[0]
                            );
                        if (Ret) {
                            LOG_ERROR("Failed to call "\
                                "\"STFAPI_ISP_RegWriteByTable\" "\
                                "API function\n");
                        } else {
                            LOG_INFO("Call \"STFAPI_ISP_RegWriteByTable\" "\
                                "API function is successful\n");
                        }
#if 0
                        for (i = 0; i < u32Length; i++) {
                            LOG_INFO("%03d-Reg[0x%08X] = 0x%08X\n",
                                i,
                                u32WriteBuffer[i * 2],
                                u32WriteBuffer[i * 2 + 1]
                                );
                        }
#endif
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        u32Length = 500;
                        for (i = 0; i < u32Length; i++) {
                            u32ReadBuffer[i * 2] = u32WriteBuffer[0 + i * 2];
                            //u32ReadBuffer[i * 2 + 1] = 0;
                        }
                        Ret = STFAPI_ISP_RegReadByTable(
                            TEST_ISP_DEVICE,
                            u32Length,
                            (ST_REG_TBL *)&u32ReadBuffer[0]
                            );
                        if (Ret) {
                            LOG_ERROR("Failed to call "\
                                "\"STFAPI_ISP_RegReadByTable\" API function\n");
                        } else  {
                            LOG_INFO("Call \"STFAPI_ISP_RegWriteByTable\" "\
                                "API function is successful\n");
                        }
#if 0
                        for (i = 0; i < u32Length; i++) {
                            LOG_INFO("%03d-Reg[0x%08X] = 0x%08X\n",
                                i,
                                u32ReadBuffer[i * 2],
                                u32ReadBuffer[i * 2 + 1]
                                );
                        }
#endif
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        u32Length = 500;
                        for (i = 0; i < u32Length; i++) {
                            if (u32ReadBuffer[i * 2 + 1]
                                != u32WriteBuffer[i * 2 + 1]) {
                                Ret = STF_ERROR_FAILURE;
                                LOG_ERROR("%03d-ReadBuffer[0x%08X]=0x%08X "\
                                    "!= WriteBuffer[0x%08X]=0x%08X\n",
                                    i,
                                    u32ReadBuffer[i * 2],
                                    u32ReadBuffer[i * 2 + 1],
                                    u32WriteBuffer[i * 2],
                                    u32WriteBuffer[i * 2 + 1]
                                    );
                                break;
                            }
                        }
                        if (i == u32Length) {
                            LOG_INFO("The read buffer content compared with "\
                                "the write buffer is the same.\n");
                            LOG_INFO("Register read/write by table method "\
                                "is successful!\n");
                        }
                    }
                    LOG_INFO("=================================\n");
                    //STFLIB_ISP_RegDebugInfoEnable(STF_DISABLE);
                }
                break;

            case TEST_API_MEM_GET_BUF_KEY:
                {
                    //=========================================================
                    //---------------------------------------------------------
                    LOG_INFO("=================================\n");
                    {
                        ST_MEM_INFO stMemInfo = {
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            NULL
                        };

                        Ret = STFAPI_ISP_ShotMemBufGet(
                            EN_SHOT_KIND_COMPLETED,
                            EN_MEM_KIND_UO,
                            &stMemInfo,
                            NULL,
                            0
                            );
                        LOG_INFO("STFAPI_ISP_ShotMemBufGet Completed shot, "\
                            "enMemKind = %d\n", EN_MEM_KIND_UO);
                        LOG_INFO("u32ImgWidth = %d\n", stMemInfo.u32ImgWidth);
                        LOG_INFO("u32ImgHeight = %d\n", stMemInfo.u32ImgHeight);
                        LOG_INFO("u32ImgStride = %d\n", stMemInfo.u32ImgStride);
                        LOG_INFO("u32BitDepth = %d\n", stMemInfo.u32BitDepth);
                        LOG_INFO("u32MosaicType = %d\n",
                            stMemInfo.u32MosaicType);
                        LOG_INFO("u32MemSize = %d\n", stMemInfo.u32MemSize);
                        LOG_INFO("pvMemBuf = %p\n", stMemInfo.pvMemBuf);
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        ST_MEM_INFO stMemInfo = {
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            NULL
                        };

                        Ret = STFAPI_ISP_ShotMemBufGet(
                            EN_SHOT_KIND_COMPLETED,
                            EN_MEM_KIND_SS0,
                            &stMemInfo,
                            NULL,
                            0
                            );
                        LOG_INFO("STFAPI_ISP_ShotMemBufGet Completed shot, "\
                            "enMemKind = %d\n", EN_MEM_KIND_SS0);
                        LOG_INFO("u32ImgWidth = %d\n", stMemInfo.u32ImgWidth);
                        LOG_INFO("u32ImgHeight = %d\n", stMemInfo.u32ImgHeight);
                        LOG_INFO("u32ImgStride = %d\n", stMemInfo.u32ImgStride);
                        LOG_INFO("u32BitDepth = %d\n", stMemInfo.u32BitDepth);
                        LOG_INFO("u32MosaicType = %d\n",
                            stMemInfo.u32MosaicType);
                        LOG_INFO("u32MemSize = %d\n", stMemInfo.u32MemSize);
                        LOG_INFO("pvMemBuf = %p\n", stMemInfo.pvMemBuf);
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        ST_MEM_INFO stMemInfo = {
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            NULL
                        };

                        Ret = STFAPI_ISP_ShotMemBufGet(
                            EN_SHOT_KIND_COMPLETED,
                            EN_MEM_KIND_SS1,
                            &stMemInfo,
                            NULL,
                            0
                            );
                        LOG_INFO("STFAPI_ISP_ShotMemBufGet Completed shot, "\
                            "enMemKind = %d\n", EN_MEM_KIND_SS1);
                        LOG_INFO("u32ImgWidth = %d\n", stMemInfo.u32ImgWidth);
                        LOG_INFO("u32ImgHeight = %d\n", stMemInfo.u32ImgHeight);
                        LOG_INFO("u32ImgStride = %d\n", stMemInfo.u32ImgStride);
                        LOG_INFO("u32BitDepth = %d\n", stMemInfo.u32BitDepth);
                        LOG_INFO("u32MosaicType = %d\n",
                            stMemInfo.u32MosaicType);
                        LOG_INFO("u32MemSize = %d\n", stMemInfo.u32MemSize);
                        LOG_INFO("pvMemBuf = %p\n", stMemInfo.pvMemBuf);
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        ST_MEM_INFO stMemInfo = {
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            NULL
                        };

                        Ret = STFAPI_ISP_ShotMemBufGet(
                            EN_SHOT_KIND_COMPLETED,
                            EN_MEM_KIND_TIL_1_WR,
                            &stMemInfo,
                            NULL,
                            0
                            );
                        LOG_INFO("STFAPI_ISP_ShotMemBufGet Completed shot, "\
                            "enMemKind = %d\n", EN_MEM_KIND_TIL_1_WR);
                        LOG_INFO("u32ImgWidth = %d\n", stMemInfo.u32ImgWidth);
                        LOG_INFO("u32ImgHeight = %d\n", stMemInfo.u32ImgHeight);
                        LOG_INFO("u32ImgStride = %d\n", stMemInfo.u32ImgStride);
                        LOG_INFO("u32BitDepth = %d\n", stMemInfo.u32BitDepth);
                        LOG_INFO("u32MosaicType = %d\n",
                            stMemInfo.u32MosaicType);
                        LOG_INFO("u32MemSize = %d\n", stMemInfo.u32MemSize);
                        LOG_INFO("pvMemBuf = %p\n", stMemInfo.pvMemBuf);
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        ST_MEM_INFO stMemInfo = {
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            NULL
                        };

                        Ret = STFAPI_ISP_ShotMemBufGet(
                            EN_SHOT_KIND_COMPLETED,
                            EN_MEM_KIND_DUMP,
                            &stMemInfo,
                            NULL,
                            0
                            );
                        LOG_INFO("STFAPI_ISP_ShotMemBufGet Completed shot, "\
                            "enMemKind = %d\n", EN_MEM_KIND_DUMP);
                        LOG_INFO("u32ImgWidth = %d\n", stMemInfo.u32ImgWidth);
                        LOG_INFO("u32ImgHeight = %d\n", stMemInfo.u32ImgHeight);
                        LOG_INFO("u32ImgStride = %d\n", stMemInfo.u32ImgStride);
                        LOG_INFO("u32BitDepth = %d\n", stMemInfo.u32BitDepth);
                        LOG_INFO("u32MosaicType = %d\n",
                            stMemInfo.u32MosaicType);
                        LOG_INFO("u32MemSize = %d\n", stMemInfo.u32MemSize);
                        LOG_INFO("pvMemBuf = %p\n", stMemInfo.pvMemBuf);
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        ST_MEM_INFO stMemInfo = {
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            NULL
                        };

                        Ret = STFAPI_ISP_ShotMemBufGet(
                            EN_SHOT_KIND_COMPLETED,
                            EN_MEM_KIND_SC,
                            &stMemInfo,
                            NULL,
                            0
                            );
                        LOG_INFO("STFAPI_ISP_ShotMemBufGet Completed shot, "\
                            "enMemKind = %d\n", EN_MEM_KIND_SC);
                        LOG_INFO("u32ImgWidth = %d\n", stMemInfo.u32ImgWidth);
                        LOG_INFO("u32ImgHeight = %d\n", stMemInfo.u32ImgHeight);
                        LOG_INFO("u32ImgStride = %d\n", stMemInfo.u32ImgStride);
                        LOG_INFO("u32BitDepth = %d\n", stMemInfo.u32BitDepth);
                        LOG_INFO("u32MosaicType = %d\n",
                            stMemInfo.u32MosaicType);
                        LOG_INFO("u32MemSize = %d\n", stMemInfo.u32MemSize);
                        LOG_INFO("pvMemBuf = %p\n", stMemInfo.pvMemBuf);
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    {
                        ST_MEM_INFO stMemInfo = {
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            NULL
                        };

                        Ret = STFAPI_ISP_ShotMemBufGet(
                            EN_SHOT_KIND_COMPLETED,
                            EN_MEM_KIND_Y_HIST,
                            &stMemInfo,
                            NULL,
                            0
                            );
                        LOG_INFO("STFAPI_ISP_ShotMemBufGet Completed shot, "\
                            "enMemKind = %d\n", EN_MEM_KIND_Y_HIST);
                        LOG_INFO("u32ImgWidth = %d\n", stMemInfo.u32ImgWidth);
                        LOG_INFO("u32ImgHeight = %d\n", stMemInfo.u32ImgHeight);
                        LOG_INFO("u32ImgStride = %d\n", stMemInfo.u32ImgStride);
                        LOG_INFO("u32BitDepth = %d\n", stMemInfo.u32BitDepth);
                        LOG_INFO("u32MosaicType = %d\n",
                            stMemInfo.u32MosaicType);
                        LOG_INFO("u32MemSize = %d\n", stMemInfo.u32MemSize);
                        LOG_INFO("pvMemBuf = %p\n", stMemInfo.pvMemBuf);
                        LOG_INFO("---------------------------------\n");
                    }
                    //---------------------------------------------------------
                    //LOG_INFO("=================================\n");
                    //=========================================================
                }
                break;

            case TEST_API_START_ISP_CAP_KEY:
                STFAPI_ISP_StartIspCapture();
                break;

            case TEST_API_STOP_ISP_CAP_KEY:
                STFAPI_ISP_StopIspCapture();
                break;

            //-----------------------------------------------------------------
            case TEST_HELP_KEY:
                printUsage();
                break;

            case EXIT_KEY:  //Exit application
                printf("exit key pressed\n");
                loopActive=false;
                break;
        }
    }

    printf("Exiting loop\n");

#ifdef UNUSED_CODE_AND_VARIABLE
    ISPC::LOG_Perf_Summary();
#endif //UNUSED_CODE_AND_VARIABLE

#if defined(V4L2_DRIVER)
    Ret = STFLIB_ISP_DEVICE_StructUninitialize(&stDevUo0);
    // check if opening was successfull
    if (STF_SUCCESS != Ret) {
        LOG_ERROR("Failed to close \'%s\': %d, %s\n",
            stDevUo0.szDeviceName, errno, strerror(errno));
        return -1;
    }

#endif //#if defined(V4L2_DRIVER)
    iSysRet = STFAPI_ISP_StopIspMainThread();
    Ret = STFLIB_ISP_BASE_DisconnectionAll(TEST_ISP_DEVICE);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int ret;
    struct termios orig_term_attr;

    enableRawMode(&orig_term_attr);

    printf("CHANGELIST %s - DATE %s\n", CI_CHANGELIST_STR, CI_DATE_STR);

    ret = run(argc, argv);

    disableRawMode(&orig_term_attr);

    return EXIT_SUCCESS;
}
