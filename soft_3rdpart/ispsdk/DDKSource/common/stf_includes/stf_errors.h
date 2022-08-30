/**
 ******************************************************************************
 @file           stf_errors.h

 @brief           Error codes used accross projects

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license        <Strictly Confidential.>
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/

#ifndef __STF_ERRORS_H__
#define __STF_ERRORS_H__


#if defined (__cplusplus)
extern "C" {
#endif


#define STF_SUCCESS                             (0)     /**< @brief Success. */
#define STF_ERROR_FAILURE                       (1)     /**< @brief Failure. */
#define STF_ERROR_TIMEOUT                       (2)     /**< @brief Timeout. */
#define STF_ERROR_MALLOC_FAILED                 (3)     /**< @brief memory allocation failed. */
#define STF_ERROR_FATAL                         (4)     /**< @brief Unspecified fatal error. */
#define STF_ERROR_OUT_OF_MEMORY                 (5)     /**< @brief Memory allocation failed. */
#define STF_ERROR_DEVICE_NOT_FOUND              (6)     /**< @brief Device is not found. */
#define STF_ERROR_DEVICE_UNAVAILABLE            (7)     /**< @brief Device is not available/in use. */
#define STF_ERROR_GENERIC_FAILURE               (8)     /**< @brief Generic/unspecified failure. */
#define STF_ERROR_INTERRUPTED                   (9)     /**< @brief Operation was interrupted - retry. */
#define STF_ERROR_INVALID_ID                    (10)    /**< @brief Invalid id. */
#define STF_ERROR_SIGNATURE_INCORRECT           (11)    /**< @brief A signature value was found to be incorrect. */
#define STF_ERROR_INVALID_PARAMETERS            (12)    /**< @brief The provided parameters were inconsistent/incorrect. */
#define STF_ERROR_STORAGE_TYPE_EMPTY            (13)    /**< @brief A list/pool has run dry. */
#define STF_ERROR_STORAGE_TYPE_FULL             (14)    /**< @brief A list is full. */
#define STF_ERROR_ALREADY_COMPLETE              (15)    /**< @brief Something has already occurred which the code thinks has not. */
#define STF_ERROR_UNEXPECTED_STATE              (16)    /**< @brief A state machine is in an unexpected/illegal state. */
#define STF_ERROR_COULD_NOT_OBTAIN_RESOURCE     (17)    /**< @brief A required resource could not be created/locked. */
#define STF_ERROR_NOT_INITIALISED               (18)    /**< @brief An attempt to access a structure/resource was made before it was initialised. */
#define STF_ERROR_ALREADY_INITIALISED           (19)    /**< @brief An attempt to initialise a structure/resource was made when it has already been initialised. */
#define STF_ERROR_VALUE_OUT_OF_RANGE            (20)    /**< @brief A provided value exceeded stated bounds. */
#define STF_ERROR_CANCELLED                     (21)    /**< @brief The operation has been cancelled. */
#define STF_ERROR_MINIMUM_LIMIT_NOT_MET         (22)    /**< @brief A specified minimum has not been met. */
#define STF_ERROR_NOT_SUPPORTED                 (23)    /**< @brief The requested feature or mode is not supported. */
#define STF_ERROR_IDLE                          (24)    /**< @brief A device or process was idle. */
#define STF_ERROR_BUSY                          (25)    /**< @brief A device or process was busy. */
#define STF_ERROR_DISABLED                      (26)    /**< @brief The device or resource has been disabled. */
#define STF_ERROR_OPERATION_PROHIBITED          (27)    /**< @brief The requested operation is not permitted at this time. */
#define STF_ERROR_MMU_PAGE_DIRECTORY_FAULT      (28)    /**< @brief The entry read from the MMU page directory is invalid. */
#define STF_ERROR_MMU_PAGE_TABLE_FAULT          (29)    /**< @brief The entry read from an MMU page table is invalid. */
#define STF_ERROR_MMU_PAGE_CATALOGUE_FAULT      (30)    /**< @brief The entry read from an MMU page catalogue is invalid. */
#define STF_ERROR_MEMORY_IN_USE                 (31)    /**< @brief Memory can not be freed as it is still been used. */
#define STF_ERROR_TEST_MISMATCH                 (32)    /**< @brief A mismatch has unexpectedly occured in data. */
#define STF_ERROR_ABORT                         (33)    /**< @brief Operation aborted. */
#define STF_ERROR_ACCESS_DENIED                 (34)    /**< @brief General access denied error. */
#define STF_ERROR_FAIL                          (35)     /**< @brief Unspecified failure. */
#define STF_ERROR_HANDLE                        (36)    /**< @brief Handle that is not valid. */
#define STF_ERROR_INVALID_ARG                   (37)    /**< @brief One or more arguments are not valid. */
#define STF_ERROR_NO_INTERFACE                  (38)    /**< @brief No such interface supported. */
#define STF_ERROR_NOT_IMPLEMENTED               (39)    /**< @brief Not implemented. */
//#define STF_ERROR_OUT_OF_MEMORY                 (40)     /**< @brief Failed to allocate necessary memory. */
#define STF_ERROR_POINTER                       (41)    /**< @brief Pointer that is not valid. */
#define STF_ERROR_AGAIN                         (42)    /**< @brief Pointer that is not available now, and please try again. */
#define STF_ERROR_UNEXPECTED                    (43)    /**< @brief Unexpected failure. */


#define STF_NO_ERRORS (43)

const char * STF_STR_ERROR(int ierr);


#if defined (__cplusplus)
}
#endif


#endif /* __STF_ERRORS_H__  */
