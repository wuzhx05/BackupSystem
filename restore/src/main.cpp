// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#define RESTORE

#include "head.hpp"

fs::path input_folder;
fs::path output_folder;
vector<fs::path> backuped_paths;
bool overwrite_existing_files = false;

int main(int argc, char *argv[]) {
    // parse command line arguments
    {
        std::string str_input_folder, str_output_folder;
        if (!parseCommandLineArgs(argc, argv, str_input_folder,
                                  str_output_folder, overwrite_existing_files))
            return 1;
        input_folder = fs::path(strencode::to_u8string(str_input_folder));
        output_folder = fs::path(strencode::to_u8string(str_output_folder));
    }

    // initialize
    strencode::init();
    print::cprintln(print::IMPORTANT,
                    "[INFO] Encoding: " + strencode::get_console_encoding());
    if (!fs::exists(output_folder)) {
        fs::create_directories(output_folder);
    }
    env::restore_init(output_folder);
    if (!print::log(print::RESET, "[INFO] Restore started.")) {
        print::cprintln(print::ERROR, "[ERROR] Failed to open the log file.");
        return 1;
    }

    // select a backup
    if (!selectBackupFolder(input_folder, output_folder))
        return 1;

    // parse backup log
    if (!parseBackupLog(input_folder, backuped_paths))
        return 1;

    // confirm
    for (auto &p : backuped_paths) {
        print::log(print::RESET, "[INFO] Backuped path: ");
        print::log(print::IMPORTANT,
                   "    " + strencode::to_console_format(p.u8string()));
        print::log(print::RESET, "  Restore to: ");
        print::log(print::IMPORTANT,
                   "    " +
                       strencode::to_console_format(
                           fs::path(output_folder / p.filename()).u8string()));
    }
    print::cprintln(print::INFO,
                    format("[INFO] Overwrite existing files: {}{}",
                           print::IMPORTANT, overwrite_existing_files));
    print::log(
        print::RESET,
        format("[INFO] Overwrite existing files: {}", overwrite_existing_files),
        false);
    print::pause();

    // create directories
    if (!create_directories(input_folder, output_folder, backuped_paths))
        return 1;
    
    // copy files
    if (!copy_files(input_folder, output_folder, backuped_paths, overwrite_existing_files))
        return 1;
    return 0;
}