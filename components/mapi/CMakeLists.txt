#Usage:
# step1: mkdir build && cd build
# step2:
# local gcc: cmake ../
# riscv64 musl: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# default used for v4.2.0; if used for v4.1.0, add  -DENABLE_DUAL_OS=OFF
# step3: make
# step4: make install

# 定义项目名称、版本号、语言、源文件、头文件目录、库路径、库文件、安装目录等信息
cmake_minimum_required(VERSION 3.16)

# 设置项目名称和版本号
project(mapi VERSION 0.0.1 LANGUAGES C CXX)
#
set(COMPS_TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "components directory." FORCE)
include("${COMPS_TOP_DIR}/../common_files/components_common.cmake")
# 设置安装前缀（如果是默认路径）
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
  set(CMAKE_INSTALL_PREFIX "${COMPS_INSTALL_DIR}/${PROJECT_NAME}" CACHE PATH "Installation directory prefix" FORCE)
  message(STATUS "Setting default CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endif()
## -------------------------------------- ##
option(ENABLE_184X "Enable DUAL_OS compile definition" OFF)
# 指定源文件
set(SOURCE_CODE_FILES
    src/audio_aac_adp.c
    src/audio_dl_adp.c
    src/mapi_acap.c
    src/mapi_adec.c
    src/mapi_aenc.c
    src/mapi_ao.c
    src/mapi_disp.c
    src/mapi_internal.c
    src/mapi_iproc.c
    src/mapi_osd.c
    src/mapi_sys.c
    src/mapi_vcap_internal.c
    src/mapi_vcap.c
    src/mapi_vdec.c
    src/mapi_venc.c
    src/mapi_vproc.c
)

# 指定头文件目录
include_directories("${COMPS_TOP_DIR}/../common_files")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/aac")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include/linux")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include/isp")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include/isp/cv181x")
include_directories("${COMPS_INSTALL_DIR}/cvi_mpi/include/isp/cv180x")
include_directories("${COMPS_INSTALL_DIR}/osal/include")
include_directories("${COMPS_INSTALL_DIR}/hal/include")

# 设置库路径和库文件
#link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")

# # Set the CMAKE C flags
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDUAL_OS")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDUAL_OS")

# # 为静态库 comp_mapi_static 添加依赖库
set(MPI_LIB_DIR "${COMPS_INSTALL_DIR}/cvi_mpi/lib")
# 动态获取文件夹中的所有库文件
file(GLOB MPI_LIB_FILES "${MPI_LIB_DIR}/3rd/*.a" "${MPI_LIB_DIR}/*.a")
# 定义一个子目录
set(MPI_LIB_OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/mpi_lib_obj")
file(MAKE_DIRECTORY ${MPI_LIB_OBJ_DIR})

foreach(LIBRARY ${MPI_LIB_FILES})
    execute_process(
        COMMAND ${CMAKE_AR} -x ${LIBRARY}
        WORKING_DIRECTORY ${MPI_LIB_OBJ_DIR}
        RESULT_VARIABLE AR_RESULT
    )
    if(NOT AR_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract objects from ${LIBRARY}")
    endif()
endforeach()

file(GLOB MPI_LIB_OBJECTS "${MPI_LIB_OBJ_DIR}/*.o")

# 生成库文件
add_library(comp_mapi SHARED ${SOURCE_CODE_FILES})
add_library(comp_mapi_static STATIC ${SOURCE_CODE_FILES} ${MPI_LIB_OBJECTS})
# add_library(comp_mapi_static STATIC ${SOURCE_CODE_FILES})
set_target_properties(comp_mapi PROPERTIES
    VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME comp_mapi PREFIX "lib" SUFFIX ".so")
set_target_properties(comp_mapi_static PROPERTIES OUTPUT_NAME comp_mapi PREFIX "lib" SUFFIX ".a")

# 设置安装目标
install(TARGETS comp_mapi DESTINATION lib)
install(TARGETS comp_mapi_static DESTINATION lib)

# 安装头文件
file(GLOB PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
install(FILES ${PUBLIC_HEADERS} DESTINATION include)

# 可选：添加编译选项或链接库（如果需要）
option(ENABLE_DUAL_OS "Enable DUAL_OS compile definition" ON)
option(ENABLE_VO_INIT "Init VO when DISP init" ON)

if(ENABLE_DUAL_OS)
    target_compile_definitions(comp_mapi PRIVATE DUAL_OS)
    target_compile_definitions(comp_mapi_static PRIVATE DUAL_OS)
endif()

if(ENABLE_184X)
    target_compile_definitions(comp_mapi PRIVATE CHIP_184X)
    target_compile_definitions(comp_mapi_static PRIVATE CHIP_184X)
endif()

if(ENABLE_VO_INIT)
    target_compile_definitions(comp_mapi PRIVATE VO_INIT)
    target_compile_definitions(comp_mapi_static PRIVATE VO_INIT)
endif()

option(ENABLE_ISP_PQ_TOOL "Enable ISP_PQ_TOOL compile definition" OFF)

if(ENABLE_ISP_PQ_TOOL)
    target_compile_definitions(comp_mapi PRIVATE ENABLE_ISP_PQ_TOOL)
    target_compile_definitions(comp_mapi_static PRIVATE ENABLE_ISP_PQ_TOOL)
endif()
# target_compile_options(... PRIVATE -Wall -Wextra)
# target_link_libraries(... PRIVATE some_other_library)