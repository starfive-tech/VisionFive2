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

#include "jpuapifunc.h"
#include "jpuapi.h"
#include "regdefine.h"
#include "jpulog.h"

static JPUCap   g_JpuAttributes;

int JPU_IsBusy(JpgHandle handle)
{
    Uint32 val;
    JpgInst *pJpgInst = (JpgInst *)handle;
    Int32 instRegIndex;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }
    val = JpuReadInstReg(instRegIndex, MJPEG_PIC_STATUS_REG);

    if ((val & (1<<INT_JPU_DONE)) ||
        (val & (1<<INT_JPU_ERROR)))
        return 0;

    return 1;
}

void JPU_ClrStatus(JpgHandle handle, Uint32 val)
{
    JpgInst *pJpgInst = (JpgInst *)handle;
    Int32 instRegIndex;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }
    if (val != 0)
        JpuWriteInstReg(instRegIndex, MJPEG_PIC_STATUS_REG, val);
}
Uint32 JPU_GetStatus(JpgHandle handle)
{
    JpgInst *pJpgInst = (JpgInst *)handle;
    Int32 instRegIndex;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    return JpuReadInstReg(instRegIndex, MJPEG_PIC_STATUS_REG);
}


Uint32 JPU_IsInit()
{
    jpu_instance_pool_t *pjip;

    pjip = (jpu_instance_pool_t *)jdi_get_instance_pool();

    if (!pjip)
        return 0;

    return 1;
}


Int32	JPU_WaitInterrupt(JpgHandle handle, int timeout)
{
    Uint32 val;
    Uint32 instPicStatusRegAddr;
    Int32 instRegIndex;

    Int32 reason = 0;

    JpgInst *pJpgInst = (JpgInst *)handle;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    instPicStatusRegAddr = ((instRegIndex*NPT_REG_SIZE) + MJPEG_PIC_STATUS_REG);

    reason = jdi_wait_interrupt(timeout, instPicStatusRegAddr, instRegIndex);
    if (reason == -1)
        return -1;

    if (reason & (1<<INT_JPU_DONE) || reason & (1<<INT_JPU_SLICE_DONE)) {
        val = JpuReadReg(MJPEG_INST_CTRL_STATUS_REG);
        if ((((val & 0xf) >> instRegIndex) & 0x01) == 0) {
            return -2;
        }
    }

    return reason;
}



int JPU_GetOpenInstanceNum()
{
    jpu_instance_pool_t *pjip;

    pjip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!pjip)
        return -1;

    return pjip->jpu_instance_num;
}

JpgRet JPU_Init()
{
    jpu_instance_pool_t *pjip;
    Uint32              val;

    if (jdi_init() < 0) {
        return JPG_RET_FAILURE;
    }

    if (jdi_get_task_num() > 1) {
        return JPG_RET_CALLED_BEFORE;
    }

    JpgEnterLock();
    pjip = (jpu_instance_pool_t *)jdi_get_instance_pool();
    if (!pjip) {
        JpgLeaveLock();
        return JPG_RET_FAILURE;
    }

    //jdi_log(JDI_LOG_CMD_INIT, 1, 0);
    JPU_SWReset(NULL);
    JpuWriteReg(MJPEG_INST_CTRL_START_REG, (1<<0));

    val = JpuReadInstReg(0, MJPEG_VERSION_INFO_REG);
    // JPU Capabilities
    g_JpuAttributes.productId    = (val>>24) & 0xf;
    g_JpuAttributes.revisoin     = (val & 0xffffff);
    g_JpuAttributes.support12bit = (val>>28) & 0x01;

    //jdi_log(JDI_LOG_CMD_INIT, 0, 0);
    JpgLeaveLock();
    return JPG_RET_SUCCESS;
}

void JPU_DeInit()
{
    JpgEnterLock();
    if (jdi_get_task_num() == 1) {
        JpuWriteReg(MJPEG_INST_CTRL_START_REG, 0);
    }
    JpgLeaveLock();
    jdi_release();
}

JpgRet JPU_GetVersionInfo(Uint32 *apiVersion, Uint32 *hwRevision, Uint32 *hwProductId)
{
    if (JPU_IsInit() == 0) {
        return JPG_RET_NOT_INITIALIZED;
    }

    JpgEnterLock();
    if (apiVersion) {
        *apiVersion = API_VERSION;
    }
    if (hwRevision) {
        *hwRevision = g_JpuAttributes.revisoin;
    }
    if (hwProductId) {
        *hwProductId = g_JpuAttributes.productId;
    }
    JpgLeaveLock();
    return JPG_RET_SUCCESS;
}

JpgRet JPU_DecOpen(JpgDecHandle * pHandle, JpgDecOpenParam * pop)
{
    JpgInst*    pJpgInst;
    JpgDecInfo* pDecInfo;
    JpgRet      ret;
    ret = CheckJpgDecOpenParam(pop);
    if (ret != JPG_RET_SUCCESS) {
        return ret;
    }

    JpgEnterLock();
    ret = GetJpgInstance(&pJpgInst);
    if (ret == JPG_RET_FAILURE) {
        *pHandle = 0;
        JpgLeaveLock();
        return JPG_RET_FAILURE;
    }

    *pHandle = pJpgInst;

    pDecInfo = &pJpgInst->JpgInfo->decInfo;
    memset(pDecInfo, 0x00, sizeof(JpgDecInfo));

    pDecInfo->streamWrPtr = pop->bitstreamBuffer;
    pDecInfo->streamRdPtr = pop->bitstreamBuffer;

    pDecInfo->streamBufStartAddr = pop->bitstreamBuffer;
    pDecInfo->streamBufSize     = pop->bitstreamBufferSize;
    pDecInfo->streamBufEndAddr  = pop->bitstreamBuffer + pop->bitstreamBufferSize;
    pDecInfo->pBitStream        = pop->pBitStream;
    pDecInfo->streamEndian      = pop->streamEndian;
    pDecInfo->frameEndian       = pop->frameEndian;
    pDecInfo->chromaInterleave  = pop->chromaInterleave;
    pDecInfo->packedFormat      = pop->packedFormat;
    pDecInfo->roiEnable         = pop->roiEnable;
    pDecInfo->roiWidth          = pop->roiWidth;
    pDecInfo->roiHeight         = pop->roiHeight;
    pDecInfo->roiOffsetX        = pop->roiOffsetX;
    pDecInfo->roiOffsetY        = pop->roiOffsetY;
    pDecInfo->pixelJustification = pop->pixelJustification;
    pDecInfo->intrEnableBit     = pop->intrEnableBit;
    pJpgInst->sliceInstMode     = 0;    /* The decoder does not support the slice-decoding */
    pDecInfo->rotationIndex     = pop->rotation / 90;
    pDecInfo->mirrorIndex       = pop->mirror;
    /* convert output format */
    switch (pop->outputFormat) {
    case FORMAT_400: ret = JPG_RET_INVALID_PARAM; break;
    case FORMAT_420: pDecInfo->ofmt = O_FMT_420;  break;
    case FORMAT_422: pDecInfo->ofmt = O_FMT_422;  break;
    case FORMAT_440: ret = JPG_RET_INVALID_PARAM; break;
    case FORMAT_444: pDecInfo->ofmt = O_FMT_444;  break;
    case FORMAT_MAX: pDecInfo->ofmt = O_FMT_NONE; break;
    default:         ret = JPG_RET_INVALID_PARAM; break;
    }

    pDecInfo->userqMatTab = 0;
    pDecInfo->decIdx = 0;

    JpgLeaveLock();

    return ret;
}

JpgRet JPU_DecClose(JpgDecHandle handle)
{
    JpgInst * pJpgInst;
    JpgRet ret;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    pJpgInst = handle;

    JpgEnterLock();

    if (GetJpgPendingInstEx(pJpgInst->instIndex)) {
        JpgLeaveLock();
        return JPG_RET_FRAME_NOT_COMPLETE;
    }

    FreeJpgInstance(pJpgInst);
    JpgLeaveLock();

    return JPG_RET_SUCCESS;
}

