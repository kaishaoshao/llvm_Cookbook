add_library(toy_frontend STATIC
  AST.cpp
  Parser.cpp
)

target_link_libraries(toy_frontend
  PUBLIC
    toy_headers
    ${llvm_libs}
)

target_include_directories(toy_frontend
  PRIVATE
    ${LLVM_INCLUDE_DIRS}
)
