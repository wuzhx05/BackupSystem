/// @file ThreadPool.hpp
/// @brief
/// 头文件，定义ThreadPool和FilesCopier类的接口及其依赖关系，用于管理线程和文件复制任务。

// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#pragma once
#ifndef _THREADPOOL_HPP_
#define _THREADPOOL_HPP_

#include <condition_variable>
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using std::u8string;
namespace fs = std::filesystem;
typedef unsigned long long ull;

/// @brief 一个用于管理一组工作线程的线程池。
class ThreadPool {
  public:
    /// @brief 使用指定的线程数构造ThreadPool。
    /// @param THREAD_NUM 要在池中创建的线程数量。
    ThreadPool(const int THREAD_NUM);

    /// @brief 等待任务完成并销毁ThreadPool。
    ~ThreadPool();

    /// @brief 将新的任务入队，以便由线程池执行。
    /// @param f 要作为任务执行的函数。
    void enqueue(auto f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(f);
        }
        condition.notify_one();
    }

  private:
    std::vector<std::thread> workers;        /// 工作线程集合
    std::queue<std::function<void()>> tasks; /// 任务队列，供程池处理的任务。
    std::mutex queueMutex;             /// 用于同步对任务队列的访问的互斥锁。
    std::condition_variable condition; /// 通知工作线程有新任务可用。
    bool stop;                         /// 指示线程池是否应停止处理新任务。
};

/// @brief 用于管理文件复制任务的类。
class FilesCopier {
  public:
    /// @brief 构造函数。
    /// @param overwrite_existing 复制期间是否覆盖现有文件。
    FilesCopier(const bool &overwrite_existing);

    /// @brief 等待任务完成并销毁FilesCopier。
    ~FilesCopier();

    /// @brief 将新的文件复制任务入队。
    /// @param from 要复制的源路径。
    /// @param to 文件将被复制到的目标路径。
    /// @param file_size 要复制的文件的大小。
    void enqueue(fs::path from, fs::path to, ull file_size);

    /// @brief 显示包含要复制的文件总数及其大小的双进度条。
    void show_progress_bar();

  private:
    /// @brief 描述一个复制任务。
    struct Task {
        fs::path from, to;
        ull file_size;
        Task(const fs::path &from, const fs::path &to, const ull &file_size)
            : from(from), to(to), file_size(file_size) {}
    };
    std::queue<Task> tasks; /// 任务队列。
    std::mutex queueMutex;  /// 用于同步对任务队列的访问的互斥锁。
    std::condition_variable
        condition;             /// 条件变量，用于通知工作线程有新任务可用。
    bool stop;                 /// FilesCopier是否应停止处理新任务。
    bool if_show_progress_bar; /// 是否显示进度条。
    bool overwrite_existing;   /// 在复制期间是否覆盖现有文件。
    ull total_size;            /// 总共要复制的文件大小。
    ull finished_size;         /// 已经复制的文件大小。
    int total_num;             /// 总共要复制的文件数量。
    int finished_num;          /// 已经复制的文件数量。
    std::thread *worker;       /// 用于执行复制任务的工作线程。

    /// @brief 从源路径复制文件到目标路径，并可选地覆盖现有文件。
    /// @param task 包含要复制的路径和文件大小的任务。
    void copy_func(const Task &task);
};

#endif