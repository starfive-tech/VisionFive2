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

#include "product.h"
#include "wave/wave5.h"
#include "vpuerror.h"
#include "wave/wave5_regdefine.h"
#include "misc/debug.h"

Uint32 Wave5VpuIsInit(Uint32 coreIdx)
{
    Uint32 pc;

    pc = (Uint32)VpuReadReg(coreIdx, W5_VCPU_CUR_PC);

    return pc;
}

Int32 Wave5VpuIsBusy(Uint32 coreIdx)
{
    return VpuReadReg(coreIdx, W5_VPU_BUSY_STATUS);
}

Int32 WaveVpuGetProductId(Uint32  coreIdx)
{
    Uint32  productId = PRODUCT_ID_NONE;
    Uint32  val;

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return PRODUCT_ID_NONE;

    val = VpuReadReg(coreIdx, W5_PRODUCT_NUMBER);

    switch (val) {
    case WAVE512_CODE:   productId = PRODUCT_ID_512;   break;
    case WAVE515_CODE:   productId = PRODUCT_ID_515;   break;
    case WAVE521_CODE:   productId = PRODUCT_ID_521;   break;
    case WAVE521C_CODE:  productId = PRODUCT_ID_521;   break;
    case WAVE511_CODE:   productId = PRODUCT_ID_511;   break;
    case WAVE521C_DUAL_CODE:  productId = PRODUCT_ID_521;   break;
    case WAVE517_CODE:   productId = PRODUCT_ID_517;   break;
    default:
        VLOG(ERR, "Check productId(%d)\n", val);
        break;
    }
    return productId;
}

void Wave5BitIssueCommand(CodecInst* instance, Uint32 cmd)
{
    Uint32 instanceIndex = 0;
    Uint32 codecMode     = 0;
    Uint32 coreIdx;

    if (instance == NULL) {
        return ;
    }

    instanceIndex = instance->instIndex;
    codecMode     = instance->codecMode;
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

RetCode Wave5VpuEncGiveCommand(CodecInst *pCodecInst, CodecCommand cmd, void *param)
{
    RetCode ret = RETCODE_SUCCESS;

    switch (cmd) {
    default:
        ret = RETCODE_NOT_SUPPORTED_FEATURE;
    }

    return ret;
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

        pAttr->productId       = WaveVpuGetProductId(coreIdx);
        pAttr->productVersion  = VpuReadReg(coreIdx, W5_RET_PRODUCT_VERSION);
        pAttr->fwVersion       = VpuReadReg(coreIdx, W5_RET_FW_VERSION);
        pAttr->customerId      = VpuReadReg(coreIdx, W5_RET_CUSTOMER_ID);
        pAttr->hwConfigDef0    = VpuReadReg(coreIdx, W5_RET_STD_DEF0);
        pAttr->hwConfigDef1    = VpuReadReg(coreIdx, W5_RET_STD_DEF1);
        pAttr->hwConfigFeature = VpuReadReg(coreIdx, W5_RET_CONF_FEATURE);
        pAttr->hwConfigDate    = VpuReadReg(coreIdx, W5_RET_CONF_DATE);
        pAttr->hwConfigRev     = VpuReadReg(coreIdx, W5_RET_CONF_REVISION);
        pAttr->hwConfigType    = VpuReadReg(coreIdx, W5_RET_CONF_TYPE);

        pAttr->supportHEVC10bitEnc = (pAttr->hwConfigFeature>>3)&1;
        if ( pAttr->hwConfigRev > 167455 ) {//20190321
            pAttr->supportAVC10bitEnc =  (pAttr->hwConfigFeature>>11)&1;
        } else {
            pAttr->supportAVC10bitEnc = pAttr->supportHEVC10bitEnc;
        }
        pAttr->supportGDIHW          = TRUE;
        pAttr->supportDecoders       = (1<<STD_HEVC);
        if (pAttr->productId == PRODUCT_ID_512) {
            pAttr->supportDecoders       |= (1<<STD_VP9);
        }
        if (pAttr->productId == PRODUCT_ID_515) {
            pAttr->supportDecoders       |= (1<<STD_VP9);
            pAttr->supportDecoders       |= (1<<STD_AVS2);
        }

        pAttr->supportEncoders       = 0;
        if (pAttr->productId == PRODUCT_ID_521) {
            pAttr->supportDecoders       |= (1<<STD_AVC);
            pAttr->supportEncoders       = (1<<STD_HEVC);
            pAttr->supportEncoders       |= (1<<STD_AVC);
            pAttr->supportBackbone       = TRUE;
        }

        if (pAttr->productId == PRODUCT_ID_511) {
            pAttr->supportDecoders       |= (1<<STD_AVC);
            if ( (pAttr->hwConfigDef0>>16)&1 ) {
                pAttr->supportBackbone = TRUE;
            }
        }
        if (pAttr->productId == PRODUCT_ID_517) {
            pAttr->supportDecoders |= (1 << STD_VP9);
            pAttr->supportDecoders |= (1 << STD_AVS2);
            pAttr->supportDecoders |= (1 << STD_AVC);
            pAttr->supportDecoders |= (1 << STD_AV1);
            pAttr->supportBackbone = TRUE;
        }

        pAttr->support2AlignScaler      = (BOOL)((pAttr->hwConfigDef0>>23)&0x01);
        pAttr->supportVcoreBackbone     = (BOOL)((pAttr->hwConfigDef0>>22)&0x01);
        pAttr->supportCommandQueue      = TRUE;
        pAttr->supportFBCBWOptimization = (BOOL)((pAttr->hwConfigDef1>>15)&0x01);
        pAttr->supportNewTimer          = (BOOL)((pAttr->hwConfigDef1>>27)&0x01);
        pAttr->supportWTL               = TRUE;
        pAttr->supportDualCore          = (BOOL)((pAttr->hwConfigDef1>>26)&0x01);
        pAttr->supportTiled2Linear      = FALSE;
        pAttr->supportMapTypes          = FALSE;
        pAttr->support128bitBus         = TRUE;
        pAttr->supportThumbnailMode     = TRUE;
        pAttr->supportEndianMask        = (Uint32)((1<<VDI_LITTLE_ENDIAN) | (1<<VDI_BIG_ENDIAN) | (1<<VDI_32BIT_LITTLE_ENDIAN) | (1<<VDI_32BIT_BIG_ENDIAN) | (0xffffUL<<16));
        pAttr->supportBitstreamMode     = (1<<BS_MODE_INTERRUPT) | (1<<BS_MODE_PIC_END);
        pAttr->framebufferCacheType     = FramebufCacheNone;
        pAttr->bitstreamBufferMargin    = 0;
        pAttr->maxNumVcores             = MAX_NUM_VCORE;
        pAttr->numberOfMemProtectRgns   = 10;
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
        VLOG(ERR, "Wave5VpuGetVersion timeout\n");
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == FALSE) {
        VLOG(ERR, "Wave5VpuGetVersion FALSE\n");
        return RETCODE_QUERY_FAILURE;
    }

    regVal = VpuReadReg(coreIdx, W5_RET_FW_VERSION);
    if (versionInfo != NULL) {
        *versionInfo = 0;
    }
    if (revision != NULL) {
        *revision    = regVal;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase;
    Uint32          codeSize, tempSize;
    Uint32          i, regVal, remapSize;
    Uint32          hwOption    = 0;
    RetCode         ret = RETCODE_SUCCESS;


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

    /* clear registers */

    for (i=W5_CMD_REG_BASE; i<W5_CMD_REG_END; i+=4)
    {
#if defined(SUPPORT_SW_UART) || defined(SUPPORT_SW_UART_V2)
        if (i == W5_SW_UART_STATUS)
            continue;
#endif
        VpuWriteReg(coreIdx, i, 0x00);
    }

    /* remap page size */
    remapSize = (codeSize >> 12) &0x1ff;
    regVal = 0x80000000 | (WAVE5_UPPER_PROC_AXI_ID<<20) | (0 << 16) | (W5_REMAP_CODE_INDEX<<12) | (1<<11) | remapSize;
    VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
    VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
    VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
    VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
    VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
    VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_UPPER_PROC_AXI_ID<<4) | 0);
    VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
    VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
    VpuWriteReg(coreIdx, W5_TIMEOUT_CNT, 0xffff);

    VpuWriteReg(coreIdx, W5_HW_OPTION, hwOption);

    /* Interrupt */
    // encoder
    regVal  = (1<<INT_WAVE5_ENC_SET_PARAM);
    regVal |= (1<<INT_WAVE5_ENC_PIC);
    regVal |= (1<<INT_WAVE5_BSBUF_FULL);
    regVal |= (1<<INT_WAVE5_ENC_LOW_LATENCY);
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
    regVal |= (1<<INT_WAVE5_ENC_SRC_RELEASE);
#endif
    // decoder
    regVal |= (1<<INT_WAVE5_INIT_SEQ);
    regVal |= (1<<INT_WAVE5_DEC_PIC);
    regVal |= (1<<INT_WAVE5_BSBUF_EMPTY);

    VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

    regVal = VpuReadReg(coreIdx, W5_VPU_RET_VPU_CONFIG0);
    if (((regVal>>16)&1) == 1) {
        regVal = ((WAVE5_PROC_AXI_ID << 28) |
                  (WAVE5_PRP_AXI_ID << 24) |
                  (WAVE5_FBD_Y_AXI_ID << 20) |
                  (WAVE5_FBC_Y_AXI_ID << 16) |
                  (WAVE5_FBD_C_AXI_ID << 12) |
                  (WAVE5_FBC_C_AXI_ID << 8) |
                  (WAVE5_PRI_AXI_ID << 4) |
                  (WAVE5_SEC_AXI_ID << 0));
        vdi_fio_write_register(coreIdx, W5_BACKBONE_PROG_AXI_ID, regVal);
    }

    if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
        return RETCODE_INSUFFICIENT_RESOURCE;

    VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
    VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);
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
    vpu_buffer_t vb;
    pDecInfo    = VPU_HANDLE_TO_DECINFO(instance);

    pDecInfo->streamRdPtrRegAddr      = W5_RET_DEC_BS_RD_PTR;
    pDecInfo->streamWrPtrRegAddr      = W5_BS_WR_PTR;
    pDecInfo->frameDisplayFlagRegAddr = W5_RET_DEC_DISP_IDC;
    pDecInfo->currentPC               = W5_VCPU_CUR_PC;
    pDecInfo->busyFlagAddr            = W5_VPU_BUSY_STATUS;
    if ((pAttr->supportDecoders&(1<<param->bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;
    switch (param->bitstreamFormat) {
    case STD_HEVC:
        pDecInfo->seqChangeMask = SEQ_CHANGE_ENABLE_ALL_HEVC;
        break;
    case STD_VP9:
        pDecInfo->seqChangeMask = SEQ_CHANGE_ENABLE_ALL_VP9;
        break;
    case STD_AVS2:
        pDecInfo->seqChangeMask = SEQ_CHANGE_ENABLE_ALL_AVS2;
        break;
    case STD_AVC:
        pDecInfo->seqChangeMask = SEQ_CHANGE_ENABLE_ALL_AVC;
        break;
    case STD_SVAC:
        pDecInfo->seqChangeMask = SEQ_CHANGE_ENABLE_ALL_SVAC;
        break;
    case STD_AV1:
        pDecInfo->seqChangeMask = SEQ_CHANGE_ENABLE_ALL_AV1;
        break;
    default:
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    pDecInfo->scaleWidth              = 0;
    pDecInfo->scaleHeight             = 0;

    if (param->vbWork.size > 0) {
        pDecInfo->vbWork = param->vbWork;
        pDecInfo->workBufferAllocExt = TRUE;
        vdi_attach_dma_memory(instance->coreIdx, &param->vbWork);
    }
    else {
        if (instance->productId == PRODUCT_ID_512) {
            pDecInfo->vbWork.size       = WAVE512DEC_WORKBUF_SIZE;
        }
        else if (instance->productId == PRODUCT_ID_515) {
            pDecInfo->vbWork.size       = WAVE515DEC_WORKBUF_SIZE;
        }
        else if (instance->productId == PRODUCT_ID_517) {
            pDecInfo->vbWork.size       = (Uint32)WAVE521DEC_WORKBUF_SIZE;  // FIX ME
        }
        else if (instance->productId == PRODUCT_ID_521) {
            pDecInfo->vbWork.size       = (Uint32)WAVE521DEC_WORKBUF_SIZE;  // FIX ME
        }
        else if (instance->productId == PRODUCT_ID_511) {
            pDecInfo->vbWork.size       = (Uint32)WAVE521DEC_WORKBUF_SIZE;  // FIX ME
        }
        pDecInfo->workBufferAllocExt    = FALSE;
        APIDPRINT("ALLOC MEM - WORK\n");
        if (vdi_allocate_dma_memory(instance->coreIdx, &pDecInfo->vbWork, DEC_WORK, instance->instIndex) < 0) {
            pDecInfo->vbWork.base       = 0;
            pDecInfo->vbWork.phys_addr  = 0;
            pDecInfo->vbWork.size       = 0;
            pDecInfo->vbWork.virt_addr  = 0;
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_VCORE_INFO, 1);

    VpuWriteReg(instance->coreIdx, W5_CMD_NUM_CQ_DEPTH_M1, COMMAND_QUEUE_DEPTH -1 );

    vdi_get_common_memory(instance->coreIdx, &vb);
    pDecInfo->vbTemp.phys_addr = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
    pDecInfo->vbTemp.size      = WAVE5_TEMPBUF_SIZE;

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
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork, DEC_WORK, instance->instIndex);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding into VCPU QUEUE
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork, DEC_WORK, instance->instIndex);
        ret = RETCODE_FAILURE;
    }

    pDecInfo->productCode   = VpuReadReg(instance->coreIdx, W5_PRODUCT_NUMBER);
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

    if (instance->codecMode == W_AV1_DEC) {
        bsOption |= ((pDecInfo->openParam.av1Format) << 2);
    }

    VpuWriteReg(instance->coreIdx, W5_BS_OPTION, (1UL<<31) | bsOption);

    VpuWriteReg(instance->coreIdx, W5_COMMAND_OPTION, cmdOption);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_USER_MASK, pDecInfo->userDataEnable);

    Wave5BitIssueCommand(instance, W5_INIT_SEQ);

    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_INIT_SEQ, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pDecInfo->instanceQueueCount = (regVal>>16)&0xff;
    pDecInfo->reportQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if ( regVal == WAVE5_QUEUEING_FAIL)
            ret = RETCODE_QUEUEING_FAILURE;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            ret =  RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            ret =  RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_SYSERR_DEC_VLC_BUF_FULL)
            ret = RETCODE_VLC_BUF_FULL;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            ret = RETCODE_ERROR_FW_FATAL;
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
    Uint32     progressiveFlag, interlacedFlag, outputBitDepthMinus8, frameMbsOnlyFlag = 0;

    pDecInfo->streamRdPtr = info->rdPtr = ProductVpuDecGetRdPtr(instance);

    pDecInfo->frameDisplayFlag = VpuReadReg(instance->coreIdx, W5_RET_DEC_DISP_IDC);
    /*regVal = VpuReadReg(instance->coreIdx, W4_BS_OPTION);
    pDecInfo->streamEndflag    = (regVal&0x02) ? TRUE : FALSE;*/

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_SIZE);
    info->picWidth            = ( (regVal >> 16) & 0xffff );
    info->picHeight           = ( regVal & 0xffff );
    info->minFrameBufferCount = VpuReadReg(instance->coreIdx, W5_RET_DEC_NUM_REQUIRED_FB);
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
    info->level                  = regVal & 0xff;
    interlacedFlag               = (regVal>>10) & 0x1;
    progressiveFlag              = (regVal>>11) & 0x1;
    profileCompatibilityFlag     = (regVal>>12)&0xff;
    info->profile                = (regVal >> 24)&0x1f;
    info->tier                   = (regVal >> 29)&0x01;
    outputBitDepthMinus8         = (regVal >> 30)&0x03;

    if (instance->codecMode == W_AVC_DEC) {
        frameMbsOnlyFlag             = (regVal >> 8) & 0x01;
        info->constraint_set_flag[0] = (regVal >> 16) & 0x01;
        info->constraint_set_flag[1] = (regVal >> 17) & 0x01;
        info->constraint_set_flag[2] = (regVal >> 18) & 0x01;
        info->constraint_set_flag[3] = (regVal >> 19) & 0x01;
    }

    info->fRateNumerator         = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAME_RATE_NR);
    info->fRateDenominator       = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAME_RATE_DR);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_COLOR_SAMPLE_INFO);
    info->lumaBitdepth           = (regVal>>0)&0x0f;
    info->chromaBitdepth         = (regVal>>4)&0x0f;
    info->chromaFormatIDC        = (regVal>>8)&0x0f;
    info->aspectRateInfo         = (regVal>>16)&0xff;
    info->isExtSAR               = (info->aspectRateInfo == 255 ? TRUE : FALSE);
    if (info->isExtSAR == TRUE) {
        info->aspectRateInfo     = VpuReadReg(instance->coreIdx, W5_RET_DEC_ASPECT_RATIO);  /* [0:15] - vertical size, [16:31] - horizontal size */
    }
    info->bitRate                = VpuReadReg(instance->coreIdx, W5_RET_DEC_BIT_RATE);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO);
    info->maxSubLayers = (regVal>>8)&0xff;


    if ( instance->codecMode == W_HEVC_DEC ) {
        /* Guessing Profile */
        if (info->profile == 0) {
            if ((profileCompatibilityFlag&0x06) == 0x06)        info->profile = 1;      /* Main profile */
            else if ((profileCompatibilityFlag&0x04) == 0x04)   info->profile = 2;      /* Main10 profile */
            else if ((profileCompatibilityFlag&0x08) == 0x08)   info->profile = 3;      /* Main Still Picture profile */
            else                                                info->profile = 1;      /* For old version HM */
        }

        if (progressiveFlag == 1 && interlacedFlag == 0)
            info->interlace = 0;
        else
            info->interlace = 1;
    }
    else if (instance->codecMode == W_AVS2_DEC) {
        if ((info->lumaBitdepth == 10) && (outputBitDepthMinus8 == 2))
            info->outputBitDepth = 10;
        else
            info->outputBitDepth = 8;

        if (progressiveFlag == 1)
            info->interlace = 0;
        else
            info->interlace = 1;
    }
    else if (instance->codecMode == W_AVC_DEC) {
        if (frameMbsOnlyFlag == 1)
            info->interlace = 0;
        else
            info->interlace = 0; // AVC on WAVE5 can't support interlace
    }

    info->vlcBufSize        = VpuReadReg(instance->coreIdx, W5_RET_VLC_BUF_SIZE);
    info->paramBufSize      = VpuReadReg(instance->coreIdx, W5_RET_PARAM_BUF_SIZE);
    pDecInfo->vlcBufSize    = info->vlcBufSize;
    pDecInfo->paramBufSize  = info->paramBufSize;
    return;
}

