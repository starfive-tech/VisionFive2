ISA ?= rv64imafdc_zicsr_zifencei_zba_zbb
ABI ?= lp64d

#TARGET_BOARD is JH7110 or NULL
SOC := JH7110
BOARD_FLAGS	:=
HWBOARD ?= visionfive2
HWBOARD_FLAG ?= HWBOARD_VISIONFIVE2

srcdir := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
srcdir := $(srcdir:/=)
confdir := $(srcdir)/conf
wrkdir := $(CURDIR)/work
ampwrkdir := $(CURDIR)/work/amp

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
ifeq ($(HWBOARD), evb)
	linux_defconfig := $(linux_srcdir)/arch/riscv/configs/starfive_jh7110_defconfig
else
	linux_defconfig := $(linux_srcdir)/arch/riscv/configs/starfive_visionfive2_defconfig
endif

vmlinux := $(linux_wrkdir)/vmlinux
vmlinux_stripped := $(linux_wrkdir)/vmlinux-stripped
vmlinux_bin := $(wrkdir)/vmlinux.bin
module_install_path:=$(wrkdir)/module_install_path
perf_tool_srcdir := $(linux_srcdir)/tools/perf
perf_tool_wrkdir := $(linux_wrkdir)/tools/perf

its_file=$(confdir)/$(HWBOARD)-fit-image.its
uboot_its_file=$(confdir)/$(HWBOARD)-uboot-fit-image.its
uboot_amp_its_file=$(confdir)/$(HWBOARD)-uboot-amp-fit-image.its
amp_its_file=$(confdir)/$(HWBOARD)-amp-fit-image.its

vfat_image := $(wrkdir)/starfive-$(HWBOARD)-vfat.part
amp_vfat_image := $(ampwrkdir)/starfive-$(HWBOARD)-vfat.part
#ext_image := $(wrkdir)  # TODO

initramfs := $(wrkdir)/initramfs.cpio.gz

sbi_srcdir := $(srcdir)/opensbi
sbi_wrkdir := $(wrkdir)/opensbi

sbi_bin := $(wrkdir)/opensbi/platform/generic/firmware/fw_payload.bin
ampsbi_bin := $(ampwrkdir)/opensbi/platform/generic/firmware/fw_payload.bin

fit := $(wrkdir)/image.fit
uboot_fit := $(wrkdir)/$(HWBOARD)_fw_payload.img
ampfit := $(ampwrkdir)/image.fit
ampuboot_fit := $(wrkdir)/$(HWBOARD)_fw_payload_amp.img

fesvr_srcdir := $(srcdir)/riscv-fesvr
fesvr_wrkdir := $(wrkdir)/riscv-fesvr
libfesvr := $(fesvr_wrkdir)/prefix/lib/libfesvr.so

spike_srcdir := $(srcdir)/riscv-isa-sim
spike_wrkdir := $(wrkdir)/riscv-isa-sim
spike := $(spike_wrkdir)/prefix/bin/spike

qemu_srcdir := $(srcdir)/riscv-qemu
qemu_wrkdir := $(wrkdir)/riscv-qemu
qemu := $(qemu_wrkdir)/prefix/bin/qemu-system-riscv64

uboot_srcdir := $(srcdir)/u-boot
uboot_wrkdir := $(wrkdir)/u-boot
rt_thread_wrkdir := $(srcdir)/rtthread/bsp/starfive/jh7110
uboot_amp_wrkdir := $(wrkdir)/amp/u-boot

uboot_dtb_file := $(wrkdir)/u-boot/arch/riscv/dts/starfive_$(HWBOARD).dtb
uboot_amp_dtb_file := $(ampwrkdir)/u-boot/arch/riscv/dts/starfive_jh7110-amp.dtb

