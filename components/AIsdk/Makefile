# AI demos 项目 Makefile
# 支持多架构编译和安装

################################################################################
# 项目配置
################################################################################
# 默认架构
ARCH := musl_arm32
# 默认编译类型
BUILD_TYPE := Debug
# demo 链接类型配置（component 默认会同时生成动态库、静态库）
LINK_TYPE := static
# 并行编译线程数
JOBS ?= $(shell nproc 2>/dev/null || echo 4)
# 获取Makefile所在目录的绝对路径，确保路径的健壮性
PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
# 构建目录 (使用绝对路径)
BUILD_DIR := $(PROJECT_ROOT)build/$(ARCH)/$(BUILD_TYPE)
INSTALL_DIR := $(PROJECT_ROOT)install/$(ARCH)
# 支持的架构列表（与 CMakeLists.txt 保持一致）
SUPPORTED_ARCHS := glibc_riscv64 glibc_arm32 glibc_aarch64 musl_arm32 musl_riscv64 x86_64
# CMake选项 (使用绝对路径)
CMAKE_OPTIONS := -DTARGET_ARCH=$(ARCH) \
                 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
                 -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)

# 根据LINK_TYPE设置BUILD_SHARED_LIBS
ifeq ($(LINK_TYPE), shared)
    CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=ON
else
    CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=OFF
endif

# 颜色输出
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
NC := \033[0m # No Color


################################################################################
# 构建目标
################################################################################
.PHONY: all clean clean-deps clean-all install help check-arch

# 默认目标
all: check-arch
	@echo "$(GREEN)编译所有内容 (架构: $(ARCH), 类型: $(BUILD_TYPE))...$(NC)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && \
		cmake $(PROJECT_ROOT) $(CMAKE_OPTIONS) && \
		make -j$(JOBS)
	@echo "$(GREEN)编译完成！$(NC)"

# 清理编译产物
clean:
	@echo "$(YELLOW)清理编译产物...$(NC)"
	@if [ -d "$(BUILD_DIR)" ]; then \
		echo "正在清理 $(BUILD_DIR)..."; \
		rm -rf $(BUILD_DIR); \
	fi
	@if [ -d "$(INSTALL_DIR)" ]; then \
		echo "正在清理 $(INSTALL_DIR)..."; \
		rm -rf $(INSTALL_DIR); \
	fi
	@echo "$(GREEN)清理完成！$(NC)"

# 清理第三方库缓存
clean-deps:
	@echo "$(YELLOW)清理第三方库缓存...$(NC)"
	@if [ -d "$(PROJECT_ROOT).deps/$(ARCH)" ]; then \
		echo "正在清理 $(PROJECT_ROOT).deps/$(ARCH)..."; \
		rm -rf $(PROJECT_ROOT).deps/$(ARCH); \
	fi
	@echo "$(GREEN)第三方库缓存清理完成！$(NC)"

# 清理所有
clean-all: clean clean-deps
	@echo "$(GREEN)全部清理完成！$(NC)"

# 安装到install目录
install: all
	@echo "$(GREEN)安装到 $(INSTALL_DIR)...$(NC)"
	@mkdir -p $(INSTALL_DIR)
	@cd $(BUILD_DIR) && make install
	@echo "$(GREEN)合并第三方库到安装目录...$(NC)"
	@if [ -d "$(PROJECT_ROOT).deps/$(ARCH)" ]; then \
		echo "正在复制第三方库从 $(PROJECT_ROOT).deps/$(ARCH) 到 $(INSTALL_DIR)..."; \
		cp -r $(PROJECT_ROOT).deps/$(ARCH)/*/* $(INSTALL_DIR)/ 2>/dev/null || true; \
		echo "$(GREEN)第三方库复制完成！$(NC)"; \
	else \
		echo "$(YELLOW)警告：第三方库目录 $(PROJECT_ROOT).deps/$(ARCH) 不存在$(NC)"; \
	fi
	@echo "$(GREEN)安装完成！$(NC)"

# 检查架构是否支持
check-arch:
	@if ! echo "$(SUPPORTED_ARCHS)" | grep -q -w "$(ARCH)"; then \
		echo "$(RED)错误：不支持的架构 '$(ARCH)'$(NC)"; \
		echo "$(YELLOW)支持的架构：$(SUPPORTED_ARCHS)$(NC)"; \
		exit 1; \
	fi
	@echo "$(BLUE)使用架构：$(ARCH)$(NC)"

# 运行 clang-tidy 静态分析
clang-tidy: check-arch
	@echo "$(GREEN)运行 clang-tidy 静态分析...$(NC)"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "$(YELLOW)构建目录不存在，先进行构建...$(NC)"; \
		$(MAKE) all; \
	fi
	@echo "$(BLUE)使用构建目录: $(BUILD_DIR)$(NC)"
	@$(PROJECT_ROOT)clang-tidy.sh \
		--project-root "$(PROJECT_ROOT)" \
		--build-dir "$(BUILD_DIR)"
	@echo "$(GREEN)clang-tidy 分析完成！$(NC)"

# 显示帮助信息
help:
	@echo "$(GREEN)使用方法：$(NC)"
	@echo "  make [目标] [ARCH=架构] [BUILD_TYPE=类型]"
	@echo ""
	@echo "$(GREEN)主要目标：$(NC)"
	@echo "  all          - 编译所有内容（默认目标）"
	@echo "  clean        - 清理当前架构的编译产物"
	@echo "  clean-deps   - 清理当前架构的第三方库缓存"
	@echo "  clean-all    - 清理所有（编译产物 + 第三方库缓存）"
	@echo "  install      - 编译并安装到install目录"
	@echo "  clang-tidy   - 运行 clang-tidy 静态分析"
	@echo "  help         - 显示此帮助信息"
	@echo ""
	@echo "$(GREEN)环境变量：$(NC)"
	@echo "  BUILD_TYPE   - 编译类型（Debug/Release，默认：Debug）"
	@echo "  LINK_TYPE    - 链接类型（static/shared，默认：static）"
	@echo "  ARCH         - 目标架构 默认：x86_64"
	@echo "  $(BLUE)支持的架构：$(SUPPORTED_ARCHS)$(NC)"
	@echo ""
	@echo "$(GREEN)示例：$(NC)"
	@echo "  make                                    # 使用默认架构和Debug模式编译"
	@echo "  make BUILD_TYPE=Release                 # 使用Release模式编译"
	@echo "  make ARCH=glibc_aarch64                 # 使用aarch64架构编译"
	@echo "  make install ARCH=musl_riscv64          # 编译并安装"
	@echo "  make ARCH=x86_64 LINK_TYPE=shared       # 动态链接编译"
	@echo "  make clean                              # 清理编译产物"
	@echo "  make clean-deps                         # 清理第三方库缓存"
	@echo "  make clean-all                          # 清理所有"
