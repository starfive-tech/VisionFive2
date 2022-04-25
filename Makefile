ISA ?= rv64imafdc
ABI ?= lp64d

#TARGET_BOARD is U74 ,JH7110 or NULL
TARGET_BOARD := JH7110
BOARD_FLAGS	:=
HWBOARD ?= fpga
HWBOARD_FLAG ?= HWBOARD_FPGA

srcdir := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
srcdir := $(srcdir:/=)
confdir := $(srcdir)/conf
wrkdir := $(CURDIR)/work

buildroot_srcdir := $(srcdir)/buildroot
buildroot_initramfs_wrkdir := $(wrkdir)/buildroot_initramfs

# TODO: make RISCV be able to be set to alternate toolchain path
RISCV ?= $(buildroot_initramfs_wrkdir)/host
RVPATH := $(RISCV)/bin:$(PATH)
target := riscv64-buildroot-linux-gnu

CROSS_COMPILE := $(RISCV)/bin/$(target)-

buildroot_initramfs_tar := $(buildroot_initramfs_wrkdir)/images/rootfs.tar
buildroot_initramfs_config := $(confdir)/buildroot_initramfs_config
buildroot_initramfs_sysroot_stamp := $(wrkdir)/.buildroot_initramfs_sysroot
buildroot_initramfs_sysroot := $(wrkdir)/buildroot_initramfs_sysroot
buildroot_rootfs_wrkdir := $(wrkdir)/buildroot_rootfs
buildroot_rootfs_ext := $(buildroot_rootfs_wrkdir)/images/rootfs.ext4
buildroot_rootfs_config := $(confdir)/buildroot_rootfs_config

linux_srcdir := $(srcdir)/linux
linux_wrkdir := $(wrkdir)/linux
linux_defconfig := $(linux_srcdir)/arch/riscv/configs/starfive_jh7110_defconfig

vmlinux := $(linux_wrkdir)/vmlinux
vmlinux_stripped := $(linux_wrkdir)/vmlinux-stripped
vmlinux_bin := $(wrkdir)/vmlinux.bin

ifeq ($(TARGET_BOARD),JH7110)
	export TARGET_BOARD
	its_file=$(confdir)/$(HWBOARD)-fit-image.its
else ifeq ($(TARGET_BOARD),U74)
	export TARGET_BOARD
	BOARD_FLAGS += -DTARGET_BOARD_U74
	bbl_link_addr :=0x80700000
	its_file=$(confdir)/u74_nvdla-uboot-fit-image.its
else
	bbl_link_addr :=0x80000000
	its_file=$(confdir)/nvdla-uboot-fit-image.its
endif

flash_image := $(wrkdir)/hifive-unleashed-a00-YYYY-MM-DD.gpt
vfat_image := $(wrkdir)/hifive-unleashed-vfat.part
#ext_image := $(wrkdir)  # TODO

initramfs := $(wrkdir)/initramfs.cpio.gz

sbi_srcdir := $(srcdir)/opensbi
sbi_wrkdir := $(wrkdir)/opensbi

ifeq ($(TARGET_BOARD),JH7110)
sbi_bin := $(wrkdir)/opensbi/platform/starfive/firmware/fw_payload.bin
else
sbi_bin := $(wrkdir)/opensbi/platform/starfive/firmware/fw_payload.bin
endif

fit := $(wrkdir)/image.fit

fesvr_srcdir := $(srcdir)/riscv-fesvr
fesvr_wrkdir := $(wrkdir)/riscv-fesvr
libfesvr := $(fesvr_wrkdir)/prefix/lib/libfesvr.so

spike_srcdir := $(srcdir)/riscv-isa-sim
spike_wrkdir := $(wrkdir)/riscv-isa-sim
spike := $(spike_wrkdir)/prefix/bin/spike

qemu_srcdir := $(srcdir)/riscv-qemu
qemu_wrkdir := $(wrkdir)/riscv-qemu
qemu := $(qemu_wrkdir)/prefix/bin/qemu-system-riscv64