amp_dts	       := starfive_jh7110-amp
uboot          := $(uboot_wrkdir)/u-boot.bin
uboot_amp_orig := $(ampwrkdir)/u-boot/u-boot.bin
uboot_amp      := $(ampwrkdir)/u-boot.bin
ampsbi_wrkdir  := $(ampwrkdir)/opensbi
rtos_file      := rtthread.bin
rtos_elf       := rtthread.elf
rtos_map       := rtthread.map
amp_uboot_size := 1216K
rtos_compile   := scons
rtos_defconfig := vf2_defconfig
rtos_boardfile := vf2_rtconfig.h

spl_tool_srcdir := $(srcdir)/soft_3rdpart/spl_tool
spl_tool_wrkdir := $(wrkdir)/spl_tool

spl_bin_normal_out := u-boot-spl.bin.normal.out
amp_spl_bin_normal_out := u-boot-amp-spl.bin.normal.out

uboot_config := starfive_$(HWBOARD)_defconfig

uboot_defconfig := $(uboot_srcdir)/configs/$(uboot_config)
rootfs := $(wrkdir)/rootfs.bin

target_gcc := $(CROSS_COMPILE)gcc
version := $(wrkdir)/version

.PHONY: all check_arg

all: check_arg $(fit) $(vfat_image) $(uboot_fit) $(spl_bin_normal_out)
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
ifeq ( , $(filter $(HWBOARD), visionfive2 evb fpga))
	$(error board $(HWBOARD) is not supported, BOARD=[visionfive2 | evb | fpga(deflault)])
endif

# TODO: depracated for now
#ifneq ($(RISCV),$(buildroot_initramfs_wrkdir)/host)
#$(target_gcc):
#	$(error The RISCV environment variable was set, but is not pointing at a toolchain install tree)
#else
#$(target_gcc): $(buildroot_initramfs_tar)
#endif

.PHONY: visionfive2 evb fpga

visionfive2: HWBOARD := visionfive2
visionfive2: HWBOARD_FLAG := HWBOARD_VISIONFIVE2
visionfive2: uboot_config := starfive_$(HWBOARD)_defconfig
visionfive2: uboot_dtb_file := $(wrkdir)/u-boot/arch/riscv/dts/starfive_$(HWBOARD).dtb
visionfive2: linux_defconfig := $(linux_srcdir)/arch/riscv/configs/starfive_visionfive2_defconfig
visionfive2: uboot_fit := $(wrkdir)/$(HWBOARD)_fw_payload.img
visionfive2: vfat_image := $(wrkdir)/starfive-$(HWBOARD)-vfat.part
visionfive2: its_file=$(confdir)/$(HWBOARD)-fit-image.its
visionfive2: uboot_its_file=$(confdir)/$(HWBOARD)-uboot-fit-image.its
visionfive2: all

evb: HWBOARD := evb
evb: HWBOARD_FLAG := HWBOARD_EVB
evb: uboot_config := starfive_$(HWBOARD)_defconfig
evb: uboot_dtb_file := $(wrkdir)/u-boot/arch/riscv/dts/starfive_$(HWBOARD).dtb
evb: linux_defconfig := $(linux_srcdir)/arch/riscv/configs/starfive_jh7110_defconfig
evb: uboot_fit := $(wrkdir)/$(HWBOARD)_fw_payload.img
evb: vfat_image := $(wrkdir)/starfive-$(HWBOARD)-vfat.part
evb: its_file=$(confdir)/$(HWBOARD)-fit-image.its
evb: uboot_its_file=$(confdir)/$(HWBOARD)-uboot-fit-image.its
evb: all

fpga: HWBOARD := fpga
fpga: all
fpga: HWBOARD_FLAG := HWBOARD_FPGA

$(version):
	cd $(linux_srcdir) && \
	sh $(confdir)/version > $(version)
	chmod 777 $(version)

