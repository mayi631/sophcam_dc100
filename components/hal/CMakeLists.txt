#Usage:
# step1: mkdir build && cd build
# step2:
# local gcc: cmake ../
# riscv64 musl: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# default used for linux; if used for alios, add  -DCONFIG_KERNEL_RHINO
# step3: make
# step4: make install

# 定义项目名称、版本号、语言、源文件、头文件目录、库路径、库文件、安装目录等信息
cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本号
project(hal VERSION 0.0.1 LANGUAGES C)
#
set(COMPS_TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "components directory." FORCE)
include("${COMPS_TOP_DIR}/../common_files/components_common.cmake")
# 设置安装前缀（如果是默认路径）
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
  set(CMAKE_INSTALL_PREFIX "${COMPS_INSTALL_DIR}/${PROJECT_NAME}" CACHE PATH "Installation directory prefix" FORCE)
  message(STATUS "Setting default CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endif()

# 检查 GCC_CROSS_COMPILE 是否包含 "arm-none-linux-musleabihf-"
string(FIND "${GCC_CROSS_COMPILE}" "arm-none-linux-musleabihf-" FOUND_ARM_MUSL)

if(FOUND_ARM_MUSL GREATER -1)
    message(STATUS "Detected arm-none-linux-musleabihf- toolchain")
    add_definitions(-DUSING_ARM_MUSL_TOOLCHAIN)
else()
    message(STATUS "arm-none-linux-musleabihf- toolchain not detected")
endif()

if(DEFINED CONFIG_KOMOD_PATH AND NOT "${CONFIG_KOMOD_PATH}" STREQUAL "")
    set(KO_PATH "${CONFIG_KOMOD_PATH}")
    message(STATUS "KOMOD_PATH defined as: ${CONFIG_KOMOD_PATH}")
else()
    set(KO_PATH "/system/ko")
    message(WARNING "CONFIG_KOMOD_PATH is not defined or is empty. Using default: ${KO_PATH}")
endif()
add_definitions(-DKOMOD_PATH=\"${KO_PATH}\")

option(ENABLE_184X "Enable DUAL_OS compile definition" OFF)
## ---------------------------------------------------------------------- ##
# 指定源文件
set(CONFIG_ADC "y" CACHE BOOL "Enable adc support")
set(CONFIG_BACKLIGHT "y" CACHE BOOL "Enable backlight support")
set(CONFIG_GPIO "y" CACHE BOOL "Enable gpio support")
set(CONFIG_PWM "y" CACHE BOOL "Enable pwm support")
set(CONFIG_LED "y" CACHE BOOL "Enable led support")
set(CONFIG_KEY "y" CACHE BOOL "Enable key support")
set(CONFIG_WATCHDOG "y" CACHE BOOL "Enable watchdog support")
set(CONFIG_UART "y" CACHE BOOL "Enable uart support")

if("${CONFIG_ADC}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/adc/hal_adc.c)
endif()
if("${CONFIG_BACKLIGHT}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/backlight/hal_backlight.c)
endif()
if("${CONFIG_GPIO}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/gpio/hal_gpio.c)
endif()
if("${CONFIG_PWM}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/pwm/hal_pwm.c)
endif()
if("${CONFIG_LED}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/led/hal_led.c)
endif()
if("${CONFIG_KEY}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/key/hal_key.c)
endif()
if("${CONFIG_WATCHDOG}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/watchdog/hal_watchdog.c)
endif()
if("${CONFIG_UART}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/uart/hal_uart.c)
endif()

#################### gsensor
set(CONFIG_GSENSOR "n" CACHE BOOL "Enable gsensor support")
set(CONFIG_GSENSOR_DA280 "n" CACHE BOOL "Enable da280 gsensor support")
set(CONFIG_GSENSOR_SC7A20E "n" CACHE BOOL "Enable sc7a20e gsensor support")

if("${CONFIG_GSENSOR}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/gsensor/hal_gsensor.c)

if("${CONFIG_GSENSOR_DA280}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/gsensor/da280/mir3da.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/gsensor/da280")
elseif("${CONFIG_GSENSOR_SC7A20E}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/gsensor/sc7a20e/sc7a20e.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/gsensor/sc7a20e")
endif()

endif()

################### wifi
set(CONFIG_WIFI "n" CACHE BOOL "Enable wifi support")
set(CONFIG_WIFI_RTL8189 "n" CACHE BOOL "Enable rtl8189fs wifi support")
set(CONFIG_WIFI_HI3881 "n" CACHE BOOL "Enable hi3881 wifi support")
set(CONFIG_WIFI_AP6201BM "n" CACHE BOOL "Enable ap6201bm wifi support")
set(CONFIG_WIFI_RTL8188 "n" CACHE BOOL "Enable rtl8188 wifi support")
set(CONFIG_WIFI_AIC8800 "n" CACHE BOOL "Enable aic8800 wifi support")

if("${CONFIG_WIFI}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/wifi/common/hal_wifi_ap.c
  src/wifi/common/hal_wifi_utils.c
  src/wifi/common/hal_wifi.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/wifi/common")

if("${CONFIG_WIFI_RTL8189}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/wifi/rtl8189fs/wifi_hal.c)
elseif("${CONFIG_WIFI_HI3881}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/wifi/hi3881/wifi_hal.c)
elseif("${CONFIG_WIFI_AP6201BM}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/wifi/ap6201bm/wifi_hal.c)
elseif("${CONFIG_WIFI_RTL8188}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/wifi/rtl8188fu/wifi_hal.c)
elseif("${CONFIG_WIFI_AIC8800}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/wifi/aic8800/wifi_hal.c)
endif()

endif()

##################### 屏幕和触摸屏
set(CONFIG_SCREEN "n" CACHE BOOL "Enable screen support")
set(CONFIG_SCREEN_OTA7290B_MIPI11INCH "n" CACHE BOOL "Enable ota7290b_mipi11inch screen support")
set(CONFIG_SCREEN_HX8394 "n" CACHE BOOL "Enable hx8394 screen support")
set(CONFIG_SCREEN_ICN9707 "n" CACHE BOOL "Enable icn9707 screen support")
set(CONFIG_SCREEN_ST7701S "n" CACHE BOOL "Enable st7701s screen support")
set(CONFIG_SCREEN_GC9503V "n" CACHE BOOL "Enable gc9503v screen support")
set(CONFIG_SCREEN_GC9503V_W820 "n" CACHE BOOL "Enable gc9503v_w820 screen support")
set(CONFIG_SCREEN_NV3049E "n" CACHE BOOL "Enable nv3049e screen support")
set(CONFIG_SCREEN_EK79007 "n" CACHE BOOL "Enable ek79007 screen support")
set(CONFIG_SCREEN_I80ST7789 "n" CACHE BOOL "Enable i80st7789 screen support")
set(CONFIG_SCREEN_JD9165A "n" CACHE BOOL "Enable jd9165a screen support")

set(CONFIG_TOUCHPAD "n" CACHE BOOL "Enable touchpad support")
set(CONFIG_TOUCHPAD_GT9XX "n" CACHE BOOL "Enable gt9xx touchpad support")

if("${CONFIG_SCREEN}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/comm/src/hal_screen_comm.c
  src/screen/mipidsi/src/screen_mipidsi.c)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/comm/include"
                    "${CMAKE_CURRENT_SOURCE_DIR}/src/screen/mipidsi/include")

if("${CONFIG_SCREEN_OTA7290B_MIPI11INCH}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/ota7290b_mipi11inch/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/ota7290b_mipi11inch")
elseif("${CONFIG_SCREEN_HX8394}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/hx8394_mipi/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/hx8394_mipi")
elseif("${CONFIG_SCREEN_ICN9707}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/icn9707/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/icn9707")
elseif("${CONFIG_SCREEN_ST7701S}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/st7701s/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/st7701s")
elseif("${CONFIG_SCREEN_GC9503V}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/gc9503v/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/gc9503v")
elseif("${CONFIG_SCREEN_GC9503V_W820}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/gc9503v_w820/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/gc9503v_w820")
elseif("${CONFIG_SCREEN_NV3049E}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/nv3049e/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/nv3049e")
elseif("${CONFIG_SCREEN_EK79007}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/ek79007/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/ek79007")
elseif("${CONFIG_SCREEN_I80ST7789}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/i80st7789/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/i80st7789")
elseif("${CONFIG_SCREEN_JD9165A}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES}
  src/screen/jd9165a/hal_screen.c)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/screen/jd9165a")
endif()

# 取决于屏幕的触摸屏
if("${CONFIG_TOUCHPAD}" STREQUAL "y")
if("${CONFIG_TOUCHPAD_GT9XX}" STREQUAL "y")
set(SOURCE_CODE_FILES ${SOURCE_CODE_FILES} src/touchpad/gt9xx/hal_touchpad.c)
if("${CONFIG_SCREEN_OTA7290B_MIPI11INCH}" STREQUAL "y")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCONFIG_SCREEN_OTA7290B_MIPI11INCH")
endif()
endif()
endif()

endif()
## -------------------------------------------------------------------- ##

# 指定头文件目录
include_directories("${COMPS_TOP_DIR}/../common_files")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${COMPS_INSTALL_DIR}/osal/include")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include/linux")

# 设置库路径和库文件
#link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")

# 生成库文件
add_library(comp_hal SHARED ${SOURCE_CODE_FILES})
add_library(comp_hal_static STATIC ${SOURCE_CODE_FILES})
set_target_properties(comp_hal PROPERTIES
    VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME comp_hal PREFIX "lib" SUFFIX ".so")
set_target_properties(comp_hal_static PROPERTIES OUTPUT_NAME comp_hal PREFIX "lib" SUFFIX ".a")

# 设置安装目标
install(TARGETS comp_hal DESTINATION lib)
install(TARGETS comp_hal_static DESTINATION lib)

# 安装头文件
file(GLOB PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
install(FILES ${PUBLIC_HEADERS} DESTINATION include)

# 可选：添加编译选项或链接库（如果需要）
# target_compile_options(... PRIVATE -Wall -Wextra)
# target_link_libraries(... PRIVATE some_other_library)

if(ENABLE_184X)
    target_compile_definitions(comp_hal PRIVATE CHIP_184X)
    target_compile_definitions(comp_hal_static PRIVATE CHIP_184X)
endif()

if(CONFIG_PWM)
  target_compile_definitions(comp_hal PRIVATE CONFIG_PWM_ON)
  target_compile_definitions(comp_hal_static PRIVATE CONFIG_PWM_ON)
endif()