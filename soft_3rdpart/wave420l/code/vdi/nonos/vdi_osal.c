//------------------------------------------------------------------------------
// File: vdi_osal.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
//#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
//#elif defined(linux) || defined(__linux) || defined(ANDROID)
//#else

#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include "vpuconfig.h"
#include "vputypes.h"

#include "../vdi_osal.h"

#define FILE_BUFFER_BASE 0x5000000
#define FILE_BUFFER_SIZE 1024*1024*10

typedef struct fileio_buf_t {
	char *_ptr;
	int   _cnt;
	char *_base;
	int   _flag;
	int   _file;
	int   _charbuf;
	int   _bufsiz;
	char *_tmpfname;
} fileio_buf_t;

static fileio_buf_t s_file;

int InitLog() 
{
	return 1;	
}

void DeInitLog()
{
	
}

void SetMaxLogLevel(int level)
{
}
int GetMaxLogLevel(void)
{
	return -1;
}
	
void SetLogColor(int level, int color)
{
}
int GetLogColor(int level)
{
	return -1;
}
	
void SetLogDecor(int decor)
{
}
int GetLogDecor(void)
{
	return -1;
}

void LogMsg(int level, const char *format, ...)
{
	va_list ptr;
	char logBuf[MAX_PRINT_LENGTH] = {0};

	va_start( ptr, format );	

	vsnprintf( logBuf, MAX_PRINT_LENGTH, format, ptr );

	va_end(ptr);

	printf(logBuf);

}

static double timer_frequency_;

void timer_init()
{

}

void timer_start()
{
	timer_init();
}

void timer_stop()
{
	
}

double timer_elapsed_ms()
{
	double ms;
	ms = timer_elapsed_us()/1000.0;	
	return ms;
}

double timer_elapsed_us()
{
	double elapsed = 0;
	return elapsed;

}

int timer_is_valid()
{
	return timer_frequency_ != 0;
}

double timer_frequency()
{
	return timer_frequency_;
}

void osal_init_keyboard()
{
    
}

void osal_close_keyboard()
{
    
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
	return 1;
}

int osal_feof(osal_file_t fp)
{
	return 0;
}

osal_file_t osal_fopen(const char * osal_file_tname, const char * mode)
{
	s_file._base = (char *)FILE_BUFFER_BASE;
	s_file._bufsiz = FILE_BUFFER_SIZE;
	s_file._ptr = (char *)0;
	return &s_file;
}
size_t osal_fwrite(const void * p, int size, int count, osal_file_t fp)
{
	long addr = (long)((int)s_file._base+(int)s_file._ptr);
	osal_memcpy((void *)addr, (void *)p, count*size);
	s_file._ptr += count*size;

	return count*size;
}
size_t osal_fread(void *p, int size, int count, osal_file_t fp)
{
	long addr = (long)((int)s_file._base+(int)s_file._ptr);
	osal_memcpy((void *)p, (void *)addr, count*size);
	s_file._ptr += count*size;

	return count*size;
}

long osal_ftell(osal_file_t fp)
{
	return s_file._bufsiz;
}

int osal_fseek(osal_file_t fp, long offset, int origin)
{

	return offset;
}
int osal_fclose(osal_file_t fp)
{
	s_file._base = (char *)FILE_BUFFER_BASE;
	s_file._bufsiz = FILE_BUFFER_SIZE;
	s_file._ptr = (char *)0;

	return 1;
}
int osal_fscanf(osal_file_t fp, const char * _Format, ...)
{
	return 1;
}

int osal_fprintf(osal_file_t fp, const char * _Format, ...)
{
	va_list ptr;
	char logBuf[MAX_PRINT_LENGTH] = {0};

	va_start( ptr, _Format);	

	vsnprintf(logBuf, MAX_PRINT_LENGTH, _Format, ptr);

	va_end(ptr);

	printf(logBuf);

	return 1;

}

int osal_kbhit(void)
{
	return 0;
}
int osal_getch(void)
{
	return -1;
}

int osal_flush_ch(void)
{
	return -1;
}

//------------------------------------------------------------------------------
// math related api
//------------------------------------------------------------------------------
#ifndef I64
typedef long long I64;
#endif

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

#ifdef LIB_C_STUB

/*
* newlib_stubs.c 
* the bellow code is just to build ref-code. customers will removed the bellow code bacuase they need a library which is related to the system library such as newlibc
*/
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

#ifndef STDOUT_USART
#define STDOUT_USART 0
#endif

#ifndef STDERR_USART
#define STDERR_USART 0
#endif

#ifndef STDIN_USART
#define STDIN_USART 0
#endif

#undef errno
extern int errno;

/*
environ
A pointer to a list of environment variables and their values. 
For a minimal environment, this empty list is adequate:
*/
char *__env[1] = { 0 };
char **environ = __env;

int _write(int file, char *ptr, int len);

void _exit(int status) {
	_write(1, "exit", 4);
	while (1) {
		;
	}
}

