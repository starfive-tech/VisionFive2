/*
 * Copyright (c) 2018, Chips&Media
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
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#else

#include <stdio.h>
#include <stdlib.h>
#include "../jdi.h"
#include "jpulog.h"


#define JPU_BIT_REG_SIZE		0x300
#define JPU_BIT_REG_BASE		(0x10000000 + 0x3000)

#define JDI_DRAM_PHYSICAL_BASE	0x00
#define JDI_DRAM_PHYSICAL_SIZE	(128*1024*1024)
#define JDI_SYSTEM_ENDIAN	JDI_LITTLE_ENDIAN

typedef struct jpu_buffer_t jpudrv_buffer_t;

typedef struct jpu_buffer_pool_t
{
	jpudrv_buffer_t jdb;
	int inuse;
} jpu_buffer_pool_t;

static int s_jpu_fd;
static jpu_instance_pool_t *s_pjip;
static jpu_instance_pool_t s_jip;
static int s_task_num;
static int s_clock_state;
static jpudrv_buffer_t s_jdb_video_memory;
static jpudrv_buffer_t s_jdb_register;
static jpu_buffer_pool_t s_jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
static int s_jpu_buffer_pool_count;

static Int32 swap_endian(BYTE* data, Uint32 len, Uint32 endian);

int jdi_probe()
{
	int ret;

	ret = jdi_init();
	jdi_release();

	return ret;
}

/* @return number of task
 */
int jdi_get_task_num()
{
    if (s_jpu_fd == -1 || s_jpu_fd == 0x00) {
        return 0;
    }

    return s_task_num;
}


int jdi_init()
{
	int ret;

	if (s_jpu_fd != -1 && s_jpu_fd != 0x00)
	{
		s_task_num++;
		return 0;
	}

    s_jpu_fd = 1;

	memset((void *)&s_jpu_buffer_pool, 0x00, sizeof(jpu_buffer_pool_t)*MAX_JPU_BUFFER_POOL);
	s_jpu_buffer_pool_count = 0;

	if (!(s_pjip = jdi_get_instance_pool()))
	{
		JLOG(ERR, "[jdi] fail to create instance pool for saving context \n");
		goto ERR_JDI_INIT;
	}


	s_jdb_video_memory.phys_addr = JDI_DRAM_PHYSICAL_BASE;
	s_jdb_video_memory.size = JDI_DRAM_PHYSICAL_SIZE;

	if (!s_pjip->instance_pool_inited)
	{
		memset(&s_pjip->vmem, 0x00, sizeof(jpeg_mm_t));
		ret = jmem_init(&s_pjip->vmem, (unsigned long)s_jdb_video_memory.phys_addr, s_jdb_video_memory.size);
		if (ret < 0)
		{
			JLOG(ERR, "[JDI] fail to init jpu memory management logic\n");
			goto ERR_JDI_INIT;
		}
	}

	s_jdb_register.phys_addr = JPU_BIT_REG_BASE;
	s_jdb_register.virt_addr = JPU_BIT_REG_BASE;
	s_jdb_register.size = JPU_BIT_REG_SIZE;


	jdi_set_clock_gate(1);

	s_task_num++;

	JLOG(INFO, "[jdi] success to init driver \n");
	return s_jpu_fd;

ERR_JDI_INIT:

	jdi_release();
	return -1;
}

int jdi_release()
{

	if (s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return 0;

	if (jdi_lock() < 0)
	{
		JLOG(ERR, "[jdi] fail to handle lock function\n");
		return -1;
	}

	if (s_task_num > 1) // means that the opened instance remains
	{
		s_task_num--;
		jdi_unlock();
		return 0;
	}

	s_task_num--;

	memset(&s_jdb_register, 0x00, sizeof(jpudrv_buffer_t));

	if (s_jpu_fd != -1 && s_jpu_fd != 0x00)
		s_jpu_fd = -1;

	s_pjip = NULL;

	jdi_unlock();


	return 0;
}


jpu_instance_pool_t *jdi_get_instance_pool()
{
	if (!s_pjip)
	{
		s_pjip = &s_jip;

		memset(s_pjip, 0x00, sizeof(jpu_instance_pool_t));
	}

	return (jpu_instance_pool_t *)s_pjip;
}

int jdi_open_instance(unsigned long instIdx)
{
	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	s_pjip->jpu_instance_num++;

	return 0;
}

int jdi_close_instance(unsigned long instIdx)
{
	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	s_pjip->jpu_instance_num--;

	return 0;
}


int jdi_get_instance_num()
{
	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	return s_pjip->jpu_instance_num;
}

int jdi_hw_reset()
{
	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	// to do any action for hw reset

	return 0;
}



int jdi_lock()
{

	return 0;
}
void jdi_unlock()
{

}

void jdi_write_register(unsigned long addr, unsigned int data)
{
	unsigned long *reg_addr;

	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return;

	reg_addr = (unsigned long *)(addr + (unsigned long)s_jdb_register.virt_addr);
	//*(volatile unsigned long *)reg_addr = data;
	*(volatile unsigned int *)reg_addr = data;

}

unsigned long jdi_read_register(unsigned long addr)
{
	unsigned long *reg_addr;

	reg_addr = (unsigned long *)(addr + (unsigned long)s_jdb_register.virt_addr);
	//return *(volatile unsigned long *)reg_addr;
	return *(volatile unsigned int *)reg_addr;
}

int jdi_write_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
	jpudrv_buffer_t jdb = {0, };
	unsigned long offset;
	int i;


	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 1)
		{
			jdb = s_jpu_buffer_pool[i].jdb;
			if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
				break;
		}
	}

	if (!jdb.size) {
		JLOG(ERR, "address 0x%08x is not mapped address!!!\n", addr);
		return -1;
	}

	offset = addr - (unsigned long)jdb.phys_addr;


	swap_endian(data, len, endian);

	memcpy((void *)((unsigned long)jdb.virt_addr+offset), data, len);

	return len;
}

