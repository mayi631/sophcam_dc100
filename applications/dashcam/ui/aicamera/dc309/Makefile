CONFIG ?= SDL
BUILD_DIR ?= build/x86_64
CMAKE_BUILD_TYPE ?= Debug
PLATFORM ?= x86_64

TOOLCHAIN_FILE =
ifeq ($(PLATFORM), musl_arm32)
	TOOLCHAIN_FILE = toolchains/arm-none-linux-musleabihf.cmake
	BUILD_DIR = build/musl_arm32
else ifeq ($(PLATFORM), glibc_arm32)
	TOOLCHAIN_FILE = toolchains/arm-linux-gnueabihf.cmake
	BUILD_DIR = build/glibc_arm32
else ifeq ($(PLATFORM), x86_64)
	TOOLCHAIN_FILE = toolchains/x86_64-linux-gnu.cmake
	BUILD_DIR = build/x86_64
endif

# 颜色定义
CYAN := \033[1;36m
YELLOW := \033[1;33m
GREEN := \033[1;32m
BLUE := \033[1;34m
RESET := \033[0m

.PHONY: all clean help lvgl

all: lv_conf_link
	cmake -B $(BUILD_DIR) -DCONFIG=$(CONFIG) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE)
	cmake --build $(BUILD_DIR) -j

lv_conf_link:
ifeq ($(CONFIG), FB)
	ln -sf config/lv_conf_fb.h lv_conf.h
else
	ln -sf config/lv_conf_sdl.h lv_conf.h
endif

lvgl: lv_conf_link
	mkdir -p third_party/lvgl/lib/$(PLATFORM)
	cmake -B build/lvgl_$(PLATFORM) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) -DLVGL_ONLY=ON
	cmake --build build/lvgl_$(PLATFORM) --target lvgl -j
	cp -ra build/lvgl_$(PLATFORM)/lvgl/lib/lib* third_party/lvgl/lib/$(PLATFORM)/

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "$(CYAN)Usage: make [target] [CONFIG=SDL|FB] [PLATFORM=x86_64|musl_arm32|glibc_arm32]$(RESET)"
	@echo " "
	@echo "$(YELLOW)Targets:$(RESET)"
	@echo "  $(GREEN)all$(RESET)        Build the project (default, CONFIG=SDL or CONFIG=FB, PLATFORM=x86_64)"
	@echo "  $(GREEN)clean$(RESET)      Remove build directory"
	@echo "  $(GREEN)help$(RESET)       Show this help message"
	@echo " "
	@echo "$(YELLOW)Examples:$(RESET)"
	@echo "  $(BLUE)make$(RESET)                # Build with SDL for x86_64 (default)"
	@echo "  $(BLUE)make CONFIG=FB$(RESET)      # Build with framebuffer/evdev support for x86_64"
	@echo "  $(BLUE)make PLATFORM=musl_arm32$(RESET) # Build for musl_arm32"
	@echo "  $(BLUE)make clean$(RESET)          # Clean build files"
	@echo " "
	@echo "$(YELLOW)Advanced Examples:$(RESET)"
	@echo "  $(BLUE)make CONFIG=FB PLATFORM=musl_arm32$(RESET)"
	@echo "  $(BLUE)make CONFIG=FB PLATFORM=glibc_arm32$(RESET)"
	@echo " "
