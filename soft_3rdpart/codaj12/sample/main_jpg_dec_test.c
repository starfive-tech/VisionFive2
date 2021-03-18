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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>
#include "jpuapi.h"
#include "regdefine.h"
#include "jpulog.h"
#include "jpuapifunc.h"
#include "main_helper.h"

#define NUM_FRAME_BUF               MAX_FRAME
#define MAX_ROT_BUF_NUM             1

#ifdef SUPPORT_MULTI_INSTANCE_TEST
#else
static void Help(
    const char* programName
    )
{
    JLOG(INFO, "------------------------------------------------------------------------------\n");
    JLOG(INFO, " CODAJ12 Decoder\n");
    JLOG(INFO, "------------------------------------------------------------------------------\n");
    JLOG(INFO, "%s [options] --input=jpg_file_path\n", programName);
    JLOG(INFO, "-h                      help\n");
    JLOG(INFO, "--input=FILE            jpeg filepath\n");
    JLOG(INFO, "--output=FILE           output file path\n");
    JLOG(INFO, "--stream-endian=ENDIAN  bitstream endianness. refer to datasheet Chapter 4.\n");
    JLOG(INFO, "--frame-endian=ENDIAN   pixel endianness of 16bit input source. refer to datasheet Chapter 4.\n");
    JLOG(INFO, "--pixelj=JUSTIFICATION  16bit-pixel justification. 0(default) - msb justified, 1 - lsb justified in little-endianness\n");
    JLOG(INFO, "--bs-size=SIZE          bitstream buffer size in byte\n");
    JLOG(INFO, "--roi=x,y,w,h           ROI region\n");
    JLOG(INFO, "--subsample             conversion sub-sample(ignore case): NONE, 420, 422, 444\n");
    JLOG(INFO, "--ordering              conversion ordering(ingore-case): NONE, NV12, NV21, YUYV, YVYU, UYVY, VYUY, AYUV\n");
    JLOG(INFO, "                        NONE - planar format\n");
    JLOG(INFO, "                        NV12, NV21 - semi-planar format for all the subsamples.\n");
    JLOG(INFO, "                                     If subsample isn't defined or is none, the sub-sample depends on jpeg information\n");
    JLOG(INFO, "                                     The subsample 440 can be converted to the semi-planar format. It means that the encoded sub-sample should be 440.\n");
    JLOG(INFO, "                        YUVV..VYUY - packed format. subsample be ignored.\n");
    JLOG(INFO, "                        AYUV       - packed format. subsample be ignored.\n");
    JLOG(INFO, "--rotation              0, 90, 180, 270\n");
    JLOG(INFO, "--mirror                0(none), 1(V), 2(H), 3(VH)\n");
    JLOG(INFO, "--scaleH                Horizontal downscale: 0(none), 1(1/2), 2(1/4), 3(1/8)\n");
    JLOG(INFO, "--scaleV                Vertical downscale  : 0(none), 1(1/2), 2(1/4), 3(1/8)\n");

    exit(1);
}
#endif /* SUPPORT_MULTI_INSTANCE_TEST */


