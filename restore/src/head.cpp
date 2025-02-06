// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include "head.hpp"

namespace po = boost::program_options;

bool parseCommandLineArgs(int argc, char *argv[], std::string &inputFolder,
                          std::string &outputFolder, bool &overwrite) {
    // define command line options
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "Display this help message")(
        "input-folder,i", po::value<std::string>(), "Input folder to process")(
        "output-folder,o", po::value<std::string>(),
        "Output folder for results")("overwrite",
                                     "Overwrite existing files when restoring");

    // parse command line arguments
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);

        // -h
        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            return false;
        }

        // -i
        if (vm.count("input-folder"))
            inputFolder = vm["input-folder"].as<std::string>();
        else {
            std::cerr << "[ERROR] Input folder is required" << std::endl;
            return false;
        }

        // -o
        if (vm.count("output-folder"))
            outputFolder = vm["output-folder"].as<std::string>();
        else {
            std::cerr << "[ERROR] Output folder is required" << std::endl;
            return false;
        }

        // --overwrite
        overwrite = vm.count("overwrite");
    } catch (const boost::program_options::required_option &e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return false;
    } catch (const po::error &e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return false;
    }
    return true;
}

int choose_an_option(int l, int r) {
    int choice;
    string input;
    while (std::getline(std::cin, input)) {
        try {
            choice = std::stoi(input);
            if (choice >= l && choice <= r)
                break;
            else
                throw std::invalid_argument("");
        } catch (const std::invalid_argument &e) {
            print::cprintln(print::ERROR, "Invalid input, please try again: ");
        }
    }
    return choice;
}
bool selectBackupFolder(fs::path &input_folder, fs::path &output_folder) {
    // ambiguious input_folder
    if (!fs::exists(config::PATH_BACKUP_DATA / input_folder)) {
        // search candidates and sort
        vector<std::pair<double, fs::path>> candidates;
        for (auto entry : fs::directory_iterator(config::PATH_BACKUP_DATA)) {
            if (entry.is_directory()) {
                auto u8_input_folder = input_folder.u8string();
                auto u8_candidate = entry.path().filename().u8string();
                candidates.push_back(
                    {str_similarity::levenshteinFullMatrix(
                         string(u8_input_folder.begin(), u8_input_folder.end()),
                         string(u8_candidate.begin(), u8_candidate.end())),
                     entry.path()});
            }
        }
        std::stable_sort(candidates.begin(), candidates.end(),
                         std::greater<>());

        // choose a candidate
        double maxVal = candidates[0].first;
        auto colored_percentage = [&](double s) -> string {
            unsigned char deg =
                maxVal ? static_cast<unsigned char>(255 * s / maxVal) : 0;
            return std::format("\033[38;2;{};{};{}m{:5.1f}%{}", deg, deg, deg,
                               s * 100, print::RESET);
        };
        print::cprintln(print::IMPORTANT, "Please input the backup path: ");
        for (size_t i = 0; i < std::min((size_t)5, candidates.size()); i++) {
            auto &[s, p] = candidates[i];
            auto str = str_encode::to_console_format(p.u8string());
            print::println(
                format("  [{}] {} {}", i + 1, colored_percentage(s), str));
        }
        if (candidates.size() > 5)
            print::println("  [6] More...");
        int opt = choose_an_option(1, std::min(6, (int)candidates.size()));

        // option: more...
        if (opt == 6 && candidates.size() > 5) {
            print::cprintln(print::IMPORTANT, "Please input the backup path: ");
            for (size_t i = 0; i < candidates.size(); i++) {
                auto &[s, p] = candidates[i];
                auto str = str_encode::to_console_format(p.u8string());
                print::println(
                    format("  [{:>{}}] {} {}", i + 1,
                           (int)std::log10((double)candidates.size()) + 1,
                           colored_percentage(s), str));
            }
            opt = choose_an_option(1, (int)candidates.size());
            input_folder = candidates[opt - 1].second;
        } else {
            input_folder = candidates[opt - 1].second;
        }
    }

    // to absolute path
    input_folder = fs::canonical(input_folder);
    output_folder = fs::canonical(output_folder);
    return true;
}

