include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcar_ui.a
TARGET_SO	:= $(TARGET_DIR)/libcar_ui.so

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter -Wno-unused-function -Wno-error

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk

$(INSTALL_PATH)/lib :
	mkdir -p $@

install: $(TARGET_A) $(TARGET_SO) | $(INSTALL_PATH)/lib
	cp $^ $(INSTALL_PATH)/lib
	cp -a $(CURDIR)/res $(INSTALL_PATH)/bin/

-include $(DEPS)
