#include "product.h"
#include "wave/common/common.h"
#include "wave/common/common_vpuconfig.h"
#include "vpuerror.h"
#include "wave/common/common_regdefine.h"

#define COMMAND_TIMEOUT     0xffff

void Wave4BitIssueCommand(CodecInst* instance, Uint32 cmd)
{
    Uint32 instanceIndex = 0;
    Uint32 codecMode     = 0;
    Uint32 coreIdx;

    if (instance != NULL) {
        instanceIndex = instance->instIndex;
        codecMode     = instance->codecMode;
    }

    coreIdx = instance->coreIdx;

    VpuWriteReg(coreIdx, W4_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W4_RET_SUCCESS, 0);	//for debug	
    VpuWriteReg(coreIdx, W4_CORE_INDEX,  0);

    if (instance->productId == PRODUCT_ID_7Q)   // only coda7q use codecModeAux for DIV
        VpuWriteReg(coreIdx, W4_INST_INDEX,  (instanceIndex&0xffff)|(codecMode<<16) | (instance->codecModeAux<<24));
    else
        VpuWriteReg(coreIdx, W4_INST_INDEX,  (instanceIndex&0xffff)|(codecMode<<16));

    VpuWriteReg(coreIdx, W4_COMMAND, cmd);

    if ((instance != NULL && instance->loggingEnable))
        vdi_log(coreIdx, cmd, 1);

    if (cmd != INIT_VPU) {
        VpuWriteReg(coreIdx, W4_VPU_HOST_INT_REQ, 1);	
    }

    return;
} 

static RetCode SetupWave4Properties(
    Uint32 coreIdx
    )
{
    VpuAttr*    pAttr = &g_VpuCoreAttributes[coreIdx];
    Uint32      regVal;
    Uint8*      str;
    CodecInst   hdr;

    /* GET FIRMWARE&HARDWARE INFORMATION */
    hdr.coreIdx   = 0;
    hdr.instIndex = 0;
    hdr.loggingEnable = 0;

    Wave4BitIssueCommand((CodecInst*)&hdr, GET_FW_VERSION);
    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }
    regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }
    
    regVal = VpuReadReg(coreIdx, W4_RET_PRODUCT_NAME);
    str    = (Uint8*)&regVal;
    pAttr->productName[0] = str[3];
    pAttr->productName[1] = str[2];
    pAttr->productName[2] = str[1];
    pAttr->productName[3] = str[0];
    pAttr->productName[4] = 0;
    pAttr->productNumber  = VpuReadReg(coreIdx, W4_RET_PRODUCT_VERSION);

    switch (pAttr->productNumber) {
    case WAVE410_CODE:   pAttr->productId = PRODUCT_ID_410;   break;
    case WAVE4102_CODE:  pAttr->productId = PRODUCT_ID_4102;  break;
    case WAVE420_CODE:   pAttr->productId = PRODUCT_ID_420;   break;
    case WAVE412_CODE:   pAttr->productId = PRODUCT_ID_412;   break;
    case CODA7Q_CODE:    pAttr->productId = PRODUCT_ID_7Q;    break;
    case WAVE420L_CODE:  pAttr->productId = PRODUCT_ID_420L;  break;
    case WAVE510_CODE:   pAttr->productId = PRODUCT_ID_510;   break;
    case WAVE512_CODE:   pAttr->productId = PRODUCT_ID_412;   break;
    case WAVE515_CODE:   pAttr->productId = PRODUCT_ID_515;   break;
    default:             pAttr->productId = PRODUCT_ID_NONE;  break;
    }

    pAttr->hwConfigDef0    = VpuReadReg(coreIdx, W4_RET_STD_DEF0);
    pAttr->hwConfigDef1    = VpuReadReg(coreIdx, W4_RET_STD_DEF1);
    pAttr->hwConfigFeature = VpuReadReg(coreIdx, W4_RET_CONF_FEATURE);
    pAttr->hwConfigDate    = VpuReadReg(coreIdx, W4_RET_CONFIG_DATE);
    pAttr->hwConfigRev     = VpuReadReg(coreIdx, W4_RET_CONFIG_REVISION);
    pAttr->hwConfigType    = VpuReadReg(coreIdx, W4_RET_CONFIG_TYPE);

    pAttr->supportGDIHW          = TRUE;
    if (pAttr->productId == PRODUCT_ID_420L) { 
        pAttr->supportDecoders       = (1<<STD_HEVC);
        pAttr->supportEncoders       = (1<<STD_HEVC);
        pAttr->supportGDIHW          = FALSE;
    }
    else if (pAttr->productId == PRODUCT_ID_420) {
        pAttr->supportDecoders       = (1<<STD_HEVC);
        pAttr->supportEncoders       = (1<<STD_HEVC);
    }
    else if (pAttr->productId == PRODUCT_ID_412) {
        pAttr->supportDecoders       = (1<<STD_HEVC);
        pAttr->supportDecoders      |= (1<<STD_VP9);;
        pAttr->supportEncoders       = (1<<STD_VP9);
    }
    else if (pAttr->productId == PRODUCT_ID_515) {
        pAttr->supportDecoders       = (1<<STD_HEVC);
        pAttr->supportDecoders      |= (1<<STD_VP9);
        pAttr->supportDecoders      |= (1<<STD_AVS2);
        pAttr->supportEncoders       = (1<<STD_VP9);
    }
    else if (pAttr->productId == PRODUCT_ID_7Q) {
        pAttr->supportDecoders = (1<<STD_AVC)   |
                                 (1<<STD_VC1)   |
                                 (1<<STD_MPEG2) |
                                 (1<<STD_MPEG4) |
                                 (1<<STD_H263)  |
                                 (1<<STD_AVS)   |
                                 (1<<STD_DIV3)  | 
                                 (1<<STD_RV)    |
                                 (1<<STD_THO)   |
                                 (1<<STD_VP8)   |
                                 (1<<STD_HEVC);
        pAttr->supportEncoders  = (1<<STD_AVC) | (1<<STD_MPEG4) | (1<<STD_H263);
    }
    else {
        // Wave410
        pAttr->supportDecoders       = (1<<STD_HEVC);
        pAttr->supportEncoders       = 0;
    }
    pAttr->supportFBCBWOptimization = (BOOL)((pAttr->hwConfigDef1>>15)&0x01);
    pAttr->supportWTL            = TRUE;
    pAttr->supportTiled2Linear   = FALSE;
    pAttr->supportMapTypes       = FALSE;
    pAttr->support128bitBus      = TRUE;
    pAttr->supportThumbnailMode  = TRUE;
    pAttr->supportEndianMask     = (Uint32)((1<<VDI_LITTLE_ENDIAN) | (1<<VDI_BIG_ENDIAN) | (1<<VDI_32BIT_LITTLE_ENDIAN) | (1<<VDI_32BIT_BIG_ENDIAN) | (0xffff<<16));
    pAttr->supportBitstreamMode  = (1<<BS_MODE_INTERRUPT) | (1<<BS_MODE_PIC_END);
    pAttr->framebufferCacheType  = FramebufCacheNone;
    pAttr->bitstreamBufferMargin = (pAttr->productId == PRODUCT_ID_7Q) ? 1024 : 0;
    pAttr->numberOfVCores        = MAX_NUM_VCORE;
    pAttr->numberOfMemProtectRgns = 10;

    return RETCODE_SUCCESS;
}