JpgRet JPU_DecGetInitialInfo(JpgDecHandle handle, JpgDecInitialInfo * info)
{
    JpgInst*    pJpgInst;
    JpgDecInfo* pDecInfo;
    JpgRet      ret;
    int         err;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    if (info == 0) {
        return JPG_RET_INVALID_PARAM;
    }
    pJpgInst = handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;

    if (0 >= (err=JpegDecodeHeader(pDecInfo))) {
        /* The value of -2 indicates that the bitstream in the buffer is not enough to decode a header. */
        return (-2 == err) ? JPG_RET_BIT_EMPTY : JPG_RET_FAILURE;
    }

    if (pDecInfo->jpg12bit == TRUE && g_JpuAttributes.support12bit == FALSE) {
        return JPG_RET_NOT_SUPPORT;
    }

    info->picWidth = pDecInfo->picWidth;
    info->picHeight = pDecInfo->picHeight;
    info->minFrameBufferCount = 1;
    info->sourceFormat = (FrameFormat)pDecInfo->format;
    info->ecsPtr = pDecInfo->ecsPtr;

    pDecInfo->initialInfoObtained = 1;
    pDecInfo->minFrameBufferNum = 1;


    if ((pDecInfo->packedFormat == PACKED_FORMAT_444) && (pDecInfo->format != FORMAT_444)) {
        return JPG_RET_INVALID_PARAM;
    }

    if (pDecInfo->roiEnable) {
        if (pDecInfo->format == FORMAT_400) {
            pDecInfo->roiMcuWidth = pDecInfo->roiWidth/8;
        }
        else  {
            pDecInfo->roiMcuWidth = pDecInfo->roiWidth/pDecInfo->mcuWidth;
        }
        pDecInfo->roiMcuHeight = pDecInfo->roiHeight/pDecInfo->mcuHeight;
        pDecInfo->roiMcuOffsetX = pDecInfo->roiOffsetX/pDecInfo->mcuWidth;
        pDecInfo->roiMcuOffsetY = pDecInfo->roiOffsetY/pDecInfo->mcuHeight;

        if ((pDecInfo->roiOffsetX > pDecInfo->alignedWidth)
            || (pDecInfo->roiOffsetY > pDecInfo->alignedHeight)
            || (pDecInfo->roiOffsetX + pDecInfo->roiWidth > pDecInfo->alignedWidth)
            || (pDecInfo->roiOffsetY + pDecInfo->roiHeight > pDecInfo->alignedHeight))
            return JPG_RET_INVALID_PARAM;

        if (pDecInfo->format == FORMAT_400) {
            if (((pDecInfo->roiOffsetX + pDecInfo->roiWidth) < 8) || ((pDecInfo->roiOffsetY + pDecInfo->roiHeight) < pDecInfo->mcuHeight))
                return JPG_RET_INVALID_PARAM;
        }
        else {
            if (((pDecInfo->roiOffsetX + pDecInfo->roiWidth) < pDecInfo->mcuWidth) || ((pDecInfo->roiOffsetY + pDecInfo->roiHeight) < pDecInfo->mcuHeight))
                return JPG_RET_INVALID_PARAM;
        }

        if (pDecInfo->format == FORMAT_400)
            pDecInfo->roiFrameWidth = pDecInfo->roiMcuWidth * 8;
        else
            pDecInfo->roiFrameWidth = pDecInfo->roiMcuWidth * pDecInfo->mcuWidth;
        pDecInfo->roiFrameHeight = pDecInfo->roiMcuHeight*pDecInfo->mcuHeight;
        info->roiFrameWidth   = pDecInfo->roiFrameWidth;
        info->roiFrameHeight  = pDecInfo->roiFrameHeight;
        info->roiFrameOffsetX = pDecInfo->roiMcuOffsetX*pDecInfo->mcuWidth;
        info->roiFrameOffsetY = pDecInfo->roiMcuOffsetY*pDecInfo->mcuHeight;
    }
    info->bitDepth        = pDecInfo->bitDepth;


    return JPG_RET_SUCCESS;
}


JpgRet JPU_DecRegisterFrameBuffer(JpgDecHandle handle, FrameBuffer * bufArray, int num, int stride)
{
    JpgInst * pJpgInst;
    JpgDecInfo * pDecInfo;
    JpgRet ret;


    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    pJpgInst = handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;


    if (!pDecInfo->initialInfoObtained) {
        return JPG_RET_WRONG_CALL_SEQUENCE;
    }

    if (bufArray == 0) {
        return JPG_RET_INVALID_FRAME_BUFFER;
    }

    if (num < pDecInfo->minFrameBufferNum) {
        return JPG_RET_INSUFFICIENT_FRAME_BUFFERS;
    }

    if ((stride % 8) != 0) {
        return JPG_RET_INVALID_STRIDE;
    }

    pDecInfo->frameBufPool    = bufArray;
    pDecInfo->numFrameBuffers = num;
    pDecInfo->stride          = stride;
    pDecInfo->stride_c        = bufArray[0].strideC;

    return JPG_RET_SUCCESS;
}

JpgRet JPU_DecGetBitstreamBuffer(JpgDecHandle handle,
    PhysicalAddress * prdPtr,
    PhysicalAddress * pwrPtr,
    int * size)
{
    JpgInst * pJpgInst;
    JpgDecInfo * pDecInfo;
    JpgRet ret;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    int room;
    Int32 instRegIndex;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    pJpgInst = handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    if (GetJpgPendingInstEx(pJpgInst->instIndex) == pJpgInst) {
        rdPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_RD_PTR_REG);
    }
    else {
        rdPtr = pDecInfo->streamRdPtr;
    }

    wrPtr = pDecInfo->streamWrPtr;

    if (wrPtr == pDecInfo->streamBufStartAddr) {
        if (pDecInfo->frameOffset == 0) {
            room = (pDecInfo->streamBufEndAddr-pDecInfo->streamBufStartAddr);
        }
        else {
            room = (pDecInfo->frameOffset);
        }
    }
    else {
        room = (pDecInfo->streamBufEndAddr - wrPtr);
    }

    room = ((room>>9)<<9); // multiple of 512

    if (prdPtr) *prdPtr = rdPtr;
    if (pwrPtr) *pwrPtr = wrPtr;
    if (size)   *size = room;

    return JPG_RET_SUCCESS;
}


JpgRet JPU_DecUpdateBitstreamBuffer(JpgDecHandle handle, int size)
{
    JpgInst * pJpgInst;
    JpgDecInfo * pDecInfo;
    PhysicalAddress wrPtr;
    PhysicalAddress rdPtr;
    JpgRet ret;
    int		val = 0;
    Int32 instRegIndex;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    pJpgInst = handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;
    wrPtr = pDecInfo->streamWrPtr;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    if (size == 0)
    {
        val = (wrPtr-pDecInfo->streamBufStartAddr) / 256;
        if ((wrPtr-pDecInfo->streamBufStartAddr) % 256)
            val = val + 1;

        if (GetJpgPendingInstEx(pJpgInst->instIndex) == pJpgInst) {
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_STRM_CTRL_REG, (1UL << 31 | val));
        }
        pDecInfo->streamEndflag = 1;
        return JPG_RET_SUCCESS;
    }

    wrPtr = pDecInfo->streamWrPtr;
    wrPtr += size;

    if (wrPtr == pDecInfo->streamBufEndAddr) {
        wrPtr = pDecInfo->streamBufStartAddr;
    }

    pDecInfo->streamWrPtr = wrPtr;

    if (GetJpgPendingInstEx(pJpgInst->instIndex) == pJpgInst) {
        rdPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_RD_PTR_REG);

        if (rdPtr >= pDecInfo->streamBufEndAddr) {
            JLOG(INFO, "!!!!! WRAP-AROUND !!!!!\n");
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_CUR_POS_REG, 0);
            JpuWriteInstReg(instRegIndex, MJPEG_GBU_TCNT_REG, 0);
            JpuWriteInstReg(instRegIndex, (MJPEG_GBU_TCNT_REG+4), 0);
        }

        JpuWriteInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG, wrPtr);
        if (wrPtr == pDecInfo->streamBufStartAddr) {
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_END_ADDR_REG, pDecInfo->streamBufEndAddr);
        }
        else {
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_END_ADDR_REG, wrPtr);
        }
    }
    else {
        rdPtr = pDecInfo->streamRdPtr;
    }

    pDecInfo->streamRdPtr = rdPtr;

    return JPG_RET_SUCCESS;
}

