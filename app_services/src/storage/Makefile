include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_storage.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_storage.so

############################################
# Dependencies
############################################
MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

QUEUE_DIR	:= $(SRCTREE)/cpsl/common/queue
INCS		+= -I$(QUEUE_DIR)/include

STG_DIR		:= $(SRCTREE)/cpsl/common/storage
INCS		+= -I$(STG_DIR)/include

SYSUTILS_DIR    := $(SRCTREE)/cpsl/common/sysutils
INCS            += -I$(SYSUTILS_DIR)/include
############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