uboot_srcdir := $(srcdir)/HiFive_U-Boot
uboot_wrkdir := $(wrkdir)/HiFive_U-Boot

ifeq ($(TARGET_BOARD),JH7110)
uboot_dtb_file := $(wrkdir)/HiFive_U-Boot/arch/riscv/dts/starfive_visionfive.dtb
else
uboot_dtb_file := $(wrkdir)/HiFive_U-Boot/arch/riscv/dts/starfive_vic7100_evb.dtb
endif

uboot := $(uboot_wrkdir)/u-boot.bin
uboot_config := HiFive-U540_regression_defconfig

ifeq ($(TARGET_BOARD),JH7110)
	uboot_config := starfive_visionfive_defconfig
else ifeq ($(TARGET_BOARD),U74)
	uboot_config := starfive_vic7100_evb_smode_defconfig
else
	uboot_config := HiFive-U540_nvdla_iofpga_defconfig
endif

uboot_defconfig := $(uboot_srcdir)/configs/$(uboot_config)
rootfs := $(wrkdir)/rootfs.bin

target_gcc := $(CROSS_COMPILE)gcc

.PHONY: all nvdla-demo check_arg
nvdla-demo: check_arg $(fit) $(vfat_image)
	@echo "To completely erase, reformat, and program a disk sdX, run:"
	@echo "  make DISK=/dev/sdX format-nvdla-disk"
	@echo "  ... you will need gdisk and e2fsprogs installed"
	@echo "  Please note this will not currently format the SDcard ext4 partition"
	@echo "  This can be done manually if needed"
	@echo

all: check_arg $(fit) $(flash_image)
	@echo
	@echo "This image has been generated for an ISA of $(ISA) and an ABI of $(ABI)"
	@echo "Find the image in work/image.fit, which should be copied to an MSDOS boot partition 1"
	@echo
	@echo "To completely erase, reformat, and program a disk sdX, run:"
	@echo "  make DISK=/dev/sdX format-boot-loader"
	@echo "  ... you will need gdisk and e2fsprogs installed"
	@echo "  Please note this will not currently format the SDcard ext4 partition"
	@echo "  This can be done manually if needed"
	@echo

check_arg:
ifeq ( , $(filter $(HWBOARD), visionfive evb fpga))
	$(error board $(HWBOARD) is not supported, BOARD=[visionfive | evb | fpga(deflault)])
endif

# TODO: depracated for now
#ifneq ($(RISCV),$(buildroot_initramfs_wrkdir)/host)
#$(target_gcc):
#	$(error The RISCV environment variable was set, but is not pointing at a toolchain install tree)
#else
#$(target_gcc): $(buildroot_initramfs_tar)
#endif

.PHONY: visionfive evb fpga

visionfive: HWBOARD := visionfive
visionfive: nvdla-demo
visionfive: HWBOARD_FLAG := HWBOARD_VISIONFIVE

evb: HWBOARD := evb
evb: nvdla-demo
evb: HWBOARD_FLAG := HWBOARD_EVB

fpga: HWBOARD := fpga
fpga: nvdla-demo
fpga: HWBOARD_FLAG := HWBOARD_FPGA

$(buildroot_initramfs_wrkdir)/.config: $(buildroot_srcdir)
	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	cp $(buildroot_initramfs_config) $@
	$(MAKE) -C $< RISCV=$(RISCV) O=$(buildroot_initramfs_wrkdir) olddefconfig

# buildroot_initramfs provides gcc
$(buildroot_initramfs_tar): $(buildroot_srcdir) $(buildroot_initramfs_wrkdir)/.config $(buildroot_initramfs_config)
	$(MAKE) -C $< RISCV=$(RISCV) O=$(buildroot_initramfs_wrkdir)

.PHONY: buildroot_initramfs-menuconfig
buildroot_initramfs-menuconfig: $(buildroot_initramfs_wrkdir)/.config $(buildroot_srcdir)
	$(MAKE) -C $(dir $<) O=$(buildroot_initramfs_wrkdir) menuconfig
	$(MAKE) -C $(dir $<) O=$(buildroot_initramfs_wrkdir) savedefconfig
	cp $(dir $<)defconfig $(buildroot_initramfs_config)