JpgRet JPU_SWReset(JpgHandle handle)
{
    Uint32 val;

    int clock_state;
    JpgInst *pJpgInst;

    pJpgInst = ((JpgInst *)handle);
    clock_state = jdi_get_clock_gate();

    if (clock_state == 0)
        jdi_set_clock_gate(1);

    if (handle)
        jdi_log(JDI_LOG_CMD_RESET, 1, pJpgInst->instIndex);

    val = JpuReadReg(MJPEG_INST_CTRL_START_REG);
    val |= (1<<2);
    JpuWriteReg(MJPEG_INST_CTRL_START_REG, val);
    if (jdi_wait_inst_ctrl_busy(JPU_INST_CTRL_TIMEOUT_MS, MJPEG_INST_CTRL_STATUS_REG, INST_CTRL_IDLE) == -1) {// wait for INST_CTRL become IDLE
        if (handle)
            jdi_log(JDI_LOG_CMD_RESET, 0, pJpgInst->instIndex);
        val &= ~(1<<2);
        JpuWriteReg(MJPEG_INST_CTRL_START_REG, val);
        if (clock_state == 0)
            jdi_set_clock_gate(0);
        return JPG_RET_INST_CTRL_ERROR;
    }
    val &= ~(1<<2);
    JpuWriteReg(MJPEG_INST_CTRL_START_REG, val);

    if (handle)
        jdi_log(JDI_LOG_CMD_RESET, 0, pJpgInst->instIndex);

    if (clock_state == 0)
        jdi_set_clock_gate(0);

    return JPG_RET_SUCCESS;
}
JpgRet JPU_HWReset()
{
    if (jdi_hw_reset() < 0 )
        return JPG_RET_FAILURE;

    return JPG_RET_SUCCESS;
}

JpgRet JPU_DecSetRdPtr(JpgDecHandle handle, PhysicalAddress addr, BOOL updateWrPtr)
{
    JpgInst*    pJpgInst;
    JpgDecInfo* pDecInfo;
    JpgRet      ret;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    pJpgInst = (JpgInst*)handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;

    JpgEnterLock();

    if (GetJpgPendingInstEx(pJpgInst->instIndex)) {
        JpgLeaveLock();
        return JPG_RET_FRAME_NOT_COMPLETE;
    }
    pDecInfo->streamRdPtr = addr;
    if (updateWrPtr)
        pDecInfo->streamWrPtr = addr;

    pDecInfo->frameOffset = addr - pDecInfo->streamBufStartAddr;
    pDecInfo->consumeByte = 0;

    JpuWriteReg(MJPEG_BBC_RD_PTR_REG, pDecInfo->streamRdPtr);

    JpgLeaveLock();

    return JPG_RET_SUCCESS;
}

JpgRet JPU_DecSetRdPtrEx(JpgDecHandle handle, PhysicalAddress addr, BOOL updateWrPtr)
{
    JpgInst*    pJpgInst;
    JpgDecInfo* pDecInfo;
    JpgRet      ret;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    pJpgInst = (JpgInst*)handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;

    JpgEnterLock();

    if (GetJpgPendingInstEx(pJpgInst->instIndex)) {
        JpgLeaveLock();
        return JPG_RET_FRAME_NOT_COMPLETE;
    }
    pDecInfo->streamRdPtr = addr;
    pDecInfo->streamBufStartAddr = addr;
    if (updateWrPtr)
        pDecInfo->streamWrPtr = addr;

    pDecInfo->frameOffset = 0;
    pDecInfo->consumeByte = 0;

    JpuWriteReg(MJPEG_BBC_RD_PTR_REG, pDecInfo->streamRdPtr);

    JpgLeaveLock();

    return JPG_RET_SUCCESS;
}

