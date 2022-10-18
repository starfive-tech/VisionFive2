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

#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "wave511/config.h"
    #include "wave511/vpuapi/vpuapifunc.h"
    #include "wave511/sample_v2/helper/main_helper.h"
#else
    #include "config.h"
    #include "vpuapifunc.h"
    #include "main_helper.h"
#endif


#define MAX_QUEUE_NUM 5

typedef void* Component;

typedef enum {
    GET_PARAM_COM_STATE,                    /*!<< It returns state of component. Param: ComponentState* */
    GET_PARAM_COM_IS_CONTAINER_CONUSUMED,   /*!<< pointer of PortContainer */
    GET_PARAM_FEEDER_BITSTREAM_BUF,         /*!<< to a feeder component  : ParamDecBitstreamBuffer */
    GET_PARAM_FEEDER_EOS,                   /*!<< to a feeder component  : BOOL */
    GET_PARAM_VPU_STATUS,                   /*!<< to a component. Get status information of the VPU : ParamVpuStatus. */
    GET_PARAM_DEC_HANDLE,
    GET_PARAM_DEC_CODEC_INFO,               /*!<< It returns a codec information. Param: DecInitialInfo */
    GET_PARAM_DEC_BITSTREAM_BUF_POS,        /*!<< to a decoder component in ring-buffer mode. */
    GET_PARAM_DEC_FRAME_BUF_NUM,            /*!<< to a decoder component : ParamDecNeedFrameBufferNum*/
    GET_PARAM_RENDERER_FRAME_BUF,           /*!<< to a renderer component. ParamDecFrameBuffer */
    GET_PARAM_RENDERER_PPU_FRAME_BUF,       /*!<< to a renderer component. ParamDecPPUFrameBuffer */
    GET_PARAM_ENC_HANDLE,
    GET_PARAM_ENC_FRAME_BUF_NUM,
    GET_PARAM_ENC_FRAME_BUF_REGISTERED,
    GET_PARAM_YUVFEEDER_FRAME_BUF,
    GET_PARAM_READER_BITSTREAM_BUF,
#ifdef SUPPORT_LOOK_AHEAD_RC
    GET_PARAM_ENC_LARC_INFO,
#endif
    GET_PARAM_MAX
} GetParameterCMD;

typedef enum {
    // Common commands
    SET_PARAM_COM_PAUSE,                        /*!<< Makes a component pause. A concrete component needs to implement its own pause state. */
    // Decoder commands
    SET_PARAM_DEC_SKIP_COMMAND,                 /*!<< Send a skip command to a decoder component. */
    SET_PARAM_DEC_TARGET_TID,                   /*!<< Send a target temporal id to a decoder component.
                                                      A parameter is pointer of ParamDecTargetTid structure. */
    SET_PARAM_DEC_RESET,                        /*!<< Reset VPU */
    SET_PARAM_DEC_FLUSH,                        /*!<< Flush command */
    //Encoder commands
    SET_PARAM_ENC_READ_BS_WHEN_FULL_INTERRUPT,  /*!<< Consume the bitstream buffer when the bitstream buffer full interrupt is asserted.
                                                      The parameter is a pointer of BOOL(TRUE or FALSE)
                                                 */
    // Renderer commands
    SET_PARAM_RENDERER_FLUSH,                   /*!<< Drop all frames in the internal queue depending on the ParamDecFlush struct*/
    SET_PARAM_RENDERER_ALLOC_FRAMEBUFFERS,
    SET_PARAM_RENDERER_REALLOC_FRAMEBUFFER,     /*!<< Re-allocate a framebuffer with given parameters.
                                                      A component which is linked with a decoder as a sink component MUST implement this command. : ParamReallocFB
                                                 */
    SET_PARAM_RENDERER_FREE_FRAMEBUFFERS,       /*!<< A command to free framebuffers */
    SET_PARAM_RENDERER_CHANGE_COM_STATE,        /*!<< A command to change a component state for renderer */
    // Feeder commands
    SET_PARAM_FEEDER_START_INJECT_ERROR,        /* The parameter is null. */
    SET_PARAM_FEEDER_STOP_INJECT_ERROR,         /* The parameter is null. */
    SET_PARAM_FEEDER_RESET,
    SET_PARAM_MAX
} SetParameterCMD;

