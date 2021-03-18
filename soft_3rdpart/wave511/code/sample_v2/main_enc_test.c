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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include "cnm_app.h"
#include "encoder_listener.h"
#include "bw_monitor.h"

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
    VLOG(INFO, "-n [num]                    output frame number, -1,0:unlimited encoding(depends on YUV size)\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-c                          compare with golden bitstream\n");

    for (i = 0;i < MAX_GETOPT_OPTIONS;i++) {
        if (opt[i].name == NULL)
            break;
        VLOG(INFO, "%s", opt[i].help);
    }
}

static BOOL CheckTestConfig(TestEncConfig *testConfig)
{
    if ( (testConfig->compareType & (1<<MODE_SAVE_ENCODED)) && testConfig->bitstreamFileName[0] == 0) {
        testConfig->compareType &= ~(1<<MODE_SAVE_ENCODED);
        VLOG(ERR,"You want to Save bitstream data. Set the path\n");
        return FALSE;
    }

    if ( (testConfig->compareType & (1<<MODE_COMP_ENCODED)) && testConfig->ref_stream_path[0] == 0) {
        testConfig->compareType &= ~(1<<MODE_COMP_ENCODED);
        VLOG(ERR,"You want to Compare bitstream data. Set the path\n");
        return FALSE;
    }

    return TRUE;
}