Int32 WaveVpuGetProductId(Uint32  coreIdx)
{
    Uint32  productId = PRODUCT_ID_NONE;
    Uint32  val;

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return PRODUCT_ID_NONE;

    val = VpuReadReg(coreIdx, W4_PRODUCT_NUMBER);

    switch (val) {
    case WAVE410_CODE:   productId = PRODUCT_ID_410;   break;
    case WAVE4102_CODE:  productId = PRODUCT_ID_4102;  break;
    case WAVE420_CODE:   productId = PRODUCT_ID_420;   break;
    case WAVE412_CODE:   productId = PRODUCT_ID_412;   break;
    case CODA7Q_CODE:    productId = PRODUCT_ID_7Q;    break;
    case WAVE420L_CODE:  productId = PRODUCT_ID_420L;  break;
    case WAVE510_CODE:   productId = PRODUCT_ID_510;   break;
    case WAVE512_CODE:   productId = PRODUCT_ID_512;   break;
    case WAVE515_CODE:   productId = PRODUCT_ID_515;   break;
    case WAVE520_CODE:   productId = PRODUCT_ID_520;   break;
    default:
        VLOG(ERR, "Check productId(%d)\n", val);
        break;
    }

    return productId;
}

RetCode Wave4VpuGetVersion(Uint32 coreIdx, Uint32* versionInfo, Uint32* revision)
{
    Uint32          regVal;
    CodecInstHeader hdr = {0};

    /* GET FIRMWARE&HARDWARE INFORMATION */
    hdr.coreIdx   = 0;
    hdr.instIndex = 0;
    hdr.loggingEnable = 0;
    Wave4BitIssueCommand((CodecInst*)&hdr, GET_FW_VERSION);
    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }
    regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
    if (regVal == 0)
        return RETCODE_FAILURE;

    regVal = VpuReadReg(coreIdx, W4_RET_FW_VERSION);
    if (versionInfo != NULL) {
        *versionInfo = 0;
    }
    if (revision != NULL) {
        *revision    = regVal; 
    }

    regVal = VpuReadReg(coreIdx,W4_RET_CONFIG_REVISION );
    VLOG(INFO, "\nget hw version %d !!!\n", regVal);

    return RETCODE_SUCCESS;
}
RetCode Wave4VpuInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase;
    Uint32          codeSize;
    Uint32          i, regVal, remapSize;
    Uint32          hwOption    = 0;
    CodecInstHeader hdr;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    vdi_get_common_memory(coreIdx, &vb);

    codeBase  = vb.phys_addr;
    /* ALIGN TO 4KB */
    codeSize = (WAVE4_MAX_CODE_BUF_SIZE&~0xfff);
    if (codeSize < size*2) {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    VLOG(INFO, "\nVPU INIT Start!!!\n");

    VpuWriteMem(coreIdx, codeBase, (unsigned char*)firmware, size*2, VDI_128BIT_LITTLE_ENDIAN);

    vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)firmware);

    regVal = 0;
    VpuWriteReg(coreIdx, W4_PO_CONF, regVal);

    /* Reset All blocks */
    regVal = 0x7ffffff;
    VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, regVal);    // Reset All blocks 
    /* Waiting reset done */

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_RESET_STATUS) == -1) {
        VLOG(ERR, "VPU init(W4_VPU_RESET_REQ) timeout\n");
        VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);

    /* clear registers */
    for (i=W4_CMD_REG_BASE; i<W4_CMD_REG_END; i+=4)
        VpuWriteReg(coreIdx, i, 0x00);

    /* remap page size */
    remapSize = (codeSize >> 12) &0x1ff;
    regVal = 0x80000000 | (0 << 16) | (W4_REMAP_CODE_INDEX<<12) | (1<<11) | remapSize;
    VpuWriteReg(coreIdx, W4_VPU_REMAP_CTRL,     regVal);
    VpuWriteReg(coreIdx, W4_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
    VpuWriteReg(coreIdx, W4_VPU_REMAP_PADDR,    codeBase);
    VpuWriteReg(coreIdx, W4_ADDR_CODE_BASE,     codeBase);
    VpuWriteReg(coreIdx, W4_CODE_SIZE,          codeSize);
    VpuWriteReg(coreIdx, W4_CODE_PARAM,         0);
    //timeoutTicks = COMMAND_TIMEOUT*VCPU_CLOCK_IN_MHZ*(1000000>>15);
    VpuWriteReg(coreIdx, W4_TIMEOUT_CNT,        0xffffffff);

    VpuWriteReg(coreIdx, W4_HW_OPTION, hwOption);
    /* Interrupt */
    // for encoder interrupt
    regVal = (1<<W4_INT_ENC_PIC);
    regVal |= (1<<W4_INT_SLEEP_VPU);
    regVal |= (1<<W4_INT_SET_PARAM);
    // for decoder interrupt
    regVal |= (1<<W4_INT_DEC_PIC_HDR);
    regVal |= (1<<W4_INT_DEC_PIC);
    regVal |= (1<<W4_INT_QUERY_DEC);
    regVal |= (1<<W4_INT_SLEEP_VPU);
    regVal |= (1<<W4_INT_BSBUF_EMPTY);

    VpuWriteReg(coreIdx, W4_VPU_VINT_ENABLE,  regVal);

    hdr.coreIdx = coreIdx;

    Wave4BitIssueCommand((CodecInst*)&hdr, INIT_VPU); 
    VpuWriteReg(coreIdx, W4_VPU_REMAP_CORE_START, 1);

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
        VLOG(ERR, "VPU init(W4_VPU_REMAP_CORE_START) timeout\n");
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);

    if (regVal == 0) {
        Uint32      reasonCode = VpuReadReg(coreIdx, W4_RET_FAIL_REASON);
        VLOG(ERR, "VPU init(W4_RET_SUCCESS) failed(%d) REASON CODE(%08x)\n", regVal, reasonCode);
        return RETCODE_FAILURE;	
    }

    SetupWave4Properties(coreIdx);

    return RETCODE_SUCCESS;
}

