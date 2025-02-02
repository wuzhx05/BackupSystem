cmake_minimum_required(VERSION 3.10)  
project(BackupSystem)  

if(NOT CMAKE_BUILD_TYPE)  
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Default to Release build" FORCE)  
endif()  

set(CMAKE_CXX_STANDARD 23)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  

# 设置输出路径  
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  

# 添加调试和发布选项  
set(CMAKE_DEBUG_POSTFIX "_d")  
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)  
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)  

# library path  
if (WIN32)  
    message(STATUS "Windows platform")  
    link_directories(${CMAKE_SOURCE_DIR}/lib_mingw)  # 使用 mingw 目录  
    # find_library(CED_LIBRARY ced PATHS "${CMAKE_SOURCE_DIR}/lib_mingw")  
else()  
    link_directories(${CMAKE_SOURCE_DIR}/lib)  
    # find_library(CED_LIBRARY ced PATHS "${CMAKE_SOURCE_DIR}/lib")  
endif()  

find_package(Boost REQUIRED COMPONENTS  
    program_options  
    locale  
)  
find_package(OpenSSL REQUIRED)  

# include path  
include_directories(  
    ${CMAKE_SOURCE_DIR}/share/include  
)  

# source files  
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

set_target_properties(backup PROPERTIES  
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_CURRENT_BINARY_DIR}"  
    BUILD_WITH_INSTALL_RPATH TRUE  
    INSTALL_RPATH_USE_LINK_PATH TRUE  
)  

target_include_directories(backup PRIVATE  
    ${CMAKE_SOURCE_DIR}/backup/include  
)  

if(CMAKE_BUILD_TYPE STREQUAL "Debug")  
    # Debug模式下的编译选项  
    message("Debug mode")  
    target_compile_definitions(backup PRIVATE DEBUG_MODE)  # define  
    target_compile_definitions(restore PRIVATE DEBUG_MODE)  
    target_compile_options(backup PRIVATE -g3)  # 添加调试信息  
    target_compile_options(restore PRIVATE -g3)  
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")  
    # Release模式下的编译选项  
    message("Release mode")  
    target_compile_definitions(backup PRIVATE RELEASE_MODE NDEBUG)  # define  
    target_compile_definitions(restore PRIVATE RELEASE_MODE NDEBUG)  
    target_compile_options(backup PRIVATE -O2)  # 优化选项  
    target_compile_options(restore PRIVATE -O2)  
endif()  

# 链接库  
target_link_libraries(backup PRIVATE  
    ${Boost_LIBRARIES}  
    ${CED_LIBRARY}  
    OpenSSL::SSL OpenSSL::Crypto  
    ced
)  
target_link_libraries(restore PRIVATE  
    ${Boost_LIBRARIES}  
    ${CED_LIBRARY}  
    ced
)  

# 安装规则  
install(TARGETS backup restore DESTINATION bin)