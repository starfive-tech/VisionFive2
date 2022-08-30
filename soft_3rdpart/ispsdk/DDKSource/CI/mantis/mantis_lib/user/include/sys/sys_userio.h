/**
 *******************************************************************************
  @file sys_userio.h

  @brief

  @copyright StarFive Technology Co., Ltd. All Rights Reserved.

  @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __SYS_USERIO_H__
#define __SYS_USERIO_H__


#include "stf_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief System file handler - it is an opaque type that is going to be 
 * different in fake device version and real device version
 */
struct __SYS_FILE;
typedef struct __SYS_FILE SYS_FILE;

// because off_t does not exists on windows
#include <sys/types.h>
#include <fcntl.h>  // open flags
#include <sys/mman.h>  // map options

/**
 * @brief Equivalent to system call open() 
 *
 * @Input pszDevName device name
 * @Input flags open flags
 * @Input extra is an <b>optional</b> pointer to struct SYS_IO_FakeOperation
 * (sys_userio_fake.h) when using fake device implementation - NULL if using
 * normal call
 *
 * @return the created SYS_FILE
 * @return NULL on failure
 */
#if defined(ENABLE_SYS_IO_OPEN_FUN_PTR)
SYS_FILE *SYS_IO_Open(const char *pszDevName, int flags, mode_t mode, void *extra);
#else
SYS_FILE *SYS_IO_Open(const char *pszDevName, int flags, void *extra);
#endif //#if defined(ENABLE_SYS_IO_OPEN_FUN_PTR)
SYS_FILE *SYS_DRM_Open(const char *pszDevName, int flags, void *extra);
SYS_FILE *SYS_VDEV_Open(const char *pszDevName, int flags, mode_t mode, void *extra);

int SYS_IO_GetFileHandle(SYS_FILE *pFile);

/**
 * @brief Equivalent to system call close()
 */
int SYS_IO_Close(SYS_FILE *pFile);
int SYS_DRM_Close(SYS_FILE *pFile);
int SYS_VDEV_Close(SYS_FILE *pFile);

/**
 * @brief Equivalent to system call ioctl()
 */
int SYS_IO_Control(SYS_FILE *pFile, unsigned int command, long int parameter);
int SYS_VDEV_Control(SYS_FILE *pFile, unsigned int command, long int parameter);

/**
 * @brief Equivalent to system call mmap2() (similar to mmap() but expects an
 * offset in pages instead of bytes)
 *
 * @return NULL on error (MAP_FAILED like mmap)
 */
void *SYS_IO_MemMap2(SYS_FILE *pFile, size_t length, int prot, int flags,
    off_t offset);
void *SYS_ION_DMA_MemMap(SYS_FILE *pFile, size_t length, int prot, int flags,
    off_t offset);
void *SYS_VDEV_MemMap(SYS_FILE *pFile, size_t length, int prot, int flags,
    off_t offset);

/**
 * @brief Equivalent to system call munmap()
 */
int SYS_IO_MemUnmap(SYS_FILE *pFile, void *addr, size_t length);
int SYS_ION_DMA_MemUnmap(SYS_FILE *pFile, void *addr, size_t length);
int SYS_VDEV_MemUnmap(SYS_FILE *pFile, void *addr, size_t length);

SYS_FILE *SYS_IO_FileCreate(int fd);


#ifdef __cplusplus
}
#endif


#endif /* __SYS_USERIO_H__ */
