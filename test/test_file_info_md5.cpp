/// @file test_file_info_md5.cpp
/// @brief 测试 FileInfo 类的 MD5 值计算与缓存
/// @author deepseek-R1

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <regex>
#include <vector>

#include "file_info_md5.hpp"
#include <openssl/evp.h>

namespace fs = std::filesystem;

class FileInfoMD5Test : public ::testing::Test {
  protected:
    void SetUp() override {
        // 创建测试文件
        fs::create_directory("test_files");

        // 空文件
        std::ofstream("test_files/empty.txt").close();

        // 生成 1MB 文件
        std::vector<char> data(1024 * 1024, 0xAA);
        std::ofstream("test_files/test1.bin", std::ios::binary)
            .write(data.data(), data.size());

        // 生成不同内容的文件
        data[512] = 0xBB;
        std::ofstream("test_files/test2.bin", std::ios::binary)
            .write(data.data(), data.size());
    }

    void TearDown() override { fs::remove_all("test_files"); }
};

class MD5ComprehensiveTest : public ::testing::Test {
  protected:
    std::vector<fs::path> test_files;

    // 生成随机内容文件
    void GenerateRandomFile(const fs::path &filename, size_t size) {
        std::ofstream file(filename, std::ios::binary);
        std::vector<char> buffer(size);

        // 填充随机数据（使用伪随机生成器）
        std::generate(buffer.begin(), buffer.end(),
                      []() { return static_cast<char>(rand() % 256); });

        file.write(buffer.data(), buffer.size());
        test_files.push_back(filename);
    }

    // 获取系统计算的 MD5 值（跨平台）
    std::string GetSystemMD5(std::string filename) {
        std::string cmd;

        filename = "\"" + filename + "\"";

#if defined(__APPLE__)
        cmd = "md5 -q " + filename; // macOS 命令
#else
        cmd = "md5sum " + filename + " | awk '{print $1}'"; // Linux / Windows 命令
#endif

        // 执行命令并捕获输出
        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe)
            return "";

        char buffer[128];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);

        // 清理换行符
        result.erase(std::remove(result.begin(), result.end(), '\n'),
                     result.end());
        return result;
    }

    void TearDown() override {
        for (const auto &file : test_files) {
            fs::remove(file);
        }
    }
};

// 测试空文件 MD5
TEST_F(FileInfoMD5Test, EmptyFile) {
    fileinfo::FileInfo file(u8"test_files/empty.txt");
    fileinfo::calculate_md5_value(file);
    EXPECT_EQ(file.get_md5_value(), "D41D8CD98F00B204E9800998ECF8427E");
}

// 测试文件内容变化检测
TEST_F(FileInfoMD5Test, ContentChangeDetection) {
    fileinfo::FileInfo file1(u8"test_files/test1.bin");
    fileinfo::FileInfo file2(u8"test_files/test2.bin");

    fileinfo::calculate_md5_value(file1);
    fileinfo::calculate_md5_value(file2);

    EXPECT_NE(file1.get_md5_value(), file2.get_md5_value());
}

// 测试缓存功能
TEST_F(FileInfoMD5Test, CacheFunctionality) {
    config::SHOULD_CHECK_CACHED_MD5 = true;

    fileinfo::FileInfo file(u8"test_files/test1.bin");
    fileinfo::FileInfo file2(u8"test_files/test1.bin");
    fileinfo::calculate_md5_value(file);  // 首次计算
    fileinfo::calculate_md5_value(file2); // 第二次应从缓存读取

    EXPECT_EQ(file.get_md5_value(), file2.get_md5_value());
}