RetCode Wave4VpuReInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase;
    PhysicalAddress oldCodeBase;
    Uint32          codeSize;
    Uint32          regVal, remapSize;
    CodecInstHeader hdr;
    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    vdi_get_common_memory(coreIdx, &vb);

    codeBase  = vb.phys_addr;
    /* ALIGN TO 4KB */
    codeSize = (WAVE4_MAX_CODE_BUF_SIZE&~0xfff);
    if (codeSize < size*2) {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    oldCodeBase = VpuReadReg(coreIdx, W4_VPU_REMAP_PADDR);

    if (oldCodeBase != codeBase) {

        VpuWriteMem(coreIdx, codeBase, (unsigned char*)firmware, size*2, VDI_128BIT_LITTLE_ENDIAN);
        vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)firmware);

        regVal = 0;
        VpuWriteReg(coreIdx, W4_PO_CONF, regVal);

        // Waiting for completion of bus transaction
        // Step1 : disable request
        vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0x100);	// WAVE410 GDI added new feature: disable_request

        // Step2 : Waiting for completion of bus transaction
        if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_GDI_VCORE0_BUS_STATUS) == -1) {
            vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0x00);
            vdi_log(coreIdx, RESET_VPU, 2);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        /* Reset All blocks */
        regVal = 0x7ffffff;
        VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, regVal);    // Reset All blocks 
        /* Waiting reset done */

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);
        // Step3 : must clear GDI_BUS_CTRL after done SW_RESET
        vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0x00);

        /* not needed to clear registers 
        for (i=W4_CMD_REG_BASE; i<W4_CMD_REG_END; i+=4) 
        VpuWriteReg(coreIdx, i, 0x00)
        */

        /* remap page size */
        remapSize = (codeSize >> 12) &0x1ff;
        regVal = 0x80000000 | (W4_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W4_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W4_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W4_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W4_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W4_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W4_CODE_PARAM,         0);
        VpuWriteReg(coreIdx, W4_TIMEOUT_CNT,        COMMAND_TIMEOUT);
        VpuWriteReg(coreIdx, W4_HW_OPTION,          0);
        /* Interrupt */
        // for encoder interrupt
        regVal = (1<<W4_INT_ENC_PIC);
        regVal |= (1<<W4_INT_SLEEP_VPU);
        regVal |= (1<<W4_INT_SET_PARAM);
        // for decoder interrupt
        regVal |= (1<<W4_INT_DEC_PIC_HDR);
        regVal |= (1<<W4_INT_DEC_PIC);
        regVal |= (1<<W4_INT_QUERY_DEC);
        regVal |= (1<<W4_INT_SLEEP_VPU);
        regVal |= (1<<W4_INT_BSBUF_EMPTY);
        VpuWriteReg(coreIdx, W4_VPU_VINT_ENABLE,  regVal);

        hdr.coreIdx = coreIdx;

        Wave4BitIssueCommand((CodecInst*)&hdr, INIT_VPU);
        VpuWriteReg(coreIdx, W4_VPU_REMAP_CORE_START, 1);

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
        if (regVal == 0) 
            return RETCODE_FAILURE;

    }
    SetupWave4Properties(coreIdx);

    return RETCODE_SUCCESS;
}

Uint32 Wave4VpuIsInit(Uint32 coreIdx)
{
    Uint32 pc;

    pc = (Uint32)VpuReadReg(coreIdx, W4_VCPU_CUR_PC);

    return pc;
}



Int32 Wave4VpuIsBusy(Uint32 coreIdx)
{
    return VpuReadReg(coreIdx, W4_VPU_BUSY_STATUS);
}