$(buildroot_initramfs_wrkdir)/.config: $(buildroot_srcdir)
#	rm -rf $(dir $@)
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
#	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	cp $(buildroot_rootfs_config) $@
	$(MAKE) -C $< RISCV=$(RISCV) PATH=$(RVPATH) O=$(buildroot_rootfs_wrkdir) olddefconfig

$(buildroot_rootfs_ext): $(buildroot_srcdir) $(buildroot_rootfs_wrkdir)/.config $(target_gcc) $(buildroot_rootfs_config) $(version)	
	mkdir -p $(buildroot_rootfs_wrkdir)/target/lib
	cp -r $(module_install_path)/lib/modules $(buildroot_rootfs_wrkdir)/target/lib/
	mkdir -p $(buildroot_rootfs_wrkdir)/target/usr/bin
	cp $(perf_tool_wrkdir)/perf $(buildroot_rootfs_wrkdir)/target/usr/bin/
	cp $(version) $(buildroot_rootfs_wrkdir)/target/usr/bin/version
	$(MAKE) -C $< RISCV=$(RISCV) PATH=$(RVPATH) O=$(buildroot_rootfs_wrkdir)

.PHONY: buildroot_rootfs
buildroot_rootfs: $(buildroot_rootfs_ext)

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
	$(MAKE) -C $(linux_srcdir) O=$(linux_wrkdir) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=riscv olddefconfig

$(uboot_wrkdir)/.config: $(uboot_defconfig)
	mkdir -p $(dir $@)
	cp -p $< $@
	$(MAKE) -C $(uboot_srcdir) O=$(uboot_wrkdir) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=riscv olddefconfig

$(vmlinux): $(linux_srcdir) $(linux_wrkdir)/.config $(target_gcc)
	$(MAKE) -C $< O=$(linux_wrkdir) \
		ARCH=riscv \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		PATH=$(RVPATH) \
		vmlinux \
		all \
		modules
	$(MAKE) -C $< O=$(linux_wrkdir) \
		ARCH=riscv \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		PATH=$(RVPATH) \
		INSTALL_MOD_PATH=$(module_install_path) \
		modules_install

vpudriver-build: $(vmlinux)
	$(MAKE) -C $(buildroot_initramfs_wrkdir) O=$(buildroot_initramfs_wrkdir) \
		INSTALL_MOD_PATH=$(module_install_path) wave511driver
	$(MAKE) -C $(buildroot_initramfs_wrkdir) O=$(buildroot_initramfs_wrkdir) \
		INSTALL_MOD_PATH=$(module_install_path) wave420ldriver
	$(MAKE) -C $(buildroot_initramfs_wrkdir) O=$(buildroot_initramfs_wrkdir) \
		INSTALL_MOD_PATH=$(module_install_path) codaj12driver

$(perf_tool_wrkdir)/perf: $(linux_srcdir) $(vmlinux)
	mkdir -p $(perf_tool_wrkdir)
	$(MAKE) -C $(perf_tool_srcdir) O=$(perf_tool_wrkdir)/ \
		ARCH=riscv \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		WERROR=0 NO_GTK2=1 NO_LIBPERL=1 \
		NO_LIBBIONIC=1 NO_LIBELF=1 NO_LIBTRACEEVENT=1

.PHONY: perf
perf: $(perf_tool_wrkdir)/perf

.PHONY: initrd
initrd: $(initramfs)

$(initramfs).d: $(buildroot_initramfs_sysroot)
	$(linux_srcdir)/usr/gen_initramfs_list.sh -l $(confdir)/initramfs.txt $(buildroot_initramfs_sysroot) > $@

$(initramfs): $(buildroot_initramfs_sysroot) $(vmlinux) vpudriver-build $(version) $(perf_tool_wrkdir)/perf
	cp -r $(module_install_path)/lib/modules $(buildroot_initramfs_sysroot)/lib/
	cp $(perf_tool_wrkdir)/perf $(buildroot_initramfs_sysroot)/usr/bin/
	cp $(version) $(buildroot_initramfs_sysroot)/usr/bin/version && \
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
	$(MAKE) -C $(linux_srcdir) O=$(dir $<) ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE) menuconfig
	$(MAKE) -C $(linux_srcdir) O=$(dir $<) ARCH=riscv CROSS_COMPILE=$(CROSS_COMPILE) savedefconfig
	cp $(dir $<)defconfig $(linux_defconfig)

