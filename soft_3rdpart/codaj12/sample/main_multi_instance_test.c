/*
 * Copyright (c) 2018, Chips&Media
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
#include "jpulog.h"
#include "cnm_fpga.h"
#ifdef PLATFORM_LINUX
#include <pthread.h>
#include <unistd.h>
#endif
#ifdef PLATFORM_NON_OS
#error "Not support multi-instance test in non-os platform"
#endif

extern BOOL TestDecoder(DecConfigParam *param);
extern BOOL TestEncoder(EncConfigParam *param);

#define CFG_DIR                 "./cfg"
#define YUV_DIR                 "./yuv"



static void Help(const char *programName)
{
    JLOG(INFO, "------------------------------------------------------------------------------\n");
    JLOG(INFO, " CODAJ12 Multi Instance test\n");
    JLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    JLOG(INFO, "\tSample program controlling the Chips&Media VPU\n");
    JLOG(INFO, "------------------------------------------------------------------------------\n");
    JLOG(INFO, "%s [option] --input <stream list file, aka cmd file>\n", programName);
    JLOG(INFO, "-h                          help\n");
    JLOG(INFO, "-v                          print version information\n");
    JLOG(INFO, "-f                          conformance test without reset CNM FPGA\n");
    JLOG(INFO, "--input                     bitstream(decoder) or cfg(encoder) path\n");
    JLOG(INFO, "--output                    yuv(decoder) or bitstream(encoder) path\n");
    JLOG(INFO, "--enable-sync               sync control by a user\n");
}

static void SetDecDefaultValues(DecConfigParam* decCfg)
{
    if (decCfg->bsSize == 0) {
        decCfg->bsSize = 2*1024*1024;
    }
}

static void SetEncDefaultValues(EncConfigParam* encCfg)
{
    if (strstr(encCfg->cfgFileName, "12b")) {
        encCfg->extendedSequential = 1;
    }
    else {
        encCfg->extendedSequential = 0;
    }
    if (strstr(encCfg->cfgFileName, "_tiled")) {
        encCfg->tiledModeEnable  = TRUE;
    }
    else {
        encCfg->tiledModeEnable  = FALSE;
    }
    encCfg->bsSize               = 2*1024*1024;
    if (strlen(encCfg->strCfgDir) == 0) {
        strcpy(encCfg->strCfgDir, CFG_DIR);
    }
    if (strlen(encCfg->strYuvDir) == 0) {
        strcpy(encCfg->strYuvDir, YUV_DIR);
    }
}

static Int32 MultiInstanceTest(TestMultiConfig* multiConfig)
{
    Int32               result = 0; // success = 0
    Uint32              i = 0;
    pthread_t           thread_id[MAX_NUM_INSTANCE];
    void*               ret[MAX_NUM_INSTANCE];

    for (i=0; i<multiConfig->numInstances; i++) {
        switch (multiConfig->type[i]) {
        case JPU_DECODER:
            pthread_create(&thread_id[i], NULL, (void*)TestDecoder, (void*)&multiConfig->u[i].decConfig);
            break;
        case JPU_ENCODER:
            pthread_create(&thread_id[i], NULL, (void*)TestEncoder, (void*)&multiConfig->u[i].encConfig);
            break;
        default:
            JLOG(INFO, "<%s:%d> Unknown type: %d\n", __FUNCTION__, __LINE__, multiConfig->type[i]);
            break;
        }
        usleep(10 * 1000);
    }

    for (i=0; i < multiConfig->numInstances; i++) {
        pthread_join(thread_id[i], &ret[i]);
    }
    for (i=0; i < multiConfig->numInstances; i++) {
        printf("thread return = %ld\n", (intptr_t)ret[i]);
        if ((intptr_t)ret[i] != 1) { // success = 1
            result = 1;
        }
    }

    return result;
}

int main(int argc, char **argv)
{
    TestMultiConfig multiConfig;
    Int32           opt, i;
    int             l;
    char*           tempArg;
    char*           optString = "e:h";

    struct option options[] = {
        /* -------------------- common ------------------- */
        { "aclk",               required_argument, NULL, 0 },   /* single parameter */
        { "cclk",               required_argument, NULL, 0 },   /* single parameter */
        { "rdelay",             required_argument, NULL, 0 },   /* single parameter */
        { "wdelay",             required_argument, NULL, 0 },   /* single parameter */
        { "instance-num",       required_argument, NULL, 0 },   /* single parameter */
        { "cfg-dir",            required_argument, NULL, 0 },   /* single parameter for encoder */
        { "yuv-dir",            required_argument, NULL, 0 },   /* single parameter for encoder */
        { "input",              required_argument, NULL, 0 },
        { "output",             required_argument, NULL, 0 },
        { "stream-endian",      required_argument, NULL, 0 },
        { "frame-endian",       required_argument, NULL, 0 },
        { "pixelj",             required_argument, NULL, 0 },
        { "bs-size",            required_argument, NULL, 0 },
        /* -------------------- decoder ------------------- */
        { "format",             required_argument, NULL, 0 },
        { "rotation",           required_argument, NULL, 0 },
        { "mirror",             required_argument, NULL, 0 },
        /* -------------------- encoder --------------------*/
        { "slice-height",       required_argument, NULL, 0 },
        { "enable-slice-intr",  required_argument, NULL, 0 },
        { "enable-tiledMode",   required_argument, NULL, 0 },
        { NULL,                 0,                 NULL, 0 },
    };

    memset((void*)&multiConfig, 0x00, sizeof(multiConfig));
    for(i = 0; i < MAX_NUM_INSTANCE; i++) {
        multiConfig.type[i] = JPU_NONE;
        multiConfig.u[i].decConfig.subsample = FORMAT_MAX;
    }
    multiConfig.devConfig.aclk = ACLK_MIN;
    multiConfig.devConfig.cclk = CCLK_MIN;

    while ((opt=getopt_long(argc, argv, optString, options, &l)) != -1) {
        switch (opt) {
        case 'e':
            tempArg = strtok(optarg, ",");
            for(i = 0; i < MAX_NUM_INSTANCE; i++) {
                multiConfig.type[i] = (JPUComponentType)atoi(tempArg);
                tempArg = strtok(NULL, ",");
                if (tempArg == NULL)
                    break;
            }
            break;
        case 'h':
            Help(argv[0]);
            return 0;
        case 0:
            if (ParseMultiLongOptions(&multiConfig, options[l].name, optarg) == FALSE) {
                Help(argv[0]);
                return 0;
            }
            break;
        default:
            JLOG(ERR, "%s\n", optarg);
            return 1;
        }
    }

    for (i = 0; i < multiConfig.numInstances; i++) {
        if (multiConfig.type[i] == JPU_DECODER) {
            SetDecDefaultValues(&multiConfig.u[i].decConfig);
        }
        else if (multiConfig.type[i] == JPU_ENCODER) {
            SetEncDefaultValues(&multiConfig.u[i].encConfig);
        }
        else {
            JLOG(ERR, "%s:%d Unknown type: %d\n", __FUNCTION__, __LINE__, multiConfig.type[i]);
        }
    }

    InitLog("ErrorLog.txt");


    JPU_Init();
    if (MultiInstanceTest(&multiConfig) != 0) {
        JLOG(ERR, "Failed to MultiInstanceTest()\n");
        DeInitLog();
        return 1;
    }
    JPU_DeInit();

    return 0;
}
