
LOCAL_PATH := $(call my-dir)

# FOR C&M FPGA Environment
CNM_FPGA_FLAGS := -DCNM_FPGA_PLATFORM -DCNM_FPGA_USB_INTERFACE -DSUPPORT_USB_TRANSFER -DSUPPORT_CONF_TEST
CNM_CFLAGS := -DCODAJ12 $(CNM_FPGA_FLAGS)
#

#Building vpurun binary which will be placed in the /system/bin folder

SRC_FILES := helper/cnm_fpga.c				\
             helper/datastructure.c         \
             helper/jpuhelper.c             \
             helper/jpulog.c                \
             helper/platform.c              \
             helper/yuv_feeder.c

C_INCLUDES := $(LOCAL_PATH)				\
		$(TOP)/hardware/vpu/ffmpeg/include	\
		$(TOP)/hardware/vpu/theoraparser/include	\
		$(TOP)/hardware/vpu/vpuapi		        \
		$(TOP)/hardware/vpu/sample			\
		$(TOP)/hardware/vpu/sample/helper           
 
SHARED_LIBRARIES :=       \
        libvpu			\
        libtheoraparser         \
        libutils                \

define build_executable
	include $(CLEAR_VARS)
	LOCAL_C_INCLUDES := $(C_INCLUDES)
	LOCAL_SHARED_LIBRARIES := $(SHARED_LIBRARIES)
	LOCAL_SRC_FILES := main_$(1).c
	LOCAL_SRC_FILES += $(SRC_FILES)
	LOCAL_MODULE_TAGS := eng
	LOCAL_MODULE := $(1)
	LOCAL_CFLAGS := $(CNM_CFLAGS)
	include $(BUILD_EXECUTABLE)
endef

prog_names := 
#ifdef CODAJ_ENC_SAMPLE
prog_names += jpg_enc_test
#endif
#ifdef CODAJ_DEC_SAMPLE
prog_names += jpg_dec_test
#endif

$(foreach item,$(prog_names),$(eval $(call build_executable,$(item))))

