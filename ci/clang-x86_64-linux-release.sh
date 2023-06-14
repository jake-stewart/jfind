#!/usr/bin/env sh
set -e

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -S . -B build
cd build
make "-j$(nproc)"
