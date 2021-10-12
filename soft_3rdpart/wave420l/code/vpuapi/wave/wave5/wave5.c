#include "product.h"
#include "wave/common/common.h"
#include "wave/common/common_vpuconfig.h"
#include "wave/wave5/wave5.h"
#include "vpuerror.h"
#include "wave/wave5/wave5_regdefine.h"


void Wave5BitIssueCommand(CodecInst* instance, Uint32 cmd)
{
    Uint32 instanceIndex = 0;
    Uint32 codecMode     = 0;
    Uint32 coreIdx;
    if (instance != NULL) {
        instanceIndex = instance->instIndex;
        codecMode     = instance->codecMode;
    }
    coreIdx = instance->coreIdx;

    VpuWriteReg(coreIdx, W5_CMD_INSTANCE_INFO,  (codecMode<<16)|(instanceIndex&0xffff));
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, cmd);

    if ((instance != NULL && instance->loggingEnable))
        vdi_log(coreIdx, cmd, 1);

    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);	
    return;
} 
static RetCode SendQuery(CodecInst* instance, QUERY_OPT queryOpt)
{
    // Send QUERY cmd
    VpuWriteReg(instance->coreIdx, W5_QUERY_OPTION, queryOpt);
    VpuWriteReg(instance->coreIdx, W5_VPU_BUSY_STATUS, 1);
    Wave5BitIssueCommand(instance, W5_QUERY);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, W5_QUERY, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE)
        return RETCODE_FAILURE;

    return RETCODE_SUCCESS;

}
static RetCode SetupWave5Properties(Uint32 coreIdx)
{
    VpuAttr*    pAttr = &g_VpuCoreAttributes[coreIdx];
    Uint32      regVal;
    Uint8*      str;
    RetCode     ret = RETCODE_SUCCESS;

    VpuWriteReg(coreIdx, W5_QUERY_OPTION, GET_VPU_INFO);
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_QUERY);
    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);	
    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == FALSE) {
        ret = RETCODE_QUERY_FAILURE;
    }
    else {
        regVal = VpuReadReg(coreIdx, W5_RET_PRODUCT_NAME);
        str    = (Uint8*)&regVal;
        pAttr->productName[0] = str[3];
        pAttr->productName[1] = str[2];
        pAttr->productName[2] = str[1];
        pAttr->productName[3] = str[0];
        pAttr->productName[4] = 0;
        pAttr->productNumber  = VpuReadReg(coreIdx, W5_RET_PRODUCT_VERSION);

        switch (pAttr->productNumber) {
        case WAVE510_CODE:  pAttr->productId = PRODUCT_ID_510; break;
        case WAVE512_CODE:  pAttr->productId = PRODUCT_ID_512; break;
        case WAVE515_CODE:  pAttr->productId = PRODUCT_ID_515; break;
        case WAVE520_CODE:  pAttr->productId = PRODUCT_ID_520; break;
        default:            pAttr->productId = PRODUCT_ID_NONE; break;
        }

        pAttr->hwConfigDef0    = VpuReadReg(coreIdx, W5_RET_STD_DEF0);
        pAttr->hwConfigDef1    = VpuReadReg(coreIdx, W5_RET_STD_DEF1);
        pAttr->hwConfigFeature = VpuReadReg(coreIdx, W5_RET_CONF_FEATURE);
        pAttr->hwConfigDate    = VpuReadReg(coreIdx, W5_RET_CONF_DATE);
        pAttr->hwConfigRev     = VpuReadReg(coreIdx, W5_RET_CONF_REVISION);
        pAttr->hwConfigType    = VpuReadReg(coreIdx, W5_RET_CONF_TYPE);

        pAttr->supportGDIHW          = TRUE;
        pAttr->supportDecoders       = (1<<STD_HEVC);
        if (pAttr->productId == PRODUCT_ID_512) {
            pAttr->supportDecoders       |= (1<<STD_VP9);
        }
        pAttr->supportEncoders       = 0;
        if (pAttr->productId == PRODUCT_ID_520) {
            pAttr->supportEncoders |= (1<<STD_HEVC);
        }

        pAttr->supportCommandQueue   = TRUE;

        pAttr->supportFBCBWOptimization = (BOOL)((pAttr->hwConfigDef1>>15)&0x01);
        if (pAttr->productId == PRODUCT_ID_520)
            pAttr->supportWTL        = FALSE;
        else
            pAttr->supportWTL        = TRUE;

        pAttr->supportTiled2Linear   = FALSE;
        pAttr->supportMapTypes       = FALSE;
        pAttr->support128bitBus      = TRUE;
        pAttr->supportThumbnailMode  = TRUE;
        pAttr->supportEndianMask     = (Uint32)((1<<VDI_LITTLE_ENDIAN) | (1<<VDI_BIG_ENDIAN) | (1<<VDI_32BIT_LITTLE_ENDIAN) | (1<<VDI_32BIT_BIG_ENDIAN) | (0xffff<<16));
        pAttr->supportBitstreamMode  = (1<<BS_MODE_INTERRUPT) | (1<<BS_MODE_PIC_END);
        pAttr->framebufferCacheType  = FramebufCacheNone;
        pAttr->bitstreamBufferMargin = 0;
        pAttr->numberOfVCores        = MAX_NUM_VCORE;
        pAttr->numberOfMemProtectRgns = 10;
    }

    return ret;
}

RetCode Wave5VpuGetVersion(Uint32 coreIdx, Uint32* versionInfo, Uint32* revision)
{
    Uint32          regVal;

    VpuWriteReg(coreIdx, W5_QUERY_OPTION, GET_VPU_INFO);
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_QUERY);
    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);	
    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == FALSE)
        return RETCODE_QUERY_FAILURE;

    regVal = VpuReadReg(coreIdx, W5_RET_FW_VERSION);
    if (versionInfo != NULL) {
        *versionInfo = 0;
    }
    if (revision != NULL) {
        *revision    = regVal; 
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuGetProductInfo(Uint32 coreIdx, ProductInfo *productInfo)
{
    /* GET FIRMWARE&HARDWARE INFORMATION */
    VpuWriteReg(coreIdx, W5_QUERY_OPTION, GET_VPU_INFO);
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_QUERY);
    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);	
    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == FALSE) {
        return RETCODE_QUERY_FAILURE;
    }

    productInfo->fwVersion      = VpuReadReg(coreIdx, W5_RET_FW_VERSION);
    productInfo->productName    = VpuReadReg(coreIdx, W5_RET_PRODUCT_NAME);
    productInfo->productVersion = VpuReadReg(coreIdx, W5_RET_PRODUCT_VERSION);
    productInfo->customerId     = VpuReadReg(coreIdx, W5_RET_CUSTOMER_ID);
    productInfo->stdDef0        = VpuReadReg(coreIdx, W5_RET_STD_DEF0);
    productInfo->stdDef1        = VpuReadReg(coreIdx, W5_RET_STD_DEF1);
    productInfo->confFeature    = VpuReadReg(coreIdx, W5_RET_CONF_FEATURE);
    productInfo->configDate     = VpuReadReg(coreIdx, W5_RET_CONF_DATE);
    productInfo->configRevision = VpuReadReg(coreIdx, W5_RET_CONF_REVISION);
    productInfo->configType     = VpuReadReg(coreIdx, W5_RET_CONF_TYPE);

    productInfo->configVcore[0]  = 0;
    productInfo->configVcore[1]  = 0;
    productInfo->configVcore[2]  = 0;
    productInfo->configVcore[3]  = 0;

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase;
    PhysicalAddress taskBufBase;
    Uint32          codeSize, tempSize;
    Uint32          i, regVal, remapSize;
    Uint32          hwOption    = 0;
    RetCode         ret = RETCODE_SUCCESS;
    CodecInstHeader hdr;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    vdi_get_common_memory(coreIdx, &vb);

    codeBase  = vb.phys_addr;
    /* ALIGN TO 4KB */
    codeSize = (WAVE5_MAX_CODE_BUF_SIZE&~0xfff);
    if (codeSize < size*2) {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    tempBase = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
    tempSize = WAVE5_TEMPBUF_SIZE;

    VLOG(INFO, "\nVPU INIT Start!!!\n");

    VpuWriteMem(coreIdx, codeBase, (unsigned char*)firmware, size*2, VDI_128BIT_LITTLE_ENDIAN);

    vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)firmware);

    regVal = 0;
    VpuWriteReg(coreIdx, W5_PO_CONF, regVal);

    /* Reset All blocks */
    regVal = 0x7ffffff;
    VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, regVal);    // Reset All blocks 
    /* Waiting reset done */

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
        VLOG(ERR, "VPU init(W5_VPU_RESET_REQ) timeout\n");
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);

    /* clear registers */
    for (i=W5_CMD_REG_BASE; i<W5_CMD_REG_END; i+=4) 
        VpuWriteReg(coreIdx, i, 0x00);

    /* remap page size */
    remapSize = (codeSize >> 12) &0x1ff;
    regVal = 0x80000000 | (WAVE5_AXI_ID<<20) | (0 << 16) | (W5_REMAP_CODE_INDEX<<12) | (1<<11) | remapSize;
    VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
    VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
    VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
    VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
    VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
    VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_AXI_ID<<4) | 0);
    VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
    VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
    VpuWriteReg(coreIdx, W5_TIMEOUT_CNT, 0xffff);

    VpuWriteReg(coreIdx, W5_HW_OPTION, hwOption);

    /* Interrupt */
    // decoder
    regVal  = (1<<W5_INT_INIT_SEQ);
    regVal |= (1<<W5_INT_DEC_PIC);
    regVal |= (1<<W5_INT_BSBUF_EMPTY);

    VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

    VpuWriteReg(coreIdx, W5_CMD_INIT_NUM_TASK_BUF, COMMAND_QUEUE_DEPTH);
    VpuWriteReg(coreIdx, W5_CMD_INIT_TASK_BUF_SIZE, ONE_TASKBUF_SIZE_FOR_CQ);
    vdi_get_common_memory(coreIdx, &vb);
    for (i = 0; i < COMMAND_QUEUE_DEPTH; i++) {
        taskBufBase = vb.phys_addr + WAVE5_TASK_BUF_OFFSET + (i*ONE_TASKBUF_SIZE_FOR_CQ);
        VpuWriteReg(coreIdx, W5_CMD_INIT_ADDR_TASK_BUF0 + (i*4), taskBufBase);
    }

    if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
        return RETCODE_INSUFFICIENT_RESOURCE;

    VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
    VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);

    hdr.coreIdx = coreIdx;
    
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_INIT_VPU);
    VpuWriteReg(coreIdx, W5_VPU_REMAP_CORE_START, 1);

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        VLOG(ERR, "VPU init(W5_VPU_REMAP_CORE_START) timeout\n");
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
    if (regVal == 0) {
        Uint32      reasonCode = VpuReadReg(coreIdx, W5_RET_FAIL_REASON);
        VLOG(ERR, "VPU init(W5_RET_SUCCESS) failed(%d) REASON CODE(%08x)\n", regVal, reasonCode);
        return RETCODE_FAILURE;	
    }

    ret = SetupWave5Properties(coreIdx);
    return ret;
}

RetCode Wave5VpuBuildUpDecParam(CodecInst* instance, DecOpenParam* param)
{
    RetCode     ret = RETCODE_SUCCESS;
    DecInfo*    pDecInfo;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[instance->coreIdx];
    Uint32      bsEndian = 0;
    pDecInfo    = VPU_HANDLE_TO_DECINFO(instance);

    pDecInfo->streamRdPtrRegAddr      = W5_RET_DEC_BS_RD_PTR;
    pDecInfo->streamWrPtrRegAddr      = W5_BS_WR_PTR;
    pDecInfo->frameDisplayFlagRegAddr = W5_RET_DEC_DISP_FLAG;
    pDecInfo->currentPC               = W5_VCPU_CUR_PC;
    pDecInfo->busyFlagAddr            = W5_VPU_BUSY_STATUS;
    if ((pAttr->supportDecoders&(1<<param->bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;
    pDecInfo->seqChangeMask           = param->bitstreamFormat == STD_HEVC ? 
                                        SEQ_CHANGE_ENABLE_ALL_HEVC : SEQ_CHANGE_ENABLE_ALL_VP9;

    pDecInfo->targetSubLayerId       = HEVC_MAX_SUB_LAYER_ID;

    if (param->vbWork.size > 0) {
        pDecInfo->vbWork = param->vbWork;
        pDecInfo->workBufferAllocExt = TRUE;
        vdi_attach_dma_memory(instance->coreIdx, &param->vbWork);
    }
    else {
        if (instance->productId == PRODUCT_ID_512) {
            pDecInfo->vbWork.size       = WAVE512DEC_WORKBUF_SIZE;
        }
        else {
            pDecInfo->vbWork.size       = WAVE510DEC_WORKBUF_SIZE;
        }
        pDecInfo->workBufferAllocExt    = FALSE;
        if (vdi_allocate_dma_memory(instance->coreIdx, &pDecInfo->vbWork) < 0) {
            pDecInfo->vbWork.base       = 0;
            pDecInfo->vbWork.phys_addr  = 0;
            pDecInfo->vbWork.size       = 0;
            pDecInfo->vbWork.virt_addr  = 0;
            return RETCODE_INSUFFICIENT_RESOURCE;
        } 
    }

    vdi_clear_memory(instance->coreIdx, pDecInfo->vbWork.phys_addr, pDecInfo->vbWork.size, 0);

    VpuWriteReg(instance->coreIdx, W5_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W5_WORK_SIZE,      pDecInfo->vbWork.size);
    
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_BS_START_ADDR, pDecInfo->streamBufStartAddr);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_BS_SIZE, pDecInfo->streamBufSize);
    
    bsEndian = vdi_convert_endian(instance->coreIdx, param->streamEndian);
    /* NOTE: When endian mode is 0, SDMA reads MSB first */
    bsEndian = (~bsEndian&VDI_128BIT_ENDIAN_MASK);
    VpuWriteReg(instance->coreIdx, W5_CMD_BS_PARAM, bsEndian);

    VpuWriteReg(instance->coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(instance->coreIdx, W5_RET_SUCCESS, 0);	//for debug	

    Wave5BitIssueCommand(instance, W5_CREATE_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, W5_CREATE_INSTANCE, 2);
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding into VCPU QUEUE
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork);
        ret = RETCODE_FAILURE;
    }

    return ret;
}

RetCode Wave5VpuDecInitSeq(CodecInst* instance)
{
    RetCode     ret = RETCODE_SUCCESS;
    DecInfo*    pDecInfo;
    Uint32      cmdOption = INIT_SEQ_NORMAL, bsOption;
    Uint32      regVal;

    if (instance == NULL) 
        return RETCODE_INVALID_PARAM;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);
    if (pDecInfo->thumbnailMode)
        cmdOption = INIT_SEQ_W_THUMBNAIL;


    /* Set attributes of bitstream buffer controller */
    bsOption = 0;
    switch (pDecInfo->openParam.bitstreamMode) {
    case BS_MODE_INTERRUPT: 
        if(pDecInfo->seqInitEscape == TRUE)
            bsOption = BSOPTION_ENABLE_EXPLICIT_END;
        break;
    case BS_MODE_PIC_END:
        bsOption = BSOPTION_ENABLE_EXPLICIT_END;
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    if (pDecInfo->streamEndflag == 1)
        bsOption = 3;

    VpuWriteReg(instance->coreIdx, W5_BS_RD_PTR, pDecInfo->streamRdPtr);
    VpuWriteReg(instance->coreIdx, W5_BS_WR_PTR, pDecInfo->streamWrPtr);
    
    VpuWriteReg(instance->coreIdx, W5_BS_OPTION, (1<<31) | bsOption);

    VpuWriteReg(instance->coreIdx, W5_COMMAND_OPTION, cmdOption);
    Wave5BitIssueCommand(instance, W5_INIT_SEQ);

    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, W5_INIT_SEQ, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pDecInfo->instanceQueueCount = (regVal>>16)&0xff;
    pDecInfo->totalQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == 1)
            ret = RETCODE_QUEUEING_FAILURE;
        else 
            ret = RETCODE_FAILURE;
    }

    return ret;
}

static void GetDecSequenceResult(CodecInst* instance, DecInitialInfo* info)
{
    DecInfo*   pDecInfo   = &instance->CodecInfo->decInfo;
    Uint32     regVal;
    Uint32     profileCompatibilityFlag;
    Uint32     left, right, top, bottom;

    info->rdPtr = VpuReadReg(instance->coreIdx, W5_RET_DEC_BS_RD_PTR);
    //info->wrPtr = VpuReadReg(instance->coreIdx, W4_BS_WR_PTR);

    pDecInfo->streamRdPtr      = VpuReadReg(instance->coreIdx, W5_RET_DEC_BS_RD_PTR);   
    pDecInfo->frameDisplayFlag = VpuReadReg(instance->coreIdx, W5_RET_DEC_DISP_FLAG);  
    /*regVal = VpuReadReg(instance->coreIdx, W4_BS_OPTION); 
    pDecInfo->streamEndflag    = (regVal&0x02) ? TRUE : FALSE;*/

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_SIZE);
    info->picWidth            = ( (regVal >> 16) & 0xffff );
    info->picHeight           = ( regVal & 0xffff );
    info->minFrameBufferCount = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAMEBUF_NEEDED);
    info->frameBufDelay       = VpuReadReg(instance->coreIdx, W5_RET_DEC_NUM_REORDER_DELAY);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_CROP_LEFT_RIGHT);
    left   = (regVal >> 16) & 0xffff;
    right  = regVal & 0xffff;
    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_CROP_TOP_BOTTOM);
    top    = (regVal >> 16) & 0xffff;
    bottom = regVal & 0xffff;

    info->picCropRect.left   = left;
    info->picCropRect.right  = info->picWidth - right;
    info->picCropRect.top    = top;
    info->picCropRect.bottom = info->picHeight - bottom;

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SEQ_PARAM);
    profileCompatibilityFlag     = (regVal>>12)&0xff;
    info->profile                = (regVal >> 24)&0x1f;
    info->level                  = regVal & 0xff;
    info->tier                   = (regVal >> 29)&0x01;
    info->maxSubLayers           = (regVal >> 21)&0x07;
    info->fRateNumerator         = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAME_RATE_NR);
    info->fRateDenominator       = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAME_RATE_DR);
    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_COLOR_SAMPLE_INFO);
    info->chromaFormatIDC        = (regVal>>8)&0x0f;
    info->lumaBitdepth           = (regVal>>0)&0x0f;
    info->chromaBitdepth         = (regVal>>4)&0x0f;
    info->aspectRateInfo         = (regVal>>16)&0xff;
    info->isExtSAR               = (info->aspectRateInfo == 255 ? TRUE : FALSE);
    if (info->isExtSAR == TRUE) {
        info->aspectRateInfo     = VpuReadReg(instance->coreIdx, W5_RET_DEC_ASPECT_RATIO);  /* [0:15] - vertical size, [16:31] - horizontal size */
    }
    info->bitRate                = VpuReadReg(instance->coreIdx, W5_RET_DEC_BIT_RATE);

    if ( instance->codecMode == C7_HEVC_DEC ) {
        /* Guessing Profile */
        if (info->profile == 0) {
            if ((profileCompatibilityFlag&0x06) == 0x06)        info->profile = 1;      /* Main profile */
            else if ((profileCompatibilityFlag&0x04) == 0x04)   info->profile = 2;      /* Main10 profile */
            else if ((profileCompatibilityFlag&0x08) == 0x08)   info->profile = 3;      /* Main Still Picture profile */
            else                                                info->profile = 1;      /* For old version HM */
        }
    }

    return;
}

RetCode Wave5VpuDecGetSeqInfo(CodecInst* instance, DecInitialInfo* info)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      regVal, i;
    DecInfo*    pDecInfo;
    
    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    // Send QUERY cmd
    ret = SendQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else
            return RETCODE_QUERY_FAILURE;
    }

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_INIT_SEQ, 0);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pDecInfo->instanceQueueCount = (regVal>>16)&0xff;
    pDecInfo->totalQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_SUCCESS) != 1) {
        info->seqInitErrReason = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
        if (info->seqInitErrReason == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            ret = RETCODE_MEMORY_ACCESS_VIOLATION;
        else
            ret = RETCODE_FAILURE;
    }
    
    // Get Sequence Info
    info->userDataSize   = 0;
    info->userDataNum    = 0;
    info->userDataHeader = VpuReadReg(instance->coreIdx, W5_RET_DEC_USERDATA_IDC);
    if (info->userDataHeader != 0) {
        regVal = info->userDataHeader;
        for (i=0; i<32; i++) {
            if (regVal & (1<<i)) {
                info->userDataNum++;
            }
        }
        info->userDataSize = pDecInfo->userDataBufSize;
    }
    
    regVal = VpuReadReg(instance->coreIdx, W5_RET_DONE_INSTANCE_INFO);
    
    GetDecSequenceResult(instance, info);

    return ret;
}

RetCode Wave5VpuDecRegisterFramebuffer(CodecInst* inst, FrameBuffer* fbArr, TiledMapType mapType, Uint32 count)
{
    RetCode      ret = RETCODE_SUCCESS;
    DecInfo*     pDecInfo = &inst->CodecInfo->decInfo;
    DecInitialInfo* sequenceInfo = &inst->CodecInfo->decInfo.initialInfo;
    Int32        q, j, i, remain, idx;
    Uint32 mvCount;
    Uint32       k;
    Int32        coreIdx, startNo, endNo;
    Uint32       regVal, cbcrInterleave, nv21, picSize;
    Uint32       endian, yuvFormat = 0;
    Uint32       addrY, addrCb, addrCr;
    Uint32       mvColSize, fbcYTblSize, fbcCTblSize;
    vpu_buffer_t vbBuffer;
    Uint32       stride;
    Uint32       colorFormat  = 0;
    Uint32       outputFormat = 0;
    Uint32       axiID;


    coreIdx        = inst->coreIdx;
    axiID          = pDecInfo->openParam.virtAxiID;
    cbcrInterleave = pDecInfo->openParam.cbcrInterleave;
    nv21           = pDecInfo->openParam.nv21;
    mvColSize      = fbcYTblSize = fbcCTblSize = 0;
    if (mapType == COMPRESSED_FRAME_MAP) {
        cbcrInterleave = 0;
        nv21           = 0;

        if (inst->codecMode == C7_HEVC_DEC) {
            mvColSize          = WAVE4_DEC_HEVC_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        } 
        else if(inst->codecMode == C7_VP9_DEC) {
            mvColSize          = WAVE4_DEC_VP9_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        else {
            /* Unknown codec */
            return RETCODE_NOT_SUPPORTED_FEATURE;
        }
        
        mvColSize          = VPU_ALIGN16(mvColSize);
        vbBuffer.phys_addr = 0;
        if (inst->codecMode == HEVC_DEC || inst->codecMode == C7_HEVC_DEC || inst->codecMode == C7_VP9_DEC) {
            vbBuffer.size      = ((mvColSize+4095)&~4095)+4096;   /* 4096 is a margin */
            mvCount = count;

            for (k=0  ; k<mvCount ; k++) {
                if ( pDecInfo->vbMV[k].size == 0) {
                    if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
                        return RETCODE_INSUFFICIENT_RESOURCE;
                    pDecInfo->vbMV[k] = vbBuffer;
                }
            }
        }

        //VP9 Decoded size : 64 aligned.
        if (inst->codecMode == C7_HEVC_DEC){
            fbcYTblSize        = WAVE4_FBC_LUMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        } 
        else if (inst->codecMode == C7_VP9_DEC) {
            fbcYTblSize        = WAVE4_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
        }
        else {
            /* Unknown codec */
            return RETCODE_NOT_SUPPORTED_FEATURE;
        }

        fbcYTblSize        = VPU_ALIGN16(fbcYTblSize);
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcYTblSize+4095)&~4095)+4096;
        for (k=0  ; k<count ; k++) {
            if ( pDecInfo->vbFbcYTbl[k].size == 0) {
                if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
                    return RETCODE_INSUFFICIENT_RESOURCE;
                pDecInfo->vbFbcYTbl[k] = vbBuffer;
            }
        }

        if (inst->codecMode == C7_HEVC_DEC) {
            fbcCTblSize        = WAVE4_FBC_CHROMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        } 
        else if (inst->codecMode == C7_VP9_DEC) {
            fbcCTblSize        = WAVE4_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
        }
        else {
            /* Unknown codec */
            return RETCODE_NOT_SUPPORTED_FEATURE;
        }

        fbcCTblSize        = VPU_ALIGN16(fbcCTblSize);
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcCTblSize+4095)&~4095)+4096;
        for (k=0  ; k<count ; k++) {
            if ( pDecInfo->vbFbcCTbl[k].size == 0) {
                if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
                    return RETCODE_INSUFFICIENT_RESOURCE;
                pDecInfo->vbFbcCTbl[k] = vbBuffer;
            }
        }
        picSize = (pDecInfo->initialInfo.picWidth<<16)|(pDecInfo->initialInfo.picHeight);
    } 
    else
    {
        picSize = (pDecInfo->initialInfo.picWidth<<16)|(pDecInfo->initialInfo.picHeight);
    }
    endian = vdi_convert_endian(coreIdx, fbArr[0].endian) & VDI_128BIT_ENDIAN_MASK;

    VpuWriteReg(coreIdx, W5_PIC_SIZE, picSize);

    yuvFormat = 0; /* YUV420 8bit */
    if (mapType == LINEAR_FRAME_MAP) {
        BOOL   justified = W4_WTL_RIGHT_JUSTIFIED;
        Uint32 formatNo  = W4_WTL_PIXEL_8BIT;
        switch (pDecInfo->wtlFormat) {
        case FORMAT_420_P10_16BIT_MSB:
            justified = W4_WTL_RIGHT_JUSTIFIED; 
            formatNo  = W4_WTL_PIXEL_16BIT;
            break;
        case FORMAT_420_P10_16BIT_LSB: 
            justified = W4_WTL_LEFT_JUSTIFIED; 
            formatNo  = W4_WTL_PIXEL_16BIT;
            break;
        case FORMAT_420_P10_32BIT_MSB: 
            justified = W4_WTL_RIGHT_JUSTIFIED; 
            formatNo  = W4_WTL_PIXEL_32BIT;
            break;
        case FORMAT_420_P10_32BIT_LSB: 
            justified = W4_WTL_LEFT_JUSTIFIED; 
            formatNo  = W4_WTL_PIXEL_32BIT;
            break;
        default:
            break;
        }
        yuvFormat = justified<<2 | formatNo;
    }

    stride = fbArr[0].stride;
    if (mapType == COMPRESSED_FRAME_MAP) {
        if ( pDecInfo->chFbcFrameIdx != -1 )
            stride = fbArr[pDecInfo->chFbcFrameIdx].stride;
    } else {
        if ( pDecInfo->chBwbFrameIdx != -1 )
            stride = fbArr[pDecInfo->chBwbFrameIdx].stride;
    }

    if (mapType == LINEAR_FRAME_MAP) {
        outputFormat  = 0;
        outputFormat |= (nv21 << 1);
        outputFormat |= (cbcrInterleave << 0);
    }


    regVal = 
        ((mapType == LINEAR_FRAME_MAP) << 28)   |
        (axiID << 24)                           |
        (1<< 23)                                |   /* PIXEL ORDER in 128bit. first pixel in low address */
        (yuvFormat     << 20)                   |
        (colorFormat  << 19)                    | 
        (outputFormat << 16)                    |  
        (stride);

    VpuWriteReg(coreIdx, W5_COMMON_PIC_INFO, regVal);
    
    remain = count;
    q      = (remain+7)/8;
    idx    = 0;
    for (j=0; j<q; j++) {
        regVal = (pDecInfo->openParam.fbc_mode<<20)|(endian<<16) | (j==q-1)<<4 | ((j==0)<<3) ;
        VpuWriteReg(coreIdx, W5_SFB_OPTION, regVal);
        startNo = j*8;
        endNo   = startNo + (remain>=8 ? 8 : remain) - 1;

        VpuWriteReg(coreIdx, W5_SET_FB_NUM, (startNo<<8)|endNo);

        for (i=0; i<8 && i<remain; i++) {
            if (mapType == LINEAR_FRAME_MAP && pDecInfo->openParam.cbcrOrder == CBCR_ORDER_REVERSED) {
                addrY  = fbArr[i+startNo].bufY;
                addrCb = fbArr[i+startNo].bufCr;
                addrCr = fbArr[i+startNo].bufCb;
            }
            else {
                addrY  = fbArr[i+startNo].bufY;
                addrCb = fbArr[i+startNo].bufCb;
                addrCr = fbArr[i+startNo].bufCr;
            }
            VpuWriteReg(coreIdx, W5_ADDR_LUMA_BASE0  + (i<<4), addrY);
            VpuWriteReg(coreIdx, W5_ADDR_CB_BASE0    + (i<<4), addrCb);
            APIDPRINT("REGISTER FB[%02d] Y(0x%08x), Cb(0x%08x) ", i, addrY, addrCb);
            if (mapType == COMPRESSED_FRAME_MAP) {
                VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET0 + (i<<4), pDecInfo->vbFbcYTbl[idx].phys_addr); /* Luma FBC offset table */
                VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), pDecInfo->vbFbcCTbl[idx].phys_addr);        /* Chroma FBC offset table */
                VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), pDecInfo->vbMV[idx].phys_addr);
                APIDPRINT("Yo(0x%08x) Co(0x%08x), Mv(0x%08x)\n",
                    pDecInfo->vbFbcYTbl[idx].phys_addr,
                    pDecInfo->vbFbcCTbl[idx].phys_addr,
                    pDecInfo->vbMV[idx].phys_addr);
            }
            else {
                VpuWriteReg(coreIdx, W5_ADDR_CR_BASE0 + (i<<4), addrCr);
                VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), 0);
                VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), 0);
                APIDPRINT("Cr(0x%08x)\n", addrCr);
            }
            idx++;
        }
        remain -= i;

        Wave5BitIssueCommand(inst, W5_SET_FB);
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
    }

    regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    if (ConfigSecAXIWave(coreIdx, inst->codecMode, 
        &pDecInfo->secAxiInfo, pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight, 
        sequenceInfo->profile, sequenceInfo->level) == 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
    }

    return ret;
}

