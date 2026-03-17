#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CLANG_ROOT=$(readlink -f $SCRIPT_DIR)
CLANG_FORMAT=clang-format-12

find $CLANG_ROOT/components -regex '.*\.\(cpp\|h\|hpp\|cc\|c\|cxx\|inc\)' | xargs $CLANG_FORMAT --style=file -i || exit 1
find $CLANG_ROOT/src -regex '.*\.\(cpp\|h\|hpp\|cc\|c\|cxx\|inc\)' | xargs $CLANG_FORMAT --style=file -i || exit 1