typedef enum {
    COMPONENT_STATE_NONE,
    COMPONENT_STATE_CREATED,
    COMPONENT_STATE_PREPARED,
    COMPONENT_STATE_EXECUTED,
    COMPONENT_STATE_TERMINATED,
    COMPONENT_STATE_MAX
} ComponentState;

typedef enum {
    CNM_COMPONENT_PARAM_FAILURE,
    CNM_COMPONENT_PARAM_SUCCESS,
    CNM_COMPONENT_PARAM_NOT_READY,
    CNM_COMPONENT_PARAM_NOT_FOUND,
    CNM_COMPONENT_PARAM_TERMINATED,
    CNM_COMPONENT_PARAM_MAX
} CNMComponentParamRet;

typedef enum {
    CNM_COMPONENT_TYPE_NONE,
    CNM_COMPONENT_TYPE_ISOLATION,
    CNM_COMPONENT_TYPE_SOURCE,
    CNM_COMPONENT_TYPE_FILTER,
    CNM_COMPONENT_TYPE_SINK,
} CNMComponentType;

typedef enum {
    CNM_PORT_CONTAINER_TYPE_DATA,
    CNM_PORT_CONTAINER_TYPE_CLOCK,
    CNM_PORT_CONTAINER_TYPE_MAX
} CNMPortContainerType;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct PortContainer {
    Uint32  packetNo;
    BOOL    consumed;
    BOOL    reuse;
    BOOL    last;
    Uint32  type;
} PortContainer;

typedef struct PortContainerClock {
    Uint32  packetNo;
    BOOL    consumed;
    BOOL    reuse;
    BOOL    last;
    Uint32  type;
} PortContainerClock;

/** @ingroup buf */
typedef struct PortContainerExternal
{
    Uint32 nSize;              /**< size of the structure in bytes */
    Uint8* pBuffer;            /**< Pointer to actual block of memory
                                     that is acting as the buffer */
    Uint32 nAllocLen;          /**< size of the buffer allocated, in bytes */
    Uint32 nFilledLen;         /**< number of bytes currently in the
                                     buffer */
    Uint32 nOffset;            /**< start offset of valid data in bytes from
                                     the start of the buffer */
    void*  pAppPrivate;        /**< pointer to any data the application
                                     wants to associate with this buffer */
    Uint32 nBufferIndex;
    Uint32 nTickCount;         /**< Optional entry that the component and
                                     application can update with a tick count
                                     when they access the component.  This
                                     value should be in microseconds.  Since
                                     this is a value relative to an arbitrary
                                     starting point, this value cannot be used
                                     to determine absolute time.  This is an
                                     optional entry and not all components
                                     will update it.*/
    Uint64 nTimeStamp;       /**< Timestamp corresponding to the sample
                                     starting at the first logical sample
                                     boundary in the buffer. Timestamps of
                                     successive samples within the buffer may
                                     be inferred by adding the duration of the
                                     of the preceding buffer to the timestamp
                                     of the preceding buffer.*/
    Uint32  nFlags;           /**< buffer specific flags */
    Uint32  index;
} PortContainerExternal;

typedef struct PortContainerES {
    Uint32          packetNo;
    BOOL            consumed;
    BOOL            reuse;                  /*!<< If data in container wasn't consumed then @reuse is assigned to 1. */
    BOOL            last;
    Uint32          type;
    /* ---- DO NOT TOUCH THE ABOVE FIELDS ---- */
    vpu_buffer_t    buf;
    Uint32          size;
    Uint32          streamBufFull;
    /* ---- Belows vairables are for ringbuffer ---- */
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    PhysicalAddress paBsBufStart;
    PhysicalAddress paBsBufEnd;
    vpu_buffer_t    newBsBuf;
} PortContainerES;

