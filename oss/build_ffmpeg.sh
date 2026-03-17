#!/bin/bash
set -e

# 记录开始时间
START_TIME=$(date +%s)

# 💬使用方法：
# 1. 通过 CROSS_COMPILE 指定编译版本；
# 2. 要确保对应的编译工具链路径有添加到 PATH 环境变量中；
# 3. 在 ffmpeg 目录下执行脚本；
# export CROSS_COMPILE=riscv64-unknown-linux-musl-; ./build_ffmpeg.sh

if [ -z "${CROSS_COMPILE}" ]; then
    echo "❌错误：CROSS_COMPILE 环境变量为空，请通过 CROSS_COMPILE 指定编译工具链。"
    echo "💬例如：export CROSS_COMPILE=riscv64-unknown-linux-musl-; ./build_ffmpeg.sh"
    exit 1
fi

# 🔧FFmpeg 源文件路径
FFMPEG_PATH="$(pwd)"

# 🛠编译目录与安装目录
CROSS_COMPILE_NAME="${CROSS_COMPILE%-}"
INSTALL_DIR=${FFMPEG_PATH}/install/${CROSS_COMPILE_NAME}
mkdir -p ${INSTALL_DIR}

# 定义颜色变量
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=========================================================${NC}"
echo -e "${BLUE}ffmpeg build CROSS_COMPILE${NC}=${CROSS_COMPILE}"
echo -e "${BLUE}ffmpeg source code path${NC}=${FFMPEG_PATH}"
echo -e "${BLUE}ffmpeg install path${NC}=${INSTALL_DIR}"
echo -e "${GREEN}=========================================================${NC}"

# 💬默认配置
# 使用 ./configure -h 查看所有配置说明
# 先禁用所有解码器，在启用需要的，可以用 list-decoders 查看支持的解码器，其他也类似
# ❗❗ 注意：如果不确定哪些模块需要，建议先将下面的变量设置都注释掉，全部开启！
CONFIG_DECODER="--disable-decoders --enable-decoder=mjpeg --enable-decoder=hevc --enable-decoder=pcm_s16le --enable-decoder=h264 --enable-decoder=aac"
CONFIG_ENCODER="--disable-encoders --enable-encoder=aac  --enable-encoder=mjpeg"
CONFIG_HWACCELS="--disable-hwaccels"
CONFIG_DEMUXER="--disable-demuxers --enable-demuxer=mov --enable-demuxer=mpegts --enable-demuxer=image2 --enable-demuxer=mjpeg --enable-demuxer=pcm_s16le --enable-demuxer=aac "
CONFIG_MUXERS="--disable-muxers --enable-muxer=mp4 --enable-muxer=mov --enable-muxer=mpegts --enable-muxer=hevc --enable-muxer=aac"
CONFIG_PARSERS="--disable-parsers --enable-parser=aac"
CONFIG_PROTOCOLS="--disable-protocols --enable-protocol=file --enable-protocol=http --enable-protocol=ftp"
CONFIG_BSFS="--disable-bsfs --enable-bsf=h264_mp4toannexb"
CONFIG_INDEVS="--disable-indevs"
CONFIG_OUTDEVS="--disable-outdevs"
CONFIG_FILTERS="--disable-filters"
CONFIG_OTHER="--cpu=armv7-a --enable-neon --enable-gpl --enable-fast-unaligned --extra-cflags="-O3" "
CONFIG_LDEXEFLAGS=""

## extra flags
# --enable-gpl
# --enable-libx264
# --enable-nonfree
# --enable-libaacplus
# --extra-cflags="-I/my/path/were/i/keep/built/arm/stuff/include"
# --extra-ldflags="-L/my/path/were/i/keep/built/arm/stuff/lib"
# --extra-libs=-ldl

if [ ${CROSS_COMPILE} = arm-cvitek-linux-uclibcgnueabihf- ]; then
  ARCH=arm
elif [ ${CROSS_COMPILE} = arm-none-linux-musleabihf- ]; then
  ARCH=arm
  sed -i "45s/__off_t/off_t/g" ${FFMPEG_PATH}/libavformat/file.c
elif [ ${CROSS_COMPILE} = arm-none-linux-gnueabihf- ]; then
  ARCH=arm
  sed -i "45s/__off_t/off_t/g" ${FFMPEG_PATH}/libavformat/file.c
elif [ ${CROSS_COMPILE} = aarch64-none-linux-gnu- ]; then
  ARCH=aarch64
  sed -i "45s/__off_t/off_t/g" ${FFMPEG_PATH}/libavformat/file.c
elif [ ${CROSS_COMPILE} = aarch64-linux-gnu- ]; then
  ARCH=aarch64
elif [ ${CROSS_COMPILE} = riscv64-unknown-linux-gnu- ]; then
  ARCH=riscv64
  CONFIG_OTHER+="--enable-pic --cpu=rv64imafdv"
  CONFIG_LDEXEFLAGS+="--extra-ldexeflags=-mcpu=c906fdv"
elif [ ${CROSS_COMPILE} = riscv64-unknown-linux-musl- ]; then
  ARCH=riscv64
  CONFIG_OTHER+="--enable-pic --cpu=rv64imafdv"
  CONFIG_LDEXEFLAGS+="--extra-ldexeflags=-mcpu=c906fdv"
  sed -i "45s/__off_t/off_t/g" ${FFMPEG_PATH}/libavformat/file.c
elif [ ${CROSS_COMPILE} = aarch64-none-linux-musl- ]; then
  ARCH=aarch64
  sed -i "45s/__off_t/off_t/g" ${FFMPEG_PATH}/libavformat/file.c
elif [ ${CROSS_COMPILE} = arm-linux-gnueabihf- ]; then
  ARCH=arm
fi

pushd ${FFMPEG_PATH}
${FFMPEG_PATH}/configure \
    --enable-cross-compile \
    --cross-prefix=${CROSS_COMPILE} \
    --arch=${ARCH} \
    --target-os=linux \
    --enable-shared \
    $CONFIG_DECODER \
    $CONFIG_ENCODER \
    $CONFIG_HWACCELS \
    $CONFIG_DEMUXER \
    $CONFIG_MUXERS \
    $CONFIG_PARSERS \
    $CONFIG_PROTOCOLS \
    $CONFIG_BSFS \
    $CONFIG_INDEVS \
    $CONFIG_OUTDEVS \
    $CONFIG_FILTERS \
    $CONFIG_OTHER \
    $CONFIG_LDEXEFLAGS \
    --prefix=${INSTALL_DIR}

# first clean build cache, then build
make clean && rm -rf ${INSTALL_DIR}/*
make -j$(nproc)
make install
popd

# 记录结束时间并计算运行时间
END_TIME=$(date +%s)
RUNTIME=$((END_TIME - START_TIME))
HOURS=$((RUNTIME / 3600))
MINUTES=$(( (RUNTIME % 3600) / 60 ))
SECONDS=$((RUNTIME % 60))

# 显示运行时间
echo -e ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${BLUE}FFmpeg 编译完成！总用时${NC}= ${HOURS}小时 ${MINUTES}分钟 ${SECONDS}秒"
echo -e "${BLUE}编译结果已安装到${NC}= ${INSTALL_DIR}"
echo -e "${GREEN}=========================================================${NC}"
