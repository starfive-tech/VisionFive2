/*************************************************************************
	> File Name: rtsp_utils.h
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Sunday, May 22, 2016 PM09:35:22 CST
 ************************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	char* base64_encode(char* out, int out_size, const uint8_t* in, int in_size);

#ifdef __cplusplus
}
#endif
#endif