JpgRet JPU_DecStartOneFrame(JpgDecHandle handle, JpgDecParam *param)
{
    JpgInst * pJpgInst;
    JpgDecInfo * pDecInfo;
    JpgRet ret;
    Uint32 val;
    PhysicalAddress rdPtr, wrPtr;
    BOOL    is12Bit     = FALSE;
    BOOL    ppuEnable   = FALSE;
    Int32 instRegIndex;
    BOOL bTableInfoUpdate;
    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;


    pJpgInst = handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;
    is12Bit  = (pDecInfo->bitDepth == 8) ? FALSE : TRUE;

    if (pDecInfo->frameBufPool == 0) { // This means frame buffers have not been registered.
        return JPG_RET_WRONG_CALL_SEQUENCE;
    }

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    JpgEnterLock();
    if (GetJpgPendingInstEx(pJpgInst->instIndex) == pJpgInst) {
        JpgLeaveLock();
        return JPG_RET_FRAME_NOT_COMPLETE;
    }


    if (pDecInfo->frameOffset < 0) {
        SetJpgPendingInstEx(0, pJpgInst->instIndex);
        return JPG_RET_EOS;
    }

    pDecInfo->q_prec0 = 0;
    pDecInfo->q_prec1 = 0;
    pDecInfo->q_prec2 = 0;
    pDecInfo->q_prec3 = 0;

    //check for stream empty case
    if (pDecInfo->streamEndflag == 0) {
        rdPtr = pDecInfo->streamRdPtr;
        wrPtr = pDecInfo->streamWrPtr;
        if (wrPtr == pDecInfo->streamBufStartAddr)
            wrPtr = pDecInfo->streamBufEndAddr;
        if (rdPtr > wrPtr) { // wrap-around case
            if ( ((pDecInfo->streamBufEndAddr-rdPtr) + (wrPtr-pDecInfo->streamBufStartAddr)) <1024 ) {
                JpgLeaveLock();
                return JPG_RET_BIT_EMPTY;
            }
        }
        else {
            if (wrPtr - rdPtr < 1024) {
                JpgLeaveLock();
                return JPG_RET_BIT_EMPTY;
            }
        }
    }

    {
        val = JpegDecodeHeader(pDecInfo);
        if (val == 0) {
            JpgLeaveLock();
            return JPG_RET_FAILURE;
        }

        if (val == (Uint32)-2) {	// wrap around case
            pDecInfo->frameOffset = 0;
            pDecInfo->ecsPtr = 0;
            val = JpegDecodeHeader(pDecInfo);
            if (val == 0) {
                JpgLeaveLock();
                return JPG_RET_FAILURE;
            }
        }

        if (val == (Uint32)-1) {	//stream empty case
            if (pDecInfo->streamEndflag == 1) {
                SetJpgPendingInstEx(0, pJpgInst->instIndex);
                pDecInfo->frameOffset = -1;
                if (pJpgInst->sliceInstMode == TRUE) {
                    JpgLeaveLock();
                }
                return JPG_RET_EOS;
            }
            JpgLeaveLock();
            return JPG_RET_BIT_EMPTY;
        }
    }

    JpuWriteInstReg(instRegIndex, MJPEG_INTR_MASK_REG, ((~pDecInfo->intrEnableBit) & 0x3ff));
    /* The registers related to the slice encoding should be clear */
    JpuWriteInstReg(instRegIndex, MJPEG_SLICE_INFO_REG,    pDecInfo->alignedHeight);
    JpuWriteInstReg(instRegIndex, MJPEG_SLICE_DPB_POS_REG, pDecInfo->alignedHeight);
    JpuWriteInstReg(instRegIndex, MJPEG_SLICE_POS_REG,     0);
    JpuWriteInstReg(instRegIndex, MJPEG_PIC_SETMB_REG,     0);

    if (pDecInfo->streamRdPtr == pDecInfo->streamBufEndAddr) {
        JpuWriteInstReg(instRegIndex, MJPEG_BBC_CUR_POS_REG, 0);
        JpuWriteInstReg(instRegIndex, MJPEG_GBU_TCNT_REG, 0);
        JpuWriteInstReg(instRegIndex, (MJPEG_GBU_TCNT_REG+4), 0);
    }

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG, pDecInfo->streamWrPtr);
    if (pDecInfo->streamWrPtr == pDecInfo->streamBufStartAddr) {
        JpuWriteInstReg(instRegIndex, MJPEG_BBC_END_ADDR_REG, pDecInfo->streamBufEndAddr);
    }
    else {
        JpuWriteInstReg(instRegIndex, MJPEG_BBC_END_ADDR_REG, JPU_CEIL(256, pDecInfo->streamWrPtr));
    }

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_BAS_ADDR_REG, pDecInfo->streamBufStartAddr);
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_TCNT_REG, 0);
    JpuWriteInstReg(instRegIndex, (MJPEG_GBU_TCNT_REG+4), 0);
    JpuWriteInstReg(instRegIndex, MJPEG_PIC_ERRMB_REG, 0);
    JpuWriteInstReg(instRegIndex, MJPEG_PIC_CTRL_REG, is12Bit << 31 | pDecInfo->q_prec0 << 30 | pDecInfo->q_prec1 << 29 | pDecInfo->q_prec2 << 28 | pDecInfo->q_prec3 << 27 |
                                                       pDecInfo->huffAcIdx << 13 | pDecInfo->huffDcIdx << 7 | pDecInfo->userHuffTab << 6 |
                                                       (JPU_CHECK_WRITE_RESPONSE_BVALID_SIGNAL << 2) | 0);

    JpuWriteInstReg(instRegIndex, MJPEG_PIC_SIZE_REG, (pDecInfo->alignedWidth << 16) | pDecInfo->alignedHeight);

    JpuWriteInstReg(instRegIndex, MJPEG_OP_INFO_REG,  pDecInfo->busReqNum);
    JpuWriteInstReg(instRegIndex, MJPEG_MCU_INFO_REG, (pDecInfo->mcuBlockNum&0x0f) << 17 | (pDecInfo->compNum&0x07) << 14    |
                                                      (pDecInfo->compInfo[0]&0x3f) << 8  | (pDecInfo->compInfo[1]&0x0f) << 4 |
                                                      (pDecInfo->compInfo[2]&0x0f));

    //enable intlv NV12: 10, NV21: 11
    //packedFormat:0 => 4'd0
    //packedFormat:1,2,3,4 => 4, 5, 6, 7,
    //packedFormat:5 => 8
    //packedFormat:6 => 9
    val = (pDecInfo->ofmt << 9) | (pDecInfo->frameEndian << 6) | ((pDecInfo->chromaInterleave==0)?0:(pDecInfo->chromaInterleave==1)?2:3);
    if (pDecInfo->packedFormat == PACKED_FORMAT_NONE) {
        val |= (0<<5) | (0<<4);
    }
    else if (pDecInfo->packedFormat == PACKED_FORMAT_444) {
        val |= (1<<5) | (0<<4) | (0<<2);
    }
    else {
        val |= (0<<5) | (1<<4) | ((pDecInfo->packedFormat-1)<<2);
    }
    val |= (pDecInfo->pixelJustification << 11);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_CONFIG_REG, val);
    JpuWriteInstReg(instRegIndex, MJPEG_RST_INTVAL_REG, pDecInfo->rstIntval);

    if (param) {
        if (param->scaleDownRatioWidth > 0 )
            pDecInfo->iHorScaleMode = param->scaleDownRatioWidth;
        if (param->scaleDownRatioHeight > 0 )
            pDecInfo->iVerScaleMode = param->scaleDownRatioHeight;
    }
    if (pDecInfo->iHorScaleMode | pDecInfo->iVerScaleMode)
        val = ((pDecInfo->iHorScaleMode&0x3)<<2) | ((pDecInfo->iVerScaleMode&0x3)) | 0x10 ;
    else {
        val = 0;
    }
    JpuWriteInstReg(instRegIndex, MJPEG_SCL_INFO_REG, val);

    bTableInfoUpdate = FALSE;
    if (pDecInfo->userHuffTab) {
        bTableInfoUpdate = TRUE;
    }

    if (bTableInfoUpdate == TRUE) {
        if (is12Bit == TRUE){
            if (!JpgDecHuffTabSetUp_12b(pDecInfo, instRegIndex)) {
                JpgLeaveLock();
                return JPG_RET_INVALID_PARAM;
            }
        }else{
            if (!JpgDecHuffTabSetUp(pDecInfo, instRegIndex)) {
                JpgLeaveLock();
                return JPG_RET_INVALID_PARAM;
            }
        }
    }

    bTableInfoUpdate = TRUE; // it always should be TRUE for multi-instance
    if (bTableInfoUpdate == TRUE) {
        if (!JpgDecQMatTabSetUp(pDecInfo, instRegIndex)) {
            JpgLeaveLock();
            return JPG_RET_INVALID_PARAM;
        }
    }

    JpgDecGramSetup(pDecInfo, instRegIndex);

    if (pDecInfo->streamEndflag == 1) {
        val = JpuReadInstReg(instRegIndex, MJPEG_BBC_STRM_CTRL_REG);
        if ((val & (1UL << 31)) == 0 ) {
            val = (pDecInfo->streamWrPtr-pDecInfo->streamBufStartAddr) / 256;
            if ((pDecInfo->streamWrPtr-pDecInfo->streamBufStartAddr) % 256)
                val = val + 1;
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_STRM_CTRL_REG, (1UL << 31 | val));
        }
    }
    else {
        JpuWriteInstReg(instRegIndex, MJPEG_BBC_STRM_CTRL_REG, 0);
    }

    JpuWriteInstReg(instRegIndex, MJPEG_RST_INDEX_REG, 0);	// RST index at the beginning.
    JpuWriteInstReg(instRegIndex, MJPEG_RST_COUNT_REG, 0);

    JpuWriteInstReg(instRegIndex, MJPEG_DPCM_DIFF_Y_REG, 0);
    JpuWriteInstReg(instRegIndex, MJPEG_DPCM_DIFF_CB_REG, 0);
    JpuWriteInstReg(instRegIndex, MJPEG_DPCM_DIFF_CR_REG, 0);

    JpuWriteInstReg(instRegIndex, MJPEG_GBU_FF_RPTR_REG, pDecInfo->bitPtr);
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_CTRL_REG, 3);

    ppuEnable = (pDecInfo->rotationIndex > 0) || (pDecInfo->mirrorIndex > 0);
    JpuWriteInstReg(instRegIndex, MJPEG_ROT_INFO_REG, (ppuEnable<<4) | (pDecInfo->mirrorIndex<<2) | pDecInfo->rotationIndex);

    val = (pDecInfo->frameIdx % pDecInfo->numFrameBuffers);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_BASE00_REG, pDecInfo->frameBufPool[val].bufY);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_BASE01_REG, pDecInfo->frameBufPool[val].bufCb);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_BASE02_REG, pDecInfo->frameBufPool[val].bufCr);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_YSTRIDE_REG, pDecInfo->stride);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_CSTRIDE_REG, pDecInfo->stride_c);

    if (pDecInfo->roiEnable) {
        JpuWriteInstReg(instRegIndex, MJPEG_CLP_INFO_REG, 1);
        JpuWriteInstReg(instRegIndex, MJPEG_CLP_BASE_REG, pDecInfo->roiOffsetX << 16 | pDecInfo->roiOffsetY);	// pixel unit
        JpuWriteInstReg(instRegIndex, MJPEG_CLP_SIZE_REG, (pDecInfo->roiFrameWidth << 16) | pDecInfo->roiFrameHeight); // pixel Unit
    }
    else {
        JpuWriteInstReg(instRegIndex, MJPEG_CLP_INFO_REG, 0);
    }

    if (pJpgInst->loggingEnable)
        jdi_log(JDI_LOG_CMD_PICRUN, 1, instRegIndex);

    if (pJpgInst->sliceInstMode == TRUE) {
        JpuWriteInstReg(instRegIndex, MJPEG_PIC_START_REG, (1<<JPG_DEC_SLICE_ENABLE_START_PIC));
    }
    else {
        JpuWriteInstReg(instRegIndex, MJPEG_PIC_START_REG, (1<<JPG_ENABLE_START_PIC));
    }

    pDecInfo->decIdx++;

    SetJpgPendingInstEx(pJpgInst, pJpgInst->instIndex);
    if (pJpgInst->sliceInstMode == TRUE) {
        JpgLeaveLock();
    }
    return JPG_RET_SUCCESS;
}