# use buildroot_initramfs toolchain
# TODO: fix path and conf/buildroot_rootfs_config
$(buildroot_rootfs_wrkdir)/.config: $(buildroot_srcdir) $(buildroot_initramfs_tar)
	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	cp $(buildroot_rootfs_config) $@
	$(MAKE) -C $< RISCV=$(RISCV) PATH=$(RVPATH) O=$(buildroot_rootfs_wrkdir) olddefconfig

$(buildroot_rootfs_ext): $(buildroot_srcdir) $(buildroot_rootfs_wrkdir)/.config $(target_gcc) $(buildroot_rootfs_config)
	$(MAKE) -C $< RISCV=$(RISCV) PATH=$(RVPATH) O=$(buildroot_rootfs_wrkdir)

.PHONY: buildroot_rootfs
buildroot_rootfs: $(buildroot_rootfs_ext)
	cp $< $@

.PHONY: buildroot_rootfs-menuconfig
buildroot_rootfs-menuconfig: $(buildroot_rootfs_wrkdir)/.config $(buildroot_srcdir)
	$(MAKE) -C $(dir $<) O=$(buildroot_rootfs_wrkdir) menuconfig
	$(MAKE) -C $(dir $<) O=$(buildroot_rootfs_wrkdir) savedefconfig
	cp $(dir $<)defconfig conf/buildroot_rootfs_config

$(buildroot_initramfs_sysroot_stamp): $(buildroot_initramfs_tar)
	mkdir -p $(buildroot_initramfs_sysroot)
	tar -xpf $< -C $(buildroot_initramfs_sysroot) --exclude ./dev --exclude ./usr/share/locale
	touch $@

$(linux_wrkdir)/.config: $(linux_defconfig) $(linux_srcdir)
	mkdir -p $(dir $@)
	cp -p $< $@
	$(MAKE) -C $(linux_srcdir) O=$(linux_wrkdir) ARCH=riscv olddefconfig
ifeq (,$(filter rv%c,$(ISA)))
	sed 's/^.*CONFIG_RISCV_ISA_C.*$$/CONFIG_RISCV_ISA_C=n/' -i $@
	$(MAKE) -C $(linux_srcdir) O=$(linux_wrkdir) ARCH=riscv olddefconfig
endif
ifeq ($(ISA),$(filter rv32%,$(ISA)))
	sed 's/^.*CONFIG_ARCH_RV32I.*$$/CONFIG_ARCH_RV32I=y/' -i $@
	sed 's/^.*CONFIG_ARCH_RV64I.*$$/CONFIG_ARCH_RV64I=n/' -i $@
	$(MAKE) -C $(linux_srcdir) O=$(linux_wrkdir) ARCH=riscv olddefconfig
endif

$(uboot_wrkdir)/.config: $(uboot_defconfig)
	mkdir -p $(dir $@)
	cp -p $< $@
	$(MAKE) -C $(uboot_srcdir) O=$(uboot_wrkdir) ARCH=riscv olddefconfig

$(vmlinux): $(linux_srcdir) $(linux_wrkdir)/.config $(target_gcc)
	$(MAKE) -C $< O=$(linux_wrkdir) \
		ARCH=riscv \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		PATH=$(RVPATH) \
		vmlinux		\
		HWBOARD_FLAG=$(HWBOARD_FLAG) \
		all \
		modules


.PHONY: initrd
initrd: $(initramfs)

$(initramfs).d: $(buildroot_initramfs_sysroot)
	$(linux_srcdir)/usr/gen_initramfs_list.sh -l $(confdir)/initramfs.txt $(buildroot_initramfs_sysroot) > $@

$(initramfs): $(buildroot_initramfs_sysroot) $(vmlinux)
	cd $(linux_wrkdir) && \
		$(linux_srcdir)/usr/gen_initramfs_list.sh \
		-o $@ -u $(shell id -u) -g $(shell id -g) \
		$(confdir)/initramfs.txt \
		$(buildroot_initramfs_sysroot)

