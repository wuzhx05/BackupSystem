// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#pragma once
#ifndef PRINT_HPP
#define PRINT_HPP

#include <format>
#include <iostream>
#include <mutex>
#include <string>

#include "config.hpp"

#define CLOSE_LOG() print::log(print::RESET, "__CLOSE__", false)

/**
 * @brief 与打印相关的函数
 * @warning 实现中使用了mutex同步打印，尽量通过此文件中的函数打印
 */
namespace print {

const char *const RESET = "\033[0m";   // reset
const char *const BLACK = "\033[30m";  // 30: Black
const char *const RED = "\033[31m";    // 31: Red
const char *const GREEN = "\033[32m";  // 32: Green
const char *const YELLOW = "\033[33m"; // 33: Yellow
const char *const BLUE = "\033[34m";   // 34: Blue
const char *const PURPLE = "\033[35m"; // 35: Purple
const char *const CYAN = "\033[36m";   // 36: Cyan
const char *const WHITE = "\033[37m";  // 37: White

const char *const INFO = BLUE;
const char *const IMPORTANT = PURPLE;
const char *const WARN = YELLOW;
const char *const ERROR = RED;
const char *const SUCCESS = GREEN;

extern std::mutex stdio_lock;

void println(const std::string &s, std::ostream &os = std::cout);
void cprintln(const char *const col, const std::string &s);
// void log(const std::string &s);
bool log(const char *const col, const std::string &s, bool print_to_console = 1);
void pause();

namespace progress_bar {
typedef unsigned long long ull;

void print_double_progress_bar(double ratio1, double ratio2);
void print_progress_bar(double ratio);


class ProgressBar;

/**
 * @brief 双进度条
 * @details 用于显示两个进度条的进度
 */
class DoubleProgressBar {
    friend class ProgressBar;

  public:
    DoubleProgressBar(ProgressBar *bar1, ProgressBar *bar2);

  private:
    ProgressBar *progressBar1, *progressBar2;
    void show();
};

/**
 * @brief 进度条
 */
class ProgressBar {
    friend class DoubleProgressBar;

  public:
    ProgressBar(ull total, bool to_show = false)
        : total(total), current(0), to_show(to_show), ratio(0),
          doubleProgressBar(nullptr) {}

    void update(ull progress);
    void accumulate(ull increasement);
    void show_bar();
    void hide_bar();

  private:
    void show();

    ull total;
    ull current;
    bool to_show;
    double ratio;
    DoubleProgressBar *doubleProgressBar;
};
} // namespace progress_bar
} // namespace print
#endif