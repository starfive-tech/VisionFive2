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

#ifndef JPUAPI_H_INCLUDED
#define JPUAPI_H_INCLUDED

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "codaj12/jpuapi/jpuconfig.h"
    #include "codaj12/jdi/jdi.h"
    #include "codaj12/jpuapi/jputypes.h"
#else
    #include "jpuconfig.h"
    #include "../jdi/jdi.h"
    #include "jputypes.h"
#endif

/* _n: number, _s: significance */
#define JPU_CEIL(_s, _n)        (((_n)+(_s-1))&~(_s-1))
#define JPU_FLOOR(_s, _n)       (_n&~(_s-1))
//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------

/**
* @brief
This is a special enumeration type for representing the JPEG profiles.
*/
typedef enum {
    /* Non-differential, Huffman coding */
    JPEG_BASELINE_DCT,            /**< JPEG baseline profile */
    JPEG_EXTENDED_SEQUENTIAL_DCT, /**< JPEG extended sequential profile */
    /* The others are not supported on CODAJ12 */
} JpgProfile;

/**
* @brief
This is a special enumeration type for some configuration commands from host
processor to the JPU. Most of these commands can be called occasionally, not
periodically for changing the configuration of decoder operations running on the
JPU. Details of these commands will be presented in the next chapter.
*/
typedef enum {
    SET_JPG_SCALE_HOR, /**< This command sets the scale down ratio for picture width. The scale ratio value can be either 1(1/2 scale), 2(1/4 scale) or 3(1/8 scale). */
    SET_JPG_SCALE_VER, /**< This command sets the scale down ratio for picture height. The scale ratio value can be either 1(1/2 scale), 2(1/4 scale) or 3(1/8 scale). */
    SET_JPG_USE_STUFFING_BYTE_FF, /**< This command sets use of stuffing byte, 0xFF. */
    SET_JPG_QUALITY_FACTOR, /**< This command sets use of quality factor. */
    ENC_JPG_GET_HEADER,  /**< This command generates JPEG high-level header syntaxes. */
    ENABLE_LOGGING,      /**< This command activates message logging once JPU_DecOpen() or JPU_EncOpen() is called. */
    DISABLE_LOGGING,     /**< This command deactivates message logging. (default) */
    JPG_CMD_END
} JpgCommand;


/**
* @brief
This is a type for representing physical addresses which is recognizable by the
JPU. In general, the JPU hardware does not know about virtual address space
which is set and handled by host processor. All these virtual addresses are
translated into physical addresses by Memory Management Unit. All data buffer
addresses such as stream buffer, frame buffer, should be given to the JPU as an
address on physical address space.
*/
typedef enum {
    JPG_RET_SUCCESS, /**< This means that operation was done successfully. */
    JPG_RET_FAILURE, /**< This means that operation was not done successfully. */
    JPG_RET_BIT_EMPTY, /**< This means that bitstream buffer is found to be empty during decode operation. */
    JPG_RET_EOS, /**< This means the end of stream. */
    JPG_RET_INVALID_HANDLE, /**< This means that the given handle for the current API function call was invalid(for example, not initialized yet, improper function call for the given handle, etc.).  */
    JPG_RET_INVALID_PARAM,  /**< This means that the given argument parameters (for example, input data structure) was invalid (not initialized yet or not valid anymore). */
    JPG_RET_INVALID_COMMAND, /**< This means that the given command was invalid (for example, undefined, or not allowed in the given instances). */
    JPG_RET_ROTATOR_OUTPUT_NOT_SET, /**< This means that rotator output buffer was not allocated even though postprocessor (rotation, mirroring, or deringing) is enabled.  */
    JPG_RET_ROTATOR_STRIDE_NOT_SET, /**< This means that rotator stride was not provided even though postprocessor (rotation, mirroring, or deringing) is enabled. */
    JPG_RET_FRAME_NOT_COMPLETE, /**< This means that frame decoding operation was not completed yet, so the given API function call cannot be allowed. */
    JPG_RET_INVALID_FRAME_BUFFER, /**< This means that the given source frame buffer pointers were invalid in encoder (not initialized yet or not valid anymore).  */
    JPG_RET_INSUFFICIENT_FRAME_BUFFERS, /**< This means that the given numbers of frame buffers were not enough for the operations of the given handle. */
    JPG_RET_INVALID_STRIDE, /**< This means that the given stride was invalid (for example, 0, not a multiple of 8 or smaller than picture size). */
    JPG_RET_WRONG_CALL_SEQUENCE, /**< This means that the current API function call was invalid considering the allowed sequences between API functions (for example, missing one crucial function call before  this function call). */
    JPG_RET_CALLED_BEFORE, /**< This means that multiple calls of the current API function for a given instance are invalid. */
    JPG_RET_NOT_INITIALIZED, /**< This means that JPU was not initialized yet. Before calling any API functions, the initialization API function, JPU_Init(), should be called at the beginning. */
    JPG_RET_INSUFFICIENT_RESOURCE, /**< This means that JPU cannot allocate memory due to lack of memory. */
    JPG_RET_INST_CTRL_ERROR, /**< This means there was a fault in instance control. */
    JPG_RET_NOT_SUPPORT, /**< This means that HOST application uses an API option that is not supported in current hardware. */
} JpgRet;

/**
* @brief
This is a special enumeration type for representing the current encoder states.
*/
typedef enum {
    ENCODE_STATE_NEW_FRAME = 0,  /**< The state indicates the start of a new frame. It appears for slice encoding. */
    ENCODE_STATE_FRAME_DONE = 0, /**< The state indicates the completion of encoding a frame. */
    ENCODE_STATE_SLICE_DONE = 1  /**< The state indicates the completion of encoding a slice. */
} EncodeState;


/**
* @brief
This is an enumeration type for representing mirroring directions.
*/
typedef enum {
    MIRDIR_NONE, /**< No mirroring */
    MIRDIR_VER,  /**< Vertical mirroring */
    MIRDIR_HOR,  /**< Horizontal mirroring */
    MIRDIR_HOR_VER /**< Horizontal and veritical mirroring */
} JpgMirrorDirection;