$(vmlinux_stripped): $(vmlinux)
	PATH=$(RVPATH) $(target)-strip -o $@ $<

$(vmlinux_bin): $(vmlinux)
	PATH=$(RVPATH) $(target)-objcopy -O binary $< $@

.PHONY: linux-menuconfig
linux-menuconfig: $(linux_wrkdir)/.config
	$(MAKE) -C $(linux_srcdir) O=$(dir $<) ARCH=riscv menuconfig
	$(MAKE) -C $(linux_srcdir) O=$(dir $<) ARCH=riscv savedefconfig
	cp $(dir $<)defconfig $(linux_defconfig)

ifeq ($(TARGET_BOARD),JH7110)
sbi: $(uboot) $(vmlinux)
	rm -rf $(sbi_wrkdir)
	mkdir -p $(sbi_wrkdir)
	cd $(sbi_wrkdir) && O=$(sbi_wrkdir) CFLAGS="-mabi=$(ABI) -march=$(ISA)" ${MAKE} -C $(sbi_srcdir) CROSS_COMPILE=$(CROSS_COMPILE) \
		PLATFORM=starfive FW_PAYLOAD_PATH=$(uboot) FW_FDT_PATH=$(uboot_dtb_file)
else
sbi: $(uboot) $(vmlinux)
	rm -rf $(sbi_wrkdir)
	mkdir -p $(sbi_wrkdir)
	cd $(sbi_wrkdir) && O=$(sbi_wrkdir) CFLAGS="-mabi=$(ABI) -march=$(ISA)" ${MAKE} -C $(sbi_srcdir) CROSS_COMPILE=$(CROSS_COMPILE) \
		PLATFORM=starfive FW_PAYLOAD_PATH=$(uboot) FW_FDT_PATH=$(uboot_dtb_file)
endif


$(fit): sbi $(vmlinux_bin) $(uboot) $(its_file) ${initramfs}
	$(uboot_wrkdir)/tools/mkimage -f $(its_file) -A riscv -O linux -T flat_dt $@
	@if [ -f fsz.sh ]; then ./fsz.sh $(sbi_bin); fi

$(libfesvr): $(fesvr_srcdir)
	rm -rf $(fesvr_wrkdir)
	mkdir -p $(fesvr_wrkdir)
	mkdir -p $(dir $@)
	cd $(fesvr_wrkdir) && $</configure \
		--prefix=$(dir $(abspath $(dir $@)))
	$(MAKE) -C $(fesvr_wrkdir)
	$(MAKE) -C $(fesvr_wrkdir) install
	touch -c $@

$(spike): $(spike_srcdir) $(libfesvr)
	rm -rf $(spike_wrkdir)
	mkdir -p $(spike_wrkdir)
	mkdir -p $(dir $@)
	cd $(spike_wrkdir) && PATH=$(RVPATH) $</configure \
		--prefix=$(dir $(abspath $(dir $@))) \
		--with-fesvr=$(dir $(abspath $(dir $(libfesvr))))
	$(MAKE) PATH=$(RVPATH) -C $(spike_wrkdir)
	$(MAKE) -C $(spike_wrkdir) install
	touch -c $@

$(qemu): $(qemu_srcdir)
	rm -rf $(qemu_wrkdir)
	mkdir -p $(qemu_wrkdir)
	mkdir -p $(dir $@)
	which pkg-config
	# pkg-config from buildroot blows up qemu configure
	cd $(qemu_wrkdir) && $</configure \
		--prefix=$(dir $(abspath $(dir $@))) \
		--target-list=riscv64-softmmu
	$(MAKE) -C $(qemu_wrkdir)
	$(MAKE) -C $(qemu_wrkdir) install
	touch -c $@

