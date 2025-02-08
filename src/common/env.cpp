/// @file env.cpp
/// @brief env.hpp的实现文件
//
// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#include <chrono>

#include "env.hpp"
#include "print.hpp"

namespace env {

std::string CALLED_TIME;
std::string UUID;

/// @brief 初始化全局变量 `CALLED_TIME` 和 `UUID`。
/// @details 这个函数将 `CALLED_TIME` 设置为当前时间的格式化字符串 "YYYY_MM_DD_HH_MM_SS"，并将 `UUID` 设置为 "NULL"。
static void init() {
    CALLED_TIME = get_current_time("%Y_%m_%d_%H_%M_%S");
    UUID = "NULL";
}
void backup_init() {
    init();
    config::PATH_LOGS = config::PATH_BACKUP_DATA / CALLED_TIME / "log.txt";
}
void restore_init(fs::path output_folder) {
    init();
    config::PATH_LOGS = output_folder / (CALLED_TIME + "_restore_log.txt");
}

std::string get_current_time(const char* format) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, format);
    return oss.str();
}

} // namespace env