RetCode Wave5VpuDecGetSeqInfo(CodecInst* instance, DecInitialInfo* info)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      regVal, i;
    DecInfo*    pDecInfo;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_ADDR_REPORT_BASE, pDecInfo->userDataBufAddr);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_SIZE, pDecInfo->userDataBufSize);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_PARAM, VPU_USER_DATA_ENDIAN&VDI_128BIT_ENDIAN_MASK);

    // Send QUERY cmd
    ret = SendQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if (regVal == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_SYSERR_DEC_VLC_BUF_FULL)
            return RETCODE_VLC_BUF_FULL;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_QUERY_FAILURE;
    }

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_INIT_SEQ, 0);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pDecInfo->instanceQueueCount = (regVal>>16)&0xff;
    pDecInfo->reportQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_SUCCESS) != 1) {
#ifdef SUPPORT_SW_UART
         ret = RETCODE_FAILURE;
#else
        info->seqInitErrReason = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
        ret = RETCODE_FAILURE;
#endif
    }
    else {
#ifdef SUPPORT_SW_UART
        info->warnInfo = 0;
#else
        info->warnInfo = VpuReadReg(instance->coreIdx, W5_RET_DEC_WARN_INFO);
#endif
    }


    // Get Sequence Info
    info->userDataSize   = 0;
    info->userDataNum    = 0;
    info->userDataBufFull= 0;
    info->userDataHeader = VpuReadReg(instance->coreIdx, W5_RET_DEC_USERDATA_IDC);
    if (info->userDataHeader != 0) {
        regVal = info->userDataHeader;
        for (i=0; i<32; i++) {
            if (i == 1) {
                if (regVal & (1<<i))
                    info->userDataBufFull = 1;
            }
            else {
                if (regVal & (1<<i))
                    info->userDataNum++;
            }
        }
        info->userDataSize = pDecInfo->userDataBufSize;
    }

    if (instance->codecMode == W_SVAC_DEC) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO);
        info->spatialSvcEnable = (regVal>>16)&0x1;
        info->spatialSvcMode  = (regVal>>17)&0x1;
    }
    GetDecSequenceResult(instance, info);

    return ret;
}

RetCode Wave5VpuDecRegisterFramebuffer(CodecInst* inst, FrameBuffer* fbArr, TiledMapType mapType, Uint32 count)
{
    RetCode      ret = RETCODE_SUCCESS;
    DecInfo*     pDecInfo = &inst->CodecInfo->decInfo;
    DecInitialInfo* sequenceInfo = &inst->CodecInfo->decInfo.initialInfo;
    Int32        q, j, i, remain, idx, svcBLbaseIdx;
    Uint32       mvCount;
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
    Uint32       initPicWidth = 0, initPicHeight = 0;
    Uint32       scalerFlag = 0;
    Uint32       pixelOrder=1;
    Uint32       bwbFlag = (mapType == LINEAR_FRAME_MAP) ? 1 : 0;


    coreIdx        = inst->coreIdx;
    axiID          = pDecInfo->openParam.virtAxiID;
    cbcrInterleave = pDecInfo->openParam.cbcrInterleave;
    nv21           = pDecInfo->openParam.nv21;
    mvColSize      = fbcYTblSize = fbcCTblSize = 0;

    initPicWidth   = pDecInfo->initialInfo.picWidth;
    initPicHeight  = pDecInfo->initialInfo.picHeight;

    if (inst->codecMode == W_SVAC_DEC && mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL) {
        initPicWidth   = pDecInfo->initialInfo.picWidth>>1;      // BL size is half as EL
        initPicHeight  = pDecInfo->initialInfo.picHeight>>1;
        svcBLbaseIdx        = count;
    }
    else
        svcBLbaseIdx        = 0;

    if (mapType >= COMPRESSED_FRAME_MAP) {
        cbcrInterleave = 0;
        nv21           = 0;

        switch (inst->codecMode) {
        case W_HEVC_DEC:    mvColSize = WAVE5_DEC_HEVC_MVCOL_BUF_SIZE(initPicWidth, initPicHeight); break;
        case W_VP9_DEC:     mvColSize = WAVE5_DEC_VP9_MVCOL_BUF_SIZE(initPicWidth, initPicHeight); break;
        case W_AVS2_DEC:    mvColSize = WAVE5_DEC_AVS2_MVCOL_BUF_SIZE(initPicWidth, initPicHeight); break;
        case W_SVAC_DEC:    mvColSize = 0; break; // case of SVAC, mvColbuffer included in WorkBuffer due to sequence change.
        case W_AVC_DEC:     mvColSize = WAVE5_DEC_AVC_MVCOL_BUF_SIZE(initPicWidth, initPicHeight); break;
        case W_AV1_DEC:     mvColSize = WAVE5_DEC_AV1_MVCOL_BUF_SIZE(initPicWidth, initPicHeight); break;
        default:
            return RETCODE_NOT_SUPPORTED_FEATURE;
        }

        mvColSize          = VPU_ALIGN16(mvColSize);
        vbBuffer.phys_addr = 0;
        if (inst->codecMode == W_HEVC_DEC || inst->codecMode == W_AVS2_DEC || inst->codecMode == W_VP9_DEC || inst->codecMode == W_AVC_DEC || inst->codecMode == W_AV1_DEC) {
            vbBuffer.size      = ((mvColSize+4095)&~4095)+4096;   /* 4096 is a margin */
            mvCount = count;

            APIDPRINT("ALLOC MEM - MV\n");
            for (k=0  ; k<mvCount ; k++) {
                if ( pDecInfo->vbMV[k].size == 0) {
                    if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer, DEC_MV, inst->instIndex) < 0)
                        return RETCODE_INSUFFICIENT_RESOURCE;
                    pDecInfo->vbMV[k] = vbBuffer;
                }
            }
        }

        if (pDecInfo->productCode == WAVE521C_DUAL_CODE) {
            Uint32 bgs_width       = (pDecInfo->initialInfo.lumaBitdepth >8 ? 256 : 512);
            Uint32 ot_bg_width    = 1024;
            Uint32 frm_width  = VPU_CEIL(initPicWidth, 16);
            Uint32 frm_height = VPU_CEIL(initPicHeight, 16);
            Uint32 comp_frm_width = VPU_CEIL(VPU_CEIL(frm_width , 16) + 16, 16); // valid_width = align(width, 16), comp_frm_width = align(valid_width+pad_x, 16)
            Uint32 ot_frm_width   = VPU_CEIL(comp_frm_width, ot_bg_width);    // 1024 = offset table BG width

            // sizeof_offset_table()
            Uint32 ot_bg_height = 32;
            Uint32 bgs_height  = (1<<14) / bgs_width / (pDecInfo->initialInfo.lumaBitdepth >8 ? 2 : 1);
            Uint32 comp_frm_height = VPU_CEIL(VPU_CEIL(frm_height, 4) + 4, bgs_height);
            Uint32 ot_frm_height = VPU_CEIL(comp_frm_height, ot_bg_height);
            fbcYTblSize = (ot_frm_width/16) * (ot_frm_height/4) *2;
        }
        else {
            switch (inst->codecMode) {
            case W_HEVC_DEC:    fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(initPicWidth, initPicHeight); break;
            case W_VP9_DEC:     fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(initPicWidth), VPU_ALIGN64(initPicHeight)); break;
            case W_AVS2_DEC:    fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(initPicWidth, initPicHeight); break;
            case W_SVAC_DEC:    fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(VPU_ALIGN128(initPicWidth), VPU_ALIGN128(initPicHeight)); break;
            case W_AVC_DEC:     fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(initPicWidth, initPicHeight); break;
            case W_AV1_DEC:     fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(VPU_ALIGN16(initPicWidth), VPU_ALIGN8(initPicHeight)); break;
            default:
                return RETCODE_NOT_SUPPORTED_FEATURE;
            }
            fbcYTblSize        = VPU_ALIGN16(fbcYTblSize);
        }

        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcYTblSize+4095)&~4095)+4096;
        APIDPRINT("ALLOC MEM - FBC Y TBL\n");
        for (k=0  ; k<count ; k++) {
            if ( pDecInfo->vbFbcYTbl[k+svcBLbaseIdx].size == 0) {
                if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer, DEC_FBCY_TBL, inst->instIndex) < 0)
                    return RETCODE_INSUFFICIENT_RESOURCE;
                pDecInfo->vbFbcYTbl[k+svcBLbaseIdx] = vbBuffer;
            }
        }

        if (pDecInfo->productCode == WAVE521C_DUAL_CODE) {
            Uint32 bgs_width       = (pDecInfo->initialInfo.chromaBitdepth >8 ? 256 : 512);
            Uint32 ot_bg_width    = 1024;
            Uint32 frm_width  = VPU_CEIL(initPicWidth, 16);
            Uint32 frm_height = VPU_CEIL(initPicHeight, 16);
            Uint32 comp_frm_width = VPU_CEIL(VPU_CEIL(frm_width/2 , 16) + 16, 16); // valid_width = align(width, 16), comp_frm_width = align(valid_width+pad_x, 16)
            Uint32 ot_frm_width   = VPU_CEIL(comp_frm_width, ot_bg_width);    // 1024 = offset table BG width

            // sizeof_offset_table()
            Uint32 ot_bg_height = 32;
            Uint32 bgs_height  = (1<<14) / bgs_width / (pDecInfo->initialInfo.chromaBitdepth >8 ? 2 : 1);
            Uint32 comp_frm_height = VPU_CEIL(VPU_CEIL(frm_height, 4) + 4, bgs_height);
            Uint32 ot_frm_height = VPU_CEIL(comp_frm_height, ot_bg_height);
            fbcCTblSize = (ot_frm_width/16) * (ot_frm_height/4) *2;
        }
        else {
            switch (inst->codecMode) {
            case W_HEVC_DEC:    fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(initPicWidth, initPicHeight); break;
            case W_VP9_DEC:     fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(initPicWidth), VPU_ALIGN64(initPicHeight)); break;
            case W_AVS2_DEC:    fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(initPicWidth, initPicHeight); break;
            case W_SVAC_DEC:    fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(initPicWidth), VPU_ALIGN64(initPicHeight)); break;
            case W_AVC_DEC:     fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(initPicWidth, initPicHeight); break;
            case W_AV1_DEC:     fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN16(initPicWidth), VPU_ALIGN8(initPicHeight)); break;
            default:
                return RETCODE_NOT_SUPPORTED_FEATURE;
            }
            fbcCTblSize        = VPU_ALIGN16(fbcCTblSize);
        }

        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcCTblSize+4095)&~4095)+4096;
        APIDPRINT("ALLOC MEM - FBC C TBL\n");
        for (k=0  ; k<count ; k++) {
            if ( pDecInfo->vbFbcCTbl[k+svcBLbaseIdx].size == 0) {
                if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer, DEC_FBCC_TBL, inst->instIndex) < 0)
                    return RETCODE_INSUFFICIENT_RESOURCE;
                pDecInfo->vbFbcCTbl[k+svcBLbaseIdx] = vbBuffer;
            }
        }
        picSize = (initPicWidth<<16)|(initPicHeight);

        // Allocate TaskBuffer
        vbBuffer.size       = (Uint32)((pDecInfo->vlcBufSize * VLC_BUF_NUM) + (pDecInfo->paramBufSize * COMMAND_QUEUE_DEPTH));
        vbBuffer.phys_addr  = 0;
        if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer, DEC_TASK, inst->instIndex) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;

        pDecInfo->vbTask = vbBuffer;


        VpuWriteReg(coreIdx, W5_CMD_SET_FB_ADDR_TASK_BUF, pDecInfo->vbTask.phys_addr);
        VpuWriteReg(coreIdx, W5_CMD_SET_FB_TASK_BUF_SIZE, vbBuffer.size);
    }
    else
    {
        picSize = (initPicWidth<<16)|(initPicHeight);
        if (pDecInfo->scalerEnable == TRUE) {
            picSize = (pDecInfo->scaleWidth << 16) | (pDecInfo->scaleHeight);
        }
    }
    endian = vdi_convert_endian(coreIdx, fbArr[0].endian);
    VpuWriteReg(coreIdx, W5_PIC_SIZE, picSize);

    yuvFormat = 0; /* YUV420 8bit */
    if (mapType == LINEAR_FRAME_MAP) {
        BOOL   justified = WTL_RIGHT_JUSTIFIED;
        Uint32 formatNo  = WTL_PIXEL_8BIT;
        switch (pDecInfo->wtlFormat) {
        case FORMAT_420_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_MSB:
            justified = WTL_RIGHT_JUSTIFIED;
            formatNo  = WTL_PIXEL_16BIT;
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_422_P10_16BIT_LSB:
            justified = WTL_LEFT_JUSTIFIED;
            formatNo  = WTL_PIXEL_16BIT;
            break;
        case FORMAT_420_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_MSB:
            justified = WTL_RIGHT_JUSTIFIED;
            formatNo  = WTL_PIXEL_32BIT;
            break;
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_422_P10_32BIT_LSB:
            justified = WTL_LEFT_JUSTIFIED;
            formatNo  = WTL_PIXEL_32BIT;
            break;
        default:
            break;
        }
        yuvFormat = justified<<2 | formatNo;
    }

    stride = fbArr[0].stride;
    if (mapType >= COMPRESSED_FRAME_MAP) {
        if ( pDecInfo->chFbcFrameIdx != -1 )
            stride = fbArr[pDecInfo->chFbcFrameIdx].stride;
    } else {
        if ( pDecInfo->chBwbFrameIdx != -1 )
            stride = fbArr[pDecInfo->chBwbFrameIdx].stride;
    }

    if (mapType == LINEAR_FRAME_MAP) {
        scalerFlag = pDecInfo->scalerEnable;
        switch (pDecInfo->wtlFormat) {
        case FORMAT_422:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_LSB:
        case FORMAT_422_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
            colorFormat   = 1;
            outputFormat  = 0;
            outputFormat |= (nv21 << 1);
            outputFormat |= (cbcrInterleave << 0);
            break;
        default:
            colorFormat   = 0;
            outputFormat  = 0;
            outputFormat |= (nv21 << 1);
            outputFormat |= (cbcrInterleave << 0);
            break;
        }
    }


    regVal =
        (scalerFlag   << 29) |
        (bwbFlag      << 28) |
        (axiID        << 24) |
        (pixelOrder   << 23) | /* PIXEL ORDER in 128bit. first pixel in low address */
        (yuvFormat    << 20) |
        (colorFormat  << 19) |
        (outputFormat << 16) |
        (stride);

    VpuWriteReg(coreIdx, W5_COMMON_PIC_INFO, regVal); //// 0x008012c0

    remain = count;
    q      = (remain+7)/8;
    idx    = 0;
    for (j=0; j<q; j++) {
        regVal = (endian<<16) | (j==q-1)<<4 | ((j==0)<<3);//lint !e514
        regVal |= (pDecInfo->openParam.bwOptimization<<26);
        regVal |= (pDecInfo->initialInfo.spatialSvcEnable == TRUE ) ? (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL ? 0 : 1<<27) : 0 ;
        regVal |= (pDecInfo->initialInfo.spatialSvcEnable<<28);
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
            if (mapType >= COMPRESSED_FRAME_MAP) {
                VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET0 + (i<<4), pDecInfo->vbFbcYTbl[idx+svcBLbaseIdx].phys_addr); /* Luma FBC offset table */
                VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), pDecInfo->vbFbcCTbl[idx+svcBLbaseIdx].phys_addr);        /* Chroma FBC offset table */
                VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), pDecInfo->vbMV[idx+svcBLbaseIdx].phys_addr);
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
        &pDecInfo->secAxiInfo, initPicWidth, initPicHeight,
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
    vpu_buffer_t*   pvbFbcYOffset = NULL;
    vpu_buffer_t*   pvbFbcCOffset = NULL;
    CodStd          codec;
    unsigned long   fbcYoffsetAddr = 0;
    unsigned long   fbcCoffsetAddr = 0;
    coreIdx     = inst->coreIdx;
    linearIndex = (linearFb == NULL) ? -1 : linearFb->myIndex - pDecInfo->numFbsForDecoding;
    fbcIndex    = (fbcFb    == NULL) ? -1 : fbcFb->myIndex;
    mvColSize   = fbcYTblSize = fbcCTblSize = 0;
    codec       = pDecInfo->openParam.bitstreamFormat;
    if (codec != STD_VP9) {
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    mvColSize = WAVE5_DEC_VP9_MVCOL_BUF_SIZE(picWidth, picHeight);

    if ((fbcFb != NULL) && (fbcIndex >= 0)) {
        pDecInfo->frameBufPool[fbcIndex] = *fbcFb;
    }
    if ((linearFb != NULL) && (linearIndex >= 0)) {
        pDecInfo->frameBufPool[pDecInfo->numFbsForDecoding + linearIndex] = *linearFb;
    }

    if (mvIndex >= 0) {
        pvbMv = &pDecInfo->vbMV[mvIndex];
        vdi_free_dma_memory(inst->coreIdx, pvbMv, DEC_MV, inst->instIndex);
        pvbMv->size = ((mvColSize+4095)&~4095) + 4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, pvbMv, DEC_MV, inst->instIndex) < 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }

    /* Reallocate FBC offset tables */
    if (codec == STD_HEVC){
        fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(picWidth, picHeight);
    }
    else if (codec == STD_VP9) {
        //VP9 Decoded size : 64 aligned.
        fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(picWidth), VPU_ALIGN64(picHeight));
    }
    else if (codec == STD_AVS2){
        fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(picWidth, picHeight);
    }
    else if (codec == STD_AVC){
        fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(picWidth, picHeight); // FIX ME
    }
    else if (codec == STD_AV1){
        fbcYTblSize = WAVE5_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(picWidth), VPU_ALIGN64(picHeight));
    }
    else {
        /* Unknown codec */
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    if (fbcIndex >= 0) {
        pvbFbcYOffset = &pDecInfo->vbFbcYTbl[fbcIndex];
        vdi_free_dma_memory(inst->coreIdx, pvbFbcYOffset, DEC_FBCY_TBL, inst->instIndex);
        pvbFbcYOffset->phys_addr = 0;
        pvbFbcYOffset->size      = ((fbcYTblSize+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, pvbFbcYOffset, DEC_FBCY_TBL, inst->instIndex) < 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
        fbcYoffsetAddr = pvbFbcYOffset->phys_addr;
    }

    if (codec == STD_HEVC) {
        fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(picWidth, picHeight);
    }
    else if (codec == STD_VP9) {
        fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(picWidth), VPU_ALIGN64(picHeight));
    }
    else if (codec == STD_AVS2) {
        fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(picWidth, picHeight);
    }
    else if (codec == STD_AVC) {
        fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(picWidth, picHeight); // FIX ME
    }
    else if (codec == STD_AV1) {
        fbcCTblSize = WAVE5_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(picWidth), VPU_ALIGN64(picHeight));
    }
    else {
        /* Unknown codec */
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    if (fbcIndex >= 0) {
        pvbFbcCOffset = &pDecInfo->vbFbcCTbl[fbcIndex];
        vdi_free_dma_memory(inst->coreIdx, pvbFbcCOffset, DEC_FBCC_TBL, inst->instIndex);
        pvbFbcCOffset->phys_addr = 0;
        pvbFbcCOffset->size      = ((fbcCTblSize+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, pvbFbcCOffset, DEC_FBCC_TBL, inst->instIndex) < 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
        fbcCoffsetAddr = pvbFbcCOffset->phys_addr;
    }

    linearStride = linearFb == NULL ? 0 : linearFb->stride;
    fbcStride    = fbcFb == NULL ? 0 : fbcFb->stride;
    regVal = linearStride<<16 | fbcStride;
    VpuWriteReg(coreIdx, W5_CMD_SET_FB_STRIDE, regVal);

    regVal = (picWidth<<16) | picHeight;
    if (pDecInfo->scalerEnable == TRUE) {
        regVal = (pDecInfo->scaleWidth << 16) | (pDecInfo->scaleHeight);
    }
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
    VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET,  fbcYoffsetAddr);
    VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET,  fbcCoffsetAddr);
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
        case WAVE_SKIPMODE_NON_IRAP:
            modeOption   = SKIP_NON_IRAP;
            forceLatency = 0;
            break;
        case WAVE_SKIPMODE_NON_REF:
            modeOption = SKIP_NON_REF_PIC;
            break;
        default:
            // skip off
            break;
        }
    }

    if (instance->codecMode == W_SVAC_DEC) {
        switch (option->selSvacLayer) {
            case SEL_SVAC_ALL_LAYER:
                break;
            case SEL_SVAC_BL:
                modeOption |= SKIP_SVAC_EL;
                break;
            case SEL_SVAC_EL:
                modeOption |= SKIP_SVAC_BL;
                break;
            default:
                break;
        }
    }

    if (option->craAsBlaFlag == TRUE) {
        modeOption |= (1<<5);
    }

    // set disable reorder
    if (pDecInfo->reorderEnable == FALSE) {
        forceLatency = 0;
    }

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

    VpuWriteReg(instance->coreIdx, W5_BS_RD_PTR, pDecInfo->streamRdPtr);
    VpuWriteReg(instance->coreIdx, W5_BS_WR_PTR, pDecInfo->streamWrPtr);
    if (pDecInfo->streamEndflag == 1)
        bsOption = 3;   // (streamEndFlag<<1) | EXPLICIT_END
    if (pOpenParam->bitstreamMode == BS_MODE_PIC_END || pDecInfo->rdPtrValidFlag == TRUE)
        rdptr_valid = 1;

    if (instance->codecMode == W_AV1_DEC) {
        bsOption |= ((pOpenParam->av1Format) << 2);
    }

    VpuWriteReg(instance->coreIdx, W5_BS_OPTION,  (rdptr_valid<<31) | bsOption);

    pDecInfo->rdPtrValidFlag = FALSE;       // reset rdptrValidFlag.
    /* Secondary AXI */
    regVal = (pDecInfo->secAxiInfo.u.wave.useBitEnable<<0)    |
             (pDecInfo->secAxiInfo.u.wave.useIpEnable<<9)     |
             (pDecInfo->secAxiInfo.u.wave.useLfRowEnable<<15);
    regVal |= (pDecInfo->secAxiInfo.u.wave.useSclEnable<<5);
    VpuWriteReg(instance->coreIdx, W5_USE_SEC_AXI,  regVal);

    /* Set attributes of User buffer */
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_USER_MASK, pDecInfo->userDataEnable);



    if (pDecInfo->tempIdSelectMode == FALSE)
        VpuWriteReg(instance->coreIdx, W5_CMD_DEC_TEMPORAL_ID_PLUS1, pDecInfo->tempIdSelectMode<<8 | (pDecInfo->targetSubLayerId+1));
    else
        VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REL_TEMPORAL_ID, pDecInfo->tempIdSelectMode<<8 | pDecInfo->relTargetLayerId);

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
    pDecInfo->reportQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if ( regVal == WAVE5_QUEUEING_FAIL)
            return RETCODE_QUEUEING_FAILURE;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_SYSERR_DEC_VLC_BUF_FULL)
            return RETCODE_VLC_BUF_FULL;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuDecGetResult(CodecInst* instance, DecOutputInfo* result)
{
    RetCode              ret = RETCODE_SUCCESS;
    Uint32               regVal, index, nalUnitType;
    DecInfo*             pDecInfo;
    vpu_instance_pool_t* instancePool = NULL;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_ADDR_REPORT_BASE, pDecInfo->userDataBufAddr);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_SIZE,      pDecInfo->userDataBufSize);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_PARAM,     VPU_USER_DATA_ENDIAN&VDI_128BIT_ENDIAN_MASK);

    // Send QUERY cmd
    ret = SendQuery(instance, GET_RESULT);

    if (ret != RETCODE_SUCCESS) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if (regVal == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_SYSERR_DEC_VLC_BUF_FULL)
            return RETCODE_VLC_BUF_FULL;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_QUERY_FAILURE;
    }

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_DEC_PIC, 0);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pDecInfo->instanceQueueCount = (regVal>>16)&0xff;
    pDecInfo->reportQueueCount    = (regVal & 0xffff);

    result->decodingSuccess = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_SUCCESS);
