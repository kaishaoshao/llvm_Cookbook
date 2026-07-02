cmake_minimum_required(VERSION 3.20)
project(toy-mlir)

# 全局配置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

  find_package(LLVM REQUIRED CONFIG)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  find_package(MLIR REQUIRED CONFIG)

  message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
  set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})
 
#   list(APPEND CMAKE_MODULE_PATH "{MLIR_CMAKE_DIR}")
#   list(APPEND CMAKE_MODULE_PATH "{LLVM_CMAKE_DIR}")
  list(APPEND CMAKE_MODULE_PATH "/usr/lib/llvm-20/lib/cmake/llvm")
  list(APPEND CMAKE_MODULE_PATH "/usr/lib/llvm-20/lib/cmake/mlir")

  include(TableGen)
  include(AddLLVM)
  include(AddMLIR)
  include(HandleLLVMOptions)
else()
  set(MLIR_MAIN_SRC_DIR ${LLVM_MAIN_SRC_DIR}/../mlir)
  set(MLIR_INCLUDE_DIR ${MLIR_MAIN_SRC_DIR}/include)
  set(MLIR_GENERATED_INCLUDE_DIR ${LLVM_BINARY_DIR}/tools/mlir/include)
  set(MLIR_INCLUDE_DIRS "${MLIR_INCLUDE_DIR};${MLIR_GENERATED_INCLUDE_DIR}")
endif()


# 包含 LLVM 头文件和定义
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${LLVM_BUILD_LIBRARY_DIR})
add_definitions(${LLVM_DEFINITIONS})
# 映射所有需要的 LLVM 组件
llvm_map_components_to_libnames(llvm_libs
    Core          # 核心数据结构
    Support       # 基础工具（MemoryBuffer、errs() 等）
)
# MLIR 相关组件
get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(translation_libs GLOBAL PROPERTY MLIR_TRANSLATION_LIBS)

# 开启内存检查
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
add_compile_options(-fsanitize=undefined)
add_link_options(-fsanitize=undefined)

add_subdirectory(include)
add_subdirectory(parser)
add_subdirectory(dialect)

# add_subdirectory(test)

add_executable(toy-mlir toyc.cpp)

set(LLVM20_LIB_DIR /usr/lib/llvm-20/lib)
set(MLIR_LIBS
    ${LLVM20_LIB_DIR}/libMLIRIR.a
    ${LLVM20_LIB_DIR}/libMLIRParser.a
    ${LLVM20_LIB_DIR}/libMLIRPass.a
    ${LLVM20_LIB_DIR}/libMLIRSPIRVDialect.a
    ${LLVM20_LIB_DIR}/libMLIRTranslateLib.a
    ${LLVM20_LIB_DIR}/libMLIRSupport.a
)

target_link_libraries(toy-mlir
PRIVATE
    AST_LIB
    ${llvm_libs}
    ${MLIR_LIBS}
)
