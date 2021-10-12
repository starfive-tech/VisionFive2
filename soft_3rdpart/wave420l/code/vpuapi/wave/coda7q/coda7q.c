#include "product.h"
#include "wave/common/common.h"
#include "wave/common/common_vpuconfig.h"
#include "wave/common/common_regdefine.h"
#include "wave/coda7q/coda7q.h"
#include "vpuerror.h"
#include "wave/coda7q/coda7q_regdefine.h"

#define CODA7Q_DEFAULT_TEMPBUF_SIZE          1024*1024
#define CODA7Q_TEMPBUF_OFFSET                1024*1024

#define OPT_ENC_PIC                         (1)
#define OPT_HEADER_ENC                      (1<<16)

static RetCode Coda7qPrepareDecodingPicture(CodecInst* instance, Uint32 cmd)
{
    Int32           coreIdx;
    Uint32          regVal = 0, bsParam, bsOption;
    DecInfo*        pDecInfo;
    DecOpenParam*   pOpenParam;
    vpu_buffer_t    vb;

    coreIdx    = instance->coreIdx;
    pDecInfo   = VPU_HANDLE_TO_DECINFO(instance);

    pOpenParam = &pDecInfo->openParam;

    /* Set attributes of bitstream buffer controller */
    bsParam = bsOption = 0;
    regVal = 0;
    switch (pOpenParam->bitstreamMode) {
    case BS_MODE_INTERRUPT: 
        bsParam = BSPARAM_ENABLE_RINGBUFFER;
        break;
    case BS_MODE_PIC_END:
        bsOption = BSOPTION_ENABLE_EXPLICIT_END;
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    regVal = vdi_convert_endian(coreIdx, pOpenParam->streamEndian);
    bsParam |= (regVal&VDI_128BIT_ENDIAN_MASK);

    VpuWriteReg(coreIdx, W4_BS_PARAM,      bsParam);
    VpuWriteReg(coreIdx, W4_BS_START_ADDR, pDecInfo->streamBufStartAddr);
    VpuWriteReg(coreIdx, W4_BS_SIZE,       pDecInfo->streamBufSize);
    VpuWriteReg(coreIdx, W4_BS_RD_PTR,     pDecInfo->streamRdPtr);
    VpuWriteReg(coreIdx, W4_BS_WR_PTR,     pDecInfo->streamWrPtr);

    if (pDecInfo->streamEndflag == 1)
        bsOption = 3;
    VpuWriteReg(coreIdx, W4_BS_OPTION,     bsOption);

    /* Secondary AXI */
    vdi_get_sram_memory(coreIdx, &vb);
    VpuWriteReg(coreIdx, W4_ADDR_SEC_AXI, vb.phys_addr);
    VpuWriteReg(coreIdx, W4_SEC_AXI_SIZE, pDecInfo->secAxiInfo.bufSize);

    if (instance->codecMode == C7_HEVC_DEC) {
        regVal = (pDecInfo->secAxiInfo.u.wave4.useBitEnable<<0)    |
                 (pDecInfo->secAxiInfo.u.wave4.useIpEnable<<9)     |
                 (pDecInfo->secAxiInfo.u.wave4.useLfRowEnable<<15);
    }
    else {
        regVal = ((pDecInfo->secAxiInfo.u.coda9.useBitEnable&0x01) <<0    |
                 (pDecInfo->secAxiInfo.u.coda9.useIpEnable&0x01) <<1     |
                 (pDecInfo->secAxiInfo.u.coda9.useDbkYEnable&0x01) <<2   |
                 (pDecInfo->secAxiInfo.u.coda9.useDbkCEnable&0x01) <<3   |
                 (pDecInfo->secAxiInfo.u.coda9.useOvlEnable&0x01) <<4    |
                 (pDecInfo->secAxiInfo.u.coda9.useBitEnable&0x01) <<8    |
                 (pDecInfo->secAxiInfo.u.coda9.useIpEnable&0x01) <<9     |
                 (pDecInfo->secAxiInfo.u.coda9.useDbkYEnable&0x01) <<10  |
                 (pDecInfo->secAxiInfo.u.coda9.useDbkCEnable&0x01) <<11  |
                 (pDecInfo->secAxiInfo.u.coda9.useOvlEnable&0x01) <<12 );
    }
    
    VpuWriteReg(coreIdx, W4_USE_SEC_AXI,  regVal);

    /* Set up work-buffer */
    VpuWriteReg(coreIdx, W4_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
    VpuWriteReg(coreIdx, W4_WORK_SIZE,      pDecInfo->vbWork.size);
    VpuWriteReg(coreIdx, W4_WORK_PARAM,     0);

    /* Set up temp-buffer */
    VpuWriteReg(coreIdx, W4_ADDR_TEMP_BASE, pDecInfo->vbTemp.phys_addr);
    VpuWriteReg(coreIdx, W4_TEMP_SIZE,      pDecInfo->vbTemp.size);
    VpuWriteReg(coreIdx, W4_TEMP_PARAM,     0);

    {
        PhysicalAddress addr;
        Uint32          size;

        addr = pDecInfo->userDataBufAddr;
        size = pDecInfo->userDataBufSize;
        if (pDecInfo->userDataEnable == FALSE) {
            addr = 0L;
            size = 0L;
        }
        /* Set attributes of User buffer */
        VpuWriteReg(coreIdx, W4_CMD_DEC_ADDR_USER_BASE, addr);
        VpuWriteReg(coreIdx, W4_CMD_DEC_USER_SIZE,      size);
        VpuWriteReg(coreIdx, W4_CMD_DEC_USER_PARAM,     VDI_128BIT_LITTLE_ENDIAN&VDI_128BIT_ENDIAN_MASK);

        /** Configure CU data report */
        addr = pDecInfo->cuDataBufAddr;
        size = pDecInfo->cuDataBufSize;
        if (pDecInfo->cuDataEnable == FALSE) {
            addr = 0L;
            size = 0L;
        }
        VpuWriteReg(coreIdx, W4_CMD_DEC_ADDR_REPORT_BASE, addr);
        VpuWriteReg(coreIdx, W4_CMD_DEC_REPORT_SIZE,      size);
        VpuWriteReg(coreIdx, W4_CMD_DEC_REPORT_PARAM,     (pDecInfo->cuDataEnable<<31 | (VDI_128BIT_LE_WORD_BYTE_SWAP&VDI_128BIT_ENDIAN_MASK)));
    }

    return RETCODE_SUCCESS;
}

static void Coda7qGetSequenceInformation(CodecInst* instance, DecInitialInfo* info)
{
    DecInfo*    pDecInfo   = &instance->CodecInfo->decInfo;
    Uint32      regVal;
    Uint32      left, right, top, bottom;
    Uint32      profileCompatibilityFlag;

    info->rdPtr = VpuReadReg(instance->coreIdx, W4_BS_RD_PTR);
    info->wrPtr = VpuReadReg(instance->coreIdx, W4_BS_WR_PTR);

    pDecInfo->streamRdPtr      = VpuReadReg(instance->coreIdx, pDecInfo->streamRdPtrRegAddr);   
    pDecInfo->frameDisplayFlag = VpuReadReg(instance->coreIdx, pDecInfo->frameDisplayFlagRegAddr);  
    regVal = VpuReadReg(instance->coreIdx, W4_BS_OPTION); 
    pDecInfo->streamEndflag    = (regVal&0x02) ? TRUE : FALSE;

    regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_SIZE);
    info->picWidth            = ( (regVal >> 16) & 0xffff );
    info->picHeight           = ( regVal & 0xffff );

    if (instance->codecMode == C7_HEVC_DEC) {
        regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_CROP_LEFT_RIGHT);
        left   = (regVal >> 16) & 0xffff;
        right  = regVal & 0xffff;
        regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_CROP_TOP_BOTTOM);
        top    = (regVal >> 16) & 0xffff;
        bottom = regVal & 0xffff;

        info->picCropRect.left   = left;
        info->picCropRect.right  = info->picWidth - right;
        info->picCropRect.top    = top;
        info->picCropRect.bottom = info->picHeight - bottom;

        info->minFrameBufferCount = VpuReadReg(instance->coreIdx, C7_RET_DEC_FRAMEBUF_NEEDED);
        info->frameBufDelay       = VpuReadReg(instance->coreIdx, C7_RET_DEC_NUM_REORDER_DELAY);
        regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_SEQ_PARAM);
        profileCompatibilityFlag     = (regVal>>12)&0xff;
        info->profile                = (regVal >> 24)&0x1f;
        info->level                  = regVal & 0xff;
        info->tier                   = (regVal >> 29)&0x01;
        info->maxSubLayers           = (regVal >> 21)&0x07;
        info->fRateNumerator         = VpuReadReg(instance->coreIdx, C7_RET_DEC_FRAME_RATE_NR);
        info->fRateDenominator       = VpuReadReg(instance->coreIdx, C7_RET_DEC_FRAME_RATE_DR);
        regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_COLOR_SAMPLE_INFO);
        info->chromaFormatIDC        = (regVal>>8)&0x0f;
        info->lumaBitdepth           = (regVal>>0)&0x0f;
        info->chromaBitdepth         = (regVal>>4)&0x0f;
        info->aspectRateInfo         = (regVal>>16)&0xff;
        info->isExtSAR               = (info->aspectRateInfo == 255 ? TRUE : FALSE);
        if (info->isExtSAR == TRUE) {
            info->aspectRateInfo     = VpuReadReg(instance->coreIdx, C7_RET_DEC_ASPECT_RATIO);  /* [0:15] - vertical size, [16:31] - horizontal size */
        }
        info->bitRate                = VpuReadReg(instance->coreIdx, C7_RET_DEC_BIT_RATE);

        /* Guessing Profile */
        if (info->profile == 0) {
            if ((profileCompatibilityFlag&0x06) == 0x06)        info->profile = 1;      /* Main profile */
            else if ((profileCompatibilityFlag&0x04) == 0x04)   info->profile = 2;      /* Main10 profile */
            else if ((profileCompatibilityFlag&0x08) == 0x08)   info->profile = 3;      /* Main Still Picture profile */
            else                                                info->profile = 1;      /* For old version HM */
        }
    }
    else {  
        //---------------------------------------------------
        // for Legacy codecs
        //---------------------------------------------------
        info->lumaBitdepth           = 8;
        info->chromaBitdepth         = 8;
        if (instance->codecMode == C7_AVC_DEC) {
            info->numReorderFrames = VpuReadReg(instance->coreIdx, C7_RET_DEC_SEQ_VUI_INFO);
            regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_CROP_LEFT_RIGHT);
            left   = (regVal >> 16) & 0xffff;
            right  = regVal & 0xffff;
            regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_CROP_TOP_BOTTOM);
            top    = (regVal >> 16) & 0xffff;
            bottom = regVal & 0xffff;

            info->picCropRect.left   = left;
            info->picCropRect.right  = info->picWidth - right;
            info->picCropRect.top    = top;
            info->picCropRect.bottom = info->picHeight - bottom;
        }

        info->aspectRateInfo         = VpuReadReg(instance->coreIdx, C7_RET_DEC_ASPECT_RATIO);
        info->fRateNumerator         = VpuReadReg(instance->coreIdx, C7_RET_DEC_FRAME_RATE_NR);
        info->fRateDenominator       = VpuReadReg(instance->coreIdx, C7_RET_DEC_FRAME_RATE_DR);

        if (instance->codecMode == C7_VP8_DEC && instance->codecModeAux == VPX_AUX_VP8) {
            regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_SEQ_VP8_SCALE_INFO);
            info->vp8ScaleInfo.hScaleFactor = (regVal >> 30) & 0x03;
            info->vp8ScaleInfo.vScaleFactor = (regVal >> 28) & 0x03;
            info->vp8ScaleInfo.picWidth     = (regVal >> 14) & 0x3FFF;
            info->vp8ScaleInfo.picHeight    = (regVal >> 0)  & 0x3FFF;
        }
    }
}

