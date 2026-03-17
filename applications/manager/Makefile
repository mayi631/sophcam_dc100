TOP_CONFIG_FILE ?=
#
include $(TOP_CONFIG_FILE)
#
include $(SRCTREE)/Kbuild
CVI_MPI_DIR := $(CONFIG_CVI_PLATFORM_DIR)/cvi_mpi

##
SDIR		:= $(CURDIR)/src
# ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
SRCS_C		:= $(SDIR)/vol/volmng.c
# endif
SRCS_C		+= $(SDIR)/appcomm/appcomm.c
ifeq ($(CONFIG_SERVICES_VIDEO_MD), y)
SRCS_C		+= $(SDIR)/md/mdmng.c
endif
SRCS_C		+= $(SDIR)/temper/tempermng.c
ifeq ($(CONFIG_SERVICES_PHOTO), y)
SRCS_C		+= $(SDIR)/photo/photomng.c
endif
SRCS_C		+= $(SDIR)/file/filemng.c
ifeq ($(CONFIG_SERVICES_PLAYER), y)
SRCS_C		+= $(SDIR)/playback/playbackmng.c
endif
SRCS_C		+= $(SDIR)/powercontrol/powercontrol.c
ifeq ($(CONFIG_GPS), y)
SRCS_C		+= $(SDIR)/gps/gps_data_analysis.c
SRCS_C		+= $(SDIR)/gps/gpsmng.c
endif
SRCS_C		+= $(SDIR)/record/recordmng.c
SRCS_C		+= $(SDIR)/system/system.c
ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
SRCS_C		+= $(SDIR)/liveview/liveviewmng.c
endif
ifeq ($(CONFIG_SERVICES_ADAS), y)
SRCS_C		+= $(SDIR)/adas/adasmng.c
endif
SRCS_C		+= $(SDIR)/storage/storagemng.c
SRCS_C		+= $(SDIR)/timedtask/timedtask.c
ifeq ($(CONFIG_WATCHDOG), y)
SRCS_C		+= $(SDIR)/dev/watchdogmng.c
endif
ifeq ($(CONFIG_LED), y)
SRCS_C		+= $(SDIR)/dev/ledmng.c
endif
ifeq ($(CONFIG_GSENSOR), y)
SRCS_C		+= $(SDIR)/dev/gsensormng.c
endif
ifeq ($(CONFIG_WIFI), y)
SRCS_C		+= $(SDIR)/dev/wifimng.c
endif
ifeq ($(CONFIG_ADC), y)
SRCS_C		+= $(SDIR)/dev/gaugemng.c
endif
ifeq ($(CONFIG_KEY),y)
SRCS_C		+= $(SDIR)/dev/keymng.c
endif
SRCS_C		+= $(SDIR)/cmd/cmdmng.c
SRCS_C		+= $(SDIR)/usb/usb.c
SRCS_C		+= $(SDIR)/usb/usb_storage.c
SRCS_C		+= $(SDIR)/usb/uvc.c
SRCS_C		+= $(SDIR)/usb/uvc_gadget.c
ifeq ($(CONFIG_SERVICES_SPEECH), y)
SRCS_C		+= $(SDIR)/speech/speechmng.c
endif
ifeq ($(CONFIG_APPLICATION_NET), y)
SRCS_C		+= $(SDIR)/cvinet/net.c
endif
#
APP_SERVICE_DIR			:= $(SRCTREE)/app_services
CVI_PARAM_DIR			:= $(SRCTREE)/applications/dashcam/param/core
MANAGEER_DIR			:= $(SRCTREE)/applications/manager
CVI_MPI_DIR			?= $(SRCTREE)/components/cvi_mpi
MODE_DIR			:= $(SRCTREE)/applications/dashcam/mode
NETCTRL_DIR			:= $(SRCTREE)/applications/dashcam/netctrl
MEDIA_DIR			:= $(SRCTREE)/applications/dashcam/media

# include
INCS			+= -I$(SRCTREE)/applications/include
INCS			+= -I$(MODE_DIR)/include
INCS			+= -I$(NETCTRL_DIR)/include
INCS			+= -I$(MEDIA_DIR)/include
INCS			+= -I$(CVI_MPI_DIR)/include
INCS			+= -I$(CVI_MPI_DIR)/include/isp
INCS			+= -I$(MANAGEER_DIR)/include
INCS			+= -I$(OSAL_DIR)/include
INCS			+= -I$(SYSUTILS_DIR)/include
INCS			+= -I$(MAPI_DIR)/include
INCS			+= -I$(APP_SERVICE_DIR)/include
INCS			+= -I$(DTCF_DIR)/include
INCS			+= -I$(STORAGE_DIR)/include
INCS			+= -I$(HAL_DIR)/include
INCS			+= -I$(RECORDER_DIR)/include
INCS			+= -I$(MUXER_DIR)/include
INCS			+= -I$(CVI_PARAM_DIR)/include
INCS			+= -I$(COMMON_DIR)/
INCS			+= -I$(THTTPD_DIR)/include
INCS			+= -I$(FILE_RECOVER_DIR)/include
INCS			+= -I$(PLAYER_DIR)/include
INCS			+= -I$(DEMUXER_DIR)/include
INCS			+= -I$(TDL_SDK_DIR)/include/cvi_tdl
INCS			+= -I$(FLASH_TOOL_DIR)/include

#
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libapp_mng.a
TARGET_SO	:= $(TARGET_DIR)/libapp_mng.so
#

CFLAGS += -Wno-stringop-truncation -Wno-format-truncation \
		-Wno-type-limits -Wno-enum-conversion -Wno-implicit-fallthrough \
		-Wno-stringop-overflow -Wno-sizeof-pointer-memaccess \
		-Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-function
#

CFLAGS += $(KBUILD_DEFINES)
CXXFLAGS += $(KBUILD_DEFINES)
include $(COMMON_DIR)/common_lib.mk

clean:
	$(call print_target)
	$(Q)rm -rf $(TARGET_DIR)
	$(Q)rm -rf $(ODIR)
