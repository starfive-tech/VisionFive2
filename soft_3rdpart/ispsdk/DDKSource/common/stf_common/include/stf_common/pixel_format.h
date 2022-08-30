/**
 ******************************************************************************
  @file pixel_format.h

  @brief Definition of pixel formats and associated functions

  @copyright StarFive Technology Co., Ltd. All Rights Reserved.

  @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __PIXEL_FORMAT_H__
#define __PIXEL_FORMAT_H__


#include "stf_types.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifndef __maybe_unused
#define __maybe_unused
#endif

/**
 * @defgroup MANTIS_COMMON Common elements
 * @brief Common elements for different libraries, such as the pixel format, 
 * generic buffer transformation and specialised binary file storage
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in MANTIS_COMMON documentation group
 *---------------------------------------------------------------------------*/

#define PXL_FMT_TO_TYPE(PXL_FMT)                                             \
    (((PXL_NONE == PXL_FMT) || (PXL_ISP_444IL3YCrCb10 < PXL_FMT))            \
    ? (TYPE_NONE)                                                            \
    : (((RGB_888_24 > PXL_FMT) || (BAYER_RGGB_NO_GAP_PACK_14 < PXL_FMT))     \
    ? (TYPE_YUV) : ((BAYER_RGGB_8 > PXL_FMT) ? (TYPE_RGB) : (TYPE_BAYER))))

#define ISP_ID_STR(fmt) #fmt

/**
 * @brief high level formats
 */