# Note: opensbi generic platform default FW_TEXT_START is 0x80000000
#     For JH7110, need to specify the FW_TEXT_START to 0x40000000
#     Otherwise, the fw_payload.bin downloading via jtag will not run.
#     not affect the evb_fw_payload.img for its file has FW_TEXT_START
$(sbi_bin): $(uboot) $(vmlinux)
	rm -rf $(sbi_wrkdir)
	mkdir -p $(sbi_wrkdir)
	cd $(sbi_wrkdir) && O=$(sbi_wrkdir) CFLAGS="-mabi=$(ABI) -march=$(ISA)" ${MAKE} -C $(sbi_srcdir) CROSS_COMPILE=$(CROSS_COMPILE) \
		PLATFORM=generic FW_PAYLOAD_PATH=$(uboot) FW_FDT_PATH=$(uboot_dtb_file) FW_TEXT_START=0x40000000

$(fit): $(sbi_bin) $(vmlinux_bin) $(uboot) $(its_file) ${initramfs}
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
	$(MAKE) -C $(uboot_srcdir) O=$(dir $<) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=riscv menuconfig
	$(MAKE) -C $(uboot_srcdir) O=$(dir $<) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=riscv savedefconfig
	cp $(dir $<)defconfig $(uboot_defconfig)

$(uboot): $(uboot_srcdir) $(target_gcc)
	rm -rf $(uboot_wrkdir)
	mkdir -p $(uboot_wrkdir)
	mkdir -p $(dir $@)
	$(MAKE) -C $(uboot_srcdir) O=$(uboot_wrkdir) $(uboot_config)
	$(MAKE) -C $(uboot_srcdir) O=$(uboot_wrkdir) CROSS_COMPILE=$(CROSS_COMPILE)

$(spl_tool_wrkdir)/spl_tool: $(spl_tool_srcdir)
	rm -rf $(spl_tool_wrkdir)
	cp -ar $< $(spl_tool_wrkdir)
	$(MAKE) -C $(spl_tool_wrkdir)

.PHONY: spl_tool
spl_tool: $(spl_tool_wrkdir)/spl_tool

$(spl_bin_normal_out): $(uboot) $(spl_tool_wrkdir)/spl_tool
	$(spl_tool_wrkdir)/spl_tool -c -f $(uboot_wrkdir)/spl/u-boot-spl.bin
	cp $(uboot_wrkdir)/spl/$(spl_bin_normal_out) $(wrkdir)

$(uboot_fit): $(sbi_bin) $(uboot_its_file) $(uboot)
	$(uboot_wrkdir)/tools/mkimage -f $(uboot_its_file) -A riscv -O u-boot -T firmware $(uboot_fit)

$(uboot_amp_orig): $(uboot_srcdir) $(target_gcc)
	rm -rf $(uboot_amp_wrkdir)
	mkdir -p $(uboot_amp_wrkdir)
	mkdir -p $(dir $@)
	$(MAKE) DEVICE_TREE=$(amp_dts) -C $(uboot_srcdir) O=$(uboot_amp_wrkdir) $(uboot_config)
	$(MAKE) DEVICE_TREE=$(amp_dts) -C $(uboot_srcdir) O=$(uboot_amp_wrkdir) CROSS_COMPILE=$(CROSS_COMPILE)
	cp $(rt_thread_wrkdir)/configs/$(rtos_defconfig) $(rt_thread_wrkdir)/.config
	cp $(rt_thread_wrkdir)/configs/$(rtos_boardfile) $(rt_thread_wrkdir)/rtconfig.h
	cd $(rt_thread_wrkdir) && $(rtos_compile) -c && $(rtos_compile)
	cd -
	cp $(rt_thread_wrkdir)/$(rtos_file) $(ampwrkdir)
	cp $(rt_thread_wrkdir)/$(rtos_elf) $(ampwrkdir)
	cp $(rt_thread_wrkdir)/$(rtos_map) $(ampwrkdir)
	cp $(uboot_amp_orig) $(uboot_amp)
	truncate $(uboot_amp) -c -s $(amp_uboot_size)
	cat $(ampwrkdir)/$(rtos_file) >> $(uboot_amp)

