#!/bin/bash
rm -rf build
mkdir -p build
CC=clang CXX=clang++ cmake -S . -B build make .. \
-DLLVM_DIR=/home/shaokai/Desktop/code/llvm/triton-cpu/llvm-project/build/lib/cmake/llvm \
-DMLIR_DIR=/home/shaokai/Desktop/code/llvm/triton-cpu/llvm-project/build/lib/cmake/mlir
make -C build -j $(nproc)
