//------------------------------------------------------------------------------
// File: main.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#include <string.h>
#include <getopt.h>
#include "main_helper.h"
#include "vpuapi.h"

#define STREAM_READ_SIZE                (512 * 16)
#define STREAM_READ_ALL_SIZE            (0)
#define STREAM_BUF_MIN_SIZE             (0x18000)
#define STREAM_BUF_SIZE                 0xB00000    // max bitstream size (11MB)
#define MAX_CTU_NUM                     0x4000      // CTU num for max resolution = 8192x8192/(64x64)
#define WAVE4_ENC_REPORT_SIZE           144


#ifdef ANDROID
#define CHECK_CORE_INDEX(COREINDEX) if (COREINDEX > (MAX_NUM_VPU_CORE-1) ) {  break; }
#else
#define CHECK_CORE_INDEX(COREINDEX) if (COREINDEX > (MAX_NUM_VPU_CORE-1) ) { fprintf(stderr, "Invalid Core Index\n"); break; } 
#endif

#define NUM_OF_BS_BUF                       1

static Uint32               sizeInWord;
static Uint16*              pusBitCode;
static char                 optYuvPath[256];

static void 
help(struct OptionExt *opt, const char *programName )
{
    int i;

    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", programName, API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "\tSample program controlling the Chips&Media VPU\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option]\n", programName);
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-n [num]                    output frame number\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-c                          compare with golden bitstream\n");

    for (i = 0;i < MAX_GETOPT_OPTIONS;i++)
    {
        if (opt[i].name == NULL)
            break;
        VLOG(INFO, "%s", opt[i].help);
    }
}