$(ampsbi_bin): $(uboot_amp_orig)
	rm -rf $(ampsbi_wrkdir)
	mkdir -p $(ampsbi_wrkdir)
	cd $(ampsbi_wrkdir) && O=$(ampsbi_wrkdir) CFLAGS="-mabi=$(ABI) -march=$(ISA)" ${MAKE} -C $(sbi_srcdir) CROSS_COMPILE=$(CROSS_COMPILE) \
		PLATFORM=generic FW_PAYLOAD_PATH=$(uboot_amp) FW_FDT_PATH=$(uboot_amp_dtb_file) FW_TEXT_START=0x40000000

$(ampuboot_fit): $(ampsbi_bin) $(uboot_amp_its_file) $(amp_uboot) $(spl_tool_wrkdir)/spl_tool
	$(spl_tool_wrkdir)/spl_tool -c -f $(uboot_amp_wrkdir)/spl/u-boot-spl.bin
	cp $(uboot_amp_wrkdir)/spl/$(spl_bin_normal_out) $(wrkdir)/$(amp_spl_bin_normal_out)
	$(uboot_amp_wrkdir)/tools/mkimage -f $(uboot_amp_its_file) -A riscv -O u-boot -T firmware $(ampuboot_fit)

$(ampfit): $(ampsbi_bin) $(vmlinux_bin) $(ampuboot_fit) $(amp_its_file) ${initramfs}
	$(uboot_wrkdir)/tools/mkimage -f $(amp_its_file) -A riscv -O linux -T flat_dt $@
	@if [ -f fsz.sh ]; then ./fsz.sh $(sbi_bin); fi

$(amp_vfat_image): $(ampfit) $(vfat_image)
	cp $(vfat_image) $(amp_vfat_image)
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(amp_vfat_image) -o $(ampfit) ::starfiveu.fit

$(rootfs): $(buildroot_rootfs_ext)
	cp $< $@

$(buildroot_initramfs_sysroot): $(buildroot_initramfs_sysroot_stamp)

.PHONY: buildroot_initramfs_sysroot vmlinux fit uboot_fit
buildroot_initramfs_sysroot: $(buildroot_initramfs_sysroot)
vmlinux: $(vmlinux)
fit: $(fit)
uboot_fit: $(uboot_fit)

.PHONY: ampuboot_fit ampfit
ampuboot_fit: $(ampuboot_fit)
ampfit: $(ampfit)
amp_vfat_image: $(amp_vfat_image)

.PHONY: amp-clean
amp-clean:
	rm -rf work/amp
	rm -f work/u-boot-amp-spl.bin.normal.out
	rm -f work/visionfive2_fw_payload_amp.img