#ifdef SUPPORT_SW_UART
#else
    if (result->decodingSuccess == FALSE) {
        result->errorReason = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
    }
    else {
        result->warnInfo = VpuReadReg(instance->coreIdx, W5_RET_DEC_WARN_INFO);
    }
#endif

    result->decOutputExtData.userDataSize   = 0;
    result->decOutputExtData.userDataNum    = 0;
    result->decOutputExtData.userDataBufFull= 0;
    result->decOutputExtData.userDataHeader = VpuReadReg(instance->coreIdx, W5_RET_DEC_USERDATA_IDC);
    if (result->decOutputExtData.userDataHeader != 0) {
        regVal = result->decOutputExtData.userDataHeader;
        for (index=0; index<32; index++) {
            if (index == 1) {
                if (regVal & (1<<index))
                    result->decOutputExtData.userDataBufFull = 1;
            }
            else {
                if (regVal & (1<<index))
                    result->decOutputExtData.userDataNum++;
            }
        }
        result->decOutputExtData.userDataSize = pDecInfo->userDataBufSize;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_TYPE);

    if (instance->codecMode == W_VP9_DEC) {
        if      (regVal&0x01) result->picType = PIC_TYPE_I;
        else if (regVal&0x02) result->picType = PIC_TYPE_P;
        else if (regVal&0x04) result->picType = PIC_TYPE_REPEAT;
        else                  result->picType = PIC_TYPE_MAX;
    }
    else if (instance->codecMode == W_HEVC_DEC) {
        if      (regVal&0x04) result->picType = PIC_TYPE_B;
        else if (regVal&0x02) result->picType = PIC_TYPE_P;
        else if (regVal&0x01) result->picType = PIC_TYPE_I;
        else                  result->picType = PIC_TYPE_MAX;
    }
    else if (instance->codecMode == W_AVC_DEC) {
        if      (regVal&0x04) result->picType = PIC_TYPE_B;
        else if (regVal&0x02) result->picType = PIC_TYPE_P;
        else if (regVal&0x01) result->picType = PIC_TYPE_I;
        else                  result->picType = PIC_TYPE_MAX;
    }
    else if (instance->codecMode == W_SVAC_DEC) {
        if (regVal&0x01)      result->picType = PIC_TYPE_KEY;
        else if(regVal&0x02)  result->picType = PIC_TYPE_INTER;
        else                  result->picType = PIC_TYPE_MAX;
    }
    else if (instance->codecMode == W_AV1_DEC) {
        switch (regVal & 0x07) {
        case 0: result->picType = PIC_TYPE_KEY;        break;
        case 1: result->picType = PIC_TYPE_INTER;      break;
        case 2: result->picType = PIC_TYPE_AV1_INTRA;  break;
        case 3: result->picType = PIC_TYPE_AV1_SWITCH; break;
        default:
            result->picType = PIC_TYPE_MAX; break;
        }
    }
    else {  // AVS2
        switch(regVal&0x07) {
        case 0: result->picType = PIC_TYPE_I;      break;
        case 1: result->picType = PIC_TYPE_P;      break;
        case 2: result->picType = PIC_TYPE_B;      break;
        case 3: result->picType = PIC_TYPE_AVS2_F; break;
        case 4: result->picType = PIC_TYPE_AVS2_S; break;
        case 5: result->picType = PIC_TYPE_AVS2_G; break;
        case 6: result->picType = PIC_TYPE_AVS2_GB;break;
        default:
             result->picType = PIC_TYPE_MAX; break;
        }
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

    if (instance->codecMode == W_HEVC_DEC) {
        result->decodedPOC = -1;
        result->displayPOC = -1;
        if (result->indexFrameDecoded >= 0 || result->indexFrameDecoded  == DECODED_IDX_FLAG_SKIP)
            result->decodedPOC = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_POC);
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO);
        result->temporalId              = regVal & 0xff;
    }
    else if (instance->codecMode == W_SVAC_DEC) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO);
        result->temporalId              = regVal & 0xff;
        result->svacInfo.spatialSvcFlag = (regVal >> 16) & 0x1;
        result->svacInfo.spatialSvcMode = (regVal >> 17) & 0x1;
        result->svacInfo.spatialSvcLayer= (regVal >> 18) & 0x1;
    }
    else if (instance->codecMode == W_AVS2_DEC) {
        result->avs2Info.decodedPOI = -1;
        result->avs2Info.displayPOI = -1;
        if (result->indexFrameDecoded >= 0)
            result->avs2Info.decodedPOI = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_POC);

        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO);
        result->temporalId              = regVal & 0xff;
    }
    else if (instance->codecMode == W_AVC_DEC) {
        result->decodedPOC = -1;
        result->displayPOC = -1;
        if (result->indexFrameDecoded >= 0 || result->indexFrameDecoded  == DECODED_IDX_FLAG_SKIP)
            result->decodedPOC = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_POC);
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO);
        result->temporalId              = regVal & 0xff;
    }
    else if (instance->codecMode == W_AV1_DEC) {
        result->decodedPOC = -1;
        result->displayPOC = -1;
        if (result->indexFrameDecoded >= 0 || result->indexFrameDecoded == DECODED_IDX_FLAG_SKIP)
            result->decodedPOC = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_POC);
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO);
        result->temporalId = regVal & 0xff;
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_PARAM);
        result->av1Info.enableIntraBlockCopy = (regVal >> 0) & 0x1;
        result->av1Info.enableScreenContents = (regVal >> 1) & 0x1;
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
        if ( instance->codecMode == W_VP9_DEC ) {
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

    result->decHostCmdTick     = VpuReadReg(instance->coreIdx, W5_RET_DEC_HOST_CMD_TICK);
    result->decSeekStartTick   = VpuReadReg(instance->coreIdx, W5_RET_DEC_SEEK_START_TICK);
    result->decSeekEndTick     = VpuReadReg(instance->coreIdx, W5_RET_DEC_SEEK_END__TICK);
    result->decParseStartTick  = VpuReadReg(instance->coreIdx, W5_RET_DEC_PARSING_START_TICK);
    result->decParseEndTick    = VpuReadReg(instance->coreIdx, W5_RET_DEC_PARSING_END_TICK);
    result->decDecodeStartTick = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_START_TICK);
    result->decDecodeEndTick   = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_ENC_TICK);

    instancePool = vdi_get_instance_pool(instance->coreIdx);
    if (pDecInfo->firstCycleCheck == FALSE) {
        result->frameCycle                  = (result->decDecodeEndTick - result->decHostCmdTick)*pDecInfo->cyclePerTick;
        instancePool->lastPerformanceCycles = result->decDecodeEndTick;
        pDecInfo->firstCycleCheck           = TRUE;
    }
    else {
        if ( result->indexFrameDecodedForTiled != -1 ) {
            result->frameCycle          = (result->decDecodeEndTick - instancePool->lastPerformanceCycles)*pDecInfo->cyclePerTick;
            instancePool->lastPerformanceCycles = result->decDecodeEndTick;
            if (instancePool->lastPerformanceCycles < result->decHostCmdTick)
                result->frameCycle   = (result->decDecodeEndTick - result->decHostCmdTick);
        }
    }
    result->seekCycle    = (result->decSeekEndTick   - result->decSeekStartTick)*pDecInfo->cyclePerTick;
    result->parseCycle   = (result->decParseEndTick  - result->decParseStartTick)*pDecInfo->cyclePerTick;
    result->DecodedCycle = (result->decDecodeEndTick - result->decDecodeStartTick)*pDecInfo->cyclePerTick;

    if (0 == pDecInfo->instanceQueueCount && 0 == pDecInfo->reportQueueCount) {
        // No remaining command. Reset frame cycle.
        pDecInfo->firstCycleCheck = FALSE;
    }

    if ( result->sequenceChanged  && (instance->codecMode != W_VP9_DEC)) {
        pDecInfo->scaleWidth  = pDecInfo->newSeqInfo.picWidth;
        pDecInfo->scaleHeight = pDecInfo->newSeqInfo.picHeight;
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
        Uint32 regVal;
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if (regVal == WAVE5_VPU_STILL_RUNNING)
            return RETCODE_VPU_STILL_RUNNING;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_SYSERR_DEC_VLC_BUF_FULL)
            return RETCODE_VLC_BUF_FULL;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_QUERY_FAILURE;
    }
    else {
        DecInfo* pDecInfo = VPU_HANDLE_TO_DECINFO(instance);
        pDecInfo->instanceQueueCount = 0;
        pDecInfo->reportQueueCount   = 0;
    }

    return ret;

}

RetCode Wave5VpuReInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase;
    PhysicalAddress oldCodeBase, tempSize;
    Uint32          codeSize;
    Uint32          regVal, remapSize;

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
        VpuAttr*    pAttr = &g_VpuCoreAttributes[coreIdx];

        VpuWriteMem(coreIdx, codeBase, (unsigned char*)firmware, size*2, VDI_128BIT_LITTLE_ENDIAN);
        vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)firmware);

        regVal = 0;
        VpuWriteReg(coreIdx, W5_PO_CONF, regVal);

        if (pAttr->supportBackbone == TRUE) {
            if (pAttr->supportDualCore == TRUE) {
                // check CORE0
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x7);

                if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_BACKBONE_BUS_STATUS_VCORE0) == -1) {
                    vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
                    return RETCODE_VPU_RESPONSE_TIMEOUT;
                }

                // check CORE1
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE1, 0x7);

                if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_BACKBONE_BUS_STATUS_VCORE1) == -1) {
                    vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE1, 0x00);
                    return RETCODE_VPU_RESPONSE_TIMEOUT;
                }

            }
            else {
                if (pAttr->supportVcoreBackbone == TRUE) {
                    // Step1 : disable request
                    vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x7);

                    // Step2 : Waiting for completion of bus transaction
                    if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_BACKBONE_BUS_STATUS_VCORE0) == -1) {
                        vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
                        return RETCODE_VPU_RESPONSE_TIMEOUT;
                    }
                }
                else {
                    // Step1 : disable request
                    vdi_fio_write_register(coreIdx, W5_COMBINED_BACKBONE_BUS_CTRL, 0x7);

                    // Step2 : Waiting for completion of bus transaction
                    if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_COMBINED_BACKBONE_BUS_STATUS) == -1) {
                        vdi_fio_write_register(coreIdx, W5_COMBINED_BACKBONE_BUS_CTRL, 0x00);
                        return RETCODE_VPU_RESPONSE_TIMEOUT;
                    }
                }
            }
        }
        else {
            // Step1 : disable request
            vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x100);

            // Step2 : Waiting for completion of bus transaction
            if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_GDI_BUS_STATUS) == -1) {
                vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }
        }

        // Step3 : Waiting for completion of VCPU bus transaction
        if (vdi_wait_vcpu_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_VCPU_STATUS) == -1) {
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
        if (pAttr->supportBackbone == TRUE) {
            if (pAttr->supportDualCore == TRUE) {
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE1, 0x00);
            }
            else {
                if (pAttr->supportVcoreBackbone == TRUE) {
                    vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
                }
                else {
                    vdi_fio_write_register(coreIdx, W5_COMBINED_BACKBONE_BUS_CTRL, 0x00);
                }
            }
        }
        else {
            vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
        }

        /* remap page size */
        remapSize = (codeSize >> 12) &0x1ff;
        regVal = 0x80000000 | (WAVE5_UPPER_PROC_AXI_ID<<20) | (W5_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_UPPER_PROC_AXI_ID<<4) | 0);
        VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
        VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
        VpuWriteReg(coreIdx, W5_TIMEOUT_CNT,   0);

        VpuWriteReg(coreIdx, W5_HW_OPTION, 0);
        /* Interrupt */
        // encoder
        regVal  = (1<<INT_WAVE5_ENC_SET_PARAM);
        regVal |= (1<<INT_WAVE5_ENC_PIC);
        regVal |= (1<<INT_WAVE5_BSBUF_FULL);
        regVal |= (1<<INT_WAVE5_ENC_LOW_LATENCY);
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
        regVal |= (1<<INT_WAVE5_ENC_SRC_RELEASE);
