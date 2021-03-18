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
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include "main_helper.h"
#include "cnm_app.h"
#include "decoder_listener.h"
#include "encoder_listener.h"

enum waitStatus {
    WAIT_AFTER_INIT         = 0,

    WAIT_AFTER_DEC_OPEN     = 1,
    WAIT_AFTER_SEQ_INIT     = 2,
    WAIT_AFTER_REG_BUF      = 3,
    WAIT_BEFORE_DEC_START   = 4,
    WAIT_BEFORE_DEC_CLOSE   = 5,

    WAIT_AFTER_ENC_OPEN     = 6,
    WAIT_AFTER_INIT_SEQ     = 7,
    WAIT_AFTER_REG_FRAME    = 8,
    WAIT_BEFORE_ENC_START   = 9,
    WAIT_BEFORE_ENC_CLOSE   = 10,
    WAIT_NOT_DEFINED        = 11,
    WAIT_STATUS_MAX
};

enum get_result_status{
    GET_RESULT_INIT,
    GET_RESULT_BEFORE_WAIT_INT, //before calling wait interrupt.
    GET_RESULT_AFTER_WAIT_INT   //got interrupt.
};

static Uint32           sizeInWord;
static Uint16*          pusBitCode;


typedef struct {
    char             inputFilePath[256];
    char             localFile[256];
    char             outputFilePath[256];
    char             refFilePath[256];
    CodStd           stdMode;
    BOOL             afbce;
    BOOL             afbcd;
    BOOL             scaler;
    size_t           sclw;
    size_t           sclh;
    Int32            bsmode;
    BOOL             enableWTL;
    BOOL             enableMVC;
    int              compareType;
    TestDecConfig    decConfig;
    TestEncConfig    encConfig;
    BOOL             isEncoder;
} InstTestConfig;

typedef struct {
    Uint32          totProcNum;             /* the number of process */
    Uint32          curProcNum;
    Uint32          numMulti;
    Uint32          cores;
    BOOL            performance;            /* for performance measurement */
    Uint32          pfClock;                /* for performance measurement */
    Uint32          fps;                    /* for performance measurement */
    Uint32          numFrames;
    InstTestConfig  instConfig[MAX_NUM_INSTANCE];
} TestMultiConfig;


static void WaitForNextStep(Int32 instIdx, Int32 curStepName)
{
}

static void Help(const char *programName)
{
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", GetBasename(programName), API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "\tSample program controlling the Chips&Media VPU\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input <stream list file, aka cmd file>\n", GetBasename(programName));
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-c                          enable comparison mode\n");
    VLOG(INFO, "                            1 : compare with golden stream that specified --ref_file_path option\n");
    VLOG(INFO, "-e                          0 : decoder, 1: encoder\n");
    VLOG(INFO, "-n                          number of frames\n");
    VLOG(INFO, "--codec                     The index of codec\n");
    VLOG(INFO, "                            AVC = 0, VC1 = 1, MPEG2 = 2, MPEG4 = 3, H263 = 4\n");
    VLOG(INFO, "                            DIV3 = 5, RV = 6, AVS = 7, THO = 9, VP3 = 10, VP8 = 11, HEVC = 12, VP9 = 13\n");
    VLOG(INFO, "                            AVC_ENC = 14, MP4_ENC = 17, H263_ENC = 18, HEVC_ENC = 26\n");
    VLOG(INFO, "--input                     bitstream(decoder) or cfg(encoder) path\n");
    VLOG(INFO, "--output                    yuv(decoder) or bitstream(encoder) path\n");
    VLOG(INFO, "--ref_file_path             Golden md5 or stream path\n");
    VLOG(INFO, "--pf                        0: disable peformance report(default), 1: enable performance report\n");
    VLOG(INFO, "--pf-clock                  peformance clock in Hz(It must be BCLK for WAVE5 series).\n");
    VLOG(INFO, "--scaler                    enable scaler option. default 0\n");
    VLOG(INFO, "--sclw                      set scale width value.\n");
    VLOG(INFO, "--sclh                      set scale height value.\n");
    VLOG(INFO, "--bsmode                    set bitstream mode.\n");
    VLOG(INFO, "--enable-wtl                enable wtl option. default 0\n");
}