typedef enum pxlFormat
{
    /**
     * @brief No pixel format provided
     */
    PXL_NONE = 0,
    
    /**
     * @brief 8b 420, VU interleaved - also known as NV21
     */
    YVU_420_PL12_8,
    /**
     * @brief 8b 420, UV interleaved - also known as NV12
     */
    YUV_420_PL12_8,
    /**
     * @brief 8b 422, VU interleaved - also known as NV61
     */
    YVU_422_PL12_8,
    /**
     * @brief 8b 422, UV intereaved - also known as NV16
     */
    YUV_422_PL12_8,
    /**
     * @brief 10b (3 packed in 32b) 420, VU interleaved - also known as 
     * NV21-10bit
     */
    YVU_420_PL12_10,
    /**
     * @brief 10b (3 packed in 32b) 420, UV interleaved - also known as 
     * NV12-10bit
     */
    YUV_420_PL12_10,
    /**
     * @brief 10b (3 packed in 32b) 422, VU interleaved - also known as 
     * NV61-10bit
     */
    YVU_422_PL12_10,
    /**
     * @brief 10b (3 packed in 32b) 422, UV interleaved - also known as 
     * NV16-10bit
     */
    YUV_422_PL12_10,
    
    /**
     * @brief 8b per channel in 24b RGB - also known as BI_RGB24 (B in LSB)
     */
    RGB_888_24,
    /**
     * @brief 8b per channel in 32b - also known as BI_RGB32 or RGBA (with 
     * empty alpha channel and B in LSB)
     */
    RGB_888_32,
    /**
     * @brief 10b per channel in 32b: 0-9 B, 10-19 G, 20-29 R
     */
    RGB_101010_32,
    /**
     * @brief 8/9/10/11/12/13/14/15/16b per channel in 48b: 0-15 R, 16-31 G, 32-47 B
     */
    RGB_161616_48,
    /**
     * @brief 16b per channel in 64b: 0-15 R, 16-31 G, 32-47 B - used for HDF
     * insertion only
     */
    RGB_161616_64,
    /**
     * @brief 10b per channel, and 4 pixel in 120b: 0-9 B, 10-19 G, 20-29 R
     */
    RGB_101010_30_NO_GAP,
    /**
     * @brief 12b per channel, and 2 pixel in 72b: 0-11 B, 12-23 G, 24-35 R
     */
    RGB_121212_36_NO_GAP,
    /**
     * @brief 14b per channel, and 4 pixel in 168b: 0-13 B, 14-27 G, 28-41 R
     */
    RGB_141414_42_NO_GAP,
    /**
     * @brief 16b per channel in 48b: 0-15 B, 16-31 G, 32-47 R
     */
    RGB_161616_48_NO_GAP,
    /**
     * @brief 8b per channel in 24b BGR - inverse of @ref RGB_888_24 (R in LSB)
     */
    BGR_888_24,
    /**
     * @brief 8b per channel in 32b BGR - inverse of @ref RGB_101010_32 (R in 
     * LSB)
     */
    BGR_888_32,
    /**
     * @brief 10b per channel in 32b: 0-9 R, 10-19 G, 20-29 B - inverse of
     * @ref RGB_101010_32 (R in LSB)
     */
    BGR_101010_32,
    /**
     * @brief 8/9/10/11/12/13/14/15/16b per channel in 48b: 0-15 R, 16-31 G, 32-47 B
     */
    BGR_161616_48,
    /**
     * @brief 16b per channel in 64b: 0-15 R, 16-31 G, 32-47 B - used for HDF
     * insertion only
     */
    BGR_161616_64,
    /**
     * @brief 10b per channel, and 4 pixel in 120b: 0-9 R, 10-19 G, 20-29 B - inverse of
     * @ref RGB_101010_30_NO_GAP (R in LSB)
     */
    BGR_101010_30_NO_GAP,
    /**
     * @brief 12b per channel, and 2 pixel in 72b: 0-11 R, 12-23 G, 24-35 B - inverse of
     * @ref RGB_121212_36_NO_GAP (R in LSB)
     */
    BGR_121212_36_NO_GAP,
    /**
     * @brief 14b per channel, and 4 pixel in 168b: 0-13 R, 14-27 G, 28-41 B - inverse of
     * @ref RGB_141414_42_NO_GAP (R in LSB)
     */
    BGR_141414_42_NO_GAP,
    /**
     * @brief 16b per channel in 48b: 0-15 R, 16-31 G, 32-47 B - inverse of
     * @ref RGB_161616_48_NO_GAP (R in LSB)
     */
    BGR_161616_48_NO_GAP,

    /**
     * @brief 8b Bayer (2x2 CFA, pattern is RGGB or a variant) - also known as
     * BA81
     */ 
    BAYER_RGGB_8,
    /**
     * @brief 10b Bayer (2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_10,
    /**
     * @brief 10b Bayer (6 pixels per 1QW)
     */
    BAYER_RGGB_10_1QW,
    /**
     * @brief 10b Bayer (fully packed RAW10, 320 bits = 32 pixels = 5QW)
     */
    BAYER_RGGB_10_5QW,
    /**
     * @brief 12b Bayer (2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_12,
    /**
     * @brief 12b Bayer (5 pixels per 1QW)
     */
    BAYER_RGGB_12_1QW,
    /**
     * @brief 12b Bayer (fully packed RAW12, 16 pixel per 3QW. 192 bits = 16 pixels = 3QW)
     */
    BAYER_RGGB_12_3QW,
    /**
     * @brief 10b Bayer TIFF order (pattern is RGGB or a variant)
     */
    BAYER_TIFF_10,
    /**
     * @brief 12b Bayer TIFF order (pattern is RGBB or a variant)
     */
    BAYER_TIFF_12,
    /**
     * @brief 16b Bayer (2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_16,
    /**
     * @brief 8b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_8,
    /**
     * @brief 9b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_9,
    /**
     * @brief 10b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_10,
    /**
     * @brief 11b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_11,
    /**
     * @brief 12b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_12,
    /**
     * @brief 13b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_13,
    /**
     * @brief 14b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_14,
    /**
     * @brief 15b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_15,
    /**
     * @brief 16b Bayer no gap packed(2x2 CFA, pattern is RGGB or a variant)
     */
    BAYER_RGGB_NO_GAP_PACK_16,

    /**
     * @brief 8 bit YUV packed - also known as IYU2 ((24 bit YUV)
     *
     * @note represents mode 0 of the IEEE 1394 Digital Camera 1.04 spec
     * or YUV444Packed in IIDC2 Digital Camera Control Specification
     * Ver.1.1.0
     *
     */
    PXL_ISP_444IL3YCbCr8,

    /**
     * @brief 8 bit YVU packed , note different order of U and V
     *
     */
    PXL_ISP_444IL3YCrCb8,

    /**
     * @brief 10 bit YUV packed (3 packed in 32b)
     *
     */
    PXL_ISP_444IL3YCbCr10,

    /**
     * @brief 10 bit YVU packed (3 packed in 32b)
     * note different order of U and V
     *
     */
    PXL_ISP_444IL3YCrCb10,

    /**
     * @brief 8/9/10/11/12/13/14/15/16 bit YUV packed (3 packed in 48b)
     *
     */
    PXL_ISP_444IL3YCbCr16,

    /**
     * @brief 8/9/10/11/12/13/14/15/16 bit YVU packed (3 packed in 48b)
     * note different order of U and V
     *
     */
    PXL_ISP_444IL3YCrCb16,

    /**
     * @brief 10 bit YUV packed (3 packed in 30b), and 4 pixel in 120b.
     *
     */
    PXL_ISP_444IL3YCbCr10_NO_GAP,

    /**
     * @brief 10 bit YVU packed (3 packed in 30b), and 4 pixel in 120b.
     *
     */
    PXL_ISP_444IL3YCrCb10_NO_GAP,

    /**
     * @brief 12 bit YUV packed (3 packed in 36b), and 2 pixel in 72b.
     *
     */
    PXL_ISP_444IL3YCbCr12_NO_GAP,

    /**
     * @brief 12 bit YVU packed (3 packed in 36b), and 2 pixel in 72b.
     *
     */
    PXL_ISP_444IL3YCrCb12_NO_GAP,

    /**
     * @brief 14 bit YUV packed (3 packed in 42b), and 4 pixel in 168b.
     *
     */
    PXL_ISP_444IL3YCbCr14_NO_GAP,

    /**
     * @brief 14 bit YVU packed (3 packed in 42b), and 4 pixel in 168b.
     *
     */
    PXL_ISP_444IL3YCrCb14_NO_GAP,

    /**
     * @brief 16 bit YUV packed (3 packed in 48b) in 48b.
     *
     */
    PXL_ISP_444IL3YCbCr16_NO_GAP,

    /**
     * @brief 16 bit YVU packed (3 packed in 48b) in 48b.
     *
     */
    PXL_ISP_444IL3YCrCb16_NO_GAP,

    /**
     * @brief Number of pixels formats - not a valid pixel format!
     */
    PXL_N,

    /**
     * @brief Invalid pixel format
     */
    PXL_INVALID,


} ePxlFormat;

