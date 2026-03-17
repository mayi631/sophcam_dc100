COMMON_DIR ?= $(CURDIR)/../common_files

TOP_CONFIG_FILE ?=
#
include $(TOP_CONFIG_FILE)
#
CVI_MPI_DIR := $(CONFIG_CVI_PLATFORM_DIR)/cvi_mpi

TOPTARGETS := all clean install clean_all

components-y 												+= osal
components-y												+= sysutils
components-y												+= ini
components-y												+= flash_tool
components-$(CONFIG_COMPONENTS_HAL)							+= hal
components-$(CONFIG_COMPONENTS_RINGBUFFER)					+= ringbuffer
components-$(CONFIG_COMPONENTS_MUXER)						+= muxer
components-$(CONFIG_COMPONENTS_DEMUXER)						+= demuxer
components-$(CONFIG_COMPONENTS_THUMBNAIL_EXTRACTOR)			+= thumbnail_extractor
components-$(CONFIG_COMPONENTS_FILE_RECOVER)				+= file_recover
components-$(CONFIG_COMPONENTS_MAPI)						+= mapi
components-$(CONFIG_COMPONENTS_PLAYER)						+= player
components-$(CONFIG_COMPONENTS_RECORDER)					+= recorder
components-$(CONFIG_COMPONENTS_ONVIF)						+= onvif
components-$(CONFIG_COMPONENTS_PERF)						+= perf
components-$(CONFIG_COMPONENTS_RTSP)						+= rtsp
components-$(CONFIG_COMPONENTS_DTCF)						+= dtcf
components-$(CONFIG_COMPONENTS_UPGRADE)						+= upgrade
components-$(CONFIG_COMPONENTS_STORAGE)						+= storage
components-$(CONFIG_COMPONENTS_FSTOOL)						+= fstool
ifeq ($(CVI_MPI_DIR),)
components-y												+= cvi_mpi
endif

COMPONENTS_LIB_DIR := $(CURDIR)/comps_install/$(PROCESSED_CONFIG_CROSS_COMPILE)

SUBDIRS += $(components-y)
$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	@if [ "$@" = "cvi_mpi" ]; then \
		if [ "$(MAKECMDGOALS)" = "clean_all" ]; then \
			echo "Skipping clean_all for cvi_mpi"; \
		else \
			$(MAKE) -C $@ $(MAKECMDGOALS) -j20 CHIP_ARCH=CV181X CROSS_COMPILE=$(CONFIG_CROSS_COMPILE); \
		fi; \
	else \
		mkdir -p $@/build && \
		cd $@/build && \
		if [ "$(MAKECMDGOALS)" = "install" ]; then \
			mkdir -p $(INSTALL_PATH)/lib && \
			cp -d $(COMPONENTS_LIB_DIR)/$@/lib/*.so* $(INSTALL_PATH)/lib && \
			cd -; \
		elif [ "$(MAKECMDGOALS)" = "clean" ]; then \
			: ; \
		elif [ "$(MAKECMDGOALS)" = "clean_all" ]; then \
			if [ "$@" = "hal" ] || [ "$@" = "mapi" ]; then \
				rm -rf * && \
				cd -; \
			fi; \
		else \
			if [ "$@" = "hal" ]; then \
				cmake ../ -DGCC_CROSS_COMPILE=${CONFIG_CROSS_COMPILE} -DENABLE_184X=$(CONFIG_CV184X) -DCONFIG_ADC=$(CONFIG_ADC) -DCONFIG_GPIO=$(CONFIG_GPIO) \
				-DCONFIG_PWM=$(CONFIG_PWM) -DCONFIG_LED=$(CONFIG_LED) -DCONFIG_KEY=$(CONFIG_KEY) -DCONFIG_WATCHDOG=$(CONFIG_WATCHDOG) \
				-DCONFIG_UART=$(CONFIG_UART) -DCONFIG_TOUCHPAD=$(CONFIG_TOUCHPAD) -DCONFIG_TOUCHPAD_GT9XX=$(CONFIG_TOUCHPAD_GT9XX) \
				-DCONFIG_GSENSOR=$(CONFIG_GSENSOR) -DCONFIG_GSENSOR_DA280=$(CONFIG_GSENSOR_DA280) -DCONFIG_GSENSOR_SC7A20E=$(CONFIG_GSENSOR_SC7A20E) \
				-DCONFIG_WIFI=$(CONFIG_WIFI) -DCONFIG_WIFI_HI3881=$(CONFIG_WIFI_HI3881) -DCONFIG_WIFI_AP6201BM=$(CONFIG_WIFI_AP6201BM) \
				-DCONFIG_WIFI_RTL8188=$(CONFIG_WIFI_RTL8188) -DCONFIG_WIFI_AIC8800=$(CONFIG_WIFI_AIC8800) -DCONFIG_WIFI_RTL8189=$(CONFIG_WIFI_RTL8189) \
				-DCONFIG_SCREEN=$(CONFIG_SCREEN) -DCONFIG_SCREEN_OTA7290B_MIPI11INCH=$(CONFIG_SCREEN_OTA7290B_MIPI11INCH) -DCONFIG_SCREEN_HX8394=$(CONFIG_SCREEN_HX8394) \
				-DCONFIG_SCREEN_ICN9707=$(CONFIG_SCREEN_ICN9707) -DCONFIG_SCREEN_ST7701S=$(CONFIG_SCREEN_ST7701S) -DCONFIG_SCREEN_GC9503V=$(CONFIG_SCREEN_GC9503V) \
				-DCONFIG_SCREEN_GC9503V_W820=$(CONFIG_SCREEN_GC9503V_W820) -DCONFIG_SCREEN_NV3049E=$(CONFIG_SCREEN_NV3049E) -DCONFIG_SCREEN_EK79007=$(CONFIG_SCREEN_EK79007) \
				-DCONFIG_SCREEN_I80ST7789=$(CONFIG_SCREEN_I80ST7789) -DCONFIG_SCREEN_JD9165A=$(CONFIG_SCREEN_JD9165A) -DCONFIG_KOMOD_PATH=$(CONFIG_KOMOD_PATH); \
				$(MAKE) $(MAKECMDGOALS) && \
				$(MAKE) install && \
				cd -; \
			elif [ "$@" = "mapi" ]; then \
				cmake ../ -DGCC_CROSS_COMPILE=${CONFIG_CROSS_COMPILE} -DENABLE_ISP_PQ_TOOL=$(CONFIG_ENABLE_ISP_PQ_TOOL) -DENABLE_184X=$(CONFIG_CV184X) -DENABLE_VO_INIT=$(CONFIG_ENABLE_VO_INIT); \
				$(MAKE) $(MAKECMDGOALS) && \
				$(MAKE) install && \
				cd -; \
			else \
				cd -; \
			fi; \
		fi; \
	fi


.PHONY: $(TOPTARGETS) $(SUBDIRS)