RetCode Coda7qVpuDecInitSeq(CodecInst* instance)
{
    RetCode     ret;
    DecInfo*    pDecInfo;
    Uint32      cmdOption = INIT_SEQ_NORMAL;
    Uint32      val;
    vpu_buffer_t vb;

    if (instance == NULL) 
        return RETCODE_INVALID_PARAM;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);
    vdi_get_common_memory(instance->coreIdx, &vb);
    pDecInfo->vbTemp.phys_addr = vb.phys_addr + CODA7Q_TEMPBUF_OFFSET;
    pDecInfo->vbTemp.virt_addr = vb.virt_addr + CODA7Q_TEMPBUF_OFFSET;
    pDecInfo->vbTemp.size      = CODA7Q_DEFAULT_TEMPBUF_SIZE;
    if (pDecInfo->thumbnailMode)
        cmdOption = INIT_SEQ_W_THUMBNAIL;

    if ((ret=Coda7qPrepareDecodingPicture(instance, DEC_PIC_HDR)) != RETCODE_SUCCESS) {
        return ret;
    }

    if (instance->codecMode == C7_HEVC_DEC) {
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_USER_DATA_MASK, 0xFFFFFFFF);    // fix me.
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEVERITY_LEVEL,  0);
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_FORCE_FB_LATENCY_PLUS1, 0);
    }
    else {  // for Legacy codecs
        if (pDecInfo->userDataEnable) {
            VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEQ_USER_DATA_OPTION, (pDecInfo->userDataReportMode << 10) | (pDecInfo->userDataEnable << 5));
            VpuWriteReg(instance->coreIdx, W4_CMD_DEC_ADDR_USER_BASE,       pDecInfo->userDataBufAddr);
            VpuWriteReg(instance->coreIdx, W4_CMD_DEC_USER_SIZE,            pDecInfo->userDataBufSize);
        }
        else {
            VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEQ_USER_DATA_OPTION, 0);
            VpuWriteReg(instance->coreIdx, W4_CMD_DEC_ADDR_USER_BASE,       0);
            VpuWriteReg(instance->coreIdx, W4_CMD_DEC_USER_SIZE,            0);
        }

        // set reorderEnable && DeblkEnable flag
        val = 0;
        if (pDecInfo->openParam.bitstreamMode == BS_MODE_PIC_END) {
            val |= (1 << 3) & 0x8;
            val |= (1 << 2) & 0x4;
        }

        if (!pDecInfo->lowDelayInfo.lowDelayEn) {
            val |= (pDecInfo->reorderEnable<<1) & 0x2;
        }

        val |= (pDecInfo->openParam.mp4DeblkEnable & 0x1);  
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEQ_OPTION, val);  

        // removed CMD_DEC_SEQ_START_BYTE setting.
        if (instance->codecMode == C7_DV3_DEC)
            VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEQ_SRC_SIZE, (pDecInfo->div3Width<<16) | (pDecInfo->div3Height));

        switch (instance->codecMode) {
        case C7_MP4_DEC:
            VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEQ_MP4_ASP_CLASS, (CODA7Q_VPU_GMC_PROCESS_METHOD<<3)|pDecInfo->openParam.mp4Class);
            break;
        case C7_VC1_DEC:
            VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEQ_VC1_STREAM_FMT, (0 << 3) & 0x08);
            break;
        case C7_AVC_DEC:
            VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEQ_X264_MV_EN, CODA7Q_VPU_AVC_X264_SUPPORT);
            break;
        }
    }

    VpuWriteReg(instance->coreIdx, W4_CMD_DEC_DISP_FLAG, 0);
    VpuWriteReg(instance->coreIdx, W4_COMMAND_OPTION, cmdOption);  /* INIT_SEQ */
    
    Wave4BitIssueCommand(instance, DEC_PIC_HDR);

    return RETCODE_SUCCESS;
}

RetCode Coda7qVpuDecode(CodecInst* instance, DecParam* option)
{
    RetCode ret;
    Uint32      modeOption = DEC_PIC_NORMAL;
    Int32       forceLatency = -1; 

    if (instance->CodecInfo->decInfo.thumbnailMode)
        modeOption = DEC_PIC_W_THUMBNAIL;
    else if (option->skipframeMode)
    {
        switch (option->skipframeMode) {
        case 1:
            modeOption = SKIP_NON_IRAP;
            forceLatency = 0;
            break;
        case 3:
            modeOption = SKIP_NON_REF_PIC;
            break;
        default:
            break;
        }
    }

    if ((ret=Coda7qPrepareDecodingPicture(instance, DEC_PIC)) != RETCODE_SUCCESS) {
        return ret;
    }   


    if (instance->codecMode == C7_HEVC_DEC) {
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_USER_DATA_MASK, 0xFFFFFFFF);    // fix me.
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_TEMPORAL_ID, 7);
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_FORCE_FB_LATENCY_PLUS1, forceLatency+1);
        VpuWriteReg(instance->coreIdx, C7_CMD_DEC_SEVERITY_LEVEL,  0);
        VpuWriteReg(instance->coreIdx, C7_CMD_SEQ_CHANGE_ENABLE_FLAG, instance->CodecInfo->decInfo.seqChangeMask);
    }
    VpuWriteReg(instance->coreIdx, W4_COMMAND_OPTION, modeOption);
    
    Wave4BitIssueCommand(instance, DEC_PIC);

    return RETCODE_SUCCESS;
}