#endif
        // decoder
        regVal  = (1<<INT_WAVE5_INIT_SEQ);
        regVal |= (1<<INT_WAVE5_DEC_PIC);
        regVal |= (1<<INT_WAVE5_BSBUF_EMPTY);

        VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

        regVal  =  ((WAVE5_PROC_AXI_ID<<28)     |
            (WAVE5_PRP_AXI_ID<<24)      |
            (WAVE5_FBD_Y_AXI_ID<<20)    |
            (WAVE5_FBC_Y_AXI_ID<<16)    |
            (WAVE5_FBD_C_AXI_ID<<12)    |
            (WAVE5_FBC_C_AXI_ID<<8)     |
            (WAVE5_PRI_AXI_ID<<4)       |
            (WAVE5_SEC_AXI_ID<<0));

        vdi_fio_write_register(coreIdx, W5_BACKBONE_PROG_AXI_ID, regVal);

        if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
            return RETCODE_INSUFFICIENT_RESOURCE;

        VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
        VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);
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
    Uint32          regVal;
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase;
    Uint32          codeSize, tempSize;
    Uint32          remapSize;

    if(iSleepWake==1)  //saves
    {
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

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
        regVal = 0x80000000 | (WAVE5_UPPER_PROC_AXI_ID<<20) | (W5_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_UPPER_PROC_AXI_ID<<4) | 0);
        VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
        VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
        VpuWriteReg(coreIdx, W5_TIMEOUT_CNT,   0);

        VpuWriteReg(coreIdx, W5_HW_OPTION, hwOption);

        // encoder
        regVal  = (1<<INT_WAVE5_ENC_SET_PARAM);
        regVal |= (1<<INT_WAVE5_ENC_PIC);
        regVal |= (1<<INT_WAVE5_BSBUF_FULL);
        regVal |= (1<<INT_WAVE5_ENC_LOW_LATENCY);
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
        regVal |= (1<<INT_WAVE5_ENC_SRC_RELEASE);
#endif
        // decoder
        regVal  = (1<<INT_WAVE5_INIT_SEQ);
        regVal |= (1<<INT_WAVE5_DEC_PIC);
        regVal |= (1<<INT_WAVE5_BSBUF_EMPTY);

        VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

        regVal  =  ((WAVE5_PROC_AXI_ID<<28)     |
            (WAVE5_PRP_AXI_ID<<24)      |
            (WAVE5_FBD_Y_AXI_ID<<20)    |
            (WAVE5_FBC_Y_AXI_ID<<16)    |
            (WAVE5_FBD_C_AXI_ID<<12)    |
            (WAVE5_FBC_C_AXI_ID<<8)     |
            (WAVE5_PRI_AXI_ID<<4)       |
            (WAVE5_SEC_AXI_ID<<0));

        vdi_fio_write_register(coreIdx, W5_BACKBONE_PROG_AXI_ID, regVal);

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
    Uint32      val = 0;
    RetCode     ret = RETCODE_SUCCESS;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[coreIdx];

    // VPU doesn't send response. Force to set BUSY flag to 0.
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 0);

    val = VpuReadReg(coreIdx, W5_VPU_RET_VPU_CONFIG0);
    if (((val>>16) & 0x1) == 0x01) {
        pAttr->supportBackbone = TRUE;
    }
    if (((val>>22) & 0x1) == 0x01) {
        pAttr->supportVcoreBackbone = TRUE;
    }

    val = VpuReadReg(coreIdx, W5_VPU_RET_VPU_CONFIG1);
    if (((val>>26) & 0x1) == 0x01) {
       pAttr->supportDualCore = TRUE;
    }


    // Waiting for completion of bus transaction
    if (pAttr->supportBackbone == TRUE) {
        if (pAttr->supportDualCore == TRUE) {
            // check CORE0
            vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x7);

            if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_BACKBONE_BUS_STATUS_VCORE0) == -1) {
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }

            // check CORE1
            vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE1, 0x7);

            if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_BACKBONE_BUS_STATUS_VCORE1) == -1) {
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE1, 0x00);
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }

        }
        else {
            if (pAttr->supportVcoreBackbone == TRUE) {
                // Step1 : disable request
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x7);

                // Step2 : Waiting for completion of bus transaction
                if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_BACKBONE_BUS_STATUS_VCORE0) == -1) {
                    vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
                    VLOG(ERR, "VpuReset Error = %d\n", pAttr->supportBackbone);
                    return RETCODE_VPU_RESPONSE_TIMEOUT;
                }
            }
            else {
                // Step1 : disable request
                vdi_fio_write_register(coreIdx, W5_COMBINED_BACKBONE_BUS_CTRL, 0x7);

                // Step2 : Waiting for completion of bus transaction
                if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_COMBINED_BACKBONE_BUS_STATUS) == -1) {
                    vdi_fio_write_register(coreIdx, W5_COMBINED_BACKBONE_BUS_CTRL, 0x00);
                    VLOG(ERR, "VpuReset Error = %d\n", pAttr->supportBackbone);
                    return RETCODE_VPU_RESPONSE_TIMEOUT;
                }
            }
        }
    }
    else {
        // Step1 : disable request
        vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x100);

        // Step2 : Waiting for completion of bus transaction
        if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_GDI_BUS_STATUS) == -1) {
            vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
            VLOG(ERR, "VpuReset Error = %d\n", pAttr->supportBackbone);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
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
    if (pAttr->supportBackbone == TRUE) {
        if (pAttr->supportDualCore == TRUE) {
            vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
            vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE1, 0x00);
        }
        else {
            if (pAttr->supportVcoreBackbone == TRUE) {
                vdi_fio_write_register(coreIdx, W5_BACKBONE_BUS_CTRL_VCORE0, 0x00);
            }
            else {
                vdi_fio_write_register(coreIdx, W5_COMBINED_BACKBONE_BUS_CTRL, 0x00);
            }
        }
    }
    else {
        vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
    }
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
        Uint32 regVal;
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL && regVal != WAVE5_VPU_STILL_RUNNING)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if (regVal == WAVE5_VPU_STILL_RUNNING)
            ret = RETCODE_VPU_STILL_RUNNING;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            ret = RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            ret = RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_SYSERR_DEC_VLC_BUF_FULL)
            ret = RETCODE_VLC_BUF_FULL;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            ret = RETCODE_ERROR_FW_FATAL;
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

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "Wave5DecClrDispFlag QUERY FAILURE\n");
        return RETCODE_QUERY_FAILURE;
    }

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
#ifdef SUPPORT_MULTI_INST_INTR
#else
    Int32  orgReason = -1;
    Int32  remain_intr = -1; // to set VPU_VINT_REASON for remain interrupt.
    Int32  ownInt      = 0;
    Uint32 regVal;
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
    Uint32 IntrMask = ((1 << INT_WAVE5_BSBUF_EMPTY) | (1 << INT_WAVE5_DEC_PIC) | (1 << INT_WAVE5_INIT_SEQ) | (1 << INT_WAVE5_ENC_SET_PARAM) | (1 << INT_WAVE5_ENC_SRC_RELEASE));
#else
    Uint32 IntrMask = ((1 << INT_WAVE5_BSBUF_EMPTY) | (1 << INT_WAVE5_DEC_PIC) | (1 << INT_WAVE5_INIT_SEQ) | (1 << INT_WAVE5_ENC_SET_PARAM));
#endif

#endif /* SUPPORT_MULTI_INST_INTR */
#ifdef SUPPORT_MULTI_INST_INTR
    // check an interrupt for my instance during timeout
    reason = vdi_wait_interrupt(instance->coreIdx, instance->instIndex, timeout);
#else
    EnterLock(instance->coreIdx);

    // check one interrupt for current instance even if the number of interrupt triggered more than one.
    if ((reason = vdi_wait_interrupt(instance->coreIdx, timeout)) > 0) {

        remain_intr = reason;

        if (reason & (1 << INT_WAVE5_BSBUF_EMPTY)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_BS_EMPTY_INST);
            regVal = (regVal & 0xffff);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_BSBUF_EMPTY);
                remain_intr &= ~(Uint32)reason;

                regVal = regVal & ~(1UL << instance->instIndex);
                VpuWriteReg(instance->coreIdx, W5_RET_BS_EMPTY_INST, regVal);
            }
        }

        if (reason & (1 << INT_WAVE5_INIT_SEQ)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
            regVal = (regVal & 0xffff);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_INIT_SEQ);
                remain_intr &= ~(Uint32)reason;

                regVal = regVal & ~(1UL << instance->instIndex);
                VpuWriteReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST, regVal);
            }
        }

        if (reason & (1 << INT_WAVE5_DEC_PIC)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
            regVal = (regVal & 0xffff);

            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                orgReason = reason;
                reason = (1 << INT_WAVE5_DEC_PIC);
                remain_intr &= ~(Uint32)reason;
                /* Clear Low Latency Interrupt if two interrupts are occured */
                if (orgReason & (1 << INT_WAVE5_ENC_LOW_LATENCY)) {
                    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
                    regVal = (regVal>>16);
                    if (regVal & (1 << instance->instIndex)) {
                        remain_intr &= ~(1<<INT_WAVE5_ENC_LOW_LATENCY);
                        Wave5VpuClearInterrupt(instance->coreIdx, 1<<INT_WAVE5_ENC_LOW_LATENCY);
                    }
                }
                regVal = regVal & ~(1UL << instance->instIndex);
                VpuWriteReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST, regVal);
            }
        }



        if (reason & (1 << INT_WAVE5_ENC_SET_PARAM)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
            regVal = (regVal & 0xffff);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_ENC_SET_PARAM);
                remain_intr &= ~(Uint32)reason;

                regVal = regVal & ~(1UL << instance->instIndex);
                VpuWriteReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST, regVal);
            }
        }
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
        if (reason & (1 << INT_WAVE5_ENC_SRC_RELEASE)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_RELEASED_SRC_INSTANCE);
            regVal = (regVal & 0xffff);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_ENC_SRC_RELEASE);
                remain_intr &= ~(Uint32)reason;

                regVal = regVal & ~(1UL << instance->instIndex);
                VpuWriteReg(instance->coreIdx, W5_RET_RELEASED_SRC_INSTANCE, regVal);
            }
        }
#endif

        if (reason & (1 << INT_WAVE5_ENC_LOW_LATENCY)) {
            regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST);
            regVal = (regVal>>16);
            if (regVal & (1 << instance->instIndex)) {
                ownInt = 1;
                reason = (1 << INT_WAVE5_ENC_LOW_LATENCY);
                remain_intr &= ~(Uint32)reason;

                regVal = regVal & ~(1UL << instance->instIndex);
                regVal = (regVal << 16);
                VpuWriteReg(instance->coreIdx, W5_RET_QUEUE_CMD_DONE_INST, regVal);
            }
        }

        if (reason & ~IntrMask) {    // when interrupt is not for empty, dec_pic, init_seq.
            regVal = VpuReadReg(instance->coreIdx, W5_RET_SEQ_DONE_INSTANCE_INFO)&0xFF;
            if (regVal == instance->instIndex) {
                ownInt = 1;
                reason = (reason & ~IntrMask);
                remain_intr &= ~(Uint32)reason;
            }
        }

        VpuWriteReg(instance->coreIdx, W5_VPU_VINT_REASON, remain_intr);     // set remain interrupt flag to trigger interrupt next time.

        if (!ownInt)
            reason = -1;    // if there was no interrupt for current instance id, reason should be -1;
    }
    LeaveLock(instance->coreIdx);
#endif
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

RetCode Wave5VpuDecGetRdPtr(CodecInst* instance, PhysicalAddress *rdPtr)
{
    RetCode ret = RETCODE_SUCCESS;

    ret = SendQuery(instance, GET_BS_RD_PTR);

    if (ret != RETCODE_SUCCESS)
        return RETCODE_QUERY_FAILURE;

    *rdPtr = VpuReadReg(instance->coreIdx, W5_RET_QUERY_DEC_BS_RD_PTR);

    return RETCODE_SUCCESS;
}


RetCode Wave5VpuGetBwReport(CodecInst* instance, VPUBWData* bwMon)
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

    bwMon->prpBwRead    = VpuReadReg(coreIdx, RET_QUERY_BW_PRP_AXI_READ)    * 16;
    bwMon->prpBwWrite   = VpuReadReg(coreIdx, RET_QUERY_BW_PRP_AXI_WRITE)   * 16;
    bwMon->fbdYRead     = VpuReadReg(coreIdx, RET_QUERY_BW_FBD_Y_AXI_READ)  * 16;
    bwMon->fbcYWrite    = VpuReadReg(coreIdx, RET_QUERY_BW_FBC_Y_AXI_WRITE) * 16;
    bwMon->fbdCRead     = VpuReadReg(coreIdx, RET_QUERY_BW_FBD_C_AXI_READ)  * 16;
    bwMon->fbcCWrite    = VpuReadReg(coreIdx, RET_QUERY_BW_FBC_C_AXI_WRITE) * 16;
    bwMon->priBwRead    = VpuReadReg(coreIdx, RET_QUERY_BW_PRI_AXI_READ)    * 16;
    bwMon->priBwWrite   = VpuReadReg(coreIdx, RET_QUERY_BW_PRI_AXI_WRITE)   * 16;
    bwMon->secBwRead    = VpuReadReg(coreIdx, RET_QUERY_BW_SEC_AXI_READ)    * 16;
    bwMon->secBwWrite   = VpuReadReg(coreIdx, RET_QUERY_BW_SEC_AXI_WRITE)   * 16;
    bwMon->procBwRead   = VpuReadReg(coreIdx, RET_QUERY_BW_PROC_AXI_READ)   * 16;
    bwMon->procBwWrite  = VpuReadReg(coreIdx, RET_QUERY_BW_PROC_AXI_WRITE)  * 16;

    return RETCODE_SUCCESS;
}