/**
* @brief
This is an enumeration type for representing frame buffer formats.
*/
typedef enum {
    FORMAT_420 = 0, /**< YUV420 format */
    FORMAT_422 = 1, /**< YUV422 format */
    FORMAT_440 = 2, /**< YUV440 format */
    FORMAT_444 = 3, /**< YUV444 format */
    FORMAT_400 = 4, /**< YUV400 format */
    FORMAT_MAX
} FrameFormat;

/**
* @brief
This is an enumeration type for representing a CbCr interleave mode of frame buffer.
*/
typedef enum {
    CBCR_SEPARATED = 0, /**< Cb data and Cr data are located in each separate plane. */
    CBCR_INTERLEAVE, /**< Chroma data are interleaved in Cb and Cr order. */
    CRCB_INTERLEAVE  /**< Chroma data are interleaved in Cr and Cb order. */
} CbCrInterLeave;

/**
* @brief
This is an enumeration type for representing a packed format of frame buffer.
*/
typedef enum {
    PACKED_FORMAT_NONE,
    PACKED_FORMAT_422_YUYV,  /**< This is YUV422 packed format with YUYV component order. */
    PACKED_FORMAT_422_UYVY,  /**< This is YUV422 packed format with UYVY component order. */
    PACKED_FORMAT_422_YVYU,  /**< This is YUV422 packed format with YVYU component order. */
    PACKED_FORMAT_422_VYUY,  /**< This is YUV422 packed format with VYUY component order. */
    PACKED_FORMAT_444,       /**< This is YUV444 packed format.  */
    PACKED_FORMAT_MAX
} PackedFormat;

typedef enum {
    O_FMT_NONE,
    O_FMT_420,
    O_FMT_422,
    O_FMT_444,
    O_FMT_MAX
} OutputFormat;

/* Assume that pixel endianness is big-endian.
 * b0 is low address in a framebuffer.
 * b1 is high address in a framebuffer.
 * pixel consists of b0 and b1.
 * RIGHT JUSTIFICATION: (default)
 * lsb         msb
 * |----|--------|
 * |0000| pixel  |
 * |----|--------|
 * | b0   |   b1 |
 * |-------------|
 * LEFT JUSTIFICATION:
 * lsb         msb
 * |--------|----|
 * | pixel  |0000|
 * |--------|----|
 * | b0   |   b1 |
 * |-------------|
 */
enum JPU_PIXEL_JUSTIFICATION {
    PIXEL_16BIT_MSB_JUSTIFIED,
    PIXEL_16BIT_LSB_JUSTIFIED,
    PIXEL_16BIT_JUSTIFICATION_MAX,
};

/**
* @brief
This is an enumeration type for representing the JPU interrupt reasons.
*/
typedef enum {
    INT_JPU_DONE = 0,  /**< This interrupt indicates the completion of encoding/decoding a frame. */
    INT_JPU_ERROR = 1, /**< This interrupt indicates occurrence of error. */
    INT_JPU_BIT_BUF_EMPTY = 2, /**< This interrupt indicates JPU encounters empty bitstream buffer while decoding. */
    INT_JPU_BIT_BUF_FULL = 2,  /**< This interrupt indicates full state of bitstream buffer while encoding. */     /* For encoder */
    INT_JPU_SLICE_DONE = 9,    /**< This interrupt indicates the completion of encoding a slice. */     /* For encoder */
}InterruptJpu;

/**
* @brief
This is an enumeration type for representing quantization and Huffman table encoding modes.
*/
typedef enum {
    JPG_TBL_NORMAL,
} JpgTableMode;

/**
* @brief
This is an enumeration type for representing encode header modes.
*/
typedef enum {
    ENC_HEADER_MODE_NORMAL,  /**< Generation of the whole JPEG header syntaxes  */
    ENC_HEADER_MODE_SOS_ONLY /**< Generation of scan header syntax only  */
} JpgEncHeaderMode;

/**
* @brief This is an enumeration type for representing scale down ratios.
*/
enum JPU_DOWNSCALE_VAL {
    JPG_SCALE_DOWN_NONE,     /**< No scaling */
    JPG_SCALE_DOWN_ONE_HALF, /**< 1/2 scale*/
    JPG_SCALE_DOWN_ONE_QUARTER, /**< 1/4 scale */
    JPG_SCALE_DOWN_ONE_EIGHTS,  /**< 1/8 scale */
    JPG_SCALE_DOWN_MAX
};

/**
* @brief This is an enumeration type for representing a product ID.
*/
enum JPU_PRODUCT_ID {
    PRODUCT_ID_CODAJ12 = 12,
};

/**
* @brief
@verbatim
This is a data structure for representing frame buffer pointers of each Y/Cb/Cr color
component. In case of CbCr interleave mode, Cb and Cr frame data are written to memory area starting
from the given bufCb address.

All of the Y/Cb/Cr component addresses must be 8-byte aligned.
A pixel value occupies 1 byte for JPEG baseline sequential and 2 bytes for JPEG extended sequential(12bit).
In case the frame format is YCbCr420, the sizes of Cb and Cr buffer are 1/4 of Y buffer size.
Host application must allocate frame buffers for each Y/Cb/Cr component in consideration of alignment and chroma format buffer size .

@endverbatim
*/
typedef struct {
    PhysicalAddress bufY;  /**< It indicates the base address for Y component in the physical address space when linear map is used. */
    PhysicalAddress bufCb; /**< It indicates the base address for Cb component in the physical address space when linear map is used. */
    PhysicalAddress bufCr; /**< It indicates the base address for Cr component in the physical address space when linear map is used. */
    Uint32          stride; /**< A horizontal stride for luma frame buffer */    /* luma stride */
    Uint32          strideC;/**< A horizontal stride for chroma frame buffer */    /* chroma stride */
    FrameFormat     format; /**< <<jpuapi_h_OutputFormat>> */
/**
@verbatim
Bitstream buffer endianess

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bits little endian format
@* 3 : 32 bits big endian format
@endverbatim
*/
    Uint32          endian;
} FrameBuffer;

struct JpgInst;

/**
* @brief
This is a dedicated type for JPG handle returned when an instance is opened.
An instance can be referred to by the corresponding handle.
Application does not need to care about it.
*/
typedef struct JpgInst * JpgHandle;

//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------

