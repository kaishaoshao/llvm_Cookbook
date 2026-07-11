cmake_minimum_required(VERSION 3.20)
project(toy-mlir LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # 静态库输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # 动态库输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin) # 可执行文件输出目录



find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
find_package(MLIR REQUIRED CONFIG)
message(STATUS "Found MLIR ${MLIR_PACKAGE_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)

# 包含 LLVM 和 MLIR 头文件和定义
include_directories(${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS} ${MLIR_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs
  Core
  Support
)

add_subdirectory(include)
add_subdirectory(src)

add_subdirectory(include)
add_subdirectory(paser)
add_subdirectory(test)

add_executable(toy-mlir toyc.cpp)

add_mlir_chapter(mlir-ch1 test/ch1/toyc.cpp)
add_mlir_chapter(mlir-ch2 test/ch2/toyc.cpp)

add_subdirectory(test)
