/*************************************************************************
	> File Name: rtsp_main.c
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Saturday, December 12, 2015 PM03:19:12 CST
 ************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "comm.h"
#include "main.h"
#include "rtsp_demo.h"
#include "ispc_main.h"



int rtsp_server_main(void* pparameters)
{
	rtsp_demo_handle demo;
	ISPC_IMAGE_PACK pack;
	ISPC_IMAGE* pimg;
	uint64_t ts = 0;
	uint64_t ps = 1000000 / 25;
	uint32_t kind;
	int ret, ch;
	uint32_t width = 0, height = 0, bit = 0, stride = 0, mosaic = 0;
	void *pvoid = NULL;

	//usleep(200000); //wait for ispc thread run until wake up

	if (g_cfg.rtsp_port == 0)
		g_cfg.rtsp_port = RTSP_SERVER_PORT;

	main_print_app_info(RTSP_SERVER_NAME, RTSP_SERVER_VERSION, g_cfg.rtsp_port);

	demo = rtsp_new_demo(g_cfg.rtsp_port);
	if (NULL == demo)
	{
		printf("rtsp_new_demo failed\n");
		return 0;
	}

	kind = MEM_KIND_UO | MEM_KIND_SS0 | MEM_KIND_SS1 | MEM_KIND_DUMP;
	ispc_main_driver_mem_info_get(kind, &pack);

	//printf("rtsp: session_count=%d\n", g_cfg.session_count);
	for (ch = 0; ch < g_cfg.session_count; ch++)
	{
		if (strlen(g_cfg.session[ch].vszfile))
		{
			g_cfg.session[ch].vcodec = main_parse_filename_info(g_cfg.session[ch].vszfile, &width, &height, &bit, &mosaic);
			//g_cfg.session[ch].vcodec = main_parse_filename_ext(g_cfg.session[ch].vszfile);
			if (g_cfg.session[ch].vcodec >= RTSP_CODEC_ID_VIDEO_UO && g_cfg.session[ch].vcodec <= RTSP_CODEC_ID_VIDEO_DUMP)
			{
				pimg = NULL;
				if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_UO)
					pimg = &pack.img_uo;
				else if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_SS0)
					pimg = &pack.img_ss0;
				else if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_SS1)
					pimg = &pack.img_ss1;
				else if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_DUMP)
					pimg = &pack.img_dump;

				if (pimg)
				{
					//printf("Info w=%d, h=%d, s=%d, b=%d, mo=%d, ms=%d, mp=%p\n", pimg->width, pimg->height, pimg->stride, pimg->bit, pimg->mosaic, pimg->memsize, pimg->pmemaddr);
					if (pimg->width == 0 || pimg->height == 0 || pimg->memsize == 0)
						continue;
					width = pimg->width;
					height = pimg->height;
					bit = pimg->bit;
					stride = pimg->stride;
					mosaic = pimg->mosaic;
					g_cfg.session[ch].vfp = (FILE*)0xFFFF; //Set fake FILE point 
				}
				//printf("rtsp: ch=%d, vfp=%p\n", ch, g_cfg.session[ch].vfp);
			}
			else
			{
				g_cfg.session[ch].vfp = fopen(g_cfg.session[ch].vszfile, "rb");
				if (!g_cfg.session[ch].vfp)
				{
					err("open %s failed\n", g_cfg.session[ch].vszfile);
					goto exit_rtsp;
				}
			}
		}


		if (g_cfg.session[ch].vfp == NULL)
			continue;

		g_cfg.session[ch].handle = rtsp_new_session(demo, g_cfg.session[ch].path);
		if (g_cfg.session[ch].handle == NULL)
		{
			err("rtsp_new_session failed\n");
			continue;
		}

		if (g_cfg.session[ch].vfp)
		{
			rtsp_set_video(g_cfg.session[ch].handle, g_cfg.session[ch].vcodec, NULL, 0, width, height, bit, stride, mosaic);
			rtsp_sync_video_ts(g_cfg.session[ch].handle, rtsp_get_reltime(), rtsp_get_ntptime());
		}

		info("[%d] %s \t %s (%d x %d - %dbit - %d - %d) \n", ch, g_cfg.session[ch].path,
			 (g_cfg.session[ch].vfp) ? g_cfg.session[ch].vszfile : "",
			 width, height, bit, stride, mosaic);
	}

	g_cfg.task_status |= TASK_READY_RTSP;
	ts = rtsp_get_reltime();
	ps = (g_cfg.rtsp_fps == 0) ? 1 : 1000000 / g_cfg.rtsp_fps;
	#ifdef __WIN32__
	info("fps= %d, ts = %llu, ps = %llu\n", g_cfg.rtsp_fps, ts, ps);
	#endif //__WIN32__
	#ifdef __LINUX__
	info("fps= %d, ts = %lu, ps = %lu\n", g_cfg.rtsp_fps, ts, ps);
	#endif //__LINUX__
	
	do
	{
		kind = MEM_KIND_UO | MEM_KIND_SS0 | MEM_KIND_SS1 | MEM_KIND_DUMP;
		pvoid = NULL;
		ispc_main_driver_mem_pop(kind, &pack, &pvoid);

		for (ch = 0; ch < g_cfg.session_count; ch++)
		{
			if (g_cfg.session[ch].vfp) // Video
			{
			read_video_again:
				if (g_cfg.session[ch].vcodec >= RTSP_CODEC_ID_VIDEO_UO && g_cfg.session[ch].vcodec <= RTSP_CODEC_ID_VIDEO_DUMP)
				{
					pimg = NULL;
					if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_UO)
						pimg = &pack.img_uo;
					else if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_SS0)
						pimg = &pack.img_ss0;
					else if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_SS1)
						pimg = &pack.img_ss1;
					else if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_DUMP)
						pimg = &pack.img_dump;
					if (pimg)
					{
						g_cfg.session[ch].vbuf = pimg->pmemaddr;
						g_cfg.session[ch].vbufsize = pimg->memsize;
					}
				}
				else
				{
					ret = rtsp_get_next_video_frame(g_cfg.session[ch].handle, g_cfg.session[ch].vfp, &g_cfg.session[ch].vbuf, &g_cfg.session[ch].vbufsize);
					if (ret < 0)
					{
						err("rtsp_get_next_video_frame() failed\n");
						break;
					}
					if (ret == 0 && g_cfg.session[ch].vfp != (FILE*)0xFFFF)
					{
						fseek(g_cfg.session[ch].vfp, 0, SEEK_SET);
						goto read_video_again;
					}
				}

				if (g_cfg.session[ch].handle)
					rtsp_tx_video(g_cfg.session[ch].handle, g_cfg.session[ch].vbuf, g_cfg.session[ch].vbufsize, ts);

				if (g_cfg.session[ch].vcodec == RTSP_CODEC_ID_VIDEO_H264)
				{
					uint8_t* vbuf = g_cfg.session[ch].vbuf;
					uint8_t type = 0;
					if (vbuf[0] == 0 && vbuf[1] == 0 && vbuf[2] == 1)
						type = vbuf[3] & 0x1f;
					if (vbuf[0] == 0 && vbuf[1] == 0 && vbuf[2] == 0 && vbuf[3] == 1)
						type = vbuf[4] & 0x1f;
					if (type != 5 && type != 1) //NALU_TYPE_IDR(5) || NALU_TYPE_SLICE(1)
						goto read_video_again;
				}
			}
		} //end of for (ch = 0; ch < g_cfg.session_count; ch++)

		if (pvoid) {
			ispc_main_driver_mem_push(pvoid);
			pvoid = NULL;
		}

		do
		{
			ret = rtsp_do_event(demo);
			if (ret > 0)
				continue;
			if (ret < 0)
				goto exit_rtsp;
			if (g_cfg.rtsp_fps == 0)
				break;
			//usleep(1000);
		} while (rtsp_get_reltime() - ts < ps);

		ts += ps;
		//fflush(stdout);
	} while (g_cfg.task_status & TASK_READY_RTSP);

exit_rtsp:
	for (ch = 0; ch < g_cfg.session_count; ch++)
	{
		if (g_cfg.session[ch].vbuf)
			free(g_cfg.session[ch].vbuf);
		if (g_cfg.session[ch].vfp && g_cfg.session[ch].vfp != (FILE*)0xFFFF)
			fclose(g_cfg.session[ch].vfp);
		if (g_cfg.session[ch].handle)
			rtsp_del_session(g_cfg.session[ch].handle);
	}

	rtsp_del_demo(demo);

	main_print_app_end(RTSP_SERVER_NAME);
	return 0;
}