extern char *optarg;                /* argument associated with option */
int TestEncoder(TestEncConfig *param)
{
    EncHandle           handle;
    EncOpenParam        encOP;
    EncParam            encParam;
    EncInitialInfo      initialInfo;
    EncOutputInfo       outputInfo;
    SecAxiUse           secAxiUse;
    vpu_buffer_t        vbStream[NUM_OF_BS_BUF];
    Uint32              bsBufferCount       = 1;
    Uint32              bsQueueIndex        = 1;
    vpu_buffer_t        vbReport;
    vpu_buffer_t        vbReconFrameBuf[MAX_REG_FRAME];
    vpu_buffer_t        vbSourceFrameBuf[MAX_REG_FRAME];
    int                 FrameBufSize;
    FrameBuffer         fbSrc[ENC_SRC_BUF_NUM];
    FrameBufferAllocInfo fbAllocInfo;
    int                 ret = RETCODE_SUCCESS;
    EncHeaderParam encHeaderParam;
    int                 srcFrameIdx = 0, frameIdx = 0;
    int                 srcFrameWidth, srcFrameHeight, srcFrameStride;
    int                 framebufStride = 0, framebufWidth = 0, framebufHeight = 0, mapType;
    int                 suc = 0;
    int                 regFrameBufCount = 0;
    int                 int_reason = 0, i;
    int                 timeoutCount = 0;
    int                 instIdx, coreIdx;
    MirrorDirection     mirrorDirection;
    TestEncConfig       encConfig;
    Int32               productId;
    Comparator          comparatorBitStream  = NULL;
    BitstreamReader     bsReader        = NULL;
    FrameBuffer         fbRecon[MAX_REG_FRAME];
    vpu_buffer_t        vbRoi[MAX_REG_FRAME];
    vpu_buffer_t        vbCtuQp[MAX_REG_FRAME];
    Uint8               ctuQpMapBuf[MAX_CTU_NUM];
    // for encode host rbsp header
    vpu_buffer_t        vbPrefixSeiNal[MAX_REG_FRAME];
    vpu_buffer_t        vbSuffixSeiNal[MAX_REG_FRAME];
    vpu_buffer_t        vbHrdRbsp;
    vpu_buffer_t        vbVuiRbsp;
#ifdef TEST_ENCODE_CUSTOM_HEADER
    vpu_buffer_t        vbSeiNal[MAX_REG_FRAME];
    vpu_buffer_t        vbCustomHeader;
#endif

    Uint8               roiMapBuf[MAX_CTU_NUM];
    void*               yuvFeeder = NULL;
    FrameBufferFormat   srcFrameFormat;
    YuvInfo             yuvFeederInfo;
    Uint32              interruptTimeout = VPU_ENC_TIMEOUT;
#ifdef TEST_ENCODE_CUSTOM_HEADER
    hrd_t hrd;
#endif

    osal_memcpy(&encConfig, param, sizeof(TestEncConfig));

    osal_memset(&yuvFeederInfo,   0x00, sizeof(YuvInfo));
    osal_memset(&fbSrc[0],   0x00, sizeof(fbSrc));
    osal_memset(&fbRecon[0], 0x00, sizeof(fbRecon));
    osal_memset(vbReconFrameBuf, 0x00, sizeof(vbReconFrameBuf));
    osal_memset(vbSourceFrameBuf, 0x00, sizeof(vbSourceFrameBuf));
    osal_memset(&encOP, 0x00, sizeof(EncOpenParam));
    osal_memset(&encParam, 0x00, sizeof(EncParam));
    osal_memset(&initialInfo, 0x00, sizeof(EncInitialInfo));
    osal_memset(&outputInfo, 0x00, sizeof(EncOutputInfo));
    osal_memset(&secAxiUse, 0x00, sizeof(SecAxiUse));
    osal_memset(vbStream, 0x00, sizeof(vbStream));
    osal_memset(&vbReport, 0x00, sizeof(vpu_buffer_t));
    osal_memset(&encHeaderParam, 0x00, sizeof(EncHeaderParam));
    osal_memset(vbRoi, 0x00, sizeof(vbRoi));
    osal_memset(vbCtuQp, 0x00, sizeof(vbCtuQp));
    osal_memset(vbPrefixSeiNal, 0x00, sizeof(vbPrefixSeiNal));
    osal_memset(vbSuffixSeiNal, 0x00, sizeof(vbSuffixSeiNal));
    osal_memset(&vbVuiRbsp, 0x00, sizeof(vpu_buffer_t));
    osal_memset(&vbHrdRbsp, 0x00, sizeof(vpu_buffer_t));
#ifdef TEST_ENCODE_CUSTOM_HEADER
    osal_memset(vbSeiNal, 0x00, sizeof(vbSeiNal));
    osal_memset(&vbCustomHeader, 0x00, sizeof(vpu_buffer_t));
#endif

    instIdx = encConfig.instNum;
    coreIdx = encConfig.coreIdx;

    productId = VPU_GetProductId(coreIdx);

    ret = VPU_InitWithBitcode(coreIdx, pusBitCode, sizeInWord);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS)
    {
        VLOG(ERR, "Failed to boot up VPU(coreIdx: %d, productId: %d)\n", coreIdx, productId);
        goto ERR_ENC_INIT;
    }

    PrintVpuVersionInfo(coreIdx);

    encOP.bitstreamFormat = encConfig.stdMode;
    mapType = (encConfig.mapType & 0x0f);
    if (strlen(encConfig.cfgFileName) != 0) {
        ret = GetEncOpenParam(&encOP, &encConfig, NULL);
    }
    else {
        ret = GetEncOpenParamDefault(&encOP, &encConfig);
    }
    if (ret == 0)
        goto ERR_ENC_INIT;

    srcFrameWidth  = ((encOP.picWidth+7)&~7);        // width = 8-aligned (CU unit)
    srcFrameHeight = ((encOP.picHeight+7)&~7);       // height = 8-aligned (CU unit)
    srcFrameStride = ((encOP.picWidth+31)&~31);      // stride should be a 32-aligned.

    /* Set user's options */
    if (optYuvPath[0] != 0) 
        strcpy(encConfig.yuvFileName, optYuvPath);

    if (strlen(encConfig.cfgFileName) != 0) {
        if (encOP.srcBitDepth == 8) {
            encConfig.srcFormat = FORMAT_420;
        }
        else if (encOP.srcBitDepth == 10) {
            encConfig.srcFormat = FORMAT_420_P10_16BIT_LSB;
            if (encConfig.yuv_mode == YUV_MODE_YUV_LOADER) {
                VLOG(INFO, "Need to check YUV style.\n");
                encConfig.srcFormat = FORMAT_420_P10_16BIT_MSB;
            }
            if (encConfig.srcFormat3p4b == 1) {
                encConfig.srcFormat = FORMAT_420_P10_32BIT_MSB;
            }
            if (encConfig.srcFormat3p4b == 2) {
                encConfig.srcFormat = FORMAT_420_P10_32BIT_LSB;
            }
        }
    }



    if (encConfig.packedFormat >= 1)
        encConfig.srcFormat = FORMAT_422;

    if (encConfig.srcFormat == FORMAT_422 && encConfig.packedFormat >= PACKED_YUYV) {    
        int p10bits = encConfig.srcFormat3p4b == 0 ? 16 : 32;
        int packedFormat = GetPackedFormat(encOP.srcBitDepth, encConfig.packedFormat, p10bits, 1);

        if (packedFormat == -1) {
            VLOG(ERR, "fail to GetPackedFormat\n" );
            goto ERR_ENC_INIT;
        }
        encOP.srcFormat = packedFormat;
        srcFrameFormat  = (FrameBufferFormat)packedFormat;
        encOP.nv21      = 0;
        encOP.cbcrInterleave = 0;
    }
    else {
        encOP.srcFormat    = encConfig.srcFormat;
        srcFrameFormat     = (FrameBufferFormat)encConfig.srcFormat;
        encOP.nv21         = encConfig.nv21;
    }
    encOP.packedFormat = encConfig.packedFormat;
    
    framebufWidth  = (encOP.picWidth  +7)&~7;
    framebufHeight = (encOP.picHeight +7)&~7;

    if ((encConfig.rotAngle != 0 || encConfig.mirDir != 0) && !(encConfig.rotAngle== 180 && encConfig.mirDir == MIRDIR_HOR_VER)) {
        framebufWidth  = (encOP.picWidth  +31)&~31;
        framebufHeight = (encOP.picHeight +31)&~31;
    }

    if (encConfig.rotAngle == 90 || encConfig.rotAngle == 270) {
        framebufWidth  = (encOP.picHeight +31)&~31;
        framebufHeight = (encOP.picWidth  +31)&~31;
    }

    bsBufferCount = NUM_OF_BS_BUF;
    for (i=0; i<bsBufferCount; i++ ) {
        vbStream[i].size = STREAM_BUF_SIZE;
        if (vdi_allocate_dma_memory(coreIdx, &vbStream[i]) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n" );
            goto ERR_ENC_INIT;
        }
        VLOG(ERR, "STREAM_BUF=0x%x STREAM_BUF_SIZE=%d(0x%x)\n",vbStream[i].phys_addr, vbStream[i].size, vbStream[i].size);
    }

    encOP.bitstreamBuffer = vbStream[0].phys_addr;
    encOP.bitstreamBufferSize = vbStream[0].size;//* bsBufferCount;//

    // -- HW Constraint --
    // Required size = actual size of bitstream buffer + 32KBytes
    // Minimum size of bitstream : 96KBytes
    // Margin : 32KBytes
    // Please refer to 3.2.4.4 Encoder Stream Handling in WAVE420 programmer's guide.
    if (encConfig.ringBufferEnable)
        encOP.bitstreamBufferSize -= 0x8000; 

    encOP.ringBufferEnable =  encConfig.ringBufferEnable;
    encOP.cbcrInterleave = encConfig.cbcrInterleave;
    encOP.frameEndian   = encConfig.frame_endian;
    encOP.streamEndian  = encConfig.stream_endian;
    encOP.sourceEndian  = encConfig.source_endian;

    encOP.lineBufIntEn  =  encConfig.lineBufIntEn;
    encOP.coreIdx       = coreIdx;
    encOP.cbcrOrder     = CBCR_ORDER_NORMAL;
    encOP.EncStdParam.hevcParam.useLongTerm = (encConfig.useAsLongtermPeriod > 0 && encConfig.refLongtermPeriod > 0) ? 1 : 0;   // host can set useLongTerm to 1 or 0 directly.

    if (writeVuiRbsp(coreIdx, &encConfig, &encOP, &vbVuiRbsp) == FALSE)
        goto ERR_ENC_INIT;
    if (writeHrdRbsp(coreIdx, &encConfig, &encOP, &vbHrdRbsp) == FALSE)
        goto ERR_ENC_INIT;