RetCode Wave5VpuGetDebugInfo(CodecInst* instance, VPUDebugInfo* info)
{
    RetCode     ret = RETCODE_SUCCESS;
    Int32       coreIdx;

    coreIdx = instance->coreIdx;

    ret = SendQuery(instance, GET_DEBUG_INFO);
    if (ret != RETCODE_SUCCESS) {
        if (VpuReadReg(coreIdx, W5_RET_FAIL_REASON) == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else
            return RETCODE_QUERY_FAILURE;
    }

    info->priReason = VpuReadReg(coreIdx, W5_RET_QUERY_DEBUG_PRI_REASON);

    return RETCODE_SUCCESS;
}



/************************************************************************/
/*                       ENCODER functions                              */
/************************************************************************/
RetCode Wave5VpuEncUpdateBS(CodecInst* instance, BOOL updateNewBsbuf)
{
    EncInfo*        pEncInfo;
    Int32           coreIdx;
    Uint32          regVal = 0, bsEndian;
    EncOpenParam*   pOpenParam;

    pEncInfo        = VPU_HANDLE_TO_ENCINFO(instance);
    pOpenParam      = &pEncInfo->openParam;
    coreIdx         = instance->coreIdx;

    regVal = vdi_convert_endian(coreIdx, pOpenParam->streamEndian);
    bsEndian = (~regVal&VDI_128BIT_ENDIAN_MASK);

    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_START_ADDR, pEncInfo->streamRdPtr);
    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_SIZE, pEncInfo->streamBufSize);

    VpuWriteReg(coreIdx, W5_BS_OPTION, (pEncInfo->lineBufIntEn<<6) | bsEndian);

    Wave5BitIssueCommand(instance, W5_UPDATE_BS);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == 0) {
        return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncGetRdWrPtr(CodecInst* instance, PhysicalAddress *rdPtr, PhysicalAddress *wrPtr)
{
    EncInfo* pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);
    RetCode ret = RETCODE_SUCCESS;

    VpuWriteReg(instance->coreIdx, W5_CMD_ENC_REASON_SEL, pEncInfo->encWrPtrSel);

    ret = SendQuery(instance, GET_BS_WR_PTR);

    if (ret != RETCODE_SUCCESS)
        return RETCODE_QUERY_FAILURE;

    *rdPtr = VpuReadReg(instance->coreIdx, W5_RET_ENC_RD_PTR);
    *wrPtr = VpuReadReg(instance->coreIdx, W5_RET_ENC_WR_PTR);

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuBuildUpEncParam(CodecInst* instance, EncOpenParam* param)
{
    RetCode     ret = RETCODE_SUCCESS;
    EncInfo*    pEncInfo;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[instance->coreIdx];
    vpu_buffer_t vb;
    Uint32      regVal = 0;
    Uint32      bsEndian;

    pEncInfo    = VPU_HANDLE_TO_ENCINFO(instance);

    pEncInfo->streamRdPtrRegAddr      = W5_RET_ENC_RD_PTR;
    pEncInfo->streamWrPtrRegAddr      = W5_RET_ENC_WR_PTR;
    pEncInfo->currentPC               = W5_VCPU_CUR_PC;
    pEncInfo->busyFlagAddr            = W5_VPU_BUSY_STATUS;

    if ((pAttr->supportEncoders&(1<<param->bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;

    if (param->bitstreamFormat == STD_HEVC)
        instance->codecMode = W_HEVC_ENC;
    else if (param->bitstreamFormat == STD_SVAC)
        instance->codecMode = W_SVAC_ENC;
    else if (param->bitstreamFormat == STD_AVC)
        instance->codecMode = W_AVC_ENC;


    vdi_get_common_memory(instance->coreIdx, &vb);
    pEncInfo->vbTemp.base      = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
    pEncInfo->vbTemp.phys_addr = pEncInfo->vbTemp.base;
    pEncInfo->vbTemp.virt_addr = pEncInfo->vbTemp.base;
    pEncInfo->vbTemp.size      = WAVE5_TEMPBUF_SIZE;

    if (instance->productId == PRODUCT_ID_521)
        pEncInfo->vbWork.size       = WAVE521ENC_WORKBUF_SIZE;

    if (vdi_allocate_dma_memory(instance->coreIdx, &pEncInfo->vbWork, ENC_WORK, instance->instIndex) < 0) {
        pEncInfo->vbWork.base       = 0;
        pEncInfo->vbWork.phys_addr  = 0;
        pEncInfo->vbWork.size       = 0;
        pEncInfo->vbWork.virt_addr  = 0;
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    vdi_clear_memory(instance->coreIdx, pEncInfo->vbWork.phys_addr, pEncInfo->vbWork.size, 0);

    VpuWriteReg(instance->coreIdx, W5_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W5_WORK_SIZE,      pEncInfo->vbWork.size);

    regVal = vdi_convert_endian(instance->coreIdx, param->streamEndian);
    bsEndian = (~regVal&VDI_128BIT_ENDIAN_MASK);

    regVal = (param->lowLatencyMode<<7) | (param->lineBufIntEn<<6) | bsEndian;
    VpuWriteReg(instance->coreIdx, W5_CMD_BS_PARAM, regVal);
    VpuWriteReg(instance->coreIdx, W5_CMD_NUM_CQ_DEPTH_M1, COMMAND_QUEUE_DEPTH -1 );

    regVal = 0;
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
    regVal |= (param->srcReleaseIntEnable<<2);
#endif
    VpuWriteReg(instance->coreIdx, W5_CMD_ENC_SRC_OPTIONS, regVal);

    VpuWriteReg(instance->coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(instance->coreIdx, W5_RET_SUCCESS, 0);	//for debug

    VpuWriteReg(instance->coreIdx, W5_CMD_ENC_VCORE_INFO, 1);

    Wave5BitIssueCommand(instance, W5_CREATE_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_CREATE_INSTANCE, 2);
        vdi_free_dma_memory(instance->coreIdx, &pEncInfo->vbWork, ENC_WORK, instance->instIndex);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {  // FAILED for adding into VCPU QUEUE
        vdi_free_dma_memory(instance->coreIdx, &pEncInfo->vbWork, ENC_WORK, instance->instIndex);
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);
        if (regVal == 2)
            ret = RETCODE_INVALID_SFS_INSTANCE;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            ret = RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            ret = RETCODE_ERROR_FW_FATAL;
        else
            ret = RETCODE_FAILURE;
    }

    pEncInfo->streamRdPtr           = param->bitstreamBuffer;
    pEncInfo->streamWrPtr           = param->bitstreamBuffer;
    pEncInfo->lineBufIntEn          = param->lineBufIntEn;
    pEncInfo->streamBufStartAddr    = param->bitstreamBuffer;
    pEncInfo->streamBufSize         = param->bitstreamBufferSize;
    pEncInfo->streamBufEndAddr      = param->bitstreamBuffer + param->bitstreamBufferSize;
    pEncInfo->streamBufTobeReadStartAddr    = param->bitstreamBuffer;
    pEncInfo->streamBufTobeReadSize         = param->bitstreamBufferSize;
    pEncInfo->streamBufTobeReadEndAddr      = param->bitstreamBuffer + param->bitstreamBufferSize;

    pEncInfo->stride                = 0;
    pEncInfo->vbFrame.size          = 0;
    pEncInfo->vbPPU.size            = 0;
    pEncInfo->frameAllocExt         = 0;
    pEncInfo->ppuAllocExt           = 0;
    pEncInfo->initialInfoObtained   = 0;
    pEncInfo->productCode           = VpuReadReg(instance->coreIdx, W5_PRODUCT_NUMBER);

    return ret;
}

RetCode Wave5VpuEncInitSeq(CodecInst* instance)
{
    Int32           coreIdx, alignedWidth = 0, alignedHeight=0;
    Uint32          regVal = 0, rotMirMode;
    EncInfo*        pEncInfo;
    EncOpenParam*   pOpenParam;
    EncWaveParam*   pParam;
    coreIdx    = instance->coreIdx;
    pEncInfo = &instance->CodecInfo->encInfo;

    pOpenParam  = &pEncInfo->openParam;
    pParam       = &pOpenParam->EncStdParam.waveParam;

    /*==============================================*/
    /*  OPT_CUSTOM_GOP                              */
    /*==============================================*/
    /*
    * SET_PARAM + CUSTOM_GOP
    * only when gopPresetIdx == custom_gop, custom_gop related registers should be set
    */
    if (pParam->gopPresetIdx == PRESET_IDX_CUSTOM_GOP) {
        int i=0, j = 0;
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_OPTION, OPT_CUSTOM_GOP);
        VpuWriteReg(coreIdx, W5_CMD_ENC_CUSTOM_GOP_PARAM, pParam->gopParam.customGopSize);

        for (i=0 ; i<pParam->gopParam.customGopSize; i++) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_CUSTOM_GOP_PIC_PARAM_0 + (i*4), (pParam->gopParam.picParam[i].picType<<0)            |
                                                                            (pParam->gopParam.picParam[i].pocOffset<<2)          |
                                                                            (pParam->gopParam.picParam[i].picQp<<6)              |
                                                                            (pParam->gopParam.picParam[i].numRefPicL0<<12)       |
                                                                            ((pParam->gopParam.picParam[i].refPocL0&0x1F)<<14)   |
                                                                            ((pParam->gopParam.picParam[i].refPocL1&0x1F)<<19)   |
                                                                            (pParam->gopParam.picParam[i].temporalId<<24));
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

    if (instance->codecMode == W_AVC_ENC) {
        alignedWidth = (pOpenParam->picWidth + 15) & ~15;
        alignedHeight= (pOpenParam->picHeight+ 15) & ~15;
    }
    else {
        alignedWidth = (pOpenParam->picWidth + 31) & ~31;
        alignedHeight= (pOpenParam->picHeight+ 31) & ~31;
    }


    if (((rotMirMode != 0) && !((pEncInfo->rotationAngle == 180) && (pEncInfo->mirrorDirection == MIRDIR_HOR_VER))) && ((pOpenParam->picWidth != alignedWidth) || (pOpenParam->picHeight != alignedHeight)))  // if rot/mir enable && pic size is not 32(16)-aligned, set crop info.
        CalcEncCropInfo(instance, pParam, rotMirMode, pOpenParam->picWidth, pOpenParam->picHeight);

    /* SET_PARAM + COMMON */
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_OPTION, OPT_COMMON);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SRC_SIZE,   pOpenParam->picHeight<<16 | pOpenParam->picWidth);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MAP_ENDIAN, VDI_LITTLE_ENDIAN);


    if (instance->codecMode == W_SVAC_ENC) {
        regVal = (pParam->profile<<0)                    |
                 (pParam->level<<3)                      |
                 (pParam->internalBitDepth<<14)          |
                 (pParam->useLongTerm<<21)               |
                 (pParam->saoEnable<<24)                 |
                 (pParam->svcEnable<<28)                 |
                 (pParam->svcMode<<29);
    }
    else if (instance->codecMode == W_AVC_ENC) {
        regVal = (pParam->profile<<0)                    |
                 (pParam->level<<3)                      |
                 (pParam->internalBitDepth<<14)          |
                 (pParam->useLongTerm<<21)               |
                 (pParam->scalingListEnable<<22) ;
    }
    else {  // HEVC enc
        regVal = (pParam->profile<<0)                    |
                 (pParam->level<<3)                      |
                 (pParam->tier<<12)                      |
                 (pParam->internalBitDepth<<14)          |
                 (pParam->useLongTerm<<21)               |
                 (pParam->scalingListEnable<<22)         |
                 (pParam->tmvpEnable<<23)                |
                 (pParam->saoEnable<<24)                 |
                 (pParam->skipIntraTrans<<25)            |
                 (pParam->strongIntraSmoothEnable<<27)   |
                 (pParam->enStillPicture<<30);
    }


    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SPS_PARAM,  regVal);

    if (instance->codecMode == W_SVAC_ENC) {
        regVal = (pParam->disableDeblk<<5)               |
                 ((pParam->chromaDcQpOffset&0x1F)<<14)   |
                 ((pParam->chromaAcQpOffset&0x1F)<<19)   |
                 ((pParam->lumaDcQpOffset&0x1F)<<24);
    }
    else {
        regVal = (pParam->losslessEnable)                |
                 (pParam->constIntraPredFlag<<1)         |
                 (pParam->lfCrossSliceBoundaryEnable<<2) |
                 ((pParam->weightPredEnable&1)<<3)       |
                 (pParam->wppEnable<<4)                  |
                 (pParam->disableDeblk<<5)               |
                 ((pParam->betaOffsetDiv2&0xF)<<6)       |
                 ((pParam->tcOffsetDiv2&0xF)<<10)        |
                 ((pParam->chromaCbQpOffset&0x1F)<<14)   |
                 ((pParam->chromaCrQpOffset&0x1F)<<19)   |
                 (pParam->transform8x8Enable<<29)        |
                 (pParam->entropyCodingMode<<30);
    }
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_PPS_PARAM,  regVal);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_GOP_PARAM,  pParam->gopPresetIdx);

    if (instance->codecMode == W_AVC_ENC) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_PARAM, (pParam->intraQP<<0) | ((pParam->intraPeriod&0x7ff)<<6) | ((pParam->avcIdrPeriod&0x7ff)<<17) | ((pParam->forcedIdrHeaderEnable&3)<<28));
    }
    else {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_PARAM, (pParam->decodingRefreshType<<0) | (pParam->intraQP<<3) | (pParam->forcedIdrHeaderEnable<<9) | (pParam->intraPeriod<<16));
    }

    regVal  = (pParam->useRecommendEncParam)     |
              (pParam->rdoSkip<<2)               |
              (pParam->lambdaScalingEnable<<3)   |
              (pParam->coefClearDisable<<4)      |
              (pParam->cuSizeMode<<5)            |
              (pParam->intraNxNEnable<<8)        |
              (pParam->maxNumMerge<<18)          |
              (pParam->customMDEnable<<20)       |
              (pParam->customLambdaEnable<<21)   |
              (pParam->monochromeEnable<<22);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RDO_PARAM, regVal);

    if (instance->codecMode == W_AVC_ENC) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_REFRESH, pParam->intraMbRefreshArg<<16 | pParam->intraMbRefreshMode);
    }
    else {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_REFRESH, pParam->intraRefreshArg<<16 | pParam->intraRefreshMode);
    }


    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_FRAME_RATE, pOpenParam->frameRateInfo);
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_TARGET_RATE, pOpenParam->bitRate);

    if (instance->codecMode == W_AVC_ENC) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_PARAM,   (pOpenParam->rcEnable<<0)           |
                                                        (pParam->mbLevelRcEnable<<1)         |
                                                        (pParam->hvsQPEnable<<2)             |
                                                        (pParam->hvsQpScale<<4)              |
                                                        (pParam->bitAllocMode<<8)            |
                                                        (pParam->roiEnable<<13)              |
                                                        ((pParam->initialRcQp&0x3F)<<14)     |
                                                        (pOpenParam->vbvBufferSize<<20));
    }
    else {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_PARAM,   (pOpenParam->rcEnable<<0)           |
                                                        (pParam->cuLevelRCEnable<<1)         |
                                                        (pParam->hvsQPEnable<<2)             |
                                                        (pParam->hvsQpScale<<4)              |
                                                        (pParam->bitAllocMode<<8)            |
                                                        (pParam->roiEnable<<13)              |
                                                        ((pParam->initialRcQp&0x3F)<<14)     |
                                                        (pOpenParam->vbvBufferSize<<20));
    }

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_WEIGHT_PARAM, pParam->rcWeightBuf<<8 | pParam->rcWeightParam);

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_MIN_MAX_QP, (pParam->minQpI<<0)                   |
                                                       (pParam->maxQpI<<6)                   |
                                                       (pParam->hvsMaxDeltaQp<<12));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_INTER_MIN_MAX_QP, (pParam->minQpP << 0)   |
                                                             (pParam->maxQpP << 6)   |
                                                             (pParam->minQpB << 12)  |
                                                             (pParam->maxQpB << 18));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_0_3, (pParam->fixedBitRatio[0]<<0)  |
                                                                (pParam->fixedBitRatio[1]<<8)  |
                                                                (pParam->fixedBitRatio[2]<<16) |
                                                                (pParam->fixedBitRatio[3]<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_4_7, (pParam->fixedBitRatio[4]<<0)  |
                                                                (pParam->fixedBitRatio[5]<<8)  |
                                                                (pParam->fixedBitRatio[6]<<16) |
                                                                (pParam->fixedBitRatio[7]<<24));

    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_ROT_PARAM,  rotMirMode);


    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_BG_PARAM, (pParam->bgDetectEnable)       |
                                                  (pParam->bgThrDiff<<1)         |
                                                  (pParam->bgThrMeanDiff<<10)    |
                                                  (pParam->bgLambdaQp<<18)       |
                                                  ((pParam->bgDeltaQp&0x1F)<<24) |
                                                  (instance->codecMode == W_AVC_ENC ? pParam->s2fmeDisable<<29 : 0));


    if (instance->codecMode == W_HEVC_ENC || instance->codecMode == W_AVC_ENC) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_LAMBDA_ADDR, pParam->customLambdaAddr);

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CONF_WIN_TOP_BOT, pParam->confWinBot<<16 | pParam->confWinTop);
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CONF_WIN_LEFT_RIGHT, pParam->confWinRight<<16 | pParam->confWinLeft);

        if (instance->codecMode == W_AVC_ENC) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INDEPENDENT_SLICE, pParam->avcSliceArg<<16 | pParam->avcSliceMode);
        }
        else {
            VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INDEPENDENT_SLICE, pParam->independSliceModeArg<<16 | pParam->independSliceMode);
        }

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_USER_SCALING_LIST_ADDR, pParam->userScalingListAddr);

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NUM_UNITS_IN_TICK, pParam->numUnitsInTick);
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_TIME_SCALE, pParam->timeScale);
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NUM_TICKS_POC_DIFF_ONE, pParam->numTicksPocDiffOne);
    }

    if (instance->codecMode == W_HEVC_ENC) {
        // SVAC encoder can't configure below parameters
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU04, (pParam->pu04DeltaRate&0xFF)                 |
                                                            ((pParam->pu04IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((pParam->pu04IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((pParam->pu04IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU08, (pParam->pu08DeltaRate&0xFF)                 |
                                                            ((pParam->pu08IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((pParam->pu08IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((pParam->pu08IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU16, (pParam->pu16DeltaRate&0xFF)                 |
                                                            ((pParam->pu16IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((pParam->pu16IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((pParam->pu16IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_PU32, (pParam->pu32DeltaRate&0xFF)                 |
                                                            ((pParam->pu32IntraPlanarDeltaRate&0xFF)<<8) |
                                                            ((pParam->pu32IntraDcDeltaRate&0xFF)<<16)    |
                                                            ((pParam->pu32IntraAngleDeltaRate&0xFF)<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU08, (pParam->cu08IntraDeltaRate&0xFF)        |
                                                            ((pParam->cu08InterDeltaRate&0xFF)<<8)   |
                                                            ((pParam->cu08MergeDeltaRate&0xFF)<<16));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU16, (pParam->cu16IntraDeltaRate&0xFF)        |
                                                            ((pParam->cu16InterDeltaRate&0xFF)<<8)   |
                                                            ((pParam->cu16MergeDeltaRate&0xFF)<<16));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_MD_CU32, (pParam->cu32IntraDeltaRate&0xFF)        |
                                                            ((pParam->cu32InterDeltaRate&0xFF)<<8)   |
                                                            ((pParam->cu32MergeDeltaRate&0xFF)<<16));


        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_DEPENDENT_SLICE, pParam->dependSliceModeArg<<16 | pParam->dependSliceMode);


        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NR_PARAM,   (pParam->nrYEnable<<0)       |
                                                        (pParam->nrCbEnable<<1)      |
                                                        (pParam->nrCrEnable<<2)      |
                                                        (pParam->nrNoiseEstEnable<<3)|
                                                        (pParam->nrNoiseSigmaY<<4)   |
                                                        (pParam->nrNoiseSigmaCb<<12) |
                                                        (pParam->nrNoiseSigmaCr<<20));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NR_WEIGHT,  (pParam->nrIntraWeightY<<0)  |
                                                        (pParam->nrIntraWeightCb<<5) |
                                                        (pParam->nrIntraWeightCr<<10)|
                                                        (pParam->nrInterWeightY<<15) |
                                                        (pParam->nrInterWeightCb<<20)|
                                                        (pParam->nrInterWeightCr<<25));

    }
#ifdef SUPPORT_LOOK_AHEAD_RC
    VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_LOOK_AHEAD_RC,  (pParam->larcEnable<<0)   |
                                                        ((pParam->larcPass-1)<<1) |
                                                        (pParam->larcSize<<2)     |
                                                        (pParam->larcWeight<<9));
#endif
    Wave5BitIssueCommand(instance, W5_ENC_SET_PARAM);

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        if (instance->loggingEnable)
            vdi_log(coreIdx, W5_ENC_SET_PARAM, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == 0) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if ( regVal == WAVE5_QUEUEING_FAIL)
            return RETCODE_QUEUEING_FAILURE;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

#ifdef SUPPORT_LOOK_AHEAD_RC
#define LARC_DATA_REG_CNT 16
RetCode Wave5VpuEncSetLarcData(CodecInst* instance, EncLarcInfo larcInfo)
{
    Int32           coreIdx;
    EncInfo*        pEncInfo;
    EncOpenParam*   pOpenParam;
    EncWaveParam*   pParam;
    Uint32          regVal = 0;
    Uint32          i, addr, k, remain;
    Uint32          quotient;
    Uint32          larcSizeMinus1;

    coreIdx    = instance->coreIdx;
    pEncInfo = &instance->CodecInfo->encInfo;
    pOpenParam  = &pEncInfo->openParam;
    pParam      = &pOpenParam->EncStdParam.waveParam;

    EnterLock(coreIdx);
    larcSizeMinus1 = pParam->larcSize-1;
    quotient    = VPU_ALIGN16(larcSizeMinus1)/LARC_DATA_REG_CNT;//16 registers

    if (pParam->larcEnable == TRUE) {
        remain = larcSizeMinus1;
        for ( k=0, i = 0 ; i < quotient ; i++ ) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_SET_PARAM_OPTION, OPT_LOOKAHEAD_PARAM_1+i);
            for (addr = W5_CMD_ENC_SEQ_LOOK_AHEAD_RC_START ; addr < W5_CMD_ENC_SEQ_LOOK_AHEAD_RC_END ;addr+=12) {
                if ( k < larcSizeMinus1) {
                    VpuWriteReg(coreIdx, addr, larcInfo.larcData[k][0]);
                    VpuWriteReg(coreIdx, addr+4, larcInfo.larcData[k][1]);
                    VpuWriteReg(coreIdx, addr+8, larcInfo.larcData[k][2]);
//                    VLOG(ERR, "PASS2 set (%d) = %x, %x, %x\n", k, larcInfo.larcData[k][0], larcInfo.larcData[k][1], larcInfo.larcData[k][2]);
                }
                else {
                    VpuWriteReg(coreIdx, addr, -1);
//                    VLOG(ERR, "PASS2 set (%d) = -1\n", k);
                }
                k++;
            }
            remain -= LARC_DATA_REG_CNT;

            Wave5BitIssueCommand(instance, W5_ENC_SET_PARAM);

            if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
                if (instance->loggingEnable)
                    vdi_log(coreIdx, W5_ENC_SET_PARAM, 2);
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }

            if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == 0) {
                regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
                if ( regVal == WAVE5_QUEUEING_FAIL)
                    return RETCODE_QUEUEING_FAILURE;
                else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
                    return RETCODE_MEMORY_ACCESS_VIOLATION;
                else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
                    return RETCODE_VPU_RESPONSE_TIMEOUT;
                else
                    return RETCODE_FAILURE;
            }
        }
    }
    LeaveLock(coreIdx);

    return RETCODE_SUCCESS;
}
#endif

RetCode Wave5VpuEncGetSeqInfo(CodecInst* instance, EncInitialInfo* info)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      regVal;
    EncInfo*    pEncInfo;

    pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);

    // Send QUERY cmd
    ret = SendQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if (regVal == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else if(regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_QUERY_FAILURE;
    }

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_INIT_SEQ, 0);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16)&0xff;
    pEncInfo->reportQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_ENC_ENCODING_SUCCESS) != 1) {
        info->seqInitErrReason = VpuReadReg(instance->coreIdx, W5_RET_ENC_ERR_INFO);
        ret = RETCODE_FAILURE;
    }
    else {
        info->warnInfo = VpuReadReg(instance->coreIdx, W5_RET_ENC_WARN_INFO);
    }

    info->minFrameBufferCount   = VpuReadReg(instance->coreIdx, W5_RET_ENC_NUM_REQUIRED_FB);
    info->minSrcFrameCount      = VpuReadReg(instance->coreIdx, W5_RET_ENC_MIN_SRC_BUF_NUM);
    info->maxLatencyPictures    = VpuReadReg(instance->coreIdx, W5_RET_ENC_PIC_MAX_LATENCY_PICTURES);
    info->vlcBufSize        = VpuReadReg(instance->coreIdx, W5_RET_VLC_BUF_SIZE);
    info->paramBufSize      = VpuReadReg(instance->coreIdx, W5_RET_PARAM_BUF_SIZE);
    pEncInfo->vlcBufSize    = info->vlcBufSize;
    pEncInfo->paramBufSize  = info->paramBufSize;

    return ret;
}

RetCode Wave5VpuEncRegisterFramebuffer(CodecInst* inst, FrameBuffer* fbArr, TiledMapType mapType, Uint32 count)
{
    RetCode      ret = RETCODE_SUCCESS;
    Int32        q, j, i, remain, idx, coreIdx, startNo, endNo, stride;
    Uint32       regVal=0, picSize=0, mvColSize, fbcYTblSize, fbcCTblSize, subSampledSize=0;
    Uint32       endian, nv21=0, cbcrInterleave = 0, lumaStride, chromaStride, bufHeight = 0, bufWidth = 0;
    Uint32       addrY, addrCb, addrCr;
    Uint32       svacMvColSize0 = 0, svacMvColSize1 = 0;
    vpu_buffer_t vbMV = {0,};
    vpu_buffer_t vbFbcYTbl = {0,};
    vpu_buffer_t vbFbcCTbl = {0,};
    vpu_buffer_t vbSubSamBuf = {0,};
    vpu_buffer_t vbTask = {0,};
    EncOpenParam*   pOpenParam;
    EncInfo*     pEncInfo = &inst->CodecInfo->encInfo;
    pOpenParam = &pEncInfo->openParam;


    coreIdx        = inst->coreIdx;
    mvColSize      = fbcYTblSize = fbcCTblSize = 0;
    stride         = pEncInfo->stride;

    bufWidth       = VPU_ALIGN8(pOpenParam->picWidth);
    bufHeight      = VPU_ALIGN8(pOpenParam->picHeight);

    if (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL) {
        bufWidth       = pOpenParam->picWidthBL;
        bufHeight      = pOpenParam->picHeightBL;
    }

    if ((pEncInfo->rotationAngle != 0 || pEncInfo->mirrorDirection != 0) && !(pEncInfo->rotationAngle == 180 && pEncInfo->mirrorDirection == MIRDIR_HOR_VER)) {
        bufWidth  = VPU_ALIGN32(pOpenParam->picWidth);
        bufHeight = VPU_ALIGN32(pOpenParam->picHeight);
    }

    if (pEncInfo->rotationAngle == 90 || pEncInfo->rotationAngle == 270) {
        bufWidth  = VPU_ALIGN32(pOpenParam->picHeight);
        bufHeight = VPU_ALIGN32(pOpenParam->picWidth);
    }

    svacMvColSize0 = WAVE5_ENC_SVAC_MVCOL_0_BUF_SIZE(bufWidth, bufHeight);
    svacMvColSize1 = WAVE5_ENC_SVAC_MVCOL_1_BUF_SIZE(bufWidth, bufHeight);

    picSize = (bufWidth<<16) | bufHeight;

    if (mapType >= COMPRESSED_FRAME_MAP) {
        nv21 = 0;
        cbcrInterleave = 0;
        if (inst->codecMode == W_SVAC_ENC) {
            mvColSize  = svacMvColSize0 + svacMvColSize1;
            vbMV.phys_addr = 0;
            vbMV.size      = ((mvColSize+4095)&~4095)+4096;   /* 4096 is a margin */
        }
        else if (inst->codecMode == W_HEVC_ENC) {
            mvColSize          = WAVE5_ENC_HEVC_MVCOL_BUF_SIZE(bufWidth, bufHeight);
            mvColSize          = VPU_ALIGN16(mvColSize);
            vbMV.phys_addr = 0;
            vbMV.size      = ((mvColSize*count+4095)&~4095)+4096;   /* 4096 is a margin */
        }
        else if (inst->codecMode == W_AVC_ENC) {
            mvColSize          = WAVE5_ENC_AVC_MVCOL_BUF_SIZE(bufWidth, bufHeight);
            vbMV.phys_addr = 0;
            vbMV.size      = ((mvColSize*count+4095)&~4095)+4096;   /* 4096 is a margin */
        }

        if (vdi_allocate_dma_memory(inst->coreIdx, &vbMV, ENC_MV, inst->instIndex) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;

        if (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL)
            pEncInfo->vbMVBL = vbMV;
        else
            pEncInfo->vbMV = vbMV;


        if (pEncInfo->productCode == WAVE521C_DUAL_CODE) {
            Uint32 bgs_width, ot_bg_width, comp_frm_width, ot_frm_width, ot_bg_height, bgs_height, comp_frm_height, ot_frm_height;
            Uint32 frm_width, frm_height;
            bgs_width = (pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 256 : 512);

            ot_bg_width    = 1024;
            frm_width      = VPU_CEIL(bufWidth, 16);
            frm_height     = VPU_CEIL(bufHeight, 16);
            comp_frm_width = VPU_CEIL(VPU_CEIL(frm_width , 16) + 16, 16); // valid_width = align(width, 16), comp_frm_width = align(valid_width+pad_x, 16)
            ot_frm_width   = VPU_CEIL(comp_frm_width, ot_bg_width);    // 1024 = offset table BG width

            // sizeof_offset_table()
            ot_bg_height = 32;
            bgs_height  = (1<<14) / bgs_width / (pOpenParam->EncStdParam.waveParam.internalBitDepth>8 ? 2 : 1);
            comp_frm_height = VPU_CEIL(VPU_CEIL(frm_height, 4) + 4, bgs_height);
            ot_frm_height = VPU_CEIL(comp_frm_height, ot_bg_height);
            fbcYTblSize = (ot_frm_width/16) * (ot_frm_height/4) *2;
        }
        else {
            fbcYTblSize        = WAVE5_FBC_LUMA_TABLE_SIZE(bufWidth, bufHeight);
            fbcYTblSize        = VPU_ALIGN16(fbcYTblSize);
        }

        vbFbcYTbl.phys_addr = 0;
        vbFbcYTbl.size      = ((fbcYTblSize*count+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, &vbFbcYTbl, ENC_FBCY_TBL, inst->instIndex) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;

        if (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL)
            pEncInfo->vbFbcYTblBL = vbFbcYTbl;
        else
            pEncInfo->vbFbcYTbl = vbFbcYTbl;

        if (pEncInfo->productCode == WAVE521C_DUAL_CODE) {
            Uint32 bgs_width, ot_bg_width, comp_frm_width, ot_frm_width, ot_bg_height, bgs_height, comp_frm_height, ot_frm_height;
            Uint32 frm_width, frm_height;
            bgs_width = (pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 256 : 512);

            ot_bg_width    = 1024;
            frm_width      = VPU_CEIL(bufWidth, 16);
            frm_height     = VPU_CEIL(bufHeight, 16);
            comp_frm_width = VPU_CEIL(VPU_CEIL(frm_width/2 , 16) + 16, 16); // valid_width = align(width, 16), comp_frm_width = align(valid_width+pad_x, 16)
            ot_frm_width   = VPU_CEIL(comp_frm_width, ot_bg_width);    // 1024 = offset table BG width

            // sizeof_offset_table()
            ot_bg_height = 32;
            bgs_height  = (1<<14) / bgs_width / (pOpenParam->EncStdParam.waveParam.internalBitDepth>8 ? 2 : 1);
            comp_frm_height = VPU_CEIL(VPU_CEIL(frm_height, 4) + 4, bgs_height);
            ot_frm_height = VPU_CEIL(comp_frm_height, ot_bg_height);
            fbcCTblSize = (ot_frm_width/16) * (ot_frm_height/4) *2;
        }
        else {
            fbcCTblSize        = WAVE5_FBC_CHROMA_TABLE_SIZE(bufWidth, bufHeight);
            fbcCTblSize        = VPU_ALIGN16(fbcCTblSize);
        }

        vbFbcCTbl.phys_addr = 0;
        vbFbcCTbl.size      = ((fbcCTblSize*count+4095)&~4095)+4096;
        if (vdi_allocate_dma_memory(inst->coreIdx, &vbFbcCTbl, ENC_FBCC_TBL, inst->instIndex) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;

        if (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL)
            pEncInfo->vbFbcCTblBL = vbFbcCTbl;
        else
            pEncInfo->vbFbcCTbl = vbFbcCTbl;

        if (pOpenParam->bitstreamFormat == STD_AVC) {
            subSampledSize          = WAVE5_AVC_SUBSAMPLED_ONE_SIZE(bufWidth, bufHeight);
        }
        else {
            subSampledSize          = WAVE5_SUBSAMPLED_ONE_SIZE(bufWidth, bufHeight);
        }
        vbSubSamBuf.size           = ((subSampledSize*count+4095)&~4095)+4096;
        vbSubSamBuf.phys_addr      = 0;
        if (vdi_allocate_dma_memory(coreIdx, &vbSubSamBuf, ENC_SUBSAMBUF, inst->instIndex) < 0)
            return RETCODE_INSUFFICIENT_RESOURCE;
        if (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL)
            pEncInfo->vbSubSamBufBL   = vbSubSamBuf;
        else
            pEncInfo->vbSubSamBuf   = vbSubSamBuf;

        vbTask.size         = (Uint32)((pEncInfo->vlcBufSize * VLC_BUF_NUM) + (pEncInfo->paramBufSize * COMMAND_QUEUE_DEPTH));
        vbTask.phys_addr    = 0;
        if (pEncInfo->vbTask.size == 0) {
            if (vdi_allocate_dma_memory(coreIdx, &vbTask, ENC_TASK, inst->instIndex) < 0)
                return RETCODE_INSUFFICIENT_RESOURCE;

            pEncInfo->vbTask = vbTask;

            VpuWriteReg(coreIdx, W5_CMD_SET_FB_ADDR_TASK_BUF, pEncInfo->vbTask.phys_addr);
            VpuWriteReg(coreIdx, W5_CMD_SET_FB_TASK_BUF_SIZE, vbTask.size);
        }
    }

    VpuWriteReg(coreIdx, W5_ADDR_SUB_SAMPLED_FB_BASE, vbSubSamBuf.phys_addr);     // set sub-sampled buffer base addr
    VpuWriteReg(coreIdx, W5_SUB_SAMPLED_ONE_FB_SIZE, subSampledSize);           // set sub-sampled buffer size for one frame

    endian = vdi_convert_endian(coreIdx, fbArr[0].endian);

    VpuWriteReg(coreIdx, W5_PIC_SIZE, picSize);

    // set stride of Luma/Chroma for compressed buffer
    if ((pEncInfo->rotationAngle != 0 || pEncInfo->mirrorDirection != 0) && !(pEncInfo->rotationAngle == 180 && pEncInfo->mirrorDirection == MIRDIR_HOR_VER)){
        lumaStride = VPU_ALIGN32(bufWidth)*(pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 5 : 4);
        lumaStride = VPU_ALIGN32(lumaStride);
        chromaStride = VPU_ALIGN16(bufWidth/2)*(pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 5 : 4);
        chromaStride = VPU_ALIGN32(chromaStride);
    }
    else {
        if (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL) {
            lumaStride = VPU_ALIGN16(pOpenParam->picWidthBL)*(pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 5 : 4);
            lumaStride = VPU_ALIGN32(lumaStride);
            chromaStride = VPU_ALIGN16(pOpenParam->picWidthBL/2)*(pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 5 : 4);
            chromaStride = VPU_ALIGN32(chromaStride);
        }
        else {
            lumaStride = VPU_ALIGN16(pOpenParam->picWidth)*(pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 5 : 4);
            lumaStride = VPU_ALIGN32(lumaStride);

            chromaStride = VPU_ALIGN16(pOpenParam->picWidth/2)*(pOpenParam->EncStdParam.waveParam.internalBitDepth >8 ? 5 : 4);
            chromaStride = VPU_ALIGN32(chromaStride);
        }
    }

    VpuWriteReg(coreIdx, W5_FBC_STRIDE, lumaStride<<16 | chromaStride);

    cbcrInterleave = pOpenParam->cbcrInterleave;

    stride = pEncInfo->stride;
    if (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL) {
        stride = VPU_ALIGN32(VPU_ALIGN32(VPU_ALIGN16(pOpenParam->picWidthBL)*5)/4);
    }
    regVal =(nv21 << 29)                            |
            (cbcrInterleave << 16)                  |
            (stride);

    VpuWriteReg(coreIdx, W5_COMMON_PIC_INFO, regVal);

    remain = count;
    q      = (remain+7)/8;
    idx    = 0;
    for (j=0; j<q; j++) {
        regVal = (endian<<16) | (j==q-1)<<4 | ((j==0)<<3);//lint !e514
        regVal |= (pOpenParam->EncStdParam.waveParam.svcEnable == TRUE) ? (mapType == COMPRESSED_FRAME_MAP_SVAC_SVC_BL ? 0 : 1 << 27) : 0;   // 0 = BL, 1 = EL
        regVal |= (pOpenParam->enableNonRefFbcWrite<< 26);
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
            if (mapType >= COMPRESSED_FRAME_MAP) {
                VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET0 + (i<<4), vbFbcYTbl.phys_addr+idx*fbcYTblSize); /* Luma FBC offset table */
                VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), vbFbcCTbl.phys_addr+idx*fbcCTblSize);        /* Chroma FBC offset table */

                if (inst->codecMode == W_SVAC_ENC) {
                    if (idx == 0) { // SVAC encoder needs only 2 mv-col buffers. (COL0 = for RDO, COL1 = for MVP)
                        VpuWriteReg(coreIdx, W5_ADDR_MV_COL0, vbMV.phys_addr);
                        VpuWriteReg(coreIdx, W5_ADDR_MV_COL1, vbMV.phys_addr + svacMvColSize0);

                        APIDPRINT("Yo(0x%08x) Co(0x%08x), Mv(0x%08x), Mv1(0x%8x)\n",
                            vbFbcYTbl.phys_addr, vbFbcCTbl.phys_addr,
                            vbMV.phys_addr, vbMV.phys_addr + svacMvColSize0);
                    }
                    else {
                        APIDPRINT("Yo(0x%08x) Co(0x%08x)\n", vbFbcYTbl.phys_addr+idx*fbcYTblSize, vbFbcCTbl.phys_addr+idx*fbcCTblSize);
                    }

                }
                else {
                    VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), vbMV.phys_addr+idx*mvColSize);

                    APIDPRINT("Yo(0x%08x) Co(0x%08x), Mv(0x%08x)\n",
                        vbFbcYTbl.phys_addr+idx*fbcYTblSize,
                        vbFbcCTbl.phys_addr+idx*fbcCTblSize,
                        vbMV.phys_addr+idx*mvColSize);
                }
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
        pOpenParam->EncStdParam.waveParam.profile, pOpenParam->EncStdParam.waveParam.level) == 0) {
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
    BOOL            justified = WTL_RIGHT_JUSTIFIED;
    Uint32          formatNo  = WTL_PIXEL_8BIT;

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
        justified = WTL_RIGHT_JUSTIFIED;
        formatNo  = WTL_PIXEL_16BIT;
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
        justified = WTL_LEFT_JUSTIFIED;
        formatNo  = WTL_PIXEL_16BIT;
        break;
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
        justified = WTL_RIGHT_JUSTIFIED;
        formatNo  = WTL_PIXEL_32BIT;
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        justified = WTL_LEFT_JUSTIFIED;
        formatNo  = WTL_PIXEL_32BIT;
        break;
    case FORMAT_420:
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
        justified = WTL_LEFT_JUSTIFIED;
        formatNo  = WTL_PIXEL_8BIT;
        break;
    default:
        return RETCODE_FAILURE;
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

    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_START_ADDR, option->picStreamBufferAddr);
    VpuWriteReg(coreIdx, W5_CMD_ENC_BS_SIZE, option->picStreamBufferSize);
    pEncInfo->streamBufStartAddr = option->picStreamBufferAddr;
    pEncInfo->streamBufSize = option->picStreamBufferSize;
    pEncInfo->streamBufEndAddr = option->picStreamBufferAddr + option->picStreamBufferSize;

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_SRC_AXI_SEL, DEFAULT_SRC_AXI);
    /* Secondary AXI */
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_USE_SEC_AXI,  (pEncInfo->secAxiInfo.u.wave.useEncRdoEnable<<11) | (pEncInfo->secAxiInfo.u.wave.useEncLfEnable<<15));

    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_REPORT_ENDIAN, VDI_128BIT_LITTLE_ENDIAN);

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
                                                    (option->forceAllCtuCoefDropEnable<<24) |
                                                    (option->svcLayerFlag<<25));


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


    if (formatNo == WTL_PIXEL_32BIT) {
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

    if (instance->codecMode == W_HEVC_ENC || instance->codecMode == W_AVC_ENC) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_SIGMA_Y,  option->wpPixSigmaY);
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_SIGMA_C, (option->wpPixSigmaCr<<16) | option->wpPixSigmaCb);
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_MEAN_Y, option->wpPixMeanY);
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_WP_PIXEL_MEAN_C, (option->wpPixMeanCr<<16) | (option->wpPixMeanCb));
    }
    else if (instance->codecMode == W_SVAC_ENC) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_LF_PARAM_0, ((option->lfRefDeltaIntra&0x7f)<<0) |
                                                        ((option->lfRefDeltaRef0&0x7f)<<7)  |
                                                        ((option->lfRefDeltaRef1&0x7f)<<14) |
                                                        ((option->lfModeDelta&0x7f)<<21)           |
                                                        (option->sharpLevel<<28));

        VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_LF_PARAM_1, (option->userFilterLevelEnable<<0) | ((option->lfFilterLevel&0x3f)<<1));
    }

