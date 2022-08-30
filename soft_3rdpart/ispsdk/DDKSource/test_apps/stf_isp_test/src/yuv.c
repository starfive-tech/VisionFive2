/***************************************************************************
 *   Copyright (C) 2012 by Tobias Müller                                   *
 *   Tobias_Mueller@twam.info                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
	Convert from YUV420 format to YUV444.

	\param width width of image
	\param height height of image
	\param src source
	\param dst destination
*/

void YUV420toYUV444(int width, int height, unsigned char* src, unsigned char* dst) {
	int line, column;
	unsigned char *py, *pu, *pv;
	unsigned char *tmp = dst;

	// In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
	// Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels.
	unsigned char *base_py = src;
	unsigned char *base_pu = src+(height*width);
	unsigned char *base_pv = src+(height*width)+(height*width)/4;

	for (line = 0; line < height; ++line) {
		for (column = 0; column < width; ++column) {
			py = base_py+(line*width)+column;
			pu = base_pu+(line/2*width/2)+column/2;
			pv = base_pv+(line/2*width/2)+column/2;

			*tmp++ = *py;
			*tmp++ = *pu;
			*tmp++ = *pv;
		}
	}
}

void YUV420NV21toYUV444(int width, int height, unsigned char* src, unsigned char* dst,
        int is_nv21)
{
	int line, column;
	unsigned char *py, *pu, *pv;
	unsigned char *tmp = dst;

	// In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
	// Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels.
	unsigned char *base_py = src;
	unsigned char *base_pu = src+(height*width);
	unsigned char *base_pv = src+(height*width);

	for (line = 0; line < height; ++line) {
		for (column = 0; column < width; ++column) {
			py = base_py + (line * width) + column;
            if (is_nv21) {
                pu = base_pu + ((line / 2 * width / 2) + column / 2) * 2 + 1;
                pv = base_pv + ((line / 2 * width / 2) + column / 2) * 2;
            } else {
                pu = base_pu + ((line / 2 * width / 2) + column / 2) * 2;
                pv = base_pv + ((line / 2 * width / 2) + column / 2) * 2 + 1;
            }
			*tmp++ = *py;
			*tmp++ = *pu;
			*tmp++ = *pv;
		}
	}
}

void YUV422toYUV444(int width, int height, unsigned char* src, unsigned char* dst) {
	int line, column;
	unsigned char *py, *pu, *pv;
	unsigned char *tmp = dst;

	// In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
	// Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels.
	unsigned char *base_py = src;
	//unsigned char *base_pu = src+(height*width);
	//unsigned char *base_pv = src+(height*width)+(height*width)/2;

	for (line = 0; line < height; ++line) {
		for (column = 0; column < width; ++column) {
			py = base_py+((line*width)+column)*2;
			pu = base_py+((line*width)+column)*2 + 1;
			pv = base_py+((line*width)+column)*2 + 1;

			*tmp++ = *py;
			*tmp++ = *pu;
			*tmp++ = *pv;
		}
	}
}

void RGB565toRGB888(int width, int height, unsigned char* src, unsigned char* dst)
{
	unsigned short *pix = (unsigned short *)src;
	unsigned int i;
	for (i = 0 ; i < width * height; i++)
	{
		*dst++ = (*(pix+i) & 0b1111100000000000) >> 8;
		*dst++ = (*(pix+i) & 0b11111100000) >> 3 ;
		*dst++ = (*(pix+i) & 0b11111) << 3;
	}
}

void RAW12toRAW16(int width, int height, unsigned char* src, unsigned char* dst)
{
	unsigned char *p_src = src;
	unsigned short *p_dst = (unsigned short *)dst;
	unsigned int i, j;
	for (i = 0 ; i < height; i++) {
		p_src = src + (((width * 12 / 8 + 8 * 16 - 1) / (8 * 16)) * 128) * i;
		for (j = 0 ; j < width * 12 / 8; j += 3) {
			*p_dst++ = ((*(p_src + j)) | ((*(p_src + j + 1) & 0xF) << 8)) << 4;
			*p_dst++ = ((*(p_src + j + 2) << 4) | ((*(p_src + j + 1)) >> 4)) << 4;
		}
	}
}
