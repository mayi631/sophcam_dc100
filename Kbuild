#
# Kbuild for top-level directory of the application
#
KBUILD_DEFINES += -DDUAL_OS

ifneq ($(CONFIG_CROSS_COMPILE),)
CROSS_COMPILE := $(CONFIG_CROSS_COMPILE:"%"=%)
CVI_MPI_DIR := $(CONFIG_CVI_PLATFORM_DIR)/cvi_mpi
endif

ifdef SOPHCAM_COMMIT_ID
KBUILD_DEFINES += -DSOPHCAM_COMMIT_ID=\"$(SOPHCAM_COMMIT_ID)\"
endif

ifneq ($(strip $(CONFIG_KOMOD_PATH)),)
	KBUILD_DEFINES += -DKOMOD_PATH=\"$(CONFIG_KOMOD_PATH)\"
endif

ifeq ($(CONFIG_ENABLE_ISP_PQ_TOOL), y)
	KBUILD_DEFINES += -DENABLE_ISP_PQ_TOOL
endif

ifeq ($(CONFIG_CV184X), y)
	KBUILD_DEFINES += -DCHIP_184X
endif

include $(SRCTREE)/applications/Kbuild
include $(SRCTREE)/components/Kbuild
include $(SRCTREE)/app_services/Kbuild
