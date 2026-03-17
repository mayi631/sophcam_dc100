TOP_CONFIG_FILE ?=
#
include $(TOP_CONFIG_FILE)
include $(SRCTREE)/Kbuild

all:
	echo $(SRCTREE)
	# $(SRCTREE)/applications/tools/bmp2bitmap/./bmp2bitmap -f $(CURDIR)/bmp/cvitek.bmp -b 0xffff
	# mv bitmap.h $(SRCTREE)/applications/dashcam/media/include/

clean:
	echo $(SRCTREE)

clean_all:
	echo $(SRCTREE)

install:
	mkdir -p $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/click.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/keytone.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/photo.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/rec.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/AF22.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/shutter.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/touch.wav $(INSTALL_PATH)/bin/voice
	-cp -a $(CURDIR)/isp/$(INI_PDT)/* $(INSTALL_PATH)/bin/
ifeq ($(CONFIG_SERVICES_ADAS), y)
	cp -a $(CURDIR)/model/lane_det.cvimodel $(INSTALL_PATH)/bin/
	cp -a $(CURDIR)/model/vehicle_det.cvimodel $(INSTALL_PATH)/bin/
	cp -a $(CURDIR)/voice/distance.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/lane.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/moving.wav $(INSTALL_PATH)/bin/voice
	cp -a $(CURDIR)/voice/collision.wav $(INSTALL_PATH)/bin/voice
endif
ifeq ($(CONFIG_SERVICES_SPEECH), y)
	cp -a $(CURDIR)/model/sound_cardv_sr16k_v5_cv181x.cvimodel $(INSTALL_PATH)/bin/
endif
