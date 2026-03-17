#Usage:
# step1: mkdir build && cd build
# step2:
# local gcc: cmake ../
# riscv64 musl: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# arm32 musl: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-musleabihf/bin/arm-none-linux-musleabihf-
# step3: make
# step4: make install
# run: ./sample_file_recover.bin.static input.mov//需要修复文件和修复模板，例如input.mov, h264_pcm_template.bin

cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本号
project(sample_file_recover VERSION 0.0.1 LANGUAGES CXX)
#
set(COMPS_TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "components directory." FORCE)
include("${COMPS_TOP_DIR}/../common_files/components_common.cmake")
# 设置安装前缀（如果是默认路径）
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
  set(CMAKE_INSTALL_PREFIX "${COMPS_INSTALL_DIR}/osal" CACHE PATH "Installation directory prefix." FORCE)
  message(STATUS "Setting default CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endif()

# 指定源文件
set(SAMPLE_FILE_RECOVER_SOURCE_FILE sample_file_recover.cpp)

# 指定头文件目录
include_directories("${COMPS_TOP_DIR}/../common_files")
include_directories("${COMPS_INSTALL_DIR}/file_recover/include")

# 设置库路径和库文件
link_directories("${COMPS_INSTALL_DIR}/file_recover/lib")

# 生成可执行文件（动态链接）
add_executable(sample_file_recover ${SAMPLE_FILE_RECOVER_SOURCE_FILE})
target_link_libraries(sample_file_recover PRIVATE -lcomp_file_recover -lstdc++fs -lpthread)
set_target_properties(sample_file_recover PROPERTIES OUTPUT_NAME sample_file_recover.bin)

# 生成可执行文件（静态链接）
add_executable(sample_file_recover_static ${SAMPLE_FILE_RECOVER_SOURCE_FILE})
target_link_libraries(sample_file_recover_static PRIVATE -Wl,-gc-sections -Wl,-Bstatic -lcomp_file_recover)
target_link_libraries(sample_file_recover_static PRIVATE -Wl,-Bdynamic -lm -lpthread -lstdc++fs)
set_target_properties(sample_file_recover_static PROPERTIES OUTPUT_NAME sample_file_recover.bin.static)

# 设置安装目标
install(TARGETS sample_file_recover DESTINATION bin RENAME sample_file_recover.bin)
install(TARGETS sample_file_recover_static DESTINATION bin RENAME sample_file_recover.bin.static)

# 可选：添加编译选项或链接库（如果需要）
# target_compile_options(... PRIVATE -Wall -Wextra)
# target_link_libraries(... PRIVATE some_other_library)
