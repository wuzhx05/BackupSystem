// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once
#ifndef ENV
#define ENV

#include <string>
#include <filesystem>

namespace env {
extern std::string CALLED_TIME;
extern std::string UUID;
namespace fs = std::filesystem;

void backup_init();
void restore_init(fs::path output_folder);
std::string getCurrentTime(const char* format);

}
#endif