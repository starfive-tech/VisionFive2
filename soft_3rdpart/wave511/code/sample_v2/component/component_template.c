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

typedef struct {
    Uint32  dummy;
} ExampleContext;

static CNMComponentParamRet GetParameterExample(ComponentImpl* from, ComponentImpl* my, GetParameterCMD commandType, void* data) 
{
    BOOL            result  = TRUE;
    PortContainer* container;

    switch(commandType) {
    case GET_PARAM_COM_IS_CONTAINER_CONUSUMED:
        container = (PortContainer*)data;
        container->consumed = TRUE;
        break;
    default:
        result = FALSE;
        break;
    }

    if (result == TRUE) return CNM_COMPONENT_PARAM_SUCCESS;
    else                return CNM_COMPONENT_PARAM_FAILURE;
}

static CNMComponentParamRet SetParameterExample(ComponentImpl* from, ComponentImpl* my, SetParameterCMD commandType, void* data) 
{
    BOOL        result  = TRUE;

    switch(commandType) {
    default:
        result = FALSE;
        break;
    }

    if (result == TRUE) return CNM_COMPONENT_PARAM_SUCCESS;
    else                return CNM_COMPONENT_PARAM_FAILURE;
}

static BOOL PrepareExample(ComponentImpl* com, BOOL* done)
{
    *done = TRUE;
    return TRUE;
}

static BOOL ExecuteExample(ComponentImpl* com, PortContainer* in, PortContainer* out) 
{
    return TRUE;
}

static BOOL DestroyExample(ComponentImpl* com) 
{
    ExampleContext* myParam = (ExampleContext*)com->context;

    osal_free(myParam);

    return TRUE;
}

static Component CreateExample(ComponentImpl* com, CNMComponentConfig* componentParam) 
{
    com->context = (ExampleContext*)osal_malloc(sizeof(ExampleContext));
    osal_memset(com->context, 0, sizeof(ExampleContext));

    return (Component)com;
}

static void ReleaseExample(ComponentImpl* com)
{
}

ComponentImpl exampleComponentImpl = {
    "example",
    NULL,
    {0,},
    {0,},
    sizeof(PortContainer),
    5,
    CreateExample,
    GetParameterExample,
    SetParameterExample,
    PrepareExample,
    ExecuteExample,
    ReleaseExample,
    DestroyExample
};

