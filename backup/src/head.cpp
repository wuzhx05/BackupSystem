// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include <exception>
#include <filesystem>
#include <iostream>
#include <queue>
#include <set>

#include <boost/program_options.hpp>

#include "head.hpp"
#include "print.hpp"
#include "str_encode.hpp"

using nlohmann::json;

static bool _try_create_directory(fs::path name) {
    try {
        if (!std::filesystem::exists(name) ||
            !std::filesystem::is_directory(name))
            std::filesystem::create_directories(name);
    } catch (const std::exception &e) {
        print::log(print::ERROR, (string) "[ERROR]" + e.what());
        return false;
    }
    return true;
}
bool create_backup_folder(std::ofstream &ofs_directories,
                          std::ofstream &ofs_file_info) {
    using std::format;
    if (!_try_create_directory(config::PATH_BACKUP_COPIES))
        return false;
    if (!_try_create_directory(config::PATH_BACKUP_DATA / env::CALLED_TIME))
        return false;
    ofs_directories.open(config::PATH_BACKUP_DATA / env::CALLED_TIME /
                         "directories.json");
    ofs_file_info.open(config::PATH_BACKUP_DATA / env::CALLED_TIME /
                       "file_info.json");

    if (!ofs_directories.is_open() || !ofs_file_info.is_open()) {
        print::log(print::ERROR, "[ERROR] Cannot open files.");
        return false;
    }
    return true;
}
/**
 * @brief 解析命令行参数
 */
bool parseCommandLineArgs(int argc, char *argv[], int &threads,
                          std::vector<std::string> &folders) {
    namespace po = boost::program_options;

    // 定义命令行选项
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "Display this help message")
    ("threads,j", po::value<int>()->default_value(1), "Number of threads to use")
    ("folders,f", po::value<std::vector<std::string>>(), "Folders to backup")
    ("check-cached-md5,c", "Use cached MD5 information for verification");

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
            const auto &folderPaths =
                vm["folders"].as<std::vector<std::string>>();
            for (const auto &path : folderPaths) {
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
    string path;
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

void search_directories_and_files(const vector<u8string> &backup_folder_paths,
                                  vector<u8string> &directories,
                                  vector<u8string> &files) {
    print::cprintln(print::INFO, "Searching directories and files...");
    std::set<std::u8string> d, f;

    // BFS
    std::queue<fs::path> q;
    for (const auto &path : backup_folder_paths) {
        if (!fs::exists(path)) {
            print::log(print::WARN,
                       format("[WARN] doesn't exist: {}",
                              str_encode::to_console_format(path)));
        } else {
            std::u8string abs_path = fs::absolute(fs::path(path)).u8string();
            if (!d.contains(abs_path)) {
                print::log(print::IMPORTANT,
                           "[INFO] Folder path: " +
                               str_encode::to_console_format(abs_path));
                d.insert(abs_path);
                q.push(abs_path);
            }
        }
    }
    while (!q.empty()) {
        auto path = q.front();
        q.pop();
        try {
            for (const auto &entry : fs::directory_iterator(path)) {
                if (entry.path() == path || entry.path() == path.parent_path())
                    continue;
                if (config::IGNORED_PATH.contains(
                        entry.path().filename().u8string())) {
                    print::log(print::RESET, "[INFO] skipped: " +
                                                 str_encode::to_console_format(
                                                     entry.path().u8string()));
                    continue;
                }
                if (fs::is_regular_file(entry))
                    f.insert(entry.path().u8string());
                if (fs::is_directory(entry) &&
                    !d.contains(entry.path().u8string()))
                    d.insert(entry.path().u8string()), q.push(entry.path());
            }
        } catch (const fs::filesystem_error &e) {
            print::log(print::ERROR, (string) "[ERROR] " + e.what());
        }
    }

    directories.clear();
    files.clear();
    directories.reserve(d.size());
    files.reserve(f.size());
    for (const auto &path : d)
        directories.emplace_back(path);
    for (const auto &path : f)
        files.emplace_back(path);
    print::cprintln(
        print::SUCCESS,
        format("  Found {} directories and {} files", d.size(), f.size()));
}

void get_file_infos(const vector<u8string> &files,
                    vector<file_info::FileInfo> &file_infos) {
    print::cprintln(print::INFO, "Getting file infos...");
    file_infos.clear();
    file_infos.reserve(files.size());
    for (const auto &file : files)
        file_infos.emplace_back(file);
    print::cprintln(print::SUCCESS,
                    format("  Got {} file infos", files.size()));
}

void calculate_md5_values(ThreadPool *&pool, FilesCopier *&copier,
                          vector<file_info::FileInfo> &file_infos,
                          const int THREAD_NUM) {
    using namespace print;
    cprintln(INFO, "Calculating md5 values...");
    pool = new ThreadPool(THREAD_NUM);
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
                                print::progress_bar::PROGRESS_COLOR1,
                                file_infos.size(), print::INFO,
                                print::progress_bar::PROGRESS_COLOR2,
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

void write_to_json(std::ofstream &ofs_file_info, std::ofstream &ofs_directories,
                   const vector<u8string> &directories,
                   const vector<file_info::FileInfo> &file_infos) {
    print::cprintln(print::INFO, "Writing to json...");
    ofs_directories << json(directories)
                           .dump(config::JSON_DUMP_INDENT,
                                 config::JSON_DUMP_INDENT_CHAR);
    ofs_file_info << json(file_infos)
                         .dump(config::JSON_DUMP_INDENT,
                               config::JSON_DUMP_INDENT_CHAR);
    print::cprintln(print::SUCCESS, "  Writing to json done.");
}

void copy_files(FilesCopier *&copier) {
    print::cprintln(print::INFO, "Copying files...");
    copier->show_progress_bar();
    delete copier;
    copier = nullptr;
    print::cprintln(print::SUCCESS, "\n  Copying files done.");
}

void check(vector<file_info::FileInfo> &file_infos) {
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
            (file_info::file_time_type2time_t(last_write_time(origin_path)) !=
             file_info.get_modified_time());
        if (ec) {
            print::log(
                print::ERROR,
                std::format(
                    "[ERROR] Check: File {} is different from backup, error "
                    "code: {}",
                    str_encode::to_console_format(origin_path.u8string()), ec));
            if (fs::exists(backup_path))
                fs::remove(backup_path);
        }
    }
    print::cprintln(print::SUCCESS, "  Checking done.");
}