BOOL TestDecoder(
    DecConfigParam *param
    )
{
    JpgDecHandle        handle        = {0};
    JpgDecOpenParam     decOP        = {0};
    JpgDecInitialInfo   initialInfo = {0};
    JpgDecOutputInfo    outputInfo    = {0};
    JpgDecParam         decParam    = {0};
    JpgRet              ret = JPG_RET_SUCCESS;
    FrameBuffer         frameBuf[NUM_FRAME_BUF];
    jpu_buffer_t        vbStream    = {0};
    FRAME_BUF*          pFrame[NUM_FRAME_BUF];
    Uint32              framebufWidth=0, framebufHeight=0, framebufStride = 0;
    Int32               i = 0, frameIdx = 0, saveIdx =0, totalNumofErrMbs = 0;
    BOOL                suc = FALSE;
    Uint8*              pYuv     =    NULL;
    FILE*               fpYuv     =    NULL;
    Int32               needFrameBufCount = 0;
    Int32               int_reason = 0;
    Int32               instIdx = 0;
    Uint32              outbufSize=0;
    DecConfigParam      decConfig;
    Uint32              decodingWidth, decodingHeight;
    Uint32              displayWidth, displayHeight;
    FrameFormat         subsample;
    Uint32              bitDepth = 0;
    Uint32              temp;
    BOOL                scalerOn = FALSE;
    BSFeeder            feeder;

    memcpy(&decConfig, param, sizeof(DecConfigParam));
    memset(pFrame, 0x00, sizeof(pFrame));
    memset(frameBuf, 0x00, sizeof(frameBuf));

    ret = JPU_Init();
    if (ret != JPG_RET_SUCCESS && ret != JPG_RET_CALLED_BEFORE) {
        suc = 0;
        JLOG(ERR, "JPU_Init failed Error code is 0x%x \n", ret );
        goto ERR_DEC_INIT;
    }

    if ((feeder=BitstreamFeeder_Create(decConfig.bitstreamFileName, decConfig.feedingMode, (EndianMode)decConfig.StreamEndian)) == NULL) {
        goto ERR_DEC_INIT;
    }

    if (strlen(decConfig.yuvFileName)) {
        if ((fpYuv=fopen(decConfig.yuvFileName, "wb")) == NULL) {
            JLOG(ERR, "Can't open %s \n", decConfig.yuvFileName );
            goto ERR_DEC_INIT;
        }
    }

    // Open an instance and get initial information for decoding.
    vbStream.size = (decConfig.bsSize == 0) ? STREAM_BUF_SIZE : decConfig.bsSize;
    vbStream.size = (vbStream.size + 1023) & ~1023; // ceil128(size)
    if (jdi_allocate_dma_memory(&vbStream) < 0) {
        JLOG(ERR, "fail to allocate bitstream buffer\n" );
        goto ERR_DEC_INIT;
    }

    decOP.streamEndian          = decConfig.StreamEndian;
    decOP.frameEndian           = decConfig.FrameEndian;
    decOP.bitstreamBuffer       = vbStream.phys_addr;
    decOP.bitstreamBufferSize   = vbStream.size;
    //set virtual address mapped of physical address
    decOP.pBitStream            = (BYTE *)vbStream.virt_addr; //lint !e511
    decOP.chromaInterleave      = decConfig.cbcrInterleave;
    decOP.packedFormat          = decConfig.packedFormat;
    decOP.roiEnable             = decConfig.roiEnable;
    decOP.roiOffsetX            = decConfig.roiOffsetX;
    decOP.roiOffsetY            = decConfig.roiOffsetY;
    decOP.roiWidth              = decConfig.roiWidth;
    decOP.roiHeight             = decConfig.roiHeight;
    decOP.rotation              = decConfig.rotation;
    decOP.mirror                = decConfig.mirror;
    decOP.pixelJustification    = decConfig.pixelJustification;
    decOP.outputFormat          = decConfig.subsample;
    decOP.intrEnableBit         = ((1<<INT_JPU_DONE) | (1<<INT_JPU_ERROR) | (1<<INT_JPU_BIT_BUF_EMPTY));
    ret = JPU_DecOpen(&handle, &decOP);
    if( ret != JPG_RET_SUCCESS ) {
        JLOG(ERR, "JPU_DecOpen failed Error code is 0x%x \n", ret );
        goto ERR_DEC_INIT;
    }
    instIdx = handle->instIndex;
    //JPU_DecGiveCommand(handle, ENABLE_LOGGING, NULL);

    do {
    /* Fill jpeg data in the bitstream buffer */
        BitstreamFeeder_Act(feeder, handle, &vbStream);

        if ((ret=JPU_DecGetInitialInfo(handle, &initialInfo)) != JPG_RET_SUCCESS) {
            if (JPG_RET_BIT_EMPTY == ret) {
                JLOG(INFO, "<%s:%d> BITSTREAM EMPTY\n", __FUNCTION__, __LINE__);
                continue;
            }
            else {
                JLOG(ERR, "JPU_DecGetInitialInfo failed Error code is 0x%x, inst=%d \n", ret, instIdx);
                goto ERR_DEC_OPEN;
            }
        }
    } while (JPG_RET_SUCCESS != ret);

    if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_422)
        framebufWidth = JPU_CEIL(16, initialInfo.picWidth);
    else
        framebufWidth  = JPU_CEIL(8, initialInfo.picWidth);

    if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_440)
        framebufHeight = JPU_CEIL(16, initialInfo.picHeight);
    else
        framebufHeight = JPU_CEIL(8, initialInfo.picHeight);

    decodingWidth  = framebufWidth  >> decConfig.iHorScaleMode;
    decodingHeight = framebufHeight >> decConfig.iVerScaleMode;
    if (decOP.packedFormat != PACKED_FORMAT_NONE && decOP.packedFormat != PACKED_FORMAT_444) {
        // When packed format, scale-down resolution should be multiple of 2.
        decodingWidth  = JPU_CEIL(2, decodingWidth);
    }

    subsample = (decConfig.subsample == FORMAT_MAX) ? initialInfo.sourceFormat : decConfig.subsample;
    temp           = decodingWidth;
    decodingWidth  = (decConfig.rotation == 90 || decConfig.rotation == 270) ? decodingHeight : decodingWidth;
    decodingHeight = (decConfig.rotation == 90 || decConfig.rotation == 270) ? temp           : decodingHeight;
    if(decConfig.roiEnable == TRUE) {
        decodingWidth  = framebufWidth  = initialInfo.roiFrameWidth ;
        decodingHeight = framebufHeight = initialInfo.roiFrameHeight;
    }

    if (0 != decConfig.iHorScaleMode || 0 != decConfig.iVerScaleMode) {
        displayWidth  = JPU_FLOOR(2, (framebufWidth >> decConfig.iHorScaleMode));
        displayHeight = JPU_FLOOR(2, (framebufHeight >> decConfig.iVerScaleMode));
    }
    else {
        displayWidth  = decodingWidth;
        displayHeight = decodingHeight;
    }
    JLOG(INFO, "decodingWidth: %d, decodingHeight: %d\n", decodingWidth, decodingHeight);

    // Check restrictions
    if (decOP.rotation != 0 || decOP.mirror != MIRDIR_NONE) {
        if (decOP.outputFormat != FORMAT_MAX && decOP.outputFormat != initialInfo.sourceFormat) {
            JLOG(ERR, "The rotator cannot work with the format converter together.\n");
            goto ERR_DEC_OPEN;
        }
    }

    JLOG(INFO, "<INSTANCE %d>\n", instIdx);
    JLOG(INFO, "SOURCE PICTURE SIZE : W(%d) H(%d)\n", initialInfo.picWidth, initialInfo.picHeight);
    JLOG(INFO, "DECODED PICTURE SIZE: W(%d) H(%d)\n", displayWidth, displayHeight);
    JLOG(INFO, "SUBSAMPLE           : %d\n",          subsample);

    //Allocate frame buffer
    needFrameBufCount = initialInfo.minFrameBufferCount;
    bitDepth          = initialInfo.bitDepth;
    scalerOn          = (BOOL)(decConfig.iHorScaleMode || decConfig.iVerScaleMode);
    if (AllocateFrameBuffer(instIdx, subsample, decOP.chromaInterleave, decOP.packedFormat, decConfig.rotation, scalerOn, decodingWidth, decodingHeight, bitDepth, needFrameBufCount) == FALSE) {
        JLOG(ERR, "Failed to AllocateFrameBuffer()\n");
        goto ERR_DEC_OPEN;
    }

    for( i = 0; i < needFrameBufCount; ++i ) {
        pFrame[i] = GetFrameBuffer(instIdx, i);
        frameBuf[i].bufY  = pFrame[i]->vbY.phys_addr;
        frameBuf[i].bufCb = pFrame[i]->vbCb.phys_addr;
        if (decOP.chromaInterleave == CBCR_SEPARATED)
            frameBuf[i].bufCr = pFrame[i]->vbCr.phys_addr;
        frameBuf[i].stride  = pFrame[i]->strideY;
        frameBuf[i].strideC = pFrame[i]->strideC;
        frameBuf[i].endian  = decOP.frameEndian;
        frameBuf[i].format  = (FrameFormat)pFrame[i]->Format;
    }
    framebufStride = frameBuf[0].stride;

    outbufSize = decodingWidth * decodingHeight * 3 * (bitDepth+7)/8;
    if ((pYuv=malloc(outbufSize)) == NULL) {
        JLOG(ERR, "Fail to allocation memory for display buffer\n");
        goto ERR_DEC_OPEN;
    }

    // Register frame buffers requested by the decoder.
    if ((ret=JPU_DecRegisterFrameBuffer(handle, frameBuf, needFrameBufCount, framebufStride)) != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto ERR_DEC_OPEN;
    }

    JPU_DecGiveCommand(handle, SET_JPG_SCALE_HOR,  &(decConfig.iHorScaleMode));
    JPU_DecGiveCommand(handle, SET_JPG_SCALE_VER,  &(decConfig.iVerScaleMode));

    /* LOG HEADER */
    JLOG(INFO, "I   F    FB_INDEX  FRAME_START  ECS_START  CONSUME   RD_PTR   WR_PTR      CYCLE\n");
    JLOG(INFO, "-------------------------------------------------------------------------------\n");

    while(1) {
        // Start decoding a frame.
        ret = JPU_DecStartOneFrame(handle, &decParam);
        if (ret != JPG_RET_SUCCESS && ret != JPG_RET_EOS) {
            if (ret == JPG_RET_BIT_EMPTY) {
                JLOG(INFO, "BITSTREAM NOT ENOUGH.............\n");
                BitstreamFeeder_Act(feeder, handle, &vbStream);
                continue;
            }

            JLOG(ERR, "JPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
            goto ERR_DEC_OPEN;
        }
        if (ret == JPG_RET_EOS) {
            JPU_DecGetOutputInfo(handle, &outputInfo);
            suc = TRUE;
            break;
        }

        //JLOG(INFO, "\t<+>INSTANCE #%d JPU_WaitInterrupt\n", handle->instIndex);
        while(1) {
            if ((int_reason=JPU_WaitInterrupt(handle, JPU_INTERRUPT_TIMEOUT_MS)) == -1) {
                JLOG(ERR, "Error : timeout happened\n");
                JPU_SWReset(handle);
                break;
            }

            if (int_reason & ((1<<INT_JPU_DONE) | (1<<INT_JPU_ERROR) | (1<<INT_JPU_SLICE_DONE))) {
                // Do no clear INT_JPU_DONE and INT_JPU_ERROR interrupt. these will be cleared in JPU_DecGetOutputInfo.
                JLOG(INFO, "\tINSTANCE #%d int_reason: %08x\n", handle->instIndex, int_reason);
                break;
            }

            if (int_reason & (1<<INT_JPU_BIT_BUF_EMPTY)) {
                if (decConfig.feedingMode != FEEDING_METHOD_FRAME_SIZE) {
                    BitstreamFeeder_Act(feeder, handle, &vbStream);
                }
                JPU_ClrStatus(handle, (1<<INT_JPU_BIT_BUF_EMPTY));
            }
        }
        //JLOG(INFO, "\t<->INSTANCE #%d JPU_WaitInterrupt\n", handle->instIndex);

        if ((ret=JPU_DecGetOutputInfo(handle, &outputInfo)) != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_DecGetOutputInfo failed Error code is 0x%x \n", ret );
            goto ERR_DEC_OPEN;
        }

        if (outputInfo.decodingSuccess == 0)
            JLOG(ERR, "JPU_DecGetOutputInfo decode fail framdIdx %d \n", frameIdx);

        JLOG(INFO, "%02d %04d  %8d     %8x %8x %10d  %8x  %8x %10d\n",
            instIdx, frameIdx, outputInfo.indexFrameDisplay, outputInfo.bytePosFrameStart, outputInfo.ecsPtr, outputInfo.consumedByte,
            outputInfo.rdPtr, outputInfo.wrPtr, outputInfo.frameCycle);

        if (outputInfo.indexFrameDisplay == -1)
            break;

        saveIdx = outputInfo.indexFrameDisplay;
        if (!SaveYuvImageHelperFormat_V20(fpYuv, pYuv, &frameBuf[saveIdx], decOP.chromaInterleave, decOP.packedFormat, displayWidth, displayHeight, bitDepth, FALSE)) {
            goto ERR_DEC_OPEN;
        }

        if (outputInfo.numOfErrMBs) {
            Int32 errRstIdx, errPosX, errPosY;
            errRstIdx = (outputInfo.numOfErrMBs & 0x0F000000) >> 24;
            errPosX = (outputInfo.numOfErrMBs & 0x00FFF000) >> 12;
            errPosY = (outputInfo.numOfErrMBs & 0x00000FFF);
            JLOG(ERR, "Error restart Idx : %d, MCU x:%d, y:%d, in Frame : %d \n", errRstIdx, errPosX, errPosY, frameIdx);
        }
        frameIdx++;

        if (decConfig.outNum && (frameIdx == decConfig.outNum)) {
            suc = TRUE;
            break;
        }

        if (decConfig.feedingMode == FEEDING_METHOD_FRAME_SIZE) {
            JPU_DecSetRdPtrEx(handle, vbStream.phys_addr, TRUE);
            BitstreamFeeder_Act(feeder, handle, &vbStream);
        }
    }

    if (totalNumofErrMbs) {
        suc = 0;
        JLOG(ERR, "Total Num of Error MBs : %d\n", totalNumofErrMbs);
    }

ERR_DEC_OPEN:
    // Now that we are done with decoding, close the open instance.
    ret = JPU_DecClose(handle);
    if (ret != JPG_RET_SUCCESS)
        suc = 0;
    JLOG(INFO, "\nDec End. Tot Frame %d\n", frameIdx);

    BitstreamFeeder_Destroy(feeder);

ERR_DEC_INIT:
    FreeFrameBuffer(instIdx);

    jdi_free_dma_memory(&vbStream);

    if (pYuv)
        free(pYuv);

    if (fpYuv)
        fclose(fpYuv);

    JPU_DeInit();

    return suc;
}

