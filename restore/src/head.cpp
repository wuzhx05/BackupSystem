/// @file restore/src/head.cpp
/// @brief head.hpp的实现
///
// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include "head.hpp"

namespace po = boost::program_options;

bool parse_command_line_args(int argc, char *argv[], std::string &input_folder,
                             std::string &target_folder,
                             bool &overwrite_existing_files) {
    // Define command line options
    // clang-format off
    po::options_description options_description("Allowed options");
    options_description.add_options()
        ("help,h", "Display this help message")
        ("input-folder,i", po::value<std::string>(), "Input folder for backup data")
        ("target-folder,t", po::value<std::string>(), "Target folder to store results")
        ("overwrite,o", "Overwrite existing files when restoring");
    // clang-format on

    // Parse command line arguments
    po::variables_map variables_map;
    try {
        po::store(po::command_line_parser(argc, argv)
                      .options(options_description)
                      .run(),
                  variables_map);
        po::notify(variables_map);

        // -h
        if (variables_map.count("help")) {
            std::cerr << options_description << std::endl;
            return false;
        }

        // -i
        if (variables_map.count("input-folder"))
            input_folder = variables_map["input-folder"].as<std::string>();
        else {
            input_folder = "";
        }

        // -t
        if (variables_map.count("target-folder"))
            target_folder = variables_map["target-folder"].as<std::string>();
        else {
            print::cprintln(print::ERROR, "[ERROR] Target folder is required");
            return false;
        }

        // --overwrite
        overwrite_existing_files = variables_map.count("overwrite");
    } catch (const boost::program_options::required_option &e) {
        print::cprintln(print::ERROR, (string) "[ERROR] " + e.what());
        return false;
    } catch (const po::error &e) {
        print::cprintln(print::ERROR, (string) "[ERROR] " + e.what());
        return false;
    }
    return true;
}

/// @brief Prompts the user to select an option within a specified range.
///
/// This function repeatedly reads input from standard input until a valid
/// integer (within the range [l, r]) is provided. It handles invalid input by
/// printing an error message and prompting again.
///
/// @param lower_bound The lower bound of the range (inclusive).
/// @param upper_bound The upper bound of the range (inclusive).
///
/// @return An integer value selected by the user, which is within the specified
/// range.
static int choose_option_in_range(int lower_bound, int upper_bound) {
    int choice;
    std::string input;
    while (std::getline(std::cin, input)) {
        try {
            choice = std::stoi(input);
            if (choice >= lower_bound && choice <= upper_bound)
                break;
            else
                throw std::invalid_argument("");
        } catch (const std::invalid_argument &e) {
            print::cprintln(print::ERROR, "Invalid input, please try again: ");
        }
    }
    return choice;
}

bool select_backup_folder(fs::path &input_folder, fs::path &target_folder) {
    // Ambiguous input_folder
    if (!fs::exists(config::PATH_BACKUP_DATA / input_folder) ||
        input_folder == "." || input_folder == ".." || input_folder.empty()) {
        // Search candidates and sort
        std::vector<std::pair<double, fs::path>> candidates;
        for (const auto &entry :
             fs::directory_iterator(config::PATH_BACKUP_DATA)) {
            if (entry.is_directory()) {
                auto u8_input_folder = input_folder.u8string();
                auto u8_candidate = entry.path().filename().u8string();
                candidates.push_back(
                    {strsimilarity::levenshteinFullMatrix(
                         std::string(u8_input_folder.begin(),
                                     u8_input_folder.end()),
                         std::string(u8_candidate.begin(), u8_candidate.end())),
                     entry.path()});
            }
        }
        if (candidates.size() == 0) {
            print::cprintln(print::ERROR, "[ERROR] No backup data detected!");
            return false;
        }
        std::stable_sort(candidates.begin(), candidates.end(),
                         std::greater<>());

        // Choose a candidate
        double max_similarity_score = candidates[0].first;
        auto colored_percentage = [&](double score) -> std::string {
            unsigned char degree = max_similarity_score
                                       ? static_cast<unsigned char>(
                                             255 * score / max_similarity_score)
                                       : 0;
            return std::format("\033[38;2;{};{};{}m{:5.1f}%{}", degree, degree,
                               degree, score * 100, print::RESET);
        };

        print::cprintln(print::IMPORTANT, "Please input the backup path: ");
        for (size_t i = 0;
             i < std::min(static_cast<size_t>(5), candidates.size()); i++) {
            auto &[similarity, path] = candidates[i];
            auto formatted_path = strencode::to_console_format(path.u8string());
            print::println(format("  [{}] {} {}", i + 1,
                                  colored_percentage(similarity),
                                  formatted_path));
        }

        if (candidates.size() > 5)
            print::println("  [6] More...");

        int option = choose_option_in_range(
            1, std::min(6, static_cast<int>(candidates.size())));

        // Option: More...
        if (option == 6 && candidates.size() > 5) {
            print::cprintln(print::IMPORTANT, "Please input the backup path: ");
            for (size_t i = 0; i < candidates.size(); i++) {
                auto &[similarity, path] = candidates[i];
                auto formatted_path =
                    strencode::to_console_format(path.u8string());
                print::println(
                    format("  [{:>{}}] {} {}", i + 1,
                           static_cast<int>(std::log10(
                               static_cast<double>(candidates.size()))) +
                               1,
                           colored_percentage(similarity), formatted_path));
            }
            option =
                choose_option_in_range(1, static_cast<int>(candidates.size()));
            input_folder = candidates[option - 1].second;
        } else {
            input_folder = candidates[option - 1].second;
        }
    }

    // Convert to absolute path
    input_folder = fs::canonical(input_folder);
    target_folder = fs::canonical(target_folder);
    return true;
}

