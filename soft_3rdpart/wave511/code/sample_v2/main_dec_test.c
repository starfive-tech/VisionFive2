/*
 * Copyright (c) 2019, Chips&Media
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include "cnm_app.h"
#include "decoder_listener.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PAGE_SHIFT  (12)
#define PAGE_SIZE   (1UL << PAGE_SHIFT)
#define PAGE_MASK   (~(PAGE_SIZE - 1))

#define DEV_PAGEMAP     "/proc/self/pagemap"
int page_fd;
int  vic_pagemap_init()
{
    page_fd = open(DEV_PAGEMAP, O_RDONLY);
    if(page_fd == -1)
    {
        perror("page_fd open:\n");
        return -1;
    }
    return page_fd;
}

void  vic_pagemap_release()
{
    close(page_fd);
}

unsigned long  us_virt_to_phy(void * vaddr)
{
    unsigned long paddr,v_addr;
    unsigned long v_pageIndex,v_offset,page_offset;
    uint64_t phy_pageIndex,item = 0;
    int pageSize;

    v_addr = (unsigned long)vaddr;
    pageSize = PAGE_SIZE;
    page_offset = v_addr & ~PAGE_MASK;
    v_pageIndex = (v_addr & PAGE_MASK) >> PAGE_SHIFT;
    v_offset = v_pageIndex * sizeof(uint64_t);

    if(lseek(page_fd, v_offset, SEEK_SET) == -1) 
    {   
        perror("lseek:\n");
        goto page_error;
    }   

    if(read(page_fd, &item, sizeof(uint64_t)) != sizeof(uint64_t))
    {   
        perror("read item error\n");
        goto page_error;
    }   

    if((((uint64_t)1 << 63) & item) == 0)
    {   
        perror("page present is 0\n");
        goto page_error;
    }   

    phy_pageIndex = (((uint64_t)1 << 55) - 1) & item;//计算物理页号，即取item的bit0-54

    paddr = (phy_pageIndex * pageSize) + page_offset;
    return paddr;
page_error:
    close(page_fd);
    return 0;
}

void print_in_line_u64(unsigned char *p_name, unsigned long phys_addr, unsigned long *p_buf, unsigned int len)
{
    unsigned int len_tmp = len;
    unsigned int i = 0, j = 0;
    unsigned int line = 0;
    unsigned int rest = 0;
    unsigned int* ptmp = NULL;

    printf("%s: phys_addr:%#llx,vir_addr:%#llx, %#x\n", p_name, phys_addr,(unsigned long)p_buf, len);

    if(len >= 0x1000){
        len_tmp = 0x1000/32;  //print 128 size of memory.
        //len_tmp = len/8;  //for all
    }
    else{
        len_tmp = len/8;   //print real 100% size of memory.
    }
	
    rest = len/4;       //one line print 8 u32
    for (i = 0; i < len_tmp; i += 4)
    {   
        if (!(i % 4)) 
        {   
            printf("  %#llx: ", (sizeof(phys_addr)*i + phys_addr));
        }   
        ptmp = (unsigned int*)(p_buf + i); 
        line = (rest > 8) ? 8 : rest;

        for (j = 0; j < line; j++)
        {   
            printf("%08x ", *(ptmp + j));
        }   
        printf("\n");
        rest -= line;
    }   
}


extern CodStd BSFeederFrameSize_GetDirectFormat(const char* path);

typedef struct CommandLineArgument{
    int argc;
    char **argv;
}CommandLineArgument;

static void Help(struct OptionExt *opt, const char *programName)
{
    int i;

    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", GetBasename(programName), API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input bistream\n", GetBasename(programName));
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-n [num]                    output frame number\n");
    VLOG(INFO, "-c                          compare with golden\n");
    VLOG(INFO, "                            0 : no comparator\n");
    VLOG(INFO, "                            1 : compare with golden yuv that specified --ref-yuv option\n");
    VLOG(INFO, "-d                          dual display. A linear fb is synchronous with a compressed fb.\n");

    for (i = 0;i < MAX_GETOPT_OPTIONS;i++) {
        if (opt[i].name == NULL)
            break;
        VLOG(INFO, "%s", opt[i].help);
    }
    }

static BOOL CheckTestConfig(TestDecConfig testConfig) {
    BOOL isValidParameters = TRUE;

    /* Check parameters */
    if (testConfig.skipMode < 0 || testConfig.skipMode == 3 || testConfig.skipMode > 4) {
        VLOG(ERR, "Invalid skip mode: %d\n", testConfig.skipMode);
        isValidParameters = FALSE;
    }
    if ((FORMAT_422 < testConfig.wtlFormat && testConfig.wtlFormat <= FORMAT_400) ||
        testConfig.wtlFormat < FORMAT_420 || testConfig.wtlFormat >= FORMAT_MAX) {
            VLOG(ERR, "Invalid WTL format(%d)\n", testConfig.wtlFormat);
            isValidParameters = FALSE;
    }
    if (isValidParameters == TRUE && (testConfig.scaleDownWidth > 0 || testConfig.scaleDownHeight > 0)) {
    }
    if (testConfig.renderType < RENDER_DEVICE_NULL || testConfig.renderType >= RENDER_DEVICE_MAX) {
        VLOG(ERR, "unknown render device type(%d)\n", testConfig.renderType);
        isValidParameters = FALSE;
    }
    if (testConfig.thumbnailMode == TRUE && testConfig.skipMode != 0) {
        VLOG(ERR, "Turn off thumbnail mode or skip mode\n");
        isValidParameters = FALSE;
    }


    return isValidParameters;
}