#ifdef TEST_ENCODE_CUSTOM_HEADER
    if (writeCustomHeader(coreIdx, &encOP, &vbCustomHeader, &hrd) == FALSE)
        goto ERR_ENC_INIT;
#endif
    // Open an instance and get initial information for encoding.
    ret = VPU_EncOpen(&handle, &encOP);
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncOpen failed Error code is 0x%x \n", ret );
        goto ERR_ENC_INIT;
    }

    //VPU_EncGiveCommand(handle, ENABLE_LOGGING, 0);

    if (encConfig.rotAngle != 0 || encConfig.mirDir != 0) {
        VPU_EncGiveCommand(handle, ENABLE_ROTATION, 0);
        VPU_EncGiveCommand(handle, ENABLE_MIRRORING, 0);
        VPU_EncGiveCommand(handle, SET_ROTATION_ANGLE, &encConfig.rotAngle);
        mirrorDirection = (MirrorDirection)encConfig.mirDir;
        VPU_EncGiveCommand(handle, SET_MIRROR_DIRECTION, &mirrorDirection);
    }

    ret = VPU_EncGetInitialInfo(handle, &initialInfo);

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
        goto ERR_ENC_OPEN;
    }


    VLOG(INFO, "* Enc InitialInfo =>\n instance #%d, \n minframeBuffercount: %u\n minSrcBufferCount: %d\n", instIdx, initialInfo.minFrameBufferCount, initialInfo.minSrcFrameCount);
    VLOG(INFO, " picWidth: %u\n picHeight: %u\n ",encOP.picWidth, encOP.picHeight); 

    if (encConfig.compare_type & (1<<MODE_COMP_ENCODED)) {
        comparatorBitStream = Comparator_Create(STREAM_COMPARE, encConfig.ref_stream_path, encConfig.cfgFileName);
        if (comparatorBitStream == NULL) {
            goto ERR_ENC_OPEN;
        }
    }

    secAxiUse.u.wave4.useEncImdEnable  = (encConfig.secondary_axi & 0x1)?TRUE:FALSE;  //USE_IMD_INTERNAL_BUF
    secAxiUse.u.wave4.useEncRdoEnable  = (encConfig.secondary_axi & 0x2)?TRUE:FALSE;  //USE_RDO_INTERNAL_BUF
    secAxiUse.u.wave4.useEncLfEnable   = (encConfig.secondary_axi & 0x4)?TRUE:FALSE;  //USE_LF_INTERNAL_BUF
    VPU_EncGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

    /* Allocate framebuffers for recon. */
    framebufStride = CalcStride(framebufWidth, framebufHeight, (FrameBufferFormat)encOP.srcFormat, encOP.cbcrInterleave, (TiledMapType)mapType, FALSE);
    FrameBufSize   = VPU_GetFrameBufSize(coreIdx, framebufStride, framebufHeight, (TiledMapType)mapType, encOP.srcFormat, encOP.cbcrInterleave, NULL);

    regFrameBufCount = initialInfo.minFrameBufferCount;
    for (i = 0; i < regFrameBufCount; i++) {
        vbReconFrameBuf[i].size = FrameBufSize;
        if (vdi_allocate_dma_memory(coreIdx, &vbReconFrameBuf[i]) < 0) {
            VLOG(ERR, "fail to allocate recon buffer\n" );
            goto ERR_ENC_OPEN;
        }
        fbRecon[i].bufY  = vbReconFrameBuf[i].phys_addr;
        fbRecon[i].bufCb = (PhysicalAddress)-1;
        fbRecon[i].bufCr = (PhysicalAddress)-1;
        fbRecon[i].size  = FrameBufSize;
        fbRecon[i].updateFbInfo = TRUE;
    }

    ret = VPU_EncRegisterFrameBuffer(handle, fbRecon, regFrameBufCount, framebufStride, framebufHeight, mapType);
    if( ret != RETCODE_SUCCESS ) {
        VLOG(ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto ERR_ENC_OPEN;
    }


    fbAllocInfo.mapType = LINEAR_FRAME_MAP;

    srcFrameStride = CalcStride(srcFrameWidth, srcFrameHeight, (FrameBufferFormat)srcFrameFormat, encOP.cbcrInterleave, (TiledMapType)fbAllocInfo.mapType, FALSE);
    FrameBufSize   = VPU_GetFrameBufSize(coreIdx, srcFrameStride, srcFrameHeight, (TiledMapType)fbAllocInfo.mapType, srcFrameFormat, encOP.cbcrInterleave, NULL);

    fbAllocInfo.format = (FrameBufferFormat)srcFrameFormat;
    fbAllocInfo.cbcrInterleave = encOP.cbcrInterleave;
    fbAllocInfo.stride = srcFrameStride;
    fbAllocInfo.height = srcFrameHeight;
    fbAllocInfo.endian = encOP.sourceEndian;
    fbAllocInfo.type   = FB_TYPE_PPU;
    fbAllocInfo.num    = initialInfo.minSrcFrameCount + EXTRA_SRC_BUFFER_NUM;
    fbAllocInfo.nv21   = encOP.nv21;
    
    VLOG(INFO, "Allocated source framebuffers : %d, size : %d\n", fbAllocInfo.num, FrameBufSize);
    for (i = 0; i < fbAllocInfo.num; i++) {
        vbSourceFrameBuf[i].size = FrameBufSize;
        if (vdi_allocate_dma_memory(coreIdx, &vbSourceFrameBuf[i]) < 0)
        {
            VLOG(ERR, "fail to allocate frame buffer\n" );
            goto ERR_ENC_OPEN;
        }
        fbSrc[i].bufY  = vbSourceFrameBuf[i].phys_addr;
        fbSrc[i].bufCb = (PhysicalAddress)-1;
        fbSrc[i].bufCr = (PhysicalAddress)-1;
        fbSrc[i].size  = FrameBufSize;
        fbSrc[i].updateFbInfo = TRUE;
    }

    ret = VPU_EncAllocateFrameBuffer(handle, fbAllocInfo, fbSrc);
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
        goto ERR_ENC_OPEN;
    }
    if (openRoiMapFile(&encConfig) == FALSE) {
        goto ERR_ENC_OPEN;
    }
    if (allocateRoiMapBuf(coreIdx, encConfig, &vbRoi[0], fbAllocInfo.num, MAX_CTU_NUM) == FALSE) {
        goto ERR_ENC_OPEN;
    }
    if (openCtuQpMapFile(&encConfig) == FALSE) {
        goto ERR_ENC_OPEN;
    }
    if ( allocateCtuQpMapBuf(coreIdx, encConfig, &vbCtuQp[0], fbAllocInfo.num, MAX_CTU_NUM) == FALSE) {
        goto ERR_ENC_OPEN;
    }

    // allocate User data buffer amount of source buffer num
    if (encConfig.seiDataEnc.prefixSeiNalEnable) {
        if (encConfig.prefix_sei_nal_file_name[0]) {
            ChangePathStyle(encConfig.prefix_sei_nal_file_name);
            if ((encConfig.prefix_sei_nal_fp = osal_fopen(encConfig.prefix_sei_nal_file_name, "r")) == NULL) {
                VLOG(ERR, "fail to open Prefix SEI NAL Data file, %s\n", encConfig.prefix_sei_nal_file_name);
                goto ERR_ENC_OPEN;
            }
        }

        for (i = 0; i < fbAllocInfo.num ; i++) {         // the number of roi buffer should be the same as source buffer num.
            vbPrefixSeiNal[i].size = SEI_NAL_DATA_BUF_SIZE;
            if (vdi_allocate_dma_memory(coreIdx, &vbPrefixSeiNal[i]) < 0) {
                VLOG(ERR, "fail to allocate ROI buffer\n" );
                goto ERR_ENC_OPEN;
            }
        }
    }

    if (encConfig.seiDataEnc.suffixSeiNalEnable) {
        if (encConfig.suffix_sei_nal_file_name[0]) {
            ChangePathStyle(encConfig.suffix_sei_nal_file_name);
            if ((encConfig.suffix_sei_nal_fp = osal_fopen(encConfig.suffix_sei_nal_file_name, "r")) == NULL) {
                VLOG(ERR, "fail to open Prefix SEI NAL Data file, %s\n", encConfig.suffix_sei_nal_file_name);
                goto ERR_ENC_OPEN;
            }
        }

        for (i = 0; i < fbAllocInfo.num ; i++) {         // the number of roi buffer should be the same as source buffer num.
            vbSuffixSeiNal[i].size = SEI_NAL_DATA_BUF_SIZE;
            if (vdi_allocate_dma_memory(coreIdx, &vbSuffixSeiNal[i]) < 0) {
                VLOG(ERR, "fail to allocate ROI buffer\n" );
                goto ERR_ENC_OPEN;
            }
        }
    }
