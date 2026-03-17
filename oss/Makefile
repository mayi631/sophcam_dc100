# Usage:
# local host: make
# riscv64 musl: make CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
.SHELL := /bin/bash
TOP_CONFIG_FILE ?=
#
include $(TOP_CONFIG_FILE)
#
# 包含定义 CFG_PDT_SUB 和 UI_PACKET 的 Kbuild 文件
include $(SRCTREE)/applications/Kbuild
#
CROSS_COMPILE ?=
CONFIG_OSS_FFMPEG ?= y
CONFIG_OSS_AWTKCODE ?= n
CONFIG_OSS_THTTPD ?= n
CONFIG_OSS_LVGL ?= n

ifeq ($(CONFIG_FB_PIX_DEPTH_32), y)
	DEPTH=32
else
	DEPTH=16
endif

ifeq ($(CONFIG_FB_ARGB4444), y)
    FORMAT = ARGB4444
else
    FORMAT = ARGB888
endif

ifeq ($(CONFIG_FB_DUAL_BUFFER), y)
    BUFFER = 2
else
    BUFFER = 1
endif

ifeq ($(CONFIG_TOUCHPAD), y)
	TOUCHPAD=y
else
	TOUCHPAD=n
endif

ifeq ($(findstring riscv64, $(CONFIG_CROSS_COMPILE)), riscv64)
	ARCH_RISCV=y
else
	ARCH_RISCV=n
endif

ifeq ($(CONFIG_CV184X), y)
	BOARD=CV184XH
else
	BOARD=CV181X
endif

define print_target
	@printf '\033[1;36;40m [TARGET][OSS] $@ \033[0m\n'
endef

.PHONY: all clean clean_all install
.PHONY: ffmpeg awtkcode thttpd lvgl

all: ffmpeg awtkcode thttpd lvgl
	$(call print_target)

ffmpeg:
# 使用时间戳文件跟踪构建状态，只有当构建脚本更新或 stamp 文件不存在时才重新编译
ifeq ($(CONFIG_OSS_FFMPEG),y)
	$(call print_target)
	$(Q)if [ ! -f $(CURDIR)/ffmpeg/build.stamp ] || [ $(CURDIR)/build_ffmpeg.sh -nt $(CURDIR)/ffmpeg/build.stamp ]; then \
		$(Q)cd $(CURDIR)/ffmpeg && bash $(CURDIR)/build_ffmpeg.sh && cd - && \
		$(Q)touch $(CURDIR)/ffmpeg/build.stamp; \
	$(Q)fi
endif

awtkcode:
ifeq ($(CONFIG_OSS_AWTKCODE),y)
	$(call print_target)
	$(Q)if [ ! -f $(CURDIR)/awtkcode/build.stamp ] || [ $(CURDIR)/build_awtk.sh -nt $(CURDIR)/awtkcode/build.stamp ]; then \
		$(Q)CROSS_COMPILE=$(CROSS_COMPILE) FB_DEPTH=$(DEPTH) TOUCHPAD=$(TOUCHPAD) \
			RISCV=$(ARCH_RISCV) BOARD=$(BOARD) FB_FORMAT=$(FORMAT) FB_BUFFER=$(BUFFER) $(CURDIR)/build_awtk.sh && \
		$(Q)touch $(CURDIR)/awtkcode/build.stamp; \
	$(Q)fi
endif

thttpd:
ifeq ($(CONFIG_OSS_THTTPD),y)
	$(call print_target)
	$(Q)mkdir -p $(CURDIR)/thttpd/build/lib
	$(Q)mkdir -p $(CURDIR)/thttpd/include
	$(Q)mkdir -p $(CURDIR)/thttpd/lib
	$(Q)if [ ! -f $(CURDIR)/thttpd/build.stamp ] || [ $(CURDIR)/build_thttpd.sh -nt $(CURDIR)/thttpd/build.stamp ]; then \
		$(Q)BUILD_DIR=$(CURDIR)/thttpd/build/lib INSTALL_DIR=./install \
			CROSS_COMPILE_3RD=$(CROSS_COMPILE) $(CURDIR)/build_thttpd.sh && \
		$(Q)touch $(CURDIR)/thttpd/build.stamp; \
	$(Q)fi
endif