#ifdef SUPPORT_LOOK_AHEAD_RC
    if (TRUE == pOpenParam->EncStdParam.waveParam.larcEnable && pOpenParam->EncStdParam.waveParam.larcPass == 2) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_LARC_RC_B, option->larcData[0]);
        VpuWriteReg(coreIdx, W5_CMD_ENC_LARC_RC_BPP, option->larcData[1]);
        VpuWriteReg(coreIdx, W5_CMD_ENC_LARC_RC_ENC_ORDER, option->larcData[2]);
    }
#endif
    Wave5BitIssueCommand(instance, W5_ENC_PIC);

    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_ENC_PIC, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16)&0xff;
    pEncInfo->reportQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if (regVal == WAVE5_QUEUEING_FAIL)
            return RETCODE_QUEUEING_FAILURE;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncGetResult(CodecInst* instance, EncOutputInfo* result)
{
    RetCode              ret = RETCODE_SUCCESS;
    Uint32               encodingSuccess;
    Uint32               regVal;
    Int32                coreIdx;
    EncInfo*             pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);
    vpu_instance_pool_t* instancePool = NULL;

    coreIdx = instance->coreIdx;


    ret = SendQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);

        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        if (regVal == WAVE5_RESULT_NOT_READY)
            return RETCODE_REPORT_NOT_READY;
        else if(regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_QUERY_FAILURE;
    }
    if (instance->loggingEnable)
        vdi_log(coreIdx, W5_ENC_PIC, 0);

    regVal = VpuReadReg(coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16)&0xff;
    pEncInfo->reportQueueCount    = (regVal & 0xffff);

    encodingSuccess = VpuReadReg(coreIdx, W5_RET_ENC_ENCODING_SUCCESS);
    if (encodingSuccess == FALSE) {
        result->errorReason = VpuReadReg(coreIdx, W5_RET_ENC_ERR_INFO);
        if (result->errorReason == WAVE5_SYSERR_ENC_VLC_BUF_FULL) {
            return RETCODE_VLC_BUF_FULL;
        }
        return RETCODE_FAILURE;
    } else {
        result->warnInfo = VpuReadReg(instance->coreIdx, W5_RET_ENC_WARN_INFO);
    }

    result->encPicCnt       = VpuReadReg(coreIdx, W5_RET_ENC_PIC_NUM);
    regVal= VpuReadReg(coreIdx, W5_RET_ENC_PIC_TYPE);
    result->picType         = regVal & 0xFFFF;

    result->encVclNut       = VpuReadReg(coreIdx, W5_RET_ENC_VCL_NUT);
    result->reconFrameIndex = VpuReadReg(coreIdx, W5_RET_ENC_PIC_IDX);

    if (result->reconFrameIndex >= 0)
        result->reconFrame  = pEncInfo->frameBufPool[result->reconFrameIndex];
    result->isSvcLayerEL    = VpuReadReg(coreIdx, W5_RET_ENC_SVC_LAYER);
    if (pEncInfo->openParam.EncStdParam.waveParam.svcEnable == 1 && result->isSvcLayerEL == FALSE) {
        if (result->reconFrameIndex >= 0)
            result->reconFrame  = pEncInfo->frameBufPool[result->reconFrameIndex+pEncInfo->numFrameBuffers];
    }

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
    result->releaseSrcFlag  = VpuReadReg(coreIdx, W5_RET_ENC_SRC_BUF_FLAG);
    pEncInfo->streamWrPtr   = VpuReadReg(coreIdx, pEncInfo->streamWrPtrRegAddr);
    pEncInfo->streamRdPtr   = VpuReadReg(coreIdx, pEncInfo->streamRdPtrRegAddr);

    result->picDistortionLow    = VpuReadReg(coreIdx, W5_RET_ENC_PIC_DIST_LOW);
    result->picDistortionHigh   = VpuReadReg(coreIdx, W5_RET_ENC_PIC_DIST_HIGH);

        result->bitstreamBuffer = VpuReadReg(coreIdx, pEncInfo->streamRdPtrRegAddr);
    result->rdPtr = pEncInfo->streamRdPtr;
    result->wrPtr = pEncInfo->streamWrPtr;

    if (result->reconFrameIndex == RECON_IDX_FLAG_HEADER_ONLY) //result for header only(no vcl) encoding
        result->bitstreamSize   = result->encPicByte;
    else if (result->reconFrameIndex < 0)
        result->bitstreamSize   = 0;
    else
        result->bitstreamSize   = result->encPicByte;

    result->encHostCmdTick             = VpuReadReg(coreIdx, W5_RET_ENC_HOST_CMD_TICK);
    result->encPrepareStartTick        = VpuReadReg(coreIdx, W5_RET_ENC_PREPARE_START_TICK);
    result->encPrepareEndTick          = VpuReadReg(coreIdx, W5_RET_ENC_PREPARE_END_TICK);
    result->encProcessingStartTick     = VpuReadReg(coreIdx, W5_RET_ENC_PROCESSING_START_TICK);
    result->encProcessingEndTick       = VpuReadReg(coreIdx, W5_RET_ENC_PROCESSING_END_TICK);
    result->encEncodeStartTick         = VpuReadReg(coreIdx, W5_RET_ENC_ENCODING_START_TICK);
    result->encEncodeEndTick           = VpuReadReg(coreIdx, W5_RET_ENC_ENCODING_END_TICK);

    instancePool = vdi_get_instance_pool(instance->coreIdx);
    if ( pEncInfo->firstCycleCheck == FALSE ) {
        result->frameCycle   = (result->encEncodeEndTick - result->encHostCmdTick) * pEncInfo->cyclePerTick;
        pEncInfo->firstCycleCheck = TRUE;
    }
    else {
        result->frameCycle   = (result->encEncodeEndTick - instancePool->lastPerformanceCycles) * pEncInfo->cyclePerTick;
        if (instancePool->lastPerformanceCycles < result->encHostCmdTick)
            result->frameCycle   = (result->encEncodeEndTick - result->encHostCmdTick) * pEncInfo->cyclePerTick;
    }
    instancePool->lastPerformanceCycles = result->encEncodeEndTick;
    result->prepareCycle = (result->encPrepareEndTick    - result->encPrepareStartTick) * pEncInfo->cyclePerTick;
    result->processing   = (result->encProcessingEndTick - result->encProcessingStartTick) * pEncInfo->cyclePerTick;
    result->EncodedCycle = (result->encEncodeEndTick     - result->encEncodeStartTick) * pEncInfo->cyclePerTick;