RetCode Coda7qVpuDecGetResult(CodecInst* instance, DecOutputInfo* result)
{
    DecInfo* pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    int regVal, nalUnitType, index;

    if (instance->codecMode == C7_HEVC_DEC) {
        result->frameCycle        = VpuReadReg(instance->coreIdx, W4_FRAME_CYCLE);
        result->decodingSuccess   = VpuReadReg(instance->coreIdx, W4_RET_SUCCESS);
        result->refMissingFrameFlag = FALSE;
        if (result->decodingSuccess == FALSE) {
            result->errorReason = VpuReadReg(instance->coreIdx, W4_RET_FAIL_REASON);
            if (result->errorReason == WAVE4_SYSERR_WRITEPROTECTION) {
                return RETCODE_MEMORY_ACCESS_VIOLATION;
            }
            else if (result->errorReason == WAVE4_CODEC_ERROR) {
                result->errorReasonExt = VpuReadReg(instance->coreIdx, C7_RET_DEC_ERR_INFO);
            }
            if (result->errorReasonExt == WAVE4_ETCERR_MISSING_REFERENCE_PICTURE) {
                result->refMissingFrameFlag = TRUE;
            }
        }
        else {
            result->warnInfo = VpuReadReg(instance->coreIdx, C7_RET_DEC_WARN_INFO);
        }

        regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_TYPE);
 
        if (regVal&0x04)      result->picType = PIC_TYPE_B;
        else if (regVal&0x02) result->picType = PIC_TYPE_P;
        else if (regVal&0x01) result->picType = PIC_TYPE_I;
        else                  result->picType = PIC_TYPE_MAX;
 
        nalUnitType = (regVal & 0x3f0) >> 4;
        if ((nalUnitType == 19 || nalUnitType == 20) && result->picType == PIC_TYPE_I) {
            /* IDR_W_RADL, IDR_N_LP */
            result->picType = PIC_TYPE_IDR;
        }
        result->nalType                   = nalUnitType;
        result->ctuSize                   = 16<<((regVal>>10)&0x3);
        if (pDecInfo->openParam.wtlEnable == TRUE) {
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DISPLAY_IDX);
            result->indexFrameDisplay         = (short)(index>>16 & 0xffff);    // linear
            result->indexFrameDisplayForTiled = (short)(index&0xffff);           // fbc
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DECODED_IDX);
            result->indexFrameDecoded         = (short)(index>>16&0xffff); 
            result->indexFrameDecodedForTiled = (short)(index&0xffff); 
        } else {
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DISPLAY_IDX);
            result->indexFrameDisplay         = (short)(index & 0xffff);
            result->indexFrameDisplayForTiled = result->indexFrameDisplay;
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DECODED_IDX);
            result->indexFrameDecoded         = (short)(index&0xffff); 
            result->indexFrameDecodedForTiled = result->indexFrameDecoded;
        }

        result->h265Info.decodedPOC = -1;
        result->h265Info.displayPOC = -1;
        if (result->indexFrameDecoded >= 0) 
            result->h265Info.decodedPOC = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_POC);

        result->sequenceChanged   = VpuReadReg(instance->coreIdx, C7_RET_DEC_SEQ_CHANGE_FLAG);
        /*
        * If current picture is the last of the current sequence and sequence-change flag is not 0, then
        * the width and height of the current picture is set to the width and height of the current sequence.
        */
        if (result->sequenceChanged == 0) {
            regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_SIZE);
            result->decPicWidth   = regVal>>16;
            result->decPicHeight  = regVal&0xffff;
        }
        else {
            if (result->indexFrameDecoded < 0) {
                result->decPicWidth   = 0;
                result->decPicHeight  = 0;
            } 
            else {
                result->decPicWidth   = pDecInfo->initialInfo.picWidth;
                result->decPicHeight  = pDecInfo->initialInfo.picHeight;
            }
        }

        result->numOfErrMBs       = VpuReadReg(instance->coreIdx, C7_RET_DEC_ERR_CTB_NUM)>>16;
        result->numOfTotMBs       = VpuReadReg(instance->coreIdx, C7_RET_DEC_ERR_CTB_NUM)&0xffff;
        result->bytePosFrameStart = VpuReadReg(instance->coreIdx, C7_RET_DEC_AU_START_POS);
        result->bytePosFrameEnd   = VpuReadReg(instance->coreIdx, C7_RET_DEC_AU_END_POS);
        pDecInfo->prevFrameEndPos = result->bytePosFrameEnd;

        if (result->indexFrameDecodedForTiled == -1 && pDecInfo->openParam.bitstreamMode == BS_MODE_PIC_END) {
            result->chunkReuseRequired = 1;
        }

        result->indexFramePrescan = VpuReadReg(instance->coreIdx, C7_RET_DEC_PRESCAN_INDEX);
    } 
    else {  
        //***********************************************
        //    for Legacy codecs
        //***********************************************
        result->frameCycle        = VpuReadReg(instance->coreIdx, W4_FRAME_CYCLE);  
        regVal                    = VpuReadReg(instance->coreIdx, W4_RET_SUCCESS);
        result->decodingSuccess   = regVal;
        if (result->decodingSuccess == FALSE) {
            result->errorReason = VpuReadReg(instance->coreIdx, W4_RET_FAIL_REASON);
            if (result->errorReason == WAVE4_SYSERR_WRITEPROTECTION) {
                return RETCODE_MEMORY_ACCESS_VIOLATION;
            }
            return RETCODE_FAILURE;
        }

        if (instance->codecMode == C7_AVC_DEC) {
            result->notSufficientPsBuffer   = (regVal >> 3) & 0x1;
            result->notSufficientSliceBuffer= (regVal >> 2) & 0x1;
        }

        result->chunkReuseRequired  = 0;
        if (pDecInfo->openParam.bitstreamMode == BS_MODE_PIC_END) {
            if (instance->codecMode == C7_AVC_DEC) {
                result->chunkReuseRequired  = (regVal >> 16) & 0x1;
                regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DECODED_IDX);
                if (regVal == (Uint32)-1)
                    result->chunkReuseRequired = 1;
            }
            else if (instance->codecMode == C7_MP2_DEC) {
                result->chunkReuseRequired  = (regVal >> 16) & 0x1;
            }
            else if (instance->codecMode == C7_MP4_DEC) {
                result->chunkReuseRequired  = (regVal >> 16) & 0x1;
            }
        }

        if (pDecInfo->openParam.wtlEnable == TRUE) {    // case of H.264 && MbsOnlyFlag==1 
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DISPLAY_IDX);
            result->indexFrameDisplay         = (short)(index>>16 & 0xffff);    // linear
            result->indexFrameDisplayForTiled = (short)(index&0xffff);           // fbc
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DECODED_IDX);
            result->indexFrameDecoded         = (short)(index>>16&0xffff); 
            result->indexFrameDecodedForTiled = (short)(index&0xffff); 
        } 
        else {
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DISPLAY_IDX);
            result->indexFrameDisplay         = (short)(index & 0xffff);
            result->indexFrameDisplayForTiled = result->indexFrameDisplay;
            index                             = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_DECODED_IDX);
            result->indexFrameDecoded         = (short)(index&0xffff); 
            result->indexFrameDecodedForTiled = result->indexFrameDecoded;
        }
        
        regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_SIZE);
        result->decPicWidth = (regVal >> 16) & 0xFFFF;
        result->decPicHeight= (regVal) & 0xFFFF;

        if (result->indexFrameDecoded >= 0 && result->indexFrameDecoded < MAX_GDI_IDX) {
            if (instance->codecMode == C7_VP8_DEC) {
                regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_VP8_SCALE_INFO);  // fix me (not defined yet)
                result->vp8ScaleInfo.hScaleFactor   = (regVal >> 30) & 0x03;
                result->vp8ScaleInfo.vScaleFactor   = (regVal >> 28) & 0x03;
                result->vp8ScaleInfo.picWidth       = (regVal >> 14) & 0x3FFF;
                result->vp8ScaleInfo.picHeight      = (regVal >> 0)  & 0x3FFF;

                // ref_idx_gold[31:24], ref_idx_altr[23:16], ref_idx_last[15: 8],
                // version_number[3:1], show_frame[0]
                regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_VP8_PIC_REPORT);
                result->vp8PicInfo.refIdxGold       = (regVal >> 24) & 0x0FF;
                result->vp8PicInfo.refIdxAltr       = (regVal >> 16) & 0x0FF;
                result->vp8PicInfo.refIdxLast       = (regVal >> 8)  & 0x0FF;
                result->vp8PicInfo.versionNumber    = (regVal >> 1)  & 0x07;
                result->vp8PicInfo.showFrame        = (regVal >> 0)  & 0x01;
            }
        }

        regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_TYPE);
        result->interlacedFrame     = (regVal >> 18) & 0x1;
        result->topFieldFirst       = (regVal >> 21) & 0x1;
        if (result->interlacedFrame) {
            if (instance->codecMode == C7_VC1_DEC) {
                result->picTypeFirst    = (regVal >> 3) & 0x3;
                result->picType         = regVal & 0x3;             // pic_type of 2nd field
            }
            else {
                index = (regVal >> 2) & 0x7;                        // pic_type of 1st field
                if (index & 0x1) {
                    result->picTypeFirst    = PIC_TYPE_I;
                }
                if (index & 0x2) {
                    result->picTypeFirst    = PIC_TYPE_P;
                }
                if (index & 0x4) {
                    result->picTypeFirst    = PIC_TYPE_B;
                }

                index = (regVal >> 5) & 0x7;                        // pic_type of 2nd field
                if (index & 0x1) {
                    result->picType         = PIC_TYPE_I;
                }
                if (index & 0x2) {
                    result->picType         = PIC_TYPE_P;
                }
                if (index & 0x4) {
                    result->picType         = PIC_TYPE_B;
                }
            }
            if (result->topFieldFirst) {
                result->picType = result->picTypeFirst;
            }
        }
        else {
            result->picTypeFirst    = PIC_TYPE_MAX;
            if (instance->codecMode == C7_VC1_DEC)
                result->picType     = (regVal >> 3) & 0x3;
            else
                result->picType     = regVal & 0x3;
        }

        result->pictureStructure    = (regVal >> 19) & 0x3;
        result->repeatFirstField    = (regVal >> 22) & 0x1;
        result->progressiveFrame    = (regVal >> 23) & 0x3;

        if (instance->codecMode == C7_AVC_DEC) {
            result->nalRefIdc       = (regVal >> 7)  & 0x3;
            result->picStrPresent   = (regVal >> 27) & 0x1;
            result->picTimingStruct = (regVal >> 28) & 0xF;
            result->decFrameInfo    = (regVal >> 15) & 0x1;
            result->avcNpfFieldInfo = (regVal >> 16) & 0x3;

            regVal  = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_HRD_INFO);
            result->avcHrdInfo.cpbMinus1        = regVal >> 2;
            result->avcHrdInfo.vclHrdParamFlag  = (regVal>> 1)&1;
            result->avcHrdInfo.nalHrdParamFlag  = regVal & 1;

            regVal  = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_VUI_INFO);
            result->avcVuiInfo.fixedFrameRateFlag    = regVal &1;
            result->avcVuiInfo.timingInfoPresent     = (regVal >> 1)  & 0x01;
            result->avcVuiInfo.chromaLocBotField     = (regVal >> 2)  & 0x07;
            result->avcVuiInfo.chromaLocTopField     = (regVal >> 5)  & 0x07;
            result->avcVuiInfo.chromaLocInfoPresent  = (regVal >> 8)  & 0x01;
            result->avcVuiInfo.colorPrimaries        = (regVal >> 16) & 0xFF;
            result->avcVuiInfo.colorDescPresent      = (regVal >> 24) & 0x01;
            result->avcVuiInfo.isExtSAR              = (regVal >> 25) & 0x01;
            result->avcVuiInfo.vidFullRange          = (regVal >> 26) & 0x01;
            result->avcVuiInfo.vidFormat             = (regVal >> 27) & 0x07;
            result->avcVuiInfo.vidSigTypePresent     = (regVal >> 30) & 0x01;
            result->avcVuiInfo.vuiParamPresent       = (regVal >> 31) & 0x01;

            regVal  = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_VUI_PIC_STRUCT);
            result->avcVuiInfo.vuiPicStructPresent  = regVal & 0x1;
            result->avcVuiInfo.vuiPicStruct         = regVal >> 1;
        }

        if (instance->codecMode == C7_MP2_DEC) {
            result->fieldSequence       = (regVal >> 25) & 0x7;
            result->frameDct            = (regVal >> 28) & 0x1;
            result->progressiveSequence = (regVal >> 29) & 0x1;
            result->decFrameInfo        = (regVal >> 16) & 0x3;

            if (result->indexFrameDisplay >= 0) {
                if (result->indexFrameDisplay == result->indexFrameDecoded) {
                    result->mp2NpfFieldInfo = result->decFrameInfo;
                }
                else {
                    result->mp2NpfFieldInfo = pDecInfo->decOutInfo[result->indexFrameDisplay].decFrameInfo;
                }
            }
        }

        if (instance->codecMode == C7_VC1_DEC) {
            result->decFrameInfo    = (regVal >> 16) & 0x3;
            if (result->indexFrameDisplay >= 0) {
                if (result->indexFrameDisplay == result->indexFrameDecoded) {
                    result->vc1NpfFieldInfo = result->decFrameInfo;
                }
                else {
                    result->vc1NpfFieldInfo = pDecInfo->decOutInfo[result->indexFrameDisplay].decFrameInfo;
                }
            }
        }

        if (instance->codecMode == C7_AVC_DEC) {
            result->fRateNumerator      = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_FRAME_NR);
            result->fRateDenominator    = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_FRAME_DR);

            if (result->fRateDenominator > 0) {
                result->fRateDenominator *= 2;
            }
        }
        else {
            //regVal = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_PIC_SRC_F_RATE);  // fix me (not defined yet)
            result->fRateNumerator      = (regVal & 0xffff);
            result->fRateDenominator    = (regVal==0)?0:((regVal >> 16) + 1) & 0xffff;
        }

        if (instance->codecMode == C7_RVX_DEC) {
            result->rvTr = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_RV_TR); 
            result->rvTrB = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_RV_TR_BFRAME);     
        }

        if (instance->codecMode == C7_VP8_DEC) {
            result->aspectRateInfo = 0;
        }
        else {
            result->aspectRateInfo = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_ASPECT);
        }

        //*******************************************
        // user data reporting
        //*******************************************
        if (pDecInfo->userDataEnable == TRUE) {
            Uint32  userDataNum;
            Uint32  userDataSize;
            BYTE    tempBuf[8] = {0,};

            VpuReadMem(instance->coreIdx, pDecInfo->userDataBufAddr + 0, tempBuf, 8,  VPU_USER_DATA_ENDIAN); 

            regVal = ((tempBuf[0]<<24) & 0xFF000000) |
                     ((tempBuf[1]<<16) & 0x00FF0000) |
                     ((tempBuf[2]<< 8) & 0x0000FF00) |
                     ((tempBuf[3]<< 0) & 0x000000FF);

            userDataNum     = (regVal >> 16) & 0xFFFF;
            userDataSize    = (regVal >> 0) & 0xFFFF;
            if (userDataNum == 0) {
                userDataSize = 0;
            }

            result->decOutputExtData.userDataNum = userDataNum;
            result->decOutputExtData.userDataSize = userDataSize;

            regVal = ((tempBuf[4]<<24) & 0xFF000000) |
                     ((tempBuf[5]<<16) & 0x00FF0000) |
                     ((tempBuf[6]<< 8) & 0x0000FF00) |
                     ((tempBuf[7]<< 0) & 0x000000FF);

            if (userDataNum == 0) {
                result->decOutputExtData.userDataBufFull = 0;
            }
            else {
                result->decOutputExtData.userDataBufFull = (regVal >> 16) & 0xFFFF;
            }
        }

        result->numOfErrMBs     = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_ERR_MB);

        if (instance->codecMode == C7_VC1_DEC && result->indexFrameDisplay != -3) {
            if (pDecInfo->vc1BframeDisplayValid == 0) {
                if (result->picType == 2) {
                    result->indexFrameDisplay   = -3;
                }
                else {
                    pDecInfo->vc1BframeDisplayValid = 1;
                }
            }
        }

        if (instance->codecMode == C7_AVC_DEC && instance->codecModeAux == AVC_AUX_MVC) {
            regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_MVC_REPORT);
            result->mvcPicInfo.viewIdxDisplay   = (regVal >> 0) & 0x1;
            result->mvcPicInfo.viewIdxDecoded   = (regVal >> 1) & 0x1;
        }

        if (instance->codecMode == C7_AVC_DEC) {
            regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_AVC_FPA_SEI0);

            if (regVal < 0) {
                result->avcFpaSei.exist = 0;
            } 
            else {
                result->avcFpaSei.exist = 1;
                result->avcFpaSei.framePackingArrangementId = regVal;

                regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_AVC_FPA_SEI1);
                result->avcFpaSei.contentInterpretationType               =  regVal&0x3F; // [5:0]
                result->avcFpaSei.framePackingArrangementType             = (regVal >> 6)&0x7F; // [12:6]
                result->avcFpaSei.framePackingArrangementExtensionFlag    = (regVal >> 13)&0x01; // [13]
                result->avcFpaSei.frame1SelfContainedFlag                 = (regVal >> 14)&0x01; // [14]
                result->avcFpaSei.frame0SelfContainedFlag                 = (regVal >> 15)&0x01; // [15]
                result->avcFpaSei.currentFrameIsFrame0Flag                = (regVal >> 16)&0x01; // [16]
                result->avcFpaSei.fieldViewsFlag                          = (regVal >> 17)&0x01; // [17]
                result->avcFpaSei.frame0FlippedFlag                       = (regVal >> 18)&0x01; // [18]
                result->avcFpaSei.spatialFlippingFlag                     = (regVal >> 19)&0x01; // [19]
                result->avcFpaSei.quincunxSamplingFlag                    = (regVal >> 20)&0x01; // [20]
                result->avcFpaSei.framePackingArrangementCancelFlag       = (regVal >> 21)&0x01; // [21]

                regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_AVC_FPA_SEI2);
                result->avcFpaSei.framePackingArrangementRepetitionPeriod =  regVal&0x7FFF;       // [14:0]
                result->avcFpaSei.frame1GridPositionY                     = (regVal >> 16)&0x0F; // [19:16]
                result->avcFpaSei.frame1GridPositionX                     = (regVal >> 20)&0x0F; // [23:20]
                result->avcFpaSei.frame0GridPositionY                     = (regVal >> 24)&0x0F; // [27:24]
                result->avcFpaSei.frame0GridPositionX                     = (regVal >> 28)&0x0F; // [31:28]
            }      

            regVal = VpuReadReg(instance->coreIdx, C7_RET_DEC_PIC_POC);

            result->avcPocTop = regVal & 0x3FF;
            result->avcPocBot = (regVal >> 10) & 0x3FF;

            if (result->interlacedFrame) {
                if (result->avcPocTop > result->avcPocBot) {
                    result->avcPocPic = result->avcPocBot;
                } 
                else {
                    result->avcPocPic = result->avcPocTop;
                }
            }
            else {
                result->avcPocPic = result->avcPocTop;
            }
        }
