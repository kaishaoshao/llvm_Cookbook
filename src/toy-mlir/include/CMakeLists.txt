add_library(toy_headers INTERFACE)
target_include_directories(toy_headers INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/Frontend
  ${LLVM_INCLUDE_DIRS}
)