.PHONY: uboot-menuconfig
uboot-menuconfig: $(uboot_wrkdir)/.config
	$(MAKE) -C $(uboot_srcdir) O=$(dir $<) ARCH=riscv menuconfig
	$(MAKE) -C $(uboot_srcdir) O=$(dir $<) ARCH=riscv savedefconfig
	cp $(dir $<)defconfig $(uboot_defconfig)

$(uboot): $(uboot_srcdir) $(target_gcc)
	rm -rf $(uboot_wrkdir)
	mkdir -p $(uboot_wrkdir)
	mkdir -p $(dir $@)
	$(MAKE) -C $(uboot_srcdir) O=$(uboot_wrkdir) $(uboot_config)
	$(MAKE) -C $(uboot_srcdir) O=$(uboot_wrkdir) CROSS_COMPILE=$(CROSS_COMPILE)

$(rootfs): $(buildroot_rootfs_ext)
	cp $< $@

$(buildroot_initramfs_sysroot): $(buildroot_initramfs_sysroot_stamp)

.PHONY: buildroot_initramfs_sysroot vmlinux bbl fit
buildroot_initramfs_sysroot: $(buildroot_initramfs_sysroot)
vmlinux: $(vmlinux)
fit: $(fit)

.PHONY: clean
clean:
	rm -rf work/HiFive_U-Boot
	rm -rf work/opensbi
	rm work/vmlinux.bin
	rm work/hifive-unleashed-vfat.part
	rm work/image.fit
	rm work/initramfs.cpio.gz
	rm work/linux/vmlinux

.PHONY: distclean
distclean:
	rm -rf -- $(wrkdir) $(toolchain_dest)

.PHONY: sim
sim: $(spike) $(bbl_payload)
	$(spike) --isa=$(ISA) -p4 $(bbl_payload)

.PHONY: qemu
qemu: $(qemu) $(sbi) $(vmlinux) $(initramfs)
	$(qemu) -nographic -machine virt -bios $(bbl) -kernel $(vmlinux) -initrd $(initramfs) \
		-netdev user,id=net0 -device virtio-net-device,netdev=net0

.PHONY: qemu-rootfs
qemu-rootfs: $(qemu) $(bbl) $(vmlinux) $(initramfs) $(rootfs)
	$(qemu) -nographic -machine virt -bios $(bbl) -kernel $(vmlinux) -initrd $(initramfs) \
		-drive file=$(rootfs),format=raw,id=hd0 -device virtio-blk-device,drive=hd0 \
		-netdev user,id=net0 -device virtio-net-device,netdev=net0


.PHONY: uboot
uboot: $(uboot)

# Relevant partition type codes
BBL		= 2E54B353-1271-4842-806F-E436D6AF6985
VFAT            = EBD0A0A2-B9E5-4433-87C0-68B6B72699C7
LINUX		= 0FC63DAF-8483-4772-8E79-3D69D8477DE4
#FSBL		= 5B193300-FC78-40CD-8002-E86C45580B47
UBOOT		= 5B193300-FC78-40CD-8002-E86C45580B47
UBOOTENV	= a09354ac-cd63-11e8-9aff-70b3d592f0fa
UBOOTDTB	= 070dd1a8-cd64-11e8-aa3d-70b3d592f0fa
UBOOTFIT	= 04ffcafa-cd65-11e8-b974-70b3d592f0fa

flash.gpt: $(flash_image)

ifeq ($(TARGET_BOARD),JH7110)
VFAT_SIZE=263454
$(vfat_image): $(fit) $(confdir)/jh7110_uEnv.txt
	@if [ `du --apparent-size --block-size=512 $(uboot) | cut -f 1` -ge $(UBOOT_SIZE) ]; then \
		echo "Uboot is too large for partition!!\nReduce uboot or increase partition size"; \
		rm $(flash_image); exit 1; fi
	dd if=/dev/zero of=$(vfat_image) bs=512 count=$(VFAT_SIZE)
	/sbin/mkfs.vfat $(vfat_image)
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(fit) ::starfiveu.fit
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(confdir)/jh7110_uEnv.txt ::jh7110_uEnv.txt

