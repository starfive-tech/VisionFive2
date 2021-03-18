LOCAL_PATH := $(call my-dir)


# Building the vpuapi
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	../vdi/linux/vdi.c \
   	../vdi/linux/vdi_osal.c

LOCAL_SRC_FILES += \
 	vpuapi.c \
	product.c \
	coda9/coda9.c \
	wave/common/common.c \
	wave/coda7q/coda7q.c \
	wave/wave4/wave4.c \
	vpuapifunc.c 

# 	../vdi/mm.c

# LOCAL_SRC_FILES += \
#  		../vdi/socket/vdi.c \
#  		../vdi/socket/vdi_osal.c \
# 		../vdi/socket/hpi_client.c \
# 		../vdi/mm.c



LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libvpu

LOCAL_CFLAGS := -DCONFIG_DEBUG_LEVEL=255 $(CNM_CFLAGS)
# LOCAL_CFLAGS += -DCNM_FPGA_PLATFORM -DCNM_FPGA_USB_INTERFACE


LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES :=       \
		libutils      \
		libdl  		 \
		libdvm		\
	   	liblog	

		
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src		\
					$(TOP)/hardware/vpu	\
					$(TOP)/hardware/vpu/include		
					


include $(BUILD_SHARED_LIBRARY)

