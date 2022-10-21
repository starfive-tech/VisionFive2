# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2021 StarFive Technology Co., Ltd.
#
# Project: Starfive Openmax IL Shimlayer
#
.PHONY: CREATE_DIR
BUILD_CONFIGURATION := EmbeddedRiscvLinux

PRODUCT := OMX_IL

ifeq ("$(BUILD_CONFIGURATION)", "EmbeddedRiscvLinux")
    CROSS_CC_PREFIX = riscv64-buildroot-linux-gnu-
    PLATFORM        = riscvlinux
endif
CC  = $(CROSS_CC_PREFIX)gcc
CXX = $(CROSS_CC_PREFIX)g++
LINKER=$(CC)
AR  = $(CROSS_CC_PREFIX)ar

DEFINES = -DUSE_FEEDING_METHOD_BUFFER
INCLUDES = -I./include/khronos -I./core -I./component/video/wave5/common -I./component/video/wave4 -I./component/helper
$(warning "the value of LOCAL_PATH is$(STAGING_DIR)")
INCLUDES += -I$(STAGING_DIR)/usr/include/
INCLUDES += -I$(STAGING_DIR)/usr/include/wave420l/
INCLUDES += -I$(STAGING_DIR)/usr/include/wave420l/vpuapi/
INCLUDES += -I$(STAGING_DIR)/usr/include/wave420l/sample/helper/
#INCLUDES += -I$(STAGING_DIR)/usr/include/wave420l/sample/helper/misc/

#mjpeg
INCLUDES += -I./component/image/common

CFLAGS  += -g -I. $(INCLUDES) $(DEFINES) $(PLATFORM_FLAGS)
CFLAGS  += -fpic
ARFLAGS += cru
LDFLAGS = -ldl -lpthread -Wl,--fatal-warning
LDFLAGS_FFMPEG = -lavformat -lavcodec -lavutil -lswresample

OBJDIR=obj
ALLOBJS=*.o
ALLDEPS=*.dep
ALLLIBS=*.a
RM=rm -f
MKDIR=mkdir -p
#vpu
SOURCES_COMMON = SF_OMX_Core.c SF_OMX_video_common.c
SOURCES_COMMON += SF_OMX_Vdec_decoder.c
SOURCES_COMMON += SF_OMX_Wave420L_encoder.c
SOURCES_COMMON += sf_queue.c
SOURCES_COMMON += sf_thread.c
SOURCES_COMMON += sf_semaphore.c
#mjpeg
SOURCES_COMMON += SF_OMX_mjpeg_common.c SF_OMX_Mjpeg_decoder.c
#test
SOURCES_DEC_TEST = dec_test.c
SOURCES_ENC_TEST = enc_test.c
MJPET_SOURCES_DEC_TEST = mjpeg_dec_test.c

VPATH  = component/video/wave5/common:
#VPATH += component/video/enc:
VPATH += component/video/wave5/dec:
VPATH += component/video/wave4:
VPATH += component/helper:
VPATH += core:
VPATH += tests:

#mjpeg
VPATH += component/image/common:
VPATH += component/image/dec:

OBJECTNAMES_COMMON=$(patsubst %.c,%.o,$(SOURCES_COMMON))
OBJECTPATHS_COMMON=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_COMMON)))
OBJECTNAMES_DEC_TEST=$(patsubst %.c,%.o,$(SOURCES_DEC_TEST))
OBJECTPATHS_DEC_TEST=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_DEC_TEST)))
OBJECTNAMES_ENC_TEST=$(patsubst %.c,%.o,$(SOURCES_ENC_TEST))
OBJECTPATHS_ENC_TEST=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_ENC_TEST)))

OBJECTNAMES_MJPEG_DEC_TEST=$(patsubst %.c,%.o,$(MJPET_SOURCES_DEC_TEST))
OBJECTPATHS_MJPEG_DEC_TEST=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_MJPEG_DEC_TEST)))

all: omx-il

omx-il: CREATE_DIR $(OBJECTPATHS_COMMON)
	$(CC) -g -fPIC -shared -o libsf-omx-il.so $(OBJECTPATHS_COMMON)

test: video-dec-test video-enc-test mjpeg-dec-test

video-dec-test: CREATE_DIR $(OBJECTPATHS_DEC_TEST)
	$(CC) -o video_dec_test $(LDFLAGS) $(LDFLAGS_FFMPEG) -lsf-omx-il $(OBJECTPATHS_DEC_TEST) -L./

video-enc-test: CREATE_DIR $(OBJECTPATHS_ENC_TEST)
	$(CC) -o video_enc_test $(LDFLAGS) $(LDFLAGS_FFMPEG) -lsf-omx-il $(OBJECTPATHS_ENC_TEST) -L./

mjpeg-dec-test: CREATE_DIR $(OBJECTPATHS_MJPEG_DEC_TEST)
	$(CC) -g -o mjpeg_dec_test $(LDFLAGS) $(LDFLAGS_FFMPEG) -lsf-omx-il $(OBJECTPATHS_MJPEG_DEC_TEST) -L./

clean:

CREATE_DIR:
	-mkdir -p $(OBJDIR)

obj/%.o: %.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)
