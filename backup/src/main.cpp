// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>

#include "ThreadPool.hpp"
#include "env.hpp"
#include "file_info.hpp"
#include "head.hpp"
#include "print.hpp"
#include "str_encode.hpp"

using config::THREAD_NUM;
const string PROJECT_NAME = "backup";

vector<u8string> backup_folder_paths;
vector<u8string> directories;
vector<u8string> files;
vector<file_info::FileInfo> file_infos;

std::ofstream ofs_file_info;
std::ofstream ofs_directories;

using namespace print;

int main(int argc, char *argv[]) {
    // initialize
    env::backup_init();
    str_encode::init();
    if (!create_backup_folder(ofs_directories, ofs_file_info))
        return 1;
    file_info::init();
    cprintln(IMPORTANT,
             "[INFO] Encoding: " + str_encode::get_console_encoding());
    if (!log(print::WHITE,
             std::format("[INFO] Project started.\n"
                         "[INFO] Called time: {}\n"
                         "[INFO] UUID: {}\n",
                         env::getCurrentTime("%Y-%m-%d %H:%M:%S"), env::UUID)))
        return 1;

    // parse command line arguments
    vector<string> _backup_folder_paths;
    if (!parseCommandLineArgs(argc, argv, THREAD_NUM, _backup_folder_paths))
        return 1;
    for (const auto &path : _backup_folder_paths) {
        backup_folder_paths.emplace_back(str_encode::to_u8string(path));
    }
    cprintln(IMPORTANT, format("[INFO] Thread number: {}", THREAD_NUM));
    search_directories_and_files(backup_folder_paths, directories, files);
    print::pause();

    // get file infos
    get_file_infos(files, file_infos);

    // calculate md5
    ThreadPool *pool = nullptr;
    FilesCopier *copier = nullptr;
    calculate_md5_values(pool, copier, file_infos, THREAD_NUM);

    // write to json
    write_to_json(ofs_file_info, ofs_directories, directories, file_infos);
    ofs_file_info.close(), ofs_directories.close();

    // copy files
    copy_files(copier);

    // check
    check(file_infos);

    //
    file_info::update_cached_md5();

    // rename beta
    CLOSE_LOG();
    if (backup_folder_paths.size() <= 5) {
        u8string suf;
        for (const auto &path : backup_folder_paths) {
            suf += u8"_" + fs::path(path).filename().u8string();
        }
        fs::rename(config::PATH_BACKUP_DATA / env::CALLED_TIME,
                   config::PATH_BACKUP_DATA /
                       (str_encode::to_u8string(env::CALLED_TIME) + suf));
    }
    return 0;
}