#ifdef TEST_ENCODE_CUSTOM_HEADER
    if (allocateSeiNalDataBuf(coreIdx, &vbSeiNal[0], fbAllocInfo.num) == FALSE) {
        goto ERR_ENC_OPEN;
    }
#endif

    encParam.skipPicture        = 0;
    encParam.quantParam         = encConfig.picQpY;
    encParam.skipPicture        = 0;
    encParam.forcePicQpEnable   = 0;
    encParam.forcePicQpI        = 0;
    encParam.forcePicQpP        = 0;
    encParam.forcePicQpB        = 0;
    encParam.forcePicTypeEnable = 0;
    encParam.forcePicType       = 0;
    encParam.codeOption.implicitHeaderEncode = 1;      // FW will encode header data implicitly when changing the header syntaxes
    encParam.codeOption.encodeAUD   = encConfig.encAUD;
    encParam.codeOption.encodeEOS   = 0;


    if (encOP.ringBufferEnable == TRUE) {    
        VPU_EncSetWrPtr(handle, encOP.bitstreamBuffer, 1);
    }
    else {
        encHeaderParam.buf = encOP.bitstreamBuffer;
        encHeaderParam.size = encOP.bitstreamBufferSize;
    }

    encHeaderParam.headerType = CODEOPT_ENC_VPS | CODEOPT_ENC_SPS | CODEOPT_ENC_PPS;
    ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VPS/SPS/PPS failed Error Reason code : 0x%x \n", ret);           
        goto ERR_ENC_OPEN;
    }
    if (encHeaderParam.size == 0) {
        VLOG(ERR, "encHeaderParam.size=0\n");
        goto ERR_ENC_OPEN;
    }

