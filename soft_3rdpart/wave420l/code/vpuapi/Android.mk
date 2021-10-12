LOCAL_PATH := $(call my-dir)

# FOR C&M FPGA Environment
CNM_FPGA_FLAGS := -DCNM_FPGA_PLATFORM -DCNM_FPGA_USB_INTERFACE -DSUPPORT_USB_TRANSFER -DSUPPORT_CONF_TEST
ifeq ($(PRODUCT), 7Q)
CNM_CFLAGS := -DCODA7Q $(CNM_FPGA_FLAGS)
endif
ifeq ($(PRODUCT), 960)
CNM_CFLAGS := -DCODA960 $(CNM_FPGA_FLAGS)
endif
ifeq ($(PRODUCT), 980)
CNM_CFLAGS := -DCODA980 $(CNM_FPGA_FLAGS)
endif
ifeq ($(PRODUCT), 410)
CNM_CFLAGS := -DWAVE410 $(CNM_FPGA_FLAGS)
endif
ifeq ($(PRODUCT), 412)
CNM_CFLAGS := -DWAVE412 $(CNM_FPGA_FLAGS)
endif
ifeq ($(PRODUCT), 420)
CNM_CFLAGS := -DWAVE420 $(CNM_FPGA_FLAGS)
endif
#

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