// 测试多线程安全
#include <thread>
#include <vector>
TEST_F(FileInfoMD5Test, ThreadSafety) {
    constexpr int THREAD_NUM = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back([]() {
            fileinfo::FileInfo file(u8"test_files/test1.bin");
            for (int j = 0; j < 100; ++j) {
                fileinfo::calculate_md5_value(file);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    // 没有崩溃即通过基本测试
    SUCCEED();
}

// 测试异常处理
TEST_F(FileInfoMD5Test, InvalidFileHandling) {
    fileinfo::FileInfo file(u8"non_existent_file.txt");
    EXPECT_THROW(fileinfo::calculate_md5_value(file), std::runtime_error);
}

// <--------------MD5ComprehensiveTest-------------->

// 测试不同文件大小的边界条件
TEST_F(MD5ComprehensiveTest, BoundarySizes) {
    const std::vector<size_t> sizes = {
        0,                    // 空文件
        1,                    // 1字节
        EVP_MAX_MD_SIZE - 1,  // MD5 输出的缓冲区 - 1
        EVP_MAX_MD_SIZE,      // MD5 输出的缓冲区
        EVP_MAX_MD_SIZE + 1,  // MD5 输出的缓冲区 + 1
        EVP_MAX_MD_SIZE << 1, // MD5 输出的缓冲区 * 2
        32767,                // 缓冲区大小-1
        32768,                // 缓冲区大小（1 << 15）
        32769,                // 缓冲区大小+1
        1024 * 1024,          // 1MB
        10 * 1024 * 1024      // 10MB
    };

    for (size_t size : sizes) {
        fs::path filename = "test_file_" + std::to_string(size) + ".bin";
        GenerateRandomFile(filename, size);

        // 计算系统 MD5
        std::string system_md5 = GetSystemMD5(filename.string());
        ASSERT_FALSE(system_md5.empty())
            << "Failed to get system MD5 for " << filename;

        // 计算自定义 MD5
        fileinfo::FileInfo file(filename);
        fileinfo::calculate_md5_value(file);
        std::string custom_md5 = file.get_md5_value();

        // 转换为大写比较（系统工具输出为小写）
        std::transform(custom_md5.begin(), custom_md5.end(), custom_md5.begin(),
                       ::toupper);
        std::transform(system_md5.begin(), system_md5.end(), system_md5.begin(),
                       ::toupper);

        EXPECT_EQ(custom_md5, system_md5)
            << "MD5 mismatch for size=" << size << "\nSystem: " << system_md5
            << "\nCustom: " << custom_md5;
    }
}

TEST_F(MD5ComprehensiveTest, SmallSizes) {
    constexpr size_t MAX_SIZE = 2048; // 最大 2KB

    for (size_t size = 0; size < MAX_SIZE; ++size) {
        std::string filename = "small_size_test_" + std::to_string(size) + ".bin";
        GenerateRandomFile(filename, size);

        std::string system_md5 = GetSystemMD5(filename);
        ASSERT_FALSE(system_md5.empty()) << "Test " << size << " failed";

        fileinfo::FileInfo file{fs::path(filename).u8string()};
        fileinfo::calculate_md5_value(file);
        std::string custom_md5 = file.get_md5_value();
        std::transform(custom_md5.begin(), custom_md5.end(), custom_md5.begin(),
                       ::toupper);
        std::transform(system_md5.begin(), system_md5.end(), system_md5.begin(),
                       ::toupper);

        EXPECT_EQ(custom_md5, system_md5)
            << "Test " << size << " (size=" << size << ") failed";
    }
}

TEST_F(MD5ComprehensiveTest, RandomSizes) {
    constexpr int NUM_TESTS = 50;                // 测试次数
    constexpr size_t MAX_SIZE = 5 * 1024 * 1024; // 最大 5MB

    for (int i = 0; i < NUM_TESTS; ++i) {
        size_t size = rand() % MAX_SIZE;
        std::string filename = "random_test_" + std::to_string(i) + ".bin";
        GenerateRandomFile(filename, size);

        std::string system_md5 = GetSystemMD5(filename);
        ASSERT_FALSE(system_md5.empty()) << "Test " << i << " failed";

        fileinfo::FileInfo file{fs::path(filename).u8string()};
        fileinfo::calculate_md5_value(file);
        std::string custom_md5 = file.get_md5_value();
        std::transform(custom_md5.begin(), custom_md5.end(), custom_md5.begin(),
                       ::toupper);
        std::transform(system_md5.begin(), system_md5.end(), system_md5.begin(),
                       ::toupper);

        EXPECT_EQ(custom_md5, system_md5)
            << "Test " << i << " (size=" << size << ") failed";
    }
}

TEST_F(MD5ComprehensiveTest, SpecialFilenames) {
    const std::vector<std::string> filenames = {
        "file with spaces.bin",
        "file!@#$%^&()_+{}[].bin",
    };

    for (const auto &name : filenames) {
        GenerateRandomFile(name, 1024); // 1KB 文件

        std::string system_md5 = GetSystemMD5(name);
        ASSERT_FALSE(system_md5.empty()) << "Failed for filename: " << name;

        fileinfo::FileInfo file{fs::path(name).u8string()};
        fileinfo::calculate_md5_value(file);
        std::string custom_md5 = file.get_md5_value();
        std::transform(custom_md5.begin(), custom_md5.end(), custom_md5.begin(),
                       ::toupper);
        std::transform(system_md5.begin(), system_md5.end(), system_md5.begin(),
                       ::toupper);

        EXPECT_EQ(custom_md5, system_md5) << "Failed for filename: " << name;
    }
}