#ifdef SUPPORT_DONT_READ_STREAM
#else
    bsReader = BitstreamReader_Create(encOP.ringBufferEnable, encConfig.bitstreamFileName, (EndianMode)encOP.streamEndian, &handle);
#endif
    EnterLock(coreIdx);
    ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encOP.bitstreamBufferSize, encHeaderParam.size, comparatorBitStream);
    LeaveLock(coreIdx);
    if (ret == FALSE) {
        goto ERR_ENC_OPEN;
    }

    yuvFeederInfo.cbcrInterleave = encConfig.cbcrInterleave;
    yuvFeederInfo.nv21           = encConfig.nv21;
    yuvFeederInfo.packedFormat   = encConfig.packedFormat;
    yuvFeederInfo.srcFormat      = encOP.srcFormat;
    yuvFeederInfo.srcPlanar      = TRUE;
    yuvFeederInfo.srcStride      = srcFrameStride;
    yuvFeederInfo.srcHeight      = srcFrameHeight;
    yuvFeeder = YuvFeeder_Create(encConfig.yuv_mode, encConfig.yuvFileName, yuvFeederInfo);
    if ( yuvFeeder == NULL ) {
        VLOG(INFO, "YuvFeeder_Create error");
        goto ERR_ENC_OPEN;
    }

    VLOG(INFO, "Enc Start : Press any key to stop.\n");

    if (encOP.ringBufferEnable == TRUE) {
        VPU_EncSetWrPtr(handle, encOP.bitstreamBuffer, 1);    // this function shows that HOST can set wrPtr to start position of encoded output in ringbuffer enable mode
    }

    DisplayEncodedInformation(handle, STD_HEVC, 0, NULL, 0, 0);

    while( 1 ) {
        if (osal_kbhit()) {
            break;
        }


        srcFrameIdx = (frameIdx%fbAllocInfo.num);
        encParam.srcIdx = srcFrameIdx;
        ret = YuvFeeder_Feed(yuvFeeder, coreIdx, &fbSrc[srcFrameIdx], encOP.picWidth, encOP.picHeight, NULL);
        if ( ret == 0 ) {
            encParam.srcEndFlag = 1;            // when there is no more source image to be encoded, srcEndFlag should be set 1. because of encoding delay for WAVE420
        }

        if ( encParam.srcEndFlag != 1) {
            fbSrc[srcFrameIdx].srcBufState = SRC_BUFFER_USE_ENCODE;
            encParam.sourceFrame = &fbSrc[srcFrameIdx];
            encParam.sourceFrame->sourceLBurstEn = 0;///???
        }
        if( encOP.ringBufferEnable == FALSE) {
            bsQueueIndex = (bsQueueIndex+1)%bsBufferCount;
            encParam.picStreamBufferAddr = vbStream[bsQueueIndex].phys_addr;   // can set the newly allocated buffer.
            encParam.picStreamBufferSize = encOP.bitstreamBufferSize;
        }
        
        if ( (encConfig.seiDataEnc.prefixSeiNalEnable || encConfig.seiDataEnc.suffixSeiNalEnable) && encParam.srcEndFlag != 1) {
            if (encConfig.prefix_sei_nal_file_name) {
                Uint8   *pUserBuf;
                pUserBuf = (Uint8*)osal_malloc(SEI_NAL_DATA_BUF_SIZE);
                osal_memset(pUserBuf, 0, SEI_NAL_DATA_BUF_SIZE);
                osal_fread(pUserBuf, 1, encConfig.seiDataEnc.prefixSeiDataSize, encConfig.prefix_sei_nal_fp);
                vdi_write_memory(coreIdx, vbPrefixSeiNal[srcFrameIdx].phys_addr, pUserBuf, encConfig.seiDataEnc.prefixSeiDataSize, encOP.streamEndian);
                osal_free(pUserBuf);
            }
            
            if (encConfig.suffix_sei_nal_file_name) {
                Uint8   *pUserBuf;
                pUserBuf = (Uint8*)osal_malloc(SEI_NAL_DATA_BUF_SIZE);
                osal_memset(pUserBuf, 0, SEI_NAL_DATA_BUF_SIZE);
                osal_fread(pUserBuf, 1, encConfig.seiDataEnc.suffixSeiDataSize, encConfig.suffix_sei_nal_fp);
                vdi_write_memory(coreIdx, vbSuffixSeiNal[srcFrameIdx].phys_addr, pUserBuf, encConfig.seiDataEnc.suffixSeiDataSize, encOP.streamEndian);
                osal_free(pUserBuf);
            }
            encConfig.seiDataEnc.prefixSeiNalAddr = vbPrefixSeiNal[srcFrameIdx].phys_addr;
            encConfig.seiDataEnc.suffixSeiNalAddr = vbSuffixSeiNal[srcFrameIdx].phys_addr;
            VPU_EncGiveCommand(handle, ENC_SET_SEI_NAL_DATA, &encConfig.seiDataEnc);
        }
        osal_memset(&encParam.ctuOptParam, 0, sizeof(HevcCtuOptParam));

        setRoiMap(coreIdx, &encConfig, encOP, vbRoi[srcFrameIdx].phys_addr, &roiMapBuf[0], srcFrameWidth, srcFrameHeight, &encParam, MAX_CTU_NUM);

        setCtuQpMap(coreIdx, &encConfig, encOP, vbCtuQp[srcFrameIdx].phys_addr, &ctuQpMapBuf[0], srcFrameWidth, srcFrameHeight, &encParam, MAX_CTU_NUM);
#ifdef TEST_ENCODE_CUSTOM_HEADER
        if (writeSeiNalData(handle, encOP.streamEndian, vbSeiNal[srcFrameIdx].phys_addr, &hrd) == FALSE) {
            goto ERR_ENC_OPEN;
        }
#endif
        if (encConfig.useAsLongtermPeriod > 0 && encConfig.refLongtermPeriod > 0) {
            encParam.useCurSrcAsLongtermPic = (frameIdx % encConfig.useAsLongtermPeriod) == 0 ? 1 : 0;
            encParam.useLongtermRef         = (frameIdx % encConfig.refLongtermPeriod)   == 0 ? 1 : 0;
        }

        // Start encoding a frame.
        frameIdx++;
        ret = VPU_EncStartOneFrame(handle, &encParam);
        if(ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
            LeaveLock(coreIdx);
            goto ERR_ENC_OPEN;
        }

        timeoutCount = 0;
        while (TRUE) {
            int_reason = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);

            if (int_reason == -1) {
                if (interruptTimeout > 0 && timeoutCount*VPU_WAIT_TIME_OUT > interruptTimeout) {
                    VLOG(ERR, "Error : encoder timeout happened\n");
                    PrintVpuStatus(coreIdx, productId);
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    break;
                }
                int_reason = 0;
                timeoutCount++;
            }

            if (encOP.ringBufferEnable == TRUE) {
                if (!BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, STREAM_READ_SIZE, comparatorBitStream)) {
                    PrintVpuStatus(coreIdx, productId);
                    break;
                }
            }

            if (int_reason & (1<<INT_BIT_BIT_BUF_FULL)) {
                VLOG(WARN,"INT_BIT_BIT_BUF_FULL \n");
                BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, STREAM_READ_ALL_SIZE, comparatorBitStream);
            }


            if (int_reason) {
                VPU_ClearInterrupt(coreIdx);
                if (int_reason & (1<<INT_WAVE_ENC_PIC)) {
                    break;
                }
            }
        }

        ret = VPU_EncGetOutputInfo(handle, &outputInfo);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncGetOutputInfo failed Error code is 0x%x \n", ret );
            if (ret == RETCODE_STREAM_BUF_FULL) {
                VLOG(ERR, "RETCODE_STREAM_BUF_FULL\n");
                continue;
            }
            else if ( ret == RETCODE_MEMORY_ACCESS_VIOLATION || ret == RETCODE_CP0_EXCEPTION || ret == RETCODE_ACCESS_VIOLATION_HW)
            {
                EnterLock(coreIdx);
                PrintMemoryAccessViolationReason(coreIdx, (void*)&outputInfo);
                PrintVpuStatus(coreIdx, productId);
                VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                LeaveLock(coreIdx);
            } else {
                EnterLock(coreIdx);
                PrintVpuStatus(coreIdx, productId);
                VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                LeaveLock(coreIdx);
            }
            goto ERR_ENC_OPEN;
        }

        DisplayEncodedInformation(handle, STD_HEVC, 0, &outputInfo, encParam.srcEndFlag , srcFrameIdx);

        if (encOP.ringBufferEnable == 0) {
            if (outputInfo.bitstreamWrapAround == 1) {
                VLOG(WARN, "Warnning!! BitStream buffer wrap arounded. prepare more large buffer \n", ret );
            }
            if (outputInfo.bitstreamSize == 0 && outputInfo.reconFrameIndex >= 0) {
                VLOG(ERR, "ERROR!!! bitstreamsize = 0 \n");
            }

            if (encOP.lineBufIntEn == 0) {
                if (outputInfo.wrPtr < outputInfo.rdPtr) 
                {
                    VLOG(ERR, "wrptr < rdptr\n");
                    goto ERR_ENC_OPEN;
                }
            }

            if ( outputInfo.bitstreamSize ) {
                EnterLock(coreIdx);
                ret = BitstreamReader_Act(bsReader, outputInfo.bitstreamBuffer, encOP.bitstreamBufferSize, outputInfo.bitstreamSize, comparatorBitStream);
                LeaveLock(coreIdx);
                if (ret == FALSE) {
                    goto ERR_ENC_OPEN;
                }
            }
        }

        if (outputInfo.reconFrameIndex == -1)       // end of encoding
        {
            break;
        }