else ifeq ($(TARGET_BOARD),U74)
VFAT_START=4096
VFAT_END=270335
VFAT_SIZE=266239
UBOOT_START=270336
UBOOT_END=272383
UBOOT_SIZE=2047
UENV_START=272384
UENV_END=274431
$(vfat_image): $(fit) $(confdir)/u74_uEnv.txt
	@if [ `du --apparent-size --block-size=512 $(uboot) | cut -f 1` -ge $(UBOOT_SIZE) ]; then \
		echo "Uboot is too large for partition!!\nReduce uboot or increase partition size"; \
		rm $(flash_image); exit 1; fi
	dd if=/dev/zero of=$(vfat_image) bs=512 count=$(VFAT_SIZE)
	/sbin/mkfs.vfat $(vfat_image)
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(fit) ::hifiveu.fit
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(confdir)/u74_uEnv.txt ::u74_uEnv.txt
else

VFAT_START=4096
VFAT_END=269502
VFAT_SIZE=263454
UBOOT_START=2048
UBOOT_END=4048
UBOOT_SIZE=2000
UENV_START=1024
UENV_END=1099

$(vfat_image): $(fit) $(confdir)/uEnv.txt
	@if [ `du --apparent-size --block-size=512 $(uboot) | cut -f 1` -ge $(UBOOT_SIZE) ]; then \
		echo "Uboot is too large for partition!!\nReduce uboot or increase partition size"; \
		rm $(flash_image); exit 1; fi
	dd if=/dev/zero of=$(vfat_image) bs=512 count=$(VFAT_SIZE)
	/sbin/mkfs.vfat $(vfat_image)
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(fit) ::hifiveu.fit
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(confdir)/uEnv.txt ::uEnv.txt
endif
$(flash_image): $(uboot) $(fit) $(vfat_image)
	dd if=/dev/zero of=$(flash_image) bs=1M count=32
	/sbin/sgdisk --clear  \
		--new=1:$(VFAT_START):$(VFAT_END)  --change-name=1:"Vfat Boot"	--typecode=1:$(VFAT)   \
		--new=2:$(UBOOT_START):$(UBOOT_END)   --change-name=2:uboot	--typecode=2:$(UBOOT) \
		--new=3:$(UENV_START):$(UENV_END)   --change-name=3:uboot-env	--typecode=3:$(UBOOTENV) \
		$(flash_image)
	dd conv=notrunc if=$(vfat_image) of=$(flash_image) bs=512 seek=$(VFAT_START)
	dd conv=notrunc if=$(uboot) of=$(flash_image) bs=512 seek=$(UBOOT_START) count=$(UBOOT_SIZE)

DEMO_END=11718750

#$(demo_image): $(uboot) $(fit) $(vfat_image) $(ext_image)
#	dd if=/dev/zero of=$(flash_image) bs=512 count=$(DEMO_END)
#	/sbin/sgdisk --clear  \
#		--new=1:$(VFAT_START):$(VFAT_END)  --change-name=1:"Vfat Boot"	--typecode=1:$(VFAT)   \
#		--new=3:$(UBOOT_START):$(UBOOT_END)   --change-name=3:uboot	--typecode=3:$(UBOOT) \
#		--new=2:264192:$(DEMO_END) --change-name=2:root	--typecode=2:$(LINUX) \
#		--new=4:1024:1247   --change-name=4:uboot-env	--typecode=4:$(UBOOTENV) \
#		$(flash_image)
#	dd conv=notrunc if=$(vfat_image) of=$(flash_image) bs=512 seek=$(VFAT_START)
#	dd conv=notrunc if=$(uboot) of=$(flash_image) bs=512 seek=$(UBOOT_START) count=$(UBOOT_SIZE)

