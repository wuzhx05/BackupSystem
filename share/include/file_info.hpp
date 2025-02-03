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

namespace file_info {
using nlohmann::json;
using std::string;
using std::vector;
typedef unsigned long long ull;

time_t file_time_type2time_t(fs::file_time_type ftime);

/**
 * @brief: 文件名及路径、修改时间、文件大小、md5校验值
 */
class FileInfo {
    friend void from_json(const json &j, FileInfo &f);
    friend void to_json(json &j, const FileInfo &f);
    friend void calculate_md5_value(FileInfo &);

  public:
    FileInfo() : file_size(0) {}

    /**
     * @brief: 初始化path, modified_time, file_size
     */
    FileInfo(const u8string &path);

    const u8string &get_path() const { return path; }
    const time_t &get_modified_time() const { return modified_time; }
    const ull &get_file_size() const { return file_size; }
    const string &get_md5_value() const { return md5_value; }

  private:
    u8string path;
    time_t modified_time;
    ull file_size;

    string md5_value;
};
} // namespace file_info
#endif //_FILEINFO_H_