JpgRet JPU_DecGetOutputInfo(JpgDecHandle handle, JpgDecOutputInfo * info)
{
    JpgInst*    pJpgInst;
    JpgDecInfo* pDecInfo;
    JpgRet      ret;
    Uint32      val = 0;
    Int32       instRegIndex;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS) {
        return ret;
    }

    if (info == 0) {
        return JPG_RET_INVALID_PARAM;
    }

    pJpgInst = handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    if (pJpgInst->sliceInstMode == TRUE) {
        JpgEnterLock();
    }

    if (pJpgInst != GetJpgPendingInstEx(pJpgInst->instIndex)) {
        JpgLeaveLock();
        return JPG_RET_WRONG_CALL_SEQUENCE;
    }


    if (pDecInfo->frameOffset < 0)
    {
        info->numOfErrMBs = 0;
        info->decodingSuccess = 1;
        info->indexFrameDisplay = -1;
        SetJpgPendingInstEx(0, pJpgInst->instIndex);

        JpgLeaveLock();
        return JPG_RET_SUCCESS;
    }

    if (pDecInfo->roiEnable) {
        info->decPicWidth = pDecInfo->roiMcuWidth*pDecInfo->mcuWidth;
        info->decPicHeight = pDecInfo->roiMcuHeight*pDecInfo->mcuHeight;
    }
    else {
        info->decPicWidth = pDecInfo->alignedWidth;
        info->decPicHeight = pDecInfo->alignedHeight;
    }

    info->decPicWidth  >>= pDecInfo->iHorScaleMode;
    info->decPicHeight >>= pDecInfo->iVerScaleMode;

    info->indexFrameDisplay = (pDecInfo->frameIdx%pDecInfo->numFrameBuffers);
    info->consumedByte = (JpuReadInstReg(instRegIndex, MJPEG_GBU_TCNT_REG))/8;
    pDecInfo->streamRdPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_RD_PTR_REG);
    pDecInfo->consumeByte = info->consumedByte - 16 - pDecInfo->ecsPtr;
	
	/*
 		fix up the program may not be able to exit normally when decoder some pictures.
		due to  pDecInfo->consumeByte==0 when not first enter.
	*/
	if(pDecInfo->consumeByte == 0) 
		pDecInfo->consumeByte += 2;

    info->bytePosFrameStart = pDecInfo->frameOffset;
    info->ecsPtr = pDecInfo->ecsPtr;
    info->rdPtr  = pDecInfo->streamRdPtr;
    info->wrPtr  = JpuReadInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG);

    pDecInfo->ecsPtr = 0;
    pDecInfo->frameIdx++;

    val = JpuReadInstReg(instRegIndex, MJPEG_PIC_STATUS_REG);
    if (val & (1<<INT_JPU_DONE)) {
        info->decodingSuccess  = 1;
        info->numOfErrMBs      = 0;
    }
    else if (val & (1<<INT_JPU_ERROR)){
        info->numOfErrMBs = JpuReadInstReg(instRegIndex, MJPEG_PIC_ERRMB_REG);
        info->decodingSuccess = 0;

    }

#ifndef JPU_IRQ_CONTROL
	info->decodingSuccess  = 1;
	info->numOfErrMBs      = 0;
#endif

    info->frameCycle = JpuReadInstReg(instRegIndex, MJPEG_CYCLE_INFO_REG);

    if (val != 0)
        JpuWriteInstReg(instRegIndex, MJPEG_PIC_STATUS_REG, val);

    if (pJpgInst->loggingEnable)
        jdi_log(JDI_LOG_CMD_PICRUN, 0, instRegIndex);


    JpuWriteInstReg(instRegIndex, MJPEG_PIC_START_REG, 0);

    val = JpuReadReg(MJPEG_INST_CTRL_STATUS_REG);
    val &= ~(1UL<<instRegIndex);
    JpuWriteReg(MJPEG_INST_CTRL_STATUS_REG, val);

    SetJpgPendingInstEx(0, pJpgInst->instIndex);
    JpgLeaveLock();

    return JPG_RET_SUCCESS;
}


JpgRet JPU_DecGiveCommand(
    JpgDecHandle handle,
    JpgCommand cmd,
    void * param)
{
    JpgInst * pJpgInst;
    JpgDecInfo * pDecInfo;
    JpgRet ret;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;


    pJpgInst = handle;
    pDecInfo = &pJpgInst->JpgInfo->decInfo;
    switch (cmd)
    {
    case SET_JPG_SCALE_HOR:
        {
            int scale;
            scale = *(int *)param;
            if (pDecInfo->alignedWidth < 128 || pDecInfo->alignedHeight < 128) {
                if (scale) {
                    return JPG_RET_INVALID_PARAM;
                }
            }

            pDecInfo->iHorScaleMode = scale;
            break;
        }
    case SET_JPG_SCALE_VER:
        {
            int scale;
            scale = *(int *)param;
            if (pDecInfo->alignedWidth < 128 || pDecInfo->alignedHeight < 128) {
                if (scale) {
                    return JPG_RET_INVALID_PARAM;
                }
            }
            pDecInfo->iVerScaleMode = scale;
            break;
        }
    case ENABLE_LOGGING:
        {
            pJpgInst->loggingEnable = 1;
        }
        break;
    case DISABLE_LOGGING:
        {
            pJpgInst->loggingEnable = 0;
        }
        break;
    default:
        return JPG_RET_INVALID_COMMAND;
    }
    return JPG_RET_SUCCESS;
}