#ifdef ENC_RECON_FRAME_DISPLAY
        SimpleRenderer_Act(); 
#endif      
        if (frameIdx > (encConfig.outNum-1))
            encParam.srcEndFlag = 1;
    }

    if( encOP.ringBufferEnable == 1 ) {
        EnterLock(coreIdx);
        ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 0, comparatorBitStream);
        LeaveLock(coreIdx);
        if (ret == FALSE) {
            EnterLock(coreIdx);
            PrintVpuStatus(coreIdx, productId);
            LeaveLock(coreIdx);
            goto ERR_ENC_OPEN;
        }
    }

    if (param->outNum == 0) {
        if (comparatorBitStream) {
            if (Comparator_CheckEOF(comparatorBitStream) == FALSE) {
                VLOG(ERR, "MISMATCH BitStream data size. There is still data to compare.\n");
                goto ERR_ENC_OPEN;
            }
        }
    }

    suc = 1;


ERR_ENC_OPEN:
    // Now that we are done with encoding, close the open instance.
    for (i = 0; i < regFrameBufCount; i++) {
        if (vbReconFrameBuf[i].size > 0) {
            vdi_free_dma_memory(coreIdx, &vbReconFrameBuf[i]);
        }
    }
    for (i = 0; i < fbAllocInfo.num; i++) {
        if (vbSourceFrameBuf[i].size > 0) {
            vdi_free_dma_memory(coreIdx, &vbSourceFrameBuf[i]);
        }
        if (vbRoi[i].size) {
            vdi_free_dma_memory(coreIdx, &vbRoi[i]);
        }
        if (vbCtuQp[i].size) {
            vdi_free_dma_memory(coreIdx, &vbCtuQp[i]);
        }
        if (vbPrefixSeiNal[i].size)
            vdi_free_dma_memory(coreIdx, &vbPrefixSeiNal[i]);
        if (vbSuffixSeiNal[i].size)
            vdi_free_dma_memory(coreIdx, &vbSuffixSeiNal[i]);
#ifdef TEST_ENCODE_CUSTOM_HEADER
        if (vbSeiNal[i].size)
            vdi_free_dma_memory(coreIdx, &vbSeiNal[i]);
#endif
    }

    if (vbHrdRbsp.size)
        vdi_free_dma_memory(coreIdx, &vbHrdRbsp);

    if (vbVuiRbsp.size)
        vdi_free_dma_memory(coreIdx, &vbVuiRbsp);

