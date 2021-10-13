.PHONY: clean

CROSS_CC_PREFIX=
PLATFORM_CFLAGS=
ifeq ("$(BUILD_CONFIGURATION)", "EmbeddedLinux")
    CROSS_CC_PREFIX = arm-none-linux-gnueabi-
endif
ifeq ("$(BUILD_CONFIGURATION)", "NonOS")
    CROSS_CC_PREFIX = arm-none-eabi-
    PLATFORM_CFLAGS =
endif

ifeq ("$(BUILD_CONFIGURATION)", "RiscvLinux")
    CROSS_CC_PREFIX = riscv64-buildroot-linux-gnu-
endif

CC  = $(CROSS_CC_PREFIX)gcc
CXX = $(CROSS_CC_PREFIX)g++
AR  = $(CROSS_CC_PREFIX)ar

CFLAGS   = $(PLATFORM_CFLAGS) -g -O3 -c -I./include
CXXFLAGS = $(PLATFORM_CFLAGS) -g -O3 -c -I./include
ARFLAGS  = cru
LDFLAGS  = 
TARGET   = libtheoraparser.a


SOURCES = bitpack.c decode.c info.c internal.c state.c theora_parser.c
OBJECTS = $(SOURCES:.c=.o)
VPATH   = src

$(TARGET): $(OBJECTS) 
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f *.dep

-include $(OBJECTS:.o=.dep)

.c.o: Makefile
	$(CC) -c $(CFLAGS) -o $@ $< -MD -MF $(@:.o=.dep)