/*
 * low level format
 */

/**
 * @brief Buffers that can be used by ISP
 *
 * @note We may decide to replace this with types from stf_common.h , but for
 * now, due to the usage of the DG formats as well as bayer these handled 
 * separately.
 *
 */
typedef enum FORMAT_TYPES
{
    TYPE_NONE = 0,
    TYPE_RGB,   /**< @brief RGB data (used for display/3d core) */
    TYPE_YUV,   /**< @brief PL12 YUV data for use by video hardware */
    TYPE_BAYER, /**< @brief Bayer format used when doing data extraction */
} eFORMAT_TYPES;

/**
 * @brief Packed data position
 */
typedef enum PACKEDSTART
{
    PACKED_LSB = 0, /**< @brief Least Significant Bits */
    PACKED_MSB         /**< @brief Most Significant Bits */
} ePACKEDSTART;

/**
 * @brief Bayer patterns (2x2 RGGB variants)
 */
typedef enum MOSAICType
{
    MOSAIC_NONE = 0,
    MOSAIC_RGGB,
    MOSAIC_GRBG,
    MOSAIC_GBRG,
    MOSAIC_BGGR
} eMOSAIC;

/**
 * @brief Structure containing the information about a pixel format
 *
 * @li 420 PL12 8b (NV12;NV21):{ YUV,2,2 } + { 8,1,1,LSB }
 * @li 420 PL12 10b (NV12-10;NV21-10): { YUV,2,2 } + { 10,3,4,LSB }
 * @li 422 PL12 8b (NV16;NV61): { YUV,2,1 } + { 8,1,1,LSB }
 * @li 422 PL12 10b (NV16-10;NV61-10): { YUV,1,2 } + { 10,3,4,LSB }
 * @li RGB888-24: { RGB,1,1 } + { 8,1,3,LSB }
 * @li RGB888-32: { RGB,1,1 } + { 8,1,4,LSB } or BGR888-32
 * @li RGB101010: { RGB,1,1 } + { 10,1,4,LSB }
 *
 */