# 使用时间戳文件跟踪构建状态，三种情况才会重新编译 lvgl:
# 1. 构建脚本 build_lvgl.sh 有更新
# 2. stamp 文件不存在时才重新编译
# 3. lv_conf.h 文件和 lvgl/lv_conf.h 不一致
UI_CONFIG=$(SRCTREE)/applications/dashcam/ui/${CFG_PDT_SUB}/${UI_PACKET}/config/lv_conf_fb.h
lvgl:
ifeq ($(CONFIG_OSS_LVGL),y)
	$(call print_target)
	$(Q)if [ ! -f $(CURDIR)/lvgl/build.stamp ] || \
		   [ $(CURDIR)/build_lvgl.sh -nt $(CURDIR)/lvgl/build.stamp ] || \
		   ! cmp -s $(UI_CONFIG) $(CURDIR)/lvgl/lv_conf.h; then \
		$(Q)cp $(UI_CONFIG) $(CURDIR)/lvgl/lv_conf.h; \
		$(Q)export LVGL_C_FLAGS="-I${SRCTREE}/applications/dashcam/ui/${CFG_PDT_SUB}/${UI_PACKET}/third_party/freetype/freetype2" && \
		$(Q)cd $(CURDIR)/lvgl && bash $(CURDIR)/build_lvgl.sh && cd - && \
		$(Q)touch $(CURDIR)/lvgl/build.stamp; \
	$(Q)fi
endif

clean:
	$(call print_target)
	# oss do nothing for command clean

clean_all:
	$(call print_target)
ifeq ($(CONFIG_OSS_FFMPEG),y)
	$(Q)cd $(CURDIR)/ffmpeg && make clean && cd -
	$(Q)rm -rf $(CURDIR)/ffmpeg/build
	$(Q)rm -rf $(CURDIR)/ffmpeg/include
	$(Q)rm -rf $(CURDIR)/ffmpeg/lib
	$(Q)rm -rf $(CURDIR)/ffmpeg/bin
	$(Q)rm -rf $(CURDIR)/ffmpeg/share
	$(Q)rm -rf $(CURDIR)/ffmpeg/install
	$(Q)rm -f $(CURDIR)/ffmpeg/build.stamp
endif
ifeq ($(CONFIG_OSS_AWTKCODE),y)
	$(Q)rm -rf $(CURDIR)/awtkcode/awtk-linux-fb/build
	$(Q)rm -rf $(CURDIR)/awtkcode/build.stamp
endif
ifeq ($(CONFIG_OSS_THTTPD),y)
	$(Q)rm -rf $(CURDIR)/thttpd/include
	$(Q)rm -rf $(CURDIR)/thttpd/lib
	$(Q)rm -rf $(CURDIR)/thttpd/build
	$(Q)rm -f $(CURDIR)/thttpd/build.stamp
endif
ifeq ($(CONFIG_OSS_LVGL),y)
	$(Q)rm -rf $(CURDIR)/lvgl/build
	$(Q)rm -f $(CURDIR)/lv_conf.h
	$(Q)rm -rf $(CURDIR)/lvgl/install
	$(Q)rm -f $(CURDIR)/lvgl/build.stamp
endif

install:
	$(call print_target)
	mkdir -p $(INSTALL_PATH)/lib
ifeq ($(CONFIG_OSS_FFMPEG),y)
	$(Q)cp -rf $(CURDIR)/ffmpeg/install/$(PROCESSED_CONFIG_CROSS_COMPILE)/lib/*.* $(INSTALL_PATH)/lib
endif
ifeq ($(CONFIG_OSS_AWTKCODE),y)
	$(Q)cp -a $(CURDIR)/awtkcode/awtk-linux-fb/build/lib/libawtk.so $(INSTALL_PATH)/lib
endif
ifeq ($(CONFIG_OSS_THTTPD),y)
	$(Q)cp -a $(CURDIR)/thttpd/lib/libthttpd.so $(INSTALL_PATH)/lib
endif
ifeq ($(ARCH_RISCV), y)
	$(Q)cp -a $(CURDIR)/csi2d/lib/* $(INSTALL_PATH)/lib
endif
ifeq ($(CONFIG_OSS_LVGL),y)
	$(Q)cp -a $(CURDIR)/lvgl/install/$(PROCESSED_CONFIG_CROSS_COMPILE)/lib/*.* $(INSTALL_PATH)/lib
endif
