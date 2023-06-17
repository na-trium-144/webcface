#!/usr/bin/env bash
set -e
cd `dirname $0`
pushd ../../..
[ -d build ] || mkdir build
cd build
cmake ..
make -j$(nproc)
sudo make install
popd
[ -d build_i ] && rm -rf build_i
mkdir build_i
cd build_i
cmake .. -DSUBDIR=off
make -j$(nproc) VERBOSE=1
cd ..
[ -d build_s ] && rm -rf build_s
mkdir build_s
cd build_s
cmake .. -DSUBDIR=on
make -j$(nproc) VERBOSE=1
