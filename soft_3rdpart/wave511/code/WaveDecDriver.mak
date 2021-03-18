# Comment/uncomment the	following line to disable/enable debugging
#DEBUG = y

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O	-g # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

export CC="$(CROSS_COMPILE)gcc"
export AR="$(CROSS_COMPILE)ar"
export CXX="${CROSS_COMPILE}g++"
export AS="${CROSS_COMPILE}as"
export LD="${CROSS_COMPILE}ld"
export RANLIB="${CROSS_COMPILE}ranlib"
export READELF="${CROSS_COMPILE}readelf"
export STRIP="${CROSS_COMPILE}strip"

DRV_PATH  := $(PWD)/vdi/linux/driver
EXTRA_CFLAGS +=	$(DEBFLAGS) -I$(LDDINCDIR) -Wno-unused-function

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= vdec.o

else

KERNELDIR := ../../../work/linux

PWD	  := $(shell pwd)

default:
	$(MAKE)	-C $(KERNELDIR)	M=$(DRV_PATH) LDDINCDIR=$(DRV_PATH)/../include modules

endif



clean:
	rm -rf $(DRV_PATH)/*.o $(DRV_PATH)/*~ $(DRV_PATH)/core $(DRV_PATH)/.depend $(DRV_PATH)/.*.cmd $(DRV_PATH)/*.ko $(DRV_PATH)/*.mod.c $(DRV_PATH)/modules.order $(DRV_PATH)/.tmp_versions $(DRV_PATH)/*.dwo $(DRV_PATH)/.*.dwo

depend .depend dep:
	$(CC) $(CFLAGS)	-M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include	.depend
endif
