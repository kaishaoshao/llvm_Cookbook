# 头文件目录声明
add_library(INCLUDE_DIRS INTERFACE)
target_include_directories(INCLUDE_DIRS INTERFACE
${CMAKE_CURRENT_LIST_DIR}
)
