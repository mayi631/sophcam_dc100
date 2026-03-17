# Usage:
# local host: make
# riscv64 musl: make CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
TOPTARGETS := all clean install clean_all

COMMON_DIR ?= $(CURDIR)/../common
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
INCS	+= -I$(COMMON_DIR)
#
SRCS_C	:= $(SDIR)/flash.c
SRCS_C	+= $(SDIR)/nand_raw.c
SRCS_C  += $(SDIR)/spi_raw.c
SRCS_C  += $(SDIR)/flash_fip.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcomp_flash.a
TARGET_SO	:= $(TARGET_DIR)/libcomp_flash.so
#
CFLAGS		+= -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable
#
#debug
CFLAGS      += -DDEBUG
#
include $(COMMON_DIR)/common_lib.mk
##
SUB_DIRS := flash_tool
$(TOPTARGETS): $(SUB_DIRS)
.PHONY: $(TOPTARGETS) $(SUB_DIRS)
$(SUB_DIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