bool parse_backup_log(const fs::path &input_folder,
                      std::vector<fs::path> &backuped_paths) {
    // Read the file
    print::cprintln(print::INFO, "[INFO] Parsing backup log...");
    std::ifstream file(input_folder / "log.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file.");
    }

    std::string line;
    const std::string log_prefix = "[INFO] Folder path: ";

    // Skip other information
    while (std::getline(file, line)) {
        print::println("[LOG]" + line);
        if (line.substr(0, log_prefix.size()) == log_prefix) {
            break;
        }
    }

    // Parse: "[INFO] Folder path: xxx"
    while (line.substr(0, log_prefix.size()) == log_prefix) {
        backuped_paths.push_back(fs::canonical(
            strencode::to_u8string(line.substr(log_prefix.size()))));
        std::getline(file, line);
        if (line.substr(0, log_prefix.size()) == log_prefix) {
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

// TODO: Test & optimize
bool is_path_contained(const fs::path &base_path, const fs::path &child_path) {
    assert(base_path.is_absolute());
    assert(child_path.is_absolute());
    return child_path.string().starts_with(base_path.string());
}

bool create_directories(const fs::path &input_folder,
                        const fs::path &target_folder,
                        const std::vector<fs::path> &backuped_paths) {
    print::cprintln(print::INFO, "[INFO] Creating directories...");
    std::ifstream directory_info_file(input_folder / "directories.json");
    if (!directory_info_file.is_open()) {
        print::cprintln(print::ERROR, "[ERROR] Failed to open the file.");
        return false;
    }
    // Parse
    std::vector<std::u8string> directory_info;
    nlohmann::json json_data;
    directory_info_file >> json_data;
    directory_info = json_data.get<std::vector<std::u8string>>();
    directory_info_file.close();

    // Create
    for (const auto &dir : directory_info) {
        auto canonical_path = fs::weakly_canonical(dir);
        for (const auto &backup_path : backuped_paths) {
            if (is_path_contained(backup_path, canonical_path)) {
                auto relative_path =
                    fs::relative(canonical_path, backup_path.parent_path());
                auto target_path = target_folder / relative_path;
                if (!fs::exists(target_path)) {
                    fs::create_directories(target_path);
                } else {
                    print::log(print::RESET,
                               "[INFO] Directory already exists: " +
                                   strencode::to_console_format(
                                       canonical_path.u8string()),
                               false);
                }
            }
        }
    }
    print::cprintln(print::SUCCESS, "  Creating directories done.");
    return true;
}

bool copy_files(const fs::path &input_folder, const fs::path &target_folder,
                const std::vector<fs::path> &backuped_paths,
                bool overwrite_existing_files) {
    print::cprintln(print::INFO, "[INFO] Copying files...");
    std::ifstream file_info_file(input_folder / "file_info.json");
    if (!file_info_file.is_open()) {
        print::cprintln(print::ERROR, "[ERROR] Failed to open the file.");
        return false;
    }
    // Parse
    std::vector<fileinfo::FileInfo> file_info;
    nlohmann::json json_data;
    file_info_file >> json_data;
    file_info = json_data.get<std::vector<fileinfo::FileInfo>>();
    file_info_file.close();

    // Copy
    auto file_copier = new FilesCopier(overwrite_existing_files);
    for (const auto &file : file_info) {
        if (file.get_md5_value().empty()) {
            print::log(print::ERROR, "[ERROR] FileInfo corrupted: " +
                                         nlohmann::json(file).dump());
            continue;
        }
        if (!fs::exists(config::PATH_BACKUP_COPIES / file.get_md5_value())) {
            print::log(print::ERROR,
                       "[ERROR] Backup lost: " + nlohmann::json(file).dump());
            continue;
        }
        for (const auto &backup_path : backuped_paths) {
            if (is_path_contained(backup_path, file.get_path())) {
                auto relative_path =
                    fs::relative(file.get_path(), backup_path.parent_path());
                auto target_path = target_folder / relative_path;
                file_copier->enqueue(config::PATH_BACKUP_COPIES /
                                         file.get_md5_value(),
                                     target_path, file.get_file_size());
            }
        }
    }
    file_copier->show_progress_bar();
    delete file_copier;
    print::println("");
    print::cprintln(print::SUCCESS, "  Copying files done.");
    return true;
}