typedef struct JpgInst JpgDecInst;

/**
* @brief
@verbatim
This is a dedicated type for decoder handle returned when a decoder instance is
opened. A decoder instance can be referred to by the corresponding handle.
JpgDecInst is a type managed internally by API. Application does not need to
care about it.

NOTE: This type is vaild for decoder only.
@endverbatim
*/
typedef JpgDecInst * JpgDecHandle;

/* JPU Capabilities */
/**
* @brief
This is a data structure for representing JPU capabilities.
*/
typedef struct {
    Uint32  productId;     /**< Product ID number, 12 */
    Uint32  revisoin;      /**< Revision number of hardware */
    BOOL    support12bit;  /**< It indicates whether to support an extended sequential JPEG, 12bit. */
} JPUCap;

/**
* @brief
This data structure is used when host application wants to open a new decoder instance.
*/
typedef struct {
    PhysicalAddress bitstreamBuffer; 		/**< The start address of bit stream buffer from which the decoder can get the next bitstream. This address must be 8-byte aligned.*/
    Uint32          bitstreamBufferSize;	/**< The virtual address according to bitstreamBuffer. This address is needed when the host wants to access bitstream.*/
    BYTE*           pBitStream;				/**< The size of the buffer pointed by bitstreamBuffer in byte. This value must be a multiple of 1024. The maximum size is 16383 x 1024 bytes.*/
/**
@verbatim
Bitstream buffer endianess

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bits little endian format
@* 3 : 32 bits big endian format
@endverbatim
*/
    Uint32          streamEndian;
/**
@verbatim
Frame buffer endianess

* 0 : little endian format
* 1 : big endian format
* 2 : 32 bits little endian format
* 3 : 32 bits big endian format
@endverbatim
*/
    Uint32          frameEndian;
/**
@verbatim
@* 0: CbCr data is written in separate frame memories (chroma separate mode)
@* 1: CbCr data is interleaved in chroma memory. (chroma interleave mode)
@endverbatim
*/
    CbCrInterLeave  chromaInterleave;
    PackedFormat    packedFormat;			/**< This is for packed image format. It is valid only when source image format is written in YUV444 or RGB24. */
    BOOL            roiEnable;				/**< This enables the ROI (Region of Interest). */
    Uint32          roiOffsetX;				/**< This is ROI offset for width. */
    Uint32          roiOffsetY;				/**< This is ROI offset for height. */
    Uint32          roiWidth;				/**< This is ROI width from RoiOffsetX. */
    Uint32          roiHeight;				/**< This is ROI width from RoiOffsetY. */
    Uint32          pixelJustification;     /**< It specifies pixel justification of source image for 12bit encoding mode. */ /* avaliable in 16bit pixel. */
    Uint32          intrEnableBit;          /**< It specifies an interrupt enable bit. For the meaning of each bit, refer to <<jpuapi_h_InterruptJpu>>. */
    Uint32          rotation;              /**< It rotates decoded images in 0, 90, 180, or 270 degree */
    JpgMirrorDirection  mirror;            /**< <<jpuapi_h_JpgMirrorDirection>> */
    FrameFormat     outputFormat;          /**< <<jpuapi_h_OutputFormat>> */
} JpgDecOpenParam;

/**
* @brief
Data structure to get information necessary to start decoding from the decoder.
*/
typedef struct {
/**
@verbatim
Horizontal picture size in pixels

This width value will be used while allocating decoder frame buffers. In some
cases, this returned value, the display picture width declared on stream header,
should be modified before allocating frame buffers. When picture width is not a
multiple of 16, picture width for buffer allocation should be re-calculated from
the declared display width as follows:

 picBufWidth = {( picWidth + 15)/16}* 16,

where picBufWidth is the horizontal picture buffer width. When picWidth is a
multiple of 16, picWidth = picBufWidth.
*/
	int         picWidth;
/**
@verbatim
Vertical picture size in pixels

This height value will be used while allocating decoder frame buffers. In some
cases, this returned value, the display picture height declared on stream
header, should be modified before allocating frame buffers. When picture height
is not a multiple of 16, picture height for buffer allocation should be
re-calculated from the declared display height as follows:

 picBufHeight = {( picHeight + 15)/16}* 16,

where picBufHeight is the vertical picture buffer height. When picHeight is a
multiple of 16, picHeight = picBufHeight.
@endverbatim
*/
    int         picHeight;
/**
This is the minimum number of frame buffers required for decoding. Applications
must allocate at least as many as this number of frame buffers and register the
number of buffers to the JPU using JPU_DecRegisterFrameBuffer() before decoding
pictures.
*/
    int         minFrameBufferCount;
    FrameFormat sourceFormat;  /**< <<jpuapi_h_FrameFormat>> */
    int         ecsPtr;        /**< This is an entropy-coded segment pointer of the first frame. */
    int         roiFrameWidth; /**< This is width of ROI region. */
    int         roiFrameHeight;/**< This is height of ROI region. */
    int         roiFrameOffsetX; /**< This is x position of ROI region. */
    int         roiFrameOffsetY; /**< This is y position of ROI region. */
    Uint32      bitDepth;  /**< 8 for baseline sequential, 12 for extended sequential */
} JpgDecInitialInfo;

/**
* @brief
The data structure to set a scaler ratio.
*/
typedef struct {
    int scaleDownRatioWidth;  /**< The ratio of frame width to scale down */
/**
@verbatim
The ratio of frame height to scale down. It might have one of the following values:

@* 0 : no scale
@* 1 : (1/2 scale)
@* 2 : (1/4 scale)
@* 3 : (1/8 scale)
@endverbatim
*/
    int scaleDownRatioHeight;
} JpgDecParam;