int _close(int file) {
	return -1;
}
/*
execve
Transfer control to a new process. Minimal implementation (for a system without processes):
*/
int _execve(char *name, char **argv, char **env) {
	errno = ENOMEM;
	return -1;
}
/*
fork
Create a new process. Minimal implementation (for a system without processes):
*/

int _fork() {
	errno = EAGAIN;
	return -1;
}
/*
fstat
Status of an open file. For consistency with other minimal implementations in these examples,
all files are regarded as character special devices.
The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
*/
int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

/*
getpid
Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
*/

int _getpid() {
	return 1;
}

/*
isatty
Query whether output stream is a terminal. For consistency with the other minimal implementations,
*/
int _isatty(int file) {
	switch (file){
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case STDIN_FILENO:
		return 1;
	default:
		//errno = ENOTTY;
		errno = EBADF;
		return 0;
	}
}

/*
kill
Send a signal. Minimal implementation:
*/
int _kill(int pid, int sig) {
	errno = EINVAL;
	return (-1);
}

/*
link
Establish a new name for an existing file. Minimal implementation:
*/

int _link(char *old, char *new) {
	errno = EMLINK;
	return -1;
}

/*
lseek
Set position in a file. Minimal implementation:
*/
int _lseek(int file, int ptr, int dir) {
	return 0;
}

/*
sbrk
Increase program data space.
Malloc and related functions depend on this
*/
caddr_t _sbrk(int incr) {

	// extern char _ebss; // Defined by the linker
	char _ebss;
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_ebss;
	}
	prev_heap_end = heap_end;

	//char * stack = (char*) __get_MSP();
	char * stack;
	if (heap_end + incr >  stack)
	{
		_write (STDERR_FILENO, "Heap and stack collision\n", 25);
		errno = ENOMEM;
		return  (caddr_t) -1;
		//abort ();
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;

}

/*
read
Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
Returns -1 on error or blocks until the number of characters have been read.
*/

int _read(int file, char *ptr, int len) {
	int n;
	int num = 0;
	switch (file) {
	case STDIN_FILENO:
		for (n = 0; n < len; n++) {
			char c;
#if   STDIN_USART == 1
			while ((USART1->SR & USART_FLAG_RXNE) == (Uint16)RESET) {}
			c = (char)(USART1->DR & (Uint16)0x01FF);
#elif STDIN_USART == 2
			while ((USART2->SR & USART_FLAG_RXNE) == (Uint16) RESET) {}
			c = (char) (USART2->DR & (Uint16) 0x01FF);
#elif STDIN_USART == 3
			while ((USART3->SR & USART_FLAG_RXNE) == (Uint16)RESET) {}
			c = (char)(USART3->DR & (Uint16)0x01FF);
#endif
			*ptr++ = c;
			num++;
		}
		break;
	default:
		errno = EBADF;
		return -1;
	}
	return num;
}

/*
stat
Status of a file (by name). Minimal implementation:
int    _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
*/

int _stat(const char *filepath, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

/*
times
Timing information for current process. Minimal implementation:
*/

clock_t _times(struct tms *buf) {
	return -1;
}

/*
unlink
Remove a file's directory entry. Minimal implementation:
*/
int _unlink(char *name) {
	errno = ENOENT;
	return -1;
}

/*
wait
Wait for a child process. Minimal implementation:
*/
int _wait(int *status) {
	errno = ECHILD;
	return -1;
}

/*
write
Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
Returns -1 on error or number of bytes sent
*/
int _write(int file, char *ptr, int len) {
	int n;
	switch (file) {
	case STDOUT_FILENO: /*stdout*/
		for (n = 0; n < len; n++) {
#if STDOUT_USART == 1
			while ((USART1->SR & USART_FLAG_TC) == (Uint16)RESET) {}
			USART1->DR = (*ptr++ & (Uint16)0x01FF);
#elif  STDOUT_USART == 2
			while ((USART2->SR & USART_FLAG_TC) == (Uint16) RESET) {
			}
			USART2->DR = (*ptr++ & (Uint16) 0x01FF);
#elif  STDOUT_USART == 3
			while ((USART3->SR & USART_FLAG_TC) == (Uint16)RESET) {}
			USART3->DR = (*ptr++ & (Uint16)0x01FF);
#endif
		}
		break;
	case STDERR_FILENO: /* stderr */
		for (n = 0; n < len; n++) {
#if STDERR_USART == 1
			while ((USART1->SR & USART_FLAG_TC) == (Uint16)RESET) {}
			USART1->DR = (*ptr++ & (Uint16)0x01FF);
#elif  STDERR_USART == 2
			while ((USART2->SR & USART_FLAG_TC) == (Uint16) RESET) {
			}
			USART2->DR = (*ptr++ & (Uint16) 0x01FF);
#elif  STDERR_USART == 3
			while ((USART3->SR & USART_FLAG_TC) == (Uint16)RESET) {}
			USART3->DR = (*ptr++ & (Uint16)0x01FF);
#endif
		}
		break;
	default:
		errno = EBADF;
		return -1;
	}
	return len;
}

#endif
//#endif

