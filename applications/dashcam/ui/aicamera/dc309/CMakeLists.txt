cmake_minimum_required(VERSION 3.10)
project(lvgl)

set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项：将警告视为错误
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


# 添加 freetype 库
set(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/third_party/freetype/freetype2)
if(CMAKE_TOOLCHAIN_FILE MATCHES ".*arm-none-linux-musleabihf\\.cmake$")
	set(FREETYPE_LIBRARIES ${PROJECT_SOURCE_DIR}/third_party/freetype/lib/musl_arm32/libfreetype.a)
elseif(CMAKE_TOOLCHAIN_FILE MATCHES ".*arm-linux-gnueabihf\\.cmake$")
	set(FREETYPE_LIBRARIES ${PROJECT_SOURCE_DIR}/third_party/freetype/lib/glibc_arm32/libfreetype.a)
elseif(CMAKE_TOOLCHAIN_FILE MATCHES ".*x86_64-linux-gnu\\.cmake$")
	set(FREETYPE_LIBRARIES ${PROJECT_SOURCE_DIR}/third_party/freetype/lib/x86_64/libfreetype.a)
else()
	# 默认使用x86_64库
	set(FREETYPE_LIBRARIES ${PROJECT_SOURCE_DIR}/third_party/freetype/lib/x86_64/libfreetype.a)
endif()
message(STATUS "FREETYPE_INCLUDE_DIRS: ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "FREETYPE_LIBRARIES: ${FREETYPE_LIBRARIES}")




add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${FREETYPE_INCLUDE_DIRS})

# 为lvgl库禁用未使用变量的警告
target_compile_options(lvgl PRIVATE -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function)

add_executable(main src/main.c src/mouse_cursor_icon.c)

# 为main目标单独设置更严格的警告选项
target_compile_options(main PRIVATE -Wall -Wextra -Wunused-variable -Wunused-parameter -Wunused-function)

# 增加 guiguider_ui 相关 include 路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/guiguider_ui)
include_directories(${PROJECT_SOURCE_DIR}/src/guiguider_ui/images)
include_directories(${PROJECT_SOURCE_DIR}/src/guiguider_ui/guider_fonts)
include_directories(${PROJECT_SOURCE_DIR}/src/guiguider_ui/guider_customer_fonts)
include_directories(${PROJECT_SOURCE_DIR}/src/guiguider_ui/common)

# 添加AIGC组件包含目录
include_directories(${PROJECT_SOURCE_DIR}/../../../../components/AIdemo/components/ai_image/include)
include_directories(${PROJECT_SOURCE_DIR}/../../../../components/AIdemo/components/config/include)
include_directories(${PROJECT_SOURCE_DIR}/../../../../components/AIdemo/components/logs/include)

# 收集 guiguider_ui 相关源文件
file(GLOB GUIGUIDER_SRCS
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/*.c
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/images/*.c
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/guider_fonts/*.c
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/indev/*.c
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/guider_customer_fonts/*.c
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/common/*.c
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/power/*.c
    ${PROJECT_SOURCE_DIR}/src/guiguider_ui/voiceplay/*.c
)

# 将 guiguider_ui 源文件加入 main 目标
set_property(TARGET main APPEND PROPERTY SOURCES ${GUIGUIDER_SRCS})

# include(${CMAKE_CURRENT_LIST_DIR}/lvgl/tests/FindLibDRM.cmake)
# include_directories(${Libdrm_INCLUDE_DIRS})

if(NOT DEFINED CONFIG)
	set(CONFIG "SDL")
endif()

if(CONFIG STREQUAL "SDL")
	add_definitions(-DFB_RUN=0)
	find_package(SDL2)
	# find_package(SDL2_image)
	include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
elseif(CONFIG STREQUAL "FB")
	add_definitions(-DFB_RUN=1)
else()
	message(FATAL_ERROR "Unknown CONFIG: ${CONFIG}")
endif()

# target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${Libdrm_LIBRARIES} m pthread)
target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg m pthread ${FREETYPE_LIBRARIES} ai_image)
if(CONFIG STREQUAL "SDL")
	target_link_libraries(main ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif()
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)