#ifdef SUPPORT_LOOK_AHEAD_RC
    result->larcData[0] = VpuReadReg(coreIdx, W5_RET_ENC_LARC_RC_B);
    result->larcData[1] = VpuReadReg(coreIdx, W5_RET_ENC_LARC_RC_BPP);
    result->larcData[2] = VpuReadReg(coreIdx, W5_RET_ENC_LARC_RC_ENC_ORDER);
#endif
    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncGetHeader(EncHandle instance, EncHeaderParam * encHeaderParam)
{
    Int32           coreIdx;
    Uint32          regVal = 0;
    EncInfo*        pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);

    coreIdx     = instance->coreIdx;

    EnterLock(coreIdx);


        VpuWriteReg(coreIdx, W5_CMD_ENC_BS_START_ADDR, encHeaderParam->buf);
        VpuWriteReg(coreIdx, W5_CMD_ENC_BS_SIZE, encHeaderParam->size);
        pEncInfo->streamRdPtr = encHeaderParam->buf;
        pEncInfo->streamWrPtr = encHeaderParam->buf;
        pEncInfo->streamBufStartAddr = encHeaderParam->buf;
        pEncInfo->streamBufSize = encHeaderParam->size;
        pEncInfo->streamBufEndAddr = encHeaderParam->buf + encHeaderParam->size;

    /* Secondary AXI */
    VpuWriteReg(coreIdx, W5_CMD_ENC_PIC_USE_SEC_AXI,  (pEncInfo->secAxiInfo.u.wave.useEncImdEnable<<9)    |
                                                      (pEncInfo->secAxiInfo.u.wave.useEncRdoEnable<<11)   |
                                                      (pEncInfo->secAxiInfo.u.wave.useEncLfEnable<<15));

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
    pEncInfo->reportQueueCount    = (regVal & 0xffff);

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);

        LeaveLock(coreIdx);

        if ( regVal == WAVE5_QUEUEING_FAIL)
            return RETCODE_QUEUEING_FAILURE;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_FAILURE;
    }

    LeaveLock(coreIdx);
    return RETCODE_SUCCESS;
}

RetCode Wave5VpuEncFiniSeq(CodecInst*  instance )
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

    if (param->enable_option & ENC_SET_CHANGE_PARAM_PPS) {
        if (instance->codecMode == W_SVAC_ENC) {
            regVal = (param->disableDeblk<<5)               |
                     ((param->chromaDcQpOffset&0x1F)<<14)   |
                     ((param->chromaAcQpOffset&0x1F)<<19)   |
                     ((param->lumaDcQpOffset&0x1F)<<24);
        }
        else {
            regVal = (param->constIntraPredFlag<<1)         |
                     (param->lfCrossSliceBoundaryEnable<<2) |
                     ((param->weightPredEnable&1)<<3)       |
                     (param->disableDeblk<<5)               |
                     ((param->betaOffsetDiv2&0xF)<<6)       |
                     ((param->tcOffsetDiv2&0xF)<<10)        |
                     ((param->chromaCbQpOffset&0x1F)<<14)   |
                     ((param->chromaCrQpOffset&0x1F)<<19)   |
                     (param->transform8x8Enable<<29)        |
                     (param->entropyCodingMode<<30);
        }

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_PPS_PARAM,  regVal);
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_INTRA_PARAM) {
        if (instance->codecMode == W_AVC_ENC) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_PARAM, (param->intraQP<<0) | ( (param->intraPeriod&0x7ff)<<6) | ( (param->avcIdrPeriod&0x7ff)<<17) | ( (param->forcedIdrHeaderEnable&3)<<28));
        }
        else {
            VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INTRA_PARAM, (param->intraQP<<3) | (param->forcedIdrHeaderEnable<<9) | (param->intraPeriod<<16));
        }
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_RC_TARGET_RATE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_TARGET_RATE, param->bitRate);
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_RC) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_PARAM,   (param->hvsQPEnable<<2)             |
                                                        (param->hvsQpScale<<4)              |
                                                        (param->vbvBufferSize<<20));
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_RC_MIN_MAX_QP) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_MIN_MAX_QP, (param->minQpI<<0)                   |
                                                           (param->maxQpI<<6)                   |
                                                           (param->hvsMaxDeltaQp<<12));
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_RC_INTER_MIN_MAX_QP) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_INTER_MIN_MAX_QP, (param->minQpP<<0) |
                                                                 (param->maxQpP<<6) |
                                                                 (param->minQpB<<12) |
                                                                 (param->maxQpB<<18));
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_RC_BIT_RATIO_LAYER) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_0_3, (param->fixedBitRatio[0]<<0)  |
                                                                    (param->fixedBitRatio[1]<<8)  |
                                                                    (param->fixedBitRatio[2]<<16) |
                                                                    (param->fixedBitRatio[3]<<24));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_BIT_RATIO_LAYER_4_7, (param->fixedBitRatio[4]<<0)  |
                                                                    (param->fixedBitRatio[5]<<8)  |
                                                                    (param->fixedBitRatio[6]<<16) |
                                                                    (param->fixedBitRatio[7]<<24));
    }
    if (param->enable_option & ENC_SET_CHANGE_PARAM_RC_WEIGHT) {
        regVal  = (param->rcWeightBuf<<8 | param->rcWeightParam);
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RC_WEIGHT_PARAM, regVal);
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_RDO) {
        regVal  = (param->rdoSkip<<2)               |
                  (param->lambdaScalingEnable<<3)   |
                  (param->coefClearDisable<<4)      |
                  (param->intraNxNEnable<<8)        |
                  (param->maxNumMerge<<18)          |
                  (param->customMDEnable<<20)       |
                  (param->customLambdaEnable<<21);

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_RDO_PARAM, regVal);
    }

    if (param->enable_option & ENC_SET_CHANGE_PARAM_INDEPEND_SLICE) {
        if (instance->codecMode == W_HEVC_ENC ) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INDEPENDENT_SLICE, param->independSliceModeArg<<16 | param->independSliceMode);
        }
        else if (instance->codecMode == W_AVC_ENC) {
            VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_INDEPENDENT_SLICE, param->avcSliceArg<<16 | param->avcSliceMode);
        }
    }

    if (instance->codecMode == W_HEVC_ENC && param->enable_option & ENC_SET_CHANGE_PARAM_DEPEND_SLICE) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_DEPENDENT_SLICE, param->dependSliceModeArg<<16 | param->dependSliceMode);
    }



    if (instance->codecMode == W_HEVC_ENC && param->enable_option & ENC_SET_CHANGE_PARAM_NR) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NR_PARAM,   (param->nrYEnable<<0)       |
                                                        (param->nrCbEnable<<1)      |
                                                        (param->nrCrEnable<<2)      |
                                                        (param->nrNoiseEstEnable<<3)|
                                                        (param->nrNoiseSigmaY<<4)   |
                                                        (param->nrNoiseSigmaCb<<12) |
                                                        (param->nrNoiseSigmaCr<<20));

        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_NR_WEIGHT,  (param->nrIntraWeightY<<0)  |
                                                        (param->nrIntraWeightCb<<5) |
                                                        (param->nrIntraWeightCr<<10)|
                                                        (param->nrInterWeightY<<15) |
                                                        (param->nrInterWeightCb<<20)|
                                                        (param->nrInterWeightCr<<25));
    }


    if (param->enable_option & ENC_SET_CHANGE_PARAM_BG) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_BG_PARAM, (param->bgThrDiff<<1)         |
                                                      (param->bgThrMeanDiff<<10)    |
                                                      (param->bgLambdaQp<<18)       |
                                                      ((param->bgDeltaQp&0x1F)<<24) |
                                                      (instance->codecMode == W_AVC_ENC ? param->s2fmeDisable<<29 : 0));
    }
    if (instance->codecMode == W_HEVC_ENC && param->enable_option & ENC_SET_CHANGE_PARAM_CUSTOM_MD) {
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

    if (instance->codecMode == W_HEVC_ENC && param->enable_option & ENC_SET_CHANGE_PARAM_CUSTOM_LAMBDA) {
        VpuWriteReg(coreIdx, W5_CMD_ENC_SEQ_CUSTOM_LAMBDA_ADDR, param->customLambdaAddr);
    }

    Wave5BitIssueCommand(instance, W5_ENC_SET_PARAM);

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        if (instance->loggingEnable)
            vdi_log(coreIdx, W5_ENC_SET_PARAM, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(coreIdx, W5_RET_QUEUE_STATUS);

    pEncInfo->instanceQueueCount = (regVal>>16) & 0xFF;
    pEncInfo->reportQueueCount    = (regVal & 0xFFFF);

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == 0) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON);
        if (regVal != WAVE5_QUEUEING_FAIL)
            VLOG(ERR, "FAIL_REASON = 0x%x\n", regVal);
        LeaveLock(coreIdx);

        if ( regVal == WAVE5_QUEUEING_FAIL)
            return RETCODE_QUEUEING_FAILURE;
        else if (regVal == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        else if (regVal == WAVE5_SYSERR_WATCHDOG_TIMEOUT)
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        else if (regVal == WAVE5_ERROR_FW_FATAL)
            return RETCODE_ERROR_FW_FATAL;
        else
            return RETCODE_FAILURE;
    }

    LeaveLock(coreIdx);
    return RETCODE_SUCCESS;
}



RetCode Wave5VpuGetSrcBufFlag(CodecInst* instance, Uint32* flag) {

    RetCode ret = RETCODE_SUCCESS;

    ret = SendQuery(instance, GET_SRC_BUF_FLAG);

    if (ret != RETCODE_SUCCESS)
        return RETCODE_QUERY_FAILURE;

    *flag = VpuReadReg(instance->coreIdx, W5_RET_ENC_SRC_BUF_FLAG);


    return RETCODE_SUCCESS;
}

//static RetCode CalcEncCropInfo(EncWaveParam* param, int rotMode, int srcWidth, int srcHeight);

static Uint32   presetGopSize[] = {
    1,  /* Custom GOP, Not used */
    1,  /* All Intra */
    1,  /* IPP Cyclic GOP size 1 */
    1,  /* IBB Cyclic GOP size 1 */
    2,  /* IBP Cyclic GOP size 2 */
    4,  /* IBBBP */
    4,
    4,
    8,
};

