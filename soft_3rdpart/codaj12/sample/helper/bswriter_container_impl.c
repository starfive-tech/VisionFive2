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

#include "main_helper.h"
#include "libavformat/avformat.h"

static AVStream* add_stream(AVFormatContext *oc, EncConfigParam* encConfig)
{
    AVCodecContext* c;
    AVStream*       st;
    Uint32          frameRate = encConfig->mjpgFramerate;

    if (frameRate == 0) frameRate = 30;

    st = avformat_new_stream(oc, 0);
    if (!st) {
        JLOG(ERR, "<%s:%d> Could not alloc stream\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    st->id = oc->nb_streams-1;
    c = st->codec;
    c->codec_id   = AV_CODEC_ID_MJPEG;
    c->codec_type = AVMEDIA_TYPE_VIDEO;

    /* put sample parameters */
    //c->bit_rate = 1000;
    /* resolution must be a multiple of two */
    c->width  = encConfig->picWidth;
    c->height = encConfig->picHeight;
    if (encConfig->rotation == 90 || encConfig->rotation == 270) {
        c->width  = encConfig->picHeight;
        c->height = encConfig->picWidth;
    }

    /* time base: this is the fundamental unit of time (in seconds) in terms
    of which frame timestamps are represented. for fixed-fps content,
    timebase should be 1/framerate and timestamp increments should be
    identically 1. */
    c->time_base.den = frameRate;
    c->time_base.num = 1;

    //c->gop_size = 1; /* emit one intra frame every (gop_size)frames at most */

    c->pix_fmt = PIX_FMT_YUV420P;
    
    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

static int write_video_frame(AVFormatContext* oc, AVStream* st, Uint8* addr, Uint32 size, Int64 pts)
{
    int ret;
    
    /* if zero size, it means the image was buffered */
    if (size > 0) {
        AVPacket pkt;
        av_init_packet(&pkt);

        pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index= st->index;
        pkt.data= addr;
        pkt.size= size;
        //pkt.pts = pts;

        /* write the compressed frame in the media file */
        ret = av_interleaved_write_frame(oc, &pkt);
    } 
    else
        ret = 0;

    if (ret != 0) {
        JLOG(ERR, "<%s:%d> Error while writing video frame(ret: %x)\n", __FUNCTION__, __LINE__, ret);
        return 0;
    }

    return 1;
}

/*
* make a file and write a container's header part
* must set -> en_container, container_frame_rate, gopSize, kbps, rotAngle, picHeight, picWidth,
*             stdMode and bitstreamFileName in struct EncConfigParam
*/
static int container_init(EncConfigParam* encConfig, AVOutputFormat **fmt, AVFormatContext **oc, AVStream **video_st, const char* filename)
{
    Uint32  i;

    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();

    /* allocate the output media context */
    avformat_alloc_output_context2(&(*oc), NULL, NULL, filename);
    if (!*oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&(*oc), NULL, "mpeg", filename);
    }
    if (!*oc) {
        return 0;
    }
    *fmt = (*oc)->oformat;
    (*fmt)->video_codec   = CODEC_ID_MJPEG;
    (*oc)->video_codec_id = (*fmt)->video_codec;
    (*oc)->audio_codec_id = CODEC_ID_NONE;

    /* add the audio and video streams using the default format codecs
    and initialize the codecs */
    *video_st = add_stream(*oc, encConfig);

    av_dump_format(*oc, 0, filename, 1);

    /* open the output file, if needed */
    if (avio_open(&(*oc)->pb, filename, AVIO_FLAG_WRITE) < 0) {
        JLOG(ERR, "<%s:%d> Could not open '%s'\n", __FUNCTION__, __LINE__, filename);
        for(i=0; i<(*oc)->nb_streams; i++) {
            av_freep(&(*oc)->streams[i]->codec);
            av_freep(&(*oc)->streams[i]);
        }
        /* osal_free the stream */
        av_free(*oc);
        return 0;
    }

    /* write the stream header, if any */
    if (avformat_write_header(*oc, NULL) < 0) {
        JLOG(ERR, "Could not write header for output file (incorrect codec paramters ?)\n");
        /* osal_free the streams */
        for(i=0; i<(*oc)->nb_streams; i++) {
            av_freep(&(*oc)->streams[i]->codec);
            av_freep(&(*oc)->streams[i]);
        }

        /* close the output file */
        avio_close((*oc)->pb);

        /* osal_free the stream */
        av_free(*oc);
        *oc = NULL;
        return 0;
    }

    return 1;
}

static int container_deinit(AVFormatContext *oc)
{
    unsigned int i;

    if (!oc || !oc->pb)
        return 0;

    /* write the trailer, if any.  the trailer must be written
    * before you close the CodecContexts open when you wrote the
    * header; otherwise write_trailer may try to use memory that
    * was freed on av_codec_close() */
    av_write_trailer(oc);

    /* osal_free the streams */
    for(i=0; i<oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }

    /* close the output file */
    avio_close(oc->pb);

    /* osal_free the stream */
    av_free(oc);

    return 1;
}

/* --------------------------------------------------------------------------
 * ES writer 
  --------------------------------------------------------------------------- */
typedef struct ContainerWriterContext {
    AVOutputFormat*     fmt;
    AVFormatContext*    oc;
    AVStream*           video_st;
    Int64               pts;
} ContainerWriterContext;

static BOOL BSWriterContainer_Create(BitstreamWriterImpl* impl, EncConfigParam* encConfig, const char* path)
{
    ContainerWriterContext* ctx = NULL;

    if (NULL == (ctx=(ContainerWriterContext*)malloc(sizeof(ContainerWriterContext)))) {
        return FALSE;
    }
    
    impl->context = (void*)ctx;

    if (FALSE == container_init(encConfig, &ctx->fmt, &ctx->oc, &ctx->video_st, path)) {
        return FALSE;
    }

    return TRUE;
}

static Uint32 BSWriterContainer_Act(BitstreamWriterImpl* impl, Uint8* data, Uint32 size)
{
    ContainerWriterContext* ctx = (ContainerWriterContext*)impl->context;
    Int64                   pts;

    pts = ctx->pts++;

    if (write_video_frame(ctx->oc, ctx->video_st, data, size, pts) == 0) {
        return 0;
    }
    else {
        return size;
    }
}

static BOOL BSWriterContainer_Destroy(BitstreamWriterImpl* impl)
{
    ContainerWriterContext* ctx = (ContainerWriterContext*)impl->context;

    container_deinit(ctx->oc);
    
    free(ctx);

    return TRUE;
}

BitstreamWriterImpl containerWriter = {
    NULL,
    BSWriterContainer_Create,
    BSWriterContainer_Act,
    BSWriterContainer_Destroy
};

