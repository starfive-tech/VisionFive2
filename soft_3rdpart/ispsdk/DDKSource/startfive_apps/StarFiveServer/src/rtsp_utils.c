/*************************************************************************
	> File Name: rtsp_utils.c
	> Author: bxq
	> Mail: 544177215@qq.com
	> Created Time: Sunday, May 22, 2016 PM09:35:22 CST
 ************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "comm.h"
#include "main.h"
#include "rtsp_utils.h"

#define __UINT_MAX (~0lu)
#define __BASE64_SIZE(x) (((x) + 2) / 3 * 4 + 1)
#define __RB32(x) (((uint32_t)((const uint8_t *)(x))[0] << 24) |\
                             (((const uint8_t *)(x))[1] << 16) |\
                             (((const uint8_t *)(x))[2] << 8) |\
                              ((const uint8_t *)(x))[3])
 /*****************************************************************************
 * b64_encode: Stolen from VLC's http.c.
 * Simplified by Michael.
 * Fixed edge cases and made it work from data (vs. strings) by Ryan.
 *****************************************************************************/
char *base64_encode(char *out, int out_size, const uint8_t *in, int in_size)
{
	static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char *ret, *dst;
	unsigned i_bits = 0;
	int i_shift = 0;
	int bytes_remaining = in_size;

	if (in_size >= __UINT_MAX / 4 || out_size < __BASE64_SIZE(in_size))
		return NULL;
	ret = dst = out;
	while (bytes_remaining > 3)
	{
		i_bits = __RB32(in);
		in += 3;
		bytes_remaining -= 3;
		*dst++ = b64[i_bits >> 26];
		*dst++ = b64[(i_bits >> 20) & 0x3F];
		*dst++ = b64[(i_bits >> 14) & 0x3F];
		*dst++ = b64[(i_bits >> 8) & 0x3F];
	}
	i_bits = 0;
	while (bytes_remaining)
	{
		i_bits = (i_bits << 8) + *in++;
		bytes_remaining--;
		i_shift += 8;
	}
	while (i_shift > 0)
	{
		*dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
		i_shift -= 6;
	}
	while ((dst - ret) & 3)
		*dst++ = '=';
	*dst = '\0';

	return ret;
}