#ifdef SUPPORT_REF_FLAG_REPORT
        // regVal = VpuReadReg(pCodecInst->coreIdx, BIT_FRM_REF_FLG);   // fix me (not defined yet)
        for (index = 0; index < MAX_GDI_IDX; index++) {
            result->frameReferenceFlag[index] = ((regVal>>index)&0x01);
        }   
#endif
    }

    Coda7qGetSequenceInformation(instance, &pDecInfo->initialInfo);


    return RETCODE_SUCCESS;
}

RetCode Coda7qVpuDecGetSeqInfo(CodecInst* pCodecInst, DecInitialInfo* info)
{
    DecInfo*   pDecInfo   = &pCodecInst->CodecInfo->decInfo;
    Uint32     regVal;

    if (pDecInfo->openParam.bitstreamMode == BS_MODE_INTERRUPT && pDecInfo->seqInitEscape) {
        pDecInfo->seqInitEscape = 0;
        /* NOTE: Not implemented yet */
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    Coda7qGetSequenceInformation(pCodecInst, info);

    if (pCodecInst->codecMode != C7_HEVC_DEC) {
        
        info->minFrameBufferCount = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_FRAMEBUF_NEEDED);
        info->frameBufDelay       = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_NUM_REORDER_DELAY);

        regVal = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_SEQ_HEADER_REPORT);
        info->profile                =  (regVal >> 0) & 0xFF;
        info->level                  =  (regVal >> 8) & 0xFF;
        info->interlace              = !((regVal >> 16) & 0x01);
        info->direct8x8Flag          =  (regVal >> 17) & 0x01;
        info->vc1Psf                 =  (regVal >> 18) & 0x01;
        info->constraint_set_flag[0] =  (regVal >> 19) & 0x01;
        info->constraint_set_flag[1] =  (regVal >> 20) & 0x01;
        info->constraint_set_flag[2] =  (regVal >> 21) & 0x01;
        info->constraint_set_flag[3] =  (regVal >> 22) & 0x01; 
        info->chromaFormatIDC        =  (regVal >> 23) & 0x03;
        info->isExtSAR               =  (regVal >> 25) & 0x01;
        info->maxNumRefFrm           =  (regVal >> 27) & 0x0f;
        info->maxNumRefFrmFlag       =  (regVal >> 31) & 0x01;

        info->bitRate                = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_BIT_RATE);
        if (pCodecInst->codecMode == C7_MP4_DEC) {
            regVal = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_SEQ_INFO);
            info->mp4ShortVideoHeader       = (regVal >> 2) & 1;
            info->mp4DataPartitionEnable    = (regVal >> 0) & 1;
            info->mp4ReversibleVlcEnable    = info->mp4DataPartitionEnable ? ((regVal>>1) & 1) : 0;
            info->h263AnnexJEnable          = (regVal >> 3) & 1;
        }
    }

    info->warnInfo = 0;
    regVal = VpuReadReg(pCodecInst->coreIdx, W4_RET_SUCCESS);
    if (regVal == FALSE) {
        info->seqInitErrReason = VpuReadReg(pCodecInst->coreIdx, W4_RET_FAIL_REASON);
        if (info->seqInitErrReason == WAVE4_SYSERR_WRITEPROTECTION)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (info->seqInitErrReason == 1) {
            info->seqInitErrReason = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_ERR_INFO);
        }
        return RETCODE_FAILURE;
    }
    else {
        info->warnInfo = VpuReadReg(pCodecInst->coreIdx, C7_RET_DEC_WARN_INFO);
    }

    return RETCODE_SUCCESS;
}

