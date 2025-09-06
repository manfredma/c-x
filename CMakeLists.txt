cmake_minimum_required(VERSION 3.10)
project(c_x VERSION 0.1.0 DESCRIPTION "A cross-platform C project" LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    # MSVC编译器选项
    add_compile_options(/W4)
else()
    # GCC/Clang编译器选项
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/main.c
    src/kv_store.c
    src/http_parser.c
    src/kqueue_net.c
)

# 主可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 测试支持
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 版本信息
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h
)

# 输出配置信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
