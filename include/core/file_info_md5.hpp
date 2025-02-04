// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once
#ifndef _FILE_INFO_MD5_HPP_
#define _FILE_INFO_MD5_HPP_

#include "file_info.hpp"

namespace file_info {
void init();
void update_cached_md5();
void calculate_md5_value(FileInfo &file);
}
#endif