RetCode Wave5VpuDecUpdateFramebuffer(CodecInst* inst, FrameBuffer* fbcFb, FrameBuffer* linearFb, Int32 mvIndex, Int32 picWidth, Int32 picHeight)
{
    RetCode         ret = RETCODE_SUCCESS;
    DecInfo*        pDecInfo = &inst->CodecInfo->decInfo;
    DecInitialInfo* sequenceInfo = &inst->CodecInfo->decInfo.initialInfo;
    Int8            fbcIndex, linearIndex;
    Uint32          coreIdx, regVal;
    Uint32          mvColSize, fbcYTblSize, fbcCTblSize;
    Uint32          linearStride, fbcStride;
    vpu_buffer_t*   pvbMv = NULL;
    vpu_buffer_t*   pvbFbcYOffset;
    vpu_buffer_t*   pvbFbcCOffset;
    CodStd          codec;
    coreIdx     = inst->coreIdx;
    fbcIndex    = (fbcFb  == NULL) ? -1 : fbcFb->myIndex;
    linearIndex = (linearFb == NULL) ? -1 : linearFb->myIndex - pDecInfo->numFbsForDecoding;
    mvColSize   = fbcYTblSize = fbcCTblSize = 0;
    codec       = pDecInfo->openParam.bitstreamFormat;
    if (codec == STD_HEVC) {
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }
    if (codec == STD_HEVC) {
        mvColSize = WAVE4_DEC_HEVC_MVCOL_BUF_SIZE(picWidth, picHeight);
    } 
    else if(codec == STD_VP9) {
        mvColSize = WAVE4_DEC_VP9_MVCOL_BUF_SIZE(picWidth, picHeight);
    }
    else {
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    if (fbcFb != NULL) {
        pDecInfo->frameBufPool[fbcIndex] = *fbcFb;
    }
    if (linearFb != NULL) {
        pDecInfo->frameBufPool[pDecInfo->numFbsForDecoding + linearIndex] = *linearFb;
    }

    if (mvIndex >= 0) {
        pvbMv = &pDecInfo->vbMV[mvIndex];
        vdi_free_dma_memory(inst->coreIdx, pvbMv);
        pvbMv->size = ((mvColSize+4095)&~4095) + 4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, pvbMv) < 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }

    /* Reallocate FBC offset tables */
    if (codec == STD_HEVC){
        fbcYTblSize = WAVE4_FBC_LUMA_TABLE_SIZE(picWidth, picHeight);
    } 
    else if (codec == STD_VP9) {
        //VP9 Decoded size : 64 aligned.
        fbcYTblSize = WAVE4_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(picWidth), VPU_ALIGN64(picHeight));
    }
    else {
        /* Unknown codec */
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    if (fbcFb != NULL) {
        pvbFbcYOffset = &pDecInfo->vbFbcYTbl[fbcIndex];
        vdi_free_dma_memory(inst->coreIdx, pvbFbcYOffset);
        pvbFbcYOffset->phys_addr = 0;
        pvbFbcYOffset->size      = ((fbcYTblSize+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, pvbFbcYOffset) < 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }
    
    if (codec == STD_HEVC) {
        fbcCTblSize = WAVE4_FBC_CHROMA_TABLE_SIZE(picWidth, picHeight);
    } 
    else if (codec == STD_VP9) {
        fbcCTblSize = WAVE4_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(picWidth), VPU_ALIGN64(picHeight));
    }
    else {
        /* Unknown codec */
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    if (fbcFb != NULL) {
        pvbFbcCOffset = &pDecInfo->vbFbcCTbl[fbcIndex];
        vdi_free_dma_memory(inst->coreIdx, pvbFbcCOffset);
        pvbFbcCOffset->phys_addr = 0;
        pvbFbcCOffset->size      = ((fbcCTblSize+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, pvbFbcCOffset) < 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }
 
    linearStride = linearFb == NULL ? 0 : linearFb->stride;
    fbcStride    = fbcFb == NULL ? 0 : fbcFb->stride;
    regVal = linearStride<<16 | fbcStride;
    VpuWriteReg(coreIdx, W5_CMD_SET_FB_STRIDE, regVal);

    regVal = (picWidth<<16) | picHeight;
    VpuWriteReg(coreIdx, W5_PIC_SIZE, regVal);

    VLOG(INFO, "fbcIndex(%d), linearIndex(%d), mvIndex(%d)\n", fbcIndex, linearIndex, mvIndex);
    regVal = (mvIndex&0xff) << 16 | (linearIndex&0xff) << 8 | (fbcIndex&0xff);
    VpuWriteReg(coreIdx, W5_CMD_SET_FB_INDEX, regVal);

    VpuWriteReg(coreIdx, W5_ADDR_LUMA_BASE,     linearFb == NULL ? 0 : linearFb->bufY);
    VpuWriteReg(coreIdx, W5_ADDR_CB_BASE,       linearFb == NULL ? 0 : linearFb->bufCb);
    VpuWriteReg(coreIdx, W5_ADDR_CR_BASE,       linearFb == NULL ? 0 : linearFb->bufCr);
    VpuWriteReg(coreIdx, W5_ADDR_MV_COL,        pvbMv == NULL ? 0 : pvbMv->phys_addr);
    VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_BASE,    fbcFb == NULL ? 0 : fbcFb->bufY);
    VpuWriteReg(coreIdx, W5_ADDR_FBC_C_BASE,    fbcFb == NULL ? 0 : fbcFb->bufCb);
    VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET,  fbcFb == NULL ? 0 : pvbFbcYOffset->phys_addr);
    VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET,  fbcFb == NULL ? 0 : pvbFbcCOffset->phys_addr);
    VpuWriteReg(coreIdx, W5_SFB_OPTION,         1); /* UPDATE FRAMEBUFFER */

    Wave5BitIssueCommand(inst, W5_SET_FB);

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    if (ConfigSecAXIWave(coreIdx, inst->codecMode, 
                         &pDecInfo->secAxiInfo, pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight, 
                         sequenceInfo->profile, sequenceInfo->level) == 0) {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    return ret;
}

RetCode Wave5VpuDecode(CodecInst* instance, DecParam* option)
{
    Uint32      modeOption = DEC_PIC_NORMAL,  bsOption, regVal;
    DecOpenParam*   pOpenParam;
    Int32       forceLatency = -1; 
    Int32       rdptr_valid = 0;
    DecInfo*    pDecInfo = &instance->CodecInfo->decInfo;
    
    pOpenParam = &pDecInfo->openParam;

    if (pDecInfo->thumbnailMode) {
        modeOption = DEC_PIC_W_THUMBNAIL;
    }
    else if (option->skipframeMode) {
        switch (option->skipframeMode) {
        case 1:
            modeOption   = SKIP_NON_IRAP;
            forceLatency = 0;
            break;
        case 2:
            modeOption = SKIP_NON_REF_PIC;
            break;
        default:
            // skip off
            break;
        }
    }
    if (pDecInfo->targetSubLayerId < (pDecInfo->initialInfo.maxSubLayers-1)) {
        modeOption = SKIP_TEMPORAL_LAYER;
    }
    if (option->craAsBlaFlag == TRUE) {
        modeOption |= (1<<1);
    }

    // set disable reorder    
    if (pDecInfo->reorderEnable == FALSE) {
        forceLatency = 0;
    }
    // Bandwidth optimization
    modeOption |= (pDecInfo->openParam.bwOptimization<< 31);

    /* Set attributes of bitstream buffer controller */
    bsOption = 0;
    regVal = 0;
    switch (pOpenParam->bitstreamMode) {
    case BS_MODE_INTERRUPT: 
        bsOption = 0;
        break;
    case BS_MODE_PIC_END:
        bsOption = BSOPTION_ENABLE_EXPLICIT_END;
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    VpuWriteReg(instance->coreIdx, W5_BS_RD_PTR,     pDecInfo->streamRdPtr);
    VpuWriteReg(instance->coreIdx, W5_BS_WR_PTR,     pDecInfo->streamWrPtr);
    if (pDecInfo->streamEndflag == 1)
        bsOption = 3;   // (streamEndFlag<<1) | EXPLICIT_END

    if (pOpenParam->bitstreamMode == BS_MODE_PIC_END || pDecInfo->rdPtrValidFlag == TRUE)
        rdptr_valid = 1;

    VpuWriteReg(instance->coreIdx, W5_BS_OPTION,  (rdptr_valid<<31) | bsOption);

    pDecInfo->rdPtrValidFlag = FALSE;       // reset rdptrValidFlag.

    /* Secondary AXI */
    regVal = (pDecInfo->secAxiInfo.u.wave4.useBitEnable<<0)    |
             (pDecInfo->secAxiInfo.u.wave4.useIpEnable<<9)     |
             (pDecInfo->secAxiInfo.u.wave4.useLfRowEnable<<15);
    VpuWriteReg(instance->coreIdx, W5_USE_SEC_AXI,  regVal);

    /* Set attributes of User buffer */
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_USER_MASK,      pDecInfo->userDataEnable);
    
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_VCORE_LIMIT, 1);

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_TEMPORAL_ID_PLUS1, pDecInfo->targetSubLayerId+1);
    VpuWriteReg(instance->coreIdx, W5_CMD_SEQ_CHANGE_ENABLE_FLAG, pDecInfo->seqChangeMask);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_FORCE_FB_LATENCY_PLUS1, forceLatency+1);
    VpuWriteReg(instance->coreIdx, W5_COMMAND_OPTION, modeOption);
    Wave5BitIssueCommand(instance, W5_DEC_PIC);

    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, W5_DEC_PIC, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pDecInfo->instanceQueueCount = (regVal>>16)&0xff;
    pDecInfo->totalQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == 1)
            return RETCODE_QUEUEING_FAILURE;
        else
            return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuDecGetResult(CodecInst* instance, DecOutputInfo* result)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      regVal, index, nalUnitType;
    DecInfo*    pDecInfo;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_ADDR_REPORT_BASE, pDecInfo->userDataBufAddr);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_SIZE,      pDecInfo->userDataBufSize);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_PARAM,     VPU_USER_DATA_ENDIAN&VDI_128BIT_ENDIAN_MASK);

    // Send QUERY cmd
    ret = SendQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else
            return RETCODE_QUERY_FAILURE;
    }
    
    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_DEC_PIC, 0);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pDecInfo->instanceQueueCount = (regVal>>16)&0xff;
    pDecInfo->totalQueueCount    = (regVal & 0xffff);

    result->decodingSuccess = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_SUCCESS);
    if (result->decodingSuccess == FALSE) {
        result->errorReason = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
        if (result->errorReason == WAVE5_SYSERR_ACCESS_VIOLATION_HW) {
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        }
        else if (result->errorReason == WAVE5_CODEC_ERROR) {
            result->errorReasonExt = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
        }
    }
    else {
        result->warnInfo = VpuReadReg(instance->coreIdx, W5_RET_DEC_WARN_INFO);
    }
    
    result->decOutputExtData.userDataSize   = 0;
    result->decOutputExtData.userDataNum    = 0;
    result->decOutputExtData.userDataHeader = VpuReadReg(instance->coreIdx, W5_RET_DEC_USERDATA_IDC);
    if (result->decOutputExtData.userDataHeader != 0) {
        regVal = result->decOutputExtData.userDataHeader;
        for (index=0; index<32; index++) {
            if (regVal & (1<<index)) {
                result->decOutputExtData.userDataNum++;
            }
        }
        result->decOutputExtData.userDataSize = pDecInfo->userDataBufSize;
    }
    result->frameCycle      = VpuReadReg(instance->coreIdx, W5_FRAME_CYCLE);
    result->seekCycle       = VpuReadReg(instance->coreIdx, W5_RET_DEC_SEEK_CYCLE);
    result->parseCycle      = VpuReadReg(instance->coreIdx, W5_RET_DEC_PARSING_CYCLE);
    result->decodeCycle     = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_CYCLE);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_TYPE);
 
    if (instance->codecMode == C7_VP9_DEC) {
        if      (regVal&0x01) result->picType = PIC_TYPE_I;
        else if (regVal&0x02) result->picType = PIC_TYPE_P;    
        else if (regVal&0x04) result->picType = PIC_TYPE_REPEAT;
        else                  result->picType = PIC_TYPE_MAX;
    }
    else {  // HEVC
        if      (regVal&0x04) result->picType = PIC_TYPE_B;
        else if (regVal&0x02) result->picType = PIC_TYPE_P;
        else if (regVal&0x01) result->picType = PIC_TYPE_I;
        else                  result->picType = PIC_TYPE_MAX;
    }
    result->outputFlag      = (regVal>>31)&0x1;

    nalUnitType = (regVal & 0x3f0) >> 4;
    if ((nalUnitType == 19 || nalUnitType == 20) && result->picType == PIC_TYPE_I) {
        /* IDR_W_RADL, IDR_N_LP */
        result->picType = PIC_TYPE_IDR;
    }
    result->nalType                   = nalUnitType;
    result->ctuSize                   = 16<<((regVal>>10)&0x3);
    index                             = VpuReadReg(instance->coreIdx, W5_RET_DEC_DISPLAY_INDEX);
    result->indexFrameDisplay         = index;
    result->indexFrameDisplayForTiled = index;
    index                             = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODED_INDEX);
    result->indexFrameDecoded         = index;
    result->indexFrameDecodedForTiled = index;

    if (instance->codecMode != C7_VP9_DEC) {
        result->h265Info.decodedPOC = -1;
        result->h265Info.displayPOC = -1;
        if (result->indexFrameDecoded >= 0) 
            result->h265Info.decodedPOC = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_POC);
        result->h265Info.temporalId = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO) & 0xff;
    }

    result->sequenceChanged   = VpuReadReg(instance->coreIdx, W5_RET_DEC_NOTIFICATION);
    /*
     * If current picture is the last of the current sequence and sequence-change flag is not 0, then
     * the width and height of the current picture is set to the width and height of the current sequence.
     */
    if (result->sequenceChanged == 0) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_SIZE);
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
        if ( instance->codecMode == C7_VP9_DEC ) {
            if ( result->sequenceChanged & SEQ_CHANGE_INTER_RES_CHANGE) {
                regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_SIZE);
                result->decPicWidth   = regVal>>16;
                result->decPicHeight  = regVal&0xffff;
                result->indexInterFrameDecoded = VpuReadReg(instance->coreIdx, W5_RET_DEC_REALLOC_INDEX);
            }
        }
        osal_memcpy((void*)&pDecInfo->newSeqInfo, (void*)&pDecInfo->initialInfo, sizeof(DecInitialInfo));
        GetDecSequenceResult(instance, &pDecInfo->newSeqInfo);
    }
    result->numOfErrMBs       = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_CTB_NUM)>>16;
    result->numOfTotMBs       = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_CTB_NUM)&0xffff;
    result->bytePosFrameStart = VpuReadReg(instance->coreIdx, W5_RET_DEC_AU_START_POS);
    result->bytePosFrameEnd   = VpuReadReg(instance->coreIdx, W5_RET_DEC_AU_END_POS);
    pDecInfo->prevFrameEndPos = result->bytePosFrameEnd;

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_RECOVERY_POINT);
    result->h265RpSei.recoveryPocCnt = regVal & 0xFFFF;            // [15:0]
    result->h265RpSei.exactMatchFlag = (regVal >> 16)&0x01;        // [16]
    result->h265RpSei.brokenLinkFlag = (regVal >> 17)&0x01;        // [17]
    result->h265RpSei.exist =  (regVal >> 18)&0x01;                // [18]
    if(result->h265RpSei.exist == 0) {
        result->h265RpSei.recoveryPocCnt = 0;
        result->h265RpSei.exactMatchFlag = 0;
        result->h265RpSei.brokenLinkFlag = 0;
    }


    return RETCODE_SUCCESS;
}

RetCode Wave5VpuDecFlush(CodecInst* instance, FramebufferIndex* framebufferIndexes, Uint32 size)
{
    RetCode ret = RETCODE_SUCCESS;

    Wave5BitIssueCommand(instance, W5_FLUSH_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) 
        return RETCODE_VPU_RESPONSE_TIMEOUT;

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == WAVE5_VPU_STILL_RUNNING)
            ret = RETCODE_VPU_STILL_RUNNING;
        else 
            ret = RETCODE_FAILURE;
    }

    return ret;

}

RetCode Wave5VpuReInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase, taskBufBase;
    PhysicalAddress oldCodeBase, tempSize;
    Uint32          codeSize;
    Uint32          regVal, remapSize, i=0;
    CodecInstHeader hdr;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    vdi_get_common_memory(coreIdx, &vb);

    codeBase  = vb.phys_addr;
    /* ALIGN TO 4KB */
    codeSize = (WAVE5_MAX_CODE_BUF_SIZE&~0xfff);
    if (codeSize < size*2) {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }
    tempBase = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
    tempSize = WAVE5_TEMPBUF_SIZE;
    oldCodeBase = VpuReadReg(coreIdx, W5_VPU_REMAP_PADDR);

    if (oldCodeBase != codeBase) {

        VpuWriteMem(coreIdx, codeBase, (unsigned char*)firmware, size*2, VDI_128BIT_LITTLE_ENDIAN);
        vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)firmware);

        regVal = 0;
        VpuWriteReg(coreIdx, W5_PO_CONF, regVal);

        // Step1 : disable request
        vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x100);

        // Step2 : Waiting for completion of bus transaction
        if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_GDI_BUS_STATUS) == -1) {
            vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        /* Reset All blocks */
        regVal = 0x7ffffff;
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, regVal);    // Reset All blocks 
        /* Waiting reset done */

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
        // Step3 : must clear GDI_BUS_CTRL after done SW_RESET
        vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);

        /* remap page size */
        remapSize = (codeSize >> 12) &0x1ff;
        regVal = 0x80000000 | (WAVE5_AXI_ID<<20) | (W5_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_AXI_ID<<4) | 0);
        VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
        VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
        VpuWriteReg(coreIdx, W5_TIMEOUT_CNT,   0);

        VpuWriteReg(coreIdx, W5_HW_OPTION, 0);
        /* Interrupt */
        regVal  = (1<<W5_INT_INIT_SEQ);
        regVal |= (1<<W5_INT_DEC_PIC);
        regVal |= (1<<W5_INT_BSBUF_EMPTY);

        VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

        VpuWriteReg(coreIdx,W5_CMD_INIT_NUM_TASK_BUF, COMMAND_QUEUE_DEPTH);

        vdi_get_common_memory(coreIdx, &vb);
        for (i = 0; i < COMMAND_QUEUE_DEPTH; i++) {
            taskBufBase = vb.phys_addr + WAVE5_TASK_BUF_OFFSET + (i*ONE_TASKBUF_SIZE_FOR_CQ);
            VpuWriteReg(coreIdx, W5_CMD_INIT_ADDR_TASK_BUF0 + (i*4), taskBufBase);
        }

        if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
            return RETCODE_INSUFFICIENT_RESOURCE;

        VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
        VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);

        hdr.coreIdx = coreIdx;

        VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
        VpuWriteReg(coreIdx, W5_COMMAND, W5_INIT_VPU);
        VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);	
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CORE_START, 1);

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
        if (regVal == 0) 
            return RETCODE_FAILURE;

    }
    SetupWave5Properties(coreIdx);

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuSleepWake(Uint32 coreIdx, int iSleepWake, const Uint16* code, Uint32 size, BOOL reset)
{
    CodecInstHeader hdr;
    Uint32          regVal;
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase;
    Uint32          codeSize, tempSize;
    Uint32          remapSize;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    hdr.coreIdx = coreIdx;

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {		
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if(iSleepWake==1)  //saves
    {
        VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
        VpuWriteReg(coreIdx, W5_COMMAND, W5_SLEEP_VPU);
        VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);	

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1)
        {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
        regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
        if (regVal == 0)
        {
            APIDPRINT("SLEEP_VPU failed [0x%x]", VpuReadReg(coreIdx, W5_RET_FAIL_REASON));
            return RETCODE_FAILURE;
        }
    }
    else //restore
    {
        Uint32  hwOption  = 0;
        Uint32  i;
        PhysicalAddress taskBufBase;

        vdi_get_common_memory(coreIdx, &vb);
        codeBase  = vb.phys_addr;
        /* ALIGN TO 4KB */
        codeSize = (WAVE5_MAX_CODE_BUF_SIZE&~0xfff);
        if (codeSize < size*2) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }

        tempBase = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
        tempSize = WAVE5_TEMPBUF_SIZE;

        regVal = 0;
        VpuWriteReg(coreIdx, W5_PO_CONF, regVal);

        /* SW_RESET_SAFETY */
        regVal = W5_RST_BLOCK_ALL; 
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, regVal);    // Reset All blocks 

        /* Waiting reset done */
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);

        /* remap page size */
        remapSize = (codeSize >> 12) &0x1ff;
        regVal = 0x80000000 | (WAVE5_AXI_ID<<20) | (W5_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_AXI_ID<<4) | 0);
        VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
        VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
        VpuWriteReg(coreIdx, W5_TIMEOUT_CNT,   0);

        VpuWriteReg(coreIdx, W5_HW_OPTION, hwOption);

        /* Interrupt */
        regVal  = (1<<W5_INT_INIT_SEQ);
        regVal |= (1<<W5_INT_DEC_PIC);
        regVal |= (1<<W5_INT_BSBUF_EMPTY);

        VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

        VpuWriteReg(coreIdx, W5_CMD_INIT_NUM_TASK_BUF, COMMAND_QUEUE_DEPTH);
        VpuWriteReg(coreIdx, W5_CMD_INIT_TASK_BUF_SIZE, ONE_TASKBUF_SIZE_FOR_CQ);
        vdi_get_common_memory(coreIdx, &vb);
        for (i = 0; i < COMMAND_QUEUE_DEPTH; i++) {
            taskBufBase = vb.phys_addr + WAVE5_TASK_BUF_OFFSET + (i*ONE_TASKBUF_SIZE_FOR_CQ);
            VpuWriteReg(coreIdx, W5_CMD_INIT_ADDR_TASK_BUF0 + (i*4), taskBufBase);
        }

        if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
            return RETCODE_INSUFFICIENT_RESOURCE;

        VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
        VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);
        VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
        VpuWriteReg(coreIdx, W5_COMMAND, (reset==TRUE ? W5_INIT_VPU : W5_WAKEUP_VPU));
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CORE_START, 1);

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
        if (regVal == 0) {  
            return RETCODE_FAILURE;
        }
        VpuWriteReg(coreIdx, W5_VPU_VINT_REASON_CLR, 0xffff);
        VpuWriteReg(coreIdx, W5_VPU_VINT_REASON_USR, 0);
        VpuWriteReg(coreIdx, W5_VPU_VINT_CLEAR, 0x1);

    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuReset(Uint32 coreIdx, SWResetMode resetMode)
{
    Uint32  val = 0;
    RetCode ret = RETCODE_SUCCESS;

    // VPU doesn't send response. Force to set BUSY flag to 0.
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 0);

    // Waiting for completion of bus transaction
    // Step1 : disable request
    vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x100);

    // Step2 : Waiting for completion of bus transaction
    if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_GDI_BUS_STATUS) == -1) {
        vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (resetMode == SW_RESET_SAFETY) {
        if ((ret=Wave5VpuSleepWake(coreIdx, TRUE, NULL, 0, TRUE)) != RETCODE_SUCCESS) {
            return ret;
        }
    }

    switch (resetMode) {
    case SW_RESET_ON_BOOT:
    case SW_RESET_FORCE:
    case SW_RESET_SAFETY:
        val = W5_RST_BLOCK_ALL; 
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    if (val) {
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, val);

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
            vdi_log(coreIdx, W5_RESET_VPU, 2);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
    }
    // Step3 : must clear GDI_BUS_CTRL after done SW_RESET
        vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
    if (resetMode == SW_RESET_SAFETY || resetMode == SW_RESET_FORCE ) {
        ret = Wave5VpuSleepWake(coreIdx, FALSE, NULL, 0, TRUE);
    }

    return ret;
}

