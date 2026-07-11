add_test(
  NAME mlir-ch2-ast-smoke
  COMMAND $<TARGET_FILE:mlir-ch2> ${CMAKE_CURRENT_SOURCE_DIR}/codegen.toy -emit=ast
)

add_test(
  NAME mlir-ch2-emit-mlir-todo
  COMMAND $<TARGET_FILE:mlir-ch2> ${CMAKE_CURRENT_SOURCE_DIR}/codegen.toy -emit=mlir
)

set_tests_properties(
  mlir-ch2-emit-mlir-todo
  PROPERTIES
    WILL_FAIL TRUE
    PASS_REGULAR_EXPRESSION "MLIR emission for mlir-ch2 is not implemented yet"
)
