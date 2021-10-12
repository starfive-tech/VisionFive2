LOCAL_PATH := $(call my-dir)


# Building the theoraparser
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	./src/bitpack.c \
	./src/decode.c \
	./src/info.c \
	./src/internal.c \
	./src/state.c \
	./src/theora_parser.c \

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libtheoraparser

LOCAL_CFLAGS := 

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES :=       \
		libutils                \
        libdl					\
		libdvm					

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src		\
					$(LOCAL_PATH)/include	\
					

#LOCAL_PRELINK_MODULE:=false
include $(BUILD_SHARED_LIBRARY)
