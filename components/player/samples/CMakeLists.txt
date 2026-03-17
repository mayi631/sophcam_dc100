#Usage:
# step1: mkdir build && cd build
# step2:
# local gcc: cmake ../
# riscv64 musl: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# step3: make
# step4: make install
# run: ./sample_player.bin.static //put sample_player.static and sample.mov in same folder

cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本号
project(sample_player VERSION 0.0.1 LANGUAGES CXX)
#
set(COMPS_TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "components directory." FORCE)
include("${COMPS_TOP_DIR}/../common_files/components_common.cmake")
# 设置安装前缀（如果是默认路径）
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
  set(CMAKE_INSTALL_PREFIX "${COMPS_INSTALL_DIR}/osal" CACHE PATH "Installation directory prefix." FORCE)
  message(STATUS "Setting default CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endif()

# 指定源文件
set(SAMPLE_PLAYER_SOURCE_FILE sample_player.cpp)

# 指定头文件目录
include_directories("${COMPS_TOP_DIR}/../common_files")
include_directories("${COMPS_INSTALL_DIR}/demuxer/include")
include_directories("${COMPS_INSTALL_DIR}/player/include")
include_directories("${COMPS_INSTALL_DIR}/hal/include")
include_directories("${COMPS_INSTALL_DIR}/hal/src/screen/include")
include_directories("${COMPS_INSTALL_DIR}/hal/src/screen/comm/include")
include_directories("${COMPS_INSTALL_DIR}/mapi/include")
include_directories("${COMPS_INSTALL_DIR}/osal/include")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include")
include_directories("${OSS_INSTALL_DIR}/ffmpeg/include")

# 设置库路径和库文件
link_directories("${COMPS_INSTALL_DIR}/demuxer/lib")
link_directories("${COMPS_INSTALL_DIR}/player/lib")
link_directories("${COMPS_INSTALL_DIR}/hal/lib")
link_directories("${COMPS_INSTALL_DIR}/mapi/lib")
link_directories("${COMPS_INSTALL_DIR}/osal/lib")
link_directories("${COMPS_INSTALL_DIR}/cvi_mpi/lib")
link_directories("${COMPS_INSTALL_DIR}/cvi_mpi/3rd/lib")
link_directories("${OSS_INSTALL_DIR}/ffmpeg/lib")

set(MPI_LIB_DIR "${COMPS_INSTALL_DIR}/cvi_mpi/lib")
# 生成可执行文件（动态链接）
add_executable(sample_player ${SAMPLE_PLAYER_SOURCE_FILE})
target_link_libraries(sample_player PRIVATE -lcomp_player -lcomp_mem
  -lcomp_mapi -lcomp_hal -lcomp_demuxer -lcomp_osal -lavformat -lavcodec -lavutil -lswresample -lpthread -lm -ldl -lstdc++fs)
file(GLOB MPI_DYNAMIC_LIB_FILES "${MPI_LIB_DIR}/3rd/*.so" "${MPI_LIB_DIR}/*.so")
target_link_options(sample_player PRIVATE "LINKER:--whole-archive" ${MPI_DYNAMIC_LIB_FILES} "LINKER:--no-whole-archive")
set_target_properties(sample_player PROPERTIES OUTPUT_NAME sample_player.bin)

# 生成可执行文件（静态链接）
add_executable(sample_player_static ${SAMPLE_PLAYER_SOURCE_FILE})
target_link_libraries(sample_player_static PRIVATE -Wl,-gc-sections -Wl,-Bstatic -lcomp_player -lcomp_mem
  -lcomp_mapi -lcomp_hal -lcomp_demuxer -lcomp_osal -lavformat -lavcodec -lavutil -lswresample)
file(GLOB MPI_STATIC_LIB_FILES "${MPI_LIB_DIR}/3rd/*.a" "${MPI_LIB_DIR}/*.a")
set(FILE_TO_EXCLUDE "${MPI_LIB_DIR}/3rd/libcvi_json-c.a" "${MPI_LIB_DIR}/libdnvqe.a" "${MPI_LIB_DIR}/libcvi_ssp.a")
list(REMOVE_ITEM MPI_STATIC_LIB_FILES ${FILE_TO_EXCLUDE})
target_link_options(sample_player_static PRIVATE "LINKER:--whole-archive" ${MPI_STATIC_LIB_FILES} "LINKER:--no-whole-archive")
target_link_libraries(sample_player_static PRIVATE -Wl,-Bdynamic -lm -lpthread -lstdc++fs)
set_target_properties(sample_player_static PROPERTIES OUTPUT_NAME sample_player.bin.static)

# 设置安装目标
install(TARGETS sample_player DESTINATION bin RENAME sample_player.bin)
install(TARGETS sample_player_static DESTINATION bin RENAME sample_player.bin.static)

# 可选：添加编译选项或链接库（如果需要）
# target_compile_options(... PRIVATE -Wall -Wextra)
# target_link_libraries(... PRIVATE some_other_library)
