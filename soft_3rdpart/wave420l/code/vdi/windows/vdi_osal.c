//------------------------------------------------------------------------------
// File: vdi_osal.c
//
// Copyright (c) 2012, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)

#include <windows.h>

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "vputypes.h"
#include "vdi_osal.h"

static int log_colors[MAX_LOG_LEVEL] = { 
	0,
	TERM_COLOR_R|TERM_COLOR_G|TERM_COLOR_B|TERM_COLOR_BRIGHT, 	//INFO
	TERM_COLOR_R|TERM_COLOR_B|TERM_COLOR_BRIGHT,	//WARN
	TERM_COLOR_R|TERM_COLOR_BRIGHT,	// ERR
	TERM_COLOR_R|TERM_COLOR_G|TERM_COLOR_B	//TRACE
};

static unsigned log_decor = LOG_HAS_TIME | LOG_HAS_FILE | LOG_HAS_MICRO_SEC |
			    LOG_HAS_NEWLINE |
			    LOG_HAS_SPACE | LOG_HAS_COLOR;
static int max_log_level = MAX_LOG_LEVEL;		
static FILE *fpLog  = NULL;

#define OSAL_MUTEX_TIMEOUT 10000 //ms

static void term_restore_color(void);
static void term_set_color(int color);

int InitLog(
    void
    ) 
{
    fpLog = osal_fopen("ErrorLog.txt", "w");

	return 1;	
}

void DeInitLog(void)
{
	if (fpLog)
	{
		osal_fclose(fpLog);
		fpLog = NULL;
	}
}

void SetLogColor(int level, int color)
{
	log_colors[level] = color;
}

int GetLogColor(int level)
{
	return log_colors[level];
}

void SetLogDecor(int decor)
{
	log_decor = decor;
}

int GetLogDecor(void)
{
	return log_decor;
}

void SetMaxLogLevel(int level)
{
	max_log_level = level;
}
int GetMaxLogLevel(void)
{
	return max_log_level;
}

void LogMsg(int level, const char *format, ...)
{
    char logBuf[MAX_PRINT_LENGTH] = {0};

    if (level > max_log_level) {
        return;
    }
    else {
        /*lint -save -e438 */
        va_list ptr = NULL;
        va_start( ptr, format );	
        _vsnprintf( logBuf, MAX_PRINT_LENGTH, format, ptr );	
        va_end(ptr);
        /*lint -restore */
    }

    if (log_decor & LOG_HAS_COLOR)
        term_set_color(log_colors[level]);	

    fputs(logBuf, stdout);

    if ((log_decor & LOG_HAS_FILE) && fpLog)
    {
        osal_fwrite(logBuf, strlen(logBuf), 1,fpLog);
        osal_fflush(fpLog);
    }

    if (log_decor & LOG_HAS_COLOR)
        term_restore_color();
}

static void term_set_color(int color)
{	
	unsigned short attr = 0;
	if (color & TERM_COLOR_R)
		attr |= FOREGROUND_RED;
	if (color & TERM_COLOR_G)
		attr |= FOREGROUND_GREEN;
	if (color & TERM_COLOR_B)
		attr |= FOREGROUND_BLUE;
	if (color & TERM_COLOR_BRIGHT)
		attr |= FOREGROUND_INTENSITY;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);

}

static void term_restore_color(void)
{
	term_set_color(log_colors[4]);
}

static double timer_frequency_;

#if defined(_MSC_VER)
static LARGE_INTEGER initial_;
static LARGE_INTEGER frequency_;
static LARGE_INTEGER counter_;
#endif

void timer_init(void)
{
#if defined(_MSC_VER)
    if (QueryPerformanceFrequency(&frequency_))
    {
        //printf("High:%d, Quad:%d, Low:%d\n", frequency_.HighPart, frequency_.QuadPart, frequency_.LowPart);
        timer_frequency_ = (double)((Uint64)((Uint64)frequency_.HighPart >> 32) + frequency_.LowPart); /*lint !e572 !e571 excessive shift*/
    }
    else {
        printf("QueryPerformanceFrequency returned FAIL\n");
    }
#endif
}

void timer_start(void)
{
	timer_init();
#if defined(_MSC_VER)
	if (timer_frequency_ == 0)
		return;
	
	QueryPerformanceCounter(&initial_);
#endif
}

void timer_stop(void)
{
#if defined(_MSC_VER)
	if (timer_frequency_ == 0)
		return;
	
	QueryPerformanceCounter(&counter_);
#endif
}

double timer_elapsed_ms(void)
{
	double ms;
	ms = timer_elapsed_us()/1000.0;	
	return ms;
}

double timer_elapsed_us(void)
{
	double elapsed = 0;
#if defined(_MSC_VER)
	if (timer_frequency_ == 0)
		return 0;
	
	elapsed = (double)((long double)(counter_.QuadPart - initial_.QuadPart)*1000000 / (long double)frequency_.QuadPart);
#endif
	return elapsed;

}

