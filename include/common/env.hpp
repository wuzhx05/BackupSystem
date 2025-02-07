/// @file include/common/env.hpp
/// @brief 这个文件包含BackupSystem项目的与环境相关的函数实现。

// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#pragma once
#ifndef ENV
#define ENV

#include <filesystem>
#include <string>

namespace env {
/// 调用时间
extern std::string CALLED_TIME;
/// UUID（未实现）
extern std::string UUID;
namespace fs = std::filesystem;

/// @brief 初始化backup，通过设置日志文件路径基于当前时间。
/// @details 初始化全局变量，设置 `config::PATH_LOGS`为
/// `{config::PATH_BACKUP_DATA}/{CALLED_TIME}/log.txt`。
void backup_init();

/// @brief 初始化restore，通过设置日志文件路径基于输出文件夹和特定的格式字符串。
/// @param output_folder restore的目标文件夹，日志文件将被保存在此。
/// @details
/// 初始化全局变量，设置`config::PATH_LOGS`为`{output_folder}/{CALLED_TIME}__restore_log.txt`。
void restore_init(fs::path output_folder);

/// @brief 根据指定的格式字符串返回当前系统时间的格式化字符串。
/// @param format 指定格式，如"YYYY_MM_DD_HH_MM_SS"。
/// @return 包含格式化日期和时间的 std::string。
/// @details
/// 这个函数获取当前系统时间，将其转换为本地时间表示形式，格式化为`format`指定形式，并返回格式化的字符串。
std::string getCurrentTime(const char *format);

} // namespace env
#endif