RetCode Wave5VpuDecFiniSeq(CodecInst* instance)
{
    RetCode ret = RETCODE_SUCCESS;

    Wave5BitIssueCommand(instance, W5_DESTROY_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) 
        return RETCODE_VPU_RESPONSE_TIMEOUT;

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == WAVE5_VPU_STILL_RUNNING)
            ret = RETCODE_VPU_STILL_RUNNING;
        else 
            ret = RETCODE_FAILURE;
    }

    return ret;
}

RetCode Wave5VpuDecSetBitstreamFlag(CodecInst* instance, BOOL running, BOOL eos, BOOL explictEnd)
{
    DecInfo* pDecInfo = &instance->CodecInfo->decInfo;
    BitStreamMode bsMode = (BitStreamMode)pDecInfo->openParam.bitstreamMode;
    pDecInfo->streamEndflag = (eos == 1) ? TRUE : FALSE;

    if (bsMode == BS_MODE_INTERRUPT) {
        if (pDecInfo->streamEndflag == TRUE) explictEnd = TRUE;

        VpuWriteReg(instance->coreIdx, W5_BS_OPTION, (pDecInfo->streamEndflag<<1) | explictEnd);
        VpuWriteReg(instance->coreIdx, W5_BS_WR_PTR, pDecInfo->streamWrPtr);

        Wave5BitIssueCommand(instance, W5_UPDATE_BS);
        if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
            
        if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == 0) {
            return RETCODE_FAILURE;
        }
    }

    return RETCODE_SUCCESS;
}


RetCode Wave5DecClrDispFlag(CodecInst* instance, Uint32 index)
{
    RetCode ret = RETCODE_SUCCESS;
    DecInfo * pDecInfo;
    pDecInfo   = &instance->CodecInfo->decInfo;

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_CLR_DISP_IDC, (1<<index));
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_SET_DISP_IDC, 0);
    ret = SendQuery(instance, UPDATE_DISP_FLAG);
    
    if (ret != RETCODE_SUCCESS)
        return RETCODE_QUERY_FAILURE;

    pDecInfo->frameDisplayFlag = VpuReadReg(instance->coreIdx, pDecInfo->frameDisplayFlagRegAddr);

    return RETCODE_SUCCESS;
}

RetCode Wave5DecSetDispFlag(CodecInst* instance, Uint32 index)
{
    RetCode ret = RETCODE_SUCCESS;

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_CLR_DISP_IDC, 0);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_SET_DISP_IDC, (1<<index));
    ret = SendQuery(instance, UPDATE_DISP_FLAG);

    return ret;
}

Int32 Wave5VpuWaitInterrupt(CodecInst* instance, Int32 timeout, BOOL pending)
{
    Int32  reason = -1; 
    Int32  remain_intr = -1; // to set VPU_VINT_REASON for remain interrupt.
    Int32  ownInt      = 0;
    Uint32 regVal;
    Uint32 IntrMask = ((1 << INT_WAVE5_BSBUF_EMPTY) | (1 << INT_WAVE5_DEC_PIC) | (1 << INT_WAVE5_INIT_SEQ) | (1 << INT_WAVE5_ENC_SET_PARAM));

    EnterLock(instance->coreIdx);

    // check one interrupt for current instance even if the number of interrupt triggered more than one.
    if ((reason = vdi_wait_interrupt(instance->coreIdx, timeout, W5_VPU_VINT_REASON_USR)) > 0) {

        remain_intr = VpuReadReg(instance->coreIdx, W5_VPU_VINT_REASON_USR);

        if (reason & (1 << INT_WAVE5_BSBUF_EMPTY)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_BS_EMPTY_INST);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_BSBUF_EMPTY);
                remain_intr &= ~reason;
            }
        }

        if (reason & (1 << INT_WAVE5_DEC_PIC)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_DEC_PIC);
                remain_intr &= ~reason;
            }
        }

        if (reason & (1 << INT_WAVE5_INIT_SEQ)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_INIT_SEQ);
                remain_intr &= ~reason;
            }
        }

        if (reason & (1 << INT_WAVE5_ENC_SET_PARAM)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_ENC_SET_PARAM);
                remain_intr &= ~reason;
            }
        }

        if (reason & ~IntrMask) {    // when interrupt is not for empty, dec_pic, init_seq.
            regVal = VpuReadReg(instance->coreIdx, W5_RET_DONE_INSTANCE_INFO)&0xFF;
            if (regVal == instance->instIndex) {
                ownInt = 1;
                reason = (reason & ~IntrMask);
                remain_intr &= ~reason;
            }
        }

        VpuWriteReg(instance->coreIdx, W5_VPU_VINT_REASON, remain_intr);     // set remain interrupt flag to trigger interrupt next time.

        if (!ownInt)
            reason = -1;    // if there was no interrupt for current instance id, reason should be -1;
    }
    LeaveLock(instance->coreIdx);
    return reason;    
}

RetCode Wave5VpuClearInterrupt(Uint32 coreIdx, Uint32 flags)
{
    Uint32 interruptReason;

    interruptReason = VpuReadReg(coreIdx, W5_VPU_VINT_REASON_USR);
    interruptReason &= ~flags;
    VpuWriteReg(coreIdx, W5_VPU_VINT_REASON_USR, interruptReason);

    return RETCODE_SUCCESS;
}

/************************************************************************/
/*                       ENCODER functions                              */
/************************************************************************/

