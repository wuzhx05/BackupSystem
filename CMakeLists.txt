cmake_minimum_required(VERSION 3.10)
project(BackupSystem)

# 设置默认构建类型：Release
if(NOT CMAKE_BUILD_TYPE)  
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Default to Release build" FORCE)  
endif() 

# 基本配置
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX "_d")

# 输出路径设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)  
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)  
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找依赖
find_package(Boost REQUIRED COMPONENTS program_options locale)
find_package(OpenSSL REQUIRED)

# 平台特定库配置
if(WIN32)
    link_directories(${CMAKE_SOURCE_DIR}/lib/mingw)
else()
    link_directories(${CMAKE_SOURCE_DIR}/lib/linux_gnu)
endif()

# 公共头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/include/3rdparty
    ${CMAKE_SOURCE_DIR}/include/common
    ${CMAKE_SOURCE_DIR}/include/core
)

# 核心库源码
file(GLOB CORE_SOURCES 
    ${CMAKE_SOURCE_DIR}/src/common/*.cpp
    ${CMAKE_SOURCE_DIR}/src/core/*.cpp
)

# 构建核心静态库
add_library(CoreLib STATIC ${CORE_SOURCES})
target_link_libraries(CoreLib PRIVATE
    ${Boost_LIBRARIES}
    OpenSSL::SSL OpenSSL::Crypto
    ced
)

# 添加子项目
add_subdirectory(backup)
add_subdirectory(restore)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(test)
endif()

# 安装规则
install(TARGETS backup restore DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)