typedef struct PortContainerDisplay {
    Uint32          packetNo;
    BOOL            consumed;
    BOOL            reuse;
    BOOL            last;
    Uint32          type;
    /* ---- DO NOT TOUCH THE ABOVE FIELDS ---- */
    DecOutputInfo   decInfo;
} PortContainerDisplay;

typedef struct ParamEncNeedFrameBufferNum {
    Uint32  reconFbNum;
    Uint32  srcFbNum;
} ParamEncNeedFrameBufferNum;

typedef struct ParamEncFrameBuffer {
    Uint32               reconFbStride;
    Uint32               reconFbHeight;
    FrameBuffer*         reconFb;
    FrameBuffer*         srcFb;
    FrameBufferAllocInfo reconFbAllocInfo;
    FrameBufferAllocInfo srcFbAllocInfo;
} ParamEncFrameBuffer;

typedef struct ParamEncBitstreamBuffer {
    Uint32          num;
    vpu_buffer_t*   bs;
} ParamEncBitstreamBuffer;

typedef struct {
    BOOL ringBufferEnable;
    Uint8* encodedStreamBuf;
    Int32 encodedStreamBufSize;
    Int32 encodedStreamBufLength;
} EncodedStreamBufInfo;

typedef struct {
    Uint8* encodedHeaderBuf;
    Int32 encodedHeaderBufSize;
    osal_file_t     *fp;
} EncodedHeaderBufInfo;

typedef struct {
    BOOL ret;
    BOOL success;
    BOOL isConnectedEnc;
    ParamEncNeedFrameBufferNum fbCnt;
} ParamRenderAllocInfo;


typedef struct PortContainerYuv {
    Uint32          packetNo;
    BOOL            consumed;
    BOOL            reuse;
    BOOL            last;
    Uint32          type;
    /* ---- DO NOT TOUCH THE ABOVE FIELDS ---- */
    FrameBuffer     fb;
    FrameBuffer     fbOffsetTbl;
    Int32           srcFbIndex;
    BOOL            prevMapReuse;
} PortContainerYuv;

typedef struct ParamDecBitstreamBuffer {
    Uint32          num;
    vpu_buffer_t*   bs;
} ParamDecBitstreamBuffer;

typedef struct ParamDecNeedFrameBufferNum {
    Uint32  linearNum;                       /*!<< the number of framebuffers which are used to decompress or converter to linear data */
    Uint32  nonLinearNum;                   /*!<< the number of tiled or compressed framebuffers which are used as a reconstruction */
} ParamDecNeedFrameBufferNum;

typedef struct ParamDecFrameBuffer {
    Uint32          stride;
    Uint32          linearNum;               /*!<< the number of framebuffers which are used to decompress or converter to linear data */
    Uint32          nonLinearNum;           /*!<< the number of tiled or compressed framebuffers which are used as a reconstruction */
    FrameBuffer*    fb;
} ParamDecFrameBuffer;

typedef struct ParamDecPPUFrameBuffer {
    BOOL            enablePPU;
    Queue*          ppuQ;
    FrameBuffer*    fb;
} ParamDecPPUFrameBuffer;

typedef struct ParamDecReallocFB {
    Int32           linearIdx;
    Int32           compressedIdx;
    Uint32          width;                  /*!<< New picture width */
    Uint32          height;                 /*!<< New picture hieght */
    FrameBuffer     newFbs[2];              /*!<< Reallocated framebuffers. newFbs[0] for compressed fb, newFbs[1] for linear fb */
} ParamDecReallocFB;

/* ParamDecBitStreamBufPos is used to get or set read pointer and write pointer of a bitstream buffer.
 */
