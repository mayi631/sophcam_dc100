#!/usr/bin/env bash
set -euo pipefail

# 记录开始时间
START_TIME=$(date +%s)

# 💬使用方法：
# 1. 通过环境变量 CROSS_COMPILE 指定编译版本；
# 2. 通过环境变量 LVGL_C_FLAGS 指定编译选项；
# 3. 在 lvgl 目录下执行脚本；
# export CROSS_COMPILE=riscv64-unknown-linux-musl-; ./build_lvgl.sh
# export LVGL_C_FLAGS="-I${SRCTREE}/applications/dashcam/ui/${CFG_PDT_SUB}/${UI_PACKET}/third_party/freetype/freetype2"; ./build_lvgl

if [ -z "${CROSS_COMPILE}" ]; then
    echo "❌错误：CROSS_COMPILE 环境变量为空，请通过 CROSS_COMPILE 指定编译工具链。"
    echo "💬例如：export CROSS_COMPILE=riscv64-unknown-linux-musl-; ./build_lvgl.sh"
    exit 1
fi

# 🔧lvgl 源文件路径
LVGL_PATH="$(pwd)"

# 🛠编译目录与安装目录
CROSS_COMPILE_NAME="${CROSS_COMPILE%-}"
INSTALL_DIR=${LVGL_PATH}/install/${CROSS_COMPILE_NAME}
build_dir="${LVGL_PATH}/build/${CROSS_COMPILE_NAME}"
mkdir -p ${build_dir}
mkdir -p ${INSTALL_DIR}
# 🛠选择 toolchain
toolchain_file="${SRCTREE}/toolchains/${CROSS_COMPILE_NAME}.cmake"

# 定义颜色变量
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# 确保 toolchain 文件存在（如果需要）
if [[ -n "$toolchain_file" && ! -f "$toolchain_file" ]]; then
  echo -e "${RED}Warning: toolchain file not found: $toolchain_file (continuing if your cmake doesn’t require it)${NC}"
  exit 1
fi
echo -e "${GREEN}=========================================================${NC}"
echo -e "${BLUE}lvgl source code path${NC}=${LVGL_PATH}"
echo -e "${BLUE}lvgl build path${NC}=${build_dir}"
echo -e "${BLUE}lvgl install path${NC}=${INSTALL_DIR}"
echo -e "${BLUE}lvgl build CROSS_COMPILE${NC}=${CROSS_COMPILE}"
echo -e "${BLUE}lvgl LVGL_C_FLAGS${NC}=${LVGL_C_FLAGS:-<unset>}"
echo -e "${BLUE}lvgl toolchain file${NC}=${toolchain_file}"
echo -e "${GREEN}=========================================================${NC}"

# 默认值处理
: "${CMAKE_BUILD_TYPE:=Debug}"

# 清理缓存
rm -rf ${build_dir}
rm -rf ${INSTALL_DIR}

# 运行 cmake 配置与编译
# -DCONFIG_LV_USE_PRIVATE_API=ON
cmake -B "$build_dir" \
  -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
  -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
  -DCMAKE_C_FLAGS="${LVGL_C_FLAGS:-}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -S ${LVGL_PATH}

cmake --build "$build_dir" -j
echo "Build completed. Output dir: $build_dir"

cmake --install "$build_dir"
echo "Install completed. Install dir: ${INSTALL_DIR}"

# 记录结束时间并计算运行时间
END_TIME=$(date +%s)
RUNTIME=$((END_TIME - START_TIME))
HOURS=$((RUNTIME / 3600))
MINUTES=$(( (RUNTIME % 3600) / 60 ))
SECONDS=$((RUNTIME % 60))

# 显示运行时间
echo -e ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${BLUE}LVGL 编译完成！总用时${NC}= ${HOURS}小时 ${MINUTES}分钟 ${SECONDS}秒"
echo -e "${BLUE}编译结果已安装到${NC}= ${INSTALL_DIR}"
echo -e "${GREEN}=========================================================${NC}"