JpgRet JPU_EncOpen(JpgEncHandle * pHandle, JpgEncOpenParam * pop)
{
    JpgInst*    pJpgInst;
    JpgEncInfo* pEncInfo;
    JpgRet      ret;
    Uint32      i;
    Uint32      mcuWidth, mcuHeight;
    Uint32      comp0McuWidth, comp0McuHeight;
    BOOL        rotation_90_270 = FALSE;
    FrameFormat format;

    ret = CheckJpgEncOpenParam(pop, &g_JpuAttributes);
    if (ret != JPG_RET_SUCCESS) {
        return ret;
    }

    JpgEnterLock();
    ret = GetJpgInstance(&pJpgInst);
    if (ret == JPG_RET_FAILURE) {
        JpgLeaveLock();
        return JPG_RET_FAILURE;
    }

    *pHandle = pJpgInst;
    pEncInfo = &pJpgInst->JpgInfo->encInfo;
    memset(pEncInfo, 0x00, sizeof(JpgEncInfo));
    pEncInfo->streamRdPtr        = pop->bitstreamBuffer;
    pEncInfo->streamWrPtr        = pop->bitstreamBuffer;
    pEncInfo->sliceHeight        = pop->sliceHeight;
    pEncInfo->intrEnableBit      = pop->intrEnableBit;

    pEncInfo->streamBufStartAddr = pop->bitstreamBuffer;
    pEncInfo->streamBufEndAddr   = pop->bitstreamBuffer + pop->bitstreamBufferSize - (4096);
    pEncInfo->streamEndian       = pop->streamEndian;
    pEncInfo->frameEndian        = pop->frameEndian;
    pEncInfo->chromaInterleave   = pop->chromaInterleave;
    pEncInfo->format             = pop->sourceFormat;
    pEncInfo->picWidth           = pop->picWidth;
    pEncInfo->picHeight          = pop->picHeight;

    if (pop->rotation == 90 || pop->rotation == 270) {
        if (pEncInfo->format == FORMAT_422)      format = FORMAT_440;
        else if (pEncInfo->format == FORMAT_440) format = FORMAT_422;
        else                                     format = pEncInfo->format;
        rotation_90_270 = TRUE;
    }
    else {
        format = pEncInfo->format;
    }

    // Picture size alignment
    if (format == FORMAT_420 || format == FORMAT_422) {
        pEncInfo->alignedWidth = JPU_CEIL(16, pEncInfo->picWidth);
        mcuWidth = 16;
    }
    else {
        pEncInfo->alignedWidth = JPU_CEIL(8, pEncInfo->picWidth);
        mcuWidth = (format == FORMAT_400) ? 32 : 8;
    }

    if (format == FORMAT_420 || format == FORMAT_440) {
        pEncInfo->alignedHeight = JPU_CEIL(16, pEncInfo->picHeight);
        mcuHeight = 16;
    }
    else {
        pEncInfo->alignedHeight = JPU_CEIL(8, pEncInfo->picHeight);
        mcuHeight = 8;
    }

    pEncInfo->mcuWidth  = mcuWidth;
    pEncInfo->mcuHeight = mcuHeight;
    if (format == FORMAT_400) {
        if (pop->rotation == 90 || pop->rotation == 270) {
            pEncInfo->mcuWidth  = mcuHeight;
            pEncInfo->mcuHeight = mcuWidth;
        }
    }

    comp0McuWidth  = pEncInfo->mcuWidth;
    comp0McuHeight = pEncInfo->mcuHeight;
    if (rotation_90_270 == TRUE) {
        if (pEncInfo->format == FORMAT_420 || pEncInfo->format == FORMAT_422) {
            comp0McuWidth = 16;
        }
        else  {
            comp0McuWidth = 8;
        }

        if (pEncInfo->format == FORMAT_420 || pEncInfo->format == FORMAT_440) {
            comp0McuHeight = 16;
        }
        else if (pEncInfo->format == FORMAT_400) {
            comp0McuHeight = 32;
        }
        else {
            comp0McuHeight = 8;
        }
    }

    if (pop->sliceInstMode == TRUE) {
        Uint32 ppuHeight = (rotation_90_270 == TRUE) ? pEncInfo->alignedWidth : pEncInfo->alignedHeight;
        if (pop->sliceHeight % pEncInfo->mcuHeight) {
            JpgLeaveLock();
            return JPG_RET_INVALID_PARAM;
        }

        if (pop->sliceHeight > ppuHeight) {
            JpgLeaveLock();
            return JPG_RET_INVALID_PARAM;
        }

        if (pop->sliceHeight < pEncInfo->mcuHeight) {
            JpgLeaveLock();
            return JPG_RET_INVALID_PARAM;
        }
    }

    pJpgInst->sliceInstMode = pop->sliceInstMode;
    pEncInfo->rstIntval     = pop->restartInterval;
    pEncInfo->jpg12bit      = pop->jpg12bit;
    pEncInfo->q_prec0       = pop->q_prec0;
    pEncInfo->q_prec1       = pop->q_prec1;
    pEncInfo->pixelJustification = pop->pixelJustification;
    if (pop->jpg12bit){
        for (i=0; i<8; i++)
            pEncInfo->pHuffVal[i] = pop->huffVal[i];
        for (i=0; i<8; i++)
            pEncInfo->pHuffBits[i] = pop->huffBits[i];
    }else{
        for (i=0; i<4; i++)
            pEncInfo->pHuffVal[i] = pop->huffVal[i];
        for (i=0; i<4; i++)
            pEncInfo->pHuffBits[i] = pop->huffBits[i];
    }

    for (i=0; i<4; i++)
        pEncInfo->pQMatTab[i] = pop->qMatTab[i];

    pEncInfo->pCInfoTab[0] = sJpuCompInfoTable[2/*pEncInfo->format*/];
    pEncInfo->pCInfoTab[1] = pEncInfo->pCInfoTab[0] + 6;
    pEncInfo->pCInfoTab[2] = pEncInfo->pCInfoTab[1] + 6;
    pEncInfo->pCInfoTab[3] = pEncInfo->pCInfoTab[2] + 6;

    if (pop->packedFormat == PACKED_FORMAT_444 && pEncInfo->format != FORMAT_444) {
        return JPG_RET_INVALID_PARAM;
    }

    pEncInfo->packedFormat = pop->packedFormat;
    if (pEncInfo->format == FORMAT_400) {
        pEncInfo->compInfo[1] = 0;
        pEncInfo->compInfo[2] = 0;
    }
    else {
        pEncInfo->compInfo[1] = 5;
        pEncInfo->compInfo[2] = 5;
    }

    if (pEncInfo->format == FORMAT_400) {
        pEncInfo->compNum = 1;
    }
    else
        pEncInfo->compNum = 3;

    if (pEncInfo->format == FORMAT_420) {
        pEncInfo->mcuBlockNum = 6;
    }
    else if (pEncInfo->format == FORMAT_422) {
        pEncInfo->mcuBlockNum = 4;
    } else if (pEncInfo->format == FORMAT_440) { /* aka YUV440 */
        pEncInfo->mcuBlockNum = 4;
    } else if (pEncInfo->format == FORMAT_444) {
        pEncInfo->mcuBlockNum = 3;
    } else if (pEncInfo->format == FORMAT_400) {
        Uint32 picHeight = (90 == pop->rotation || 270 == pop->rotation) ? pEncInfo->picWidth : pEncInfo->picHeight;
        if (0 < pEncInfo->rstIntval && picHeight == pEncInfo->sliceHeight) {
            pEncInfo->mcuBlockNum = 1;
            comp0McuWidth         = 8;
            comp0McuHeight        = 8;
        }
        else {
            pEncInfo->mcuBlockNum = 4;
        }
    }
    pEncInfo->compInfo[0] = (comp0McuWidth >> 3) << 3 | (comp0McuHeight >> 3);

    pEncInfo->busReqNum = (pop->jpg12bit == FALSE) ? GetEnc8bitBusReqNum(pEncInfo->packedFormat, pEncInfo->format) :
                                                     GetEnc12bitBusReqNum(pEncInfo->packedFormat, pEncInfo->format);


    pEncInfo->tiledModeEnable = pop->tiledModeEnable;

    pEncInfo->encIdx = 0;
    pEncInfo->encSlicePosY = 0;
    pEncInfo->rotationIndex = pop->rotation/90;
    pEncInfo->mirrorIndex   = pop->mirror;
    JpgLeaveLock();

    return JPG_RET_SUCCESS;

}

JpgRet JPU_EncClose(JpgEncHandle handle)
{
    JpgInst * pJpgInst;
    JpgRet ret;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    JpgEnterLock();

    pJpgInst = handle;
    if (GetJpgPendingInstEx(pJpgInst->instIndex)) {
        JpgLeaveLock();
        return JPG_RET_FRAME_NOT_COMPLETE;
    }

    FreeJpgInstance(pJpgInst);

    JpgLeaveLock();

    return JPG_RET_SUCCESS;
}