typedef struct ParamDecBitstreamBufPos {
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    Uint32          avail;                  /*!<< the available size */
} ParamDecBitstreamBufPos;

typedef struct ParamVpuStatus {
    QueueStatusInfo  cq;                    /*!<< The command queue status */
} ParamVpuStatus;

typedef struct ParamDecTargetTid {
    Int32           targetTid;
    Int32           tidMode;                /*!<< 0 - targetTid is used as an absolute value, 1 - targetTid is used as an relative value */
} ParamDecTargetTid;


typedef struct ParamReallocFrameBuffer {
    Uint32          tiledIndex;
    Uint32          linearIndex;
} FrameReallocFrameBuffer;

typedef struct {
    Queue*          inputQ;
    Queue*          outputQ;
    Component       owner;
    Component       connectedComponent;     /*!<< NOTE: DO NOT ACCESS THIS VARIABLE DIRECTLY */
    Uint32          sequenceNo;             /*!<< The sequential number of transferred data */
} Port;


typedef struct {
    Uint8*          bitcode;
    Uint32          sizeOfBitcode;                              /*!<< size of bitcode in word(2byte) */
    TestDecConfig   testDecConfig;
    DecOpenParam    decOpenParam;
    TestEncConfig   testEncConfig;
    EncOpenParam    encOpenParam;
    ENC_CFG         encCfg;
#ifdef USE_FEEDING_METHOD_BUFFER
    BOOL            MemoryOptimization;
#endif
} CNMComponentConfig;


#define COMPONENT_EVENT_NONE                    0
/* ------------------------------------------------ */
/* ---------------- COMMON  EVENTS ---------------- */
/* ------------------------------------------------ */
#define COMPONENT_EVENT_SLEEP                   (1ULL<<0)       /*!<< The third parameter of listener is NULL. */
#define COMPONENT_EVENT_WAKEUP                  (1ULL<<1)       /*!<< The third parameter of listener is NULL. */
#define COMPONENT_EVENT_TERMINATED              (1ULL<<2)
#define COMPONENT_EVENT_COMMON_ALL              0xffffULL
/* ------------------------------------------------ */
/* ---------------- DECODER EVENTS ---------------- */
/* ------------------------------------------------ */
#define COMPONENT_EVENT_DEC_OPEN                (1ULL<<16)      /*!<< The third parameter of listener is a pointer of CNMComListenerDecOpen. */
#define COMPONENT_EVENT_DEC_ISSUE_SEQ           (1ULL<<17)      /*!<< The third parameter of listener is a pointer of CNMComListenerDecIssueSeq */
#define COMPONENT_EVENT_DEC_COMPLETE_SEQ        (1ULL<<18)      /*!<< The third parameter of listener is a pointer of CNMComListenerDecCompleteSeq */
#define COMPONENT_EVENT_DEC_REGISTER_FB         (1ULL<<19)      /*!<< The third parameter of listener is a pointer of CNMComListenerDecRegisterFb */
#define COMPONENT_EVENT_DEC_READY_ONE_FRAME     (1ULL<<20)      /*!<< The third parameter of listener is a pointer of CNMComListenerDecReadyOneFrame */
#define COMPONENT_EVENT_DEC_START_ONE_FRAME     (1ULL<<21)      /*!<< The third parameter of listener is a pointer of CNMComListenerStartDecOneFrame. */
#define COMPONENT_EVENT_DEC_INTERRUPT           (1ULL<<22)      /*!<< The third parameter of listener is a pointer of CNMComListenerHandlingInt */
#define COMPONENT_EVENT_DEC_GET_OUTPUT_INFO     (1ULL<<23)      /*!<< The third parameter of listener is a pointer of CNMComListenerDecDone. */
#define COMPONENT_EVENT_DEC_DECODED_ALL         (1ULL<<24)      /*!<< The third parameter of listener is a pointer of CNMComListenerDecClose . */
#define COMPONENT_EVENT_DEC_CLOSE               (1ULL<<25)      /*!<< The third parameter of listener is NULL. */
#define COMPONENT_EVENT_DEC_RESET_DONE          (1ULL<<26)      /*!<< The third parameter of listener is NULL. */
#define COMPONENT_EVENT_DEC_EMPTY_BUFFER_DONE   (1ULL<<27)
#define COMPONENT_EVENT_DEC_FILL_BUFFER_DONE    (1ULL<<28)
#define COMPONENT_EVENT_DEC_ALL                 0xffff0000ULL

