/// @file backup/include/head.hpp
/// @brief 备份过程的各个模块。
///
/// 按调用的时间顺序声明了一系列函数，构成了backup程序的框架。
//
// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#pragma once
#ifndef _BACKUP_HEAD_HPP
#define _BACKUP_HEAD_HPP

#include <fstream>
#include <string>
#include <vector>

#include "config.hpp"
#include "file_info.hpp"
#include "file_info_md5.hpp"
#include "nlohmann/json.hpp"
#include "thread_pool.hpp"

/// @brief 创建备份文件夹并打开相关文件流。
///
/// 该函数尝试创建备份目录，并在成功后打开用于记录目录和文件信息的JSON文件。
/// 如果目录创建失败或文件流无法打开，则输出错误日志并返回false。
///
/// @param directories_output_stream [out] 用于写入目录信息的文件流。
/// @param file_info_output_stream [out] 用于写入文件信息的文件流。
///
/// @return true 如果备份文件夹创建成功并且文件流打开成功，否则返回false。
bool create_backup_folder(std::ofstream &directories_output_stream,
                        std::ofstream &file_info_output_stream);

/// @brief 解析命令行参数，设置线程数和备份文件夹路径。
/// @param argc 命令行参数的数量。
/// @param argv 命令行参数的字符串数组。
/// @param threads [out] 用于存储解析后的线程数。
/// @param folders [out] 用于存储解析后的备份文件夹路径列表。
/// @return 如果解析成功，返回true；否则返回false。
bool parse_command_line_args(int argc, char *argv[], int &threads,
                          std::vector<std::string> &folders);

/// @brief 使用BFS遍历搜索指定的备份文件夹路径，收集其中的所有目录和文件。
///
/// 该函数通过广度优先搜索（BFS）遍历给定的目录路径，收集找到的所有目录和文件。对于传入的路径，合并重复的情况，处理目录不存在的情况，并相应地记录警告或错误信息。
///
/// @param backup_folder_paths [in] 要搜索的文件路径列表。
/// @param directories [out] 存储找到的目录。
/// @param files [out] 存储找到的文件。
void search_directories_and_files(const std::vector<u8string> &backup_folder_paths,
                               std::vector<u8string> &directories,
                               std::vector<u8string> &files);

/// @brief 获取每个文件的信息（路径，修改时间，大小）并将其存储在 `file_infos`
/// 中。
///
/// 该函数遍历文件路径列表，使用 `FileInfo`
/// 构造函数检索其信息（路径，修改时间，大小），然后将此信息附加到 `file_infos`
/// 中。执行过程中打印状态消息。
///
/// @param files [in] 表示文件路径。
/// @param file_infos [out] 对 `fileinfo::FileInfo`
/// 对象的向量的引用，这些对象的信息将被存储。
void get_file_infos(const std::vector<u8string> &files,
                  std::vector<fileinfo::FileInfo> &file_infos);

/// @brief 计算文件的MD5值，并将它们复制到备份目录。
///
/// 该函数初始化线程池和文件复制器，然后为每个文件生成一个任务来计算其MD5值，并在完成后将其复制到备份目录。
///
/// @param pool [in, out] 指向ThreadPool对象的指针，用于管理并行计算MD5。
/// @param copier [in, out] 指向FilesCopier对象的指针，用于文件复制操作。
/// @param file_infos [in, out] 传入需要计算MD5值的文件信息，保存对应文件的MD5。
void calculate_md5_values(ThreadPool *&pool, FilesCopier *&copier,
                        std::vector<fileinfo::FileInfo> &file_infos);

/// @brief 将目录路径和文件信息写入JSON文件。
///
/// 此函数将目录列表和文件信息写入各自的目标流中，以JSON格式呈现。
/// 使用`nlohmann/json`库进行序列化。JSON数据采用`config`中指定的缩进级别和字符进行格式化。
///
/// @param file_info_output_stream [in] 用于将文件信息写入JSON文件的输出流。
/// @param directories_output_stream [in] 用于将目录路径写入JSON文件的输出流。
/// @param directories [in] 目录路径。
/// @param file_infos [in] 文件信息。
void write_to_json(std::ofstream &file_info_output_stream,
                 std::ofstream &directories_output_stream,
                 const std::vector<u8string> &directories,
                 const std::vector<fileinfo::FileInfo> &file_infos);

/// @brief 展示进度条，等待复制文件完成。
///
/// @param copier FilesCopier 对象的指针。函数将管理此对象的生命周期。
///                     执行后，`copier` 将被设置为 nullptr。
void copy_files(FilesCopier *&copier);

/// @brief 检查文件的完整性，通过比较其元数据与备份进行。
///
/// 该函数会遍历一组文件信息，并检查每个文件的元数据是否与其备份副本匹配。如果发现差异，它会记录这些差异并在必要时删除损坏的备份。
///
/// @param file_infos 对包含 `fileinfo::FileInfo`
/// 对象的向量的引用，这些对象持有待检查文件的路径和其他元数据。
void check(const std::vector<fileinfo::FileInfo> &file_infos);
#endif