/// @file print.hpp
/// @brief 与打印相关的函数和类声明
/// @details
/// 提供了一系列用于控制台输出的函数和类，包括标准输出、错误输出、日志记录以及进度条显示等功能。
/// @warning 实现中使用了mutex同步打印，尽量通过此文件中的函数打印。

// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#pragma once
#ifndef PRINT_HPP
#define PRINT_HPP

#include <format>
#include <iostream>
#include <mutex>
#include <string>

#include "config.hpp"

/// 关闭日志输出流
#define CLOSE_LOG() print::log(print::RESET, "__CLOSE__", false)
namespace print {

/// @brief 颜色常量定义
const char *const RESET = "\033[0m";   // reset
const char *const BLACK = "\033[30m";  // 30: Black
const char *const RED = "\033[31m";    // 31: Red
const char *const GREEN = "\033[32m";  // 32: Green
const char *const YELLOW = "\033[33m"; // 33: Yellow
const char *const BLUE = "\033[34m";   // 34: Blue
const char *const PURPLE = "\033[35m"; // 35: Purple
const char *const CYAN = "\033[36m";   // 36: Cyan
const char *const WHITE = "\033[37m";  // 37: White

/// @brief 信息颜色常量
const char *const INFO = BLUE;
/// @brief 重要信息颜色常量
const char *const IMPORTANT = PURPLE;
/// @brief 警告颜色常量
const char *const WARN = YELLOW;
/// @brief 错误颜色常量
const char *const ERROR = RED;
/// @brief 成功颜色常量
const char *const SUCCESS = GREEN;

extern std::mutex io_lock;

/// @brief 终端输出打印函数，带换行
/// @param s 要打印的字符串
void println(const std::string &s);

/// @brief 终端打印函数，带颜色，带换行
/// @param col ANSI颜色转义码
/// @param s 要打印的字符串
void cprintln(const char *const col, const std::string &s);

/// @brief 日志记录函数
/// @param col ANSI颜色转义码
/// @param s 要记录的日志内容
/// @param print_to_console 是否同时打印到控制台，默认为true
/// @return 成功返回true，失败（日志输出流打开失败）返回false
bool log(const char *const col, const std::string &s,
         bool print_to_console = true);

/// @brief 暂停程序执行，等待用户输入ENTER
void pause();

namespace progress_bar {
typedef unsigned long long ull;

/// @brief 打印双进度条
/// @param ratio1 第一个进度条已完成的比例（0.0到1.0）
/// @param ratio2 第二个进度条已完成的的比例（0.0到1.0）
void print_double_progress_bar(double ratio1, double ratio2);

/// @brief 打印单个进度条
/// @param ratio 进度条的比例（0.0到1.0）
void print_progress_bar(double ratio);


class ProgressBar;
/**
 * @brief 双进度条类
 */
class DoubleProgressBar {
    friend class ProgressBar;

  public:
    /// @brief 构造函数
    /// @param bar1 第一个进度条对象指针
    /// @param bar2 第二个进度条对象指针
    DoubleProgressBar(ProgressBar *bar1, ProgressBar *bar2);

  private:
    ProgressBar *progressBar1; /// 第一个进度条对象指针
    ProgressBar *progressBar2; /// 第二个进度条对象指针
    void show();               /// `\r`刷新进度条
};

/**
 * @brief 进度条类
 */
class ProgressBar {
    friend class DoubleProgressBar;

  public:
    /// @brief 构造函数
    /// @param total 总进度值
    /// @param to_show 是否初始显示，默认为false
    ProgressBar(ull total, bool to_show = false)
        : total(total), current(0), to_show(to_show), ratio(0),
        double_progress_bar(nullptr) {}

    /// @brief 更新进度条，视情况刷新进度条
    /// @param progress 当前进度值
    /// @details
    /// 若进度条已显示，则刷新进度条。若绑定了双进度条，则刷新双进度条。
    void update(ull progress);

    /// @brief 增加进度，视情况刷新进度条
    /// @param increasement 增加的进度值
    /// @details
    /// 若进度条已显示，则刷新进度条。若绑定了双进度条，则刷新双进度条。
    void accumulate(ull increasement);

    /// @brief 显示进度条
    void show_bar();

    /// @brief 隐藏进度条
    void hide_bar();

  private:
    /// @brief 显示进度条
    void show();

    ull total;                            /// 总进度值
    ull current;                          /// 当前进度值
    bool to_show;                         /// 是否显示进度条
    double ratio;                         /// 进度比例
    DoubleProgressBar *double_progress_bar; /// 双进度条对象指针
};
} // namespace progress_bar
} // namespace print
#endif