RetCode Wave5VpuBuildUpEncParam(CodecInst* instance, EncOpenParam* param)
{
    RetCode     ret = RETCODE_SUCCESS;
    EncInfo*    pEncInfo;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[instance->coreIdx];
    pEncInfo    = VPU_HANDLE_TO_ENCINFO(instance);

    pEncInfo->streamRdPtrRegAddr      = W5_RET_ENC_RD_PTR;
    pEncInfo->streamWrPtrRegAddr      = W5_RET_ENC_WR_PTR;
    pEncInfo->currentPC               = W5_VCPU_CUR_PC;
    pEncInfo->busyFlagAddr            = W5_VPU_BUSY_STATUS;

    if ((pAttr->supportEncoders&(1<<param->bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;
    
    if (param->bitstreamFormat == STD_HEVC)
        instance->codecMode = HEVC_ENC;

    pEncInfo->vbWork.size       = WAVE5ENC_WORKBUF_SIZE;
    if (vdi_allocate_dma_memory(instance->coreIdx, &pEncInfo->vbWork) < 0) {
        pEncInfo->vbWork.base       = 0;
        pEncInfo->vbWork.phys_addr  = 0;
        pEncInfo->vbWork.size       = 0;
        pEncInfo->vbWork.virt_addr  = 0;
        return RETCODE_INSUFFICIENT_RESOURCE;
    } 

    vdi_clear_memory(instance->coreIdx, pEncInfo->vbWork.phys_addr, pEncInfo->vbWork.size, 0);

    VpuWriteReg(instance->coreIdx, W5_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W5_WORK_SIZE,      pEncInfo->vbWork.size);

    VpuWriteReg(instance->coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(instance->coreIdx, W5_RET_SUCCESS, 0);	//for debug	

    VpuWriteReg(instance->coreIdx, W5_CMD_ENC_VCORE_LIMIT, 1);

    Wave5BitIssueCommand(instance, W5_CREATE_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, W5_CREATE_INSTANCE, 2);
        vdi_free_dma_memory(instance->coreIdx, &pEncInfo->vbWork);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding into VCPU QUEUE
        vdi_free_dma_memory(instance->coreIdx, &pEncInfo->vbWork);
        ret = RETCODE_FAILURE;
    }

    pEncInfo->streamRdPtr           = param->bitstreamBuffer;	
    pEncInfo->streamWrPtr           = param->bitstreamBuffer;
    pEncInfo->lineBufIntEn          = param->lineBufIntEn;	
    pEncInfo->streamBufStartAddr    = param->bitstreamBuffer;
    pEncInfo->streamBufSize         = param->bitstreamBufferSize;
    pEncInfo->streamBufEndAddr      = param->bitstreamBuffer + param->bitstreamBufferSize;
    pEncInfo->stride                = 0;
    pEncInfo->vbFrame.size          = 0;
    pEncInfo->vbPPU.size            = 0;
    pEncInfo->frameAllocExt         = 0;
    pEncInfo->ppuAllocExt           = 0;
    pEncInfo->initialInfoObtained   = 0;

    return ret;
}

RetCode Wave5VpuEncInitSeq(CodecInst* instance)
{
    Int32           coreIdx, width32 = 0, height32=0;
    Uint32          regVal = 0, rotMirMode;
    EncInfo*        pEncInfo;
    EncOpenParam*   pOpenParam;
    EncHevcParam*   pHevc;
    coreIdx    = instance->coreIdx;
    pEncInfo = &instance->CodecInfo->encInfo;

    pOpenParam  = &pEncInfo->openParam;
    pHevc       = &pOpenParam->EncStdParam.hevcParam;

    /*==============================================*/
    /*  OPT_CUSTOM_GOP                              */
    /*==============================================*/
    /*
    * SET_PARAM + CUSTOM_GOP
    * only when gopPresetIdx == custom_gop, custom_gop related registers should be set
    */
    if (pHevc->gopPresetIdx == PRESET_IDX_CUSTOM_GOP) {
        int i=0, j = 0;
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_OPTION, OPT_CUSTOM_GOP);
        VpuWriteReg(coreIdx, W5_CMD_ENC_CUSTOM_GOP_PARAM, pHevc->gopParam.customGopSize);

        for (i=0 ; i<pHevc->gopParam.customGopSize; i++) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_0 + (i*4), (pHevc->gopParam.picParam[i].picType<<0)            |
                                                                            (pHevc->gopParam.picParam[i].pocOffset<<2)          |
                                                                            (pHevc->gopParam.picParam[i].picQp<<6)              |
                                                                            (pHevc->gopParam.picParam[i].numRefPicL0<<12)       |
                                                                            ((pHevc->gopParam.picParam[i].refPocL0&0x1F)<<14)   |
                                                                            ((pHevc->gopParam.picParam[i].refPocL1&0x1F)<<19)   |
                                                                            (pHevc->gopParam.picParam[i].temporalId<<24));
        }

        for (j = i; j < MAX_GOP_NUM; j++) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_0 + (j*4), 0);
        }

        Wave5BitIssueCommand(instance, W5_ENC_SET_PARAM);

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            if (instance->loggingEnable)
                vdi_log(coreIdx, W5_ENC_SET_PARAM, 2);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

    }

    /*======================================================================*/
    /*  OPT_COMMON                                                          */
    /*      : the last SET_PARAM command should be called with OPT_COMMON   */
    /*======================================================================*/
    rotMirMode = 0;
    /* CMD_ENC_ROT_MODE : 
    *          | hor_mir | ver_mir |   rot_angle     | rot_en |
    *              [4]       [3]         [2:1]           [0]  
    */
    if (pEncInfo->rotationEnable == TRUE) {
        switch (pEncInfo->rotationAngle) {
        case 0:
            rotMirMode |= 0x0;
            break;
        case 90:
            rotMirMode |= 0x3;
            break;
        case 180:
            rotMirMode |= 0x5;
            break;
        case 270:
            rotMirMode |= 0x7;
            break;
        }
    }

    if (pEncInfo->mirrorEnable == TRUE) {
        switch (pEncInfo->mirrorDirection) {
        case MIRDIR_NONE :
            rotMirMode |= 0x0;
            break;
        case MIRDIR_VER :
            rotMirMode |= 0x9;
            break;
        case MIRDIR_HOR :
            rotMirMode |= 0x11;
            break;
        case MIRDIR_HOR_VER :
            rotMirMode |= 0x19;
            break;
        }
    }

    width32 = (pOpenParam->picWidth + 31) & ~31;
    height32= (pOpenParam->picHeight+ 31) & ~31;

    if (((rotMirMode != 0) && !((pEncInfo->rotationAngle == 180) && (pEncInfo->mirrorDirection == MIRDIR_HOR_VER))) && ((pOpenParam->picWidth != width32) || (pOpenParam->picHeight != height32)))  // if rot/mir enable && pic size is not 32-aligned, set crop info.
        CalcEncCropInfo(pHevc, rotMirMode, pOpenParam->picWidth, pOpenParam->picHeight);

    /* SET_PARAM + COMMON */
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_OPTION, OPT_COMMON);      
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_ENABLE, (unsigned int)ENC_CHANGE_SET_PARAM_ALL);    // it doesn't need to set for WAVE520

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SRC_SIZE,   pOpenParam->picHeight<<16 | pOpenParam->picWidth);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MAP_ENDIAN, VDI_LITTLE_ENDIAN);  // [fix me]

    regVal = (pHevc->profile<<0)                    |
             (pHevc->level<<3)                      |   
             (pHevc->tier<<12)                      | 
             (pHevc->internalBitDepth<<14)          | 
             (pHevc->useLongTerm<<21)               | 
             (pHevc->scalingListEnable<<22)         |
             (pHevc->tmvpEnable<<23)                |
             (pHevc->saoEnable<<24)                 |
             (pHevc->skipIntraTrans<<25)            |
             (pHevc->strongIntraSmoothEnable<<27);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SYNTAX_SPS_PARAM,  regVal);

    regVal = (pHevc->losslessEnable)                |
             (pHevc->constIntraPredFlag<<1)         | 
             (pHevc->lfCrossSliceBoundaryEnable<<2) | 
             (pHevc->weightPredEnable<<3)           | 
             (pHevc->wppEnable<<4)                  | 
             (pHevc->disableDeblk<<5)               | 
             ((pHevc->betaOffsetDiv2&0xF)<<6)       | 
             ((pHevc->tcOffsetDiv2&0xF)<<10);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SYNTAX_PPS_PARAM,  regVal);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_GOP_PARAM,  pHevc->gopPresetIdx);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_PARAM, (pHevc->decodingRefreshType<<0) | (pHevc->intraQP<<3) | (pHevc->intraPeriod<<16));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CONF_WIN_TOP_BOT, pHevc->confWinBot<<16 | pHevc->confWinTop);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CONF_WIN_LEFT_RIGHT, pHevc->confWinRight<<16 | pHevc->confWinLeft);

    regVal  = (pHevc->useRecommendEncParam)     |
              (pHevc->coefClearDisable<<4)      |
              (pHevc->cuSizeMode<<5)            |
              (pHevc->intraNxNEnable<<8)        |
              (pHevc->maxNumMerge<<18)          |
              (pHevc->customMDEnable<<20)       |
              (pHevc->customLambdaEnable<<21)   |
              (pHevc->monochromeEnable<<22);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RDO_PARAM, regVal);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INDEPENDENT_SLICE, pHevc->independSliceModeArg<<16 | pHevc->independSliceMode);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_DEPENDENT_SLICE, pHevc->dependSliceModeArg<<16 | pHevc->dependSliceMode);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_REFRESH, pHevc->intraRefreshArg<<16 | pHevc->intraRefreshMode);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_FRAME_RATE, pOpenParam->frameRateInfo);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_TARGET_RATE, pOpenParam->bitRate);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_PARAM,   (pOpenParam->rcEnable<<0)           |
                                                    (pHevc->cuLevelRCEnable<<1)         |
                                                    (pHevc->hvsQPEnable<<2)             |
                                                    (pHevc->hvsQpScaleEnable<<3)        |
                                                    (pHevc->hvsQpScale<<4)              |
                                                    (pHevc->bitAllocMode<<8)            |
                                                    (pHevc->ctuOptParam.roiEnable<<13)  |
                                                    ((pHevc->initialRcQp&0x3F)<<14)     |
                                                    (pOpenParam->initialDelay<<20));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_MIN_MAX_QP, (pHevc->minQp<<0)                    |
                                                       (pHevc->maxQp<<6)                    |
                                                       (pHevc->maxDeltaQp<<12)              |
                                                       ((pHevc->chromaCbQpOffset&0x1F)<<18)  |
                                                       ((pHevc->chromaCrQpOffset&0x1F)<<23));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_0_3, (pHevc->fixedBitRatio[0]<<0)  |
                                                                (pHevc->fixedBitRatio[1]<<8)  |  
                                                                (pHevc->fixedBitRatio[2]<<16) |  
                                                                (pHevc->fixedBitRatio[3]<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_4_7, (pHevc->fixedBitRatio[4]<<0)  |
                                                                (pHevc->fixedBitRatio[5]<<8)  |  
                                                                (pHevc->fixedBitRatio[6]<<16) |  
                                                                (pHevc->fixedBitRatio[7]<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_ROT_PARAM,  rotMirMode);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NUM_UNITS_IN_TICK, pHevc->numUnitsInTick);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_TIME_SCALE, pHevc->timeScale);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NUM_TICKS_POC_DIFF_ONE, pHevc->numTicksPocDiffOne);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU04, (pHevc->pu04DeltaRate&0xFF)                 |
                                                        ((pHevc->pu04IntraPlanarDeltaRate&0xFF)<<8) |
                                                        ((pHevc->pu04IntraDcDeltaRate&0xFF)<<16)    |
                                                        ((pHevc->pu04IntraAngleDeltaRate&0xFF)<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU08, (pHevc->pu08DeltaRate&0xFF)                 |
                                                        ((pHevc->pu08IntraPlanarDeltaRate&0xFF)<<8) |
                                                        ((pHevc->pu08IntraDcDeltaRate&0xFF)<<16)    |
                                                        ((pHevc->pu08IntraAngleDeltaRate&0xFF)<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU16, (pHevc->pu16DeltaRate&0xFF)                 |
                                                        ((pHevc->pu16IntraPlanarDeltaRate&0xFF)<<8) |
                                                        ((pHevc->pu16IntraDcDeltaRate&0xFF)<<16)    |
                                                        ((pHevc->pu16IntraAngleDeltaRate&0xFF)<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU32, (pHevc->pu32DeltaRate&0xFF)                 |
                                                        ((pHevc->pu32IntraPlanarDeltaRate&0xFF)<<8) |
                                                        ((pHevc->pu32IntraDcDeltaRate&0xFF)<<16)    |
                                                        ((pHevc->pu32IntraAngleDeltaRate&0xFF)<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU08, (pHevc->cu08IntraDeltaRate&0xFF)        |
                                                        ((pHevc->cu08InterDeltaRate&0xFF)<<8)   |
                                                        ((pHevc->cu08MergeDeltaRate&0xFF)<<16));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU16, (pHevc->cu16IntraDeltaRate&0xFF)        |
                                                        ((pHevc->cu16InterDeltaRate&0xFF)<<8)   |
                                                        ((pHevc->cu16MergeDeltaRate&0xFF)<<16));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU32, (pHevc->cu32IntraDeltaRate&0xFF)        |
                                                        ((pHevc->cu32InterDeltaRate&0xFF)<<8)   |
                                                        ((pHevc->cu32MergeDeltaRate&0xFF)<<16));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NR_PARAM,   (pHevc->nrYEnable<<0)       |
                                                    (pHevc->nrCbEnable<<1)      |
                                                    (pHevc->nrCrEnable<<2)      |
                                                    (pHevc->nrNoiseEstEnable<<3)|
                                                    (pHevc->nrNoiseSigmaY<<4)   |
                                                    (pHevc->nrNoiseSigmaCb<<11) | 
                                                    (pHevc->nrNoiseSigmaCr<<18));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NR_WEIGHT,  (pHevc->nrIntraWeightY<<0)  |
                                                    (pHevc->nrIntraWeightCb<<5) |
                                                    (pHevc->nrIntraWeightCr<<10)|
                                                    (pHevc->nrInterWeightY<<15) |
                                                    (pHevc->nrInterWeightCb<<20)|
                                                    (pHevc->nrInterWeightCr<<25));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_BG_PARAM, (pHevc->bgDetectEnable)       |
                                                  (pHevc->bgThrDiff<<1)         |
                                                  (pHevc->bgThrMeanDiff<<10)    |
                                                  (pHevc->bgLambdaQp<<18)       |
                                                  ((pHevc->bgDeltaQp&0x1F)<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_LAMBDA_ADDR, pHevc->customLambdaAddr);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_USER_SCALING_LIST_ADDR, pHevc->userScalingListAddr);

    Wave5BitIssueCommand(instance, W5_ENC_SET_PARAM);

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        if (instance->loggingEnable)
            vdi_log(coreIdx, W5_ENC_SET_PARAM, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == 0) {
        if (VpuReadReg(coreIdx, W5_RET_FAIL_REASON) == 1) {
            return RETCODE_QUEUEING_FAILURE;
        }
        return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncGetSeqInfo(CodecInst* instance, EncInitialInfo* info)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      regVal, seqInitErrReason;
    EncInfo*    pEncInfo;

    pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);

    // Send QUERY cmd
    ret = SendQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        return RETCODE_QUERY_FAILURE;
    }

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_INIT_SEQ, 0);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16)&0xff;
    pEncInfo->totalQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_ENC_ENCODING_SUCCESS) != 1) {
        seqInitErrReason = VpuReadReg(instance->coreIdx, W5_RET_ENC_ERR_INFO);
        if (seqInitErrReason == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            ret = RETCODE_MEMORY_ACCESS_VIOLATION;
        else
            ret = RETCODE_FAILURE;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DONE_INSTANCE_INFO);

    info->minFrameBufferCount   = VpuReadReg(instance->coreIdx, W5_RET_ENC_MIN_FB_NUM);
    info->minSrcFrameCount      = VpuReadReg(instance->coreIdx, W5_RET_ENC_MIN_SRC_BUF_NUM);

    return ret;
}

RetCode Wave5VpuEncRegisterFramebuffer(CodecInst* inst, FrameBuffer* fbArr, TiledMapType mapType, Uint32 count)
{
    RetCode      ret = RETCODE_SUCCESS;
    Int32        q, j, i, remain, idx, bufHeight = 0, bufWidth = 0, coreIdx, startNo, endNo, stride;
    Uint32       regVal=0, picSize=0, mvColSize, fbcYTblSize, fbcCTblSize, subSampledSize;
    Uint32       endian, nv21=0, cbcrInterleave = 0, lumaStride, chromaStride;
    Uint32       addrY, addrCb, addrCr;
    vpu_buffer_t vbBuffer;
    EncOpenParam*   pOpenParam;
    EncInfo*     pEncInfo = &inst->CodecInfo->encInfo;
    pOpenParam = &pEncInfo->openParam;


    coreIdx        = inst->coreIdx;
    mvColSize      = fbcYTblSize = fbcCTblSize = 0;
    stride         = pEncInfo->stride;
    
    bufWidth  = VPU_ALIGN8(pOpenParam->picWidth);
    bufHeight = VPU_ALIGN8(pOpenParam->picHeight);

    if ((pEncInfo->rotationAngle != 0 || pEncInfo->mirrorDirection != 0) && !(pEncInfo->rotationAngle == 180 && pEncInfo->mirrorDirection == MIRDIR_HOR_VER)) {
        bufWidth  = VPU_ALIGN32(pOpenParam->picWidth);
        bufHeight = VPU_ALIGN32(pOpenParam->picHeight);
    }

    if (pEncInfo->rotationAngle == 90 || pEncInfo->rotationAngle == 270) {
        bufWidth  = VPU_ALIGN32(pOpenParam->picHeight);
        bufHeight = VPU_ALIGN32(pOpenParam->picWidth);
    }

    picSize = (bufWidth<<16) | bufHeight;

    if (mapType == COMPRESSED_FRAME_MAP) {
        nv21 = 0;
        cbcrInterleave = 0;
        mvColSize          = WAVE5_ENC_HEVC_MVCOL_BUF_SIZE(bufWidth, bufHeight);

        mvColSize          = VPU_ALIGN16(mvColSize);
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((mvColSize*count+4095)&~4095)+4096;   /* 4096 is a margin */

        if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;

        pEncInfo->vbMV = vbBuffer;

        fbcYTblSize        = WAVE5_FBC_LUMA_TABLE_SIZE(bufWidth, bufHeight);
        fbcYTblSize        = VPU_ALIGN16(fbcYTblSize);
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcYTblSize*count+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;

        pEncInfo->vbFbcYTbl = vbBuffer;

        fbcCTblSize        = WAVE5_FBC_CHROMA_TABLE_SIZE(bufWidth, bufHeight);
        fbcCTblSize        = VPU_ALIGN16(fbcCTblSize);
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcCTblSize*count+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;

        pEncInfo->vbFbcCTbl = vbBuffer;

        subSampledSize          = WAVE5_SUBSAMPLED_ONE_SIZE(bufWidth, bufHeight);
        vbBuffer.size           = ((subSampledSize*count+4095)&~4095)+4096;
        vbBuffer.phys_addr      = 0;
        if (vdi_allocate_dma_memory(coreIdx, &vbBuffer) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;
        pEncInfo->vbSubSamBuf   = vbBuffer;
    } 

    VpuWriteReg(coreIdx, W5_ADDR_SUB_SAMPLED_FB_BASE, pEncInfo->vbSubSamBuf.phys_addr);     // set sub-sampled buffer base addr
    VpuWriteReg(coreIdx, W5_SUB_SAMPLED_ONE_FB_SIZE, subSampledSize);           // set sub-sampled buffer size for one frame
    
    endian = vdi_convert_endian(coreIdx, fbArr[0].endian) & VDI_128BIT_ENDIAN_MASK;

    VpuWriteReg(coreIdx, W5_PIC_SIZE, picSize);

    // set stride of Luma/Chroma for compressed buffer
    if ((pEncInfo->rotationAngle != 0 || pEncInfo->mirrorDirection != 0) && !(pEncInfo->rotationAngle == 180 && pEncInfo->mirrorDirection == MIRDIR_HOR_VER)){ 
        lumaStride = VPU_ALIGN32(bufWidth)*(pOpenParam->EncStdParam.hevcParam.internalBitDepth >8 ? 5 : 4);
        lumaStride = VPU_ALIGN32(lumaStride);
        chromaStride = VPU_ALIGN16(bufWidth/2)*(pOpenParam->EncStdParam.hevcParam.internalBitDepth >8 ? 5 : 4);
        chromaStride = VPU_ALIGN32(chromaStride);
    }
    else {
        lumaStride = VPU_ALIGN16(pOpenParam->picWidth)*(pOpenParam->EncStdParam.hevcParam.internalBitDepth >8 ? 5 : 4);
        lumaStride = VPU_ALIGN32(lumaStride);

        chromaStride = VPU_ALIGN16(pOpenParam->picWidth/2)*(pOpenParam->EncStdParam.hevcParam.internalBitDepth >8 ? 5 : 4);
        chromaStride = VPU_ALIGN32(chromaStride); 
    }

    VpuWriteReg(coreIdx, W5_FBC_STRIDE, lumaStride<<16 | chromaStride);

    cbcrInterleave = pOpenParam->cbcrInterleave;

    stride = pEncInfo->stride;

    regVal =(nv21 << 29)                            |
            (cbcrInterleave << 16)                  |  
            (stride);

    VpuWriteReg(coreIdx, W5_COMMON_PIC_INFO, regVal);

    remain = count;
    q      = (remain+7)/8;
    idx    = 0;
    for (j=0; j<q; j++) {
        regVal = (endian<<16) | (j==q-1)<<4 | ((j==0)<<3) ;
        VpuWriteReg(coreIdx, W5_SFB_OPTION, regVal);
        startNo = j*8;
        endNo   = startNo + (remain>=8 ? 8 : remain) - 1;

        VpuWriteReg(coreIdx, W5_SET_FB_NUM, (startNo<<8)|endNo);

        for (i=0; i<8 && i<remain; i++) {
            if (mapType == LINEAR_FRAME_MAP && pEncInfo->openParam.cbcrOrder == CBCR_ORDER_REVERSED) {
                addrY  = fbArr[i+startNo].bufY;
                addrCb = fbArr[i+startNo].bufCr;
                addrCr = fbArr[i+startNo].bufCb;
            }
            else {
                addrY  = fbArr[i+startNo].bufY;
                addrCb = fbArr[i+startNo].bufCb;
                addrCr = fbArr[i+startNo].bufCr;
            }
            VpuWriteReg(coreIdx, W5_ADDR_LUMA_BASE0  + (i<<4), addrY);
            VpuWriteReg(coreIdx, W5_ADDR_CB_BASE0    + (i<<4), addrCb);
            APIDPRINT("REGISTER FB[%02d] Y(0x%08x), Cb(0x%08x) ", i, addrY, addrCb);
            if (mapType == COMPRESSED_FRAME_MAP) {
                VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET0 + (i<<4), pEncInfo->vbFbcYTbl.phys_addr+idx*fbcYTblSize); /* Luma FBC offset table */
                VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), pEncInfo->vbFbcCTbl.phys_addr+idx*fbcCTblSize);        /* Chroma FBC offset table */
                VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), pEncInfo->vbMV.phys_addr+idx*mvColSize);
                APIDPRINT("Yo(0x%08x) Co(0x%08x), Mv(0x%08x)\n",
                    pEncInfo->vbFbcYTbl.phys_addr+idx*fbcYTblSize,
                    pEncInfo->vbFbcCTbl.phys_addr+idx*fbcCTblSize,
                    pEncInfo->vbMV.phys_addr+idx*mvColSize);
            }
            else {
                VpuWriteReg(coreIdx, W5_ADDR_CR_BASE0 + (i<<4), addrCr);
                VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), 0);
                VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), 0);
                APIDPRINT("Cr(0x%08x)\n", addrCr);
            }
            idx++;
        }
        remain -= i;

        Wave5BitIssueCommand(inst, W5_SET_FB);
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
    }

    regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    if (ConfigSecAXIWave(coreIdx, inst->codecMode, 
        &pEncInfo->secAxiInfo, pOpenParam->picWidth, pOpenParam->picHeight, 
        pOpenParam->EncStdParam.hevcParam.profile, pOpenParam->EncStdParam.hevcParam.level) == 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
    }

    return ret;
}