#ifdef TEST_ENCODE_CUSTOM_HEADER
    if (vbCustomHeader.size)
        vdi_free_dma_memory(coreIdx, &vbCustomHeader);
#endif
    if (encConfig.prefix_sei_nal_fp)
        osal_fclose(encConfig.prefix_sei_nal_fp);

    if (encConfig.suffix_sei_nal_fp)
        osal_fclose(encConfig.suffix_sei_nal_fp);

    if (encConfig.vui_rbsp_fp)
        osal_fclose(encConfig.vui_rbsp_fp);

    if (encConfig.hrd_rbsp_fp)
        osal_fclose(encConfig.hrd_rbsp_fp);

    if (encConfig.roi_file)
        osal_fclose(encConfig.roi_file);
    
    VPU_EncClose(handle);
    VLOG(INFO, "\ninst %d Enc End. Tot Frame %d\n" , instIdx, outputInfo.encPicCnt);

ERR_ENC_INIT:

    for (i=0; i< bsBufferCount ; i++) {
        if (vbStream[i].size)
            vdi_free_dma_memory(coreIdx, &vbStream[i]);
    }
    if (vbReport.size)
        vdi_free_dma_memory(coreIdx, &vbReport);

    if (comparatorBitStream != NULL) {
        Comparator_Destroy(comparatorBitStream);
        osal_free(comparatorBitStream);
    }

    BitstreamReader_Destroy(bsReader);
    if (yuvFeeder != NULL) {
        YuvFeeder_Destroy(yuvFeeder);
        osal_free(yuvFeeder);
    }

    VPU_DeInit(coreIdx);

    return suc;
}

