# This file is part of BackupSystem - a C++ project.
# 
# Licensed under the MIT License. See LICENSE file in the root directory for details.

project(Restore)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB RESTORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/str_similarity.cpp
)

add_executable(restore ${RESTORE_SOURCES})

target_link_libraries(restore PRIVATE
    CoreLib
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(restore PRIVATE DEBUG_MODE)
    target_compile_options(restore PRIVATE -g3)
else()
    target_compile_definitions(restore PRIVATE RELEASE_MODE)
    target_compile_options(restore PRIVATE -O2)
endif()