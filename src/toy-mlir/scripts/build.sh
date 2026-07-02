#!/bin/bash
rm -rf build
mkdir -p build
cmake -S . -B build && make -C build -j $(nproc)