/**
* @brief
The data structure to get result information from decoding a frame.
*/
typedef struct {
/**
This is the frame buffer index of a picture to be displayed this time among
frame buffers which were registered using JPU_DecRegisterFrameBuffer(). Frame
data to be displayed this time will be stored into the frame buffer specified by
this index. When delay in display does not exist, this index will always be the
same with indexFrameDecoded.
At the end of sequence decoding, if there is no more output for display, then
this value will have -1 (0xFFFF). So by checking this index, host applications
can easily know whether sequence decoding has been finished or not.
*/
    int         indexFrameDisplay;
    int         numOfErrMBs;        /**< The number of erroneous macroblocks while decoding a picture */
/**
@verbatim
@* 0 : incomplete finish of decoding process
@* 1 : complete finish of decoding process
@endverbatim
*/
    int	        decodingSuccess;
    int         decPicHeight;       /**< The JPU reports the height of a decoded picture in pixel unit after decoding one frame. This value will be updated, when sequence information for resolution is changed. */
    int         decPicWidth;        /**< The JPU reports the width of a decoded picture in pixel unit after decoding one frame. This value will be updated, when sequence information for resolution is changed. */
    int         consumedByte;       /**< This means how many bytes are consumed in a bitstream. */
    int         bytePosFrameStart;  /**< This means the position of an SOI maker in bitstream buffer. */
    int         ecsPtr;             /**< This is a pointer to an entropy-coded segment of current frame. */
    Uint32      frameCycle;         /**< This variable reports the number of clock cycles taken to decode one frame. */ /* clock cycle */
    Uint32      rdPtr;              /**< The position of read pointer in the bitstream buffer after completion of frame decoding */
    Uint32      wrPtr;              /**< The position of write pointer in the bitstream buffer after completion of frame decoding */
} JpgDecOutputInfo;



//------------------------------------------------------------------------------
// encode struct and definition
//------------------------------------------------------------------------------

typedef struct JpgInst JpgEncInst;

/**
* @brief
@verbatim
This is a dedicated type for encoder handle returned when an encoder instance is
opened. An encoder instance can be referred to by the corresponding handle.
JpgEncInst is a type managed internally by API. Application does not need to
care about it.
NOTE: This type is vaild for encoder only.
@endverbatim
*/
typedef JpgEncInst * JpgEncHandle;

/**
* @brief
This data structure is used when host applications want to open a new encoder
instance.
*/
typedef struct {
    PhysicalAddress bitstreamBuffer;     /**< The start address of bit stream buffer into which encoder puts bit streams. This address must be a multiple of 4, namely, 4 byte-aligned. */
    Uint32          bitstreamBufferSize; /**< The size of the buffer in bytes pointed by bitstreamBuffer. This value must be a multiple of 1024. The maximum size is 16383 x 1024 bytes. */
    Uint32          picWidth;            /**< The width of a picture to be encoded in pixels */
    Uint32          picHeight;           /**< The height of a picture to be encoded in pixels */
    FrameFormat     sourceFormat;        /**< <<jpuapi_h_FrameFormat>> */
    Uint32          restartInterval;     /**< The number of MCU in the restart interval */
/**
@verbatim
Bitstream buffer endianess

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bits little endian format
@* 3 : 32 bits big endian format
@endverbatim
*/
	Uint32          streamEndian;
/**
@verbatim
Frame buffer endianess

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bits little endian format
@* 3 : 32 bits big endian format
@endverbatim
*/
    Uint32          frameEndian;
/**
@verbatim
@* 0 : CbCr data is written in separate frame memories (chroma separate mode)
@* 1 : CbCr data is interleaved in chroma memory. (chroma interleave mode)
@endverbatim
*/
    CbCrInterLeave  chromaInterleave;
    BYTE            huffVal[8][256];     /**< A list of the 8-bit symbol values in Huffman tables. */
    BYTE            huffBits[8][256];    /**< A 16-byte list giving number of codes for each code length from 1 to 16 in Huffman tables. */
    short           qMatTab[4][64];      /**< Quantization tables */
    BOOL            jpg12bit;            /**< It enables 12bit encoding mode. */
    BOOL            q_prec0;             /**< Luma quantization tables for 12bit encoding mode */
    BOOL            q_prec1;             /**< Chroma quantization tables for 12bit encoding mode */
    PackedFormat    packedFormat;        /**< It specifies a packed image format. It is valid only when source image format is written in YUV444 or RGB24. */
    Uint32          pixelJustification;  /**< It specifies pixel justification of source image for 12bit encoding mode. */
    Uint32          tiledModeEnable;     /**< It enables use of tiled mode. */
    Uint32          sliceHeight;         /**< It specifies a slice height for slice encoding. */
    Uint32          intrEnableBit;       /**< It specifies an interrupt enable bit. For the meaning of each bit, refer to <<jpuapi_h_InterruptJpu>>. */
    BOOL            sliceInstMode;
    Uint32          rotation;            /**< It rotates source images in 0, 90, 180, or 270 degree before encoding. */
    Uint32          mirror;              /**< <<jpuapi_h_JpgMirrorDirection>> */
} JpgEncOpenParam;


/**
* @brief
This is a data structure for configuring one frame encoding operation.
*/
typedef struct {
    FrameBuffer * sourceFrame; /**< This member must represent the frame buffer containing source image to be encoded. */
} JpgEncParam;

/**
* @brief
This is a data structure for reporting the results of picture encoding operations.
*/
typedef struct {
/**
The Physical address of the starting point of newly encoded picture stream
If dynamic buffer allocation is enabled in line-buffer mode, this value will be
identical with the specified picture stream buffer address by host applications.
*/
    PhysicalAddress bitstreamBuffer;
    Uint32 bitstreamSize;            /**< The byte size of encoded bitstream */
    PhysicalAddress streamRdPtr;     /**< A read pointer of bitstream buffer */
    PhysicalAddress streamWrPtr;     /**< A write pointer of bitstream buffer */
    Uint32 encodedSliceYPos;         /**< A vertical position of the currently encoded slice */
    EncodeState encodeState;         /**< A result of the current encoding, completion of encoding the whole picture or completion of encoding a specific slice */
    Uint32  frameCycle;              /**< This variable reports the number of clock cycles taken to decode one frame. */ /* clock cycle */
} JpgEncOutputInfo;

