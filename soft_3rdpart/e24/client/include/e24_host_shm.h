/*********************************Copyright (c)*********************************************
 **
 **
 **
 **-------------------------------file info-------------------------------------------------
 ** Vrsions:      V1.0
 ** Filename:     e24_host_shm.h
 ** Creator:      shanlong.li
 ** Date:         2021/08/30
 ** Description:  share memory and mailobx
 **
 **-------------------------------history----------------------------------------------
 ** Name:         shanlong.li
 ** Versions:     V1.0
 ** Date:         2021/08/30
 ** Description: 	
 **
 ** ----------------------------------------------------------------------------------------
 ******************************************************************************************/
#ifndef __E24_HOST_SHM_H__
#define __E24_HOST_SHM_H__

#include <stdlib.h>

#define E24_IOCTL_MAGIC 'e'
#define E24_IOCTL_SEND				_IO(E24_IOCTL_MAGIC, 1)
#define E24_IOCTL_RECV		        _IO(E24_IOCTL_MAGIC, 2)
#define E24_IOCTL_GET_CHANNEL		_IO(E24_IOCTL_MAGIC, 3)
#define E24_IOCTL_FREE_CHANNEL		_IO(E24_IOCTL_MAGIC, 4)
#define E24_IOCTL_ALLOC     		_IO(E24_IOCTL_MAGIC, 5)
#define E24_IOCTL_FREE      		_IO(E24_IOCTL_MAGIC, 6)

#define BUF_LEN     28

typedef unsigned long _u64;
typedef unsigned int  _u32;

enum e24_status {
	E24_STATUS_SUCCESS,
	E24_STATUS_FAILURE,
	E24_STATUS_PENDING,
};

struct e24_ioctl_alloc {
    _u32 size;
    _u32 align;
    _u64 addr;
};

struct e24_ioctl_user {
    _u32 flags;
    _u32 in_data_size;
    _u32 out_data_size;
    _u64 in_data_addr;
    _u64 out_data_addr;
};

struct e24_device {
	int count;
	int fd;
};

struct e24_host_ops {
	void (*fn)(struct e24_device *device,struct e24_ioctl_user *data,enum e24_status *status);
};

struct e24_event {
	struct e24_device *device;
	struct e24_host_ops f_ops;
};

static inline void set_status(enum e24_status *status,enum e24_status v) {
	if(status)
		*status = v;
}

static inline void *alloc_refcounted(size_t sz) {
	void *buf = calloc(1,sz);
	struct e24_device *ref = buf;

	if(ref)
		ref->count = 1;

	return buf;
}

struct e24_device *e24_open_device(int idx, enum e24_status *status);

void e24_release_device(struct e24_device *device);

struct e24_event *e24_event_init(struct e24_device *device,enum e24_status *status);

void e24_event_release(struct e24_event *evt);

int e24_alloc_buffer(struct e24_device *device,
				struct e24_ioctl_alloc *alloc_buf,enum e24_status *status);

void e24_release_buffer(struct e24_device * device,struct e24_ioctl_alloc *alloc_buf);

void e24_run_command(struct e24_event *evt, void *in_data, size_t in_data_size,
						void *out_data, size_t out_data_size, enum e24_status *status);

#endif
