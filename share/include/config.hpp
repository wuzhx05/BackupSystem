// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once

#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <filesystem>

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

namespace fs = std::filesystem;
namespace config {
const string VERSION = "0.0.1";

// const fs::path PATH_MD5_DATA = format("./.md5_data_v{}", VERSION);
const fs::path PATH_LOGS = "./logs.txt";
const fs::path PATH_BACKUP_COPIES = "./backup_copies";
const fs::path PATH_BACKUP_DATA = format("./backup_v{}", VERSION);
const string INPUT_END_FLAG = "$END";

const int JSON_DUMP_INDENT = -1;
const int JSON_DUMP_INDENT_CHAR = ' ';

typedef unsigned long long ull;

extern std::set<u8string> IGNORED_PATH;
} // namespace config

namespace print::progress_bar {
const char PROGRESS_CHAR = '#';
const char BAR_CHAR = '-';
const int BLOCK_NUM = 50;
const char *const PROGRESS_COLOR1 = "\033[38;2;200;100;100m";
const char *const PROGRESS_COLOR2 = "\033[38;2;100;100;200m";
const char *const PROGRESS_MIXED_COLOR = "\033[38;2;150;75;150m";
} // namespace print::progress_bar

// str_code: define string code and language