/**
* @brief
This is a data structure for generating an encoding header.
*/
typedef struct {
    PhysicalAddress paraSet;   /**< The physical address of bitstream buffer where header bitstream is saved  */
    BYTE *pParaSet;            /**< The virtual address of bitstream buffer where header bitstream is saved  */
    int size;                  /**< The allocation size of buffer for saving header bitstream */
/**
@verbatim
Header encoding mode

@* ENC_HEADER_MODE_NORMAL: generates all sorts of headers.
@* ENC_HEADER_MODE_SOS_ONLY: generates only SOS header (not any other such as SOI, APP, DRI, DQT, DHT, and SOF headers).
@endverbatim
*/
    int headerMode;
/**
@verbatim
Quantization table merge option

@* 0: do not merge quantization table
@* 1: merge quantization table
@endverbatim
*/
    int quantMode;
/**
@verbatim
Huffman table merge option

@* 0: do not merge huffman table
@* 1: merge huffman table
@endverbatim
*/
    int huffMode;
    int disableAPPMarker;      /**< It disables generating an App Marker in encoding header. */
    int enableSofStuffing;     /**< It enables to fill 0xff at the end of header if it is not 8-byte aligned after encoding the header. (defalut: 0x00) */
} JpgEncParamSet;



#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief This function returns whether the JPU is initialized and ready to be used.
* @return
@verbatim
@* 0: The JPU hardware is not initialized yet. In that case, host application should call JPU_Init().
@* 1: The JPU hardware is initialized.
@endverbatim
*/
Uint32 JPU_IsInit(
	void
	);

/**
* @brief This function waits until an interrupt arises and returns the interrupt reason if it occurs.
* @param handle [Input] An instance handle obtained from JPU_EncOpen() or JPU_DecOpen()
* @param timeout [Input] A timeout value in millisecond
* @return
@verbatim
@* 0: The JPU hardware is not initialized yet. In that case, host application should call JPU_Init().
@* 1: The JPU hardware is initialized.
@endverbatim
*/
    Int32		JPU_WaitInterrupt(JpgHandle handle, int timeout);
/**
* @brief This function returns whether JPU has completed processing a frame or not.
* @param handle [Input] An instance handle obtained from JPU_EncOpen() or JPU_DecOpen()
* @return
@verbatim
@* 0: The JPU hardware is idle.
@* Non-zero value: The JPU hardware is busy with processing a frame.
@endverbatim
*/
	int			JPU_IsBusy(JpgHandle handle);
/**
* @brief This function returns the reason of JPU interrupt.
* @param handle [Input] An instance handle obtained from JPU_EncOpen() or JPU_DecOpen()
* @return
@verbatim
@* 0: No Interrupt, The JPU hardware is idle.
@* Non-zero value: It is an interrupt reason. Refer to <<jpuapi_h_InterruptJpu>> to know the meaning of each bit.
@endverbatim
*/
    Uint32		JPU_GetStatus(JpgHandle handle);
/**
* @brief Clearing Interrupt
* @param handle [Input] An instance handle obtained from JPU_EncOpen()
* @param val [Input] The interrupt bit to be cleared
* @return None
*/
    void		JPU_ClrStatus(JpgHandle handle, Uint32 val);

/**
* @brief This function initializes the JPU hardware and proper data structures/resources. Applications
must call this function only once before using the JPU.
* @param None
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means the JPU initialized successfully.
*JPG_RET_FAILURE*::
Operation was failed, which means the JPU initialized with fail.
@endverbatim
*/
    JpgRet		JPU_Init();
/**
* @brief This function frees all the resources allocated by API.
* @param None
* @return None
*/
	void		JPU_DeInit();
/**
* @brief This function returns the number of instances that have opened.
* @param None
* @return The number of instance that is currently running
*/
    int			JPU_GetOpenInstanceNum();
/**
* @brief Applications can read out the version information running on the system.
* @param  apiVersion [output] API version ID
* @param  hwRevision [output] Hardware versi
* @param  hwProductId [output] Product ID
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means version information is acquired
successfully.

*JPG_RET_NOT_INITIALIZED*::
JPU was not initialized at all before calling this function. Application should
initialize JPU by calling JPU_Init() before calling this function.
@endverbatim
*/
    JpgRet		JPU_GetVersionInfo(
        Uint32 *apiVersion,
        Uint32 *hwRevision,
        Uint32 *hwProductId);

/**
* @brief
This function resets JPU and initializes all the internal contexts.
It can be done by user's system reset signal and implmentation of jdi_hw_reset().
* @param None
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means the JPU has been reset successfully.

*JPG_RET_FAILURE*::
Operation was failed, which means the JPU was unable to reset.
@endverbatim
*/
	JpgRet JPU_HWReset();
/**
* @brief This function resets JPU by using the reset register.
* @param None
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means the JPU has been reset successfully.
@endverbatim
*/
    JpgRet JPU_SWReset();

    // function for decode
/**
* @brief
In order to decode, applications must open the decoder. By calling this
function, applications can get a handle by which they can refer to a decoder
instance. Because the JPU is multiple instance codec, applications need this
kind of handle. Once an application gets a handle, the application must pass
this handle to all subsequent decoder-related functions.
* @param handle [Output] A pointer to a JpgDecHandle type variable which will
specify each instance for application.
* @param openParam [Input] A pointer to a JpgDecOpenParam type structure which describes
required parameters for creating a new decoder instance.
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means a new decoder instance was created
successfully.

*JPG_RET_FAILURE*::
Operation was failed, which means getting a new decoder instance was not done
successfully. If there is no free instance anymore, this value will be returned
in this function call.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).
@endverbatim
*/
	JpgRet JPU_DecOpen(
		JpgDecHandle* handle,
		JpgDecOpenParam* openParam);
/**
* @brief
When an application finished decoding a sequence and wanted to release this
instance for other processing, the applicaton should close this instance. After
completion of this function call, the instance referred to by handle will get
free. Once an application closes an instance, the application cannot call any
further decoder-specific function with the current handle before re-opening a
new decoder instance with the same handle.
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means current decoder instance was closed
successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).
@endverbatim
*/
    JpgRet JPU_DecClose(
		JpgDecHandle handle);
