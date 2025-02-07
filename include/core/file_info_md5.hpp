/// @file file_info_md5.hpp
/// @brief 该头文件定义了与文件信息和MD5计算相关的函数。这些函数用于初始化系统、更新缓存的MD5数据以及计算文件的MD5值。
/// 
/// 它包括以下主要功能：
/// - `init()`: 初始化系统，加载必要的配置和缓存的MD5数据（如果可用）。
/// - `update_cached_md5()`: 更新缓存中的当前MD5值。
/// - `calculate_md5_value(FileInfo &file)`: 计算给定文件的MD5值并相应地进行更新。
/// 
/// 该模块依赖于`file_info.hpp`，并且所有函数和类都位于`file_info`命名空间中。

// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once
#ifndef _FILE_INFO_MD5_HPP_
#define _FILE_INFO_MD5_HPP_

#include "file_info.hpp"

namespace fileinfo {
/// @brief 初始化系统，加载必要的配置并加载缓存的MD5数据（如果可用）。
void init();

/// @brief 更新缓存中的当前MD5值。
void update_cached_md5();

/// @brief 计算给定文件的MD5值并相应地进行更新。
/// @param [in,out] file 需要计算其MD5值的FileInfo对象。
void calculate_md5_value(FileInfo &file);
} // namespace fileinfo
#endif