Int32 Wave4VpuWaitInterrupt(CodecInst* handle, Int32 timeout)
{
    Int32   reason = -1;
    Uint32  coreIdx = handle->coreIdx;
      
    if ((reason=vdi_wait_interrupt(coreIdx, timeout, W4_VPU_VINT_REASON_USR)) > 0) {
        /* If you are using device driver that we provide. the below codes are meaningless.
        * Because device driver clears interrupt.
        */
        VpuWriteReg(coreIdx, W4_VPU_VINT_REASON_CLR, reason);
        VpuWriteReg(coreIdx, W4_VPU_VINT_CLEAR, 1);
    }

    return reason;    
}

RetCode Wave4VpuClearInterrupt(Uint32 coreIdx, Uint32 flags)
{
    Uint32 interruptReason;
    
    interruptReason = VpuReadReg(coreIdx, W4_VPU_VINT_REASON_USR);
    interruptReason &= ~flags;
    VpuWriteReg(coreIdx, W4_VPU_VINT_REASON_USR, interruptReason);
    
    return RETCODE_SUCCESS;
}

RetCode Wave4VpuSleepWake(Uint32 coreIdx, int iSleepWake, const Uint16* code, Uint32 size)
{
    CodecInstHeader hdr;
    Uint32          regVal;
    vpu_buffer_t    vb;
    PhysicalAddress codeBase;

    Uint32          codeSize;
    Uint32          remapSize;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    hdr.coreIdx = coreIdx;

    if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {		
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }


    if(iSleepWake==1)  //saves
    {
        Wave4BitIssueCommand((CodecInst*)&hdr, SLEEP_VPU);
        
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1)
        {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
        regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
        if (regVal == 0)
        {
            APIDPRINT("SLEEP_VPU failed [0x%x]", VpuReadReg(coreIdx, W4_RET_FAIL_REASON));
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

        regVal = 0;
        VpuWriteReg(coreIdx, W4_PO_CONF, regVal);

        /* SW_RESET_SAFETY */
        regVal = W4_RST_BLOCK_ALL;
        VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, regVal);    // Reset All blocks 

        /* Waiting reset done */
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_RESET_STATUS) == -1) {
            VLOG(ERR, "VPU Wakeup(W4_VPU_RESET_REQ) timeout\n");
            VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);

        /* remap page size */
        remapSize = (codeSize >> 12) &0x1ff;
        regVal = 0x80000000 | (0 << 16) |(W4_REMAP_CODE_INDEX<<12) |  (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W4_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W4_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W4_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W4_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W4_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W4_CODE_PARAM,         0);
        VpuWriteReg(coreIdx, W4_TIMEOUT_CNT,        COMMAND_TIMEOUT);

        VpuWriteReg(coreIdx, W4_HW_OPTION, hwOption);

        /* Interrupt */
        // for encoder interrupt
        regVal = (1<<W4_INT_ENC_PIC);
        regVal |= (1<<W4_INT_SLEEP_VPU);
        regVal |= (1<<W4_INT_SET_PARAM);
        // for decoder interrupt
        regVal |= (1<<W4_INT_DEC_PIC_HDR);
        regVal |= (1<<W4_INT_DEC_PIC);
        regVal |= (1<<W4_INT_QUERY_DEC);
        regVal |= (1<<W4_INT_SLEEP_VPU);
        regVal |= (1<<W4_INT_BSBUF_EMPTY);

        VpuWriteReg(coreIdx, W4_VPU_VINT_ENABLE,  regVal);

        hdr.coreIdx = coreIdx;

        VpuWriteReg(coreIdx, W4_VPU_BUSY_STATUS, 1);
        VpuWriteReg(coreIdx, W4_RET_SUCCESS, 0);	//for debug	
        VpuWriteReg(coreIdx, W4_CORE_INDEX,  0);

        if (hdr.productId == PRODUCT_ID_7Q)   // only coda7q use codecModeAux for DIV
            VpuWriteReg(coreIdx, W4_INST_INDEX,  (hdr.instIndex&0xffff)|(hdr.codecMode<<16) | (hdr.codecModeAux<<24));
        else
            VpuWriteReg(coreIdx, W4_INST_INDEX,  (hdr.instIndex&0xffff)|(hdr.codecMode<<16));

        VpuWriteReg(coreIdx, W4_COMMAND, INIT_VPU);

        if (hdr.loggingEnable) {
            vdi_log(coreIdx, INIT_VPU, 1);
        }

        VpuWriteReg(coreIdx, W4_VPU_REMAP_CORE_START, 1);

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
            VLOG(ERR, "VPU Wakeup (W4_VPU_REMAP_CORE_START) timeout\n");
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
        if (regVal == 0) {  
            Uint32      reasonCode = VpuReadReg(coreIdx, W4_RET_FAIL_REASON);
            VLOG(ERR, "VPU Wakeup(W4_RET_SUCCESS) failed(%d) REASON CODE(%08x)\n", regVal, reasonCode);
            return RETCODE_FAILURE;
        }
    }

    return RETCODE_SUCCESS;
}

