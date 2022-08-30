// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "stf_log.h"

#define ANSI_COLOR_ERR      "\x1b[31m"       // RED
#define ANSI_COLOR_TRACE    "\x1b[32m"       // GREEN
#define ANSI_COLOR_WARN     "\x1b[33m"       // YELLOW
#define ANSI_COLOR_BLUE     "\x1b[34m"       // BLUE
#define ANSI_COLOR_INFO     ""
// For future
#define ANSI_COLOR_MAGENTA  "\x1b[35m"       // MAGENTA
#define ANSI_COLOR_CYAN     "\x1b[36m"       // CYAN
#define ANSI_COLOR_RESET    "\x1b[0m"        // RESET

#define MAX_PRINT_LENGTH        512
#define STF_LOG_FILE_PATH       "./STF_ErrorLog.txt"
#define STF_LOG_ENV_VARIABLE    "V4L2_DEBUG"

// static unsigned int log_decor = LOG_HAS_TIME | LOG_HAS_FILE | LOG_HAS_COLOR;
static unsigned int log_decor = LOG_HAS_TIME | LOG_HAS_COLOR;
static FILE *fpLog  = NULL;
static int max_log_level = STF_LEVEL_ERR;

int init_log()
{
    char *strDebug_level = NULL;
    int level;

    if ((log_decor & LOG_HAS_FILE) && !fpLog) {
        fpLog = fopen(STF_LOG_FILE_PATH, "w");
    }

    strDebug_level = getenv(STF_LOG_ENV_VARIABLE);
    if (strDebug_level) {
        level = atoi(strDebug_level);
        if (level >=0) {
            max_log_level = level;
        }
    }

    return 0;
}

void deinit_log()
{
    if (fpLog) {
        fclose(fpLog);
        fpLog = NULL;
    }
}

void set_maxLogLevel(int level)
{
    max_log_level = level;
}

int get_maxLogLevel()
{
    return max_log_level;
}

void logmsg(int level, const char *format, ...)
{
    va_list ptr;
    char    logBuf[MAX_PRINT_LENGTH] = {0};
    char*   prefix = "";
    char*   postfix= "";

    if (level > max_log_level)
        return;

    if ((log_decor & LOG_HAS_COLOR)) {
        postfix = ANSI_COLOR_RESET;
        switch (level) {
        case STF_LEVEL_ERR: prefix = ANSI_COLOR_ERR "[ERROR]";   break;
        case STF_LEVEL_WARN:  prefix = ANSI_COLOR_WARN"[WARN ]";  break;
        case STF_LEVEL_INFO:  prefix = ANSI_COLOR_INFO"[INFO ]";  break;
        case STF_LEVEL_DEBUG:  prefix = ANSI_COLOR_INFO"[DEBUG]";  break;
        case STF_LEVEL_LOG:  prefix = ANSI_COLOR_INFO"[LOG  ]";  break;
        case STF_LEVEL_TRACE: prefix = ANSI_COLOR_TRACE"[TRACE]"; break;
        default:    prefix = "";               break;
        }
    }

    va_start(ptr, format);
    vsnprintf(logBuf, MAX_PRINT_LENGTH, format, ptr);
    va_end(ptr);

    fputs(prefix,  stderr);
    fputs(logBuf,  stderr);
    fputs(postfix, stderr);

    if ((log_decor & LOG_HAS_FILE) && fpLog) {
        fwrite(logBuf, strlen(logBuf), 1,fpLog);
        fflush(fpLog);
    }
}