static void SetEncMultiParam(TestMultiConfig* multiConfig, Uint32 idx, Int32 productId)
{
    InstTestConfig *instConfig = &multiConfig->instConfig[idx];
    TestEncConfig *encCfg = &instConfig->encConfig;

    encCfg->stdMode       = multiConfig->instConfig[idx].stdMode;
    encCfg->frame_endian  = 0;
    encCfg->stream_endian = 0;
    encCfg->yuv_mode      = YUV_MODE_YUV_LOADER;
    encCfg->mapType       = LINEAR_FRAME_MAP;
    if (TRUE == PRODUCT_ID_W_SERIES(productId)) {
        encCfg->mapType   = COMPRESSED_FRAME_MAP;
    }

    strcpy(encCfg->cfgFileName, multiConfig->instConfig[idx].inputFilePath);
    strcpy(encCfg->bitstreamFileName, multiConfig->instConfig[idx].outputFilePath);
    if (TRUE == PRODUCT_ID_W_SERIES(productId)) {
        if (multiConfig->instConfig[idx].compareType == MD5_COMPARE) {
            encCfg->compareType |= (1 << MODE_COMP_RECON);
            strcpy(encCfg->ref_recon_md5_path, multiConfig->instConfig[idx].refFilePath);
        }
        else if (multiConfig->instConfig[idx].compareType == STREAM_COMPARE) {
            encCfg->compareType |= (1 << MODE_COMP_ENCODED);
            strcpy(encCfg->ref_stream_path, multiConfig->instConfig[idx].refFilePath);
        }
    }
    else { // coda9 encoder case
        char imageRootDir[256]="./yuv";
        encCfg->compareType = (TRUE == (BOOL)multiConfig->instConfig[idx].compareType) ? STREAM_COMPARE : NO_COMPARE;
        strcpy(encCfg->ref_stream_path, multiConfig->instConfig[idx].refFilePath);
        strcpy(encCfg->yuvSourceBaseDir, imageRootDir);
        multiConfig->instConfig[idx].encConfig.lineBufIntEn = TRUE;
    }
    multiConfig->instConfig[idx].encConfig.performance = multiConfig->performance;
    multiConfig->instConfig[idx].encConfig.pfClock     = multiConfig->pfClock;
    multiConfig->instConfig[idx].encConfig.fps         = multiConfig->fps;
    multiConfig->instConfig[idx].encConfig.outNum      = multiConfig->numFrames;
    encCfg->srcFormat = FORMAT_420_P10_16BIT_LSB;

    multiConfig->instConfig[idx].encConfig.productId = productId;
}