typedef struct PIXELTYPE
{
    /**
     * @brief if copied from a format it is stored here
     */
    enum pxlFormat eFmt;

    // access layer
    /**
     * @brief Buffer type
     */
    eFORMAT_TYPES eBuffer;
    /**
     * @brief Mosaic used - only if the type is Bayer
     */
    eMOSAIC eMosaic;
    /**
     * @brief horizontal subsampling
     */
    STF_U8 u8HSubsampling;
    /**
     * @brief vertical subsampling
     */
    STF_U8 u8VSubsampling;

    // packing layer
    /**
     * @brief in bits
     */
    STF_U8 u8BitDepth;
    /**
     * @brief number of elements represented
     */
    STF_U8 u8PackedElements;
    /**
     * @brief in Bytes
     */
    STF_U8 u8PackedStride;
    /**
     * @brief starts LSB or MSB
     */
    ePACKEDSTART ePackedStart;
    
} PIXELTYPE;

/**
 * @name Pixel structure from enum
 * @brief Functions that transform enums to PIXELTYPE structures
 * @{
 */

/**
 * @brief Returns bit depth for Ycc format
 * @return bit depth.
 */
STF_INT PixelFormatYccBitDepth(ePxlFormat format);

/**
 * @brief Verifies whether format is ycc packed or not.
 * @return STF_TRUE - format is ycc packed
 * @return STF_FALSE - format is not ycc packed
 */
STF_BOOL8 PixelFormatIsPackedYcc(ePxlFormat format);

/**
 * @brief Configures a PIXELTYPE structure using a high-level YUV format
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if the format is not recognised
 */
STF_RESULT PixelTransformYUV(PIXELTYPE *pType, ePxlFormat yuv);

/**
 * @brief Configures a PIXELTYPE structure using a high-level RGB format
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if the format is not recognised
 */
STF_RESULT PixelTransformRGB(PIXELTYPE *pType, ePxlFormat rgb);

/**
 * @brief Configures a PIXELTYPE structure using a high-level Bayer format 
 * - the mosaic is not used
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if the format is not recognised
 */
STF_RESULT PixelTransformBayer(PIXELTYPE *pType, ePxlFormat bayer,
    enum MOSAICType mosaic);

/**
 * @brief Configures a PIXELTYPE structure using a high-level RGB or
 *          YUV_888 format
 *
 * @note HW 2.x : to be used for display output formats
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if the format is not recognised
 */
STF_RESULT PixelTransformDisplay(PIXELTYPE *pType, ePxlFormat format);

/**
 * @brief Get the format string (4CC like) for a format
 *
 * @param ePxl pixel format
 */
const char* FormatString(ePxlFormat ePxl);

/**
 * @brief Get the format string (4CC like) for a filename of saved capture
 *
 * @note Used because existing viewer tools can't parse '444IL3YCrCb8' like
 * formats
 *
 * @param ePxl pixel format
 */
const char* FormatStringForFilename(ePxlFormat ePxl);

const char* FormatString_two_params(enum pxlFormat ePxl, STF_BOOL8 bVU);

/**
 * @brief Returns format from the string
 *
 * Inverse function of @ref FormatString()
 *
 * @warning Slow function - goes through all formats and compare string values
 *
 * @param format pixel format (as returned by @ref FormatString())
 */
enum pxlFormat FormatIndex(const char *format);

/**
 * @brief Get the format string for a mosaic
 *
 * @param mos pixel mosaic
 */
const char* MosaicString(enum MOSAICType mos);

eMOSAIC MosaicFlip(eMOSAIC input, STF_BOOL8 bFlipH, STF_BOOL8 bFlipV);