RetCode Wave5VpuEncode(CodecInst* instance, EncParam* option)
{
    Int32           coreIdx, srcFrameFormat, srcPixelFormat, packedFormat;
    Uint32          regVal = 0, bsEndian;
    Uint32          srcStrideC = 0;
    EncInfo*        pEncInfo;
    FrameBuffer*    pSrcFrame;
    EncOpenParam*   pOpenParam;
    BOOL            justified = W4_WTL_RIGHT_JUSTIFIED;
    Uint32          formatNo  = W4_WTL_PIXEL_8BIT;

    coreIdx     = instance->coreIdx;
    pEncInfo    = VPU_HANDLE_TO_ENCINFO(instance);
    pOpenParam  = &pEncInfo->openParam;
    pSrcFrame   = option->sourceFrame;


    switch (pOpenParam->srcFormat) {
    case FORMAT_420_P10_16BIT_MSB: 
    case FORMAT_YUYV_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
        justified = W4_WTL_RIGHT_JUSTIFIED; 
        formatNo  = W4_WTL_PIXEL_16BIT;
        break;
    case FORMAT_420_P10_16BIT_LSB: 
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
        justified = W4_WTL_LEFT_JUSTIFIED; 
        formatNo  = W4_WTL_PIXEL_16BIT;
        break;
    case FORMAT_420_P10_32BIT_MSB: 
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
        justified = W4_WTL_RIGHT_JUSTIFIED; 
        formatNo  = W4_WTL_PIXEL_32BIT;
        break;
    case FORMAT_420_P10_32BIT_LSB: 
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        justified = W4_WTL_LEFT_JUSTIFIED;
        formatNo  = W4_WTL_PIXEL_32BIT;
        break;
    case FORMAT_420:
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
        justified = W4_WTL_LEFT_JUSTIFIED;
        formatNo  = W4_WTL_PIXEL_8BIT;
        break;
    }
    packedFormat = (pOpenParam->packedFormat >= 1) ? 1 : 0;

    srcFrameFormat = packedFormat<<2   | 
        pOpenParam->cbcrInterleave<<1  | 
        pOpenParam->nv21;

    switch (pOpenParam->packedFormat) {     // additional packed format (interleave & nv21 bit are used to present these modes)
    case PACKED_YVYU:
        srcFrameFormat = 0x5;
        break;
    case PACKED_UYVY:
        srcFrameFormat = 0x6;
        break;
    case PACKED_VYUY:
        srcFrameFormat = 0x7;
        break;
    default:
        break;
    }

    srcPixelFormat = justified<<2 | formatNo;

    regVal = vdi_convert_endian(coreIdx, pOpenParam->streamEndian);
    /* NOTE: When endian mode is 0, SDMA reads MSB first */
    bsEndian = (~regVal&VDI_128BIT_ENDIAN_MASK);

    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_START_ADDR, option->picStreamBufferAddr);
    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_SIZE, option->picStreamBufferSize);
    pEncInfo->streamRdPtr = option->picStreamBufferAddr;
    pEncInfo->streamWrPtr = option->picStreamBufferAddr;		
    pEncInfo->streamBufStartAddr = option->picStreamBufferAddr;
    pEncInfo->streamBufSize = option->picStreamBufferSize;
    pEncInfo->streamBufEndAddr = option->picStreamBufferAddr + option->picStreamBufferSize;		

    VpuWriteReg(coreIdx, W5_BS_OPTION, (pEncInfo->lineBufIntEn<<6) | bsEndian);

    /* Secondary AXI */
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_USE_SEC_AXI,  (pEncInfo->secAxiInfo.u.wave4.useEncRdoEnable<<11) | (pEncInfo->secAxiInfo.u.wave4.useEncLfEnable<<15));

     //VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_REPORT_PARAM, ); // FIX ME
     VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_REPORT_ENDIAN, VDI_128BIT_LITTLE_ENDIAN); // FIX ME

    if (option->codeOption.implicitHeaderEncode == 1) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_CODE_OPTION, CODEOPT_ENC_HEADER_IMPLICIT | CODEOPT_ENC_VCL | // implicitly encode a header(headers) for generating bitstream. (to encode a header only, use ENC_PUT_VIDEO_HEADER for GiveCommand)
                                                        (option->codeOption.encodeAUD<<5)              |
                                                        (option->codeOption.encodeEOS<<6)              |
                                                        (option->codeOption.encodeEOB<<7));    
    } 
    else {
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_CODE_OPTION, (option->codeOption.implicitHeaderEncode<<0)   |
                                                         (option->codeOption.encodeVCL<<1)              |
                                                         (option->codeOption.encodeVPS<<2)              |
                                                         (option->codeOption.encodeSPS<<3)              |
                                                         (option->codeOption.encodePPS<<4)              |
                                                         (option->codeOption.encodeAUD<<5)              |
                                                         (option->codeOption.encodeEOS<<6)              |
                                                         (option->codeOption.encodeEOB<<7)              |
                                                         (option->codeOption.encodeFiller<<8));                                  
    }

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_PIC_PARAM,  (option->skipPicture<<0)         |
                                                    (option->forcePicQpEnable<<1)    |
                                                    (option->forcePicQpI<<2)         |
                                                    (option->forcePicQpP<<8)         |
                                                    (option->forcePicQpB<<14)        |
                                                    (option->forcePicTypeEnable<<20) |
                                                    (option->forcePicType<<21)       |
                                                    (option->forceAllCtuCoefDropEnable<<24));

    if (option->srcEndFlag == 1)
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_PIC_IDX, 0xFFFFFFFF);               // no more source image.
    else
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_PIC_IDX, option->srcIdx);

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_ADDR_Y, pSrcFrame->bufY);
    if (pOpenParam->cbcrOrder == CBCR_ORDER_NORMAL) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_ADDR_U, pSrcFrame->bufCb);
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_ADDR_V, pSrcFrame->bufCr);
    }
    else {
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_ADDR_U, pSrcFrame->bufCr);
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_ADDR_V, pSrcFrame->bufCb);
    }

    if (formatNo == W4_WTL_PIXEL_32BIT) {
        srcStrideC = VPU_ALIGN16(pSrcFrame->stride/2)*(1<<pSrcFrame->cbcrInterleave);
        if ( pSrcFrame->cbcrInterleave == 1)
            srcStrideC = pSrcFrame->stride;
    }
    else {
        srcStrideC = (pSrcFrame->cbcrInterleave == 1) ? pSrcFrame->stride : (pSrcFrame->stride>>1);
    }

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_STRIDE, (pSrcFrame->stride<<16) | srcStrideC );

    regVal = vdi_convert_endian(coreIdx, pOpenParam->sourceEndian);
    bsEndian = (~regVal&VDI_128BIT_ENDIAN_MASK);

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_FORMAT, (srcFrameFormat<<0)  |
                                                    (srcPixelFormat<<3)  |
                                                    (bsEndian<<6));
    
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_CUSTOM_MAP_OPTION_ADDR, option->customMapOpt.addrCustomMap);

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_CUSTOM_MAP_OPTION_PARAM,  (option->customMapOpt.customRoiMapEnable << 0)    |    
                                                                  (option->customMapOpt.roiAvgQp << 1)              |
                                                                  (option->customMapOpt.customLambdaMapEnable<< 8)  |
                                                                  (option->customMapOpt.customModeMapEnable<< 9)    |
                                                                  (option->customMapOpt.customCoefDropEnable<< 10));

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_LONGTERM_PIC, (option->useCurSrcAsLongtermPic<<0) | (option->useLongtermRef<<1));

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_VAR_Y,  option->wpPixVarianceY);
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_VAR_CB, option->wpPixVarianceCb);
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_VAR_CR, option->wpPixVarianceCr);
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_MEAN, (option->wpPixMeanCr<<20) | (option->wpPixMeanCb<<10) | option->wpPixMeanY);

    Wave5BitIssueCommand(instance, W5_ENC_PIC);

    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_ENC_PIC, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16)&0xff;
    pEncInfo->totalQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal == 1) {
            return RETCODE_QUEUEING_FAILURE;
        }
        else if ( regVal == 16) {
            return RETCODE_CP0_EXCEPTION;
        }
        else {
            return RETCODE_FAILURE;
        }
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncGetResult(CodecInst* instance, EncOutputInfo* result)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      encodingSuccess, errorReason = 0;
    Uint32      regVal;
    Int32       coreIdx;
    EncInfo*    pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);

    coreIdx = instance->coreIdx;

    ret = SendQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        if (VpuReadReg(coreIdx, W5_RET_FAIL_REASON) == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else
            return RETCODE_QUERY_FAILURE;
    }
    if (instance->loggingEnable)
        vdi_log(coreIdx, W5_ENC_PIC, 0);

    regVal = VpuReadReg(coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16)&0xff;
    pEncInfo->totalQueueCount    = (regVal & 0xffff);
    
    encodingSuccess = VpuReadReg(coreIdx, W5_RET_ENC_ENCODING_SUCCESS);
    if (encodingSuccess == FALSE) {
        errorReason = VpuReadReg(coreIdx, W5_RET_ENC_ERR_INFO);
        if (errorReason == WAVE5_SYSERR_ACCESS_VIOLATION_HW) {
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        }
        return RETCODE_FAILURE;
    }

    result->encPicCnt       = VpuReadReg(coreIdx, W5_RET_ENC_PIC_NUM);
    regVal= VpuReadReg(coreIdx, W5_RET_ENC_PIC_TYPE);
    result->picType         = regVal & 0xFFFF;
    result->encVclNal       = VpuReadReg(coreIdx, W5_RET_ENC_VCL_NUT);
    result->reconFrameIndex = VpuReadReg(coreIdx, W5_RET_ENC_PIC_IDX);

    if (result->reconFrameIndex >= 0)
        result->reconFrame  = pEncInfo->frameBufPool[result->reconFrameIndex];

    result->numOfSlices     = VpuReadReg(coreIdx, W5_RET_ENC_PIC_SLICE_NUM);
    result->picSkipped      = VpuReadReg(coreIdx, W5_RET_ENC_PIC_SKIP);
    result->numOfIntra      = VpuReadReg(coreIdx, W5_RET_ENC_PIC_NUM_INTRA);
    result->numOfMerge      = VpuReadReg(coreIdx, W5_RET_ENC_PIC_NUM_MERGE);
    result->numOfSkipBlock  = VpuReadReg(coreIdx, W5_RET_ENC_PIC_NUM_SKIP);
    result->bitstreamWrapAround = 0;    // wave520 only support line-buffer mode.

    result->avgCtuQp        = VpuReadReg(coreIdx, W5_RET_ENC_PIC_AVG_CTU_QP);
    result->encPicByte      = VpuReadReg(coreIdx, W5_RET_ENC_PIC_BYTE);
    result->encGopPicIdx    = VpuReadReg(coreIdx, W5_RET_ENC_GOP_PIC_IDX);
    result->encPicPoc       = VpuReadReg(coreIdx, W5_RET_ENC_PIC_POC);
    result->encSrcIdx       = VpuReadReg(coreIdx, W5_RET_ENC_USED_SRC_IDX);
    pEncInfo->streamWrPtr   = VpuReadReg(coreIdx, pEncInfo->streamWrPtrRegAddr);
    pEncInfo->streamRdPtr   = VpuReadReg(coreIdx, pEncInfo->streamRdPtrRegAddr);

    if (pEncInfo->ringBufferEnable == 0) {      
        result->bitstreamBuffer = VpuReadReg(coreIdx, pEncInfo->streamRdPtrRegAddr);
    }
    result->rdPtr = pEncInfo->streamRdPtr;
    result->wrPtr = pEncInfo->streamWrPtr;

    if (result->reconFrameIndex < 0)
        result->bitstreamSize   = 0;
    else
        result->bitstreamSize   = result->encPicByte;

    result->frameCycle              = VpuReadReg(coreIdx, W5_FRAME_CYCLE);
    result->encPrepareCycle         = VpuReadReg(coreIdx, W5_RET_ENC_PREPARE_CYCLE);
    result->encProcessingCycle      = VpuReadReg(coreIdx, W5_RET_ENC_PROCESSING_CYCLE);
    result->encEncodingCycle        = VpuReadReg(coreIdx, W5_RET_ENC_ENCODING_CYCLE);

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncGetHeader(EncHandle instance, EncHeaderParam * encHeaderParam)
{
    Int32           coreIdx;
    Uint32          regVal = 0, bsEndian;
    EncInfo*        pEncInfo;
    EncOpenParam*   pOpenParam;

    coreIdx     = instance->coreIdx;
    pEncInfo    = VPU_HANDLE_TO_ENCINFO(instance);
    pOpenParam  = &pEncInfo->openParam;

    EnterLock(coreIdx);


    regVal = vdi_convert_endian(coreIdx, pOpenParam->streamEndian);
    /* NOTE: When endian mode is 0, SDMA reads MSB first */
    bsEndian = (~regVal&VDI_128BIT_ENDIAN_MASK);

    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_START_ADDR, encHeaderParam->buf);
    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_SIZE, encHeaderParam->size);
    pEncInfo->streamRdPtr = encHeaderParam->buf;
    pEncInfo->streamWrPtr = encHeaderParam->buf;
    pEncInfo->streamBufStartAddr = encHeaderParam->buf;
    pEncInfo->streamBufSize = encHeaderParam->size;
    pEncInfo->streamBufEndAddr = encHeaderParam->buf + encHeaderParam->size;

    VpuWriteReg(coreIdx, W5_BS_OPTION, (pEncInfo->lineBufIntEn<<6) | bsEndian);

    /* Secondary AXI */
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_USE_SEC_AXI,  (pEncInfo->secAxiInfo.u.wave4.useEncImdEnable<<9)    |
                                                      (pEncInfo->secAxiInfo.u.wave4.useEncRdoEnable<<11)   |
                                                      (pEncInfo->secAxiInfo.u.wave4.useEncLfEnable<<15));

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_CODE_OPTION, encHeaderParam->headerType);
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_PIC_IDX, 0);

    Wave5BitIssueCommand(instance, W5_ENC_PIC);

    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, W5_ENC_PIC, 2);
        LeaveLock(coreIdx);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16)&0xff;
    pEncInfo->totalQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == 1) {
            LeaveLock(coreIdx);
            return RETCODE_QUEUEING_FAILURE;
        } else {
            LeaveLock(coreIdx);
            return RETCODE_FAILURE;
        }
    }

    LeaveLock(coreIdx);
    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncFiniSeq(CodecInst*  instance )
{

    Wave5BitIssueCommand(instance, W5_DESTROY_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) 
        return RETCODE_VPU_RESPONSE_TIMEOUT;

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE)
        return RETCODE_FAILURE;

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncParaChange(EncHandle instance, EncChangeParam* param)
{
    Int32           coreIdx;
    Uint32          regVal = 0;
    EncInfo*        pEncInfo;
    coreIdx    = instance->coreIdx;
    pEncInfo = &instance->CodecInfo->encInfo;

    EnterLock(coreIdx);

    /* SET_PARAM + COMMON */
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_OPTION, OPT_CHANGE_PARAM);      
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_ENABLE, param->enable_option);

    
    if (param->enable_option & ENC_SET_PPS_PARAM_CHANGE) {
        regVal = (param->losslessEnable)                |
                 (param->constIntraPredFlag<<1)         | 
                 (param->lfCrossSliceBoundaryEnable<<2) | 
                 (param->weightPredEnable<<3)           | 
                 (param->wppEnable<<4)                  | 
                 (param->disableDeblk<<5)               | 
                 ((param->betaOffsetDiv2&0xF)<<6)       | 
                 ((param->tcOffsetDiv2&0xF)<<10);

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SYNTAX_PPS_PARAM,  regVal);
    }

    if (param->enable_option & ENC_SET_RC_FRAMERATE_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_FRAME_RATE, param->frameRate);
    }

    if (param->enable_option & ENC_SET_INDEPEND_SLICE_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INDEPENDENT_SLICE, param->independSliceModeArg<<16 | param->independSliceMode);
    }

    if (param->enable_option & ENC_SET_DEPEND_SLICE_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_DEPENDENT_SLICE, param->dependSliceModeArg<<16 | param->dependSliceMode);
    }

    if (param->enable_option & ENC_SET_RDO_PARAM_CHANGE) {
        regVal  = (param->useRecommendEncParam)     |
                  (param->coefClearDisable<<4)      |
                  (param->cuSizeMode<<5)            |
                  (param->intraNxNEnable<<8)        |
                  (param->maxNumMerge<<18)          |
                  (param->customMDEnable<<20)       |
                  (param->customLambdaEnable<<11)   |
                  (param->monochromeEnable<<22);

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RDO_PARAM, regVal);

    }

    if (param->enable_option & ENC_SET_RC_TARGET_RATE_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_TARGET_RATE, param->bitRate);
    }


    if (param->enable_option & ENC_SET_RC_PARAM_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_PARAM,   (param->rcEnable<<0)                |
                                                        (param->cuLevelRCEnable<<1)         |
                                                        (param->hvsQPEnable<<2)             |
                                                        (param->hvsQpScaleEnable<<3)        |
                                                        (param->hvsQpScale<<4)              |
                                                        (param->bitAllocMode<<8)            |
                                                        (param->seqRoiEnable<<13)           |
                                                        ((param->initialRcQp &0x3F)<<14)    |
                                                        (param->initialDelay<<16));
    }

    if (param->enable_option & ENC_SET_RC_MIN_MAX_QP_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_MIN_MAX_QP, (param->minQp<<0)                    |
                                                           (param->maxQp<<6)                    |
                                                           (param->maxDeltaQp<<12)              |
                                                           ((param->chromaCbQpOffset&0x1F)<<18) |
                                                           ((param->chromaCrQpOffset&0x1F)<<23));
    }

    if (param->enable_option & ENC_SET_RC_BIT_RATIO_LAYER_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_0_3, (param->fixedBitRatio[0]<<0)  |
                                                                    (param->fixedBitRatio[1]<<8)  |  
                                                                    (param->fixedBitRatio[2]<<16) |  
                                                                    (param->fixedBitRatio[3]<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_4_7, (param->fixedBitRatio[4]<<0)  |
                                                                    (param->fixedBitRatio[5]<<8)  |  
                                                                    (param->fixedBitRatio[6]<<16) |  
                                                                    (param->fixedBitRatio[7]<<24));
    }

    if (param->enable_option & ENC_SET_BG_PARAM_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_BG_PARAM, (param->bgDetectEnable)       |
                                                      (param->bgThrDiff<<1)         |
                                                      (param->bgThrMeanDiff<<10)    |
                                                      (param->bgLambdaQp<<18)       |
                                                      ((param->bgDeltaQp&0x1F)<<24));
    }
    if (param->enable_option & ENC_SET_CUSTOM_MD_CHANGE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU04, (param->pu04DeltaRate&0xFF)                 |
                                                            ((param->pu04IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((param->pu04IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((param->pu04IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU08, (param->pu08DeltaRate&0xFF)                 |
                                                            ((param->pu08IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((param->pu08IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((param->pu08IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU16, (param->pu16DeltaRate&0xFF)                 |
                                                            ((param->pu16IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((param->pu16IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((param->pu16IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU32, (param->pu32DeltaRate&0xFF)                 |
                                                            ((param->pu32IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((param->pu32IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((param->pu32IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU08, (param->cu08IntraDeltaRate&0xFF)        |
                                                            ((param->cu08InterDeltaRate&0xFF)<<8)   |
                                                            ((param->cu08MergeDeltaRate&0xFF)<<16));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU16, (param->cu16IntraDeltaRate&0xFF)        |
                                                            ((param->cu16InterDeltaRate&0xFF)<<8)   |
                                                            ((param->cu16MergeDeltaRate&0xFF)<<16));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU32, (param->cu32IntraDeltaRate&0xFF)        |
                                                            ((param->cu32InterDeltaRate&0xFF)<<8)   |
                                                            ((param->cu32MergeDeltaRate&0xFF)<<16));
    }
     
    Wave5BitIssueCommand(instance, W5_ENC_SET_PARAM);

    if (vdi_wait_interrupt(coreIdx, VPU_ENC_TIMEOUT, W5_VPU_VINT_REASON) == -1) {
        if (instance->loggingEnable)
            vdi_log(coreIdx, W5_ENC_SET_PARAM, 2);

        LeaveLock(coreIdx);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16) & 0xFF;
    pEncInfo->totalQueueCount    = (regVal & 0xFFFF);

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == 0) {
        if (VpuReadReg(coreIdx, W5_RET_FAIL_REASON) == 1) {
            LeaveLock(coreIdx);
            return RETCODE_QUEUEING_FAILURE;
        }
        LeaveLock(coreIdx);
        return RETCODE_FAILURE;
    }

    LeaveLock(coreIdx);
    return RETCODE_SUCCESS;
}

#ifdef SUPPORT_W5ENC_BW_REPORT
RetCode Wave5VpuEncGetBwReport(EncHandle instance, EncBwMonitor* bwMon)
{
    RetCode     ret = RETCODE_SUCCESS;
    Int32       coreIdx;

    coreIdx = instance->coreIdx;

    ret = SendQuery(instance, GET_BW_REPORT);
    if (ret != RETCODE_SUCCESS) {
        if (VpuReadReg(coreIdx, W5_RET_FAIL_REASON) == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else
            return RETCODE_QUERY_FAILURE;
    }

    bwMon->prpBwRead    = VpuReadReg(coreIdx, RET_QUERY_BW_PRP_AXI_READ);
    bwMon->prpBwWrite   = VpuReadReg(coreIdx, RET_QUERY_BW_PRP_AXI_WRITE);
    bwMon->fbdYRead     = VpuReadReg(coreIdx, RET_QUERY_BW_FBD_Y_AXI_READ);
    bwMon->fbcYWrite    = VpuReadReg(coreIdx, RET_QUERY_BW_FBC_Y_AXI_WRITE);
    bwMon->fbdCRead     = VpuReadReg(coreIdx, RET_QUERY_BW_FBD_C_AXI_READ);
    bwMon->fbcCWrite    = VpuReadReg(coreIdx, RET_QUERY_BW_FBC_C_AXI_WRITE);
    bwMon->priBwRead    = VpuReadReg(coreIdx, RET_QUERY_BW_PRI_AXI_READ);
    bwMon->priBwWrite   = VpuReadReg(coreIdx, RET_QUERY_BW_PRI_AXI_WRITE);
    bwMon->secBwRead    = VpuReadReg(coreIdx, RET_QUERY_BW_SEC_AXI_READ);
    bwMon->secBwWrite   = VpuReadReg(coreIdx, RET_QUERY_BW_SEC_AXI_WRITE);
    bwMon->procBwRead   = VpuReadReg(coreIdx, RET_QUERY_BW_PROC_AXI_READ);
    bwMon->procBwWrite  = VpuReadReg(coreIdx, RET_QUERY_BW_PROC_AXI_WRITE);

    return RETCODE_SUCCESS;
}
#endif



