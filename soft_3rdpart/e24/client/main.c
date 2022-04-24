/*********************************Copyright (c)*********************************************
 **
 **
 **
 **-------------------------------file info-------------------------------------------------
 ** Vrsions:      V1.0
 ** Filename:     main.c
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
#include <string.h>
#include "e24_host_shm.h"

int comm_local_buffer(void)
{
	enum e24_status status;
	struct e24_device *device;
	struct e24_event *evt;
    char in_data[BUF_LEN],out_data[BUF_LEN];
    int i;

	device = e24_open_device(0,&status);
	if(status == E24_STATUS_FAILURE) {
		printf("open device fail\n");
		return 0;
	}

	evt = e24_event_init(device,&status);
	if(status == E24_STATUS_FAILURE) {
		printf("event init fail\n");
		goto _derro;
	}
	
    printf("input data:\n");
    for(i = 0;i< BUF_LEN; i++) {
        in_data[i] = 27;
        printf("%d,",in_data[i]);
    }
    printf("\n");

	memset(out_data,0x0,BUF_LEN);
	e24_run_command(evt,(void*)in_data,BUF_LEN,(void*)out_data,BUF_LEN,&status);
    
    printf("output data:\n");
    for(i = 0;i< BUF_LEN; i++) {
        printf("%d,",out_data[i]);
    }
	printf("\n");

_error:
	e24_release_buffer(device,NULL);
	e24_event_release(evt);
_derro:
	e24_release_device(device);

	return 0;
}

int comm_alloc_buffer(void) {
	enum e24_status status = -1;
	struct e24_device *device;
	struct e24_event *evt;
	struct e24_ioctl_alloc alloc_out_buf = {.size = BUF_LEN,};
	struct e24_ioctl_alloc alloc_in_buf = {.size = BUF_LEN,};
	int i;

	device = e24_open_device(0,&status);
	if(status == E24_STATUS_FAILURE) {
		printf("open device fail\n");
		return 0;
	}

	evt = e24_event_init(device,&status);
	if(status == E24_STATUS_FAILURE) {
		printf("event init fail\n");
		goto _derror;
	}

	e24_alloc_buffer(device,&alloc_in_buf,&status);
	if(status == E24_STATUS_FAILURE) {
		printf("alloc in buffer failed\n");
		goto _berror;
	}

	e24_alloc_buffer(device,&alloc_out_buf,&status);
	if(status == E24_STATUS_FAILURE) {
		printf("alloc out buffer failed\n");
		goto _error;
	}

	printf("input data:\n");
	for(i = 0;i< BUF_LEN; i++) {
		((char*)alloc_in_buf.addr)[i] = 7;
		printf("%d,",((char*)alloc_in_buf.addr)[i]);
	}
	printf("\n");

	memset((char *)alloc_out_buf.addr,0x0,BUF_LEN);
	e24_run_command(evt,(void*)alloc_in_buf.addr,BUF_LEN,(void*)alloc_out_buf.addr,BUF_LEN,&status);
	
	printf("output data:\n");
	for(i = 0;i< BUF_LEN; i++) {
		printf("%d,",((char *)alloc_out_buf.addr)[i]);
	}
	printf("\n");

	e24_release_buffer(device,&alloc_out_buf);
_error:
	e24_release_buffer(device,&alloc_in_buf);
_berror:
	e24_event_release(evt);
_derror:
	e24_release_device(device);

	return 0;

}

int main(int argc,char **argv) {
	printf("buffer alloced from share memory:\n");
	comm_alloc_buffer();
	printf("local buffer:\n");
	comm_local_buffer();
	return 0;
}