static BOOL ParseArgumentAndSetTestConfig(CommandLineArgument argument, TestDecConfig* testConfig)
{
    Int32  opt  = 0, i = 0, index = 0;
    Uint32 argFeeding = 0;            /* auto, depeding on --bsmode */
    int    argc = argument.argc;
    char** argv = argument.argv;
    char** stop = NULL;
    char* optString = "c:hvn:";
    struct option options[MAX_GETOPT_OPTIONS];
    struct OptionExt options_help[MAX_GETOPT_OPTIONS] = {
        {"output",                1, NULL, 0, "--output                    output YUV path\n"},    /*  0 */
        {"input",                 1, NULL, 0, "--input                     bitstream path\n"},
        {"codec",                 1, NULL, 0, "--codec                     codec index, HEVC : 12, VP9 : 13, AVS2 : 14, AVC : 0\n"},
        {"render",                1, NULL, 0, "--render                    0 : no rendering picture\n"
        "                                                                  1 : render a picture with the framebuffer device\n"},
        {"bsmode",                1, NULL, 0, "--bsmode                    0: INTERRUPT MODE, 1: reserved, 2: PICEND MODE\n"},
        {"disable-wtl",           0, NULL, 0, "--disable-wtl               disable WTL. default on.\n"},    /*  5 */
        {"coreIdx",               1, NULL, 0, "--coreIdx                   core index: default 0\n"},
        {"loop-count",            1, NULL, 0, "--loop-count                integer number. loop test, default 0\n"},
        {"enable-cbcrinterleave", 0, NULL, 0, "--enable-cbcrinterleave     enable cbcrInterleave(NV12), default off\n"},
        {"stream-endian",         1, NULL, 0, "--stream-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"frame-endian",          1, NULL, 0, "--frame-endian              16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"enable-nv21",           0, NULL, 0, "--enable-nv21               enable NV21, default off\n"},
        {"secondary-axi",         1, NULL, 0, "--secondary-axi             0~7: bit oring values, Please refer programmer's guide or datasheet\n"},
        {"wtl-format",            1, NULL, 0, "--wtl-format                yuv format. default 0.\n"},
        {"num-vcores",            1, NULL, 0, "--num-vcores                number vcores to be used, default 1\n"},
        {"enable-thumbnail",      0, NULL, 0, "--enable-thumbnail          enable thumbnail mode. default off\n"},
        {"ref-yuv",               1, NULL, 0, "--ref-yuv                   golden yuv path\n"},
        {"skip-mode",             1, NULL, 0, "--skip-mode                 0: off, 1: Non-IRAP, 2: Non-Ref, 4: skip-all\n"},
        {"bwopt",                 1, NULL, 0, "--bwopt                     bandwidth opimization\n"},
        {"sclw",                  1, NULL, 0, "--sclw                      scale width of picture down. ceil8(width/8) <= scaledW <= width, 99 for random test\n"},    /* 20 */
        {"sclh",                  1, NULL, 0, "--sclh                      scale height of picture down, ceil8(height/8) <= scaledH <= height, 99 for random test\n"},
        {"cra-bla",               0, NULL, 0, "--cra-bla                   Handle CRA as BLA\n"},
        {"userdata",              1, NULL, 0, "--userdata   hexadecimal(hevc)     0 - disable userdata            4 - VUI                     10 - pic_timing                 20 - itu_t_35 prefix\n"
                                              "                                  40 - unregistered prefix        80 - itu_t_35 suffix        100 - unregistered suffix       400 - mastering_col_vol\n"
                                              "                                 800 - chroma resample hint     1000 - knee function         2000 - tone mapping             4000 - film grain\n"
                                              "                                8000 - content light level     10000 - color remapping   10000000 - itu_t_35 prefix 1    20000000 - itu_t_35 prefix 2\n"
                                              "                            40000000 - itu_t_35 suffix 1    80000000 - itu_t_35 suffix 2 \n"
                                              "--userdata   hexadecimal(avc)      0 - disable userdata            4 - VUI                     10 - pic_timing                 20 - itu_t_35\n"
                                              "                                  40 - unregistered               80 - reserved               100 - reserved                  400 - reserved\n"
                                              "                                 800 - reserved                 1000 - reserved              2000 - tone mapping             4000 - film grain\n"
                                              "                                8000 - reserved                10000 - color remapping   10000000 - reserved             20000000 - reserved\n"
                                              "                            All bits can be or-ing. ex) 404 -> mastering_color_volume | VUI\n"},
        {"feeding",               1, NULL, 0, "--feeding                   0: auto, 1: fixed-size, 2: ffempg, 3: size(4byte)+es\n"},    /* 25 */
        {"rseed",                 1, NULL, 0, "--rseed                     random seed value, Hexadecimal\n"},
        {NULL,                    0, NULL, 0},
    };
    Uint32  minScaleWidth, minScaleHeight;


    for (i = 0; i < MAX_GETOPT_OPTIONS;i++) {
        if (options_help[i].name == NULL)
            break;
        osal_memcpy(&options[i], &options_help[i], sizeof(struct option));
    }

    while ((opt=getopt_long(argc, argv, optString, options, (int *)&index)) != -1) {
        switch (opt) {
        case 'c':
            testConfig->compareType = atoi(optarg);
            if (testConfig->compareType < NO_COMPARE || testConfig->compareType > YUV_COMPARE) {
                VLOG(ERR, "Invalid compare type(%d)\n", testConfig->compareType);
                Help(options_help, argv[0]);
                exit(1);
            }
            break;
        case 'n':
            testConfig->forceOutNum = atoi(optarg);
            break;
        case 'h':
            Help(options_help, argv[0]);
            return FALSE;
        case 0:
            if (!strcmp(options[index].name, "output")) {
                osal_memcpy(testConfig->outputPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig->outputPath);
            } else if (!strcmp(options[index].name, "input")) {
                osal_memcpy(testConfig->inputPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig->inputPath);
            } else if (!strcmp(options[index].name, "codec")) {
                testConfig->bitFormat = atoi(optarg);
            } else if (!strcmp(options[index].name, "render")) {
                testConfig->renderType = (RenderDeviceType)atoi(optarg);
                if (testConfig->renderType < RENDER_DEVICE_NULL || testConfig->renderType >= RENDER_DEVICE_MAX) {
                    VLOG(ERR, "unknown render device type(%d)\n", testConfig->renderType);
                    Help(options_help, argv[0]);
                    return FALSE;
        }
            } else if (!strcmp(options[index].name, "bsmode")) {
                testConfig->bitstreamMode = atoi(optarg);
            } else if (!strcmp(options[index].name, "disable-wtl")) {
                testConfig->enableWTL = FALSE;
                testConfig->wtlMode = FF_NONE;
            } else if (!strcmp(options[index].name, "coreIdx")) {
                testConfig->coreIdx = atoi(optarg);
            } else if (!strcmp(options[index].name, "loop-count")) {
                testConfig->loopCount = atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-cbcrinterleave")) {
                testConfig->cbcrInterleave = TRUE;
            } else if (!strcmp(options[index].name, "stream-endian")) {
                testConfig->streamEndian = (EndianMode)atoi(optarg);
            } else if (!strcmp(options[index].name, "frame-endian")) {
                testConfig->frameEndian = (EndianMode)atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-nv21")) {
                testConfig->nv21 = 1;
            } else if (!strcmp(options[index].name, "secondary-axi")) {
                testConfig->secondaryAXI = strtoul(optarg, stop, !strncmp("0x", optarg, 2) ? 16 : 10);
            } else if (!strcmp(options[index].name, "wtl-format")) {
                testConfig->wtlFormat = (FrameBufferFormat)atoi(optarg);
            } else if (!strcmp(options[index].name, "num-vcores")) {
                testConfig->wave.numVCores = atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-thumbnail")) {
                testConfig->thumbnailMode = TRUE;
            } else if (!strcmp(options[index].name, "ref-yuv")) {
                osal_memcpy(testConfig->refYuvPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig->refYuvPath);
            } else if (!strcmp(options[index].name, "skip-mode")) {
                testConfig->skipMode = atoi(optarg);
            } else if (!strcmp(options[index].name, "bwopt")) {
                testConfig->wave.bwOptimization = atoi(optarg);
            } else if (!strcmp(options[index].name, "sclw")) {
                testConfig->scaleDownWidth = VPU_CEIL(atoi(optarg), 2);
            } else if (!strcmp(options[index].name, "sclh")) {
                testConfig->scaleDownHeight = VPU_CEIL(atoi(optarg), 2);
            } else if (!strcmp(options[index].name, "cra-bla")) {
                testConfig->wave.craAsBla = TRUE;
            } else if (!strcmp(options[index].name, "userdata")) {
                testConfig->enableUserData = strtoul(optarg, stop, 16);
            } else if (!strcmp(options[index].name, "feeding")) {
                argFeeding = (Uint32)atoi(optarg);
            } else if (!strcmp(options[index].name, "fw-path")) {
                osal_memcpy(testConfig->fwPath, optarg, strlen(optarg));
            } else if (!strcmp(options[index].name, "annexb")) {
                testConfig->wave.av1Format = atoi(optarg);
                if (TRUE != testConfig->wave.av1Format) {
                    testConfig->wave.av1Format = 1; //AV1_STREAM_OBU
                } else {
                    testConfig->wave.av1Format = 2; //AV1_STREAM_ANNEXB
                }
            } else if (!strcmp(options[index].name, "rseed")) {
                randomSeed = strtoul(optarg, NULL, 16);
                srand(randomSeed);
            } else if (!strcmp(options[index].name, "bs-size")) {
                testConfig->bsSize = (size_t)atoi(optarg);
            } else {
                VLOG(ERR, "not exist param = %s\n", options[index].name);
                Help(options_help, argv[0]);
                return FALSE;
    }
            break;
        default:
            VLOG(ERR, "%s\n", optarg);
            Help(options_help, argv[0]);
            return FALSE;
    }
    }
    VLOG(INFO, "\n");

    switch (argFeeding) {
    case 0: /* AUTO */
        if (testConfig->bitstreamMode == BS_MODE_INTERRUPT) {
            testConfig->feedingMode = FEEDING_METHOD_FIXED_SIZE;
    }
    else {
            testConfig->feedingMode = FEEDING_METHOD_FRAME_SIZE;
    }
                break;
    case 1: /* FIXED SIZE */
        testConfig->feedingMode = FEEDING_METHOD_FIXED_SIZE;
        break;
    case 2: /* Using FFMPEG */
        testConfig->feedingMode = FEEDING_METHOD_FRAME_SIZE;
        break;
    case 3: /* for every frame, size(4byte) + es(1frame) */
        testConfig->feedingMode = FEEDING_METHOD_SIZE_PLUS_ES;
        break;
    default:
        VLOG(ERR, "--feeding=%d not supported\n", argFeeding);
        return FALSE;
        }

    if (testConfig->bitFormat == STD_VP9) {
        testConfig->feedingMode = FEEDING_METHOD_FRAME_SIZE;
        testConfig->bitstreamMode = BS_MODE_PIC_END;
    }

    minScaleWidth  = 8;
    minScaleHeight = 8;

    if (testConfig->scaleDownWidth != 0 && testConfig->scaleDownWidth < minScaleWidth) {
        VLOG(ERR, "The minimum scale-downed width: %d\n", minScaleWidth);
        return FALSE;
    }
    if (testConfig->scaleDownHeight != 0 && testConfig->scaleDownHeight < minScaleHeight) {
        VLOG(ERR, "The minimum scale-downed height: %d\n", minScaleHeight);
        return FALSE;
    }

    return TRUE;
}

