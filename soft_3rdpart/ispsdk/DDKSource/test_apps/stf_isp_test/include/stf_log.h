// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef __STF_LOG_H__
#define __STF_LOG_H__

// Log Debug
typedef enum {
  STF_LEVEL_NONE = 0,
  STF_LEVEL_ERR,
  STF_LEVEL_WARN,
  STF_LEVEL_INFO,
  STF_LEVEL_DEBUG,
  STF_LEVEL_LOG,
  STF_LEVEL_TRACE,
  STF_LEVEL_ALL
} StfDebugLevelType;

enum {
    LOG_HAS_DAY_NAME   =    1, /**< Include day name [default: no]         */
    LOG_HAS_YEAR       =    2, /**< Include year digit [no]                */
    LOG_HAS_MONTH      =    4, /**< Include month [no]                     */
    LOG_HAS_DAY_OF_MON =    8, /**< Include day of month [no]              */
    LOG_HAS_TIME       =   16, /**< Include time [yes]                     */
    LOG_HAS_MICRO_SEC  =   32, /**< Include microseconds [yes]             */
    LOG_HAS_FILE       =   64, /**< Include sender in the log [yes]        */
    LOG_HAS_NEWLINE    =  128, /**< Terminate each call with newline [yes] */
    LOG_HAS_CR         =  256, /**< Include carriage return [no]           */
    LOG_HAS_SPACE      =  512, /**< Include two spaces before log [yes]    */
    LOG_HAS_COLOR      = 1024, /**< Colorize logs [yes on win32]           */
    LOG_HAS_LEVEL_TEXT = 2048 /**< Include level text string [no]          */
};
enum {
    TERM_COLOR_R      = 2,   /**< Red            */
    TERM_COLOR_G      = 4,   /**< Green          */
    TERM_COLOR_B      = 1,   /**< Blue.          */
    TERM_COLOR_BRIGHT = 8    /**< Bright mask.   */
};

int init_log(void);
void deinit_log(void);
void set_maxLogLevel(int level);
int get_maxLogLevel(void);
void logmsg(int level, const char *format, ...);

#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define LOG(level, fmt, ...)  logmsg(level, "[%s,%d]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define FuncIn()  logmsg(STF_LEVEL_DEBUG, "[%s,%d]:  FUNC IN\n",  __FUNCTION__, __LINE__);
#define FuncOut() logmsg(STF_LEVEL_DEBUG, "[%s,%d]:  FUNC OUT\n",  __FUNCTION__, __LINE__);
#else
#define LOG(level, fmt, ...)
#define FuncIn()
#define FuncOut()
#endif

#endif // __STF_LOG_H__
