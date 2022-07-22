# Comment/uncomment the	following line to disable/enable debugging
#DEBUG = y

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O	-g # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

export ARCH=riscv
export SUBARCH=riscv
export CROSS_COMPILE=riscv64-buildroot-linux-gnu-

DRV_PATH  := $(shell pwd)/jdi/linux/driver
EXTRA_CFLAGS +=	$(DEBFLAGS) -I$(LDDINCDIR) -Wall

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= jdec.o

else

default:
	$(MAKE) -C $(KERNELDIR)	M=$(DRV_PATH) LDDINCDIR=$(DRV_PATH)/../include modules

endif



clean:
	rm -rf $(DRV_PATH)/*.o $(DRV_PATH)/*~ $(DRV_PATH)/core $(DRV_PATH)/.depend $(DRV_PATH)/.*.cmd $(DRV_PATH)/*.ko $(DRV_PATH)/*.mod.c $(DRV_PATH)/modules.order $(DRV_PATH)/.tmp_versions $(DRV_PATH)/*.dwo $(DRV_PATH)/.*.dwo

depend .depend dep:
	$(CC) $(CFLAGS)	-M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include	.depend
endif