.PHONY: format-boot-loader
format-boot-loader: $(bbl_bin) $(uboot) $(fit) $(vfat_image)
	@test -b $(DISK) || (echo "$(DISK): is not a block device"; exit 1)
	/sbin/sgdisk --clear  \
		--new=1:$(VFAT_START):$(VFAT_END)  --change-name=1:"Vfat Boot"	--typecode=1:$(VFAT)   \
		--new=2:$(UBOOT_START):$(UBOOT_END)   --change-name=2:uboot	--typecode=2:$(UBOOT) \
		--new=3:$(UENV_START):$(UENV_END)  --change-name=3:uboot-env	--typecode=3:$(UBOOTENV) \
		--new=4:274432:0 --change-name=4:root	--typecode=4:$(LINUX) \
		$(DISK)
	-/sbin/partprobe
	@sleep 1
ifeq ($(DISK)p1,$(wildcard $(DISK)p1))
	@$(eval PART1 := $(DISK)p1)
	@$(eval PART2 := $(DISK)p2)
	@$(eval PART3 := $(DISK)p3)
	@$(eval PART4 := $(DISK)p4)
else ifeq ($(DISK)s1,$(wildcard $(DISK)s1))
	@$(eval PART1 := $(DISK)s1)
	@$(eval PART2 := $(DISK)s2)
	@$(eval PART3 := $(DISK)s3)
	@$(eval PART4 := $(DISK)s4)
else ifeq ($(DISK)1,$(wildcard $(DISK)1))
	@$(eval PART1 := $(DISK)1)
	@$(eval PART2 := $(DISK)2)
	@$(eval PART3 := $(DISK)3)
	@$(eval PART4 := $(DISK)4)
else
	@echo Error: Could not find bootloader partition for $(DISK)
	@exit 1
endif
	dd if=$(uboot) of=$(PART2) bs=4096
	dd if=$(vfat_image) of=$(PART1) bs=4096

DEMO_IMAGE	:= sifive-debian-demo-mar7.tar.xz
DEMO_URL	:= https://github.com/tmagik/freedom-u-sdk/releases/download/hifiveu-2.0-alpha.1/