/* ------------------------------------------------ */
/* ---------------- RENDERER EVENTS ----------------*/
/* ------------------------------------------------ */
typedef enum {
    COMPONENT_EVENT_RENDER_ALLOCATE_FRAMEBUFER  = (1<<0),
    COMPONENT_EVENT_RENDER_ALL = 0xffffffff
} ComponentEventRenderer;

typedef struct CNMComListenerDecOpen {
    DecHandle   handle;
    RetCode     ret;
} CNMComListenerDecOpen;

typedef struct CNMComListenerDecIssueSeq {
    DecHandle   handle;
    RetCode     ret;
} CNMComListenerDecIssueSeq;

typedef struct CNMComListenerDecCompleteSeq {
    DecInitialInfo*     initialInfo;
    FrameBufferFormat   wtlFormat;
    Uint32              cbcrInterleave;
    CodStd              bitstreamFormat;
    char                refYuvPath[MAX_FILE_PATH];
    RetCode             ret;
} CNMComListenerDecCompleteSeq;

typedef struct CNMComListenerDecRegisterFb {
    DecHandle       handle;
    Uint32          numNonLinearFb;
    Uint32          numLinearFb;
} CNMComListenerDecRegisterFb;

typedef struct CNMComListenerDecReadyOneFrame {
    DecHandle       handle;
} CNMComListenerDecReadyOneFrame;

typedef struct CNMComListenerStartDecOneFrame {
    DecHandle   handle;
    RetCode     result;
    DecParam    decParam;
} CNMComListenerStartDecOneFrame;

typedef struct CNMComListenerDecInt {
    DecHandle       handle;
    Int32           flag;
    Uint32          decIndex;
} CNMComListenerDecInt;

typedef struct CNMComListenerDecDone {
    DecHandle       handle;
    RetCode         ret;
    DecParam*       decParam;
    DecOutputInfo*  output;
    Uint32          numDecoded;
    vpu_buffer_t    vbUser;
    CodStd          bitstreamFormat;    /* codec */
    BOOL            enableScaler;
#if defined(SUPPORT_VCPU_FPGA) || defined(SUPPORT_VCORE_FPGA)
    VcoreTestData*  vcoreData;
#endif /* SUPPORT_VCPU_FPGA || SUPPORT_VCORE_FPGA */
} CNMComListenerDecDone;

typedef struct CNMComListenerDecClose {
    DecHandle       handle;
} CNMComListenerDecClose;

