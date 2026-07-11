cmake_minimum_required(VERSION 3.20)
project(toy-mlir LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}")

include(CTest)

add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs
  Core
  Support
)

add_subdirectory(include)
add_subdirectory(src)

function(add_mlir_chapter chapter_target chapter_source)
  add_executable(${chapter_target} ${chapter_source})
  target_include_directories(${chapter_target} PRIVATE ${LLVM_INCLUDE_DIRS})
  target_link_libraries(${chapter_target} PRIVATE toy_frontend toy_headers ${llvm_libs})
endfunction()

add_mlir_chapter(mlir-ch1 test/ch1/toyc.cpp)
add_mlir_chapter(mlir-ch2 test/ch2/toyc.cpp)

add_subdirectory(test)
