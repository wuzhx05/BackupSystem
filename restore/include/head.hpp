// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#pragma once
#ifndef _RESTORE_HEAD_HPP
#define _RESTORE_HEAD_HPP

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "ThreadPool.hpp"
#include "env.hpp"
#include "file_info.hpp"
#include "print.hpp"
#include "str_encode.hpp"
#include "str_similarity.hpp"

bool parseCommandLineArgs(int argc, char *argv[], std::string &inputFolder,
                          std::string &outputFolder, bool &overwrite);
bool selectBackupFolder(fs::path &input_folder, fs::path &output_folder);
bool parseBackupLog(const fs::path &input_folder,
                    std::vector<fs::path> &backuped_paths);
bool create_directories(const fs::path &input_folder,
                        const fs::path &output_folder,
                        const std::vector<fs::path> &backuped_paths);
bool copy_files(const fs::path &input_folder, const fs::path &output_folder,
                const std::vector<fs::path> &backuped_paths, 
                bool overwrite_existing_files);
#endif