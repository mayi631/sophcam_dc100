#!/bin/bash
set -e

CUR_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
#SDK_PATH="$(dirname "$(which "$CROSS_COMPILE_3RD"gcc)")/../../../.."
CROSS_COMPILE="$(dirname "$(which "$CROSS_COMPILE"gcc)")/$CROSS_COMPILE"
echo "SDK_PATH = $SDK_PATH"
echo "CROSS_COMPILE = $CROSS_COMPILE"
pushd $CUR_DIR/awtkcode/awtk-linux-fb

SDK_ROOT=${CUR_DIR}/../
#CVILIB_DIR=$LIB_DIR SDK_PATH=$SDK_PATH CROSS_COMPILE=$CROSS_COMPILE TOUCHPAD=$TOUCHPAD SDK_ROOT=$SDK_ROOT FB_DEPTH=$FB_DEPTH RISCV=$RISCV scons -j 10
CROSS_COMPILE=$CROSS_COMPILE TOUCHPAD=$TOUCHPAD SDK_ROOT=$SDK_ROOT FB_DEPTH=$FB_DEPTH FB_FORMAT=$FB_FORMAT FB_BUFFER=$FB_BUFFER RISCV=$RISCV BOARD=$BOARD scons -j 10
#create static lib
AR="ar"
RANLIBG="ranlib"
# find $CUR_DIR/awtkcode/awtk-linux-fb/build/lib -name "*.a" -printf 'ar %p\n' -exec $CROSS_COMPILE$AR x {} \;
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libnanovg-agge.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libextwidgets.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libawtk_global.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libfribidi.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libagg.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libcommon.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libwidgets.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libawtk_linux_fb.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libtkc_static.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libbase.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libagge.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libnanovg.a
$CROSS_COMPILE$AR x $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/liblinebreak.a

$CROSS_COMPILE$AR cru $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libawtk.a *.o
echo "ranlib = $CROSS_COMPILE$RANLIBG"
$CROSS_COMPILE$RANLIBG $CUR_DIR/awtkcode/awtk-linux-fb/build/lib/libawtk.a
rm *.o -rf
popd
