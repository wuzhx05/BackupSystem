// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once
#ifndef _THREADPOOL_HPP_
#define _THREADPOOL_HPP_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using std::u8string;
typedef unsigned long long ull;

class ThreadPool {
  public:
    ThreadPool(const int THREAD_NUM);
    ~ThreadPool();

    // template <class F> void enqueue(F f);
    void enqueue(auto f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(f);
        }
        condition.notify_one();
    }

  private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

class FilesCopier {
  public:
    FilesCopier(const bool &overwrite_existing);
    ~FilesCopier();
    void enqueue(u8string from, u8string to, ull file_size);
    void show_progress_bar();

  private:
    struct Task {
        u8string from, to;
        ull file_size;
        Task(const u8string &from, const u8string &to, const ull &file_size)
            : from(from), to(to), file_size(file_size) {}
    };
    std::queue<Task> tasks;
    std::mutex queueMutex;
    std::mutex progressDataMutex;
    std::condition_variable condition;
    ull total_size, finished_size;
    ull total_num, finished_num;
    bool stop;
    bool if_show_progress_bar;
    const bool overwrite_existing;
    std::thread *worker;

    void copy_func(const Task &task);
};

#endif