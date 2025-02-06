// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#include "ThreadPool.hpp"

#include <filesystem>
#include <iostream>

#include "print.hpp"
#include "str_encode.hpp"

ThreadPool::ThreadPool(const int THREAD_NUM) : stop(false) {
    for (int i = 0; i < THREAD_NUM; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock,
                                   [this] { return stop || !tasks.empty(); });

                    if (stop && tasks.empty()) {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    if (stop)
        return;

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();

    for (std::thread &worker : workers) {
        worker.join();
    }
}

FilesCopier::FilesCopier(const bool &overwrite_existing)
    : total_size(0), finished_size(0), total_num(0), finished_num(0),
      stop(false), if_show_progress_bar(false),
      overwrite_existing(overwrite_existing), worker(nullptr) {
    worker = new std::thread([this] {
        while (true) {
            Task *task = nullptr;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] { return stop || !tasks.empty(); });
                if (stop && tasks.empty()) {
                    return;
                }

                task = new Task(tasks.front());
                tasks.pop();
            }
            if (task != nullptr) {
                copy_func(*task);
                delete task;
            }
        }
    });
}
FilesCopier::~FilesCopier() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();

    worker->join();
}
void FilesCopier::enqueue(fs::path from, fs::path to, ull file_size) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(from, to, file_size);
        total_num++, total_size += file_size;
    }
    condition.notify_one();
}
void FilesCopier::copy_func(const Task &task) {
    try {
        if (fs::exists(task.to) && overwrite_existing)
            fs::remove(task.to);
        if (!fs::exists(task.to))
            fs::copy_file(task.from, task.to);
        finished_num++, finished_size += task.file_size;
        if (if_show_progress_bar)
            print::progress_bar::print_double_progress_bar(
                static_cast<double>(finished_num) / total_num,
                static_cast<double>(finished_size) / total_size);
        // if (if_print_rate) print_rate();
    } catch (const std::exception &e) {
        print::log(print::ERROR,
                        std::format("[ERROR] FilesCopier: {} to {}, : {}.",
                                    str_encode::to_console_format(task.from.u8string()),
                                    str_encode::to_console_format(task.to.u8string()),
                                    e.what()));
    }
}
void FilesCopier::show_progress_bar() {
    print::cprintln(print::INFO,
                    std::format("  Copying {}{}{} files, size: {}{:.2f}{} MB.",
                                print::progress_bar::PROGRESS_COLOR1, total_num,
                                print::INFO,
                                print::progress_bar::PROGRESS_COLOR2,
                                total_size / (1024.0 * 1024), print::INFO));
    if_show_progress_bar = true;
}