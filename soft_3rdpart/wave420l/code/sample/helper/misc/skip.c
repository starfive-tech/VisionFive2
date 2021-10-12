//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#include <assert.h>
#include "main_helper.h"
#include "skip.h"

#define CONTINUE_VALUE      0x7fffffff

typedef struct Md5Node {
    char    md5[12];
} Md5Node;

#define MAX_SKIP_NUM_LENGTH     10
/*! @return     positive value
 *              -1 - parsing error
 */
static Int32 ParsePositiveNumber(
    const char* str,
    Int32*      consumedBytes
    )
{
    Int32       i = 0, n;
    Int32       val = -1;
    char        ch;
    char        buf[16];
    BOOL        start = FALSE;
    BOOL        continueChar = FALSE;

    n = 0;
    while ((ch=str[i++]) !=0) {
        if (ch == '[') {
            start = TRUE;
        }
        else if (ch == ']') {
            buf[n++] = 0;
            if (continueChar == TRUE) {
                val = CONTINUE_VALUE;
            }
            else {
                val = atoi(buf);
            }
            break;
        }
        else {
            if (start != TRUE) {
                break;
            }
            if ('0' <= ch && ch <= '9') {
                buf[n++] = ch;
            }
            else if (ch == '-') {
                // continue character
                continueChar = TRUE;
            }
            else {
                VLOG(ERR, "%s:%d Invalid number: %c\n", __FUNCTION__, __LINE__, ch);
                break;
            }
        }
    }

    *consumedBytes = i;

    return val;
}

Queue* BuildSkipCommandQueue(
    const char* skipCmd,
    Uint32      nFrames
    )
{
    Uint32          i = 0, j;
    Uint32          cmd, nCmds;
    Int32           num, consumed;
    char            ch;
    BOOL            success = TRUE;
    Queue*          Q;
    SkipCmd         skip;

    // Parse skip command
    // Pattern: (n)D(n), FOR CODA
    //          R(n)N(n)D(n)C(n)T(n), FOR WAVE
    Q = Queue_Create(nFrames, sizeof(SkipCmd));
    nCmds = 0;
    while (nCmds < nFrames) {
        ch=skipCmd[i++];
        if (ch == '\0') {
            // repeat pattern
            i = 0;
            continue;
        }
        switch (ch) {
        case 'D':
        case 'd':
            cmd = SKIP_CMD_DECODE;
            break;
        case 'R':
        case 'r':
            cmd = SKIP_CMD_NON_IRAP;
            break;
        case 'N':
        case 'n':
            cmd = SKIP_CMD_NON_REF;
            break;
        case 'T':
        case 't':
            cmd = SKIP_CMD_TARGET_SUBLAYER;
            break;
        default:
            cmd = SKIP_CMD_NONE;
            break;
        }

        if (cmd == SKIP_CMD_NONE) {
            VLOG(ERR, "Unknown skip command: %c\n", ch);
            success = FALSE;
            break;
        }

        if ((num=ParsePositiveNumber(&skipCmd[i], &consumed)) < 0) {
            success = FALSE;
            break;
        }
        if (num == CONTINUE_VALUE) {
            num = nFrames - nCmds;
        }

        skip.cmd = cmd;
        if (cmd == SKIP_CMD_TARGET_SUBLAYER) {
            skip.tid = num; // Target sublayer id
            Queue_Enqueue(Q, (void*)&skip);
            nCmds++;
        }
        else {
            skip.tid = H265_MAX_TEMPORAL_ID;
            for (j=nCmds; j<nCmds+num && j<nFrames; j++) {
                Queue_Enqueue(Q, (void*)&skip);
            }
            nCmds += num;
        }

        i += consumed;
    }

    // consumed all bytes

    if (success == FALSE) {
        Queue_Destroy(Q);
        Q = NULL;
    }

#ifdef DEBUG_SKIP_CMD
    {
    SkipCmd* val;
    char*    cmdString[] = { "DECODE", "NON-IRAP", "NON-REF", NULL };

    for (i=0; (val=Queue_Dequeue(Q)) != NULL; i++) {
        Uint32 cmdIndex = val->cmd;
        VLOG(INFO, "COMMAND[%-4d]: %s\n", i, cmdString[cmdIndex]);
    }
    }
#endif /* DEBUG_SKIP_CMD */

    return Q;
}

