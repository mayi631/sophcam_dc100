#!/bin/bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

pushd $BUILD_DIR
$SCRIPT_DIR/thttpd/configure \
    --prefix=$INSTALL_DIR
make -j $(expr $(nproc) - 1)
make install
popd