static BOOL  s_abort = FALSE;
static BOOL  s_brun  = FALSE;   

static void ctrlc_handler(int a)
{ 
    s_abort = TRUE;

    if(s_brun==TRUE){
       CNMAppStop();
       s_brun = FALSE;
    }
}

int main(int argc, char **argv)
{
    char*               fwPath     = NULL;
    TestDecConfig       testConfig;
    CommandLineArgument argument;
    CNMComponentConfig  config;
    CNMTask             task;
    Component           feeder;
    Component           decoder;
    Component           renderer;
    Uint32              sizeInWord;
    Uint16*             pusBitCode=NULL;
    BOOL                ret;
    BOOL                testResult;
    DecListenerContext  lsnCtx;

    signal(SIGINT,ctrlc_handler);

    osal_memset(&argument, 0x00, sizeof(CommandLineArgument));
    osal_memset(&config,   0x00, sizeof(CNMComponentConfig));

    InitLog();
    SetDefaultDecTestConfig(&testConfig);

    argument.argc = argc;
    argument.argv = argv;
    if (ParseArgumentAndSetTestConfig(argument, &testConfig) == FALSE) {
        VLOG(ERR, "fail to ParseArgumentAndSetTestConfig()\n");
        return 1;
    }

    if (CheckTestConfig(testConfig) == FALSE) {
        VLOG(ERR, "fail to CheckTestConfig()\n");
        return 1;
    }

    testConfig.productId = (ProductId)VPU_GetProductId(testConfig.coreIdx);
    /* Need to Add */
    /* FW load & show version case*/

    switch (testConfig.productId) {
    case PRODUCT_ID_521: fwPath = CORE_6_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_511: fwPath = CORE_6_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_517: fwPath = CORE_7_BIT_CODE_FILE_PATH; break;
    default:
        VLOG(ERR, "Unknown product id: %d\n", testConfig.productId);
        return 1;
    }

    VLOG(INFO, "FW PATH = %s\n", fwPath);
    if (LoadFirmware(testConfig.productId, (Uint8**)&pusBitCode, &sizeInWord, fwPath) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, fwPath);
        return 1;
    }

    config.testDecConfig = testConfig;
    config.bitcode       = (Uint8*)pusBitCode;
    config.sizeOfBitcode = sizeInWord;


	//vic_pagemap_init();
	//unsigned char name[] ="bitcode";
	//print_in_line_u64(name,us_virt_to_phy((void *)config.bitcode),(unsigned long *)config.bitcode,config.sizeOfBitcode*2);
