project(Backup)

# 包含子项目特有头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件配置
file(GLOB BACKUP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# 创建可执行文件
add_executable(backup ${BACKUP_SOURCES})

# 链接依赖库
target_link_libraries(backup PRIVATE
    CoreLib
    ced # 平台特定库
)

# 构建模式配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(backup PRIVATE DEBUG_MODE)
    target_compile_options(backup PRIVATE -g3)
else()
    target_compile_definitions(backup PRIVATE RELEASE_MODE)
    target_compile_options(backup PRIVATE -O2)
endif()