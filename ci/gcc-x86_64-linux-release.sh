#!/usr/bin/env sh
set -e

cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -S . -B build
cd build
make "-j$(nproc)"