/**
* @brief
@verbatim
Applications must pass the address of a JpgDecInitialInfo structure, where the
decoder will store information such as picture size, number of necessary frame
buffers, etc. For the details, see definition of the section called
JpgDecInitialInfo data structure. This function should be called once after
creating a decoder instance and before starting frame decoding.

It is application\'s responsibility to provide sufficient amount of bitstream to
the decoder by calling JPU_DecUpdateBitstreamBuffer() so that bitstream buffer
does not get empty before this function returns.
@endverbatim
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param info [Output] A pointer to JpgDecInitialInfo data structure
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means required information of the stream
data to be decoded was received successfully.

*JPG_RET_FAILURE*::
Operation was failed, which means there was an error in getting information for
configuring the decoder.

*JPG_RET_INVALID_PARAM*::
The given argument parameter, info, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).
@endverbatim
*/
    JpgRet JPU_DecGetInitialInfo(
        JpgDecHandle handle,
        JpgDecInitialInfo * info);
/**
* @brief
@verbatim
This function is used for registering frame buffers with the acquired
information from JPU_DecGetInitialInfo(). The frame buffers pointed to by
bufArray will be managed internally within the JPU. These include reference
frames, reconstructed frame, etc. Applications must not change the contents of
the array of frame buffers during the life time of the instance, and num must
not be less than minFrameBufferCount obtained by JPU_DecGetInitialInfo().

The distance between a pixel in a row and the corresponding pixel in the next
row is called a stride. The value of stride must be a multiple of 8. The address
of the first pixel in the second row does not necessarily coincide with the
value next to the last pixel in the first row. In other words, a stride can be
greater than the picture width in pixels.

Applications should not set a stride value smaller than the picture width. So,
for Y component, application must allocate at least a space of size (frame
height  stride), and Cb or Cr component, (frame height/2  stride/2),
respectively.

But make sure that in Cb/Cr non-interleave (separate Cb/Cr) map, the stride for
luminance frame buffers should be multiple of 16 so that the stride for
chrominance frame buffers can become a multiple of 8.
In case of MJPEG encoding, the address of frame buffer is not necessary and only
stride value is required, which will be used as source image stride.
@endverbatim
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param bufArray [Input] A number of the minimum frame buffer count
* @param num [Input] A number of frame buffers
* @param stride [Input] A stride value of the given frame buffers
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means registering frame buffer
information was done successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_WRONG_CALL_SEQUENCE*::
This means current API function call was invalid considering the allowed
sequences between API functions. An application might call this function before
calling JPU_DecGetInitialInfo() successfully. This function should be called
after successful calling JPU_DecGetInitialInfo().

*JPG_RET_INVALID_FRAME_BUFFER*::
This happens when bufArray was invalid, which means bufArray was not initialized
yet or not valid anymore.

*JPG_RET_INSUFFICIENT_FRAME_BUFFERS*::
This means the given number of frame buffers, num, was not enough for the
decoder operations of the given handle. It should be greater than or equal to
the value requested by JPU_DecGetInitialInfo().

*JPG_RET_INVALID_STRIDE*::
The given argument stride was invalid, which means it is smaller than the
decoded picture width, or is not a multiple of 8 in this case.
@endverbatim
*/
    JpgRet JPU_DecRegisterFrameBuffer(
        JpgDecHandle handle,
        FrameBuffer * bufArray,
        int num,
        int stride);
/**
* @brief
@verbatim
This function is used for registering frame buffers with the acquired
information from JPU_DecGetInitialInfo(). The frame buffers pointed to by
bufArray will be managed internally within the JPU. These include reference
frames, reconstructed frame, etc. Applications must not change the contents of
the array of frame buffers during the life time of the instance, and num must
not be less than minFrameBufferCount obtained by JPU_DecGetInitialInfo().

The distance between a pixel in a row and the corresponding pixel in the next
row is called a stride. The value of stride must be a multiple of 8. The address
of the first pixel in the second row does not necessarily coincide with the
value next to the last pixel in the first row. In other words, a stride can be
greater than the picture width in pixels.

Applications should not set a stride value smaller than the picture width. So,
for Y component, application must allocate at least a space of size (frame
height  stride), and Cb or Cr component, (frame height/2  stride/2),
respectively.

But make sure that in Cb/Cr non-interleave (separate Cb/Cr) map, the stride for
luminance frame buffers should be multiple of 16 so that the stride for
chrominance frame buffers can become a multiple of 8.
In case of MJPEG encoding, the address of frame buffer is not necessary and only
stride value is required, which will be used as source image stride.
@endverbatim
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param prdPrt [Input] A stream buffer read pointer for the current decoder instance
* @param pwrPtr [Input] A stream buffer write pointer for the current decoder instance
* @param size [Input] A variable specifying the available space in bitstream buffer for the current decoder instance
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means registering frame buffer
information was done successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_WRONG_CALL_SEQUENCE*::
This means current API function call was invalid considering the allowed
sequences between API functions. An application might call this function before
calling JPU_DecGetInitialInfo() successfully. This function should be called
after successful calling JPU_DecGetInitialInfo().

*JPG_RET_INVALID_FRAME_BUFFER*::
This happens when bufArray was invalid, which means bufArray was not initialized
yet or not valid anymore.

*JPG_RET_INSUFFICIENT_FRAME_BUFFERS*::
This means the given number of frame buffers, num, was not enough for the
decoder operations of the given handle. It should be greater than or equal to
the value requested by JPU_DecGetInitialInfo().

*JPG_RET_INVALID_STRIDE*::
The given argument stride was invalid, which means it is smaller than the
decoded picture width, or is not a multiple of 8 in this case.
@endverbatim
*/
	JpgRet JPU_DecGetBitstreamBuffer(
        JpgDecHandle handle,
        PhysicalAddress * prdPrt,
        PhysicalAddress * pwrPtr,
        int * size );
/**
* @brief
Applications must let decoder know how much bitstream has been transferred to
the address obtained from JPU_DecGetBitstreamBuffer(). By just giving the size
as an argument, API automatically handles pointer wrap-around and updates the
write pointer.
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param size [Input] A variable specifying the amount of bits transferred into
bitstream buffer for the current decoder instance
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means putting new stream data was done
successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).
@endverbatim
*/
	JpgRet JPU_DecUpdateBitstreamBuffer(
        JpgDecHandle handle,
        int size);

