// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#include "config.hpp"

namespace config {
fs::path PATH_LOGS;
std::set <u8string> IGNORED_PATH{u8"$RECYCLE.BIN", u8"..", u8"."};
int THREAD_NUM;
bool SHOULD_CHECK_CACHED_MD5;
}