// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once
#ifndef STR_ENCODE_HPP
#define STR_ENCODE_HPP

#include <string>
#include <vector>

// to-do
namespace str_encode {
using std::string;
using std::u8string;
using std::vector;

void init();
string update_console_encoding(const string &str);
string update_console_encoding(const vector <string> &strs);
string to_console_format(const u8string &str);
u8string to_u8string(const string &str);
}

#endif