.PHONY: clean
clean:
	rm -rf work/u-boot
	rm -rf work/opensbi
	rm -f work/*-vfat.part
	rm -f work/image.fit
	rm -f work/*_fw_payload.img
	rm -f work/initramfs.cpio.gz
	rm -f work/linux/vmlinux*
	rm -f work/u-boot-spl.bin.normal.out
	rm -f work/version
	rm -rf $(spl_tool_wrkdir)
	rm -rf $(perf_tool_wrkdir)

.PHONY: distclean
distclean:
	rm -rf -- $(wrkdir) $(toolchain_dest)

.PHONY: sim
sim: $(spike) $(sbi_bin)
	$(spike) --isa=$(ISA) -p4 $(sbi_bin)

.PHONY: qemu
qemu: $(qemu) $(sbi_bin) $(vmlinux) $(initramfs)
	$(qemu) -nographic -machine virt -bios $(sbi_bin) -kernel $(vmlinux) -initrd $(initramfs) \
		-netdev user,id=net0 -device virtio-net-device,netdev=net0

.PHONY: qemu-rootfs
qemu-rootfs: $(qemu) $(sbi_bin) $(vmlinux) $(initramfs) $(rootfs)
	$(qemu) -nographic -machine virt -bios $(sbi_bin) -kernel $(vmlinux) -initrd $(initramfs) \
		-drive file=$(rootfs),format=raw,id=hd0 -device virtio-blk-device,drive=hd0 \
		-netdev user,id=net0 -device virtio-net-device,netdev=net0


.PHONY: uboot
uboot: $(uboot)

# Relevant partition type codes with GUID
SPL         = 2E54B353-1271-4842-806F-E436D6AF6985
VFAT        = EBD0A0A2-B9E5-4433-87C0-68B6B72699C7
LINUX       = 0FC63DAF-8483-4772-8E79-3D69D8477DE4
UBOOT       = 5B193300-FC78-40CD-8002-E86C45580B47
UBOOTENV    = a09354ac-cd63-11e8-9aff-70b3d592f0fa
UBOOTDTB    = 070dd1a8-cd64-11e8-aa3d-70b3d592f0fa
UBOOTFIT    = 04ffcafa-cd65-11e8-b974-70b3d592f0fa

# Note: The following are the sector number used to partition
#   The default sector size is 512 Bytes
#   The partition start should be align on 2048-sector boundaries
# expand the vfat size to 300+M for the vpu/jpu or other debug
SPL_START   = 4096
SPL_END     = 8191
UBOOT_START = 8192
UBOOT_END   = 16383
UBOOT_SIZE  = $(shell expr $(UBOOT_END) - $(UBOOT_START) + 1)
VFAT_START  = 16384
VFAT_END    = 614399
VFAT_SIZE   = $(shell expr $(VFAT_END) - $(VFAT_START) + 1)
ROOT_START  = 614400

$(vfat_image): $(fit) $(confdir)/jh7110_uEnv.txt $(confdir)/vf2_uEnv.txt $(confdir)/vf2_nvme_uEnv.txt
	@if [ `du --apparent-size --block-size=512 $(uboot) | cut -f 1` -ge $(UBOOT_SIZE) ]; then \
		echo "Uboot is too large for partition!!\nReduce uboot or increase partition size"; \
		 exit 1; fi
	dd if=/dev/zero of=$(vfat_image) bs=512 count=$(VFAT_SIZE)
	/sbin/mkfs.vfat $(vfat_image)
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(fit) ::starfiveu.fit
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(confdir)/jh7110_uEnv.txt ::jh7110_uEnv.txt
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(confdir)/vf2_uEnv.txt ::vf2_uEnv.txt
	PATH=$(RVPATH) MTOOLS_SKIP_CHECK=1 mcopy -i $(vfat_image) $(confdir)/vf2_nvme_uEnv.txt ::vf2_nvme_uEnv.txt

.PHONY: format-boot-loader
format-boot-loader: $(sbi_bin) $(uboot) $(fit) $(vfat_image) $(spl_bin_normal_out)
	@test -b $(DISK) || (echo "$(DISK): is not a block device"; exit 1)
	sudo /sbin/sgdisk --clear  \
		--new=1:$(SPL_START):$(SPL_END)     --change-name=1:"spl"   --typecode=1:$(SPL)   \
		--new=2:$(UBOOT_START):$(UBOOT_END) --change-name=2:"uboot" --typecode=2:$(UBOOT) \
		--new=3:$(VFAT_START):$(VFAT_END)   --change-name=3:"image" --typecode=3:$(VFAT)  \
		--new=4:$(ROOT_START):0             --change-name=4:"root"  --typecode=4:$(LINUX) \
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
	sudo dd if=$(spl_bin_normal_out) of=$(PART1) bs=4096
	sudo dd if=$(uboot_fit)          of=$(PART2) bs=4096
	sudo dd if=$(vfat_image)         of=$(PART3) bs=4096
	sync; sleep 1;

#starfive image
format-rootfs-image: format-boot-loader
	@echo "Done setting up basic initramfs boot. We will now try to install"
	@echo "a Debian snapshot to the Linux partition, which requires sudo"
	@echo "you can safely cancel here"
	sudo /sbin/mke2fs -t ext4 $(PART4)
	-mkdir -p tmp-mnt
	-mkdir -p tmp-rootfs
	-sudo mount $(PART4) tmp-mnt && \
		sudo mount -o loop $(buildroot_rootfs_ext) tmp-rootfs&& \
		sudo cp -fr tmp-rootfs/* tmp-mnt/
	sync; sleep 1;
	sudo umount tmp-mnt
	sudo umount tmp-rootfs
	rmdir tmp-mnt
	rmdir tmp-rootfs

.PHONY: sdimg img
sdimg: $(buildroot_rootfs_ext)
	@./genimage.sh visionfive2

img: sdimg $(spl_tool_wrkdir)/spl_tool
	$(spl_tool_wrkdir)/spl_tool -i -f $(wrkdir)/sdcard.img

.PHONY: sd_amp amp_img
sd_amp: $(buildroot_rootfs_ext) $(amp_vfat_image) $(img)
	@./genimage.sh vf2-amp

amp_img: sd_amp $(spl_tool_wrkdir)/spl_tool
	$(spl_tool_wrkdir)/spl_tool -i -f $(wrkdir)/sdcard_amp.img

#usb config
format-usb-disk: $(sbi_bin) $(uboot) $(fit) $(vfat_image)
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
else ifeq ($(DISK)s1,$(wildcard $(DISK)s1))
	@$(eval PART1 := $(DISK)s1)
	@$(eval PART2 := $(DISK)s2)
else ifeq ($(DISK)1,$(wildcard $(DISK)1))
	@$(eval PART1 := $(DISK)1)
	@$(eval PART2 := $(DISK)2)
else
	@echo Error: Could not find bootloader partition for $(DISK)
	@exit 1
endif
	dd if=$(uboot) of=$(PART2) bs=4096
	dd if=$(vfat_image) of=$(PART1) bs=4096


DEB_IMAGE 	:= debian_nvdla_20190506.tar.xz
DEB_URL 	:= https://github.com/sifive/freedom-u-sdk/releases/download/nvdla-demo-0.1

DEMO_IMAGE	:= sifive-debian-demo-mar7.tar.xz
DEMO_URL	:= https://github.com/tmagik/freedom-u-sdk/releases/download/hifiveu-2.0-alpha.1/

#nvdla image
format-nvdla-image: format-boot-loader
	@echo "Done setting up basic initramfs boot. We will now try to install"
	@echo "a Debian snapshot to the Linux partition, which requires sudo"
	@echo "you can safely cancel here"
	@test -e $(wrkdir)/$(DEB_IMAGE) || (wget -P $(wrkdir) $(DEB_URL)/$(DEB_IMAGE))
	/sbin/mke2fs -t ext4 $(PART4)
	-mkdir -p tmp-mnt
	-mount $(PART4) tmp-mnt && \
		echo "please wait until checkpoint reaches 489k" && \
		tar Jxf $(wrkdir)/$(DEB_IMAGE) -C tmp-mnt --checkpoint=1000
	umount tmp-mnt

#demo image
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

-include $(initramfs).d
