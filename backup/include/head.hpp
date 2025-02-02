#pragma once
#ifndef HEAD_HPP
#define HEAD_HPP

#include <fstream>
#include <string>
#include <vector>

#include "ThreadPool.hpp"
#include "config.hpp"
#include "file_info.hpp"
#include "file_info_md5.hpp"
#include "nlohmann/json.hpp"

bool create_backup_folder(std::ofstream &ofs_directories,
                          std::ofstream &ofs_file_info);
bool parseCommandLineArgs(int argc, char *argv[], int &threads,
                          std::vector<std::string> &folders);
void search_directories_and_files(const vector<u8string> &backup_folder_paths,
                                  vector<u8string> &directories,
                                  vector<u8string> &files);
void get_file_infos(const vector<u8string> &files,
                    vector<file_info::FileInfo> &file_infos);
void calculate_md5_values(ThreadPool *&pool, FilesCopier *&copier,
                          vector<file_info::FileInfo> &file_infos,
                          const int THREAD_NUM);
void write_to_json(std::ofstream &ofs_file_info, std::ofstream &ofs_directories,
                   const vector<u8string> &directories,
                   const vector<file_info::FileInfo> &file_infos);
void copy_files(FilesCopier *&copier);
void check(vector<file_info::FileInfo> &file_infos);
#endif