/// @file restore/src/main.cpp
/// @brief 主函数，实现了restore过程的具体逻辑。
///
// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include "head.hpp"

fs::path input_folder;
fs::path target_folder;
std::vector<fs::path> backuped_paths;
bool overwrite_existing_files = false;

int main(int argc, char *argv[]) {
    // 解析命令行参数
    {
        std::string str_input_folder, str_target_folder;
        if (!parse_command_line_args(argc, argv, str_input_folder,
                                  str_target_folder, overwrite_existing_files))
            return 1;
        input_folder = fs::path(strencode::to_u8string(str_input_folder));
        target_folder = fs::path(strencode::to_u8string(str_target_folder));
    }

    // 初始化
    if (!fs::exists(config::PATH_BACKUP_DATA)) {
        print::cprintln(print::ERROR, "[ERROR] Path not exist: " + config::PATH_BACKUP_DATA.string());
        return 1;
    }
    strencode::init();
    print::cprintln(print::IMPORTANT,
                    "[INFO] Encoding: " + strencode::get_console_encoding());
    if (!fs::exists(target_folder)) {
        fs::create_directories(target_folder);
    }
    env::restore_init(target_folder);
    if (!print::log(print::RESET, "[INFO] Restore started.")) {
        print::cprintln(print::ERROR, "[ERROR] Failed to open the log file.");
        return 1;
    }

    // 选择备份
    if (!select_backup_folder(input_folder, target_folder))
        return 1;

    // 解析备份日志
    if (!parse_backup_log(input_folder, backuped_paths))
        return 1;

    // 确认备份路径
    for (auto &p : backuped_paths) {
        print::log(print::RESET, "[INFO] Backuped path: ");
        print::log(print::IMPORTANT,
                   "    " + strencode::to_console_format(p.u8string()));
        print::log(print::RESET, "  Restore to: ");
        print::log(print::IMPORTANT,
                   "    " +
                       strencode::to_console_format(
                           fs::path(target_folder / p.filename()).u8string()));
    }
    print::cprintln(print::INFO,
                    format("[INFO] Overwrite existing files: {}{}",
                           print::IMPORTANT, overwrite_existing_files));
    print::log(
        print::RESET,
        format("[INFO] Overwrite existing files: {}", overwrite_existing_files),
        false);
    print::pause();

    // 创建目录
    if (!create_directories(input_folder, target_folder, backuped_paths))
        return 1;

    // 复制文件
    if (!copy_files(input_folder, target_folder, backuped_paths,
                   overwrite_existing_files))
        return 1;

    return 0;
}