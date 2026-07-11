add_executable(test_mlir_ch1_lexer
  test_Lexer.cpp
)

target_link_libraries(test_mlir_ch1_lexer PRIVATE test_common)

add_test(
  NAME mlir-ch1-lexer-smoke
  COMMAND test_mlir_ch1_lexer ${CMAKE_CURRENT_SOURCE_DIR}/test_Lexer.toy
)