static void SetDecMultiParam(TestMultiConfig* multiConfig, Uint32 idx, Int32 productId)
{
    multiConfig->instConfig[idx].decConfig.bitFormat            = multiConfig->instConfig[idx].stdMode;
    multiConfig->instConfig[idx].decConfig.bitstreamMode        = multiConfig->instConfig[idx].bsmode;
    multiConfig->instConfig[idx].decConfig.feedingMode          = (multiConfig->instConfig[idx].bsmode == BS_MODE_INTERRUPT) ? FEEDING_METHOD_FIXED_SIZE : FEEDING_METHOD_FRAME_SIZE;
    multiConfig->instConfig[idx].decConfig.streamEndian         = VPU_STREAM_ENDIAN;
    multiConfig->instConfig[idx].decConfig.frameEndian          = VPU_FRAME_ENDIAN;
    multiConfig->instConfig[idx].decConfig.cbcrInterleave       = FALSE;
    multiConfig->instConfig[idx].decConfig.nv21                 = FALSE;
    multiConfig->instConfig[idx].decConfig.enableWTL            = multiConfig->instConfig[idx].enableWTL;
    multiConfig->instConfig[idx].decConfig.coda9.enableMvc      = multiConfig->instConfig[idx].enableMVC;
    multiConfig->instConfig[idx].decConfig.wtlMode              = FF_FRAME;
    multiConfig->instConfig[idx].decConfig.wtlFormat            = FORMAT_420;

    if (TRUE == PRODUCT_ID_W_SERIES(productId)) {
        multiConfig->instConfig[idx].decConfig.mapType          = COMPRESSED_FRAME_MAP;
        multiConfig->instConfig[idx].decConfig.wave.bwOptimization  = TRUE;
    }
    else
    {
        multiConfig->instConfig[idx].decConfig.mapType          = LINEAR_FRAME_MAP;
    }
    multiConfig->instConfig[idx].decConfig.bsSize               = (5*1024*1024);
    multiConfig->instConfig[idx].decConfig.productId            = productId;

    if (multiConfig->instConfig[idx].scaler == TRUE) {
        multiConfig->instConfig[idx].decConfig.scaleDownWidth   = multiConfig->instConfig[idx].sclw;
        multiConfig->instConfig[idx].decConfig.scaleDownHeight  = multiConfig->instConfig[idx].sclh;
    }

    strcpy(multiConfig->instConfig[idx].decConfig.inputPath, multiConfig->instConfig[idx].inputFilePath);

    if (multiConfig->instConfig[idx].localFile[0]) {
        strcpy(multiConfig->instConfig[idx].decConfig.inputPath, multiConfig->instConfig[idx].localFile);
    }
    strcpy(multiConfig->instConfig[idx].decConfig.outputPath, multiConfig->instConfig[idx].outputFilePath);

    if (multiConfig->instConfig[idx].compareType == MD5_COMPARE) {
        multiConfig->instConfig[idx].decConfig.compareType = MD5_COMPARE;
        strcpy(multiConfig->instConfig[idx].decConfig.md5Path, multiConfig->instConfig[idx].refFilePath);
    }
    else if (multiConfig->instConfig[idx].compareType == YUV_COMPARE) {
        multiConfig->instConfig[idx].decConfig.compareType = YUV_COMPARE;
        strcpy(multiConfig->instConfig[idx].decConfig.refYuvPath, multiConfig->instConfig[idx].refFilePath);
    }
    else {
        multiConfig->instConfig[idx].decConfig.compareType = NO_COMPARE;
    }

    multiConfig->instConfig[idx].decConfig.performance = multiConfig->performance;
    multiConfig->instConfig[idx].decConfig.pfClock     = multiConfig->pfClock;
    multiConfig->instConfig[idx].decConfig.fps         = multiConfig->fps;
    multiConfig->instConfig[idx].decConfig.forceOutNum = multiConfig->numFrames;
}

static void MultiDecoderListener(Component com, Uint64 event, void* data, void* context)
{
    DecHandle               decHandle = NULL;

    ComponentGetParameter(NULL, com, GET_PARAM_DEC_HANDLE, &decHandle);
    if (decHandle == NULL && event != COMPONENT_EVENT_DEC_DECODED_ALL) {
        // Terminated state
        return;
    }

    switch (event) {
    case COMPONENT_EVENT_DEC_OPEN:
        WaitForNextStep(decHandle->instIndex, WAIT_AFTER_DEC_OPEN);
        break;
    case COMPONENT_EVENT_DEC_COMPLETE_SEQ:
        WaitForNextStep(decHandle->instIndex, WAIT_AFTER_SEQ_INIT);
        HandleDecCompleteSeqEvent(com, (CNMComListenerDecCompleteSeq*)data, (DecListenerContext*)context);
        break;
    case COMPONENT_EVENT_DEC_REGISTER_FB:
        WaitForNextStep(decHandle->instIndex, WAIT_AFTER_REG_BUF);
        HandleDecRegisterFbEvent(com, (CNMComListenerDecRegisterFb*)data, (DecListenerContext*)context);
        break;
    case COMPONENT_EVENT_DEC_READY_ONE_FRAME:
        WaitForNextStep(decHandle->instIndex, WAIT_BEFORE_DEC_START);
        break;
    case COMPONENT_EVENT_DEC_GET_OUTPUT_INFO:
        HandleDecGetOutputEvent(com, (CNMComListenerDecDone*)data, (DecListenerContext*)context);
        break;
    case COMPONENT_EVENT_DEC_DECODED_ALL:
        // It isn't possible to get handle when a component is terminated state.
        decHandle = (DecHandle)data;
        if (decHandle) WaitForNextStep(decHandle->instIndex, WAIT_BEFORE_DEC_CLOSE);
        break;
    default:
        break;
    }
}

