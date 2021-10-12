//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
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

#include <stdio.h>

#include "main_helper.h"


/* make container */
#if 0//def SUPPORT_FFMPEG_DEMUX
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

/* add a video output stream */
static AVStream *add_stream(AVFormatContext *oc, int codec_id, EncConfigParam encConfig, int gopSize)
{
    AVCodecContext *c;
    AVStream *st;

    st = avformat_new_stream(oc, 0);
    if (!st) {
        VLOG(ERR, "Could not alloc stream\n");
        return 0;
    }
    st->id = oc->nb_streams-1;
    c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_VIDEO;

    /* put sample parameters */
    c->bit_rate = encConfig.kbps * 1000;
    /* resolution must be a multiple of two */
    c->width = (encConfig.rotAngle==90||encConfig.rotAngle ==270)?encConfig.picHeight:encConfig.picWidth;
    c->height = (encConfig.rotAngle==90||encConfig.rotAngle ==270)?encConfig.picWidth:encConfig.picHeight;

    /* time base: this is the fundamental unit of time (in seconds) in terms
    of which frame timestamps are represented. for fixed-fps content,
    timebase should be 1/framerate and timestamp increments should be
    identically 1. */
    c->time_base.den = encConfig.container_frame_rate;
    c->time_base.num = 1;

    c->gop_size = gopSize; /* emit one intra frame every (gop_size)frames at most */

    c->pix_fmt = PIX_FMT_YUV420P;
    if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO){
        /* Needed to avoid using macroblocks in which some coeffs overflow.
        This does not happen with normal video, it just happens here as
        the motion of the chroma plane does not match the luma plane. */
        c->mb_decision=2;
    }
    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

static int write_video_frame(AVFormatContext *oc, AVStream *st, unsigned char *addr, int size, int picType)
{
    int ret;
    static struct SwsContext *img_convert_ctx;

    /* if zero size, it means the image was buffered */
    if (size > 0) {
        AVPacket pkt;
        av_init_packet(&pkt);

        if (picType == 0)   //I Frame
            pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index= st->index;
        pkt.data= addr;
        pkt.size= size;

        /* write the compressed frame in the media file */
        ret = av_interleaved_write_frame(oc, &pkt);
    } 
    else
        ret = 0;

    if (ret != 0) {
        VLOG(ERR, "Error while writing video frame\n");
        return 0;
    }

    return 1;
}

/*
* make a file and write a container's header part
* must set -> en_container, container_frame_rate, gopSize, kbps, rotAngle, picHeight, picWidth,
*             stdMode and bitstreamFileName in struct EncConfigParam
*/
int container_init(EncConfigParam encConfig,
    AVOutputFormat **fmt,
    AVFormatContext **oc,
    AVStream **video_st, 
    int gopSize)
{
    int stdMode = encConfig.stdMode;
    char *filename = encConfig.bitstreamFileName;
    unsigned int i;

    if (encConfig.ringBufferEnable == 1)
    {
        VLOG(ERR, "not supported format\n");
        return 0;
    }

    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();

    /* allocate the output media context */
	if (avformat_open_input(oc, filename, NULL, NULL) < 0) {
        fprintf(stderr, "failed to avformat_open_input\n");
		return 0;
    }

    *fmt = (*oc)->oformat;

    if (stdMode == 0)
        (*fmt)->video_codec = AV_CODEC_ID_MPEG4;
    else if (stdMode == 1)
        (*fmt)->video_codec = AV_CODEC_ID_H263;
    else if (stdMode == 2)
        (*fmt)->video_codec = AV_CODEC_ID_H264;
    else
        VLOG(ERR, "unknown codec type = 0x%x\n", stdMode);

    (*oc)->video_codec_id = (*fmt)->video_codec;
    (*oc)->audio_codec_id = (*fmt)->audio_codec;

    /* add the audio and video streams using the default format codecs
    and initialize the codecs */
    *video_st = add_stream(*oc, (int)(*fmt)->video_codec, encConfig, gopSize);

    av_dump_format(*oc, 0, filename, 1);

    /* open the output file, if needed */
    if (avio_open(&(*oc)->pb, filename, AVIO_FLAG_WRITE) < 0) {
        VLOG(ERR, "Could not open '%s'\n", filename);
        for(i=0; i<(*oc)->nb_streams; i++) {
            av_freep(&(*oc)->streams[i]->codec);
            av_freep(&(*oc)->streams[i]);
        }
        /* free the stream */
        av_free(*oc);
        return 0;
    }

    /* write the stream header, if any */
    if (avformat_write_header(*oc, NULL) < 0) {
        VLOG(ERR, "Could not write header for output file (incorrect codec paramters ?)\n");
        /* free the streams */
        for(i=0; i<(*oc)->nb_streams; i++) {
            av_freep(&(*oc)->streams[i]->codec);
            av_freep(&(*oc)->streams[i]);
        }

        /* close the output file */
        avio_close((*oc)->pb);

        /* free the stream */
        av_free(*oc);
        *oc = NULL;
        return 0;
    }

    return 1;
}

/*
* save the header data to temp_buf(header_buf). (instead of ReadBsResetBufHelper)
*/
int container_copy_header_from_bitstream_buffer(const Uint32 core_idx,
    const PhysicalAddress bitstream,
    const int size,
    const int endian,
    char *buf,
    int *pos)
{
    vdi_read_memory(core_idx, bitstream, (unsigned char *)(buf+(*pos)), size, endian);
    *pos += size;

    //to check memory boundary
    if (*pos >= CONTAINER_HEADER_SIZE)
    {
        VLOG(ERR, "exceed header buffer, header buffer size[%d]\n", *pos);
        return 0;
    }
    return 1;
}

/*
* write the Element stream data with syntax header to the container file
*/
int container_write_es(const Uint32 core_idx,
    const PhysicalAddress bitstream,
    const int size,
    const int endian,
    AVFormatContext *oc, 
    AVStream *st,
    char *header_buffer,
    int *header_pos,
    int format, 
    int picType)
{
    unsigned char *buf = osal_malloc(size);

    if (!buf)
    {
        VLOG(ERR, "fail to allocate bitstream buffer\n");
        return 0;
    }
    vdi_read_memory(core_idx, bitstream, buf, size, endian);

    //video data = write only 1 SPS & PPS data in the file, 
    //JPEG = copy jpeg header to every chunk in the file
    if (*header_pos)//I Frame || MJPEG
    {
        unsigned char *buf2 = osal_malloc(size + *header_pos);

        osal_memcpy(buf2, header_buffer, *header_pos);
        osal_memcpy(buf2 + *header_pos, buf, size);

        write_video_frame(oc, st, buf2,size + *header_pos, picType);

        osal_free(buf2);

        //to write a header to the first chunk just one time in container except JPEG
        if ( format != 8 ) //(format != MJPEG)
            *header_pos = 0;
    }
    else
        write_video_frame(oc, st, buf, size, picType);

    osal_free(buf);

    return 1;
}

int container_deinit(AVFormatContext *oc)
{
    unsigned int i;

    if (!oc || !oc->pb)
        return 0;

    /* write the trailer, if any.  the trailer must be written
    * before you close the CodecContexts open when you wrote the
    * header; otherwise write_trailer may try to use memory that
    * was freed on av_codec_close() */
    av_write_trailer(oc);

    /* free the streams */
    for(i=0; i<oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }

    /* close the output file */
    avio_close(oc->pb);

    /* free the stream */
    av_free(oc);

    return 1;
}

#endif /* SUPPORT_FFMPEG_DEMUX */

 
