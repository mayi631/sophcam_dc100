# AI SDK 项目介绍✨✨✨

## 项目构建与项目结构

使用 cmake 管理项目构建，尽量遵守 cmake 的规范，和最佳实践。
- 顶级目录的 cmake 只做全局的项目配置，不涉及具体模块的构建。
- 使用 `add_subdirectory` 添加子目录，子目录的构建由子目录的 CMakeLists.txt 负责。
- components: 组件目录，每个子文件夹对应一个组件，会生成一个库文件。
  - 每个组件的构建由组件的 CMakeLists.txt 负责。
  - 依赖关系由组件自己负责。使用 cmake 标准的方式添加依赖。
  - install 时要提供好头文件和库文件到install目录。
  - 头文件的设计应该包含头文件本身所需要的所有依赖。不应该包含多余的头文件。
  - c++ 实现时，C++ 的版本尽量使用 C++11 标准（因为交叉编译时，部分工具链不支持较高版本标准）。
- src: 应用代码，用于生成可执行文件，也包含测试文件。
  - src 目录下同样按可执行文件组织目录结构，每个子文件夹对应一个可执行文件。
  - 每个可执行文件的构建由可执行文件的 CMakeLists.txt 负责。
  - 依赖关系由可执行文件自己负责。使用 cmake 标准的方式添加依赖。
  - install 时要提供好可执行文件到install目录。
- third_party: 第三方库，用于提供一些第三方库。
  - 目前仅提供预编译好的库。
- docs: 文档目录。
- toolchains: 提供不同工具链的配置选项。


## 编译方法

顶层目录提供一个 `Makefile` 文件，用于编译和安装。各个模块的构建还是走 `cmake` 的，只是利用这个 `Makefile` 简化一些操作。

```bash
使用方法：
  make [目标] [ARCH=架构] [BUILD_TYPE=类型]

主要目标：
  all          - 编译所有内容（默认目标）
  clean        - 清理当前架构的编译产物
  install      - 编译并安装到install目录
  help         - 显示此帮助信息

环境变量：
  BUILD_TYPE   - 编译类型（Debug/Release，默认：Debug）
  LINK_TYPE    - 链接类型（static/shared，默认：static）
  ARCH         - 目标架构 默认：x86_64
  支持的架构：glibc_riscv64 glibc_arm32 glibc_aarch64 musl_arm32 musl_riscv64 x86_64

示例：
  make                                    # 使用默认架构和Debug模式编译
  make BUILD_TYPE=Release                 # 使用Release模式编译
  make ARCH=glibc_aarch64                 # 使用aarch64架构编译
  make install ARCH=musl_riscv64          # 编译并安装
  make ARCH=x86_64 LINK_TYPE=shared       # 动态链接编译
  make clean                              # 清理编译产物
```

编译时，自动生成 `build/${ARCH}` 目录，用于存放编译产物，同时区分开不同架构的编译产物。

## 组件介绍

原则：
- 应该提供一个 c 接口的头文件，和一个库文件。
- 组件本身不依赖于其他组件，都只是为应用提供服务。
- 组件本身不应该读取配置文件，所有参数都应该通过参数传递。

C++ 类的实现：

- 类的实现中，可能会用到一些中间过程的函数，这些函数以什么样的形式实现？
  - 如果函数中会用到类的私有成员，那这个函数就应该作为类的私有成员函数。
  - 如果这个函数不访问私有成员，只有这个类会用到 --> 匿名命名空间中的自由函数。
    ```cpp
    // image_processor.cpp
    namespace {
        // 纯粹的工具函数，不需要访问类成员
        bool is_valid_image_format(const std::string& filename) {
            return filename.ends_with(".jpg") || filename.ends_with(".png");
        }

        std::string extract_filename(const std::string& path) {
            return path.substr(path.find_last_of("/") + 1);
        }
    }

    class ImageProcessor {
    public:
        bool process_image(const std::string& path) {
            if (!is_valid_image_format(path)) {  // 使用工具函数
                return false;
            }
            // ...
        }
    };
    ```
  - 而且其他类也会用到，那应该作为一个 utils 命令控件中的函数。
    ```cpp
    // utils.h
    namespace image_utils {
        bool is_supported_format(const std::string& filename);
        std::string normalize_path(const std::string& path);
    }

    // utils.cpp
    namespace image_utils {
        bool is_supported_format(const std::string& filename) {
            // 实现
        }
    }
    ```

## 三方库

项目中使用到了一些三方库，以 submodule 的方式放在 external 目录下。submodule 带有commit信息，所以不用担心版本问题。

但有一个问题：如何编译？如果每次编译应用，都去编译三方库，太麻烦了，编译速度特别慢。但又需要支持编译三方库，因为会用不同平台，不同工具链编译，手动编译太麻烦。

解决办法：将三方库的编译缓存放到 `.deps` 目录下，每次编译时，先检查 `.deps` 目录下是否存在编译缓存，如果存在，则直接使用，否则重新编译。

```bash
project/
├── .deps/          # 本地三方库缓存
│   ├── x86_64/
│   └── aarch64/
├── build/          # 应用编译产物
└── external/       # 三方库源码
```

**分层清理命令**：
- make clean - 只清理应用编译产物（build目录）
- make clean-deps - 清理三方库编译产物（deps目录）
- make clean-all - 清理所有（build + deps）

**智能清理**：
- make clean-deps ARCH=x86_64 - 只清理特定架构的三方库