int jdi_read_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
	jpudrv_buffer_t jdb = {0};
	unsigned long offset;
	int i;

	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 1)
		{
			jdb = s_jpu_buffer_pool[i].jdb;
			if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
				break;
		}
	}

	if (!jdb.size)
		return -1;


	offset = addr - (unsigned long)jdb.phys_addr;



	memcpy(data, (const void *)((unsigned long)jdb.virt_addr+offset), len);
	swap_endian(data, (Uint32)len,  (Uint32)endian);

	return len;
}

int jdi_allocate_dma_memory(jpu_buffer_t *vb)
{
	int i;
	unsigned long offset;
	jpudrv_buffer_t jdb = {0, };

	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return -1;

	jdb.size = vb->size;
	jdb.phys_addr = (unsigned long)jmem_alloc(&s_pjip->vmem, jdb.size, 0);

	if (jdb.phys_addr == (unsigned long)-1)
		return -1; // not enough memory


	offset = (unsigned long)(jdb.phys_addr - s_jdb_video_memory.phys_addr);
	jdb.base = (unsigned long )s_jdb_video_memory.base + offset;
	jdb.virt_addr = jdb.phys_addr;

	vb->phys_addr = (unsigned long)jdb.phys_addr;
	vb->base = (unsigned long)jdb.base;
	vb->virt_addr = (unsigned long)vb->phys_addr;


	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].inuse == 0)
		{
			s_jpu_buffer_pool[i].jdb = jdb;
			s_jpu_buffer_pool_count++;
			s_jpu_buffer_pool[i].inuse = 1;
			break;
		}
	}

	return 0;
}

void jdi_free_dma_memory(jpu_buffer_t *vb)
{
	int i;
	jpudrv_buffer_t jdb = {0, };


	if(!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
		return ;

	if (vb->size == 0)
		return ;



	for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
	{
		if (s_jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr)
		{
			s_jpu_buffer_pool[i].inuse = 0;
			s_jpu_buffer_pool_count--;
			jdb = s_jpu_buffer_pool[i].jdb;
			break;
		}
	}

	if (!jdb.size)
	{
		JLOG(ERR, "[JDI] invalid buffer to free address = 0x%x\n", (int)jdb.virt_addr);
		return ;
	}

	jmem_free(&s_pjip->vmem, (unsigned long)jdb.phys_addr, 0);
	memset(vb, 0, sizeof(jpu_buffer_t));
}


int jdi_set_clock_gate(int enable)
{
	s_clock_state = enable;
    return 0;
}


int jdi_get_clock_gate()
{

	return s_clock_state;
}

int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag)
{
	unsigned int data_flag_reg;

	JLOG(ERR, "<%s:%d> Need to implement counter\n", __FUNCTION__, __LINE__);

	while(1)
	{
		data_flag_reg = jdi_read_register(addr_flag_reg);

		if (((data_flag_reg >> 4)&0xf) == flag) {
			break;
		}

		//if (timeout > 0 && (cur - elapse) > timeout) {
		//	JLOG(ERR, "[jDI] jdi_wait_inst_ctrl_busy timeout, 0x%x=0x%lx\n", addr_flag_reg, jdi_read_register(addr_flag_reg));
		//	return -1;
		//}
	}
	return 0;
}

int jdi_wait_interrupt(int timeout, unsigned int addr_int_reason, unsigned long instIdx)
{
	//int count = 0;

	while(1)
	{
		if (jdi_read_register(addr_int_reason))
			break;

		//Sleep(1);	// 1ms sec
		//if (count++ > timeout)
		//	return -1;
	}

	return 0;
}

void jdi_log(int cmd, int step, int inst)
{
	int i;

	switch(cmd)
	{
	case JDI_LOG_CMD_PICRUN:
		if (step == 1)	//
			JLOG(INFO, "\n**PIC_RUN start\n");
		else
			JLOG(INFO, "\n**PIC_RUN end \n");
		break;
	}

	for (i=0; i<=0x238; i=i+16)
	{
		JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
			jdi_read_register(i), jdi_read_register(i+4),
			jdi_read_register(i+8), jdi_read_register(i+0xc));
	}
}

