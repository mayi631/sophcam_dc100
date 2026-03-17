cmake_minimum_required(VERSION 3.13)

project(
    cvi_image_viewer
    VERSION 1.0.2
    LANGUAGES C CXX
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)

include(${TOP_DIR}/cmake/compile_options.cmake)
include(${TOP_DIR}/cmake/summary.cmake)

set(THUMBNAIL_EXTRACTOR_DIR ${TOP_DIR}/components/thumbnail_extractor)
set(THUMBNAIL_EXTRACTOR_INCLUDE_DIR ${THUMBNAIL_EXTRACTOR_DIR}/include)

set(DEMUXER_DIR ${TOP_DIR}/components/demuxer)
set(DEMUXER_INCLUDE_DIR ${DEMUXER_DIR}/include)
set(DEMUXER_LIB_DIR ${DEMUXER_DIR}/lib)

set(CVI_LOG_DIR ${TOP_DIR}/components/cvi_log)
set(CVI_LOG_INCLUDE_DIR ${CVI_LOG_DIR}/include)

set(MAPI_DIR ${TOP_DIR}/components/mapi)
set(MAPI_INCLUDE_DIR ${MAPI_DIR}/include)

set(MIDDLEWARE_DIR ${TOP_DIR}/../platform/middleware/v2)
set(MIDDLEWARE_INCLUDE_DIR ${MIDDLEWARE_DIR}/include)

set(FFMPEG_DIR ${TOP_DIR}/oss/ffmpeg)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)

set(OSAL_DIR ${TOP_DIR}/components/osal)

set(ESSENTIAL_LIBS
    -ldl
)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SOURCES
    ${SOURCES_DIR}/asimage_viewer.cpp
    ${SOURCES_DIR}/image_viewer.cpp
)

set(PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/image_viewer.h
)

# build shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MIDDLEWARE_INCLUDE_DIR}
    ${MAPI_INCLUDE_DIR}
    ${CVI_LOG_INCLUDE_DIR}
PRIVATE
    ${DEMUXER_INCLUDE_DIR}
    ${THUMBNAIL_EXTRACTOR_INCLUDE_DIR}
    ${HAL_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME}
PUBLIC
    ${THUMBNAIL_EXTRACTOR_LIB_DIR}
    ${DEMUXER_LIB_DIR}
    ${FFMPEG_LIB_DIR}
    ${MIDDLEWARE_LIB_DIR}
    ${MAPI_LIB_DIR}
    ${OSAL_LIB_DIR}
    ${CVI_LOG_LIB_DIR}
    ${HAL_SCREEN_LIB_DIR}
)
target_link_libraries(${PROJECT_NAME}
PRIVATE
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
)

# build static library if need
if (${PROJECT_NAME}_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME}_static STATIC ${SOURCES})
    target_include_directories(${PROJECT_NAME}_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${MIDDLEWARE_INCLUDE_DIR}
        ${MAPI_INCLUDE_DIR}
        ${CVI_LOG_INCLUDE_DIR}
    PRIVATE
        ${DEMUXER_INCLUDE_DIR}
        ${THUMBNAIL_EXTRACTOR_INCLUDE_DIR}
        ${HAL_INCLUDE_DIR}
    )
    target_link_directories(${PROJECT_NAME}_static
    PUBLIC
        ${THUMBNAIL_EXTRACTOR_LIB_DIR}
        ${DEMUXER_LIB_DIR}
        ${FFMPEG_LIB_DIR}
        ${MIDDLEWARE_LIB_DIR}
        ${MAPI_LIB_DIR}
        ${OSAL_LIB_DIR}
        ${CVI_LOG_LIB_DIR}
        ${HAL_SCREEN_LIB_DIR}
    )
    target_link_libraries(${PROJECT_NAME}_static
    PRIVATE
    )
    set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    install(TARGETS ${PROJECT_NAME}_static
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
    )
endif()

# build test
if (${PROJECT_NAME}_BUILD_TEST)
    add_subdirectory(test)
endif()

