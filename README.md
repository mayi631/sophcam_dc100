## 介绍

sophcam是基于 cvitek 芯片开发的适用于传统相机、AI相机的应用框架.

## 目录结构

**完整 SDK 目录结构**：

```bash
.
├── build       # 板卡配置目录（编译脚本）
├── cnpy
├── cvi_alios   # 双系统小核代码
├── cvi_mpi     # 媒体处理代码
├── cvi_rtsp
├── fsbl
├── host-tools  # 交叉编译工具链
├── install     # 编译输出目录
├── isp-tool-daemon # isp 调试工具
├── isp_tuning  # isp 参数
├── ive
├── libsophon   # 支持本地算法
├── linux_5.10
├── mediapipe
├── osal
├── osdrv      # 部分驱动代码
├── oss
├── ramdisk
├── sophcam    # 相机应用代码
├── tdl_sdk    # 支持本地算法
└── u-boot-2021.10
```

**应用目录结构**：

```bash
sophcam
├── applications # 应用核心代码，包括 UI 和业务逻辑
├── app_services # 应用服务代码
├── cmake
├── common_files # 公共文件
├── components   # 组件代码
├── configs      # 配置文件
├── oss          # 三方组件
├── out          # 编译输出目录
├── release      # release 工具
├── scripts      # kconfig 相关脚本
└── toolchains   # 交叉编译工具链配置
```

## 编译

### **编译板卡固件**

> 参考[SDK 编译使用手册](https://doc.sophgo.com/cvitek-develop-docs/master/docs_latest_release/CV184x/zh/01.software/BSP/SDK_Compilation_and_Usage_Guide/build/html/2_Build_CVITEK_Software_Compilation_Environment.html#id4)

```bash
# 目前相机方案默认使用了TPU算法，需要编译TPU相关库
export TPU_REL=1;
source build/envsetup_soc.sh
defconfig cv1842cp_sm3_81_spinand
clean_all && build_all
```

目标输出（注意检查是否有编译报错或者编译后的文件缺少）：

```bash
$ tree install/soc_cv1842cp_sm3_81_spinand/ -L 1
install/soc_cv1842cp_sm3_81_spinand/
├── boot.spinand # linux 固件
├── data
├── data.spinand # data 分区，用于存放应用
├── elf
├── fip.bin	 # 烧录使用的 bootloader
├── fip_spl.bin  # 启动时使用的 bootloader
├── logo.jpg	 # 开机logo
├── partition_spinand.xml # 分区配置文件
├── ramboot.itb
├── rawimages
├── rootfs	 # rootfs 文件
├── rootfs.spinand # rootfs 固件
├── system
├── tools
├── tpu_musl_arm
├── upgrade.zip
└── yoc.bin	 # 小核 alios 的固件

7 directories, 10 files
```

### **编译相机应用**

1. 配置交叉编译器路径到环境
2. 使用 make xxx_defconfig 选择应用配置
3. make -j 编译应用
4. make install 安装应用到 out/install/ 目录

```shell
cd sophcam
# 根据不同的应用使用configs下的文件
make cv1842cp_sm3_81_dc309_defconfig
make -j && make install
```

支持的命令有：

- `make menuconfig`：图形化配置应用选项
- `make xxx_defconfig`：使用预设的配置文件
- `make`：编译应用
- `make install`：安装应用到 `out/install/` 目录
- `make clean`：清理除oss以外的所有编译文件
- `make clean_all`：清理所有编译文件（包括oss）
- `make distclean`：清理所有编译缓存文件、配置文件、安装文件
- `make release`：生成 release 包
- `make pack_data`：将应用打包到 data 分区，烧录即可。

### 调试

1. 将 out/install/bin 文件夹拷贝到板卡的 /mnt/data/ 目录下

```shell
# sd卡方式
mount /dev/mmcblk0p1 /mnt/sd
cp -rf out/install/bin /mnt/data/

# adb方式
adb push out/install/bin /mnt/data/
```

2. 运行

```shell
# 生成多媒体参数
cd /mnt/data/bin/param
chmod +x ini2bin_board
./ini2bin_board

# 运行应用
cd /mnt/data/bin
chmod +x sophcam
/mnt/data/bin/sophcam < /dev/null &
```
