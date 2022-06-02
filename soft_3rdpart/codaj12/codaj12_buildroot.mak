# ----------------------------------------------------------------------
#
# codaj12
#
# ----------------------------------------------------------------------
.PHONY: CREATE_DIR
BUILD_CONFIGURATION := EmbeddedRiscvLinux
PRODUCT := CODAJ12
PRODUCT := CODDAJ12C

USE_PTHREAD = yes
USE_RTL_SIM = no
LINT_HOME   = etc/lint

UNAME = $(shell uname -a)
ifneq (,$(findstring i386, $(UNAME)))
    USE_32BIT = yes
endif

ifeq ($(RTL_SIM), 1)
USE_RTL_SIM = yes
endif

REFC    := 0

ifeq ($(USE_32BIT), yes)
PLATFORM    = nativelinux
else
PLATFORM    = nativelinux_64bit
endif

CROSS_CC_PREFIX =
JDI_C           = jdi/linux/jdi.c
VDI_OSAL_C      =
MM_C            =
PLATFORM_FLAGS  =
JDI_VPATH       = jdi/linux
ifeq ("$(BUILD_CONFIGURATION)", "NonOS")
    CROSS_CC_PREFIX = arm-none-eabi-
    JDI_C           = jdi/nonos/jdi.c
    VDI_OSAL_C      =
    MM_C            = jdi/mm.c
    USE_PTHREAD     = no
    PLATFORM        = none
    DEFINES         = -DLIB_C_STUB
    PLATFORM_FLAGS  =
    JDI_VPATH       = jdi/nonos
    NONOS_RULE      = options_nonos.lnt
endif
ifeq ("$(BUILD_CONFIGURATION)", "EmbeddedLinux")
    CROSS_CC_PREFIX = arm-none-linux-gnueabi-
    PLATFORM        = armlinux
endif

ifeq ("$(BUILD_CONFIGURATION)", "EmbeddedRiscvLinux")
    CROSS_CC_PREFIX = riscv64-buildroot-linux-gnu-
    PLATFORM        = riscvlinux
endif
CC  = $(CROSS_CC_PREFIX)gcc
CXX = $(CROSS_CC_PREFIX)g++
LINKER=$(CC)
AR  = $(CROSS_CC_PREFIX)ar

INCLUDES = -I./jpuapi -I./ffmpeg/include -I./sample/helper -I./sample -I./jdi -I../
ifeq ($(USE_RTL_SIM), yes)
DEFINES += -DCNM_SIM_PLATFORM -DCNM_SIM_DPI_INTERFACE -DSUPPORT_DECODER
DEFINES += -D$(PRODUCT) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
else
DEFINES += -D$(PRODUCT) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif	# USE_SIM_PLATFORM
DEFINES += -DUSE_FEEDING_METHOD_BUFFER

CFLAGS  += -g -I. -Wno-format-truncation -Wl,--fatal-warning $(INCLUDES) $(DEFINES) $(PLATFORM_FLAGS)
ifeq ($(USE_RTL_SIM), yes)
ifeq ($(IUS), 1)
CFLAGS  += -fPIC # ncverilog is 64bit version
endif
endif
ARFLAGS += cru

LDFLAGS  = $(PLATFORM_FLAGS)

ifeq ($(USE_PTHREAD), yes)
LDLIBS  += -lpthread
endif
LDLIBS  += -lm

BUILDLIST=DECTEST
MAKEFILE=codaj12_buildroot.mak
DECTEST=libcodadec.so

OBJDIR=obj
ALLOBJS=*.o
ALLDEPS=*.dep
ALLLIBS=*.a
RM=rm -f
MKDIR=mkdir -p

SOURCES_COMMON = jpuapi.c jpuapifunc.c jpuhelper.c jpulog.c cnm_fpga.c main_jpg_dec_test.c \
                platform.c datastructure.c bitstreamfeeder.c bsfeeder_fixedsize_impl.c bsfeeder_buffer_impl.c\
                $(JDI_C)             \
                $(VDI_OSAL_C)        \
                $(MM_C)

VPATH = jdi:
VPATH += sample:
VPATH += sample/helper:
VPATH += $(JDI_VPATH):jpuapi

VPATH2=$(patsubst %,-I%,$(subst :, ,$(VPATH)))

OBJECTNAMES_COMMON=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES_COMMON)))
OBJECTPATHS_COMMON=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_COMMON)))

SOURCES_DECTEST = sample/main_jpg_dec_test.c
ifeq ($(USE_RTL_SIM), yes)
	SOURCES_DECTEST += sample/main_sim.c
endif

OBJECTNAMES_DECTEST=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES_DECTEST)))
OBJECTPATHS_DECTEST=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_DECTEST))) $(OBJECTPATHS_COMMON)

all: $(BUILDLIST)

ifeq ($(USE_RTL_SIM), yes)
DECTEST: CREATE_DIR $(OBJECTPATHS_COMMON)
else
DECTEST: CREATE_DIR $(OBJECTPATHS_COMMON)
	$(LINKER) -g -fPIC -shared -o $(DECTEST) $(LDFLAGS) -Wl,-gc-section -Wl,--start-group $(OBJECTPATHS_COMMON) $(LDLIBS) -Wl,--end-group
endif


-include $(OBJECTPATHS:.o=.dep)

clean:
	$(RM) $(DECTEST)
	$(RM) $(OBJDIR)/$(ALLOBJS)
	$(RM) $(OBJDIR)/$(ALLDEPS)

CREATE_DIR:
	-mkdir -p $(OBJDIR)

obj/%.o: %.c $(MAKEFILE)
	$(CC) -fPIC -shared $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

lint:
	"$(LINT_HOME)/flint" -i"$(LINT_HOME)" $(DEFINES) $(INCLUDES) $(VPATH2) linux_std.lnt $(HAPS_RULE) $(NONOS_RULE)  $(SOURCES_COMMON)

