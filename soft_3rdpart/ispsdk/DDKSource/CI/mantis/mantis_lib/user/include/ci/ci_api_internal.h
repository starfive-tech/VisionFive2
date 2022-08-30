/**
 *******************************************************************************
 @file ci_api_internal.h

 @brief Internal definition for the user API

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __CI_API_INTERNAL_H__
#define __CI_API_INTERNAL_H__


#include "stf_types.h"
#include "stf_defs.h"

#include "ci/ci_api.h"
#include "linkedlist.h"  // NOLINT
#include "sys/sys_userio.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup INT_USR Capture Interface: Internal Components
 * @brief Internal components of driver (mostly communication between user
 * and kernel-side driver)
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the INT_API documentation module
 *---------------------------------------------------------------------------*/

/**
 * @defgroup INT_FCT Internal static functions
 * @brief Internal user-side functions that have a scope limited to their
 * file only
 */

/**
 * @brief Internal connection information
 */
typedef struct
{
    /** @brief available to the user */
    CI_CONNECTION publicConnection;

    /** @brief used for open/close/ioctl */
    SYS_FILE *fileDesc;
} INT_CONNECTION;

typedef struct
{
    /** @brief available to the user */
    CI_ION_CONNECTION publicIonConnection;

    /** @brief used for open/close/ioctl */
    SYS_FILE *fileDesc;
} INT_ION_CONNECTION;

enum INT_BUFFER_STATUS
{
    /** @brief the buffer is available to trigger a new shot */
    INT_BUFFER_AVAILABLE = 0,
    /** @brief the buffer is currently used in triggered shot */
    INT_BUFFER_PENDING,
    /** @brief the buffer has been captured and is now acquired by user */
    INT_BUFFER_ACQUIRED
};

/**
 * @brief A buffer with ID
 */
typedef struct
{
    /** @brief mapping ID - 0 is invalid */
    STF_S32 ID;
    /**
     * @brief File descriptor to ION file buffer 
     */
    STF_S32 ionFd;
    /** @brief type of buffer from @ref CI_ALLOC_BUFF */
    int type;

    /** @brief pointer to memory */
    void *memory;
    /** @brief in bytes */
    STF_U32 uiSize;
    /** @brief in @ref INT_PIPELINE::sList_buffers */
    sCell_T sCell;
    /** @brief main buffer status */
    enum INT_BUFFER_STATUS eStatus;
} INT_BUFFER;

//=== Godspeed === Add new memory/buffer type support here.
typedef struct
{
    int iIdentifier;
    STF_BOOL8 bAcquired;

    CI_SHOT publicShot;
    /** @brief associated un-scaled stream output buffer */
    INT_BUFFER *pUoOutput;
    /** @brief associated scaled stream 0 output buffer */
    INT_BUFFER *pSs0Output;
    /** @brief associated scaled stream 1 output buffer */
    INT_BUFFER *pSs1Output;
    /** @brief associated CSI dump output buffer */
    INT_BUFFER *pDumpOutput;
    /** @brief associated tiling 1 read input buffer */
    INT_BUFFER *pTiling_1_RdInput;
    /** @brief associated tiling 1 write output buffer */
    INT_BUFFER *pTiling_1_WrOutput;

    /**
     * @brief cell element of @ref INT_PIPELINE::sList_shots
     */
    sCell_T sCell;
} INT_SHOT;

/**
 * @brief Internal configuration object
 */
typedef struct
{
    /** @brief available to the user */
    CI_PIPELINE publicPipeline;
    /**
     * @brief contain all the memory-mapped shots (INT_SHOT*)
     */
    sLinkedList_T sList_shots;
    /** @brief list of memory mapped buffers (INT_BUFFER*) */
    sLinkedList_T sList_buffers;

    /** @brief identifier given by kernel module */
    int iIdentifier;
    /** @brief store capture started state from kernel side */
    STF_BOOL8 bStarted;

    /** @brief element in INT_CONNECTION::sList_pipelines */
    sCell_T sCell;
    /** @brief parent connection */
    INT_CONNECTION *pConnection;
} INT_PIPELINE;

/** @brief extract stats config from load structure for HW 2.X */
STF_RESULT INT_CI_Revert_HW2(const CI_SHOT *pShot, CI_PIPELINE_STATS *pStats);

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * end of the INT_USR documentation module
 *---------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif


#endif /* __CI_API_INTERNAL_H__ */