extern RetCode Coda7qVpuEncSetup(CodecInst* instance)
{
    Int32       picWidth, picHeight, int_reason;
    Int32       data;
    Uint32      regVal = 0, rotMirMode;
    EncInfo*    pEncInfo = &instance->CodecInfo->encInfo;

    rotMirMode = 0;

    if (pEncInfo->rotationEnable == TRUE) {
        switch (pEncInfo->rotationAngle) {
        case 0:
            rotMirMode |= 0x0;
            break;
        case 90:
            rotMirMode |= 0x1;
            break;
        case 180:
            rotMirMode |= 0x2;
            break;
        case 270:
            rotMirMode |= 0x3;
            break;
        }
    }

    if (pEncInfo->mirrorEnable == TRUE) {
        switch (pEncInfo->mirrorDirection) {
        case MIRDIR_NONE :
            rotMirMode |= 0x0;
            break;
        case MIRDIR_VER :
            rotMirMode |= 0x4;
            break;
        case MIRDIR_HOR :
            rotMirMode |= 0x8;
            break;
        case MIRDIR_HOR_VER :
            rotMirMode |= 0xC;
            break;
        }
    }

    switch(pEncInfo->openParam.srcFormat) {     // FIX ME. (don't know exactly how srcFormat is set including cbcrInterleave, nv21 options)
        case FORMAT_420:    regVal = 0; break;
        case FORMAT_422:    regVal = 1; break;
        case FORMAT_YUYV:   regVal = 2; break;
        case FORMAT_UYVY:   regVal = 3; break;
    }
    rotMirMode |= (regVal<<5);

    picWidth  = pEncInfo->openParam.picWidth;
    picHeight = pEncInfo->openParam.picHeight;

    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_ROT_PARAM,  rotMirMode);

    /* Set up work-buffer */
    VpuWriteReg(instance->coreIdx, W4_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W4_WORK_SIZE,      pEncInfo->vbWork.size);
    VpuWriteReg(instance->coreIdx, W4_WORK_PARAM,     0);

    pEncInfo->vbTemp.size      = 0;

    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SET_PARAM_OPTION, (1<< C7_OPT_COMMON));

    // Rotation Left 90 or 270 case : Swap XY resolution for VPU internal usage
    if (pEncInfo->rotationAngle == 90 || pEncInfo->rotationAngle == 270)
        data = (picHeight<< 16) | picWidth;
    else
        data = (picWidth << 16) | picHeight;

    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_SRC_SIZE, data);
    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_SRC_F_RATE, pEncInfo->openParam.frameRateInfo);

    if (pEncInfo->openParam.bitstreamFormat == STD_MPEG4) {
        data = pEncInfo->openParam.EncStdParam.mp4Param.mp4IntraDcVlcThr << 2 |
            pEncInfo->openParam.EncStdParam.mp4Param.mp4ReversibleVlcEnable << 1 |
            pEncInfo->openParam.EncStdParam.mp4Param.mp4DataPartitionEnable;

        data |= ((pEncInfo->openParam.EncStdParam.mp4Param.mp4HecEnable >0)? 1:0)<<5;
        data |= ((pEncInfo->openParam.EncStdParam.mp4Param.mp4Verid == 2)? 0:1) << 6;

        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_STD_PARA, data);

    }
    else if (pEncInfo->openParam.bitstreamFormat == STD_H263) {
        data =  (1<<5) |    // short_header enable
                pEncInfo->openParam.EncStdParam.h263Param.h263AnnexIEnable << 3 |
                pEncInfo->openParam.EncStdParam.h263Param.h263AnnexJEnable << 2 |
                pEncInfo->openParam.EncStdParam.h263Param.h263AnnexKEnable << 1|
                pEncInfo->openParam.EncStdParam.h263Param.h263AnnexTEnable;
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_AUX_PARA, data);
    }
    else if (pEncInfo->openParam.bitstreamFormat == STD_AVC) {        
        data =  (pEncInfo->openParam.EncStdParam.avcParam.deblkFilterOffsetBeta & 15) << 12  |
                (pEncInfo->openParam.EncStdParam.avcParam.deblkFilterOffsetAlpha & 15) << 8  |
                pEncInfo->openParam.EncStdParam.avcParam.disableDeblk << 6                   |
                pEncInfo->openParam.EncStdParam.avcParam.constrainedIntraPredFlag << 5       |
                (pEncInfo->openParam.EncStdParam.avcParam.chromaQpOffset & 31);
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_STD_PARA, data);
    }

    data = pEncInfo->openParam.sliceMode.sliceSize << 16 |
        pEncInfo->openParam.sliceMode.sliceSizeMode << 1 |
        pEncInfo->openParam.sliceMode.sliceMode;

    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_SLICE_MODE, data);

    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_GOP_NUM, pEncInfo->openParam.gopSize);
    if (pEncInfo->openParam.rcEnable) { // rate control enabled
        data = (pEncInfo->openParam.frameSkipDisable) << 31  |
            pEncInfo->openParam.initialDelay << 16 | 1;
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_PARAM, data);

        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_RC_TARGET_RATE, pEncInfo->openParam.bitRate);
    }
    else {
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_PARAM, 0);
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_RC_TARGET_RATE, 0);
    }

    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_BUF_SIZE, pEncInfo->openParam.vbvBufferSize);

    //data = pEncInfo->openParam.intraRefresh | pEncInfo->openParam.ConscIntraRefreshEnable<<16;
    
    data = 0;
    if (instance->codecMode == C7_AVC_ENC) 
        data |= (pEncInfo->openParam.EncStdParam.avcParam.audEnable);   // [0] audenable

    if(pEncInfo->openParam.rcIntraQp>=0) {
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_INTRA_PARAM, pEncInfo->openParam.intraRefresh<<16 | (pEncInfo->openParam.rcIntraQp)<<3 );
        data |= (1<<1); // [1] rc_const_intra_qp enable

    }
    else {
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_INTRA_PARAM, pEncInfo->openParam.intraRefresh<<16);
    }
    
    if(pEncInfo->openParam.userQpMax>=0) {
        data |= (1<<2); // [2] rc_qp_max_enable
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_MAX_QP, pEncInfo->openParam.userQpMax<<6);    // fix me (support min ??)
    } 
    else {
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_MAX_QP, 0);
    }

    if(pEncInfo->openParam.userGamma >= 0) {
        data |= (1<<3); // [3] rc_gamma_enable
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_GAMMA, pEncInfo->openParam.userGamma);
    } 
    else {
        VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_GAMMA, 0);
    }


    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_PARAM, data);

    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_RC_INTERVAL_MODE, (pEncInfo->openParam.mbInterval<<2) | pEncInfo->openParam.rcIntervalMode);
    VpuWriteReg(instance->coreIdx, C7_CMD_ENC_SEQ_INTRA_WEIGHT, pEncInfo->openParam.intraCostWeight);

    Wave4BitIssueCommand(instance, SET_PARAM);

    if ((int_reason = vdi_wait_interrupt(instance->coreIdx, VPU_ENC_TIMEOUT, W4_VPU_VINT_REASON)) == -1) {
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, SET_PARAM, 0);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }
    VpuWriteReg(instance->coreIdx, W4_VPU_VINT_REASON_CLR, int_reason);
    VpuWriteReg(instance->coreIdx, W4_VPU_VINT_CLEAR, 1);

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, SET_PARAM, 0);

    if (VpuReadReg(instance->coreIdx, W4_RET_SUCCESS) == 0) {
        if (VpuReadReg(instance->coreIdx, W4_RET_FAIL_REASON) == WAVE4_SYSERR_WRITEPROTECTION) {
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        }
        return RETCODE_FAILURE;
    }

    pEncInfo->streamWrPtr   = VpuReadReg(instance->coreIdx, pEncInfo->streamWrPtrRegAddr);

    return RETCODE_SUCCESS;
}

