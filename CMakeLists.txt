# This file is part of BackupSystem - a C++ project.
# 
# Licensed under the MIT License. See LICENSE file in the root directory for details.

cmake_minimum_required(VERSION 3.10)
project(BackupSystem)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
set(CMAKE_DEBUG_POSTFIX "_d")

# 设置默认构建类型为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Default to Release build" FORCE)
endif()

# 查找依赖库
find_package(Boost REQUIRED COMPONENTS program_options locale)
find_package(OpenSSL REQUIRED)

# 设置库路径
if(WIN32)
    link_directories(${CMAKE_SOURCE_DIR}/lib)
    link_directories(${CMAKE_SOURCE_DIR}/lib/mingw)
else()
    link_directories(${CMAKE_SOURCE_DIR}/lib)
    link_directories(${CMAKE_SOURCE_DIR}/lib/linux_gnu)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/share/include)

# 源文件
set(BACKUP_SRC
    backup/main.cpp
    backup/src/head.cpp
    share/src/file_info_md5.cpp
)

set(RESTORE_SRC
    restore/main.cpp
)

set(SHARED_SRC
    share/src/env.cpp
    share/src/print.cpp
    share/src/str_encode.cpp
    share/src/config.cpp
    share/src/file_info.cpp
    share/src/ThreadPool.cpp
)

# 添加可执行文件
add_executable(backup ${BACKUP_SRC} ${SHARED_SRC})
add_executable(restore ${RESTORE_SRC} ${SHARED_SRC})

# 设置目标属性
set_target_properties(backup PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_CURRENT_BINARY_DIR}"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# 添加 include 目录
target_include_directories(backup PRIVATE
    ${CMAKE_SOURCE_DIR}/backup/include
)

# 添加编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug mode")
    target_compile_definitions(backup PRIVATE DEBUG_MODE)
    target_compile_definitions(restore PRIVATE DEBUG_MODE)
    target_compile_options(backup PRIVATE -g3)
    target_compile_options(restore PRIVATE -g3)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Release mode")
    target_compile_definitions(backup PRIVATE RELEASE_MODE NDEBUG)
    target_compile_definitions(restore PRIVATE RELEASE_MODE NDEBUG)
    target_compile_options(backup PRIVATE -O2)
    target_compile_options(restore PRIVATE -O2)
endif()

# 链接库
target_link_libraries(backup PRIVATE
    ${Boost_LIBRARIES}
    OpenSSL::SSL OpenSSL::Crypto
    ced
)

target_link_libraries(restore PRIVATE
    ${Boost_LIBRARIES}
    ced
)

# 安装规则
install(TARGETS backup restore DESTINATION bin)