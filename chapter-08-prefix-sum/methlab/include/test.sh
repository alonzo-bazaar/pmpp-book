#!/usr/bin/env bash

# compile everything in here then delete it
# just to make sure this directory all works, without having to mess with main

BUILD_DIR='./build'

shit() {
    local cfile="$1"
    local impl_macro="$2" # optional

    echo "compiling ${cfile}..."

    # https://github.com/dylanaraps/pure-bash-bible#replacement
    local barefile="$cfile"
    barefile="${barefile##*/}" # get basename
    barefile="${barefile%%.*}" # withot the file extension

    if [[ -n "$impl_macro" ]]; then
        gcc -fPIC -shared -o "./$BUILD_DIR/$barefile.o" -x c\
		-DCL_TARGET_OPENCL_VERSION=300\
		-Wall -Wextra\
		"-D${impl_macro}" "$cfile"
    else
        gcc -fPIC -shared -o "./$BUILD_DIR/$barefile.o" -x c\
		-DCL_TARGET_OPENCL_VERSION=300\
		-Wall -Wextra\
		"$cfile"
    fi
}

mkdir "$BUILD_DIR"

shit argparse.h ARGPARSE_INCLUDE_IMPLEMENTATION
shit debugging.h
shit error.h ERROR_INCLUDE_IMPLEMENTATION
shit oclutils.h OCL_UTILS_INCLUDE_IMPLEMENTATION
shit printvecs.h  PRINTVECS_INCLUDE_IMPLEMENTATION
shit readfile.h READFILE_INCLUDE_IMPLEMENTATION 
shit timing.h TIMING_INCLUDE_IMPLEMENTATION

rm -rf "$BUILD_DIR"
