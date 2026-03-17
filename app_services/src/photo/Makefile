COMMON_DIR ?= $(CURDIR)/../common
include $(COMMON_DIR)/base.mk


TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_photo_service.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_photo_service.so

############################################
# Dependencies
############################################
INCS		+= -I$(PLATFORM_MMF_PATH)/include
MAPI_DIR	?= $(CURDIR)/../components/mapi
INCS		+= -I$(MAPI_DIR)/include

DTCF_DIR	?= $(CURDIR)/../components/dtcf
INCS		+= -I$(DTCF_DIR)/include

EXIF_DIR	?= $(CURDIR)/../components/exif
INCS		+= -I$(EXIF_DIR)/include

OSAL_DIR	?= $(CURDIR)/../components/osal
INCS		+= -I$(OSAL_DIR)/include

CVI_LOG_DIR		?= $(CURDIR)/../components/cvi_log
INCS		+= -I$(CVI_LOG_DIR)/include

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
SRCS_C      := $(SDIR)/photo_service.c

include $(COMMON_DIR)/common_lib.mk