/**
* @brief
This function starts decoding one frame. Returning from this function does not
mean the completion of decoding one frame, and it is just that decoding one
frame was initiated. Every call of this function should be matched with
JPU_DecGetOutputInfo() with the same handle. Without calling a pair of these
funtions, applications cannot call any other API functions except for
JPU_IsBusy(), JPU_DecGetBitstreamBuffer(), and JPU_DecUpdateBitstreamBuffer().
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param param [Input] A pointer to a JpgDecParam type structure which describes picture decoding parameters for the given decoder instance
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means decoding a new frame was started
successfully.

NOTE: This return value does not mean that decoding a frame was completed
successfully.

*JPG_RET_FAIURE*::
Operation was failed, which means there was an error in starting a frame.

*JPG_RET_INVALID_PARAM*::
This happens when param was invalid, which means param was not initialized yet
or not valid anymore.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_FRAME_NOT_COMPLETE*::
This means frame decoding operation was not completed yet, so the given API
function call cannot be performed this time. A frame decoding operation should
be completed by calling JPU_DecGetOutputInfo(). Even though the result of the
current frame operation is not necessary, application should call
JPU_DecGetOutputInfo() to proceed this function call.

*JPG_RET_WRONG_CALL_SEQUENCE*::
This means current API function call was invalid considering the allowed
sequences between API functions. An application might call this function before
successfully calling JPU_DecRegisterFrameBuffer(). This function should be
called after calling JPU_ DecRegisterFrameBuffer() successfully.

*JPG_RET_BIT_EMPTY*::
This means the bit-stream buffer is empty. So, API requests the bit-stream data.

*JPG_RET_EOS*::
This means the stream ends.
@endverbatim
*/
    JpgRet JPU_DecStartOneFrame(
        JpgDecHandle handle,
        JpgDecParam *param );

    JpgRet JPU_DecStartOneFrameBySerialNum(
        JpgDecHandle handle,
        JpgDecParam *param,
        int bufferIndex );
/**
* @brief
Application can get the information of output of decoding. The information
includes the information of a frame buffer which contains the reconstructed
image. Host applications should call this function after frame decoding is
finished and before starting the further processing.
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param info [Output] A pointer to a JpgDecOutputInfo type structure which
describes picture decoding results for the current decoder instance.
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means receiving the output information of
current frame was done successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_WRONG_CALL_SEQUENCE*::
This means current API function call was invalid considering the allowed
sequences between API functions. It might happen because JPU_DecStartOneFrame()
with the same handle was not called before calling this function.

*JPG_RET_INVALID_PARAM*::
The given argument parameter, info, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
@endverbatim
*/
	JpgRet JPU_DecGetOutputInfo(
        JpgDecHandle handle,
        JpgDecOutputInfo * info);
/**
* @brief
@verbatim
This function is provided to let application have a certain level of freedom for
re-configuring decoder operation after creating a decoder instance. Some options
which can be changed dynamically during decoding as the video sequence has been
included. Some command-specific return codes are also presented.

The list of valid commands can be summarized as follows:

@* SET_JPG_SCALE_HOR
@* SET_JPG_SCALE_VER
@* ENABLE_LOGGING
@* DISABLE_LOGGING
@endverbatim
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param cmd [Intput] A variable specifying the given command of <<jpuapi_h_JpgCommand>>
* @param parameter [Input/Output] A pointer to command-specific data structure which
describes picture I/O parameters for the current decoder instance
* @return
@verbatim
*JPG_RET_INVALID_COMMAND*::
The given argument, cmd, was invalid, which means the given cmd was undefined,
or not allowed in the current instance.

*JPG_RET_SUCCESS*::
Operation was done successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).
@endverbatim
*/
	JpgRet JPU_DecGiveCommand(
        JpgDecHandle handle,
        JpgCommand cmd,
        void * parameter);
/**
* @brief
This function specifies the location of read pointer in bitstream buffer.
It can also set a write pointer with same value of read pointer (addr) when updateWrPtr is not a zero value,
which allows to flush up the bitstream buffer at once.
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param addr [Input] Updated read or write pointer
* @param updateWrPtr [Input] A flag whether to update a write pointer or not as well as a read pointer.
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means required information of the stream data to be
decoded was received successfully.

*JPG_RET_FAIURE*::
Operation was failed, which means there was an error in starting a frame.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_FRAME_NOT_COMPLETE*::
This means frame decoding operation was not completed yet, so the given API function call
cannot be performed this time. A frame decoding operation should be completed by calling
JPU_DecSetRdPtr().
@endverbatim
*/
	JpgRet JPU_DecSetRdPtr(
        JpgDecHandle handle,
        PhysicalAddress addr,
        BOOL updateWrPtr);
/**
* @brief
This function is used to change the location of read pointer in bitstream buffer.
It is useful when host application decodes MJPEG with binary of every frame which is obtained from demuxer.
After frame decoding, host application can use this function which allows the given read pointer to be the base address of bitstream buffer like in line buffer mode.
* @param handle [Input] A decoder handle obtained from JPU_DecOpen()
* @param addr [Input] Updated read or write pointer
* @param updateWrPtr [Input] A flag whether to update a write pointer or not as well as a read pointer.
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means required information of the stream data to be
decoded was received successfully.

*JPG_RET_FAIURE*::
Operation was failed, which means there was an error in starting a frame.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_FRAME_NOT_COMPLETE*::
This means frame decoding operation was not completed yet, so the given API function call
cannot be performed this time. A frame decoding operation should be completed by calling
JPU_DecSetRdPtr().

@endverbatim
*/
	JpgRet JPU_DecSetRdPtrEx(
        JpgDecHandle handle,
        PhysicalAddress addr,
        BOOL updateWrPtr);

    // function for encode
/**
* @brief
In order to start a new encoder operation, applications must open a new instance
for this encoder operation.
* @param handle [Output] A pointer to a JpgEncHandle type variable
* @param openParam [Input] A pointer to a JpgEncOpenParam type structure
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means a new encoder instance was opened
successfully.

*JPG_RET_FAILURE*::
Operation was failed, which means getting a new encoder instance was not done
successfully. If there is no free instance anymore, this value will be returned
in this function call.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).
@endverbatim
*/
    JpgRet JPU_EncOpen(
		JpgEncHandle* handle,
		JpgEncOpenParam* openParam);