static BOOL CreateDecoderTask(CNMTask task, CNMComponentConfig* config, DecListenerContext* lsnCtx)
{
    BOOL        success = FALSE;
    RetCode         ret = RETCODE_SUCCESS;

    if (RETCODE_SUCCESS != (ret=SetUpDecoderOpenParam(&(config->decOpenParam), &(config->testDecConfig)))) {
        VLOG(ERR, "%s:%d SetUpDecoderOpenParam failed Error code is 0x%x \n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    Component feeder   = ComponentCreate("feeder", config);
    Component renderer = ComponentCreate("renderer", config);
    Component decoder  = ComponentCreate("wave_decoder",  config);

    CNMTaskAdd(task, feeder);
    CNMTaskAdd(task, decoder);
    CNMTaskAdd(task, renderer);

    if ((success=SetupDecListenerContext(lsnCtx, config, renderer)) == TRUE) {
        ComponentRegisterListener(decoder, COMPONENT_EVENT_DEC_ALL, MultiDecoderListener, (void*)lsnCtx);
    }

    return success;
}

static void MultiEncoderListener(Component com, Uint64 event, void* data, void* context)
{
    EncHandle                       encHandle   = NULL;
    CNMComListenerEncStartOneFrame* lsnStartEnc = NULL;
    ProductId                       productId;

    UNREFERENCED_PARAMETER(lsnStartEnc);

    ComponentGetParameter(NULL, com, GET_PARAM_ENC_HANDLE, &encHandle);
    if (encHandle == NULL && event != COMPONENT_EVENT_ENC_ENCODED_ALL) {
        // Terminated state
        return;
    }

    switch (event) {
    case COMPONENT_EVENT_ENC_OPEN:
        WaitForNextStep(encHandle->instIndex, WAIT_AFTER_ENC_OPEN);
        break;
    case COMPONENT_EVENT_ENC_COMPLETE_SEQ:
        WaitForNextStep(encHandle->instIndex, WAIT_AFTER_INIT_SEQ);
        HandleEncCompleteSeqEvent(com, (CNMComListenerEncCompleteSeq*)data, (EncListenerContext*)context);
        break;
    case COMPONENT_EVENT_ENC_REGISTER_FB:
        WaitForNextStep(encHandle->instIndex, WAIT_AFTER_REG_FRAME);
        break;
    case COMPONENT_EVENT_ENC_READY_ONE_FRAME:
        WaitForNextStep(encHandle->instIndex, WAIT_BEFORE_ENC_START);
        break;
    case COMPONENT_EVENT_ENC_START_ONE_FRAME:
        break;
    case COMPONENT_EVENT_ENC_GET_OUTPUT_INFO:
        encHandle = ((CNMComListenerEncDone*)data)->handle;
        productId = VPU_GetProductId(VPU_HANDLE_CORE_INDEX(encHandle));
        if (TRUE == PRODUCT_ID_W_SERIES(productId)) {
            HandleEncGetOutputEvent(com, (CNMComListenerEncDone*)data, (EncListenerContext*)context);
        }
        else {
            Coda9HandleEncGetOutputEvent(com, (CNMComListenerEncDone*)data, (EncListenerContext*)context);
        }
        break;
    case COMPONENT_EVENT_CODA9_ENC_MAKE_HEADER:
        Coda9HandleEncMakeHeaderEvent(com, (CNMComListenerEncMakeHeader*)data, (EncListenerContext*)context);
        break;
    case COMPONENT_EVENT_ENC_ENCODED_ALL:
        // It isn't possible to get handle when a component is terminated state.
        encHandle = (EncHandle)data;
        if (encHandle)
            WaitForNextStep(encHandle->instIndex, WAIT_BEFORE_ENC_CLOSE);
        break;
    case COMPONENT_EVENT_ENC_FULL_INTERRUPT:
        HandleEncFullEvent(com, (CNMComListenerEncFull*)data, (EncListenerContext*)context);
        break;
    case COMPONENT_EVENT_ENC_CLOSE:
        HandleEncGetEncCloseEvent(com, (CNMComListenerEncClose*)data, (EncListenerContext*)context);
        break;
    default:
        break;
    }
}

static BOOL CreateEncoderTask(CNMTask task, CNMComponentConfig* config, EncListenerContext* lsnCtx)
{
    Component feeder;
    Component encoder;
    Component reader;
    BOOL      success = FALSE;
#ifdef SUPPORT_LOOK_AHEAD_RC
    CNMComponentConfig  config_pass1;
    Component           yuvFeeder_pass1;
    Component           encoder_pass1;
    Component           reader_pass1;
    EncListenerContext  lsnCtxPass1;
#endif

    if (SetupEncoderOpenParam(&config->encOpenParam, &config->testEncConfig, NULL) == FALSE) {
        return FALSE;
    }
#ifdef SUPPORT_LOOK_AHEAD_RC
    if (config->encOpenParam.EncStdParam.waveParam.larcEnable == TRUE) {

        osal_memcpy(&config_pass1, config, sizeof(*config));
        replaceAll(config_pass1.testEncConfig.cfgFileName, config->testEncConfig.cfgFileName, "pass2", "pass1");
        config_pass1.testEncConfig.larcPass = LOOK_AHEAD_RC_PASS1;
        SetupEncoderOpenParam(&config_pass1.encOpenParam, &config_pass1.testEncConfig, NULL);

        config_pass1.testEncConfig.compareType = 0; //we don't use the look-ahead-rc pass 1 data
        config_pass1.testEncConfig.bitstreamFileName[0]=0;

        config->testEncConfig.larcPass       = LOOK_AHEAD_RC_PASS2;
    }

    if (config->encOpenParam.EncStdParam.waveParam.larcEnable == TRUE) {
        yuvFeeder_pass1  = ComponentCreate("yuvfeeder", &config_pass1);
        encoder_pass1    = ComponentCreate("wave_encoder", &config_pass1);
        reader_pass1     = ComponentCreate("reader", &config_pass1);

        CNMTaskAdd(task, yuvFeeder_pass1);
        CNMTaskAdd(task, encoder_pass1);
        CNMTaskAdd(task, reader_pass1);
    }
#endif

    feeder  = ComponentCreate("yuvfeeder",      config);
    encoder = ComponentCreate("wave_encoder",   config);
    reader  = ComponentCreate("reader",         config);

    CNMTaskAdd(task, feeder);
    CNMTaskAdd(task, encoder);
    CNMTaskAdd(task, reader);

    if ((success=SetupEncListenerContext(lsnCtx, config)) == TRUE) {
        ComponentRegisterListener(encoder, COMPONENT_EVENT_ENC_ALL, MultiEncoderListener, (void*)lsnCtx);
#ifdef SUPPORT_LOOK_AHEAD_RC
        if (config->encOpenParam.EncStdParam.waveParam.larcEnable == TRUE) {
            if ((SetupEncListenerContext(&lsnCtxPass1, &config_pass1)) == TRUE) {
                ComponentRegisterListener(encoder_pass1, COMPONENT_EVENT_ENC_ALL, EncoderListener, (void*)&lsnCtxPass1);
            }
        }
#endif
    }

    return success;
}

static BOOL MultiInstanceTest(TestMultiConfig* multiConfig, Uint16* fw, Uint32 size)
{
    Uint32              i;
    CNMComponentConfig  config;
    CNMTask             task;
    DecListenerContext* decListenerCtx = (DecListenerContext*)osal_malloc(sizeof(DecListenerContext) * MAX_NUM_INSTANCE);
    EncListenerContext* encListenerCtx = (EncListenerContext*)osal_malloc(sizeof(EncListenerContext) * MAX_NUM_INSTANCE);
    BOOL                ret     = FALSE;
    BOOL                success = TRUE;
    BOOL                match   = TRUE;

    CNMAppInit();

    for (i=0; i < multiConfig->numMulti; i++) {
        task = CNMTaskCreate();
        memset((void*)&config, 0x00, sizeof(CNMComponentConfig));
        config.bitcode       = (Uint8*)fw;
        config.sizeOfBitcode = size;
        if (multiConfig->instConfig[i].isEncoder == TRUE) {
            memcpy((void*)&config.testEncConfig, &multiConfig->instConfig[i].encConfig, sizeof(TestEncConfig));
            success = CreateEncoderTask(task, &config, &encListenerCtx[i]);
        }
        else
        {
            memcpy((void*)&config.testDecConfig, &multiConfig->instConfig[i].decConfig, sizeof(TestDecConfig));
            success = CreateDecoderTask(task, &config, &decListenerCtx[i]);
        }
        CNMAppAdd(task);
        if (success == FALSE) {
            CNMAppStop();
            return FALSE;
        }
    }

    ret = CNMAppRun();

    for (i=0; i<multiConfig->numMulti; i++) {
        if (multiConfig->instConfig[i].isEncoder == TRUE) {
            match &= (encListenerCtx[i].match == TRUE && encListenerCtx[i].matchOtherInfo == TRUE);
            ClearEncListenerContext(&encListenerCtx[i]);
        }
        else
        {
            match &= decListenerCtx[i].match;
            ClearDecListenerContext(&decListenerCtx[i]);
        }
    }
    if (ret == TRUE) ret = match;

    osal_free(decListenerCtx);
    osal_free(encListenerCtx);

    return ret;
}

int main(int argc, char **argv)
{
    Int32           coreIndex   = 0;
    Uint32          productId   = 0;
    TestMultiConfig multiConfig;
    int             opt, index, i;
    char*           tempArg;
    char*           optString   = "fc:e:hn:";
    struct option   options[]   = {
        {"instance-num",          1, NULL, 0},
        {"codec",                 1, NULL, 0},
        {"input",                 1, NULL, 0},
        {"output",                1, NULL, 0},
        {"ref_file_path",         1, NULL, 0},
        {"scaler",                1, NULL, 0},
        {"sclw",                  1, NULL, 0},
        {"sclh",                  1, NULL, 0},
        {"bsmode",                1, NULL, 0},
        {"enable-wtl",            1, NULL, 0},
        {"pf",                    1, NULL, 0},
        {"pf-clock",              1, NULL, 0},
        {"fps",                   1, NULL, 0},
        {"cores",                 1, NULL, 0},
        {"ring",                  1, NULL, 0},
        {NULL,                    0, NULL, 0},
    };
    const char*     name;
    char*           firmwarePath = NULL;
    Uint32          ret = 0;

    osal_memset(&multiConfig, 0x00, sizeof(multiConfig));

#if 1
    for(i = 0; i < MAX_NUM_INSTANCE; i++) {
        SetDefaultDecTestConfig(&multiConfig.instConfig[i].decConfig);
    }
#endif

    while ((opt=getopt_long(argc, argv, optString, options, &index)) != -1) {
        switch (opt) {
        case 'c':
            tempArg = strtok(optarg, ",");
            for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                multiConfig.instConfig[i].compareType = atoi(tempArg);
                tempArg = strtok(NULL, ",");
                if (tempArg == NULL)
                    break;
            }
            break;
        case 'e':
            tempArg = strtok(optarg, ",");
            for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                multiConfig.instConfig[i].isEncoder = atoi(tempArg);
                tempArg = strtok(NULL, ",");
                if (tempArg == NULL)
                    break;
            }
            break;
        case 'h':
            Help(argv[0]);
            return 0;
        case 'n':
            multiConfig.numFrames = atoi(optarg);
            break;
        case 0:
            name = options[index].name;
            if (strcmp("instance-num", name) == 0) {
                multiConfig.numMulti = atoi(optarg);
            }
            else if (strcmp("codec", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig.instConfig[i].stdMode = (CodStd)atoi(tempArg);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("input", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    osal_memcpy(multiConfig.instConfig[i].inputFilePath, tempArg, strlen(tempArg));
                    multiConfig.instConfig[i].inputFilePath[strlen(tempArg)] = 0;
                    VLOG(INFO, "inputFilePath[%d]=%s\n",i,multiConfig.instConfig[i].inputFilePath);
                    ChangePathStyle(multiConfig.instConfig[i].inputFilePath);
                    
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("output", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    osal_memcpy(multiConfig.instConfig[i].outputFilePath, tempArg, strlen(tempArg));
                    multiConfig.instConfig[i].outputFilePath[strlen(tempArg)] = 0;
                    ChangePathStyle(multiConfig.instConfig[i].outputFilePath);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("ref_file_path", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    osal_memcpy(multiConfig.instConfig[i].refFilePath, tempArg, strlen(tempArg));
                    ChangePathStyle(multiConfig.instConfig[i].refFilePath);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("scaler", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig.instConfig[i].scaler = atoi(tempArg);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("sclw", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig.instConfig[i].sclw = atoi(tempArg);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("sclh", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig.instConfig[i].sclh = atoi(tempArg);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("bsmode", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig.instConfig[i].bsmode = atoi(tempArg);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (strcmp("enable-wtl", name) == 0) {
                tempArg = strtok(optarg, ",");
                for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig.instConfig[i].enableWTL = atoi(tempArg);
                    tempArg = strtok(NULL, ",");
                    if (tempArg == NULL)
                        break;
                }
            }
            else if (!strcmp(options[index].name, "pf")) {
                multiConfig.performance = (BOOL)atoi(optarg);
            }
            else if (!strcmp(options[index].name,  "pf-clock")) {
                multiConfig.pfClock = (Uint32)atoi(optarg);
            }
            else if (!strcmp(options[index].name,  "fps")) {
                multiConfig.fps = (Uint32)atoi(optarg);
            }
            else {
                VLOG(ERR, "unknown --%s\n", name);
                Help(argv[0]);
                return 1;
            }
            break;
        case '?':
            return 1;
        }
    }

    InitLog();


    productId = VPU_GetProductId(coreIndex);

    switch (productId) {
    case PRODUCT_ID_960: firmwarePath = CORE_0_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_980: firmwarePath = CORE_1_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_511: firmwarePath = CORE_6_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_517: firmwarePath = CORE_7_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_521: firmwarePath = CORE_6_BIT_CODE_FILE_PATH; break;
    default:
        VLOG(ERR, "<%s:%d> Unknown productId(%d)\n", __FUNCTION__, __LINE__, productId);
        return 1;
    }

    if (LoadFirmware(productId, (Uint8**)&pusBitCode, &sizeInWord, firmwarePath) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, firmwarePath);
        return 1;
    }
    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
        if (multiConfig.instConfig[i].isEncoder == FALSE)
            SetDecMultiParam(&multiConfig, i, productId);
        else
            SetEncMultiParam(&multiConfig, i, productId);
    }

    if (TRUE == multiConfig.performance) {
        LoadBalancerInit();
    }

    do {

        if (MultiInstanceTest(&multiConfig, pusBitCode, sizeInWord) == FALSE) {
            VLOG(ERR, "Failed to MultiInstanceTest()\n");
            ret = 1;
            break;
        }
    } while(FALSE);

    osal_free(pusBitCode);

    if (TRUE == multiConfig.performance) {
        LoadBalancerRelease();
    }

    return ret;
}

