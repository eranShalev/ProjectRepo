#!/bin/bash
if [ $# -eq 0 ]
then
    set -- "debug"
fi

if [ "$1" = "debug" ]
then
    printf "Debug Mode\n"
    build_type=Debug
    build_dir="build.debug"
elif [ "$1" = "release" ]
then
    printf "Release Mode\n"
    build_type=Release
    build_dir="build.release"
else
    printf "Build Mode Not Supported\n"
    exit 1
fi

mkdir $build_dir
cd $build_dir
printf "CMake:\n"
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$build_type ..
printf "\nMake:\n"
make -j 8
cd ..