static void SwapByte(Uint8* data, Uint32 len)
{
    Uint8   temp;
    Uint32  i;

    for (i=0; i<len; i+=2) {
        temp      = data[i];
        data[i]   = data[i+1];
        data[i+1] = temp;
    }
}

static void SwapWord(Uint8* data, Uint32 len)
{
    Uint16  temp;
    Uint16* ptr = (Uint16*)data;
    Int32   i, size = len/sizeof(Uint16);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

static void SwapDword(Uint8* data, Uint32 len)
{
    Uint32  temp;
    Uint32* ptr = (Uint32*)data;
    Int32   i, size = len/sizeof(Uint32);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}


static Int32 swap_endian(BYTE* data, Uint32 len, Uint32 endian)
{
    Uint8   endianMask[8] = {   // endianMask : [2] - 4byte unit swap
        0x00, 0x07, 0x04, 0x03, //              [1] - 2byte unit swap
        0x06, 0x05, 0x02, 0x01  //              [0] - 1byte unit swap
    };
    Uint8   targetEndian;
    Uint8   systemEndian;
    Uint8   changes;
    BOOL    byteSwap=FALSE, wordSwap=FALSE, dwordSwap=FALSE;

    if (endian > 7) {
        JLOG(ERR, "Invalid endian mode: %d, expected value: 0~7\n", endian);
        return -1;
    }

    targetEndian = endianMask[endian];
    systemEndian = endianMask[JDI_SYSTEM_ENDIAN];
    changes      = targetEndian ^ systemEndian;
    byteSwap     = changes & 0x01 ? TRUE : FALSE;
    wordSwap     = changes & 0x02 ? TRUE : FALSE;
    dwordSwap    = changes & 0x04 ? TRUE : FALSE;

    if (byteSwap == TRUE)  SwapByte(data, len);
    if (wordSwap == TRUE)  SwapWord(data, len);
    if (dwordSwap == TRUE) SwapDword(data, len);

    return changes == 0 ? 0 : 1;
}



int kbhit(void)
{
	return 0;
}
int  getch(void)
{
	return -1;
}


/*
* newlib_stubs.c
* the bellow code is just to build ref-code. customers will removed the bellow code because they need a library which is related to the system library such as newlibc
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

int _open(int file) {
		return -1;
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

static char* __get_MSP(void)
{
	JLOG(ERR, "<%s:%d> Need to implement it\n", __FUNCTION__, __LINE__);
	return NULL;
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

	JLOG(ERR, "<%s:%d> check _ebss to implement it\n", __FUNCTION__, __LINE__);
	if (heap_end == 0) {
		heap_end = &_ebss;
	}
	prev_heap_end = heap_end;

	char * stack = (char*) __get_MSP();
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
			char c=0;
#if   STDIN_USART == 1
			while ((USART1->SR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
			c = (char)(USART1->DR & (uint16_t)0x01FF);
#elif STDIN_USART == 2
			while ((USART2->SR & USART_FLAG_RXNE) == (uint16_t) RESET) {}
			c = (char) (USART2->DR & (uint16_t) 0x01FF);
#elif STDIN_USART == 3
			while ((USART3->SR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
			c = (char)(USART3->DR & (uint16_t)0x01FF);
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
			while ((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
			USART1->DR = (*ptr++ & (uint16_t)0x01FF);
#elif  STDOUT_USART == 2
			while ((USART2->SR & USART_FLAG_TC) == (uint16_t) RESET) {
			}
			USART2->DR = (*ptr++ & (uint16_t) 0x01FF);
#elif  STDOUT_USART == 3
			while ((USART3->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
			USART3->DR = (*ptr++ & (uint16_t)0x01FF);
#endif
		}
		break;
	case STDERR_FILENO: /* stderr */
		for (n = 0; n < len; n++) {
#if STDERR_USART == 1
			while ((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
			USART1->DR = (*ptr++ & (uint16_t)0x01FF);
#elif  STDERR_USART == 2
			while ((USART2->SR & USART_FLAG_TC) == (uint16_t) RESET) {
			}
			USART2->DR = (*ptr++ & (uint16_t) 0x01FF);
#elif  STDERR_USART == 3
			while ((USART3->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
			USART3->DR = (*ptr++ & (uint16_t)0x01FF);
#endif
		}
		break;
	default:
		errno = EBADF;
		return -1;
	}
	return len;
}

int _gettimeofday( struct timeval *tv, void *tzvp )
{
    Uint64 t = 0;//__your_system_time_function_here__();  // get uptime in nanoseconds
    tv->tv_sec = t / 1000000000;  // convert to seconds
    tv->tv_usec = ( t % 1000000000 ) / 1000;  // get remaining microseconds

    JLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return 0;
}


#endif
