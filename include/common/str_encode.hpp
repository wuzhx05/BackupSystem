/// @file include/common/str_encode.hpp
/// @brief 字符串编码处理模块。
/// 该文件包含用于检测和转换字符串编码的函数实现。
/// 支持Windows和非Windows系统的编码检测和转换。
/// 使用Boost库和CompactEncDet库进行编码检测。

// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#pragma once
#ifndef STR_ENCODE_HPP
#define STR_ENCODE_HPP

#include <string>
#include <vector>

/// @namespace strencode
/// 包含字符串编码处理相关的函数和变量。
namespace strencode {
using std::string;
using std::u8string;
using std::vector;

/// @brief 初始化函数，用于初始化控制台编码和检测编码。
void init();

/// @brief 检测控制台的编码格式。
/// @return 返回检测到的编码格式字符串。
std::string detectConsoleEncoding();

/// @brief 获取当前控制台的编码格式。
/// @return 返回当前控制台的编码格式字符串。
string get_console_encoding();

/// @brief 检测输入字符串的编码格式。
/// @param str 需要检测编码的字符串。
/// @return 返回检测到的编码格式字符串，如"UTF-8"。
string detect_encoding(const string &str);

/// @brief 将输入的UTF-8字符串转换为控制台编码格式。
/// @param str 需要转换的字符串。
/// @return 返回转换后的字符串。
string to_console_format(const u8string &str);

/// @brief 将输入的字符串转换为UTF-8编码的u8string。
/// @param str 需要转换的字符串。
/// @return 返回转换后的u8string。
u8string to_u8string(const string &str);
} // namespace strencode

#endif