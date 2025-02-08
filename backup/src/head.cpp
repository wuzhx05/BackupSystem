/// @file backup/src/head.cpp
/// @brief 实现 head.hpp 的功能
//
// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include <exception>
#include <filesystem>
#include <iostream>
#include <queue>
#include <set>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "head.hpp"
#include "print.hpp"
#include "str_encode.hpp"

using nlohmann::json;

/// 尝试创建目录。如果目录不存在且无法创建，则返回false并记录错误信息。
static bool try_create_directory(fs::path name) {
    try {
        if (!std::filesystem::exists(name) ||
            !std::filesystem::is_directory(name))
            std::filesystem::create_directories(name);
    } catch (const std::exception &e) {
        print::log(print::ERROR, (std::string) "[ERROR]" + e.what());
        return false;
    }
    return true;
}

bool create_backup_folder(std::ofstream &directories_output_stream,
                          std::ofstream &file_info_output_stream) {
    using std::format;
    if (!try_create_directory(config::PATH_BACKUP_COPIES))
        return false;
    if (!try_create_directory(config::PATH_BACKUP_DATA / env::CALLED_TIME))
        return false;
    directories_output_stream.open(config::PATH_BACKUP_DATA / env::CALLED_TIME /
                                   "directories.json");
    file_info_output_stream.open(config::PATH_BACKUP_DATA / env::CALLED_TIME /
                                 "file_info.json");

    if (!directories_output_stream.is_open() ||
        !file_info_output_stream.is_open()) {
        print::log(print::ERROR, "[ERROR] Cannot open files.");
        return false;
    }
    return true;
}

bool parse_command_line_args(int argc, char *argv[], int &threads,
                             std::vector<std::string> &folders) {
    namespace po = boost::program_options;

    // 定义命令行选项
    // clang-format off
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display this help message")
        ("threads,j", po::value<int>()->default_value(1), "Number of threads to use")
        ("folders,f", po::value<std::vector<std::string>>(), "Folders to backup")
        ("check-cached-md5,c", "Use cached MD5 information for verification");
    // clang-format on

    // 解析命令行参数
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            return false;
        }

        if (vm.count("folders")) {
            const auto &folder_paths =
                vm["folders"].as<std::vector<std::string>>();
            for (const auto &path : folder_paths) {
                folders.push_back(path);
            }
        }

        if (vm.count("check-cached-md5"))
            config::SHOULD_CHECK_CACHED_MD5 = true;
    } catch (const boost::program_options::required_option &e) {
        print::log(print::ERROR, "[ERROR] " + std::string(e.what()));
        return false;
    } catch (const po::error &e) {
        print::log(print::ERROR, "[ERROR] " + std::string(e.what()));
        return false;
    }

    // 设置变量值
    if (vm.count("threads"))
        threads = vm["threads"].as<int>();

    // 补充备份路径
    using namespace print;
    cprintln(INFO,
             "More source paths (\"" + config::INPUT_END_FLAG + "\" to end):");
    std::string path;
    while (std::getline(std::cin, path) && path != config::INPUT_END_FLAG) {
        if (!path.empty())
            folders.push_back(path);
    }
    if (folders.empty()) {
        cprintln(ERROR, "No source path specified");
        return false;
    }

    return true;
}

void search_directories_and_files(
    const std::vector<u8string> &backup_folder_paths,
    std::vector<u8string> &directories, std::vector<u8string> &files) {
    print::cprintln(print::INFO, "Searching directories and files...");

    // Sets to store discovered directories and files.
    std::unordered_set<fs::path> discovered_directories;
    std::unordered_set<fs::path> discovered_files;

    // Queue for breadth-first search traversal.
    std::queue<fs::path> traversal_queue;

    // Enqueue initial backup folder paths if they exist.
    for (auto path : backup_folder_paths) {
        if (!fs::exists(path)) {
            print::log(print::WARN, format("[WARN] doesn't exist: {}",
                                           strencode::to_console_format(path)));
        } else {
            fs::path canonical_path = fs::canonical(path);
            if (!discovered_directories.contains(canonical_path)) {
                print::log(print::IMPORTANT,
                           "[INFO] Folder path: " +
                               strencode::to_console_format(
                                   canonical_path.u8string()));
                discovered_directories.insert(canonical_path);
                traversal_queue.push(canonical_path);
            }
        }
    }

    // Perform breadth-first search to discover directories and files.
    while (!traversal_queue.empty()) {
        fs::path current_path = traversal_queue.front();
        traversal_queue.pop();

        try {
            for (const auto &entry : fs::directory_iterator(current_path)) {
                if (entry.path() == current_path ||
                    entry.path() == current_path.parent_path())
                    continue;
                if (config::IGNORED_PATH.contains(
                        entry.path().filename().u8string())) {
                    print::log(print::RESET, "[INFO] skipped: " +
                                                 strencode::to_console_format(
                                                     entry.path().u8string()));
                    continue;
                }
                if (fs::is_regular_file(entry))
                    discovered_files.insert(entry.path().u8string());
                if (fs::is_directory(entry) &&
                    !discovered_directories.contains(entry.path())) {
                    discovered_directories.insert(entry.path().u8string());
                    traversal_queue.push(entry.path());
                }
            }
        } catch (const fs::filesystem_error &e) {
            print::log(print::ERROR, std::string("[ERROR] ") + e.what());
        }
    }

    // Transfer discovered directories and files to the output vectors.
    directories.clear();
    files.clear();
    directories.reserve(discovered_directories.size());
    files.reserve(discovered_files.size());
    for (const auto &path : discovered_directories)
        directories.emplace_back(path.u8string());
    for (const auto &path : discovered_files)
        files.emplace_back(path.u8string());

    print::cprintln(print::SUCCESS,
                    format("  Found {} directories and {} files",
                           discovered_directories.size(),
                           discovered_files.size()));
}

