//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

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
    osal_file_t *fp;

    if ( path == NULL) {
        VLOG(ERR, "%s:%d path is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if ((fp=osal_fopen(path, "wb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open bin file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }
    VLOG(INFO, "output bin file: %s\n", path);

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
#ifdef SUPPORT_DONT_READ_STREAM
        return TRUE;
#else
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
#endif
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
                Uint32   room;
                room = paBsBufEnd - paRdPtr;
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

    if (reader == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    osal_fclose(absReader->fp);
    osal_free(absReader);

    return TRUE;
}

BitstreamReader BufferStreamReader_Create(
    Uint32      type,
    EndianMode  endian,
    EncHandle*  handle
    )
{
    AbstractBitstreamReader* reader;

    reader = (AbstractBitstreamReader*)osal_malloc(sizeof(AbstractBitstreamReader));

    reader->handle  = handle;
    reader->type    = type;
    reader->endian  = endian;

    return reader;
}

Uint32 BufferStreamReader_Act(
    BitstreamReader reader,
    PhysicalAddress bitstreamBuffer,
    Uint32          bitstreamBufferSize,
    Uint32          streamReadSize,
    Uint8*          pBuffer,
    Comparator      comparator
    )
{
    AbstractBitstreamReader* absReader = (AbstractBitstreamReader*)reader;
    EncHandle       *handle;
    RetCode         ret = RETCODE_SUCCESS;
    PhysicalAddress paRdPtr;
    PhysicalAddress paWrPtr;
    int             size = 0;
    Int32           loadSize = 0;
    PhysicalAddress paBsBufStart = bitstreamBuffer;
    PhysicalAddress paBsBufEnd   = bitstreamBuffer+bitstreamBufferSize;
    Uint32          coreIdx;

    if (reader == NULL) {

        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

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

        if (absReader->type == BUFFER_MODE_TYPE_RINGBUFFER) {
            if ((paRdPtr+loadSize) > paBsBufEnd) {
                Uint32   room;
                room = paBsBufEnd - paRdPtr;
                vdi_read_memory(coreIdx, paRdPtr, pBuffer, room,  absReader->endian);
                vdi_read_memory(coreIdx, paBsBufStart, pBuffer+room, (loadSize-room), absReader->endian);
            }
            else {
                vdi_read_memory(coreIdx, paRdPtr, pBuffer, loadSize, absReader->endian);
            }
        }
        else {
            /* Linebuffer */
            vdi_read_memory(coreIdx, paRdPtr, pBuffer, loadSize, absReader->endian);
        }

        ret = VPU_EncUpdateBitstreamBuffer(*handle, loadSize);
        if( ret != RETCODE_SUCCESS ) {
            VLOG(ERR, "VPU_EncUpdateBitstreamBuffer failed Error code is 0x%x \n", ret );
            return 0;
        }
    }

    return loadSize;
}

BOOL BufferStreamReader_Destroy(
    BitstreamReader reader
    )
{
    AbstractBitstreamReader* absReader = (AbstractBitstreamReader*)reader;

    if (reader == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    osal_free(absReader);

    return TRUE;
}