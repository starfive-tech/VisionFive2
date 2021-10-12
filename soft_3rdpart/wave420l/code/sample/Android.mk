
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

#Building vpurun binary which will be placed in the /system/bin folder

SRC_FILES := helper/main_helper.c				\
          helper/vpuhelper.c                         \
          helper/bitstream/bitstreamfeeder.c         \
          helper/bitstream/bitstreamreader.c         \
          helper/bitstream/bsfeeder_fixedsize_impl.c \
          helper/bitstream/bsfeeder_framesize_impl.c \
          helper/comparator/bin_comparator_impl.c    \
          helper/comparator/comparator.c             \
          helper/comparator/md5_comparator_impl.c    \
          helper/comparator/yuv_comparator_impl.c    \
          helper/display/fbdev_impl.c                \
          helper/display/simplerenderer.c            \
          helper/misc/cfgParser.c                    \
          helper/misc/cnm_fpga.c                     \
          helper/misc/cnm_video_helper.c             \
          helper/misc/container.c                    \
          helper/misc/datastructure.c                \
          helper/misc/platform.c                     \
          helper/yuv/yuvfeeder.c                     \
          helper/yuv/yuvLoaderfeeder.c               
	




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
#ifdef CODA980_DEC_SAMPLE
prog_names := coda980_dec_test
#endif
#ifdef CODA980_ENC_SAMPLE
prog_names += coda980_enc_test
#endif
#ifdef WAVE4XX_DEC_SAMPLE 
prog_names += w4_dec_test
#endif
#ifdef WAVE4XX_ENC_SAMPLE 
prog_names += w4_enc_test
#endif

$(foreach item,$(prog_names),$(eval $(call build_executable,$(item))))