int timer_is_valid(void)
{
	return timer_frequency_ != 0;
}

double timer_frequency(void)
{
	return timer_frequency_;
}

void osal_init_keyboard(void)
{
	return;
}

void osal_close_keyboard(void)
{
	return;
}

void * osal_memcpy(void * dst, const void * src, int count)
{
	return memcpy(dst, src, count);
}

void * osal_memset(void *dst, int val, int count)
{
	return memset(dst, val, count);
}

int osal_memcmp(const void* src, const void* dst, int size)
{
    return memcmp(src, dst, size);
}

void * osal_malloc(int size)
{
	return malloc(size);
}

void * osal_realloc(void* ptr, int size)
{
    return realloc(ptr, size);
}

void osal_free(void *p)
{
	free(p);
}

int osal_fflush(osal_file_t fp)
{
	return fflush(fp);
}

osal_file_t osal_fopen(const char * osal_file_tname, const char * mode)
{
	return fopen(osal_file_tname, mode);
}
size_t osal_fwrite(const void * p, int size, int count, osal_file_t fp)
{
	return fwrite(p, size, count, fp);
}
size_t osal_fread(void *p, int size, int count, osal_file_t fp)
{
	return fread(p, size, count, fp);
}

long osal_ftell(osal_file_t fp)
{
	return ftell(fp);
}

int osal_fseek(osal_file_t fp, long offset, int origin)
{
	return fseek(fp, offset, origin);
}
int osal_fclose(osal_file_t fp)
{
	return fclose(fp);
}
int osal_fscanf(osal_file_t fp, const char * _Format, ...)
{
    /*lint -save -e438 */
	int ret;
	va_list arglist;
	va_start(arglist, _Format);

#ifdef MFHMFT_EXPORTS
#else
	ret = fscanf(fp, _Format, arglist);
#endif
	
	va_end(arglist);

	return ret;
    /*lint -restore */
}

int osal_fprintf(osal_file_t fp, const char * _Format, ...)
{
    /*lint -save -e438 */
	int ret;
	va_list arglist;
	va_start(arglist, _Format);

	ret = vfprintf(fp, _Format, arglist);

	va_end(arglist);

	return ret;
    /*lint -restore */
}

int osal_kbhit(void)
{
	return _kbhit();
}
int osal_getch(void)
{
	return _getch();
}
int osal_flush_ch(void)
{
	fflush(stdout);
	return 1;
}

int osal_feof(osal_file_t fp)
{
    return feof((FILE *)fp);
}

void * osal_create_mutex(const char *name)
{
	HANDLE  h_mutex;
	h_mutex = CreateMutex(NULL, FALSE, (LPCWSTR)name);
	return h_mutex;
}
void osal_close_mutex(void *handle)
{
	CloseHandle((HANDLE)handle);
	return;
}
int osal_mutex_lock(void *handle)
{
	if(!handle)
		return -1;

	if (WaitForSingleObject(handle, OSAL_MUTEX_TIMEOUT) != WAIT_OBJECT_0) {
		return -1;
	}
	return 0;
}
int osal_mutex_unlock(void *handle)
{
	if(!handle)
		return -1;

	ReleaseMutex(handle);
	return 0;
}

//------------------------------------------------------------------------------
// math related api
//------------------------------------------------------------------------------
#ifndef I64
typedef __int64 I64;
// To avoid VS9 warning code 4996, 4305
#pragma warning(disable : 4996)
#pragma warning(disable : 4305)
#endif // I64

// 32 bit / 16 bit ==> 32-n bit remainder, n bit quotient
static int fixDivRq(int a, int b, int n)
{
    I64 c;
    I64 a_36bit;
    I64 mask, signBit, signExt;
    int  i;

    // DIVS emulation for BPU accumulator size
    // For SunOS build
    mask = 0x0F; mask <<= 32; mask |= 0x00FFFFFFFF; // mask = 0x0FFFFFFFFF;
    signBit = 0x08; signBit <<= 32;                 // signBit = 0x0800000000;
    signExt = 0xFFFFFFF0; signExt <<= 32;           // signExt = 0xFFFFFFF000000000;

    a_36bit = (I64) a;

    for (i=0; i<n; i++) {
        c =  a_36bit - (b << 15);
        if (c >= 0)
            a_36bit = (c << 1) + 1;
        else
            a_36bit = a_36bit << 1;

        a_36bit = a_36bit & mask;
        if (a_36bit & signBit)
            a_36bit |= signExt;
    }

    a = (int) a_36bit;
    return a;                           // R = [31:n], Q = [n-1:0]
}

int math_div(int number, int denom)
{
    int  c;
    c = fixDivRq(number, denom, 17);             // R = [31:17], Q = [16:0]
    c = c & 0xFFFF;
    c = (c + 1) >> 1;                   // round
    return (c & 0xFFFF);
}

#endif 