RetCode Wave4VpuReset(Uint32 coreIdx, SWResetMode resetMode)
{
    Uint32  val = 0;
    RetCode ret = RETCODE_FAILURE;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[coreIdx];

    // VPU doesn't send response. Force to set BUSY flag to 0.
    VpuWriteReg(coreIdx, W4_VPU_BUSY_STATUS, 0);
    if (pAttr->supportGDIHW == TRUE) {
        // Waiting for completion of bus transaction
        // Step1 : disable request
        vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0x100);	// WAVE410 GDI added new feature: disable_request

        // Step2 : Waiting for completion of bus transaction
        if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_GDI_VCORE0_BUS_STATUS) == -1) {
            vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0x00);
            vdi_log(coreIdx, RESET_VPU, 2);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
    }
    else {
        if (pAttr->productId == PRODUCT_ID_420L) {
            val = vdi_fio_read_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL);
            if ((val>>24) == 0x01) {
                /* VPU has a bus transaction controller */
                vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0x11);
            }
            if (vdi_wait_bus_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_GDI_VCORE0_BUS_STATUS) == -1) {
                vdi_log(coreIdx, RESET_VPU, 2);
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }
        }
    }

    if (resetMode == SW_RESET_SAFETY) {
        if ((ret=Wave4VpuSleepWake(coreIdx, TRUE, NULL, 0)) != RETCODE_SUCCESS) {
            return ret;
        }
    }

    switch (resetMode) {
    case SW_RESET_ON_BOOT:
    case SW_RESET_FORCE:
        val = W4_RST_BLOCK_ALL;
        break;
    case SW_RESET_SAFETY:
        val = W4_RST_BLOCK_ACLK_ALL | W4_RST_BLOCK_BCLK_ALL | W4_RST_BLOCK_CCLK_ALL;
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    if (val) {
        VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, val);

        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);
            vdi_log(coreIdx, RESET_VPU, 2);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
        VpuWriteReg(coreIdx, W4_VPU_RESET_REQ, 0);
    }
    if (pAttr->supportGDIHW == TRUE) {
        // Step3 : must clear GDI_BUS_CTRL after done SW_RESET
        vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0x00);
    }
    else {
        if (pAttr->productId == PRODUCT_ID_420L) {
            val = vdi_fio_read_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL);
            if ((val>>24) == 0x01) {
                /* VPU has a bus transaction controller */
                vdi_fio_write_register(coreIdx, W4_GDI_VCORE0_BUS_CTRL, 0);
            }
        }
    }

    if (resetMode == SW_RESET_SAFETY || resetMode == SW_RESET_FORCE) {
        ret = Wave4VpuSleepWake(coreIdx, FALSE, NULL, 0);
    }

    return RETCODE_SUCCESS;
}

RetCode Wave4VpuBuildUpDecParam(CodecInst* instance, DecOpenParam* param)
{
    RetCode     ret = RETCODE_SUCCESS;
    DecInfo*    pDecInfo;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[instance->coreIdx];

    pDecInfo    = VPU_HANDLE_TO_DECINFO(instance);

    pDecInfo->streamRdPtrRegAddr      = W4_BS_RD_PTR;
    pDecInfo->streamWrPtrRegAddr      = W4_BS_WR_PTR;
    pDecInfo->frameDisplayFlagRegAddr = W4_RET_DEC_DISP_FLAG;
    pDecInfo->currentPC               = W4_VCPU_CUR_PC;
    pDecInfo->busyFlagAddr            = W4_VPU_BUSY_STATUS;
    if ((pAttr->supportDecoders&(1<<param->bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;
    pDecInfo->seqChangeMask           = (param->bitstreamFormat == STD_HEVC) ? 
                                        SEQ_CHANGE_ENABLE_ALL_HEVC : SEQ_CHANGE_ENABLE_ALL_VP9;

    pDecInfo->targetSubLayerId       = HEVC_MAX_SUB_LAYER_ID;

    if (param->vbWork.size > 0) {
        pDecInfo->vbWork = param->vbWork;
        pDecInfo->workBufferAllocExt = TRUE;
        vdi_attach_dma_memory(instance->coreIdx, &param->vbWork);
    }
    else {
        if ((instance->productId == PRODUCT_ID_412) || (instance->productId == PRODUCT_ID_512) || (instance->productId == PRODUCT_ID_515)) {
            pDecInfo->vbWork.size = WAVE412DEC_WORKBUF_SIZE;
        } 
        else {
            pDecInfo->vbWork.size = WAVE4DEC_WORKBUF_SIZE;
        }
        pDecInfo->workBufferAllocExt = FALSE;
        if (vdi_allocate_dma_memory(instance->coreIdx, &pDecInfo->vbWork) < 0) {
            pDecInfo->vbWork.base      = 0;
            pDecInfo->vbWork.phys_addr = 0;
            pDecInfo->vbWork.size      = 0;
            pDecInfo->vbWork.virt_addr = 0;
            return RETCODE_INSUFFICIENT_RESOURCE;
        } 
    }


    vdi_clear_memory(instance->coreIdx, pDecInfo->vbWork.phys_addr, pDecInfo->vbWork.size, 0);

    VpuWriteReg(instance->coreIdx, W4_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W4_WORK_SIZE,      pDecInfo->vbWork.size);
    VpuWriteReg(instance->coreIdx, W4_WORK_PARAM,     0);

    Wave4BitIssueCommand(instance, CREATE_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, CREATE_INSTANCE, 2);
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W4_RET_SUCCESS) == FALSE) {
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork);
        ret = RETCODE_FAILURE;
    }

    return ret;
}

RetCode Wave4VpuDecFiniSeq(CodecInst* instance)
{
    DecInfo*    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);


    VpuWriteReg(instance->coreIdx, W4_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W4_WORK_SIZE,      pDecInfo->vbWork.size);
    VpuWriteReg(instance->coreIdx, W4_WORK_PARAM,     0);

    Wave4BitIssueCommand(instance, FINI_SEQ);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) 
        return RETCODE_VPU_RESPONSE_TIMEOUT;

    if (VpuReadReg(instance->coreIdx, W4_RET_SUCCESS) == FALSE)
        return RETCODE_FAILURE;

    return RETCODE_SUCCESS;
}

RetCode Wave4VpuDecSetBitstreamFlag(CodecInst* instance, BOOL running, BOOL eos, BOOL explicitEnd)
{
    DecInfo* pDecInfo = &instance->CodecInfo->decInfo;

    pDecInfo->streamEndflag = (eos == 1) ? TRUE : FALSE;

    if (running == TRUE) {
        BitStreamMode bsMode = (BitStreamMode)pDecInfo->openParam.bitstreamMode;
        BOOL          explicitEndFlag=0;

        explicitEndFlag = (BOOL)(bsMode == BS_MODE_PIC_END || pDecInfo->streamEndflag == TRUE || explicitEnd == TRUE);
        VpuWriteReg(instance->coreIdx, W4_BS_OPTION,  (pDecInfo->streamEndflag<<1)|explicitEndFlag );
    }

    return RETCODE_SUCCESS;
}

