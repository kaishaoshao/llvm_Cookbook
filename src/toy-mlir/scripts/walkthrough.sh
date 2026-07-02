#!/bin/bash
MLIR_PATH=/home/shaokai/Desktop/code/llvm/tritons/triton/llvm-project/build-mlir/bin/
MLIR_SRC_ROOT=/home/shaokai/Desktop/code/llvm/tritons/triton/llvm-project/mlir
MLIR_INCLUDE_DIR=${MLIR_SRC_ROOT}/include/
export PATH="$MLIR_PATH:$PATH"
set -x

TOYC=build/toy-mlir
${TOYC} test/ch1/basic.toy -emit=ast
mlir-tblgen -I${MLIR_INCLUDE_DIR} -gen-dialect-decls toy/Ops.td
mlir-tblgen -I${MLIR_INCLUDE_DIR} -gen-op-defs Dialect/Ops.td &> /dev/null

${TOYC} test/ch2/codegen.toy -emit=mlir -mlir-print-debuginfo 2> test/ch2/codegen.mlir
${TOYC} test/ch2/codegen.mlir -emit=mlir