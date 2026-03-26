TOP_CONFIG_FILE ?=
#
include $(TOP_CONFIG_FILE)
#
include $(SRCTREE)/Kbuild
all:
	mkdir -p $(CURDIR)/bin
	chmod +x $(CURDIR)/bin -R
	python scripts/update_res.py all $(CURDIR) $(SRCTREE)/applications/dashcam/ui/$(CFG_PDT_SUB)/$(UI_PACKET)

clean:

clean_all:

install:
