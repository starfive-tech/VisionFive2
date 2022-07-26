# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2022 StarFive Technology Co., Ltd.
#
CROSS_BIN_PATH := $(CURDIR)/../../work/buildroot_initramfs/host/bin
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

CFLAGS  += -g -I. $(INCLUDES)
LDFLAGS = -lpthread -Wl,--fatal-warning

SOURCES_READ_TEST=read_mbox.c
OBJECTS_READ_TEST=$(patsubst %.c,%.o,$(SOURCES_READ_TEST))

.PHONY: all clean

all: read_test

read_test: $(OBJECTS_READ_TEST)
	$(CC) -o $@ $(LDFLAGS) $^

clean:
	rm -rf read_test *.o *.dep

%.o: %.c
	$(CC) $(CFLAGS) -Wall -Werror  -c $< -o $@ -MD -MF $(@:.o=.dep)
