# This file is part of BackupSystem - a C++ project.
# 
# Licensed under the MIT License. See LICENSE file in the root directory for details.

cmake_minimum_required(VERSION 3.10)
project(BackupSystemTests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/../share/include)

# 添加测试可执行文件
add_executable(file_info_md5_tests 
    test_file_info_md5.cpp
    ${CMAKE_SOURCE_DIR}/../share/src/file_info_md5.cpp
    ${CMAKE_SOURCE_DIR}/../share/src/file_info.cpp
    ${CMAKE_SOURCE_DIR}/../share/src/config.cpp
    ${CMAKE_SOURCE_DIR}/../share/src/print.cpp
)

# 链接库
target_link_libraries(file_info_md5_tests PRIVATE
    GTest::GTest GTest::Main
    OpenSSL::SSL OpenSSL::Crypto
)

# 添加测试
enable_testing()
add_test(NAME file_info_md5_tests 
    COMMAND file_info_md5_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)