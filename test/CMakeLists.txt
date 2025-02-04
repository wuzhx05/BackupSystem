# test/CMakeLists.txt
project(BackupSystemTests)

# 启用测试
enable_testing()

# 查找 GTest
find_package(GTest REQUIRED)

# 定义测试可执行文件路径（根据构建模式）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TEST_EXECUTABLE ${CMAKE_BINARY_DIR}/bin/debug/test_file_info_md5)
else()
    set(TEST_EXECUTABLE ${CMAKE_BINARY_DIR}/bin/test_file_info_md5)
endif()

# 添加测试可执行文件
add_executable(test_file_info_md5 test_file_info_md5.cpp)

# 链接依赖库
target_link_libraries(test_file_info_md5 PRIVATE
    CoreLib
    GTest::GTest
    GTest::Main
)

# 注册测试用例
add_test(
    NAME FileInfoMD5Test
    COMMAND ${TEST_EXECUTABLE}
    # WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)