JpgRet JPU_EncGetBitstreamBuffer( JpgEncHandle handle,
    PhysicalAddress * prdPtr,
    PhysicalAddress * pwrPtr,
    int * size)
{
    JpgInst * pJpgInst;
    JpgRet ret;
    Int32 instRegIndex;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    if ( prdPtr == 0 || pwrPtr == 0 || size == 0) {
        return JPG_RET_INVALID_PARAM;
    }

    pJpgInst = handle;

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }


    *pwrPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG);
    *prdPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_RD_PTR_REG);

    *size = *pwrPtr - *prdPtr;

    return JPG_RET_SUCCESS;

}

JpgRet JPU_EncUpdateBitstreamBuffer(
    JpgEncHandle handle,
    int size)
{
    JpgInst * pJpgInst;
    JpgEncInfo * pEncInfo;
    JpgRet ret;
    Int32 instRegIndex;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;

    pJpgInst = handle;

    pEncInfo = &pJpgInst->JpgInfo->encInfo;
    if (GetJpgPendingInstEx(pJpgInst->instIndex) == pJpgInst) {
        if (pJpgInst->sliceInstMode == TRUE) {
            instRegIndex = pJpgInst->instIndex;
        }
        else {
            instRegIndex = 0;
        }

        pEncInfo->streamWrPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG);
        pEncInfo->streamRdPtr += size;
        if ((pEncInfo->streamWrPtr >= pEncInfo->streamBufEndAddr) || (size == 0)) {	//Full Interrupt case. wrap to the start address
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_CUR_POS_REG, 0);
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_BAS_ADDR_REG, pEncInfo->streamBufStartAddr);
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_EXT_ADDR_REG, pEncInfo->streamBufStartAddr);
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_END_ADDR_REG, pEncInfo->streamBufEndAddr);
            pEncInfo->streamRdPtr = pEncInfo->streamBufStartAddr;
            pEncInfo->streamWrPtr = pEncInfo->streamBufStartAddr;
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_RD_PTR_REG, pEncInfo->streamRdPtr);
            JpuWriteInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG, pEncInfo->streamWrPtr);
        }
    }
    else {
        pEncInfo->streamRdPtr = pEncInfo->streamBufStartAddr;
        pEncInfo->streamWrPtr = pEncInfo->streamBufStartAddr;
    }

    return JPG_RET_SUCCESS;
}


JpgRet JPU_EncStartOneFrame(JpgEncHandle handle, JpgEncParam * param)
{
    JpgInst * pJpgInst;
    JpgEncInfo * pEncInfo;
    FrameBuffer * pBasFrame;
    JpgRet ret;
    Uint32 val;
    Int32 instRegIndex;
    BOOL bTableInfoUpdate;
    Uint32  rotMirEnable = 0;
    Uint32  rotMirMode   = 0;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;


    pJpgInst = handle;
    pEncInfo = &pJpgInst->JpgInfo->encInfo;


    ret = CheckJpgEncParam(handle, param);
    if (ret != JPG_RET_SUCCESS) {
        return ret;
    }

    pBasFrame = param->sourceFrame;

    JpgEnterLock();

    if (GetJpgPendingInstEx(pJpgInst->instIndex) == pJpgInst) {
        JpgLeaveLock();
        return JPG_RET_FRAME_NOT_COMPLETE;
    }

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    JpuWriteInstReg(instRegIndex, MJPEG_INTR_MASK_REG, ((~pEncInfo->intrEnableBit) & 0x3ff));
    JpuWriteInstReg(instRegIndex, MJPEG_SLICE_INFO_REG, pEncInfo->sliceHeight);
    JpuWriteInstReg(instRegIndex, MJPEG_SLICE_DPB_POS_REG, pEncInfo->picHeight); // assume that the all of source buffer is available
    JpuWriteInstReg(instRegIndex, MJPEG_SLICE_POS_REG, pEncInfo->encSlicePosY);
    val = (0 << 16) | (pEncInfo->encSlicePosY / pEncInfo->mcuHeight);
    JpuWriteInstReg(instRegIndex, MJPEG_PIC_SETMB_REG,val);

    JpuWriteInstReg(instRegIndex, MJPEG_CLP_INFO_REG, 0);	//off ROI enable due to not supported feature for encoder.

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_BAS_ADDR_REG, pEncInfo->streamBufStartAddr);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_END_ADDR_REG, pEncInfo->streamBufEndAddr);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG, pEncInfo->streamWrPtr);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_RD_PTR_REG, pEncInfo->streamRdPtr);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_CUR_POS_REG, 0);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_DATA_CNT_REG, JPU_GBU_SIZE / 4);	// 64 * 4 byte == 32 * 8 byte
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_EXT_ADDR_REG, pEncInfo->streamBufStartAddr);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_INT_ADDR_REG, 0);

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_BAS_ADDR_REG, pEncInfo->streamWrPtr);
    JpuWriteInstReg(instRegIndex, MJPEG_BBC_EXT_ADDR_REG, pEncInfo->streamWrPtr);

    JpuWriteInstReg(instRegIndex, MJPEG_GBU_BPTR_REG, 0);
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_WPTR_REG, 0);

    JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBSR_REG, 0);
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_CTRL_REG, 0);

    JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBER_REG, ((JPU_GBU_SIZE / 4) * 2) - 1);
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBIR_REG, JPU_GBU_SIZE / 4);	// 64 * 4 byte == 32 * 8 byte
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_BBHR_REG, JPU_GBU_SIZE / 4);	// 64 * 4 byte == 32 * 8 byte

