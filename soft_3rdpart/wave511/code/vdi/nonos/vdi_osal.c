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

void LogMsg(int level, const char *format, ...)
{
    va_list ptr;
    char logBuf[MAX_PRINT_LENGTH] = {0};

    va_start( ptr, format );

    vsnprintf( logBuf, MAX_PRINT_LENGTH, format, ptr );

    va_end(ptr);

    printf(logBuf);

}


void osal_init_keyboard()
{

}

void osal_close_keyboard()
{

}

void * osal_memcpy(void * dst, const void * src, int count)
{
    return memcpy(dst, src, count);//lint !e670
}

int osal_memcmp(const void* src, const void* dst, int size)
{
    return memcmp(src, dst, size);
}

void * osal_memset(void *dst, int val, int count)
{
    return memset(dst, val, count);
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
    free(p);//lint -e{424}
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
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return 0;
}
int osal_getch(void)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return -1;
}

int osal_flush_ch(void)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return -1;
}

void osal_msleep(Uint32 millisecond)
{
    Uint32 countDown = millisecond;
    while (countDown > 0) countDown--;
    VLOG(WARN, "<%s:%d> Please implemenet osal_msleep()\n", __FUNCTION__, __LINE__);
}

osal_thread_t osal_thread_create(void(*start_routine)(void*), void*arg)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return NULL;
}

Int32 osal_thread_join(osal_thread_t thread, void** retval)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return -1;
}

Int32 osal_thread_timedjoin(osal_thread_t thread, void** retval, Uint32 millisecond)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return 2;   /* error */
}

osal_mutex_t osal_mutex_create(void)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return NULL;
}

void osal_mutex_destroy(osal_mutex_t mutex)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
}

BOOL osal_mutex_lock(osal_mutex_t mutex)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return FALSE;
}

BOOL osal_mutex_unlock(osal_mutex_t mutex)
{
    return FALSE;
}

osal_sem_t osal_sem_init(Uint32 count)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return NULL;
}

void osal_sem_destroy(osal_sem_t sem)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
}

void osal_sem_wait(osal_sem_t sem)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
}

void osal_sem_post(osal_sem_t sem)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
}

int _gettimeofday( struct timeval *tv, void *tzvp )
{
    Uint64 t = 0;//__your_system_time_function_here__();  // get uptime in nanoseconds
    tv->tv_sec = t / 1000000000;  // convert to seconds
    tv->tv_usec = ( t % 1000000000 ) / 1000;  // get remaining microseconds

    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return 0;
}

Uint64 osal_gettime(void)
{
    VLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return 0ULL;
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
    char * stack = 0;
    if (heap_end + incr >  stack)//lint !e413
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
            char c =0;
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

