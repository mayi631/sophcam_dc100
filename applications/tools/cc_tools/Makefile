TARGET_DIR	:= $(OUTPUT)/bin
TARGET		:= $(TARGET_DIR)/cc_tools

############################################
# Dependencies
############################################
INCS		+= -I$(SYSUTILS_DIR)/include
LIBS		+= -L$(SYSUTILS_DIR)/lib -lcomp_sysutils

INCS		+= -I$(OSAL_DIR)/include
LIBS		+= -L$(OSAL_DIR)/lib -lcomp_osal

DYN_LIBS	+= -lm -lstdc++ -pthread -ldl
############################################
# End of Dependencies
############################################
CFLAGS		+= -Wno-unused-parameter

SEXT		:= c
SDIR		:= $(CURDIR)
SRCS_C		:= $(wildcard $(SDIR)/*.c)
include $(COMMON_DIR)/common_lib.mk

clean:
	$(call print_target)
	$(Q)rm -rf $(TARGET_DIR)
	$(Q)rm -rf $(ODIR)
