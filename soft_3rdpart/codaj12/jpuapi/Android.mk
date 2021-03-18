LOCAL_PATH := $(call my-dir)

# Building the vpuapi
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	jpuapi.c \
	jpuapifunc.c \
	../src/jpuhelper.c \
	../src/jpulog.c

 LOCAL_SRC_FILES += \
	../jdi/linux/jdi.c \


LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libjpu

# LOCAL_CFLAGS +=  -DCNM_FPGA_PLATFORM


LOCAL_SHARED_LIBRARIES :=       \
		libutils		\
		libcutils		\
		libdl  			\
		libdvm			\
		liblog

							

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src		\
					$(TOP)/hardware/jpu	\
					$(TOP)/hardware/jpu/include		\
							

					

include $(BUILD_SHARED_LIBRARY)