void get_file_infos(const std::vector<u8string> &files,
                    std::vector<fileinfo::FileInfo> &file_infos) {
    print::cprintln(print::INFO, "Getting file infos...");
    file_infos.clear();
    file_infos.reserve(files.size());
    for (const auto &file : files)
        file_infos.emplace_back(file);
    print::cprintln(print::SUCCESS,
                    format("  Got {} file infos", files.size()));
}

void calculate_md5_values(ThreadPool *&pool, FilesCopier *&copier,
                          std::vector<fileinfo::FileInfo> &file_infos) {
    using namespace print;
    cprintln(INFO, "Calculating md5 values...");
    pool = new ThreadPool(config::THREAD_NUM);
    copier = new FilesCopier(false);
    unsigned long long total_size = 0;
    for (const auto &file_info : file_infos)
        total_size += file_info.get_file_size();
    print::progress_bar::ProgressBar file_number_progress_bar(
        file_infos.size());
    print::progress_bar::ProgressBar file_size_progress_bar(total_size);
    print::progress_bar::DoubleProgressBar double_progress_bar(
        &file_number_progress_bar, &file_size_progress_bar);

    print::cprintln(print::INFO,
                    std::format("  {}{}{} files, size: {}{:.2f}{} MB.",
                                print::progress_bar::DOUBLE_PROGRESS_COLOR1,
                                file_infos.size(), print::INFO,
                                print::progress_bar::DOUBLE_PROGRESS_COLOR2,
                                total_size / (1024.0 * 1024), print::INFO));
    file_size_progress_bar.show_bar(), file_number_progress_bar.show_bar();

    for (auto &file_info : file_infos) {
        pool->enqueue([&] {
            try {
                calculate_md5_value(file_info);
                file_number_progress_bar.accumulate(1);
                file_size_progress_bar.accumulate(file_info.get_file_size());
                copier->enqueue(file_info.get_path(),
                                (fs::path(config::PATH_BACKUP_COPIES) /
                                 fs::path(file_info.get_md5_value()))
                                    .u8string(),
                                file_info.get_file_size());
            } catch (const std::exception &e) {
                print::log(print::ERROR, std::format("[ERROR]: {}", e.what()));
            }
        });
    }
    delete pool;
    pool = nullptr;
    cprintln(SUCCESS, "\n  Calculating md5 values done.");
}

void write_to_json(std::ofstream &file_info_output_stream,
                   std::ofstream &directories_output_stream,
                   const std::vector<u8string> &directories,
                   const std::vector<fileinfo::FileInfo> &file_infos) {
    print::cprintln(print::INFO, "Writing to json...");
    directories_output_stream
        << json(directories)
               .dump(config::JSON_DUMP_INDENT, config::JSON_DUMP_INDENT_CHAR);
    file_info_output_stream
        << json(file_infos)
               .dump(config::JSON_DUMP_INDENT, config::JSON_DUMP_INDENT_CHAR);
    print::cprintln(print::SUCCESS, "  Writing to json done.");
}

void copy_files(FilesCopier *&copier) {
    print::cprintln(print::INFO, "Copying files...");
    copier->show_progress_bar();
    delete copier;
    copier = nullptr;
    print::cprintln(print::SUCCESS, "\n  Copying files done.");
}

void check(const std::vector<fileinfo::FileInfo> &file_infos) {
    using namespace fs;
    print::cprintln(print::INFO, "Checking...");
    for (auto &file_info : file_infos) {
        auto origin_path = fs::path(file_info.get_path());
        auto backup_path = fs::path(config::PATH_BACKUP_COPIES) /
                           fs::path(file_info.get_md5_value());
        unsigned char ec =
            (!exists(origin_path) << 4) | (!exists(backup_path) << 3) |
            ((file_size(origin_path) != file_size(backup_path)) << 2) |
            ((file_size(origin_path) != file_info.get_file_size()) << 1) |
            (fileinfo::file_time_type2time_t(last_write_time(origin_path)) !=
             file_info.get_modified_time());
        if (ec) {
            print::log(print::ERROR,
                       std::format(
                           "[ERROR] Check: File {} is different from backup,  "
                           "error code: {}",
                           strencode::to_console_format(origin_path.u8string()),
                           ec));
            if (fs::exists(backup_path))
                fs::remove(backup_path);
        }
    }
    print::cprintln(print::SUCCESS, "  Checking done.");
}