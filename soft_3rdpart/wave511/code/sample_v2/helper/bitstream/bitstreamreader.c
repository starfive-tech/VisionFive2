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

#include "main_helper.h"


typedef struct {
    Uint32                  type;
    EndianMode              endian;
    BitstreamReaderImpl*    impl;
    osal_file_t*            fp;
    EncHandle*              handle;
} AbstractBitstreamReader;

BitstreamReader BitstreamReader_Create(
    Uint32      type,
    char*       path,
    EndianMode  endian,
    EncHandle*  handle
    )
{
    AbstractBitstreamReader* reader;
    osal_file_t *fp=NULL;

    if ( path[0] != 0) {
        if ((fp=osal_fopen(path, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open bin file: %s\n", __FUNCTION__, __LINE__, path);
            return FALSE;
        }
        VLOG(INFO, "output bin file: %s\n", path);
    }
    else
        VLOG(ERR, "%s:%d Bitstream File path is NULL : no save\n", __FUNCTION__, __LINE__);

    reader = (AbstractBitstreamReader*)osal_malloc(sizeof(AbstractBitstreamReader));

    reader->fp      = fp;
    reader->handle  = handle;
    reader->type    = type;
    reader->endian  = endian;

    return reader;
}

BOOL BitstreamReader_Act(
    BitstreamReader reader,
    PhysicalAddress bitstreamBuffer,
    Uint32          bitstreamBufferSize,
    Uint32          streamReadSize,
    Comparator      comparator
    )
{
    AbstractBitstreamReader* absReader = (AbstractBitstreamReader*)reader;
    osal_file_t     *fp;
    EncHandle       *handle;
    RetCode         ret = RETCODE_SUCCESS;
    PhysicalAddress paRdPtr;
    PhysicalAddress paWrPtr;
    int             size = 0;
    Int32           loadSize = 0;
    PhysicalAddress paBsBufStart = bitstreamBuffer;
    PhysicalAddress paBsBufEnd   = bitstreamBuffer+bitstreamBufferSize;
    Uint8*          buf          = NULL;
    Uint32          coreIdx;
    BOOL            success      = TRUE;

    if (reader == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if (streamReadSize == 0) {
        return TRUE;
    }
    fp = absReader->fp;
    handle = absReader->handle;
    coreIdx = VPU_HANDLE_CORE_INDEX(*handle);
    
    ret = VPU_EncGetBitstreamBuffer(*handle, &paRdPtr, &paWrPtr, &size);
    if (size > 0) {
        if (streamReadSize > 0) {
            if ((Uint32)size < streamReadSize) {
                loadSize = size;
            } 
            else {
                loadSize = streamReadSize;
            }
        }
        else {
            loadSize = size;
        }

        buf = (Uint8*)osal_malloc(loadSize);
        if (buf == NULL) {
            return FALSE;
        }

        if (absReader->type == BUFFER_MODE_TYPE_RINGBUFFER) {
            if ((paRdPtr+loadSize) > paBsBufEnd) {
                Uint32   room = paBsBufEnd - paRdPtr;
                vdi_read_memory(coreIdx, paRdPtr, buf, room,  absReader->endian);
                vdi_read_memory(coreIdx, paBsBufStart, buf+room, (loadSize-room), absReader->endian);
            } 
            else {
                vdi_read_memory(coreIdx, paRdPtr, buf, loadSize, absReader->endian); 
            }
        } 
        else {
            /* Linebuffer */
            vdi_read_memory(coreIdx, paRdPtr, buf, loadSize, absReader->endian); 
        }

        if (fp != NULL) {
            osal_fwrite((void *)buf, sizeof(Uint8), loadSize, fp);
            osal_fflush(fp);
        }

        if (comparator != NULL) {
            if (Comparator_Act(comparator, buf, loadSize) == FALSE) {
                success = FALSE;
            }
        }

        osal_free(buf);

        ret = VPU_EncUpdateBitstreamBuffer(*handle, loadSize);
        if( ret != RETCODE_SUCCESS ) {
            VLOG(ERR, "VPU_EncUpdateBitstreamBuffer failed Error code is 0x%x \n", ret );
            success = FALSE;
        }
    }

    return success;
}

BOOL BitstreamReader_Destroy(
    BitstreamReader reader
    )
{
    AbstractBitstreamReader* absReader = (AbstractBitstreamReader*)reader;

    if (absReader == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    osal_fclose(absReader->fp);
    osal_free(absReader);

    return TRUE;
}

