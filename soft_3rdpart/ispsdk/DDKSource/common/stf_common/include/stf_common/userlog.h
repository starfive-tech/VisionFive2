/**
 *******************************************************************************
  @file userlog.h

  @brief Defines functions to log information regardless of platform

  @copyright StarFive Technology Co., Ltd. All Rights Reserved.

  @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __USERLOG_H__
#define __USERLOG_H__


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
 * Following elements are in MANTIS_COMMON defined in pixel_format.h
 *---------------------------------------------------------------------------*/

#ifdef STF_KERNEL_MODULE
#error Should not be compiled as part of kernel code!
#endif //STF_KERNEL_MODULE

/**
 * @name Logging macros
 * @addtogroup MANTIS_COMMON
 * @{
 */
#ifdef __ANDROID__
// Android logging to logcat

// in ISPC, sometimes LOG_TAG is defined AFTER #include <userlog.h>
// so make life easier for everyone in any case and prevent #define LOG_TAG NULL
// to avoid redefine warnings
#ifndef LOG_TAG
#define DUMMY_LOG_TAG
#endif

#include <utils/Log.h>

#ifdef DUMMY_LOG_TAG
// if we are here then LOG_TAG=NULL defined in Log.h
#undef DUMMY_LOG_TAG
#undef LOG_TAG
#endif

#define LOG_ERROR_TAG(tag, format,...) \
    ALOG(LOG_ERROR, tag, "%s:%d: " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_WARNING_TAG(tag, format,...) \
    ALOG(LOG_WARN, tag, "%s:%d: " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_PERF_TAG(tag, format,...) \
    ALOG(LOG_WARN, tag, "%s:%d: " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO_TAG(tag, format,...) \
    ALOG(LOG_INFO, tag, "%s: " format, __FUNCTION__, ##__VA_ARGS__)
#define LOG_DEBUG_TAG(tag, format,...) \
    ALOG(LOG_VERBOSE, tag, "%s: " format, __FUNCTION__, ##__VA_ARGS__)

#else
// other builds -> fprintf() based logging

// copy-pasted from Android log.h
#ifndef LOG_NDEBUG
#ifdef NDEBUG
#define LOG_NDEBUG 1
#else
#define LOG_NDEBUG 0
#endif
#endif

#ifndef LOG_NPERF
#ifdef NDEBUG
#define LOG_NPERF 1
#else
#define LOG_NPERF 0
#endif
#endif

#define LOG_ERROR_TAG(tag, ...) \
    LOG_Error(__FUNCTION__, __LINE__, tag, __VA_ARGS__)
#define LOG_WARNING_TAG(tag, ...) \
    LOG_Warning(__FUNCTION__, __LINE__, tag, __VA_ARGS__)

#if LOG_NPERF
#define LOG_PERF_TAG(...)
#else
#define LOG_PERF_TAG(tag, ...) \
    LOG_Perf(__FUNCTION__, __LINE__, tag, __VA_ARGS__)
#endif /* LOG_NDEBUG */

#define LOG_INFO_TAG(tag, ...) \
    LOG_Info(__FUNCTION__, __LINE__, tag, __VA_ARGS__)

#if LOG_NDEBUG
#define LOG_DEBUG_TAG(...)
#else
#define LOG_DEBUG_TAG(tag, ...) \
    LOG_Debug(__FUNCTION__, __LINE__, tag, __VA_ARGS__)
#endif /* LOG_NDEBUG */

/**
 * @brief Log an erroneous message (e.g. return error in function)
 */
void LOG_Error(const char *function, unsigned int line, const char *log_tag,
    const char* format, ...);

/**
 * @brief Log an warning message (e.g. modified input parameter to have
 * correct behaviour)
 */
void LOG_Warning(const char *function, unsigned int line, const char *log_tag,
    const char* format, ...);

/**
 * @brief Log an performance message with time sec:usec
 */
void LOG_Perf(const char *function, unsigned int line, const char *log_tag,
    const char* format, ...);

/**
 * @brief Log an information message (e.g. starting the processing of a new
 * frame)
 */
void LOG_Info(const char *function, unsigned int line, const char *log_tag,
    const char* format, ...);

/**
 * @brief Log a debug information (e.g. wrong parameter to a function)
 */
void LOG_Debug(const char *function, unsigned int line, const char *log_tag,
    const char* format, ...);

#endif /* __ANDROID__ */

/* macros used throughout the libraries */
#define LOG_ERROR(...)    LOG_ERROR_TAG(LOG_TAG, __VA_ARGS__)
#define LOG_WARNING(...)  LOG_WARNING_TAG(LOG_TAG, __VA_ARGS__)
#define LOG_INFO(...)     LOG_INFO_TAG(LOG_TAG, __VA_ARGS__)
#define LOG_DEBUG(...)    LOG_DEBUG_TAG(LOG_TAG, __VA_ARGS__)
#define LOG_PERF(...)     LOG_PERF_TAG(LOG_TAG, __VA_ARGS__)

/**
 * @}
 */
/**
 * @}
 */

/*-------------------------------------------------------------------------
 * End of the MANTIS_COMMON logging macro documentation module
 *------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif


#endif // __USERLOG_H__