RetCode Coda7qVpuEncRegisterFramebuffer(CodecInst* instance, FrameBuffer* fbArr, TiledMapType mapType, Uint32 count)
{
    RetCode      ret = RETCODE_SUCCESS;
    EncInfo*     pEncInfo = &instance->CodecInfo->encInfo;
    Int32        q, j, i, remain, idx, bufHeight = 0, bufWidth = 0;
    Int32        coreIdx, startNo, endNo;
    Uint32       regVal, cbcrInterleave=0, nv21=0;
    Uint32       endian, yuvFormat = 0;
    Uint32       addrY, addrCb, addrCr;
    Uint32       mvColSize, fbcYTblSize, fbcCTblSize;
    Uint32       bpp=1;/* byte per pixel */
    int          stride;
    vpu_buffer_t vbBuffer;

    coreIdx        = instance->coreIdx;
    mvColSize      = fbcYTblSize = fbcCTblSize = 0;
    stride         = pEncInfo->stride;

    bufWidth  = VPU_ALIGN8(pEncInfo->openParam.picWidth);
    bufHeight = VPU_ALIGN8(pEncInfo->openParam.picHeight);

    if ((pEncInfo->rotationAngle != 0 || pEncInfo->mirrorDirection != 0) && !(pEncInfo->rotationAngle == 180 && pEncInfo->mirrorDirection == MIRDIR_HOR_VER)) {
        bufWidth  = VPU_ALIGN32(pEncInfo->openParam.picWidth);
        bufHeight = VPU_ALIGN32(pEncInfo->openParam.picHeight);
    }

    if (pEncInfo->rotationAngle == 90 || pEncInfo->rotationAngle == 270) {
        bufWidth  = VPU_ALIGN32(pEncInfo->openParam.picHeight);
        bufHeight = VPU_ALIGN32(pEncInfo->openParam.picWidth);
    }

    if (mapType == COMPRESSED_FRAME_MAP) {
        fbcYTblSize = CODA7Q_FBC_LUMA_TABLE_SIZE(bufWidth, bufHeight);
        fbcYTblSize = VPU_ALIGN16(fbcYTblSize);

        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcYTblSize*count+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(instance->coreIdx, &vbBuffer) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;
        pEncInfo->vbFbcYTbl = vbBuffer;

        fbcCTblSize = CODA7Q_FBC_CHROMA_TABLE_SIZE(bufWidth, bufHeight);
        fbcCTblSize = VPU_ALIGN16(fbcCTblSize);
        
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcCTblSize*count+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(instance->coreIdx, &vbBuffer) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;
        pEncInfo->vbFbcCTbl = vbBuffer;
    } 

    endian = vdi_convert_endian(coreIdx, fbArr[0].endian) & VDI_128BIT_ENDIAN_MASK;

    regVal = (nv21 << 29)                       |
        ((mapType == LINEAR_FRAME_MAP)<<28)     |
        (0 << 24)                               |
        (yuvFormat << 20)                       | 
        (cbcrInterleave << 16)                  |
        // (fbcDisable << 20)                   |       
        (stride*bpp);

    VpuWriteReg(coreIdx, W4_COMMON_PIC_INFO, regVal);

    regVal = (bufWidth<<16) | bufHeight;
    VpuWriteReg(coreIdx, W4_PIC_SIZE, regVal);

    remain = count;
    q      = (remain+7)/8;
    idx    = 0;
    for (j=0; j<q; j++) {
        regVal = (endian<<16) | (j==q-1)<<4 | ((j==0)<<3) ;
        VpuWriteReg(coreIdx, W4_SFB_OPTION, regVal);
        startNo = j*8;
        endNo   = startNo + (remain>=8 ? 8 : remain) - 1;

        VpuWriteReg(coreIdx, W4_SET_FB_NUM, (startNo<<8)|endNo);

        for (i=0; i<8 && i<remain; i++) {
            addrY  = fbArr[i+startNo].bufY;
            addrCb = fbArr[i+startNo].bufCb;
            addrCr = fbArr[i+startNo].bufCr;

            VpuWriteReg(coreIdx, W4_ADDR_LUMA_BASE0  + (i<<4), addrY);
            VpuWriteReg(coreIdx, W4_ADDR_CB_BASE0    + (i<<4), addrCb);
            APIDPRINT("REGISTER FB[%02d] Y(0x%08x), Cb(0x%08x) ", i, addrY, addrCb);
            if (mapType == COMPRESSED_FRAME_MAP) {
                VpuWriteReg(coreIdx, W4_ADDR_FBC_Y_OFFSET0 + (i<<4), pEncInfo->vbFbcYTbl.phys_addr+idx*fbcYTblSize); /* Luma FBC offset table */
                VpuWriteReg(coreIdx, W4_ADDR_FBC_C_OFFSET0 + (i<<4), pEncInfo->vbFbcCTbl.phys_addr+idx*fbcCTblSize); /* Chroma FBC offset table */
                VpuWriteReg(coreIdx, W4_ADDR_MV_COL0  + (i<<2), pEncInfo->vbMV.phys_addr+idx*mvColSize);
                APIDPRINT("Yo(0x%08x) Co(0x%08x), Mv(0x%08x)\n",
                    pEncInfo->vbFbcYTbl.phys_addr+idx*fbcYTblSize,
                    pEncInfo->vbFbcCTbl.phys_addr+idx*fbcCTblSize,
                    pEncInfo->vbMV.phys_addr+idx*mvColSize);
            }
            else {
                VpuWriteReg(coreIdx, W4_ADDR_CR_BASE0 + (i<<4), addrCr);
                VpuWriteReg(coreIdx, W4_ADDR_FBC_C_OFFSET0 + (i<<4), 0);
                VpuWriteReg(coreIdx, W4_ADDR_MV_COL0  + (i<<2), 0);
                APIDPRINT("Cr(0x%08x)\n", addrCr);
            }
            idx++;
        }
        remain -= i;

        VpuWriteReg(coreIdx, W4_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
        VpuWriteReg(coreIdx, W4_WORK_SIZE,      pEncInfo->vbWork.size);
        VpuWriteReg(coreIdx, W4_WORK_PARAM,     0);

        Wave4BitIssueCommand(instance, SET_FRAMEBUF);
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
    }

    regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    if (ConfigSecAXICoda7(coreIdx, pEncInfo->openParam.bitstreamFormat,
        &pEncInfo->secAxiInfo, 
        pEncInfo->openParam.picWidth, pEncInfo->openParam.picHeight, pEncInfo->openParam.EncStdParam.hevcParam.profile) == 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
    }


    return ret;
}

RetCode Coda7qVpuEncGetHeader(EncHandle instance, EncHeaderParam* encHeaderParam)
{
    EncInfo * pEncInfo;
    EncOpenParam *encOP;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;        
    Int32       coreIdx, bsEndian, wrapAroundFlag, regVal, int_reason;
    RetCode         ret;

    pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);
    encOP = &(pEncInfo->openParam);
    coreIdx = instance->coreIdx;

    EnterLock(coreIdx);


    regVal = vdi_convert_endian(coreIdx, encOP->streamEndian);
    bsEndian = (regVal&VDI_128BIT_ENDIAN_MASK);

	if (pEncInfo->ringBufferEnable == 1) {
		VpuWriteReg(coreIdx, W4_BS_START_ADDR, pEncInfo->streamBufStartAddr);
		VpuWriteReg(coreIdx, W4_BS_SIZE,       pEncInfo->streamBufSize);
	}
	else {
		VpuWriteReg(coreIdx, W4_BS_START_ADDR, encHeaderParam->buf);
		VpuWriteReg(coreIdx, W4_BS_SIZE, encHeaderParam->size);
		pEncInfo->streamRdPtr = encHeaderParam->buf;
		pEncInfo->streamWrPtr = encHeaderParam->buf;
		pEncInfo->streamBufStartAddr = encHeaderParam->buf;
		pEncInfo->streamBufSize = encHeaderParam->size;
		pEncInfo->streamBufEndAddr = encHeaderParam->buf + encHeaderParam->size;		
	}

    //[FIX ME!!!!!]
    if (pEncInfo->ringBufferEnable == 1)
        wrapAroundFlag = 0;
    else
        wrapAroundFlag = 1;

    VpuWriteReg(coreIdx, W4_BS_PARAM, (wrapAroundFlag<<4) | bsEndian);
    VpuWriteReg(coreIdx, W4_BS_OPTION, 0);
    /* Set up work-buffer */
    VpuWriteReg(coreIdx, W4_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
    VpuWriteReg(coreIdx, W4_WORK_SIZE,      pEncInfo->vbWork.size);
    VpuWriteReg(coreIdx, W4_WORK_PARAM,     0);

    VpuWriteReg(coreIdx, pEncInfo->streamRdPtrRegAddr, pEncInfo->streamRdPtr);
    VpuWriteReg(coreIdx, pEncInfo->streamWrPtrRegAddr, pEncInfo->streamWrPtr);

    if (encHeaderParam->headerType == SPS_RBSP && pEncInfo->openParam.bitstreamFormat == STD_AVC) {
        if (encOP->EncStdParam.avcParam.frameCroppingFlag == 1) {
            VpuWriteReg(coreIdx, C7_CMD_ENC_CONF_WIN_TOP_BOT, encOP->EncStdParam.avcParam.frameCropTop << 16 | encOP->EncStdParam.avcParam.frameCropBottom);
            VpuWriteReg(coreIdx, C7_CMD_ENC_CONF_WIN_LEFT_RIGHT, encOP->EncStdParam.avcParam.frameCropLeft << 16 | encOP->EncStdParam.avcParam.frameCropRight);
        }
    }
    
    VpuWriteReg(coreIdx, C7_CMD_ENC_CODE_OPTION, (encOP->EncStdParam.avcParam.frameCroppingFlag<<6) | 1<<(encHeaderParam->headerType));	

    VpuWriteReg(coreIdx, C7_CMD_ENC_PIC_OPTION, OPT_HEADER_ENC);    // encode header

    Wave4BitIssueCommand(instance, ENC_PIC);

    if ((int_reason = vdi_wait_interrupt(coreIdx, VPU_ENC_TIMEOUT, W4_VPU_VINT_REASON)) == -1) {
        LeaveLock(coreIdx);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }
    VpuWriteReg(coreIdx, W4_VPU_VINT_REASON_CLR, int_reason);
    VpuWriteReg(coreIdx, W4_VPU_VINT_CLEAR, 1);

    if (instance->loggingEnable)
        vdi_log(coreIdx, ENC_PIC, 0);

    rdPtr = VpuReadReg(coreIdx, pEncInfo->streamRdPtrRegAddr);
    wrPtr = VpuReadReg(coreIdx, pEncInfo->streamWrPtrRegAddr);    
    encHeaderParam->buf  = rdPtr;
    encHeaderParam->size = wrPtr - rdPtr;

    pEncInfo->streamWrPtr = wrPtr;
    pEncInfo->streamRdPtr = rdPtr;

    regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
    if (regVal == 0) {
        encHeaderParam->failReasonCode = VpuReadReg(coreIdx, W4_RET_FAIL_REASON);
        ret = RETCODE_FAILURE;
    }
    else {
        ret = RETCODE_SUCCESS;
    }
    LeaveLock(coreIdx);

    return ret;
}

RetCode Coda7qVpuEncode(CodecInst* instance, EncParam* option)
{
    Int32           coreIdx, srcFrameFormat;
    Uint32          regVal = 0, bsEndian, wrapAroundFlag;
    EncInfo*        pEncInfo;
    FrameBuffer*    pSrcFrame;
    EncOpenParam*   pOpenParam;

    coreIdx     = instance->coreIdx;
    pEncInfo    = VPU_HANDLE_TO_ENCINFO(instance);
    pOpenParam  = &pEncInfo->openParam;
    pSrcFrame   = option->sourceFrame;

	if (pEncInfo->openParam.ringBufferEnable == 1) {
		VpuWriteReg(coreIdx, W4_BS_START_ADDR, pEncInfo->streamBufStartAddr);
		VpuWriteReg(coreIdx, W4_BS_SIZE,       pEncInfo->streamBufSize);
	}
	else {
		VpuWriteReg(coreIdx, W4_BS_START_ADDR, option->picStreamBufferAddr);
		VpuWriteReg(coreIdx, W4_BS_SIZE,	   option->picStreamBufferSize);
		pEncInfo->streamRdPtr = option->picStreamBufferAddr;
		pEncInfo->streamWrPtr = option->picStreamBufferAddr;
		pEncInfo->streamBufStartAddr = option->picStreamBufferAddr;
		pEncInfo->streamBufSize = option->picStreamBufferSize;
		pEncInfo->streamBufEndAddr = option->picStreamBufferAddr + option->picStreamBufferSize;		
	}

    VpuWriteReg(coreIdx, pEncInfo->streamRdPtrRegAddr, pEncInfo->streamRdPtr);
    VpuWriteReg(coreIdx, pEncInfo->streamWrPtrRegAddr, pEncInfo->streamWrPtr);

    //[FIX ME!!!!!]
    if (pEncInfo->ringBufferEnable == 1)
        wrapAroundFlag = 0;  
    else
        wrapAroundFlag = 1;

    regVal = vdi_convert_endian(coreIdx, pOpenParam->streamEndian);
    bsEndian = (regVal&VDI_128BIT_ENDIAN_MASK);

    VpuWriteReg(coreIdx, W4_BS_PARAM, (wrapAroundFlag<<4) | bsEndian);

    /* Secondary AXI */
    VpuWriteReg(coreIdx, W4_ADDR_SEC_AXI, pEncInfo->secAxiInfo.bufBase);
    VpuWriteReg(coreIdx, W4_SEC_AXI_SIZE, pEncInfo->secAxiInfo.bufSize);
    
    VpuWriteReg(coreIdx, W4_USE_SEC_AXI, ((pEncInfo->secAxiInfo.u.coda9.useBitEnable&0x01) <<0    |   // fix me.
                                          (pEncInfo->secAxiInfo.u.coda9.useIpEnable&0x01) <<1     | 
                                          (pEncInfo->secAxiInfo.u.coda9.useDbkYEnable&0x01) <<2   |
                                          (pEncInfo->secAxiInfo.u.coda9.useDbkCEnable&0x01) <<3   |
                                          (pEncInfo->secAxiInfo.u.coda9.useOvlEnable&0x01) <<4    | 
                                          (pEncInfo->secAxiInfo.u.coda9.useMeEnable&0x01) <<5     | 
                                          (pEncInfo->secAxiInfo.u.coda9.useBitEnable&0x01) <<8    |
                                          (pEncInfo->secAxiInfo.u.coda9.useIpEnable&0x01) <<9     |
                                          (pEncInfo->secAxiInfo.u.coda9.useDbkYEnable&0x01) <<10  | 
                                          (pEncInfo->secAxiInfo.u.coda9.useDbkCEnable&0x01) <<11  | 
                                          (pEncInfo->secAxiInfo.u.coda9.useOvlEnable&0x01) <<12   | 
                                          (pEncInfo->secAxiInfo.u.coda9.useMeEnable&0x01) <<13));     

    /* Set up work-buffer */
    VpuWriteReg(coreIdx, W4_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
    VpuWriteReg(coreIdx, W4_WORK_SIZE,      pEncInfo->vbWork.size);
    VpuWriteReg(coreIdx, W4_WORK_PARAM,     0);

    /* Set up temp-buffer */
    VpuWriteReg(coreIdx, W4_ADDR_TEMP_BASE, pEncInfo->vbTemp.phys_addr);
    VpuWriteReg(coreIdx, W4_TEMP_SIZE,      pEncInfo->vbTemp.size);
    VpuWriteReg(coreIdx, W4_TEMP_PARAM,     0);

    /* Set up reporting-buffer */

    VpuWriteReg(coreIdx, C7_CMD_ENC_PIC_OPTION, OPT_ENC_PIC);

    srcFrameFormat = pOpenParam->cbcrInterleave<<1  | pOpenParam->nv21;

    VpuWriteReg(coreIdx, C7_CMD_ENC_SRC_FORMAT, (srcFrameFormat<<0)  |
                                                //(srcPixelFormat<<3)  |
                                                (option->srcEndFlag<<31));
    
    VpuWriteReg(coreIdx, C7_CMD_ENC_PIC_QS, option->quantParam);

    if (option->skipPicture) {
        VpuWriteReg(coreIdx, C7_CMD_ENC_PIC_PARAM, 1);  // pic_skip_flag = 1
    }
    else {
        VpuWriteReg(coreIdx, C7_CMD_ENC_SRC_STRIDE, pSrcFrame->stride);
        VpuWriteReg(coreIdx, C7_CMD_ENC_SRC_ADDR_Y, pSrcFrame->bufY);
        if (pOpenParam->cbcrOrder == CBCR_ORDER_NORMAL) {
            VpuWriteReg(coreIdx, C7_CMD_ENC_SRC_ADDR_U, pSrcFrame->bufCb);
            VpuWriteReg(coreIdx, C7_CMD_ENC_SRC_ADDR_V, pSrcFrame->bufCr);    
        }
        else {
            VpuWriteReg(coreIdx, C7_CMD_ENC_SRC_ADDR_U, pSrcFrame->bufCr);
            VpuWriteReg(coreIdx, C7_CMD_ENC_SRC_ADDR_V, pSrcFrame->bufCb);
        }

        VpuWriteReg(coreIdx, C7_CMD_ENC_PIC_PARAM,  0    |
                                                    (option->forcePicQpEnable<<1) |
                                                    (option->forcePicQpI<<2) |
                                                    (option->forcePicQpP<<8) | 
                                                    (option->forcePicTypeEnable<<20) |
                                                    (option->forcePicType)<< 21);
    }


    Wave4BitIssueCommand(instance, ENC_PIC);

    return RETCODE_SUCCESS;
}

RetCode Coda7qVpuEncGetResult(CodecInst* instance, EncOutputInfo* result)
{
    Uint32      encodingSuccess, errorReason = 0;
    Int32       coreIdx;
    EncInfo*    pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);

    coreIdx = instance->coreIdx;

    if (instance->loggingEnable)
        vdi_log(coreIdx, ENC_PIC, 0);

    encodingSuccess = VpuReadReg(coreIdx, W4_RET_SUCCESS);
    if (encodingSuccess == FALSE) {
        errorReason = VpuReadReg(coreIdx, W4_RET_FAIL_REASON);

        if (errorReason == WAVE4_SYSERR_WRITEPROTECTION) {
            vdi_print_vpu_status(coreIdx);
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        }
        if (errorReason == WAVE4_SYSERR_CP0_EXCEPTION) {
            vdi_print_vpu_status(coreIdx);
            return RETCODE_CP0_EXCEPTION;
        }
        if (errorReason == WAVE4_SYSERR_STREAM_BUF_FULL) {
            vdi_print_vpu_status(coreIdx);
            return RETCODE_STREAM_BUF_FULL;
        }

        if (errorReason == WAVE4_SYSERR_ACCESS_VIOLATION_HW) {
            vdi_print_vpu_status(coreIdx);
            return RETCODE_ACCESS_VIOLATION_HW;
        }
        return RETCODE_FAILURE;
    }

    pEncInfo->frameIdx          = VpuReadReg(coreIdx, C7_RET_ENC_PIC_NUM);
    result->picType             = VpuReadReg(coreIdx, C7_RET_ENC_PIC_TYPE);
    result->numOfSlices         = VpuReadReg(coreIdx, C7_RET_ENC_PIC_SLICE_NUM);
    result->reconFrameIndex     = VpuReadReg(coreIdx, C7_RET_ENC_PIC_FRAME_IDX);
    result->bitstreamWrapAround = VpuReadReg(coreIdx, C7_RET_ENC_PIC_FLAG);

    if (result->reconFrameIndex >= 0)
        result->reconFrame  = pEncInfo->frameBufPool[result->reconFrameIndex];

    result->frameCycle      = VpuReadReg(coreIdx, W4_FRAME_CYCLE);

    pEncInfo->streamWrPtr   = VpuReadReg(coreIdx, pEncInfo->streamWrPtrRegAddr);
    if (pEncInfo->ringBufferEnable == 0) {      
        result->bitstreamBuffer = VpuReadReg(coreIdx, pEncInfo->streamRdPtrRegAddr);
        result->bitstreamSize   = pEncInfo->streamWrPtr - result->bitstreamBuffer;
    }

    result->rdPtr = pEncInfo->streamRdPtr;
    result->wrPtr = pEncInfo->streamWrPtr;

    if (result->reconFrameIndex < 0)
        result->bitstreamSize   = 0;

    return RETCODE_SUCCESS;
}
 
