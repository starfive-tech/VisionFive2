/*********************************Copyright (c)*********************************************
 **
 **
 **
 **-------------------------------file info-------------------------------------------------
 ** Vrsions:      V1.0
 ** Filename:     e24_host_linux.c
 ** Creator:      shanlong.li
 ** Date:         2021/08/30
 ** Description:  share memory and mailbox
 **
 **-------------------------------history----------------------------------------------
 ** Name:         shanlong.li
 ** Versions:     V1.0
 ** Date:         2021/08/30
 ** Description: 	
 **
 ** ----------------------------------------------------------------------------------------
 ******************************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "e24_host_shm.h"

void e24_request_process(struct e24_device *device,
					struct e24_ioctl_user *data,enum e24_status *status);

volatile int msg_done = 0;

void e24_irq_handler(int val)
{
	printf("e24 msg recv %d\n",val);
	msg_done = 1;
}

void init_signal(int fd)
{
	int flags,ret;
	
	signal(SIGIO, e24_irq_handler);

    ret = fcntl(fd, F_SETOWN, getpid());

    flags = fcntl(fd, F_GETFL);

    fcntl(fd, F_SETFL, flags | FASYNC);
}

struct e24_device *e24_open_device(int idx, enum e24_status *status)
{
	struct e24_device *device;

	char name[sizeof("/dev/eboot")+sizeof(int)*4];
	int fd;

	sprintf(name,"/dev/eboot%u",idx);
	fd = open(name,O_RDWR);
	if(fd < 0) {
		set_status(status,E24_STATUS_FAILURE);
		return NULL;
	}

	device = alloc_refcounted(sizeof(*device));
	if(!device) {
		set_status(status,E24_STATUS_FAILURE);
		return NULL;
	}

	device->fd = fd;
	set_status(status, E24_STATUS_SUCCESS);
	
	return device;
}

void e24_release_device(struct e24_device *device) {
	if(device) {
		close(device->fd);
		free(device);
	}
}

struct e24_event *e24_event_init(struct e24_device *device,enum e24_status *status) {
	struct e24_event *evt;

	evt = malloc(sizeof(*evt));
	if(!evt) {
		set_status(status,E24_STATUS_FAILURE);
		return NULL;
	}

	evt->device = device;
	evt->f_ops.fn = e24_request_process;
	return evt;
}

void e24_event_release(struct e24_event *evt) {

    ioctl(evt->device->fd,E24_IOCTL_FREE_CHANNEL,NULL);

	free(evt);
}

void e24_request_process(struct e24_device *device,
					struct e24_ioctl_user *data,enum e24_status *status) {
	int ret;
	
	data->flags = 4;
	ret = ioctl(device->fd,E24_IOCTL_GET_CHANNEL,NULL);
    ret = ioctl(device->fd,E24_IOCTL_SEND,data);

	if(ret < 0)
		set_status(status,E24_STATUS_FAILURE);
	else
		set_status(status, E24_STATUS_SUCCESS);
}

int e24_alloc_buffer(struct e24_device *device,
				struct e24_ioctl_alloc *alloc_buf,enum e24_status *status) {
	int ret;
	
    ret = ioctl(device->fd,E24_IOCTL_ALLOC,alloc_buf);
    if(ret < 0)
        set_status(status,E24_STATUS_FAILURE);
	else
		set_status(status, E24_STATUS_SUCCESS);

	return 0;
}

void e24_release_buffer(struct e24_device * device,struct e24_ioctl_alloc *alloc_buf) {
	if(alloc_buf)
		ioctl(device->fd,E24_IOCTL_FREE,alloc_buf);
}

void e24_run_push(struct e24_event *evt,struct e24_ioctl_user *user_data, enum e24_status *status) {
	evt->f_ops.fn(evt->device,user_data,status);
}

void e24_run_command(struct e24_event *evt, void *in_data, size_t in_data_size,
						void *out_data, size_t out_data_size, enum e24_status *status) {
	struct e24_ioctl_user *user_data;

	user_data = malloc(sizeof(*user_data));
	if(!user_data) {
		set_status(status, E24_STATUS_FAILURE);
		return;
	}

	user_data->in_data_addr = (_u64)in_data;
	user_data->in_data_size = in_data_size;
	user_data->out_data_addr = (_u64)out_data;
	user_data->out_data_size = out_data_size;

	e24_run_push(evt,user_data,status);
	free(user_data);
}


