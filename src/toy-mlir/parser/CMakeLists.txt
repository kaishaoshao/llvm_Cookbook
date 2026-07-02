file(GLOB SOURCES *.cpp)
add_library(AST_LIB STATIC ${SOURCES})

# 链接头文件
target_link_libraries(
    AST_LIB PRIVATE 
    INCLUDE_DIRS
)

# 安装规则
install(TARGETS AST_LIB DESTINATION lib)
# install(FILES AST.h DESTINATION include)