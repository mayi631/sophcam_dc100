#!/bin/bash

# [Usage-Example] : ./scripts/parse_dir.sh subtree.xml

function printusage {
    echo "Usage: $0 arg1"
    echo "arg1: xxx.xml"
}

# 判断传参数目是否合法
if [[ $# -lt 1 ]]; then
    printusage
    exit 1
fi

dir=$1

# 获取xml文件中所有的project name
repo_list=$(grep -o '<project name="[^"]*"' ${dir} | sed 's/.*name="\([^"]*\)".*/\1/')

current_dir=$(pwd)

common_dir=
common_inc_mk=

# git clone function
function parse_dir {

    revision=$(grep "name=\"$repo\"" ${dir} | sed -n 's/.*revision="\([^"]*\)".*/\1/p')
    path=$(grep "name=\"$repo\"" ${dir} | sed -n 's/.*path="\([^"]*\)".*/\1/p')

    # 判断参数是否带有path
    if [[ ! -n $path ]]; then
        path=$repo
    fi

	if [ "$repo" = "framework/common" ]; then
		common_dir=$current_dir/$path
		common_inc_mk=$common_dir/repo_dir_def.mk
        echo "############## repo_dir_def ##############" > $common_inc_mk
        echo "## common_dir ${common_dir}"
	fi

    path_lower=$(basename "$path")
    path_upper=$(echo "$path_lower" | awk '{print toupper($0)}')
	new_def_dir_value="export ${path_upper}_DIR=\$(SRCTREE)/${path}"
	if grep -q "^${path_upper}_DIR=" "$common_inc_mk"; then
		echo "exist same def_dir $new_def_dir_value"
	else
		 echo "$new_def_dir_value" >> $common_inc_mk
	fi
}

function main {

    for repo in $repo_list
    do
        parse_dir
    done

    sed -i "s|^CURDIR=.*|CURDIR=$current_dir|" $current_dir/Makefile
    sed -i "s|^COMMON_DIR=.*|COMMON_DIR=$common_dir|" $current_dir/Makefile
    sed -i '/^COMMON_DIR=/d' $common_inc_mk
}

main