#ifdef SUPPORT_MULTI_INSTANCE_TEST
#else
/*line -e14 to inhibit multiple main symbol error*/
int main(int argc, char** argv)
{
    Int32   ret = 1;
    struct option longOpt[] = {
        { "stream-endian",      required_argument, NULL, 0 },
        { "frame-endian",       required_argument, NULL, 0 },
        { "output",             required_argument, NULL, 0 },
        { "input",              required_argument, NULL, 0 },
        { "pixelj",             required_argument, NULL, 0 },
        { "bs-size",            required_argument, NULL, 0 },
        { "roi",                required_argument, NULL, 0 },
        { "subsample",          required_argument, NULL, 0 },
        { "ordering",           required_argument, NULL, 0 },
        { "rotation",           required_argument, NULL, 0 },
        { "mirror",             required_argument, NULL, 0 },
        { "scaleH",             required_argument, NULL, 0 },
        { "scaleV",             required_argument, NULL, 0 },
        { NULL,                 no_argument,       NULL, 0 },
    };
    Int32           c;
    int             l;
    const char*     shortOpt    = "fh";
    DecConfigParam  config;
    TestDevConfig   devConfig   = { ACLK_MIN, CCLK_MIN, TRUE };
    char*           ext         = NULL;

    memset((void*)&config, 0x00, sizeof(DecConfigParam));
    config.subsample = FORMAT_MAX;

    while ((c=getopt_long(argc, argv, shortOpt, longOpt, &l)) != -1) {
        switch (c) {
        case 'h':
            Help(argv[0]);
            break;
        case 'f':
            devConfig.reset = FALSE;
            break;
        case 0:
            if (strcmp(longOpt[l].name, "aclk") == 0) {
                devConfig.aclk=atoi(optarg);
                if (devConfig.aclk < ACLK_MIN || devConfig.aclk > ACLK_MAX) {
                    JLOG(ERR, "Invalid ACLK(%d) valid range(%d ~ %d)\n", devConfig.aclk, ACLK_MIN, ACLK_MAX);
                    Help(argv[0]);
                }
            }
            else if (strcmp(longOpt[l].name, "cclk") == 0) {
                devConfig.cclk=atoi(optarg);
                if (devConfig.cclk < CCLK_MIN || devConfig.cclk > CCLK_MAX) {
                    JLOG(ERR, "Invalid CCLK(%d) valid range(%d ~ %d)\n", devConfig.cclk, CCLK_MIN, CCLK_MAX);
                    Help(argv[0]);
                }
            }
            else {
                if (ParseDecTestLongArgs((void*)&config, longOpt[l].name, optarg) == FALSE) {
                    Help(argv[0]);
                }
            }
            break;
        default:
            Help(argv[0]);
            break;
        }
    }

    if (CNM_InitTestDev(devConfig) == FALSE) {
        JLOG(ERR, "Failed to initialize FPGA\n");
        return 1;
    }

#ifndef SUPPORT_FFMPEG
    config.feedingMode = FEEDING_METHOD_FIXED_SIZE;
#endif
    /* CHECK PARAMETERS */
    if ((config.iHorScaleMode || config.iVerScaleMode) && config.roiEnable) {
        JLOG(ERR, "Invalid operation mode : ROI cannot work with the scaler\n");
        return 1;
    }
    if(config.packedFormat && config.roiEnable) {
        JLOG(ERR, "Invalid operation mode : ROI cannot work with the packed format conversion\n");
        return 1;
    }
    if (config.roiEnable && (config.rotation || config.mirror)) {
        JLOG(ERR, "Invalid operation mode : ROI cannot work with the PPU.\n");
    }

    ext = GetFileExtension(config.bitstreamFileName);
    if (strcasecmp("avi", ext) == 0 || strcasecmp("mkv", ext) == 0) {
        config.feedingMode = FEEDING_METHOD_FRAME_SIZE;
    }
    InitLog("ErrorLog.txt");
    ret = TestDecoder(&config);
    DeInitLog();

    return ret == TRUE ? 0 : 1;
}
#endif /* SUPPORT_MULTI_INSTANCE_TEST */