RetCode CheckEncCommonParamValid(EncOpenParam* pop)
{
    RetCode ret = RETCODE_SUCCESS;
    Int32   low_delay = 0;
    Int32   intra_period_gop_step_size;
    Int32   i;

    EncWaveParam* param     = &pop->EncStdParam.waveParam;

    // check low-delay gop structure
    if(param->gopPresetIdx == PRESET_IDX_CUSTOM_GOP)  // common gop
    {
        Int32 minVal = 0;
        if(param->gopParam.customGopSize > 1)
        {
            minVal = param->gopParam.picParam[0].pocOffset;
            low_delay = 1;
            for(i = 1; i < param->gopParam.customGopSize; i++)
            {
                if(minVal > param->gopParam.picParam[i].pocOffset)
                {
                    low_delay = 0;
                    break;
                }
                else
                    minVal = param->gopParam.picParam[i].pocOffset;
            }
        }
    }
    else if(param->gopPresetIdx == PRESET_IDX_ALL_I ||
            param->gopPresetIdx == PRESET_IDX_IPP   ||
            param->gopPresetIdx == PRESET_IDX_IBBB  ||
            param->gopPresetIdx == PRESET_IDX_IPPPP ||
            param->gopPresetIdx == PRESET_IDX_IBBBB) // low-delay case (IPPP, IBBB)
        low_delay = 1;

    if(low_delay) {
        intra_period_gop_step_size = 1;
    }
    else {
        if (param->gopPresetIdx == PRESET_IDX_CUSTOM_GOP) {
            intra_period_gop_step_size = param->gopParam.customGopSize;
        }
        else {
            intra_period_gop_step_size = presetGopSize[param->gopPresetIdx];
        }
    }

    if (pop->bitstreamFormat == STD_HEVC) {
        if( !low_delay && (param->intraPeriod != 0) && (param->decodingRefreshType != 0) && (intra_period_gop_step_size != 0) &&
            (param->intraPeriod < intra_period_gop_step_size * 2)) {
                VLOG(ERR,"CFG FAIL : Not support intra period[%d] for the gop structure\n", param->intraPeriod);
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : Intra period >= %d\n", intra_period_gop_step_size * 2);
                ret = RETCODE_FAILURE;
        }
    }
    else if (pop->bitstreamFormat == STD_SVAC) {
        if( !low_delay && (param->intraPeriod != 0) && (intra_period_gop_step_size != 0) &&
            (param->intraPeriod % intra_period_gop_step_size) != 1) {
                VLOG(ERR,"CFG FAIL : Not support intra period[%d] for the gop structure\n", param->intraPeriod);
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : Intra period = %d\n", intra_period_gop_step_size * (param->intraPeriod / intra_period_gop_step_size));
                ret = RETCODE_FAILURE;
        }
    }

    if( !low_delay && (param->intraPeriod != 0) && (intra_period_gop_step_size != 0) && ((param->intraPeriod % intra_period_gop_step_size) == 1) && param->decodingRefreshType == 0)
    {
        VLOG(ERR,"CFG FAIL : Not support decoding refresh type[%d] for closed gop structure\n", param->decodingRefreshType );
        VLOG(ERR,"RECOMMEND CONFIG PARAMETER : Decoding refresh type = IDR\n");
        ret = RETCODE_FAILURE;
    }

    if (param->gopPresetIdx == PRESET_IDX_CUSTOM_GOP) {
        for(i = 0; i < param->gopParam.customGopSize; i++)
        {
            if (param->gopParam.picParam[i].temporalId >= MAX_NUM_TEMPORAL_LAYER )
            {
                VLOG(ERR,"CFG FAIL : temporalId %d exceeds MAX_NUM_TEMPORAL_LAYER\n", param->gopParam.picParam[i].temporalId);
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : Adjust temporal ID under MAX_NUM_TEMPORAL_LAYER(7) in GOP structure\n");
                ret = RETCODE_FAILURE;
            }

            if (param->gopParam.picParam[i].temporalId < 0)
            {
                VLOG(ERR,"CFG FAIL : Must be %d-th temporal_id >= 0\n",param->gopParam.picParam[i].temporalId);
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : Adjust temporal layer above '0' in GOP structure\n");
                ret = RETCODE_FAILURE;
            }
        }
    }

    if (param->useRecommendEncParam == 0)
    {
        // RDO
        {
            int align_32_width_flag  = pop->picWidth % 32;
            int align_16_width_flag  = pop->picWidth % 16;
            int align_8_width_flag   = pop->picWidth % 8;
            int align_32_height_flag = pop->picHeight % 32;
            int align_16_height_flag = pop->picHeight % 16;
            int align_8_height_flag  = pop->picHeight % 8;

            if( ((param->cuSizeMode&0x1) == 0) && ((align_8_width_flag != 0) || (align_8_height_flag != 0)) )
            {
                VLOG(ERR,"CFG FAIL : Picture width and height must be aligned with 8 pixels when enable CU8x8 of cuSizeMode \n");
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : cuSizeMode |= 0x1 (CU8x8)\n");
                ret = RETCODE_FAILURE;
            }
            else if(((param->cuSizeMode&0x1) == 0) && ((param->cuSizeMode&0x2) == 0) && ((align_16_width_flag != 0) || (align_16_height_flag != 0)) )
            {
                VLOG(ERR,"CFG FAIL : Picture width and height must be aligned with 16 pixels when enable CU16x16 of cuSizeMode\n");
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : cuSizeMode |= 0x2 (CU16x16)\n");
                ret = RETCODE_FAILURE;
            }
            else if(((param->cuSizeMode&0x1) == 0) && ((param->cuSizeMode&0x2) == 0) && ((param->cuSizeMode&0x4) == 0) && ((align_32_width_flag != 0) || (align_32_height_flag != 0)) )
            {
                VLOG(ERR,"CFG FAIL : Picture width and height must be aligned with 32 pixels when enable CU32x32 of cuSizeMode\n");
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : cuSizeMode |= 0x4 (CU32x32)\n");
                ret = RETCODE_FAILURE;
            }
        }
    }

    // Slice
    {
        if ( param->wppEnable == 1 && param->independSliceMode == 1) {
            int num_ctb_in_width = VPU_ALIGN64(pop->picWidth)>>6;
            if (param->independSliceModeArg % num_ctb_in_width) {
                printf("CFG FAIL : If WaveFrontSynchro(WPP) '1', the number of IndeSliceArg(ctb_num) must be multiple of num_ctb_in_width\n");
                printf("RECOMMEND CONFIG PARAMETER : IndeSliceArg = num_ctb_in_width * a\n");
                ret = RETCODE_FAILURE;
            }
        }

        // multi-slice & wpp
        if(param->wppEnable == 1 && param->dependSliceMode != 0) {
            VLOG(ERR,"CFG FAIL : If WaveFrontSynchro(WPP) '1', the option of multi-slice must be '0'\n");
            VLOG(ERR,"RECOMMEND CONFIG PARAMETER : independSliceMode = 0, dependSliceMode = 0\n");
            ret = RETCODE_FAILURE;
        }

        if(param->independSliceMode == 0 && param->dependSliceMode != 0)
        {
            VLOG(ERR,"CFG FAIL : If independSliceMode is '0', dependSliceMode must be '0'\n");
            VLOG(ERR,"RECOMMEND CONFIG PARAMETER : independSliceMode = 1, independSliceModeArg = TotalCtuNum\n");
            ret = RETCODE_FAILURE;
        }
        else if((param->independSliceMode == 1) && (param->dependSliceMode == 1) )
        {
            if(param->independSliceModeArg < param->dependSliceModeArg)
            {
                VLOG(ERR,"CFG FAIL : If independSliceMode & dependSliceMode is both '1' (multi-slice with ctu count), must be independSliceModeArg >= dependSliceModeArg\n");
                VLOG(ERR,"RECOMMEND CONFIG PARAMETER : dependSliceMode = 0\n");
                ret = RETCODE_FAILURE;
            }
        }

        if (param->independSliceMode != 0)
        {
            if (param->independSliceModeArg > 65535)
            {
                VLOG(ERR,"CFG FAIL : If independSliceMode is not 0, must be independSliceModeArg <= 0xFFFF\n");
                ret = RETCODE_FAILURE;
            }
        }

        if (param->dependSliceMode != 0)
        {
            if (param->dependSliceModeArg > 65535)
            {
                VLOG(ERR,"CFG FAIL : If dependSliceMode is not 0, must be dependSliceModeArg <= 0xFFFF\n");
                ret = RETCODE_FAILURE;
            }
        }
    }

    if (param->confWinTop % 2) {
        VLOG(ERR, "CFG FAIL : conf_win_top : %d value is not available. The value should be equal to multiple of 2.\n", param->confWinTop);
        ret = RETCODE_FAILURE;
    }

    if (param->confWinBot % 2) {
        VLOG(ERR, "CFG FAIL : conf_win_bot : %d value is not available. The value should be equal to multiple of 2.\n", param->confWinBot);
        ret = RETCODE_FAILURE;
    }

    if (param->confWinLeft % 2) {
        VLOG(ERR, "CFG FAIL : conf_win_left : %d value is not available. The value should be equal to multiple of 2.\n", param->confWinLeft);
        ret = RETCODE_FAILURE;
    }

    if (param->confWinRight % 2) {
        VLOG(ERR, "CFG FAIL : conf_win_right : %d value is not available. The value should be equal to multiple of 2.\n", param->confWinRight);
        ret = RETCODE_FAILURE;
    }

    // RDO
    if (param->cuSizeMode == 0) {
        VLOG(ERR, "CFG FAIL :  EnCu8x8, EnCu16x16, and EnCu32x32 must be equal to 1 respectively.\n");
        ret = RETCODE_FAILURE;
    }

    if (param->losslessEnable && (param->nrYEnable || param->nrCbEnable || param->nrCrEnable)) {
        VLOG(ERR, "CFG FAIL : LosslessCoding and NoiseReduction (EnNrY, EnNrCb, and EnNrCr) cannot be used simultaneously.\n");
        ret = RETCODE_FAILURE;
    }

    if (param->losslessEnable && param->bgDetectEnable) {
        VLOG(ERR, "CFG FAIL : LosslessCoding and BgDetect cannot be used simultaneously.\n");
        ret = RETCODE_FAILURE;
    }

    if (param->losslessEnable && pop->rcEnable) {
        VLOG(ERR, "CFG FAIL : osslessCoding and RateControl cannot be used simultaneously.\n");
        ret = RETCODE_FAILURE;
    }

    if (param->losslessEnable && param->roiEnable) {
        VLOG(ERR, "CFG FAIL : LosslessCoding and Roi cannot be used simultaneously.\n");
        ret = RETCODE_FAILURE;
    }

    if (param->losslessEnable && !param->skipIntraTrans) {
        VLOG(ERR, "CFG FAIL : IntraTransSkip must be enabled when LosslessCoding is enabled.\n");
        ret = RETCODE_FAILURE;
    }

    // Intra refresh
    {
        Int32 num_ctu_row = (pop->picHeight + 64 - 1) / 64;
        Int32 num_ctu_col = (pop->picWidth + 64 - 1) / 64;

        if(param->intraRefreshMode && param->intraRefreshArg <= 0)
        {
            VLOG(ERR, "CFG FAIL : IntraCtuRefreshArg must be greater then 0 when IntraCtuRefreshMode is enabled.\n");
            ret = RETCODE_FAILURE;
        }
        if(param->intraRefreshMode == 1 && param->intraRefreshArg > num_ctu_row)
        {
            VLOG(ERR, "CFG FAIL : IntraCtuRefreshArg must be less then the number of CTUs in a row when IntraCtuRefreshMode is equal to 1.\n");
            ret = RETCODE_FAILURE;
        }
        if(param->intraRefreshMode == 2 && param->intraRefreshArg > num_ctu_col)
        {
            VLOG(ERR, "CFG FAIL : IntraCtuRefreshArg must be less then the number of CTUs in a column when IntraCtuRefreshMode is equal to 2.\n");
            ret = RETCODE_FAILURE;
        }
        if(param->intraRefreshMode == 3 && param->intraRefreshArg > num_ctu_row*num_ctu_col)
        {
            VLOG(ERR, "CFG FAIL : IntraCtuRefreshArg must be less then the number of CTUs in a picture when IntraCtuRefreshMode is equal to 3.\n");
            ret = RETCODE_FAILURE;
        }
        if(param->intraRefreshMode == 4 && param->intraRefreshArg > num_ctu_row*num_ctu_col)
        {
            VLOG(ERR, "CFG FAIL : IntraCtuRefreshArg must be less then the number of CTUs in a picture when IntraCtuRefreshMode is equal to 4.\n");
            ret = RETCODE_FAILURE;
        }
        if(param->intraRefreshMode == 4 && param->losslessEnable)
        {
            VLOG(ERR, "CFG FAIL : LosslessCoding and IntraCtuRefreshMode (4) cannot be used simultaneously.\n");
            ret = RETCODE_FAILURE;
        }
        if(param->intraRefreshMode == 4 && param->roiEnable)
        {
            VLOG(ERR, "CFG FAIL : Roi and IntraCtuRefreshMode (4) cannot be used simultaneously.\n");
            ret = RETCODE_FAILURE;
        }
    }
    return ret;
}

RetCode CheckEncRcParamValid(EncOpenParam* pop)
{
    RetCode       ret = RETCODE_SUCCESS;
    EncWaveParam* param     = &pop->EncStdParam.waveParam;

    if(pop->rcEnable == 1)
    {
        if((param->minQpI > param->maxQpI) || (param->minQpP > param->maxQpP) || (param->minQpB > param->maxQpB))
        {
            VLOG(ERR,"CFG FAIL : Not allowed MinQP > MaxQP\n");
            VLOG(ERR,"RECOMMEND CONFIG PARAMETER : MinQP = MaxQP\n");
            ret = RETCODE_FAILURE;
        }

        if(pop->bitRate <= (int) pop->frameRateInfo)
        {
            VLOG(ERR,"CFG FAIL : Not allowed EncBitRate <= FrameRate\n");
            VLOG(ERR,"RECOMMEND CONFIG PARAMETER : EncBitRate = FrameRate * 10000\n");
            ret = RETCODE_FAILURE;
        }
    }

    return ret;
}

RetCode CheckEncCustomGopParamValid(EncOpenParam* pop)
{
    RetCode       ret = RETCODE_SUCCESS;
    CustomGopParam* gopParam;
    CustomGopPicParam* gopPicParam;

    CustomGopPicParam new_gop[MAX_GOP_NUM*2 +1];
    Int32   curr_poc, i, ei, gi, gop_size;
    Int32   enc_tid[MAX_GOP_NUM*2 +1];

    gopParam    = &(pop->EncStdParam.waveParam.gopParam);
    gop_size    = gopParam->customGopSize;

    new_gop[0].pocOffset    = 0;
    new_gop[0].temporalId   = 0;
    new_gop[0].picType      = PIC_TYPE_I;
    new_gop[0].numRefPicL0  = 0;
    enc_tid[0]              = 0;

    for(i = 0; i < gop_size * 2; i++)
    {
        ei = i % gop_size;
        gi = i / gop_size;
        gopPicParam = &gopParam->picParam[ei];

        curr_poc                    = gi * gop_size + gopPicParam->pocOffset;
        new_gop[i + 1].pocOffset    = curr_poc;
        new_gop[i + 1].temporalId   = gopPicParam->temporalId;
        new_gop[i + 1].picType      = gopPicParam->picType;
        new_gop[i + 1].refPocL0     = gopPicParam->refPocL0 + gi * gop_size;
        new_gop[i + 1].refPocL1     = gopPicParam->refPocL1 + gi * gop_size;
        new_gop[i + 1].numRefPicL0  = gopPicParam->numRefPicL0;
        enc_tid[i + 1] = -1;
    }

    for(i = 0; i < gop_size; i++)
    {
        gopPicParam = &gopParam->picParam[ei];

        if(gopPicParam->pocOffset <= 0)
        {
            VLOG(ERR, "CFG FAIL : the POC of the %d-th picture must be greater then -1\n", i+1);
            ret = RETCODE_FAILURE;
        }
        if(gopPicParam->pocOffset > gop_size)
        {
            VLOG(ERR, "CFG FAIL : the POC of the %d-th picture must be less then GopSize + 1\n", i+1);
            ret = RETCODE_FAILURE;
        }
        if(gopPicParam->temporalId < 0)
        {
            VLOG(ERR, "CFG FAIL : the temporal_id of the %d-th picture must be greater than -1\n", i+1);
            ret = RETCODE_FAILURE;
        }
    }

    for(ei = 1; ei < gop_size * 2 + 1; ei++)
    {
        CustomGopPicParam* cur_pic = &new_gop[ei];
        if(ei <= gop_size)
        {
            enc_tid[cur_pic->pocOffset] = cur_pic->temporalId;
            continue;
        }

        if(new_gop[ei].picType != PIC_TYPE_I)
        {
            Int32 ref_poc = cur_pic->refPocL0;
            if(enc_tid[ref_poc] < 0) // reference picture is not encoded yet
            {
                VLOG(ERR, "CFG FAIL : the 1st reference picture cannot be used as the reference of the picture (POC %d) in encoding order\n", cur_pic->pocOffset - gop_size);
                ret = RETCODE_FAILURE;
            }
            if(enc_tid[ref_poc] > cur_pic->temporalId)
            {
                VLOG(ERR, "CFG FAIL : the temporal_id of the picture (POC %d) is wrong\n", cur_pic->pocOffset - gop_size);
                ret = RETCODE_FAILURE;
            }
            if(ref_poc >= cur_pic->pocOffset)
            {
                VLOG(ERR, "CFG FAIL : the POC of the 1st reference picture of %d-th picture is wrong\n", cur_pic->pocOffset - gop_size);
                ret = RETCODE_FAILURE;
            }
        }
        if(new_gop[ei].picType != PIC_TYPE_P)
        {
            Int32 ref_poc = cur_pic->refPocL1;
            if(enc_tid[ref_poc] < 0) // reference picture is not encoded yet
            {
                VLOG(ERR, "CFG FAIL : the 2nd reference picture cannot be used as the reference of the picture (POC %d) in encoding order\n", cur_pic->pocOffset - gop_size);
                ret = RETCODE_FAILURE;
            }
            if(enc_tid[ref_poc] > cur_pic->temporalId)
            {
                VLOG(ERR, "CFG FAIL : the temporal_id of %d-th picture is wrong\n", cur_pic->pocOffset - gop_size);
                ret = RETCODE_FAILURE;
            }
            if(new_gop[ei].picType == PIC_TYPE_P && new_gop[ei].numRefPicL0>1)
            {
                if(ref_poc >= cur_pic->pocOffset)
                {
                    VLOG(ERR, "CFG FAIL : the POC of the 2nd reference picture of %d-th picture is wrong\n", cur_pic->pocOffset - gop_size);
                    ret = RETCODE_FAILURE;
                }
            }
            else // HOST_PIC_TYPE_B
            {
                if(ref_poc == cur_pic->pocOffset)
                {
                    VLOG(ERR, "CFG FAIL : the POC of the 2nd reference picture of %d-th picture is wrong\n", cur_pic->pocOffset - gop_size);
                    ret = RETCODE_FAILURE;
                }
            }
        }
        curr_poc = cur_pic->pocOffset;
        enc_tid[curr_poc] = cur_pic->temporalId;
    }
    return ret;
}