RetCode Wave4VpuDecRegisterFramebuffer(CodecInst* inst, FrameBuffer* fbArr, TiledMapType mapType, Uint32 count)
{
    RetCode      ret = RETCODE_SUCCESS;
    DecInfo*     pDecInfo = &inst->CodecInfo->decInfo;
    DecInitialInfo* sequenceInfo = &inst->CodecInfo->decInfo.initialInfo;
    Int32        q, j, i, remain, idx;
    Uint32 mvCount;
    Uint32       k;
    Int32        coreIdx, startNo, endNo;
    Uint32       regVal, cbcrInterleave, nv21;
    Uint32       endian, yuvFormat = 0;
    Uint32       addrY, addrCb, addrCr;
    Uint32       mvColSize, fbcYTblSize, fbcCTblSize, lumaStride, chromaStride;
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

        if (inst->codecMode == C7_VP9_DEC) {
            mvColSize          = WAVE4_DEC_VP9_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        else if (inst->codecMode == HEVC_DEC) {
            mvColSize          = WAVE4_DEC_HEVC_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        else if (inst->codecMode == C7_AVS2_DEC) {
            mvColSize          = CODA7Q_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight); // [FIX ME!!!]
        }
        else {
            mvColSize          = CODA7Q_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        mvColSize          = VPU_ALIGN16(mvColSize);
        vbBuffer.phys_addr = 0;
        if (inst->codecMode == HEVC_DEC || inst->codecMode == C7_HEVC_DEC || inst->codecMode == C7_VP9_DEC || inst->codecMode == C7_AVS2_DEC) {
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
        if (inst->codecMode == C7_VP9_DEC)
            fbcYTblSize        = WAVE4_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
        else if (inst->codecMode == HEVC_DEC)
            fbcYTblSize        = WAVE4_FBC_LUMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        else if (inst->codecMode == C7_AVS2_DEC)
            fbcYTblSize        = CODA7Q_FBC_LUMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight); // [FIX ME!!!]
        else
            fbcYTblSize        = CODA7Q_FBC_LUMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
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

        if (inst->codecMode == C7_VP9_DEC)
            fbcCTblSize        = WAVE4_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
        else if (inst->codecMode == HEVC_DEC) 
            fbcCTblSize        = WAVE4_FBC_CHROMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        else if (inst->codecMode == C7_AVS2_DEC)
            fbcCTblSize        = CODA7Q_FBC_CHROMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight); // [FIX ME!!!]
        else
            fbcCTblSize        = CODA7Q_FBC_CHROMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);

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

        if (inst->productId == PRODUCT_ID_420 || inst->productId == PRODUCT_ID_420L) {
            lumaStride = VPU_ALIGN16(pDecInfo->initialInfo.picWidth)*(pDecInfo->initialInfo.lumaBitdepth>8 ? 5 : 4);
            lumaStride = VPU_ALIGN32(lumaStride);
            chromaStride = VPU_ALIGN16(pDecInfo->initialInfo.picWidth/2)*(pDecInfo->initialInfo.chromaBitdepth>8 ? 5 : 4);
            chromaStride = VPU_ALIGN32(chromaStride); 
            VpuWriteReg(coreIdx, W4_FBC_STRIDE, lumaStride<<16 | chromaStride);
        }
    } 
    endian = vdi_convert_endian(coreIdx, fbArr[0].endian) & VDI_128BIT_ENDIAN_MASK;

    regVal = (pDecInfo->initialInfo.picWidth<<16)|(pDecInfo->initialInfo.picHeight);
    VpuWriteReg(coreIdx, W4_PIC_SIZE, regVal);


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

        if (inst->productId == PRODUCT_ID_420 || inst->productId == PRODUCT_ID_420L) {
            lumaStride = VPU_ALIGN16(pDecInfo->initialInfo.picWidth)*(pDecInfo->initialInfo.lumaBitdepth>8 ? 5 : 4);
            lumaStride = VPU_ALIGN32(lumaStride);
            chromaStride = VPU_ALIGN16(pDecInfo->initialInfo.picWidth/2)*(pDecInfo->initialInfo.chromaBitdepth>8 ? 5 : 4);
            chromaStride = VPU_ALIGN32(chromaStride); 
            VpuWriteReg(coreIdx, W4_FBC_STRIDE, lumaStride<<16 | chromaStride);
        }
    }


    regVal = 
            ((mapType == LINEAR_FRAME_MAP)<< 28)    |
            (axiID << 24)                           |
            (1<< 23)                                |   /* PIXEL ORDER in 128bit. first pixel in low address */
            (yuvFormat     << 20)                   |
            (colorFormat  << 19)                    | 
            (outputFormat << 16)                    |  
            (stride);

    VpuWriteReg(coreIdx, W4_COMMON_PIC_INFO, regVal);

    if ( pDecInfo->interResChange == 1)
    {
        i=0;
        // W4_SFB_OPTION [2:0] : 0x3 = SET_FB_UPDATE_MODE
        VpuWriteReg(coreIdx, W4_SFB_OPTION, (pDecInfo->openParam.fbc_mode<<20) | (endian<<16) | (1<<4) | (1<<3) | 3);
        if (mapType == COMPRESSED_FRAME_MAP) {
            idx=pDecInfo->chFbcFrameIdx;
        } else {
            idx=pDecInfo->chBwbFrameIdx;
        }
        VpuWriteReg(coreIdx, W4_SET_FB_NUM, (idx<<8)|idx);
        if (mapType == LINEAR_FRAME_MAP && pDecInfo->openParam.cbcrOrder == CBCR_ORDER_REVERSED) {
            addrY  = fbArr[idx].bufY;
            addrCb = fbArr[idx].bufCr;
            addrCr = fbArr[idx].bufCb;
        }
        else {
            addrY  = fbArr[idx].bufY;
            addrCb = fbArr[idx].bufCb;
            addrCr = fbArr[idx].bufCr;
        }
        VpuWriteReg(coreIdx, W4_ADDR_LUMA_BASE0  + (i<<4), addrY);
        VpuWriteReg(coreIdx, W4_ADDR_CB_BASE0    + (i<<4), addrCb);
        APIDPRINT("REGISTER FB[%02d] Y(0x%08x), Cb(0x%08x) ", i, addrY, addrCb);
        if (mapType == COMPRESSED_FRAME_MAP) {
            VpuWriteReg(coreIdx, W4_ADDR_FBC_Y_OFFSET0 + (i<<4), pDecInfo->vbFbcYTbl[idx].phys_addr); /* Luma FBC offset table */
            VpuWriteReg(coreIdx, W4_ADDR_FBC_C_OFFSET0 + (i<<4), pDecInfo->vbFbcCTbl[idx].phys_addr);        /* Chroma FBC offset table */
            VpuWriteReg(coreIdx, W4_ADDR_MV_COL0  + (i<<2), pDecInfo->vbMV[idx].phys_addr);
            APIDPRINT("Yo(0x%08x) Co(0x%08x), Mv(0x%08x)\n",
                pDecInfo->vbFbcYTbl[idx].phys_addr,
                pDecInfo->vbFbcCTbl[idx].phys_addr,
                pDecInfo->vbMV[idx].phys_addr);
        }
        else {
            VpuWriteReg(coreIdx, W4_ADDR_CR_BASE0 + (i<<4), addrCr);
            VpuWriteReg(coreIdx, W4_ADDR_FBC_C_OFFSET0 + (i<<4), 0);
            VpuWriteReg(coreIdx, W4_ADDR_MV_COL0  + (i<<2), 0);
            APIDPRINT("Cr(0x%08x)\n", addrCr);
        }
        VpuWriteReg(coreIdx, W4_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
        VpuWriteReg(coreIdx, W4_WORK_SIZE,      pDecInfo->vbWork.size);
        VpuWriteReg(coreIdx, W4_WORK_PARAM,     0);

        Wave4BitIssueCommand(inst, SET_FRAMEBUF);
        if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
    }
    else
    {
        remain = count;
        q      = (remain+7)/8;
        idx    = 0;
        for (j=0; j<q; j++) {
            regVal = (pDecInfo->openParam.fbc_mode<<20)|(endian<<16) | (j==q-1)<<4 | ((j==0)<<3) ;
            VpuWriteReg(coreIdx, W4_SFB_OPTION, regVal);
            startNo = j*8;
            endNo   = startNo + (remain>=8 ? 8 : remain) - 1;

            VpuWriteReg(coreIdx, W4_SET_FB_NUM, (startNo<<8)|endNo);

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
                VpuWriteReg(coreIdx, W4_ADDR_LUMA_BASE0  + (i<<4), addrY);
                VpuWriteReg(coreIdx, W4_ADDR_CB_BASE0    + (i<<4), addrCb);
                APIDPRINT("REGISTER FB[%02d] Y(0x%08x), Cb(0x%08x) ", i, addrY, addrCb);
                if (mapType == COMPRESSED_FRAME_MAP) {
                    VpuWriteReg(coreIdx, W4_ADDR_FBC_Y_OFFSET0 + (i<<4), pDecInfo->vbFbcYTbl[idx].phys_addr); /* Luma FBC offset table */
                    VpuWriteReg(coreIdx, W4_ADDR_FBC_C_OFFSET0 + (i<<4), pDecInfo->vbFbcCTbl[idx].phys_addr);        /* Chroma FBC offset table */
                    VpuWriteReg(coreIdx, W4_ADDR_MV_COL0  + (i<<2), pDecInfo->vbMV[idx].phys_addr);
                    APIDPRINT("Yo(0x%08x) Co(0x%08x), Mv(0x%08x)\n",
                        pDecInfo->vbFbcYTbl[idx].phys_addr,
                        pDecInfo->vbFbcCTbl[idx].phys_addr,
                        pDecInfo->vbMV[idx].phys_addr);
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

            VpuWriteReg(coreIdx, W4_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
            VpuWriteReg(coreIdx, W4_WORK_SIZE,      pDecInfo->vbWork.size);
            VpuWriteReg(coreIdx, W4_WORK_PARAM,     0);

            // CODA7Q needs temporal buffer address.
            VpuWriteReg(coreIdx, W4_ADDR_TEMP_BASE, pDecInfo->vbTemp.phys_addr);
            VpuWriteReg(coreIdx, W4_TEMP_SIZE,      pDecInfo->vbTemp.size);
            VpuWriteReg(coreIdx, W4_TEMP_PARAM,     0);

            Wave4BitIssueCommand(inst, SET_FRAMEBUF);
            if (vdi_wait_vpu_busy(coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }
        }
    }

    regVal = VpuReadReg(coreIdx, W4_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    if (inst->productId == PRODUCT_ID_7Q) {
        if (ConfigSecAXICoda7(coreIdx, pDecInfo->openParam.bitstreamFormat, 
            &pDecInfo->secAxiInfo, 
            pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight, sequenceInfo->profile) == 0) {
                return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }
    else {
        if (ConfigSecAXIWave(coreIdx, inst->codecMode, 
            &pDecInfo->secAxiInfo, pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight, 
            sequenceInfo->profile, sequenceInfo->level) == 0) {
                return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }

    return ret;
}

RetCode Wave4VpuDecFlush(CodecInst* instance, FramebufferIndex* framebufferIndexes, Uint32 size)
{
    Int32       regVal;
    Uint32      i, j;
    DecInfo*    pDecInfo = &instance->CodecInfo->decInfo;

    VpuWriteReg(instance->coreIdx, W4_FLUSH_DECODER_OPTION, FLUSH_DPB);
    /* Set up work-buffer */
    VpuWriteReg(instance->coreIdx, W4_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W4_WORK_SIZE,      pDecInfo->vbWork.size);
    VpuWriteReg(instance->coreIdx, W4_WORK_PARAM,     0);

    Wave4BitIssueCommand(instance, FLUSH_DECODER);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    // Clear display flags.
    pDecInfo->frameDisplayFlag = 0;

    regVal = VpuReadReg(instance->coreIdx, W4_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    /* Get remaining framebuffers */
    if (framebufferIndexes != NULL) {
        for (j=0,i=W4_RET_DEC_DISPLAY_INDEX_0; i<=W4_RET_DEC_DISPLAY_INDEX_15 && j<size; i+=4, j++) {
            regVal = VpuReadReg(instance->coreIdx, i);
            framebufferIndexes[j].linearIndex = regVal>>16;
            framebufferIndexes[j].tiledIndex  = regVal&0xffff;
            if (pDecInfo->wtlEnable == FALSE) 
                framebufferIndexes[j].linearIndex = framebufferIndexes[j].tiledIndex;
            APIDPRINT("FLUSH LINEAR(%d) TILED(%d)\n", framebufferIndexes[j].linearIndex, framebufferIndexes[j].tiledIndex);
        }
    }

    return RETCODE_SUCCESS;
}

RetCode Wave4VpuEncFiniSeq(CodecInst* instance)
{
    EncInfo*    pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);


    VpuWriteReg(instance->coreIdx, W4_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W4_WORK_SIZE,      pEncInfo->vbWork.size);
    VpuWriteReg(instance->coreIdx, W4_WORK_PARAM,     0);

    Wave4BitIssueCommand(instance, FINI_SEQ);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) 
        return RETCODE_VPU_RESPONSE_TIMEOUT;

    if (VpuReadReg(instance->coreIdx, W4_RET_SUCCESS) == FALSE)
        return RETCODE_FAILURE;

    return RETCODE_SUCCESS;
}

RetCode Wave4VpuBuildUpEncParam(CodecInst* instance, EncOpenParam* param)
{
    RetCode  ret = RETCODE_SUCCESS;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[instance->coreIdx];
    EncInfo* pEncInfo = &instance->CodecInfo->encInfo;

    pEncInfo->streamRdPtrRegAddr        = W4_BS_RD_PTR;
    pEncInfo->streamWrPtrRegAddr        = W4_BS_WR_PTR;
    pEncInfo->currentPC                 = W4_VCPU_CUR_PC;
    pEncInfo->busyFlagAddr              = W4_VPU_BUSY_STATUS;
    if (instance->productId == PRODUCT_ID_7Q) {
        if (param->bitstreamFormat == STD_HEVC)
            instance->codecMode = C7_HEVC_ENC;
        else if (param->bitstreamFormat == STD_AVC)
            instance->codecMode = C7_AVC_ENC;
        else if (param->bitstreamFormat == STD_MPEG4 || param->bitstreamFormat == STD_H263)
            instance->codecMode = C7_MP4_ENC;
        else 
            return RETCODE_NOT_SUPPORTED_FEATURE;
    }
    else {
        if (param->bitstreamFormat == STD_HEVC)
            instance->codecMode = HEVC_ENC;
        else 
            return RETCODE_NOT_SUPPORTED_FEATURE;
    }
 
    if ((pAttr->supportEncoders&(1<<param->bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;


    pEncInfo->vbWork.size        = WAVE4ENC_WORKBUF_SIZE;

    if (vdi_allocate_dma_memory(instance->coreIdx, &pEncInfo->vbWork) < 0)
    {
        pEncInfo->vbWork.base      = 0;
        pEncInfo->vbWork.phys_addr = 0;
        pEncInfo->vbWork.size      = 0;
        pEncInfo->vbWork.virt_addr = 0;
        return RETCODE_INSUFFICIENT_RESOURCE;
    }
    vdi_clear_memory(instance->coreIdx, pEncInfo->vbWork.phys_addr, pEncInfo->vbWork.size, 0);


    VpuWriteReg(instance->coreIdx, W4_ADDR_WORK_BASE, pEncInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W4_WORK_SIZE,      pEncInfo->vbWork.size);
    VpuWriteReg(instance->coreIdx, W4_WORK_PARAM,     0);

    Wave4BitIssueCommand(instance, CREATE_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, __VPU_BUSY_TIMEOUT, W4_VPU_BUSY_STATUS) == -1) {
        if (instance->loggingEnable)		
            vdi_log(instance->coreIdx, CREATE_INSTANCE, 2);
        vdi_free_dma_memory(instance->coreIdx, &pEncInfo->vbWork);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W4_RET_SUCCESS) == FALSE) {
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
    pEncInfo->secAxiInfo.u.wave4.useBitEnable   = 0;
    pEncInfo->secAxiInfo.u.wave4.useIpEnable    = 0;
    pEncInfo->secAxiInfo.u.wave4.useLfRowEnable = 0;

    pEncInfo->rotationEnable        = 0;
    pEncInfo->mirrorEnable          = 0;
    pEncInfo->mirrorDirection       = MIRDIR_NONE;
    pEncInfo->rotationAngle         = 0;
    pEncInfo->initialInfoObtained   = 0;
    pEncInfo->ringBufferEnable      = param->ringBufferEnable;	
    pEncInfo->sliceIntEnable      = 0;


    return ret;
}

