
MLIR_DIR=/home/shaokai/Desktop/code/llvm/tritons/triton/llvm-project/build-mlir/lib/cmake/mlir
LLVM_EXTERNAL_LIT=/home/shaokai/Desktop/code/llvm/tritons/triton/llvm-project/build-mlir/bin/llvm-lit


cmake -G Ninja -B build -S . -DMLIR_DIR=$MLIR_DIR -DLLVM_EXTERNAL_LIT=$LLVM_EXTERNAL_LIT
cmake --build build --target toy-mlir