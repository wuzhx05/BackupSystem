/// @file backup/src/main.hpp
/// @brief backup的主函数
//
// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>

#include "env.hpp"
#include "file_info.hpp"
#include "head.hpp"
#include "print.hpp"
#include "str_encode.hpp"
#include "thread_pool.hpp"

using config::THREAD_NUM;
const string PROJECT_NAME = "backup";

vector<u8string> backup_folder_paths;
vector<u8string> directories;
vector<u8string> files;
vector<fileinfo::FileInfo> file_infos;

std::ofstream file_info_output_stream;
std::ofstream directories_output_stream;

using namespace print;

int main(int argc, char *argv[]) {
    // parse command line arguments
    vector<string> _backup_folder_paths;
    if (!parse_command_line_args(argc, argv, THREAD_NUM, _backup_folder_paths))
        return 1;
    for (const auto &path : _backup_folder_paths) {
        backup_folder_paths.emplace_back(strencode::to_u8string(path));
    }

    // initialize
    env::backup_init();
    strencode::init();
    if (!create_backup_folder(directories_output_stream,
                              file_info_output_stream))
        return 1;
    fileinfo::init();
    if (!log(print::WHITE,
             std::format("[INFO] Project started.\n"
                         "[INFO] Called time: {}\n"
                         "[INFO] UUID: {}\n",
                         env::get_current_time("%Y-%m-%d %H:%M:%S"),
                         env::UUID)))
        return 1;
    cprintln(IMPORTANT,
             "[INFO] Encoding: " + strencode::get_console_encoding());
    cprintln(IMPORTANT, format("[INFO] Thread number: {}", THREAD_NUM));

    // get file infos
    search_directories_and_files(backup_folder_paths, directories, files);
    print::pause();
    get_file_infos(files, file_infos);

    // calculate md5
    ThreadPool *pool = nullptr;
    FilesCopier *copier = nullptr;
    calculate_md5_values(pool, copier, file_infos);

    // write to json
    write_to_json(file_info_output_stream, directories_output_stream,
                  directories, file_infos);
    file_info_output_stream.close(), directories_output_stream.close();

    // copy files
    copy_files(copier);

    // check
    check(file_infos);

    //
    fileinfo::update_cached_md5();

    // rename beta
    CLOSE_LOG();
    if (backup_folder_paths.size() <= 5) {
        u8string suf;
        for (const auto &path : backup_folder_paths) {
            suf += u8"_" + fs::path(path).filename().u8string();
        }
        fs::rename(config::PATH_BACKUP_DATA / env::CALLED_TIME,
                   config::PATH_BACKUP_DATA /
                       (strencode::to_u8string(env::CALLED_TIME) + suf));
    }
    return 0;
}