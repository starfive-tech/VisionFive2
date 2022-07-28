# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2022 StarFive Technology Co., Ltd.
#
CROSS_BIN_PATH := $(CURDIR)/../../../work/buildroot_initramfs/host/bin
target := riscv64-buildroot-linux-gnu

CC  = $(target)-gcc
CXX = $(target)-g++
AR  = $(target)-ar

# if cross gcc file exist, then use it, else use the buildroot to build
ifeq ($(CROSS_BIN_PATH)/$(target)-gcc, $(wildcard $(CROSS_BIN_PATH)/$(target)-gcc))
CC  = $(CROSS_BIN_PATH)/$(target)-gcc
CXX = $(CROSS_BIN_PATH)/$(target)-g++
AR  = $(CROSS_BIN_PATH)/$(target)-ar
endif

INCLUDEDIR= -I$(CURDIR)/include
CUR_SOURCE=${wildcard *.c src/*.c}
#SOURCES=$(notdir $(CUR_SOURCE))
CUR_OBJS=${patsubst %.c,%.o,$(CUR_SOURCE)}
ROOT_DIR=$(CURDIR)/obj
ALL_OBJ_POS=$(addprefix $(ROOT_DIR)/,$(CUR_OBJS))

test_bin := $(CURDIR)/e24_share_mem

# $(info $(CUR_OBJS))

.PHONY: all CREATE_DIR
all: CREATE_DIR $(test_bin)

$(test_bin): $(CUR_OBJS)
	$(CC) $(INCLUDEDIR) $(ALL_OBJ_POS) -o $@

$(CUR_OBJS):%.o:%.c
	$(CC) $(INCLUDEDIR) -c $^ -o $(ROOT_DIR)/$@

clean:
	rm -rf $(ALL_OBJ_POS) $(test_bin) $(ROOT_DIR)

CREATE_DIR:
	mkdir -p $(ROOT_DIR) $(ROOT_DIR)/src
