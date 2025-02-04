// This file is part of BackupSystem - a C++ project.
// 
// Licensed under the MIT License. See LICENSE file in the root directory for details.

#include <fstream>

#include "print.hpp"

namespace print {

std::mutex io_lock;

void println(const std::string &s, std::ostream &os) {
    io_lock.lock();
    os << s << std::endl;
    io_lock.unlock();
}
void cprintln(const char *const c, const std::string &s) {
    io_lock.lock();
    std::cerr << c << s << RESET << std::endl;
    io_lock.unlock();
}
bool log(const char *const col, const std::string &s, bool print_to_console) {
    static std::ofstream ofs(config::PATH_LOGS, std::ios::app);
    if (!ofs) {
        return false;
    }
    if (print_to_console)
        cprintln(col, s);
    io_lock.lock();
    ofs << s << std::endl;
    io_lock.unlock();
    return true;
}

void pause() {
    io_lock.lock();
    std::cerr << "Press ENTER to continue...";
    std::string s;
    std::getline(std::cin, s);
    io_lock.unlock();
}

namespace progress_bar {

void print_double_progress_bar(double ratio1, double ratio2) {
    ratio1 = std::clamp(ratio1, 0.0, 1.0);
    ratio2 = std::clamp(ratio2, 0.0, 1.0);

    size_t mn =
        static_cast<size_t>(std::min(ratio1, ratio2) * BLOCK_NUM + 0.5f);
    size_t mx =
        static_cast<size_t>(std::max(ratio1, ratio2) * BLOCK_NUM + 0.5f);
    io_lock.lock();
    std::cerr << std::format(
        "\r[Processing] {}{:5.1f}%{}, {}{:5.1f}%{} [{}{}{}] \r",
        PROGRESS_COLOR1, ratio1 * 100.0f, RESET, PROGRESS_COLOR2,
        ratio2 * 100.0f, RESET,
        PROGRESS_MIXED_COLOR + std::string(mn, PROGRESS_CHAR),
        (ratio1 > ratio2 ? PROGRESS_COLOR1 : PROGRESS_COLOR2) +
            std::string(mx - mn, PROGRESS_CHAR),
        RESET + std::string(BLOCK_NUM - mx, BAR_CHAR));
    io_lock.unlock();
}
void print_progress_bar(double ratio) {
    ratio = std::clamp(ratio, 0.0, 1.0);

    io_lock.lock();
    size_t blocksToShow = static_cast<size_t>(ratio * BLOCK_NUM + 0.5f);

    std::cerr << std::format("\r[Processing] {:5.1f}% [{}{}] \r",
                             ratio * 100.0f,
                             std::string(blocksToShow, PROGRESS_CHAR),
                             std::string((BLOCK_NUM - blocksToShow), BAR_CHAR));
    io_lock.unlock();
}

DoubleProgressBar::DoubleProgressBar(ProgressBar *bar1, ProgressBar *bar2)
    : progressBar1(bar1), progressBar2(bar2) {
    progressBar1->doubleProgressBar = this;
    progressBar2->doubleProgressBar = this;
}
void DoubleProgressBar::show() {
    print_double_progress_bar(progressBar1->ratio, progressBar2->ratio);
}
void ProgressBar::update(ull progress) {
    current = progress;
    ratio = static_cast<double>(current) / static_cast<double>(total);
    if (!to_show)
        return;
    if (doubleProgressBar != nullptr)
        doubleProgressBar->show();
    else
        show();
}
void ProgressBar::accumulate(ull increment) { update(current + increment); }
void ProgressBar::show_bar() { to_show = true; }
void ProgressBar::hide_bar() { to_show = false; }
void ProgressBar::show() { print_progress_bar(ratio); }

} // namespace progress_bar
} // namespace print