/* ------------------------------------------------ */
/* ---------------- ENCODER EVENTS ---------------- */
/* ------------------------------------------------ */
#define COMPONENT_EVENT_ENC_OPEN                    (1ULL<<32)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncOpen. */
#define COMPONENT_EVENT_ENC_ISSUE_SEQ               (1ULL<<33)      /*!<< The third parameter of listener is NULL */
#define COMPONENT_EVENT_ENC_COMPLETE_SEQ            (1ULL<<34)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncCompleteSeq */
#define COMPONENT_EVENT_ENC_REGISTER_FB             (1ULL<<35)      /*!<< The third parameter of listener is NULL */
#define COMPONENT_EVENT_ENC_READY_ONE_FRAME         (1ULL<<36)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncReadyOneFrame  */
#define COMPONENT_EVENT_ENC_START_ONE_FRAME         (1ULL<<37)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncStartOneFrame */
#define COMPONENT_EVENT_ENC_HANDLING_INT            (1ULL<<38)      /*!<< The third parameter of listener is a pointer of CNMComListenerHandlingInt */
#define COMPONENT_EVENT_ENC_GET_OUTPUT_INFO         (1ULL<<39)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncDone. */
#define COMPONENT_EVENT_ENC_CLOSE                   (1ULL<<40)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncClose. */
#define COMPONENT_EVENT_ENC_FULL_INTERRUPT          (1ULL<<41)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncFull . */
#define COMPONENT_EVENT_ENC_ENCODED_ALL             (1ULL<<42)      /*!<< The third parameter of listener is a pointer of EncHandle. */
#define COMPONENT_EVENT_ENC_RESET                   (1ULL<<43)      /*!<< The third parameter of listener is a pointer of EncHandle. */
#define COMPONENT_EVENT_CODA9_ENC_MAKE_HEADER       (1ULL<<44)      /*!<< The third parameter of listener is a pointer of CNMComListenerEncDone. */
#define COMPONENT_EVENT_ENC_EMPTY_BUFFER_DONE       (1ULL<<45)
#define COMPONENT_EVENT_ENC_FILL_BUFFER_DONE        (1ULL<<46)
#define COMPONENT_EVENT_ENC_ALL                     0xffff00000000ULL

/* ------------------------------------------------ */
/* ---------------- ENC FEEDER EVENTS ----------------*/
/* ------------------------------------------------ */
typedef enum {
    COMPONENT_EVENT_ENC_FEEDER_PREPARE              = (1<<0),
    COMPONENT_EVENT_ENC_FEEDER_ALL                  = 0xffffffff
} ComponentEventEncFeeder;

typedef struct CNMComListenerEncOpen{
    EncHandle       handle;
} CNMComListenerEncOpen;

typedef struct CNMComListenerEncCompleteSeq {
    EncHandle       handle;
} CNMComListenerEncCompleteSeq;

typedef struct CNMComListenerHandlingInt {
    EncHandle       handle;
} CNMComListenerHandlingInt;

typedef struct CNMComListenerEncMakeHeader{
    EncHandle       handle;
    EncodedHeaderBufInfo encHeaderInfo;
} CNMComListenerEncMakeHeader;

typedef struct CNMComListenerEncReadyOneFrame{
    EncHandle       handle;
    RetCode         result;
} CNMComListenerEncReadyOneFrame;

typedef struct CNMComListenerEncStartOneFrame{
    EncHandle       handle;
    RetCode         result;
} CNMComListenerEncStartOneFrame;

typedef struct CNMComListenerEncDone {
    EncHandle       handle;
    EncOutputInfo*  output;
    BOOL            fullInterrupted;
    EncodedStreamBufInfo encodedStreamInfo;
    PhysicalAddress bitstreamBuffer;
    Uint32          bitstreamBufferSize;
} CNMComListenerEncDone;

typedef struct CNMComListenerEncFull {
    EncHandle       handle;
} CNMComListenerEncFull;

typedef struct CNMComListenerEncClose {
    EncHandle       handle;
} CNMComListenerEncClose;

typedef Int32 (*ListenerFuncType)(Component, Port*, void*);
typedef void (*ComponentListenerFunc)(Component com, Uint64 event, void* data, void* context);
typedef struct {
    Uint64                  events;         /*!<< See COMPONENT_EVENT_XXXX, It can be ORed with other events. */
    ComponentListenerFunc   update;
    void*                   context;
} ComponentListener;
#define MAX_NUM_LISTENERS                       32