/**
* @brief
When an application finished encoding operations and wanted to release this
instance for other processing, the application should close this instance by
calling this function. After completion of this function call, the instance
referred to by handle will get free. Once an application closes an instance, the
application cannot call any further encoder-specific function with the current
handle before re-opening a new instance with the same handle.
* @param handle [Input] An encoder handle obtained from JPU_EncOpen()
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully. That means current encoder instance was closed
successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_FRAME_NOT_COMPLETE*::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling JPU_EncGetOutputInfo() or
JPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, applications should call JPU_EncGetOutputInfo() or
JPU_DecGetOutputInfo() to proceed this function call.
@endverbatim
*/
    JpgRet JPU_EncClose(
		JpgEncHandle handle);
/**
* @brief
After encoding frame, applications must get bitstream from the encoder. To do
that, they must know where to get bitstream and the maximum size. Applications
can get the information by calling this function.
* @param handle [Input] A encoder handle obtained from JPU_EncOpen()
* @param prdPrt [Output] A stream buffer read pointer for the current encoder instance
* @param pwrPtr [Output] A stream buffer write pointer for the current encoder instance
* @param size [Output] A variable specifying the available space in bitstream buffer
for the current encoder instance
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means required information for encoder
stream buffer was received successfully.

*RETCODE_INVALID_PARAM*::
The given argument parameter, prdPrt, pwrPtr or size, was invalid, which means
it has a null pointer, or given values for some member variables are improper
values.
@endverbatim
*/
    JpgRet JPU_EncGetBitstreamBuffer(
        JpgEncHandle handle,
        PhysicalAddress * prdPrt,
        PhysicalAddress * pwrPtr,
        int * size);
/**
* @brief
Applications must let encoder know how much bitstream has been transferred from
the address obtained from JPU_EncGetBitstreamBuffer(). By just giving the size
as an argument, API automatically handles pointer wrap-around and updates the
read pointer.
* @param handle [Input] A encoder handle obtained from JPU_EncOpen()
* @param size [Input] A variable specifying the amount of bits being filled from
bitstream buffer for the current encoder instance.
If 0 is given, JPUAPI forces a read pointer and a write pointer to be wrapped around and to be updated.
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means putting new stream data was done
successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).
@endverbatim
*/
	JpgRet JPU_EncUpdateBitstreamBuffer(
        JpgEncHandle handle,
        int size);
/**
* @brief
@verbatim
This function starts encoding one frame. Returning from this function does not
mean the completion of encoding one frame, and it is just that encoding one
frame was initiated.

Every call of this function should be matched with JPU_EncGetOutputInfo() with
the same handle. Without calling a pair of these funtions, applications cannot
call any other API functions except for JPU_IsBusy(),
JPU_EncGetBitstreamBuffer(), and JPU_EncUpdateBitstreamBuffer().
@endverbatim
* @param handle [Input] An encoder handle obtained from JPU_EncOpen()
* @param param [Input] A pointer to JpgEncParam type structure which describes
picture encoding parameters for the current encoder instance.
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means encoding a new frame was started
successfully.

NOTE: This return value does not mean that encoding a frame was completed
successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_FRAME_NOT_COMPLETE*::
This means frame decoding or encoding operation was not completed yet, so the
given API function call cannot be performed this time. A frame encoding or
decoding operation should be completed by calling JPU_EncGetOutputInfo() or
JPU_DecGetOutputInfo(). Even though the result of the current frame operation is
not necessary, applications should call JPU_EncGetOutputInfo() or
JPU_DecGetOutputInfo() to proceed this function call.

*JPG_RET_INVALID_PARAM*::
The given argument parameter, param, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
@endverbatim
*/
	JpgRet JPU_EncStartOneFrame(
        JpgEncHandle handle,
        JpgEncParam * param );
/**
* @brief
This function gets information of the output of encoding. Application can know
about picture type, the address and size of the generated bitstream, the number
of generated slices, the end addresses of the slices, and macroblock bit
position information. Host application should call this function after frame
encoding is finished, and before starting the further processing.
* @param handle [Input] An encoder handle obtained from JPU_EncOpen().
* @param info [Output] A pointer to a JpgEncOutputInfo  type structure which
describes picture encoding results for the current encoder instance.
* @return
@verbatim
*JPG_RET_SUCCESS*::
Operation was done successfully, which means the output information of current
frame encoding was received successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

*JPG_RET_WRONG_CALL_SEQUENCE*::
This means current API function call was invalid considering the allowed
sequences between API functions. An application might call this function before
calling JPU_EncStartOneFrame() successfully. This function should be called
after successful calling of JPU_EncStartOneFrame().

*JPG_RET_INVALID_PARAM*::
The given argument parameter, info, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.
@endverbatim
*/
	JpgRet JPU_EncGetOutputInfo(
        JpgEncHandle handle,
        JpgEncOutputInfo * info);
/**
* @brief
@verbatim
This function is provided to let application have a certain level of freedom for
re-configuring encoder operation after creating an encoder instance. Some
options which can be changed dynamically during encoding as the video sequence
has been included. Some command-specific return codes are also presented.

The list of valid commands can be summarized as follows:

@* SET_JPG_USE_STUFFING_BYTE_FF
@* ENC_JPG_GET_HEADER
@* ENABLE_LOGGING
@* DISABLE_LOGGING
@endverbatim
* @param handle [Input] An encoder handle obtained from JPU_EncOpen()
* @param cmd [Input] A variable specifying the given command of <<jpuapi_h_JpgCommand>>
* @param param [Input/Output] A pointer to command-specific data structure which
describes picture I/O parameters for the current encoder instance
* @return
@verbatim
*JPG_RET_INVALID_COMMAND*::
This means the given argument, cmd, was invalid which means the given cmd was
undefined, or not allowed in the current instance.

*JPG_RET_INVALID_PARAM*::
The given argument parameter, info, was invalid, which means it has a null
pointer, or given values for some member variables are improper values.

*JPG_RET_SUCCESS*::
Operation was done successfully, which means the given cmd was received
successfully.

*JPG_RET_INVALID_HANDLE*::
This means the given handle for current API function call was invalid (for
example, not initialized yet, improper function call for the given handle,
etc.).

@endverbatim
*/
	JpgRet JPU_EncGiveCommand(
        JpgEncHandle handle,
        JpgCommand cmd,
        void * parameter);

#ifdef __cplusplus
}
#endif

#endif
