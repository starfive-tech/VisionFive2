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

#include <string.h>
#include "component.h"
#include "cnm_app.h"
#include <sys/prctl.h>

#ifdef PLATFORM_NON_OS
BOOL supportThread = FALSE;
#else
BOOL supportThread = TRUE;
#endif

#define DEFAULT_NO_RESPONSE_TIMEOUT     720          // in second

typedef struct ClockData {
    BOOL    start;
    Uint64  lastClock;          /* in millisecond */
} ClockData;

/*
 * component_list.h is generated in the makefiles.
 * See Wave5xxDecV2.mak or Wave5xxEncV2.mak.
 */
#include "component_list.h"

static void SetupSinkPort(ComponentImpl* component, ComponentImpl* connectedComponent)
{
    component->sinkPort.connectedComponent = (Component)connectedComponent;
}

static void SetupSrcPort(ComponentImpl* component, ComponentImpl* connectedComponent, Port sinkPort)
{
    Port* srcPort = &component->srcPort;

    component->srcPort.inputQ             = sinkPort.outputQ;
    component->srcPort.outputQ            = sinkPort.inputQ;
    component->srcPort.owner              = component;
    component->srcPort.connectedComponent = connectedComponent;
    component->usingQ = Queue_Create_With_Lock(srcPort->inputQ->size, srcPort->inputQ->itemSize);
}

Component ComponentCreate(const char* componentName, CNMComponentConfig* componentParam)
{
    ComponentImpl* instance = NULL;
    ComponentImpl* com;
    Uint32         i=0;

    while ((com=componentList[i++])) {
        if (strcmp(componentName, com->name) == 0) break;
    }
    if (com == NULL) {
        VLOG(ERR, "%s:%d Can't find %s component\n", __FUNCTION__, __LINE__, componentName);
        return NULL;
    }

    // Create an instance.
    instance = (ComponentImpl*)osal_malloc(sizeof(ComponentImpl));
    osal_memcpy(instance, com, sizeof(ComponentImpl));
    if (instance->Create(instance, componentParam) == NULL) {
        osal_free(instance);
        instance= NULL;
    }
    else {
        Port*  port = &instance->sinkPort;
        Uint32 size = instance->containerSize;
        void*  data = osal_malloc(size);

        osal_memset((void*)data, 0x00, size);
        ComponentPortCreate(port, instance, instance->numSinkPortQueue, size);
        // Fill input queue
        for (i=0; i<instance->numSinkPortQueue; i++) {
            Queue_Enqueue(port->inputQ, data);
        }
        osal_free(data);

        instance->state       = COMPONENT_STATE_CREATED;
        instance->type        = CNM_COMPONENT_TYPE_ISOLATION;
        if (instance->Hz) {
            ClockData* clk = (ClockData*)osal_malloc(sizeof(ClockData));
            clk->start     = FALSE;
            clk->lastClock = 0ULL;
            instance->internalData = (void*)clk;
        }
    }

    return (Component)instance;
}