bool parseBackupLog(const fs::path &input_folder,
                    std::vector<fs::path> &backuped_paths) {
    // read the file
    print::cprintln(print::INFO, "[INFO] Parsing backup log...");
    std::ifstream file(input_folder / "log.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file.");
    }

    std::string line;
    const string prefix = "[INFO] Folder path: ";

    // skip other information
    while (std::getline(file, line)) {
        print::println("[LOG]" + line);
        if (line.substr(0, prefix.size()) == prefix) {
            break;
        }
    }

    // parse: "[INFO] Folder path: xxx"
    while (line.substr(0, prefix.size()) == prefix) {
        backuped_paths.push_back(fs::canonical(line.substr(prefix.size())));
        std::getline(file, line);
        if (line.substr(0, prefix.size()) == prefix) {
            print::println("[LOG]" + line);
        } else {
            break;
        }
    }

    if (backuped_paths.empty()) {
        print::cprintln(print::ERROR, "[ERROR] No backuped paths found");
        return false;
    }
    return true;
}

// to-do: test& optimize
bool path_contain(const fs::path &base, const fs::path &child) {
    assert(base.is_absolute());
    assert(child.is_absolute());
    return child.string().starts_with(base.string());
}
bool create_directories(const fs::path &input_folder,
                        const fs::path &output_folder,
                        const std::vector<fs::path> &backuped_paths) {
    print::cprintln(print::INFO, "[INFO] Creating directories...");
    std::ifstream ifs_directory_info(input_folder / "directories.json");
    if (!ifs_directory_info.is_open()) {
        print::cprintln(print::ERROR, "[ERROR] Failed to open the file.");
        return false;
    }
    // parse
    vector<u8string> directory_info;
    nlohmann::json j;
    ifs_directory_info >> j;
    directory_info = j.get<vector<u8string>>();
    ifs_directory_info.close();

    // create
    for (auto str : directory_info) {
        auto p = fs::weakly_canonical(str);
        for (auto bp : backuped_paths) {
            if (path_contain(bp, p)) {
                auto relative_path = fs::relative(p, bp.parent_path());
                auto target_path = output_folder / relative_path;
                if (!fs::exists(target_path)) {
                    fs::create_directories(target_path);
                } else {
                    print::log(print::RESET,
                               "[INFO] Directory already exists: " + p.string(),
                               false);
                }
            }
        }
    }
    print::cprintln(print::SUCCESS, "  Creating directories done.");
    return true;
}

bool copy_files(const fs::path &input_folder, const fs::path &output_folder,
                const std::vector<fs::path> &backuped_paths,
                bool overwrite_existing_files) {
    print::cprintln(print::INFO, "[INFO] Copying files...");
    std::ifstream ifs_file_info(input_folder / "file_info.json");
    if (!ifs_file_info.is_open()) {
        print::cprintln(print::ERROR, "[ERROR] Failed to open the file.");
        return false;
    }
    // parse
    vector<file_info::FileInfo> file_info;
    nlohmann::json j;
    ifs_file_info >> j;
    file_info = j.get<vector<file_info::FileInfo>>();
    ifs_file_info.close();
    // copy
    auto copier = new FilesCopier(overwrite_existing_files);
    for (auto &fi : file_info) {
        if (fi.get_md5_value() == "") {
            print::log(print::ERROR, "[ERROR] FileInfo corrupted: " + nlohmann::json(fi).dump());
            continue;
        }
        if (!fs::exists(config::PATH_BACKUP_COPIES / fi.get_md5_value())) {
            print::log(print::ERROR, "[ERROR] Backup lost: " + nlohmann::json(fi).dump());
            continue;
        }
        for (auto &bp : backuped_paths) {
            if (path_contain(bp, fi.get_path())) {
                auto relative_path =
                    fs::relative(fi.get_path(), bp.parent_path());
                auto target_path = output_folder / relative_path;
                copier->enqueue(config::PATH_BACKUP_COPIES / fi.get_md5_value(),
                                target_path, fi.get_file_size());
            }
        }
    }
    copier->show_progress_bar();
    delete copier;
    print::println("");
    print::cprintln(print::SUCCESS, "  Copying files done.");
    return true;
}