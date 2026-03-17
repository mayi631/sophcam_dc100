# =============================================================================
# 工具链配置文件
# =============================================================================

# 架构支持配置（必须在 project() 之前）
set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture")
set(SUPPORTED_ARCHS "glibc_riscv64;glibc_arm32;glibc_aarch64;musl_arm32;musl_riscv64;x86_64")

# 检查架构是否支持
if(NOT TARGET_ARCH IN_LIST SUPPORTED_ARCHS)
    message(FATAL_ERROR "不支持的架构 '${TARGET_ARCH}'. 支持的架构: ${SUPPORTED_ARCHS}")
endif()

# 根据架构设置工具链文件
if(TARGET_ARCH STREQUAL "glibc_riscv64")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchains/riscv64-unknown-linux-gnu.cmake")
elseif(TARGET_ARCH STREQUAL "glibc_arm32")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchains/arm-none-linux-gnueabihf.cmake")
elseif(TARGET_ARCH STREQUAL "glibc_aarch64")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchains/aarch64-linux-gnu.cmake")
elseif(TARGET_ARCH STREQUAL "musl_arm32")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchains/arm-none-linux-musleabihf.cmake")
elseif(TARGET_ARCH STREQUAL "musl_riscv64")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchains/riscv64-unknown-linux-musl.cmake")
elseif(TARGET_ARCH STREQUAL "x86_64")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchains/x86_64-linux-gnu.cmake")
endif()

# 检查工具链文件是否存在
if(CMAKE_TOOLCHAIN_FILE AND NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "工具链文件不存在: ${CMAKE_TOOLCHAIN_FILE}")
endif()

message(STATUS "使用架构: ${TARGET_ARCH}")
message(STATUS "使用工具链文件: ${CMAKE_TOOLCHAIN_FILE}")
