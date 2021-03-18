# ----------------------------------------------------------------------
#
# Project: C&M Video decoder sample
#
# ----------------------------------------------------------------------
.PHONY: CREATE_DIR

USE_PTHREAD = yes
USE_RTL_SIM = no
BUILD_CONFIGURATION = EmbeddedLinux
MAKECMDGOALS ?=

ifeq ($(RTL_SIM), 1)
USE_RTL_SIM = yes
endif

PLATFORM = nativelinux_64bit

CROSS_CC_PREFIX =
JDI_C           = jdi/linux/jdi.c
MM_C            =
PLATFORM_FLAGS  =
VPATH           = jdi/linux:
USE_FFMPEG      = yes
USE_ALLOCATOR   = no

INCLUDES = -I./jpuapi -I./include -I./jdi -I./sample -I./sample/helper
ifeq ("$(BUILD_CONFIGURATION)", "NonOS")
    CROSS_CC_PREFIX = arm-none-eabi-
    JDI_C           = jdi/nonos/jdi.c
    USE_FFMPEG      = no
    USE_PTHREAD     = no
    PLATFORM        = none
    DEFINES         = -DLIB_C_STUB
    PLATFORM_FLAGS  =
    VPATH           = jdi/nonos:
    USE_FFMPEG      = no
    USE_ALLOCATOR   = yes
endif
ifeq ("$(BUILD_CONFIGURATION)", "EmbeddedLinux")
    CROSS_CC_PREFIX = riscv64-buildroot-linux-gnu-
    PLATFORM        = riscvlinux
    USE_FFMPEG      ?= yes
endif

CC  = $(CROSS_CC_PREFIX)gcc
CXX = $(CROSS_CC_PREFIX)g++
AR  = $(CROSS_CC_PREFIX)ar

ifeq ($(USE_FFMPEG), yes)
DEFINES += -DSUPPORT_FFMPEG
INCLUDES += -I./ffmpeg/include
LDLIBS  += -lavformat -lavcodec -lavutil -lz
LDFLAGS += -L./ffmpeg/$(PLATFORM)
LDFLAGS += -L./ffmpeg/lib
endif

ifeq ($(USE_RTL_SIM), yes)
DEFINES += -DCNM_SIM_PLATFORM -DCNM_SIM_DPI_INTERFACE
else
endif

CFLAGS  += -g -I. -Wno-implicit-function-declaration -Wno-int-to-pointer-cast -Wno-format -Wl,--fatal-warning $(INCLUDES) $(DEFINES) $(PLATFORM_FLAGS) -Wall -Werror
ifeq ($(MAKECMDGOALS), multi)
CFLAGS  +=  -DSUPPORT_MULTI_INSTANCE_TEST
endif
ifeq ($(USE_RTL_SIM), yes)
ifeq ($(IUS), 1)
#CFLAGS  += -m32 # ncverilog is 32bit version
CFLAGS  += -fPIC # ncverilog is 64bit version
endif
USE_ALLOCATOR = yes
endif
ARFLAGS += cru
ifeq ($(USE_PTHREAD), yes)
LDLIBS  += -lpthread
endif
LDLIBS  += -lm

OBJDIR=obj
ALLOBJS=*.o
ALLDEPS=*.dep
ALLLIBS=*.a
RM=rm -f
MKDIR=mkdir -p

ENC_SOURCES = $(JDI_C) jpuapi.c jpuapifunc.c jpuhelper.c jpulog.c cnm_fpga.c main_jpg_enc_test.c \
              yuv_feeder.c platform.c datastructure.c bitstreamwriter.c
DEC_SOURCES = $(JDI_C) jpuapi.c jpuapifunc.c jpuhelper.c jpulog.c cnm_fpga.c main_jpg_dec_test.c \
              platform.c datastructure.c bitstreamfeeder.c bsfeeder_fixedsize_impl.c
MUL_SOURCES = $(JDI_C) jpuapi.c jpuapifunc.c jpuhelper.c jpulog.c cnm_fpga.c main_multi_instance_test.c \
              platform.c datastructure.c yuv_feeder.c
MUL_SOURCES += bitstreamfeeder.c bsfeeder_fixedsize_impl.c main_jpg_dec_test.c
MUL_SOURCES += bitstreamwriter.c main_jpg_enc_test.c

ifeq ($(USE_RTL_SIM), yes)
ENC_SOURCES += main_sim.c
DEC_SOURCES += main_sim.c
MUL_SOURCES += main_sim.c
endif
ifeq ($(USE_ALLOCATOR), yes)
ENC_SOURCES += mm.c
DEC_SOURCES += mm.c
MUL_SOURCES += mm.c
endif
ifeq ($(USE_FFMPEG), yes)
ENC_SOURCES += bswriter_container_impl.c
DEC_SOURCES += bsfeeder_framesize_impl.c
MUL_SOURCES += bsfeeder_framesize_impl.c
MUL_SOURCES += bswriter_container_impl.c
endif



VPATH += jdi:
VPATH += jdi/nonos:
VPATH += jpuapi:
VPATH += sample:sample/helper:


DEC_OBJECTNAMES=$(patsubst %.c,%.o,$(DEC_SOURCES))
DEC_OBJECTPATHS=$(addprefix $(OBJDIR)/,$(notdir $(DEC_OBJECTNAMES)))
ENC_OBJECTNAMES=$(patsubst %.c,%.o,$(ENC_SOURCES))
ENC_OBJECTPATHS=$(addprefix $(OBJDIR)/,$(notdir $(ENC_OBJECTNAMES)))
MUL_OBJECTNAMES=$(patsubst %.c,%.o,$(MUL_SOURCES))
MUL_OBJECTPATHS=$(addprefix $(OBJDIR)/,$(notdir $(MUL_OBJECTNAMES)))

ifeq ($(MAKECMDGOALS), multi)
	TARGET = multi
else
	TARGET = decoder
	TARGET += encoder
endif

all: $(TARGET)


encoder: CREATE_DIR $(ENC_OBJECTPATHS)
ifeq ($(USE_RTL_SIM), yes)
	# do not need to make excutable
else
	$(CC) -o jpg_enc_test $(LDFLAGS) -Wl,-gc-section -Wl,--start-group $(ENC_OBJECTPATHS) $(LDLIBS) -Wl,--end-group
endif

decoder: CREATE_DIR $(DEC_OBJECTPATHS)
ifeq ($(USE_RTL_SIM), yes)
	# do not need to make excutable
else
	$(CC) -o jpg_dec_test $(LDFLAGS) -Wl,-gc-section -Wl,--start-group $(DEC_OBJECTPATHS) $(LDLIBS) -Wl,--end-group
endif

multi: CREATE_DIR $(MUL_OBJECTPATHS)
ifeq ($(USE_RTL_SIM), yes)
	# do not need to make excutable
else
	$(CC) -o multi_instance_test $(LDFLAGS) -Wl,-gc-section -Wl,--start-group $(MUL_OBJECTPATHS) $(LDLIBS) -Wl,--end-group
endif

-include $(OBJECTPATHS:.o=.dep)

clean:
	$(RM) $(TARGET) jpg_enc_test jpg_dec_test multi_instance_test
	$(RM) $(OBJDIR)/$(ALLOBJS)
	$(RM) $(OBJDIR)/$(ALLDEPS)

CREATE_DIR:
	-mkdir -p $(OBJDIR)
obj/%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