/**
 * @}
 */
/**
 * @name Buffer transformations
 * @brief Transform buffers to different formats
 * @{
 */

/**
 * @brief Dummy up-sampler (and de-interlace chroma) a YUV format to YUV 444 
 * (duplicate pixels)
 */
unsigned char* BufferTransformYUVTo444(STF_U8 *bufferA, STF_BOOL8 bUV,
    ePxlFormat eTypeA, size_t inWidth, size_t inHeight, size_t outWidth, 
    size_t outHeight);

// 
// conversion matrix & offset
// each line:
// out = coeff[0]*(Y+offset[0]) + coeff[1]*(Cb+offset[1]) 
// + coeff[2]*(Cr+offset[2])
//

/**
 * @brief BT 709 YUV to RGB conversion matrix
 * @note From: BT.709 from Video Demystified 3rd edition, Keith Jack
 * @see BT709_Inputoff[] populated in pixel_transform.c
 */
extern const double BT709_coeff[9];

/**
 * @brief BT 709 YUV to RGB conversion offsets
 * @note From: BT.709 from Video Demystified 3rd edition, Keith Jack
 * @see BT709_coeff populated in pixel_transform.c
 */
extern const double BT709_Inputoff[3];

/**
 * @brief BT 601 YUV to RGB conversion matrix
 * @note From: BT.601 from Video Demystified 3rd edition, Keith Jack
 * @see BT601_Inputoff populated in pixel_transform.c
 */
extern const double BT601_coeff[9];

/**
 * @brief BT 601 YUV to RGB conversion offsets
 * @note From: BT.601 from Video Demystified 3rd edition, Keith Jack
 * @see BT601_coeff populated in pixel_transform.c
 */
extern const double BT601_Inputoff[3];

/**
 * @brief JFIF YUV to RGB conversion matrix
 * @note From: JFIF from http://www.w3.org/Graphics/JPEG/jfif3.pdf (JPEG File 
 * Exchange Format, version 1.02, Eric Hamilton)
 * @see JFIF_Inputoff populated in pixel_transform.c
 */
extern const double JFIF_coeff[9];

/**
 * @brief JFIF YUV to RGB conversion offsets
 * @note From: JFIF from http://www.w3.org/Graphics/JPEG/jfif3.pdf (JPEG File 
 * Exchange Format, version 1.02, Eric Hamilton)
 * @see JFIF_coeff populated in pixel_transform.c
 */
extern const double JFIF_Inputoff[3];

/**
 * @brief Transformation of YUV 444 to RGB using a matrix
 *
 * @see JFIF_coeff, BT601_coeff or BT709_coeff as standard matrices
 */
unsigned char* BufferTransformYUV444ToRGB(
    STF_U8 *bufferA, STF_BOOL8 bUV, enum pxlFormat rgb, STF_BOOL8 bBGR,
    size_t width, size_t height, const double convMatrix[9], 
    const double convInputOff[3]);

/**
 * @brief De-tile a buffer tiled by HW
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if pInput or pOutput is NULL
 * @return STF_ERROR_INVALID_PARAMETERS if u32TileWidth, u32TileHeight,
 * u32TileStride, u32OutputHStride or u32OutputVStride is 0
 * @return STF_ERROR_INVALID_PARAMETERS if u32TileStride > u32OutputHStride
 * or u32TileWidth > u32TileStride
 * @return STF_ERROR_NOT_SUPPORTED if u32OutputHStride is not a multiple of
 * u32TileWidth
 * @return STF_ERROR_NOT_SUPPORTED if u32TileStride is not a multiple of
 * u32TileWidth or u32OutputHStride is not a multiple of u32TileStride
 */
STF_RESULT BufferDeTile(STF_U32 u32TileWidth, STF_U32 u32TileHeight,
    STF_U32 u32TileStride, const STF_U8 *pInput, STF_U8 *pOutput,
    STF_U32 u32OutputHStride, STF_U32 u32OutputVStride);

/**
 * @}
 */

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the StfCommon documentation module
 *---------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif


#endif // __PIXEL_FORMAT_H__
