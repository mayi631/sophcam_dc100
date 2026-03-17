include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_speech.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_speech.so

############################################
# Dependencies
############################################
AUDIO_DIR   := $(SRCTREE)/framework/services/audio
INCS		+= -I$(AUDIO_DIR)/include

MAPI_DIR	:= $(SRCTREE)/framework/mapi
INCS		+= -I$(MAPI_DIR)/include

MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

SYSUTILS_DIR := $(SRCTREE)/cpsl/common/sysutils
INCS         += -I$(SYSUTILS_DIR)/include

AI_DIR		:= $(SRCTREE)/cpsl/tpu/cvitek_ai_sdk
INCS		+= -I$(AI_DIR)/include/cvi_tdl
INCS		+= -I$(AI_DIR)/include/cvi_tdl_app
INCS		+= -I$(AI_DIR)/sample/3rd/opencv/include

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
