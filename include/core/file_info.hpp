/// @file file_info.hpp
/// @brief 该文件定义了用于处理文件信息的类和函数。
/// 
/// 这个模块包含以下主要功能：
/// - 描述文件信息，包括文件名及路径、修改时间、文件大小和 MD5 校验值。
/// 
/// 该模块依赖于以下其他模块：
/// - config.hpp: 配置相关的功能。
/// - env.hpp: 环境变量处理的功能。
/// - print.hpp: 打印调试信息的功能。
/// - nlohmann/json.hpp: JSON 处理的功能。

// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once

#ifndef _FILEINFO_H_
#define _FILEINFO_H_

#include <condition_variable>
#include <queue>
#include <string>
#include <functional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "nlohmann/json.hpp"
#pragma GCC diagnostic pop

#include "config.hpp"
#include "env.hpp"
#include "print.hpp"

namespace fileinfo {
using nlohmann::json;
using std::string;
using std::vector;
typedef unsigned long long ull;

/// @brief 将 filesystem::file_time_type 转换为 std::time_t。
/// @param ftime 要转换的文件时间类型。
/// @return 转换后的时间，格式为 std::time_t。
time_t file_time_type2time_t(fs::file_time_type ftime);

/// @brief: 描述文件信息：文件名及路径、修改时间、文件大小、md5校验值
class FileInfo {
    friend void from_json(const json &j, FileInfo &f);
    friend void to_json(json &j, const FileInfo &f);
    friend void calculate_md5_value(FileInfo &);

  public:
    /// @brief 默认构造函数，将文件大小初始化为 0。
    FileInfo() : file_size(0) {}

    /// @brief 为给定路径构造一个 FileInfo 对象。
    /// @details 初始化文件的路径、修改时间、大小和 MD5 值。如果文件不存在，记录错误信息并设置默认值。
    /// @param path 文件的路径。
    FileInfo(const fs::path &path);

    const fs::path &get_path() const { return path; }
    const time_t &get_modified_time() const { return modified_time; }
    const ull &get_file_size() const { return file_size; }
    const string &get_md5_value() const { return md5_value; }

  private:
    fs::path path;
    time_t modified_time;
    ull file_size;

    string md5_value;
};
} // namespace fileinfo
#endif //_FILEINFO_H_