int main(int argc, char **argv)
{
    Uint32          productId;
    BOOL            showVersion = FALSE;
    BOOL            debugMode   = FALSE;
    int opt, index, ret = 0, i;
    TestEncConfig   encConfig;
    struct option options[MAX_GETOPT_OPTIONS];
    struct OptionExt options_help[] = {
        {"output",                1, NULL, 0, "--output                    bitstream path\n"},
        {"input",                 1, NULL, 0, "--input                     YUV file path. The value of InputFile in a cfg is replaced to this value.\n"},
        {"codec",                 1, NULL, 0, "--codec                     codec index, HEVC: 12\n"},
        {"cfgFileName",           1, NULL, 0, "--cfgFileName               cfg file path\n"},
        {"coreIdx",               1, NULL, 0, "--coreIdx                   core index: default 0\n"},
        {"picWidth",              1, NULL, 0, "--picWidth                  source width\n"},
        {"picHeight",             1, NULL, 0, "--picHeight                 source height\n"},
        {"kbps",                  1, NULL, 0, "--kbps                      RC bitrate in kbps. In case of without cfg file, if this option has value then RC will be enabled\n"},
        {"enable-ringBuffer",     0, NULL, 0, "--enable-ringBuffer         enable stream ring buffer mode\n"},
        {"enable-lineBufInt",     0, NULL, 0, "--enable-lineBufInt         enable linebuffer interrupt\n"},
        {"mapType",               1, NULL, 0, "--mapType                   mapType\n"},
        {"loop-count",            1, NULL, 0, "--loop-count                integer number. loop test, default 0\n"},
        {"enable-cbcrInterleave", 0, NULL, 0, "--enable-cbcrInterleave     enable cbcr interleave\n"},
        {"nv21",                  1, NULL, 0, "--nv21                      enable NV21(must set enable-cbcrInterleave)\n"},
        {"packedFormat",          1, NULL, 0, "--packedFormat              1:YUYV, 2:YVYU, 3:UYVY, 4:VYUY\n"},
        {"rotAngle",              1, NULL, 0, "--rotAngle                  rotation angle(0,90,180,270), Not supported on WAVE420L\n"},
        {"mirDir",                1, NULL, 0, "--mirDir                    1:Vertical, 2: Horizontal, 3:Vert-Horz, Not supported on WAVE420L\n"}, /* 20 */
        {"secondary-axi",         1, NULL, 0, "--secondary-axi             0~7: bit mask values, Please refer programmer's guide or datasheet\n"
                                              "                            1:IMD(not supported on WAVE420L), 2: RDO, 4: LF\n"},
        {"frame-endian",          1, NULL, 0, "--frame-endian              16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"stream-endian",         1, NULL, 0, "--stream-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"source-endian",         1, NULL, 0, "--source-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"ref_stream_path",       1, NULL, 0, "--ref_stream_path           golden data which is compared with encoded stream when -c option\n"},
        {"srcFormat3p4b",         1, NULL, 0, "--srcFormat3p4b             [WAVE420]This option MUST BE enabled when format of source yuv is 3pixel 4byte format\n"},
        {NULL,                    0, NULL, 0},
    };
    char*           optString = "rbhvn:";
    char*           fwPath    = NULL;


    InitLog();

    //default setting.
    osal_memset(&encConfig, 0, sizeof(encConfig));
    encConfig.stdMode       = STD_HEVC;
    encConfig.frame_endian  = VPU_FRAME_ENDIAN;
    encConfig.stream_endian = VPU_STREAM_ENDIAN;
    encConfig.source_endian = VPU_SOURCE_ENDIAN;
    encConfig.mapType       = COMPRESSED_FRAME_MAP;



    for (i = 0; i < MAX_GETOPT_OPTIONS;i++) {
        if (options_help[i].name == NULL)
            break;
        osal_memcpy(&options[i], &options_help[i], sizeof(struct option));
    }
    while ((opt=getopt_long(argc, argv, optString, options, &index)) != -1) {
        switch (opt) {
        case 'n':
            encConfig.outNum = atoi(optarg);
            break;
        case 'c':
            encConfig.compare_type |= (1 << MODE_COMP_ENCODED);
            VLOG(ERR,"Stream compare Enable\n");
            break;
        case 'h':
            help(options_help, argv[0]);
            return 0;
            break;
        case 'v':
            showVersion = TRUE;
            break;
        case 0:
            if (!strcmp(options[index].name, "output")) {
                osal_memcpy(encConfig.bitstreamFileName, optarg, strlen(optarg));
                ChangePathStyle(encConfig.bitstreamFileName);
            } else if (!strcmp(options[index].name, "input")) {
                strcpy(optYuvPath, optarg);
                ChangePathStyle(optYuvPath);
            } else if (!strcmp(options[index].name, "codec")) {
                encConfig.stdMode = (CodStd)atoi(optarg);
            } else if (!strcmp(options[index].name, "cfgFileName")) {
                osal_memcpy(encConfig.cfgFileName, optarg, strlen(optarg));
            } else if (!strcmp(options[index].name, "coreIdx")) {
                encConfig.coreIdx = atoi(optarg);
            } else if (!strcmp(options[index].name, "picWidth")) {
                encConfig.picWidth = atoi(optarg);
            } else if (!strcmp(options[index].name, "picHeight")) {
                encConfig.picHeight = atoi(optarg);
            } else if (!strcmp(options[index].name, "kbps")) {
                encConfig.kbps = atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-ringBuffer")) {
                encConfig.ringBufferEnable = TRUE;
            } else if (!strcmp(options[index].name, "enable-lineBufInt")) {
                encConfig.lineBufIntEn = TRUE;
            } else if (!strcmp(options[index].name, "loop-count")) {
                encConfig.loopCount = atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-cbcrInterleave")) {
                encConfig.cbcrInterleave = 1;
            } else if (!strcmp(options[index].name, "nv21")) {
                encConfig.nv21 = atoi(optarg);
            } else if (!strcmp(options[index].name, "packedFormat")) {
                encConfig.packedFormat = atoi(optarg);
            } else if (!strcmp(options[index].name, "rotAngle")) {
                encConfig.rotAngle = atoi(optarg);
            } else if (!strcmp(options[index].name, "mirDir")) {
                encConfig.mirDir = atoi(optarg);
            } else if (!strcmp(options[index].name, "secondary-axi")) {
                encConfig.secondary_axi = atoi(optarg);
            } else if (!strcmp(options[index].name, "frame-endian")) {
                encConfig.frame_endian = atoi(optarg);
            } else if (!strcmp(options[index].name, "stream-endian")) {
                encConfig.stream_endian = atoi(optarg);
            } else if (!strcmp(options[index].name, "source-endian")) {
                encConfig.source_endian = atoi(optarg);
            } else if (!strcmp(options[index].name, "ref_stream_path")) {
                osal_memcpy(encConfig.ref_stream_path, optarg, strlen(optarg));
                ChangePathStyle(encConfig.ref_stream_path);
            } else if (!strcmp(options[index].name, "srcFormat3p4b")) {
                encConfig.srcFormat3p4b = atoi(optarg);

            } else {
                VLOG(ERR, "not exist param = %s\n", options[index].name);
                help(options_help, argv[0]);
                return 1;
            }
            break;
        default:
            help(options_help, argv[0]);
            return 1;
        }
    }



    productId = VPU_GetProductId(encConfig.coreIdx);

    if (checkParamRestriction(productId, &encConfig) == FALSE)
        return 1;

    switch (productId) {
    case PRODUCT_ID_420:    fwPath = CORE_0_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_420L:   fwPath = CORE_5_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_520:    fwPath = CORE_9_BIT_CODE_FILE_PATH; break;
    default:
        VLOG(ERR, "Unknown product id: %d\n", productId);
        return 1;
    }
    VLOG(INFO, "FW PATH = %s\n", fwPath);
    if (LoadFirmware(productId, (Uint8**)&pusBitCode, &sizeInWord, fwPath) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, fwPath);
        return 1;
    }

    if (showVersion == TRUE || debugMode == TRUE) {
        Uint32   ver, rev;

        ret = VPU_InitWithBitcode(encConfig.coreIdx, (const Uint16*)pusBitCode, sizeInWord);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to boot up VPU(coreIdx: %d, productId: %d)\n", 
                __FUNCTION__, __LINE__, encConfig.coreIdx, productId);
            osal_free(pusBitCode);
            return 1;
        }
        VPU_GetVersionInfo(encConfig.coreIdx, &ver, &rev, &productId);
        printf("VERSION=%d\n", ver);
        printf("REVISION=%d\n", rev);
        printf("PRODUCT_ID=%d\n", productId);
        if (showVersion == TRUE) {
            osal_free(pusBitCode);
            VPU_DeInit(encConfig.coreIdx);
            return 0;
        }
    }

    osal_init_keyboard();

    InitializeDebugEnv(productId, encConfig.testEnvOptions);

    ret = TestEncoder(&encConfig);

    ReleaseDebugEnv();

    if (debugMode == TRUE) {
        VPU_DeInit(encConfig.coreIdx);
    }

    osal_close_keyboard();

    osal_free(pusBitCode);

    return ret == 1 ? 0 : 1;
}
 