static BOOL ParseArgumentAndSetTestConfig(CommandLineArgument argument, TestEncConfig* testConfig) {
    Int32  opt  = 0, i = 0, idx = 0;
    int    argc = argument.argc;
    char** argv = argument.argv;
    char* optString = "rbhvn:";
    struct option options[MAX_GETOPT_OPTIONS];
    struct OptionExt options_help[MAX_GETOPT_OPTIONS] = {
        {"output",                1, NULL, 0, "--output                    An output bitstream file path\n"},
        {"input",                 1, NULL, 0, "--input                     YUV file path. The value of InputFile in a cfg is replaced to this value.\n"},
        {"codec",                 1, NULL, 0, "--codec                     codec index, 12:HEVC, 0:AVC\n"},
        {"cfgFileName",           1, NULL, 0, "--cfgFileName               cfg file path\n"},
#ifdef SUPPORT_LOOK_AHEAD_RC
        {"cfgFileName_larc_pass1",1, NULL, 0, "--cfgFileName_larc_pass1    look-ahead rc pass1 cfg file path\n"},
        {"input_larc_pass1",      1, NULL, 0, "--input_larc_pass1          look-ahead rc pass1 YUV file path. The value of InputFile in a cfg is replaced to this value.\n"},
#endif
        {"coreIdx",               1, NULL, 0, "--coreIdx                   core index: default 0\n"},
        {"picWidth",              1, NULL, 0, "--picWidth                  source width\n"},
        {"picHeight",             1, NULL, 0, "--picHeight                 source height\n"},
        {"kbps",                  1, NULL, 0, "--kbps                      RC bitrate in kbps. In case of without cfg file, if this option has value then RC will be enabled\n"},
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
        {"srcReleaseInt",         1, NULL, 0, "--srcReleaseInt             1 : enable source release Interrupt\n"},
#endif
        {"lineBufInt",            1, NULL, 0, "--lineBufInt                1 : enable linebuffer interrupt\n"},
        {"lowLatencyMode",        1, NULL, 0, "--lowLatencyMode            bit[1]: low latency interrupt enable, bit[0]: fast bitstream-packing enable\n"},
        {"loop-count",            1, NULL, 0, "--loop-count                integer number. loop test, default 0\n"},
        {"enable-cbcrInterleave", 0, NULL, 0, "--enable-cbcrInterleave     enable cbcr interleave\n"},
        {"nv21",                  1, NULL, 0, "--nv21                      enable NV21(must set enable-cbcrInterleave)\n"},
        {"packedFormat",          1, NULL, 0, "--packedFormat              1:YUYV, 2:YVYU, 3:UYVY, 4:VYUY\n"},
        {"rotAngle",              1, NULL, 0, "--rotAngle                  rotation angle(0,90,180,270), Not supported on WAVE420L, WAVE525, WAVE521C_DUAL\n"},
        {"mirDir",                1, NULL, 0, "--mirDir                    1:Vertical, 2: Horizontal, 3:Vert-Horz, Not supported on WAVE420L, WAVE525, WAVE521C_DUAL\n"},
        {"secondary-axi",         1, NULL, 0, "--secondary-axi             0~3: bit mask values, Please refer programmer's guide or datasheet\n"
                                              "                            1: RDO, 2: LF\n"},
        {"frame-endian",          1, NULL, 0, "--frame-endian              16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"stream-endian",         1, NULL, 0, "--stream-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"source-endian",         1, NULL, 0, "--source-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"ref_stream_path",       1, NULL, 0, "--ref_stream_path           golden data which is compared with encoded stream when -c option\n"},
        {"srcFormat",             1, NULL, 0, "--srcFormat                 0:8bit, 1:10bit 1P2B_MSB, 2:10bit 1P2B_LSB, 3:10bit 3P4B_MSB, 4:10bit 3P4B_LSB\n"
                                              "                            1P2B : 10bit 1pixel 2bytes source format\n"
                                              "                            3P4B : 10bit 3pixel 4bytes srouce format\n"},
        {NULL,                    0, NULL, 0},
    };


    for (i = 0; i < MAX_GETOPT_OPTIONS;i++) {
        if (options_help[i].name == NULL)
            break;
        osal_memcpy(&options[i], &options_help[i], sizeof(struct option));
    }

    while ((opt=getopt_long(argc, argv, optString, options, (int *)&idx)) != -1) {
        switch (opt) {
        case 'n':
            testConfig->outNum = atoi(optarg);
            break;
        case 'c':
            testConfig->compareType |= (1 << MODE_COMP_ENCODED);
            VLOG(TRACE, "Stream compare Enable\n");
            break;
        case 'h':
            Help(options_help, argv[0]);
            exit(0);
        case 0:
            if (!strcmp(options[idx].name, "output")) {
                osal_memcpy(testConfig->bitstreamFileName, optarg, strlen(optarg));
                ChangePathStyle(testConfig->bitstreamFileName);
            } else if (!strcmp(options[idx].name, "input")) {
                strcpy(testConfig->optYuvPath, optarg);
                ChangePathStyle(testConfig->optYuvPath);
            } else if (!strcmp(options[idx].name, "codec")) {
                testConfig->stdMode = (CodStd)atoi(optarg);
            } else if (!strcmp(options[idx].name, "cfgFileName")) {
                osal_memcpy(testConfig->cfgFileName, optarg, strlen(optarg));
#ifdef SUPPORT_LOOK_AHEAD_RC
            } else if (!strcmp(options[idx].name, "input_larc_pass1")) {
                strcpy(testConfig->optYuvPath_larc_pass1, optarg);
                ChangePathStyle(testConfig->optYuvPath_larc_pass1);
            } else if (!strcmp(options[idx].name, "cfgFileName_larc_pass1")) {
                osal_memcpy(testConfig->cfgFileName_larc_pass1, optarg, strlen(optarg));
#endif
            } else if (!strcmp(options[idx].name, "coreIdx")) {
                testConfig->coreIdx = atoi(optarg);
            } else if (!strcmp(options[idx].name, "picWidth")) {
                testConfig->picWidth = atoi(optarg);
            } else if (!strcmp(options[idx].name, "picHeight")) {
                testConfig->picHeight = atoi(optarg);
            } else if (!strcmp(options[idx].name, "kbps")) {
                testConfig->kbps = atoi(optarg);
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
            } else if (!strcmp(options[idx].name, "srcReleaseInt")) {
                testConfig->srcReleaseIntEnable = atoi(optarg);
#endif
            } else if (!strcmp(options[idx].name, "lineBufInt")) {
                testConfig->lineBufIntEn = atoi(optarg);
            } else if (!strcmp(options[idx].name, "lowLatencyMode")) {
                testConfig->lowLatencyMode = atoi(optarg);
            } else if (!strcmp(options[idx].name, "loop-count")) {
                testConfig->loopCount = atoi(optarg);
            } else if (!strcmp(options[idx].name, "enable-cbcrInterleave")) {
                testConfig->cbcrInterleave = 1;
            } else if (!strcmp(options[idx].name, "nv21")) {
                testConfig->nv21 = atoi(optarg);
            } else if (!strcmp(options[idx].name, "packedFormat")) {
                testConfig->packedFormat = atoi(optarg);
            } else if (!strcmp(options[idx].name, "rotAngle")) {
                testConfig->rotAngle = atoi(optarg);
            } else if (!strcmp(options[idx].name, "mirDir")) {
                testConfig->mirDir = atoi(optarg);
            } else if (!strcmp(options[idx].name, "secondary-axi")) {
                testConfig->secondaryAXI = atoi(optarg);
            } else if (!strcmp(options[idx].name, "frame-endian")) {
                testConfig->frame_endian = atoi(optarg);
            } else if (!strcmp(options[idx].name, "stream-endian")) {
                testConfig->stream_endian = (EndianMode)atoi(optarg);
            } else if (!strcmp(options[idx].name, "source-endian")) {
                testConfig->source_endian = atoi(optarg);
            } else if (!strcmp(options[idx].name, "ref_stream_path")) {
                osal_memcpy(testConfig->ref_stream_path, optarg, strlen(optarg));
                ChangePathStyle(testConfig->ref_stream_path);
            } else if (!strcmp(options[idx].name, "srcFormat")) {
                Uint32 temp;
                temp = atoi(optarg);
                testConfig->srcFormat = FORMAT_420;
                if (temp == 1) {
                    testConfig->srcFormat = FORMAT_420_P10_16BIT_MSB;
                }
                else if (temp == 2) {
                    testConfig->srcFormat = FORMAT_420_P10_16BIT_LSB;
                }
                else if (temp == 3) {
                    testConfig->srcFormat = FORMAT_420_P10_32BIT_MSB;
                }
                else if (temp == 4) {
                    testConfig->srcFormat = FORMAT_420_P10_32BIT_LSB;
                }
            } else {
                VLOG(ERR, "not exist param = %s\n", options[idx].name);
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

    return TRUE;
}


int main(int argc, char **argv)
{
    char*               fwPath     = NULL;
    TestEncConfig       testConfig;
    CommandLineArgument argument;
    CNMComponentConfig  config;
    CNMTask             task;
    Component           yuvFeeder;
    Component           encoder;
    Component           reader;
#ifdef SUPPORT_LOOK_AHEAD_RC
    CNMComponentConfig  config_pass1;
    Component           yuvFeeder_pass1;
    Component           encoder_pass1;
    Component           reader_pass1;
    EncListenerContext  lsnCtxPass1;
#endif
    Uint32              sizeInWord;
    Uint16*             pusBitCode;
    BOOL                ret;
    BOOL                testResult;
    EncListenerContext  lsnCtx;

    osal_memset(&argument, 0x00, sizeof(CommandLineArgument));
    osal_memset(&config,   0x00, sizeof(CNMComponentConfig));

    InitLog();
    SetDefaultEncTestConfig(&testConfig);
    argument.argc = argc;
    argument.argv = argv;
    if (ParseArgumentAndSetTestConfig(argument, &testConfig) == FALSE) {
        VLOG(ERR, "fail to ParseArgumentAndSetTestConfig()\n");
        return 1;
    }

    testConfig.productId = (ProductId)VPU_GetProductId(testConfig.coreIdx);

    if (CheckTestConfig(&testConfig) == FALSE) {
        VLOG(ERR, "fail to CheckTestConfig()\n");
        return 1;
    }

    switch (testConfig.productId) {
    case PRODUCT_ID_521:    fwPath = CORE_6_BIT_CODE_FILE_PATH; break;
    default:
        VLOG(ERR, "Unknown product id: %d\n", testConfig.productId);
        return 1;
    }

    VLOG(INFO, "FW PATH = %s\n", fwPath);

    if (LoadFirmware(testConfig.productId, (Uint8**)&pusBitCode, &sizeInWord, fwPath) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, fwPath);
        return 1;
    }


    config.testEncConfig = testConfig;
    config.bitcode       = (Uint8*)pusBitCode;
    config.sizeOfBitcode = sizeInWord;

    if (SetupEncoderOpenParam(&config.encOpenParam, &config.testEncConfig, NULL) == FALSE) {
        VLOG(ERR, "SetupEncoderOpenParam error\n");
        return 1;
    }

#ifdef SUPPORT_LOOK_AHEAD_RC
    if (config.encOpenParam.EncStdParam.waveParam.larcEnable == TRUE) {

        osal_memcpy(&config_pass1, &config, sizeof(config_pass1));
        osal_memcpy(&config_pass1.testEncConfig.cfgFileName, &config_pass1.testEncConfig.cfgFileName_larc_pass1, sizeof(config_pass1.testEncConfig.cfgFileName));
        config_pass1.testEncConfig.larcPass = LOOK_AHEAD_RC_PASS1;
        SetupEncoderOpenParam(&config_pass1.encOpenParam, &config_pass1.testEncConfig, NULL);

        config_pass1.testEncConfig.compareType = 0; //we don't use the look-ahead-rc pass 1 data
        config_pass1.testEncConfig.bitstreamFileName[0]=0;

        config.testEncConfig.larcPass       = LOOK_AHEAD_RC_PASS2;
    }
#endif

    CNMAppInit();

    task       = CNMTaskCreate();
#ifdef SUPPORT_LOOK_AHEAD_RC
    if (config.encOpenParam.EncStdParam.waveParam.larcEnable == TRUE) {
        yuvFeeder_pass1  = ComponentCreate("yuvfeeder", &config_pass1);
        encoder_pass1    = ComponentCreate("wave_encoder", &config_pass1);
        reader_pass1     = ComponentCreate("reader", &config_pass1);

        CNMTaskAdd(task, yuvFeeder_pass1);
        CNMTaskAdd(task, encoder_pass1);
        CNMTaskAdd(task, reader_pass1);
    }
#endif

    yuvFeeder  = ComponentCreate("yuvfeeder",       &config);
    encoder    = ComponentCreate("wave_encoder",    &config);
    reader     = ComponentCreate("reader",          &config);

    CNMTaskAdd(task, yuvFeeder);
    CNMTaskAdd(task, encoder);
    CNMTaskAdd(task, reader);

    CNMAppAdd(task);

    if ((ret=SetupEncListenerContext(&lsnCtx, &config)) == TRUE) {
        ComponentRegisterListener(encoder, COMPONENT_EVENT_ENC_ALL, EncoderListener, (void*)&lsnCtx);
#ifdef SUPPORT_LOOK_AHEAD_RC
        if (config.encOpenParam.EncStdParam.waveParam.larcEnable == TRUE) {
            if ((ret=SetupEncListenerContext(&lsnCtxPass1, &config_pass1)) == TRUE) {
                ComponentRegisterListener(encoder_pass1, COMPONENT_EVENT_ENC_ALL, EncoderListener, (void*)&lsnCtxPass1);
            }
        }
#endif
        ret = CNMAppRun();
    }
    else {
        CNMAppStop();
    }

    osal_free(pusBitCode);
    ClearEncListenerContext(&lsnCtx);
#ifdef SUPPORT_LOOK_AHEAD_RC
    if (config.testEncConfig.larcEnable == TRUE) {
        ClearEncListenerContext(&lsnCtxPass1);
    }
#endif

    testResult = (ret == TRUE && lsnCtx.match == TRUE && lsnCtx.matchOtherInfo == TRUE);

#ifdef SUPPORT_LOOK_AHEAD_RC
    if (config.testEncConfig.larcEnable == TRUE) {
        testResult = (ret == TRUE && lsnCtxPass1.match == TRUE && lsnCtxPass1.matchOtherInfo == TRUE);
    }
#endif

    if (testResult == TRUE) VLOG(INFO, "[RESULT] SUCCESS\n");
    else                    VLOG(ERR,  "[RESULT] FAILURE\n");

    if ( CNMErrorGet() != 0 )
        return CNMErrorGet();

    return (testResult == TRUE) ? 0 : 1;
}

