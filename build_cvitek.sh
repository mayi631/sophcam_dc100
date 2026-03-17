#!/bin/bash
set -e
current_path=$(pwd)
CONFIG_FILE=$(pwd)/.config
BUILD_CONFIG_FILE=$(pwd)/../sophpi/cvitek/build/.config

if [ -n "$3" ]; then
  SDK_PATH="$3"
else
  SDK_PATH="$current_path/../sophpi/cvitek"
fi
echo "SDK_PATH = $SDK_PATH"

function build_cvitek_all()
{
    pushd $SDK_PATH
    source build/envsetup_soc.sh
    defconfig $1

    # # 删除 newconfig 文件中带有 CONFIG_SENSOR 的行
    # sed -i '/^CONFIG_SENSOR/d' "$BUILD_CONFIG_FILE"

    # # 从 .config 文件中提取目标行并替换内容
    # while IFS= read -r line; do
    #     if [[ $line == CONFIG_SNS0_* || $line == CONFIG_SNS1_* || $line == CONFIG_SNS2_* ]]; then
    #         new_line="${line#CONFIG_SNS[0-2]_}"
    #         echo "CONFIG_SENSOR_$new_line" >> $BUILD_CONFIG_FILE
    #     fi
    # done < $CONFIG_FILE

    # export ENABLE_BOOTLOGO=1
    clean_all
    build_all
    popd
}

function pack_data()
{
    # rm $SDK_PATH/install/soc_$1/rawimages/data.spinor
    # rm $SDK_PATH/install/soc_$1/data.spinor
    rm -rf $SDK_PATH/install/soc_$1/data/*

    cp -a $current_path/out/install/* $SDK_PATH/install/soc_$1/data
    pushd $SDK_PATH
    source build/envsetup_soc.sh
    defconfig $1
    pack_data
    popd
}

# function build_cvitek_sensor()
# {
#     pushd $SDK_PATH
#     source build/envsetup_soc.sh
#     defconfig $1

#     # 删除 newconfig 文件中带有 CONFIG_SENSOR 的行
#     sed -i '/^CONFIG_SENSOR/d' "$BUILD_CONFIG_FILE"

#     # 从 .config 文件中提取目标行并替换内容
#     while IFS= read -r line; do
#         if [[ $line == CONFIG_SNS0_* || $line == CONFIG_SNS1_* || $line == CONFIG_SNS2_* ]]; then
#             new_line="${line#CONFIG_SNS[0-2]_}"
#             echo "CONFIG_SENSOR_$new_line" >> $BUILD_CONFIG_FILE
#         fi
#     done < $CONFIG_FILE

#     clean_alios
#     build_alios
#     popd
# }

if [ "$#" -eq 0 ]; then
  echo "wrong Usage"
  exit 1
elif [ $1 == "cvitek" ]; then
  echo "build_cvitek_all"
  build_cvitek_all $2

# elif [ $1 == "sensor" ]; then
#   build_cvitek_sensor $2

elif [ $1 == "pack_data" ]; then
  echo "pack_data"
  pack_data $2

fi


