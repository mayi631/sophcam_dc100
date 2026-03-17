 #!/bin/bash

# clang-tidy 静态分析脚本
# 使用项目根目录的 .clang-tidy 配置文件

# 默认值
PROJECT_ROOT=""
BUILD_DIR=""

# 解析命令行参数
while [ $# -gt 0 ]; do
    case $1 in
        --project-root)
            PROJECT_ROOT="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --help)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  --project-root PATH  项目根目录路径"
            echo "  --build-dir PATH     构建目录路径"
            echo "  --help               显示此帮助信息"
            echo ""
            echo "示例:"
            echo "  $0 --project-root /path/to/project --build-dir /path/to/build"
            exit 0
            ;;
        *)
            echo "未知选项: $1"
            echo "使用 --help 查看帮助"
            exit 1
            ;;
    esac
done

# 如果没有提供参数，使用默认值
if [ -z "$PROJECT_ROOT" ]; then
    SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    PROJECT_ROOT=$(readlink -f $SCRIPT_DIR)
fi

if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR="$PROJECT_ROOT/build"
fi

# 检查是否存在 .clang-tidy 配置文件
if [ ! -f "$PROJECT_ROOT/.clang-tidy" ]; then
    echo "错误: 未找到 .clang-tidy 配置文件: $PROJECT_ROOT/.clang-tidy"
    echo "请确保在项目根目录存在 .clang-tidy 文件"
    exit 1
fi

# 检查是否存在 build 目录和 compile_commands.json
if [ ! -d "$BUILD_DIR" ]; then
    echo "错误: 未找到构建目录: $BUILD_DIR"
    echo "请先运行构建命令"
    exit 1
fi

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "错误: 未找到 compile_commands.json 文件: $BUILD_DIR/compile_commands.json"
    echo "请确保 CMake 配置时启用了 compile_commands.json"
    exit 1
fi

echo "开始运行 clang-tidy 静态分析..."
echo "项目根目录: $PROJECT_ROOT"
echo "构建目录: $BUILD_DIR"
echo "使用配置文件: $PROJECT_ROOT/.clang-tidy"
echo "编译数据库: $BUILD_DIR/compile_commands.json"
echo ""

# 运行 clang-tidy
# 使用项目根目录的 .clang-tidy 配置文件
# 分析 src 和 components 目录下的源文件

# 查找源文件
echo "查找源文件..."
SOURCE_FILES=$(find "$PROJECT_ROOT/src" "$PROJECT_ROOT/components" \
    -name "*.cpp" -o -name "*.cc" -o -name "*.c" -o -name "*.cxx" \
    -type f 2>/dev/null | grep -v "external/" | grep -v "third_party/")

if [ -z "$SOURCE_FILES" ]; then
    echo "警告: 未找到源文件"
    echo "检查的目录: $PROJECT_ROOT/src, $PROJECT_ROOT/components"
    exit 1
fi

echo "找到源文件:"
echo "$SOURCE_FILES" | head -5
if [ $(echo "$SOURCE_FILES" | wc -l) -gt 5 ]; then
    echo "... 还有更多文件"
fi
echo ""

# 使用 clang-tidy 直接分析文件
echo "开始分析..."
clang-tidy \
    -p "$BUILD_DIR" \
    --header-filter=".*" \
    $SOURCE_FILES

echo ""
echo "clang-tidy 分析完成！"