BOOL ComponentSetupTunnel(Component fromComponent, Component toComponent)
{
    ComponentImpl* src  = (ComponentImpl*)fromComponent;
    ComponentImpl* sink = (ComponentImpl*)toComponent;
    BOOL           hasComponent;

    if (fromComponent == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (toComponent == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    SetupSinkPort(src, sink);
    SetupSrcPort(sink, src, src->sinkPort);

    hasComponent = (BOOL)(src->srcPort.connectedComponent != NULL);
    src->type = (hasComponent == FALSE) ? CNM_COMPONENT_TYPE_SOURCE : CNM_COMPONENT_TYPE_FILTER;

    hasComponent = (BOOL)(sink->sinkPort.connectedComponent != NULL);
    sink->type = (hasComponent == FALSE) ? CNM_COMPONENT_TYPE_SINK : CNM_COMPONENT_TYPE_FILTER;

#ifdef SUPPORT_LOOK_AHEAD_RC
    if ( !strcmp(src->name, "yuvfeeder") ) {
        src->type = CNM_COMPONENT_TYPE_SOURCE;
    }
    if ( !strcmp(src->name, "reader") ) {
        src->type = CNM_COMPONENT_TYPE_SINK;
    }
#endif
    return TRUE;
}

void ComponentWaitState(Component component, ComponentState state)
{
    ComponentImpl* imp = (ComponentImpl*)component;

    if (imp == NULL) return;

    do {
        osal_msleep(1);
    } while (imp->state < state);
}

static void WaitReturningPortData(ComponentImpl* com)
{
    PortContainer* container;

    while ((container=(PortContainer*)Queue_Dequeue(com->usingQ))) {
        ComponentPortSetData(&com->srcPort, container);
    }
    while ((container=ComponentPortGetData(&com->srcPort))) {
        ComponentPortSetData(&com->srcPort, container);
    }
}

Int32 ComponentWait(Component component)
{
    ComponentImpl*  com = (ComponentImpl*)component;
    Int32           ret;
    Int32           retval;

    if (supportThread == FALSE || com->thread == NULL) {
        return (com->state != COMPONENT_STATE_TERMINATED) ? 2 : 0;
    }

    if ((ret=osal_thread_timedjoin(com->thread, (void**)&retval, 100)) == 0) {
        com->thread = NULL;
        WaitReturningPortData(com);
    }

    return ret;
}

static BOOL HasPortData(ComponentImpl* com, PortContainer** in, PortContainer** out)
{
    BOOL success = FALSE;

    *in     = NULL;
    *out    = NULL;
    switch (com->type) {
    case CNM_COMPONENT_TYPE_SOURCE:
        if ((*out=ComponentPortPeekData(&com->sinkPort)) != NULL) {
            success = TRUE;
        }
        break;
    case CNM_COMPONENT_TYPE_FILTER:
        *in=ComponentPortPeekData(&com->srcPort);
        *out=ComponentPortPeekData(&com->sinkPort);
        /* A filter component needs an container for output */
        success = (BOOL)(*out != NULL);
        break;
    case CNM_COMPONENT_TYPE_SINK:
        if ((*in=ComponentPortPeekData(&com->srcPort)) != NULL) {
            success = TRUE;
        }
        break;
    default:
        /* SINGLE COMPONENT */
        success = TRUE;
        break;
    }
    if (*in)  (*in)->reuse  = TRUE;
    if (*out) (*out)->reuse = TRUE;

    return success;
}

static BOOL ReturnPortContainer(ComponentImpl* com, BOOL inputConsumed, BOOL hasOutput)
{
    BOOL  doReturn = FALSE;
    BOOL  returned = FALSE;

    switch (com->type) {
    case CNM_COMPONENT_TYPE_FILTER:
        doReturn = (BOOL)(hasOutput || inputConsumed);
        break;
    case CNM_COMPONENT_TYPE_SINK:
        doReturn = inputConsumed;
        break;
    default:
        doReturn = FALSE;
        break;
    }

    if (doReturn) {
        PortContainer* container;
        Uint32          i, numItems = Queue_Get_Cnt(com->usingQ);
        for (i=0; i<numItems; i++) {
            container=(PortContainer*)Queue_Dequeue(com->usingQ);
            if (container == NULL) break;
            // Intentionally infinite loop to debug easily
            ComponentGetParameter(NULL, com, GET_PARAM_COM_IS_CONTAINER_CONUSUMED, (void*)container);
            if (container->consumed == TRUE) {
                ComponentPortSetData(&com->srcPort, container);
                // Return the used source data
                returned = TRUE;
                break;
            }
            else {
                Queue_Enqueue(com->usingQ, container);
            }
        }
    }

    return returned;
}

static void SendClockSignal(ComponentImpl* com)
{
    if (com->Hz) {
        PortContainerClock data;
        ClockData*  clk  = (ClockData*)com->internalData;
        BOOL        send = FALSE;

        if (clk->start == FALSE) {
            clk->start = TRUE;
            clk->lastClock = osal_gettime();
            send = TRUE;
        }
        else {
            Uint32 diff = osal_gettime() - clk->lastClock;
            send = (diff >= com->Hz);
        }

        if (send == TRUE) {
            data.type = CNM_PORT_CONTAINER_TYPE_CLOCK;
            com->Execute(com, (PortContainer*)&data, NULL);
        }
    }
}

static BOOL Execute(ComponentImpl* com)
{
    PortContainer* in      = NULL;
    PortContainer* out     = NULL;
    BOOL            success = TRUE;

    SendClockSignal(com);

    if (HasPortData(com, &in, &out) == TRUE) {
        if ((success=com->Execute(com, in, out)) == FALSE) {
            com->terminate = TRUE;
        }

        if (in && in->reuse == FALSE) {
            Queue_Enqueue(com->usingQ, (void*)in);
            // The "in" container still exists in a source port. It removes the container from the source port.
            ComponentPortGetData(&com->srcPort);
        }

        if (out && out->reuse == FALSE) {
            // The "out" container still exists in a sink port. It removes the container from the the sink port.
            // If the container has no content, it has to be reused.
            ComponentPortGetData(&com->sinkPort);
            // Send data to the sink component
            ComponentPortSetData(&com->sinkPort, out);
        }
        // Return a consumed container to the source port.
        ReturnPortContainer(com, (in && in->consumed), (out && out->reuse == FALSE));
    }

    if (com->portFlush == TRUE) {
        // Flush all data in the source port
        void* data;

        while ((data=Queue_Dequeue(com->usingQ))) {
            ComponentPortSetData(&com->srcPort, data);
        }

        while ((data=ComponentPortGetData(&com->srcPort))) {
            ComponentPortSetData(&com->srcPort, data);
        }
        com->portFlush = FALSE;
    }

    return success;
}


static void DoYourJob(ComponentImpl* com)
{
    BOOL            success;
    BOOL            done = FALSE;
    ComponentImpl*  sinkComponent;

    if (CNMErrorGet() == CNM_ERROR_HANGUP) {
        com->terminate = TRUE;
    }
    else {
        switch (com->state) {
        case COMPONENT_STATE_CREATED:
            if ((success=com->Prepare(com, &done)) == TRUE) {
                if (done == TRUE) com->state = COMPONENT_STATE_PREPARED;
            }
            break;
        case COMPONENT_STATE_PREPARED:
            if ((success=Execute(com)) == TRUE) {
                com->state = COMPONENT_STATE_EXECUTED;
            }
            break;
        case COMPONENT_STATE_EXECUTED:
            success = Execute(com);
            break;
        default:
            success = FALSE;
            break;
        }
        if ((com->success=success) == FALSE) {
            com->terminate = TRUE;
        }
    }
    /* Check if connected components are terminated */
    sinkComponent = (ComponentImpl*)com->sinkPort.connectedComponent;
    if (sinkComponent && sinkComponent->terminate == TRUE) {
        printf("%s skin terminate\r\n",com->name);
        com->terminate = TRUE;
    }

    if (com->terminate == TRUE) {
        com->state = COMPONENT_STATE_TERMINATED;
    }
}

static void DoThreadWork(void* arg)
{
    ComponentImpl* com = (ComponentImpl*)arg;
    prctl(PR_SET_NAME, com->name);
    while (com->terminate == FALSE) {
        DoYourJob(com);
        osal_msleep(2); // To yield schedule
    }
    ComponentNotifyListeners(com, COMPONENT_EVENT_TERMINATED, NULL);

    com->state = COMPONENT_STATE_TERMINATED;
}

ComponentState ComponentExecute(Component component)
{
    ComponentImpl* com   = (ComponentImpl*)component;

    if (com == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return COMPONENT_STATE_TERMINATED;
    }

    if (supportThread) {
        com->thread = osal_thread_create(DoThreadWork, (void*)com);
    }
    else {
        DoYourJob(com);
    }

    return com->state;
}

void ComponentStop(Component component)
{
    ComponentImpl*  com   = (ComponentImpl*)component;

    if (com->terminate == FALSE) {
        com->terminate = TRUE;
        if (supportThread == FALSE) {
            com->state = COMPONENT_STATE_TERMINATED;
            WaitReturningPortData(com);
        }
    }
}

void ComponentRelease(Component component)
{
    ComponentImpl*  impl = (ComponentImpl*)component;

    if (impl) {
        impl->Release(impl);
    }

    return;
}

BOOL ComponentDestroy(Component component, BOOL* ret)
{
    ComponentImpl*  impl = (ComponentImpl*)component;
    BOOL            success;

    if (impl == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (ret) {
        *ret = impl->success;
    }

    ComponentPortDestroy(&impl->sinkPort);

    success = impl->Destroy(impl);
    Queue_Destroy(impl->usingQ);
    if (impl->internalData) osal_free(impl->internalData);

    osal_free(impl);

    return success;
}

static char* getParamName[GET_PARAM_MAX] = {
    "GET_PARAM_COM_STATE",
    "GET_PARAM_FEEDER_BITSTREAM_BUF",
    "GET_PARAM_DEC_HANDLE",
    "GET_PARAM_DEC_CODEC_INFO",
    "GET_PARAM_DEC_BITSTREAM_BUF_POS",
    "GET_PARAM_DEC_FRAME_BUF_NUM",
    "GET_PARAM_VPU_STATUS",
    "GET_PARAM_RENDERER_FRAME_BUF",
    "GET_PARAM_ENC_HANDLE",
    "GET_PARAM_ENC_FRAME_BUF_NUM",
    "GET_PARAM_ENC_FRAME_BUF_REGISTERED",
    "GET_PARAM_YUVFEEDER_FRAME_BUF",
    "GET_PARAM_READER_BITSTREAM_BUF",
};

CNMComponentParamRet ComponentGetParameter(Component from, Component to, GetParameterCMD commandType, void* data)
{
    ComponentImpl* com = (ComponentImpl*)to;
    CNMComponentParamRet ret;

    if (com == NULL) {
        VLOG(ERR, "%s:%d Invalid handle or the port closed\n", __FUNCTION__, __LINE__);
        return CNM_COMPONENT_PARAM_FAILURE;
    }

    if (com->terminate == TRUE) {
        return CNM_COMPONENT_PARAM_TERMINATED;
    }
    else {
        if ((ret=com->GetParameter((ComponentImpl*)from, com, commandType, data)) == CNM_COMPONENT_PARAM_NOT_FOUND) {
            // Redirect a command.
            // Send a command other connected components.
            if (com->srcPort.connectedComponent == from) {
                // source component ----> current component -----> sink component
                to = com->sinkPort.connectedComponent;
            }
            else {
                // source component <---- current component <----- sink component
                to = com->srcPort.connectedComponent;
            }

            if (to == NULL) {
                VLOG(ERR, "%s:%d The command(%s) is not supported\n", __FUNCTION__, __LINE__, getParamName[commandType]);
                return CNM_COMPONENT_PARAM_FAILURE;
            }
            from = com;
            return ComponentGetParameter((Component*)from, to, commandType, data);
        }
    }

    return ret;
}

void ComponentNotifyListeners(Component component, Uint64 event, void* data)
{
    ComponentImpl*          com = (ComponentImpl*)component;
    Uint32                  i   = 0;
    Uint64                  listeningEvents;
    void*                   context;
    ComponentListenerFunc   update  = NULL;

    for (i=0; i<com->numListeners; i++) {
        listeningEvents = com->listeners[i].events;
        update          = com->listeners[i].update;
        context         = com->listeners[i].context;
        if (listeningEvents & event) {
            update(component, event, data, context);
        }
    }
}

BOOL ComponentRegisterListener(Component component, Uint64 events, ComponentListenerFunc func, void* context)
{
    ComponentImpl*  com = (ComponentImpl*)component;
    Uint32          num;

    if (com == NULL) return FALSE;

    num = com->numListeners;

    if (num == MAX_NUM_LISTENERS) {
        VLOG(ERR, "%s:%d Failed to ComponentRegisterListener\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    com->listeners[num].events  = events;
    com->listeners[num].update  = func;
    com->listeners[num].context = context;
    com->numListeners           = num+1;

    return TRUE;
}

CNMComponentParamRet ComponentSetParameter(Component from, Component to, SetParameterCMD commandType, void* data)
{
    ComponentImpl* com = (ComponentImpl*)to;

    if (com == NULL) {
        VLOG(ERR, "%s:%d Invalid handle or the port closed\n", __FUNCTION__, __LINE__);
        return CNM_COMPONENT_PARAM_FAILURE;
    }

    if (com->terminate == TRUE) {
        return CNM_COMPONENT_PARAM_FAILURE;
    }
    else {
        if (com->SetParameter((ComponentImpl*)from, com, commandType, data) == FALSE) {
            // Redirect a command.
            // Send a command other connected components.
            if (com->srcPort.connectedComponent == from) {
                // source component ----> current component -----> sink component
                to = com->sinkPort.connectedComponent;
            }
            else {
                // source component <---- current component <----- sink component
                to = com->srcPort.connectedComponent;
            }

            if (to == NULL) {
                VLOG(ERR, "%s:%d The command(%d) is not supported\n", __FUNCTION__, __LINE__, commandType);
                return CNM_COMPONENT_PARAM_FAILURE;
            }
            from = com;
            return ComponentSetParameter((Component*)from, to, commandType, data);
        }
    }

    return CNM_COMPONENT_PARAM_SUCCESS;
}

void ComponentPortCreate(Port* port, Component owner, Uint32 depth, Uint32 size)
{
    // Release the queues which are created previously.
    port->inputQ  = Queue_Create_With_Lock(depth, size);
    port->outputQ = Queue_Create_With_Lock(depth, size);
    port->owner   = owner;
    port->sequenceNo = 0;
}

void ComponentPortSetData(Port* port, PortContainer* data)
{
    if (data == NULL) {
        // Silent error
        return;
    }

    data->consumed   = FALSE;
    if (Queue_Enqueue(port->outputQ, (void*)data) == FALSE) {
        VLOG(INFO, "%s FAILURE\n", __FUNCTION__);
    }
}

PortContainer* ComponentPortPeekData(Port* port)
{
    PortContainer* c = Queue_Peek(port->inputQ);
    if (c) {
        c->packetNo = port->sequenceNo;
    }
    return c;
}

PortContainer* ComponentPortGetData(Port* port)
{
    PortContainer* c = Queue_Dequeue(port->inputQ);
    if (c) {
        port->sequenceNo++;
    }

    return c;
}

void* WaitBeforeComponentPortGetData(Port* port)
{
    BOOL  loop     = TRUE;
    void* portData = NULL;

    while (loop) {
        if (Queue_Get_Cnt(port->inputQ) > 0) {
            portData = Queue_Dequeue(port->inputQ);
            loop = FALSE;
        }
    }

    return portData;
}

void ComponentPortDestroy(Port* port)
{
    Queue_Destroy(port->inputQ);
    Queue_Destroy(port->outputQ);
    port->connectedComponent = NULL;
}

void ComponentPortWaitReadyStatus(Port* port)
{
    while (port->outputQ && Queue_Get_Cnt(port->outputQ) > 0) {
        osal_msleep(1);
    }
}

BOOL ComponentPortHasInputData(Port* port)
{
    return (Queue_Get_Cnt(port->inputQ) > 0);
}

Uint32 ComponentPortGetSize(Port* port)
{
    return port->inputQ->size;
}

void ComponentPortFlush(Component component)
{
    ComponentImpl* com = (ComponentImpl*)component;

    com->portFlush = TRUE;
}

ComponentState ComponentGetState(Component component)
{
    ComponentImpl* com = (ComponentImpl*)component;

    if (com == NULL) {
        return COMPONENT_STATE_NONE;
    }

    return com->state;
}

BOOL ComponentChangeState(Component component, Uint32 state)
{
    ComponentImpl* com = (ComponentImpl*)component;

    if (NULL != com) {
        if (COMPONENT_STATE_NONE < state && COMPONENT_STATE_MAX > state) {
            com->state = (ComponentState)state;
            return TRUE;
        }
    }

    return FALSE;
}

/* return TRUE  - Go next step
 *        FALSE - Retry
 */
BOOL ComponentParamReturnTest(CNMComponentParamRet ret, BOOL* success)
{
    BOOL retry = TRUE;

    switch (ret) {
    case CNM_COMPONENT_PARAM_FAILURE:    retry = FALSE; *success = FALSE; break;
    case CNM_COMPONENT_PARAM_SUCCESS:    retry = TRUE;  *success = TRUE;  break;
    case CNM_COMPONENT_PARAM_NOT_READY:  retry = FALSE; *success = TRUE;  break;
    case CNM_COMPONENT_PARAM_NOT_FOUND:  retry = FALSE; *success = FALSE; break;
    case CNM_COMPONENT_PARAM_TERMINATED: retry = FALSE; *success = TRUE;  break;
    default:                             retry = FALSE; *success = FALSE; break;
    }

    return retry;
}