#ifdef SUPPORT_FFMPEG_DEMUX
    /* webb add */
    {
        CodStd              vformat;
        vformat = BSFeederFrameSize_GetDirectFormat(config.testDecConfig.inputPath);
        VLOG(INFO,"%s bs-format is %d\n",config.testDecConfig.inputPath,vformat);
        if((vformat==STD_AVC)||(vformat==STD_HEVC)){
            config.testDecConfig.bitFormat = vformat;
            config.testDecConfig.feedingMode   = FEEDING_METHOD_FRAME_SIZE;
            config.testDecConfig.bitstreamMode = BS_MODE_PIC_END;
        }
        else
        {
            VLOG(ERR,"bs-format can't be supported\n");
            return 1;
        }
    }    
#endif

    if (RETCODE_SUCCESS != (ret=SetUpDecoderOpenParam(&(config.decOpenParam), &(config.testDecConfig)))) {
        VLOG(ERR, "%s:%d SetUpDecoderOpenParam failed Error code is 0x%x \n", __FUNCTION__, __LINE__, ret);
        return 1;
    }

    CNMAppInit();

    task     = CNMTaskCreate();
    feeder   = ComponentCreate("feeder",   &config);
    decoder  = ComponentCreate("wave_decoder",  &config);
    renderer = ComponentCreate("renderer", &config);

    CNMTaskAdd(task, feeder);
    CNMTaskAdd(task, decoder);
    CNMTaskAdd(task, renderer);
    CNMAppAdd(task);

    if ((ret=SetupDecListenerContext(&lsnCtx, &config, renderer)) == TRUE) {
        ComponentRegisterListener(decoder, COMPONENT_EVENT_DEC_ALL, DecoderListener, (void*)&lsnCtx);
        if(s_abort==FALSE){
           s_brun = TRUE;
           ret  = CNMAppRun();
        }
    }
    else {
        CNMAppStop();
    }

    osal_free(pusBitCode);
    ClearDecListenerContext(&lsnCtx);

    testResult = (ret == TRUE && lsnCtx.match == TRUE && CNMErrorGet() == 0);
    VLOG(INFO, "[RESULT] %s\n", (testResult == TRUE)  ? "SUCCESS" : "FAILURE");

	//vic_pagemap_release();
    if ( CNMErrorGet() != 0 )
        return CNMErrorGet();

    return testResult == TRUE ? 0 : 1;
}