#define DEFAULT_TDI_TAI_DATA 0x055
    JpuWriteInstReg(instRegIndex, MJPEG_PIC_CTRL_REG, (pEncInfo->jpg12bit<<31) | (pEncInfo->q_prec0<<30) | (pEncInfo->q_prec1<<29) | (pEncInfo->tiledModeEnable<<19) |
                                                      (DEFAULT_TDI_TAI_DATA<<7) | 0x18 | (1<<6) | (JPU_CHECK_WRITE_RESPONSE_BVALID_SIGNAL<<2));
    JpuWriteInstReg(instRegIndex, MJPEG_SCL_INFO_REG, 0);

    val = 0;
    //PackMode[3:0]: 0(NONE), 8(PACK444), 4,5,6,7(YUYV => VYUY)
    val = (pEncInfo->frameEndian << 6) | ((pEncInfo->chromaInterleave==0)?0:(pEncInfo->chromaInterleave==1)?2:3);
    if (pEncInfo->packedFormat == PACKED_FORMAT_NONE) {
        val |= (0<<5) | (0<<4) | (0<<2);
    }
    else if (pEncInfo->packedFormat == PACKED_FORMAT_444) {
        val |= (1<<5) | (0<<4) | (0<<2);
    }
    else {
        val |= (0<<5) | (1<<4) | ((pEncInfo->packedFormat-1)<<2);
    }
    val |= (pEncInfo->pixelJustification << 11);

    JpuWriteInstReg(instRegIndex, MJPEG_DPB_CONFIG_REG, val);

    JpuWriteInstReg(instRegIndex, MJPEG_RST_INTVAL_REG, pEncInfo->rstIntval);
    if (pEncInfo->encSlicePosY == 0) {
        JpuWriteInstReg(instRegIndex, MJPEG_RST_INDEX_REG, 0);// RST index from 0.
        JpuWriteInstReg(instRegIndex, MJPEG_BBC_STRM_CTRL_REG, 0);// clear BBC ctrl status.
    }

    JpuWriteInstReg(instRegIndex, MJPEG_BBC_CTRL_REG, (pEncInfo->streamEndian << 1) | 1);
    JpuWriteInstReg(instRegIndex, MJPEG_OP_INFO_REG, pEncInfo->busReqNum);


    bTableInfoUpdate = FALSE;
    if (pEncInfo->encSlicePosY == 0) {
        bTableInfoUpdate = TRUE; // if sliceMode is disabled. HW process frame by frame between instances. so Huff table can be updated between instances. instances can have the different HuffTable.
    }

    if (bTableInfoUpdate == TRUE) {
        // Load HUFFTab
        if (pEncInfo->jpg12bit){
            if (!JpgEncLoadHuffTab_12b(pJpgInst, instRegIndex)) {
                JpgLeaveLock();
                return JPG_RET_INVALID_PARAM;
            }
        }else{
            if (!JpgEncLoadHuffTab(pJpgInst, instRegIndex)) {
                JpgLeaveLock();
                return JPG_RET_INVALID_PARAM;
            }
        }
    }

    bTableInfoUpdate = FALSE;
    if (pEncInfo->encSlicePosY == 0) {
        bTableInfoUpdate = TRUE;
    }

    if (bTableInfoUpdate == TRUE) {
        // Load QMATTab
        if (!JpgEncLoadQMatTab(pJpgInst, instRegIndex)) {
            JpgLeaveLock();
            return JPG_RET_INVALID_PARAM;
        }
    }



    JpuWriteInstReg(instRegIndex, MJPEG_PIC_SIZE_REG, pEncInfo->alignedWidth<<16 | pEncInfo->alignedHeight);

    if (pEncInfo->rotationIndex || pEncInfo->mirrorIndex) {
        rotMirEnable = 0x10;
        rotMirMode   = (pEncInfo->mirrorIndex << 2) | pEncInfo->rotationIndex;
    }
    JpuWriteInstReg(instRegIndex, MJPEG_ROT_INFO_REG, rotMirEnable | rotMirMode);

    JpuWriteInstReg(instRegIndex, MJPEG_MCU_INFO_REG, (pEncInfo->mcuBlockNum&0x0f) << 17 | (pEncInfo->compNum&0x07) << 14    |
                                                      (pEncInfo->compInfo[0]&0x3f) << 8  | (pEncInfo->compInfo[1]&0x0f) << 4 |
                                                      (pEncInfo->compInfo[2]&0x0f));

    //JpgEncGbuResetReg
    JpuWriteInstReg(instRegIndex, MJPEG_GBU_CTRL_REG, pEncInfo->stuffByteEnable<<3);     // stuffing "FF" data where frame end

    JpuWriteInstReg(instRegIndex, MJPEG_DPB_BASE00_REG,  pBasFrame->bufY);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_BASE01_REG,  pBasFrame->bufCb);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_BASE02_REG,  pBasFrame->bufCr);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_YSTRIDE_REG, pBasFrame->stride);
    JpuWriteInstReg(instRegIndex, MJPEG_DPB_CSTRIDE_REG, pBasFrame->strideC);


    if (pJpgInst->loggingEnable)
        jdi_log(JDI_LOG_CMD_PICRUN, 1, instRegIndex);


    JpuWriteInstReg(instRegIndex, MJPEG_PIC_START_REG, (1<<JPG_ENABLE_START_PIC));

    pEncInfo->encIdx++;


    SetJpgPendingInstEx(pJpgInst, pJpgInst->instIndex);
    if (pJpgInst->sliceInstMode == TRUE) {
        JpgLeaveLock();
    }
    return JPG_RET_SUCCESS;
}


JpgRet JPU_EncGetOutputInfo(
    JpgEncHandle handle,
    JpgEncOutputInfo * info
    )
{
    JpgInst * pJpgInst;
    JpgEncInfo * pEncInfo;
    Uint32 val;
    Uint32 intReason;
    JpgRet ret;
    Int32 instRegIndex;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS) {
        return ret;
    }

    if (info == 0) {
        return JPG_RET_INVALID_PARAM;
    }

    pJpgInst = handle;
    pEncInfo = &pJpgInst->JpgInfo->encInfo;


    if (pJpgInst->sliceInstMode == TRUE) {
        JpgEnterLock();
    }
    if (pJpgInst != GetJpgPendingInstEx(pJpgInst->instIndex)) {
        JpgLeaveLock();
        return JPG_RET_WRONG_CALL_SEQUENCE;
    }

    if (pJpgInst->sliceInstMode == TRUE) {
        instRegIndex = pJpgInst->instIndex;
    }
    else {
        instRegIndex = 0;
    }

    info->frameCycle = JpuReadInstReg(instRegIndex, MJPEG_CYCLE_INFO_REG);
    intReason = JpuReadInstReg(instRegIndex, MJPEG_PIC_STATUS_REG);

    if ((intReason & 0x4) >> 2) {
        JpgLeaveLock();
        return JPG_RET_WRONG_CALL_SEQUENCE;
    }

    info->encodedSliceYPos = JpuReadInstReg(instRegIndex, MJPEG_SLICE_POS_REG);
    pEncInfo->encSlicePosY = info->encodedSliceYPos;
    if (intReason & (1<<INT_JPU_DONE))
        pEncInfo->encSlicePosY = 0;

    pEncInfo->streamWrPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_WR_PTR_REG);
    pEncInfo->streamRdPtr = JpuReadInstReg(instRegIndex, MJPEG_BBC_RD_PTR_REG);
    info->bitstreamBuffer = pEncInfo->streamRdPtr;
    info->bitstreamSize = pEncInfo->streamWrPtr - pEncInfo->streamRdPtr;
    info->streamWrPtr = pEncInfo->streamWrPtr;
    info->streamRdPtr = pEncInfo->streamRdPtr;

    if (intReason != 0) {
        JpuWriteInstReg(instRegIndex, MJPEG_PIC_STATUS_REG, intReason);

        if (intReason & (1<<INT_JPU_SLICE_DONE))
            info->encodeState = ENCODE_STATE_SLICE_DONE;
        if (intReason & (1<<INT_JPU_DONE))
            info->encodeState = ENCODE_STATE_FRAME_DONE;
    }
	
#ifndef JPU_IRQ_CONTROL
    info->encodeState = ENCODE_STATE_FRAME_DONE;
#endif

    if (pJpgInst->loggingEnable)
        jdi_log(JDI_LOG_CMD_PICRUN, 0, instRegIndex);

    JpuWriteInstReg(instRegIndex, MJPEG_PIC_START_REG, 0);

    val = JpuReadReg(MJPEG_INST_CTRL_STATUS_REG);
    val &= ~(1UL<<instRegIndex);
    JpuWriteReg(MJPEG_INST_CTRL_STATUS_REG, val);

    SetJpgPendingInstEx(0, pJpgInst->instIndex);

    JpgLeaveLock();
    return JPG_RET_SUCCESS;
}


JpgRet JPU_EncGiveCommand(
    JpgEncHandle handle,
    JpgCommand cmd,
    void * param)
{
    JpgInst * pJpgInst;
    JpgEncInfo * pEncInfo;
    JpgRet ret;

    ret = CheckJpgInstValidity(handle);
    if (ret != JPG_RET_SUCCESS)
        return ret;


    pJpgInst = handle;
    pEncInfo = &pJpgInst->JpgInfo->encInfo;
    switch (cmd)
    {
    case ENC_JPG_GET_HEADER:
        {
            if (param == 0) {
                return JPG_RET_INVALID_PARAM;
            }

            if (!JpgEncEncodeHeader(handle, param)) {
                return JPG_RET_INVALID_PARAM;
            }
            break;
        }
    case SET_JPG_USE_STUFFING_BYTE_FF:
        {
            int enable;
            enable = *(int *)param;
            pEncInfo->stuffByteEnable = enable;
            break;
        }
    case SET_JPG_QUALITY_FACTOR:
        {
            Uint32 encQualityPercentage;
            encQualityPercentage = *(Uint32*)param;
            JpgEncSetQualityFactor(handle, encQualityPercentage, TRUE);
            break;
        }
    case ENABLE_LOGGING:
        {
            pJpgInst->loggingEnable = 1;
        }
        break;
    case DISABLE_LOGGING:
        {
            pJpgInst->loggingEnable = 0;
        }
        break;

    default:
        return JPG_RET_INVALID_COMMAND;
    }
    return JPG_RET_SUCCESS;
}