typedef struct ComponentImpl {
    char*                name;
    void*                context;
    Port                 sinkPort;
    Port                 srcPort;
    Uint32               containerSize;
    Uint32               numSinkPortQueue;
    Component            (*Create)(struct ComponentImpl*, CNMComponentConfig*);
    CNMComponentParamRet (*GetParameter)(struct ComponentImpl*, struct ComponentImpl*, GetParameterCMD, void*);
    CNMComponentParamRet (*SetParameter)(struct ComponentImpl*, struct ComponentImpl*, SetParameterCMD, void*);
    BOOL                 (*Prepare)(struct ComponentImpl*, BOOL*);
    /* \brief   process input data and return output.
     * \return  TRUE - process done
     */
    BOOL                 (*Execute)(struct ComponentImpl*, PortContainer*, PortContainer*);
    /* \brief   release all memories that are allocated by vdi_dma_allocate_memory().
     */
    void                 (*Release)(struct ComponentImpl*);
    BOOL                 (*Destroy)(struct ComponentImpl*);
    BOOL                 success;
    osal_thread_t        thread;
    ComponentState       state;
    BOOL                 terminate;
    ComponentListener    listeners[MAX_NUM_LISTENERS];
    Uint32               numListeners;
    Queue*               usingQ;                /*<<! NOTE: DO NOT USE Enqueue() AND Dequeue() IN YOUR COMPONENT.
                                                            BUT, YOU CAN USE Peek() FUNCTION.
                                                 */
    CNMComponentType     type;
    Uint32               updateTime;
    Uint32               Hz;                    /* Use clock signal ex) 30Hz, A component will receive 30 clock signals per second. */
    void*                internalData;
    BOOL                 pause;
    BOOL                 portFlush;
} ComponentImpl;

Component ComponentCreate(const char* componentName, CNMComponentConfig* componentParam);
BOOL      ComponentSetupTunnel(Component fromComponent, Component toComponent);
ComponentState ComponentExecute(Component component);
/* @return  0 - done
 *          1 - running
 *          2 - error
 */
Int32     ComponentWait(Component component);
void      ComponentStop(Component component);
void      ComponentRelease(Component component);
BOOL      ComponentChangeState(Component component, Uint32 state);
/* \brief   Release all resources of the component
 * \param   ret     The output variable that has status of success or failure.
 */
BOOL      ComponentDestroy(Component component, BOOL* ret);
CNMComponentParamRet ComponentGetParameter(Component from, Component to, GetParameterCMD commandType, void* data);
CNMComponentParamRet ComponentSetParameter(Component from, Component to, SetParameterCMD commandType, void* data);
void      ComponentNotifyListeners(Component component, Uint64 event, void* data);
BOOL      ComponentRegisterListener(Component component, Uint64 events, ComponentListenerFunc func, void* context);
/* \brief   Create a port
 * \param   size        The size of PortStruct of the component
 *          depth       The size of internal input queue and output queue.
 */
void      ComponentPortCreate(Port* port, Component owner, Uint32 depth, Uint32 size);
/* \brief   Fill data into the output queue.
 */
void      ComponentPortSetData(Port* port, PortContainer* portData);
/* \brief   Peek data from the input queue.
 */
PortContainer* ComponentPortPeekData(Port* port);
/* \brief   Get data from the input queue.
 */
PortContainer* ComponentPortGetData(Port* port);
/* \brief   Wait before get data from the input queue.
 */
void*     WaitBeforeComponentPortGetData(Port* port);
/* \brief   Ready status: the output queue is empty.
 */
void      ComponentPortWaitReadyStatus(Port* port);
/* \brief   Destroy port instance
 */
void      ComponentPortDestroy(Port* port);
/* \brief   If a port has input data, it returns true.
 */
BOOL      ComponentPortHasInputData(Port* port);
Uint32    ComponentPortGetSize(Port* port);
/* \brief   Clear input data
 */
void      ComponentPortFlush(Component component);
ComponentState ComponentGetState(Component component);
BOOL      ComponentParamReturnTest(CNMComponentParamRet ret, BOOL* retry);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _COMPONENT_H_

