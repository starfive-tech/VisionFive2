wrkdir := $(CURDIR)
RISCVV := $(wrkdir)/../../../work/buildroot_initramfs/host/bin
target := riscv64-buildroot-linux-gnu
CROSS_COMPILE := $(RISCVV)/$(target)-

INCLUDEDIR= -I$(wrkdir)/include
CUR_SOURCE=${wildcard *.c src/*.c}
#SOURCES=$(notdir $(CUR_SOURCE))
CUR_OBJS=${patsubst %.c,%.o,$(CUR_SOURCE)}
ROOT_DIR=$(CURDIR)/obj
ALL_OBJ_POS=$(addprefix $(ROOT_DIR)/,$(CUR_OBJS))

test_bin := $(wrkdir)/e24_share_mem

$(info $(CUR_OBJS))
.PHONY:all
all:$(CUR_OBJS) $(test_bin)
$(test_bin):$(ALL_OBJ_POS)
	$(CROSS_COMPILE)gcc $(INCLUDEDIR) $^ -o $@

$(CUR_OBJS):%.o:%.c
	$(CROSS_COMPILE)gcc $(INCLUDEDIR) -c $^ -o $(ROOT_DIR)/$@

clean:
	rm -rf $(ALL_OBJ_POS) $(test_bin)
