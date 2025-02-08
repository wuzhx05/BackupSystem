/// @file file_info.cpp
/// @brief file_info.hpp的实现。
//
// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include <cassert>
#include <chrono>
#include <filesystem>
#include <format>
#include <sstream>

#include "file_info.hpp"
#include "nlohmann/json.hpp"
#include "print.hpp"

namespace fileinfo {
time_t file_time_type2time_t(fs::file_time_type ftime) {
    auto systemTimePoint =
        std::chrono::clock_cast<std::chrono::system_clock>(ftime);
    return std::chrono::system_clock::to_time_t(systemTimePoint);
}

void from_json(const json &j, FileInfo &f) {
    std::string path;
    j.at("path").get_to(path), f.path = path;
    j.at("modified").get_to(f.modified_time);
    j.at("size").get_to(f.file_size);
    j.at("md5").get_to(f.md5_value);
}
void to_json(json &j, const FileInfo &f) {
    std::u8string p = f.path.u8string();
    j = json{{"path", string(p.begin(), p.end())},
             {"modified", f.modified_time},
             {"size", f.file_size},
             {"md5", f.md5_value}};
}

FileInfo::FileInfo(const fs::path &path)
    : path(path), modified_time(0), file_size(0), md5_value("") {
    if (!std::filesystem::exists(path)) {
        print::log(print::ERROR, "[ERROR] FileInfo: File does not exist");
        return;
        // throw std::runtime_error(
        //     "FileInfo: File does not exist"); // TODO: 异常处理，鲁棒性？
    }

    std::error_code ec;
    auto _modified_time =
        std::filesystem::last_write_time(path, ec);
    if (!ec) {
        modified_time = file_time_type2time_t(_modified_time);
    } else {
        std::cerr << "[ERROR] FileInfo: Failed to retrieve modification time: "
                  << ec.message() << std::endl;
    }

    file_size = std::filesystem::file_size(path);
}

} // namespace fileinfo