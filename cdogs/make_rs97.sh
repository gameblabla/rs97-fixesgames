#!/bin/sh

# To create a debug build, run `cmake -D CMAKE_BUILD_TYPE=Debug .` instead

# Update repo properly (should be handled by most git GUI clients)
git submodule init
git submodule update --init --recursive
git submodule update --recursive

mkdir gcw0build
cd gcw0build
cmake -DCMAKE_TOOLCHAIN_FILE="/opt/rs97-toolchain/usr/share/buildroot/toolchainfile.cmake" ..
#cmake -D CMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="/home/steward/Github/buildroot/output/host/usr/share/buildroot/toolchainfile.cmake" ..
#cmake -D CMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="/home/steward/Github/gh_retrogame_toolchain/for_os/usr/share/buildroot/toolchainfile.cmake" ..
make

