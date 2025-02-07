/// @file config.hpp
/// @brief 备份系统配置文件
/// 该文件定义了在整个备份系统项目中使用的各种配置设置和常量。
/// 包含但不限于：
/// - 一些常量：版本号、备份文件路径、控制台界面颜色、要屏蔽的路径等；
/// - 将在命令行设定的参数：线程数量、是否启用MD5缓存等。

// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <set>

#include <boost/filesystem.hpp>
#include <cassert>
#include <format>
#include <functional>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::format;
using std::string;
using std::u8string;
using std::vector;

/// MD5缓存在二进制文件中；相对地，使用文本文件。后续将被移除，完全使用二进制文件。
#define BINARY_CACHED_MD5

namespace fs = std::filesystem;

/// 配置命名空间，包含项目中使用的常量和部分变量。
namespace config {
typedef unsigned long long ull;

/// 备份系统版本号
const string VERSION = "0.0.1";

/// MD5缓存目录的路径，基于当前版本。
const fs::path PATH_MD5_DATA = format("./.md5_cache_v{}", VERSION);

/// 备份副本目录的路径。
const fs::path PATH_BACKUP_COPIES = "./backup_copies";

/// 备份数据目录的路径，基于当前版本。
const fs::path PATH_BACKUP_DATA = format("./backup_v{}", VERSION);

/// 日志文件的外部路径，依赖于环境变量CALLED_TIME，restore中还依赖目标文件夹。
extern fs::path PATH_LOGS;

/// 多行输入结束标志。
const string INPUT_END_FLAG = "$END";

/// JSON转储缩进设置，`-1`即无缩进。
const int JSON_DUMP_INDENT = -1;

/// JSON转储缩进的字符。
const int JSON_DUMP_INDENT_CHAR = ' ';

/// 一组将被忽略的路径。
extern std::set<fs::path> IGNORED_PATH;

// the following are defined by command line arguments
extern int THREAD_NUM;
extern bool SHOULD_CHECK_CACHED_MD5;
} // namespace config

namespace print::progress_bar {
/// 用于表示进度的字符。
const char PROGRESS_CHAR = '#';

/// 进度条剩余部分的填充字符。
const char BAR_CHAR = '-';

/// 进度条的总长度。
const int BLOCK_NUM = 50;

/// 二重进度条第一部分的颜色。
const char *const PROGRESS_COLOR1 = "\033[38;2;200;100;100m";

/// 二重进度条第二部分的颜色。
const char *const PROGRESS_COLOR2 = "\033[38;2;100;100;200m";

/// 二重进度条混合部分的颜色。
const char *const PROGRESS_MIXED_COLOR = "\033[38;2;150;75;150m";
} // namespace print::progress_bar

namespace fileinfo {
/// 读取文件时使用的缓冲区大小。
const size_t READ_FILE_BUFFER_SIZE = 1 << 15;
} // namespace fileinfo

// str_encode.cpp: 额外定义了编码识别的默认语言

#endif