format-rootfs-image: format-boot-loader
	@echo "Done setting up basic initramfs boot. We will now try to install"
	@echo "a Debian snapshot to the Linux partition, which requires sudo"
	@echo "you can safely cancel here"
	/sbin/mke2fs -t ext4 $(PART4)
	-mkdir -p tmp-mnt
	-mkdir -p tmp-rootfs
	-sudo mount $(PART4) tmp-mnt && \
		sudo mount -o loop $(buildroot_rootfs_ext) tmp-rootfs&& \
		sudo cp -fr tmp-rootfs/* tmp-mnt/
	sudo umount tmp-mnt
	sudo umount tmp-rootfs
format-demo-image: format-boot-loader
	@echo "Done setting up basic initramfs boot. We will now try to install"
	@echo "a Debian snapshot to the Linux partition, which requires sudo"
	@echo "you can safely cancel here"
	/sbin/mke2fs -t ext4 $(PART4)
	-mkdir tmp-mnt
	-sudo mount $(PART4) tmp-mnt && cd tmp-mnt && \
		sudo wget $(DEMO_URL)$(DEMO_IMAGE) && \
		sudo tar -Jxvf $(DEMO_IMAGE)
	sudo umount tmp-mnt

ROOT_BEGIN=272384
# default size: 20GB
ROOT_CLUSTER_NUM=$(shell echo $$((20*1024*1024*1024/512)))
ROOT_END=$(shell echo $$(($(ROOT_BEGIN)+$(ROOT_CLUSTER_NUM))))

format-nvdla-disk: $(bbl_bin) $(uboot) $(fit) $(vfat_image)
	@test -b $(DISK) || (echo "$(DISK): is not a block device"; exit 1)
	/sbin/sgdisk --clear  \
		--new=1:$(VFAT_START):$(VFAT_END)  --change-name=1:"Vfat Boot"  --typecode=1:$(VFAT)   \
		--new=2:$(UBOOT_START):$(UBOOT_END)   --change-name=2:uboot --typecode=2:$(UBOOT) \
		--new=3:$(ROOT_BEGIN):0 --change-name=3:root  --typecode=3:$(LINUX) \
		$(DISK)
	-/sbin/partprobe
	@sleep 1
ifeq ($(DISK)p1,$(wildcard $(DISK)p1))
	@$(eval PART1 := $(DISK)p1)
	@$(eval PART2 := $(DISK)p2)
	@$(eval PART3 := $(DISK)p3)
else ifeq ($(DISK)s1,$(wildcard $(DISK)s1))
	@$(eval PART1 := $(DISK)s1)
	@$(eval PART2 := $(DISK)s2)
	@$(eval PART3 := $(DISK)s3)
else ifeq ($(DISK)1,$(wildcard $(DISK)1))
	@$(eval PART1 := $(DISK)1)
	@$(eval PART2 := $(DISK)2)
	@$(eval PART3 := $(DISK)3)
else
	@echo Error: Could not find bootloader partition for $(DISK)
	@exit 1
endif
	dd if=$(uboot) of=$(PART2) bs=4096
	dd if=$(vfat_image) of=$(PART1) bs=4096

#usb config

format-usb-disk: sbi $(uboot) $(fit) $(vfat_image)
	@test -b $(DISK) || (echo "$(DISK): is not a block device"; exit 1)
	/sbin/sgdisk --clear  \
	--new=1:0:+256M  --change-name=1:"Vfat Boot"  --typecode=1:$(VFAT)   \
	--new=2:0:+1G   --change-name=2:uboot --typecode=2:$(UBOOT) -g\
	$(DISK)
	-/sbin/partprobe
	@sleep 1
ifeq ($(DISK)p1,$(wildcard $(DISK)p1))
	@$(eval PART1 := $(DISK)p1)
	@$(eval PART2 := $(DISK)p2)
#	@$(eval PART3 := $(DISK)p3)
else ifeq ($(DISK)s1,$(wildcard $(DISK)s1))
	@$(eval PART1 := $(DISK)s1)
	@$(eval PART2 := $(DISK)s2)
#	@$(eval PART3 := $(DISK)s3)
else ifeq ($(DISK)1,$(wildcard $(DISK)1))
	@$(eval PART1 := $(DISK)1)
	@$(eval PART2 := $(DISK)2)
#	@$(eval PART3 := $(DISK)3)
else
	@echo Error: Could not find bootloader partition for $(DISK)
	@exit 1
endif
	dd if=$(uboot) of=$(PART2) bs=4096
	dd if=$(vfat_image) of=$(PART1) bs=4096

DEB_IMAGE := debian_nvdla_20190506.tar.xz
DEB_URL := https://github.com/sifive/freedom-u-sdk/releases/download/nvdla-demo-0.1

format-nvdla-rootfs: format-nvdla-disk
	@echo "Done setting up basic initramfs boot. We will now try to install"
	@echo "a Debian snapshot to the Linux partition, which requires sudo"
	@echo "you can safely cancel here"
	/sbin/mke2fs -t ext4 $(PART3)
	-mkdir -p tmp-mnt
	-mkdir -p tmp-rootfs
	-sudo mount $(PART3) tmp-mnt && \
		sudo mount -o loop $(buildroot_rootfs_ext) tmp-rootfs&& \
		sudo cp -fr tmp-rootfs/* tmp-mnt/
	sudo umount tmp-mnt
	sudo umount tmp-rootfs

format-nvdla-root: format-nvdla-disk
	@echo "Done setting up basic initramfs boot. We will now try to install"
	@echo "a Debian snapshot to the Linux partition, which requires sudo"
	@echo "you can safely cancel here"
	@test -e $(wrkdir)/$(DEB_IMAGE) || (wget -P $(wrkdir) $(DEB_URL)/$(DEB_IMAGE))
	/sbin/mke2fs -t ext4 $(PART3)
	-mkdir -p tmp-mnt
	-mount $(PART3) tmp-mnt && \
		echo "please wait until checkpoint reaches 489k" && \
		tar Jxf $(wrkdir)/$(DEB_IMAGE) -C tmp-mnt --checkpoint=1000
	umount tmp-mnt

-include $(initramfs).d
