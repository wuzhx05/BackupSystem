/// @file restore/include/head.hpp
/// @brief 恢复过程的各个模块。
///
/// 按时间顺序声明了一系列函数，构成了restore程序的框架。

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

#include "thread_pool.hpp"
#include "env.hpp"
#include "file_info.hpp"
#include "print.hpp"
#include "str_encode.hpp"
#include "str_similarity.hpp"

/// @brief 解析命令行参数以设置输入和输出文件夹，并指定是否应覆盖现有文件。
///
/// @param argc 命令行参数的数量。
/// @param argv 命令行参数字符串数组。
/// @param input_folder [out] 存储输入文件夹的路径。
/// @param target_folder [out] 存储输出文件夹的路径。
/// @param overwrite_existing_files [out] 是否应覆盖现有文件。
/// @return 如果解析成功则返回 true，否则返回 false。如果请求帮助或发生错误，它将返回 false 并打印相关的消息到 stderr。
bool parse_command_line_args(int argc, char *argv[], std::string &input_folder,
                          std::string &target_folder, bool &overwrite_existing_files);

/// @brief 选择备份数据文件夹，基于用户输入和相似性搜索。
/// 
/// 此函数尝试已有的备份数据路径中找到与给定 `input_folder` 路径匹配的目录。如果找不到完全匹配的目录，它会列出类似的目录并允许用户选择一个。然后选择的目录将被转换为绝对路径。
/// 
/// @param input_folder [in, out] 命令行传递的备份数据文件夹，若没有完全匹配，根据提示再次选择目录。
/// @param target_folder [in, out] 命令行传递的输出文件夹，在此函数中仅作`canonical`处理。
/// 
/// @return 如果成功选择了一个有效的目录，则返回 `true`，否则返回 `false`。
bool select_backup_folder(fs::path &input_folder, fs::path &target_folder);

/// @brief 解析备份日志文件以提取备份的元路径。
/// 
/// 此函数读取位于指定输入文件夹中的日志文件，提取并规范化在日志中提到的路径，并将它们存储在提供的向量中。
/// 
/// @param input_folder [in] 备份数据文件夹的路径，包含日志文件。
/// @param backuped_paths `std::vector<fs::path>` [out]，存储解析后的路径。
/// @return 如果成功找到并解析了已备份的路径，则返回 `true`；否则返回 `false`。
bool parse_backup_log(const fs::path &input_folder,
                    std::vector<fs::path> &backuped_paths);

/// @brief 根据JSON文件中的信息创建目录。
///
/// 该函数从输入文件夹中读取目录路径，这些路径位于“directories.json”文件内，
/// 检查这些路径是否需要备份（即它们包含在任何备份的路径中），并在输出文件夹中按需创建它们。
/// 如果目录已经存在，则记录一条信息性消息。
///
/// @param input_folder 备份数据文件夹的路径，包含目录信息。
/// @param target_folder 从命令行传入，新目录将被创建的基本目录。
/// @param backuped_paths 备份元路径的列表。
/// @return 如果所有目录都成功创建或已经存在，则返回true；否则返回false。
bool create_directories(const fs::path &input_folder,
                       const fs::path &target_folder,
                       const std::vector<fs::path> &backuped_paths);

/// @brief 根据存储在JSON文件中的文件信息，将文件从备份目录复制到输出目录。
/// @param input_folder 备份数据文件夹的路径，包含文件信息。
/// @param target_folder 目标文件夹的路径，文件将被复制到这里。
/// @param backuped_paths 备份元路径的列表。
/// @param overwrite_existing_files `bool`，指示是否覆盖输出目录中存在的文件。
/// @return 如果成功读取JSON文件信息并复制文件，则返回true；否则返回false。
bool copy_files(const fs::path &input_folder, const fs::path &target_folder,
               const std::vector<fs::path> &backuped_paths, 
               bool overwrite_existing_files);

#endif // _RESTORE_HEAD_HPP