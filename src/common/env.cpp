// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#include <chrono>

#include "env.hpp"
#include "print.hpp"

namespace env {

std::string CALLED_TIME;
std::string UUID;

static void init() {
    CALLED_TIME = getCurrentTime("%Y_%m_%d_%H_%M_%S");
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

std::string getCurrentTime(const char* format) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, format);
    return oss.str();
}

} // namespace env