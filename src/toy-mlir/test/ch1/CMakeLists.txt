# 单个测试用例构建
add_executable(test_Lexer
    test_Lexer.cpp
)

target_link_libraries(test_Lexer PRIVATE
    AST_LIB
)

add_test(NAME lexer_test COMMAND test_Lexer ${CMAKE_CURRENT_SOURCE_DIR}/test_Lexer.toy)
