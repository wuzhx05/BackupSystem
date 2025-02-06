// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#ifdef _WIN32
#include <windows.h>
#else
#include <clocale>
#include <cstdlib>
#include <langinfo.h>
#include <locale>
#endif

#include <boost/locale.hpp>
#include <compact_enc_det/compact_enc_det.h>

#include "str_encode.hpp"

namespace str_encode {
string console_encoding; // initialize to UTF-8
Language DEFAULT_LANG = Language::CHINESE;

void init() {
    console_encoding = "UTF-8";
    auto dect = detectConsoleEncoding();
    if (dect != "unkown")
        console_encoding = dect;
}
string get_console_encoding() { return console_encoding; }
std::string detectConsoleEncoding() {
#ifdef _WIN32
    // Windows 系统使用代码页检测
    UINT codePage = GetConsoleOutputCP();

    switch (codePage) {
    case 65001:
        return "UTF-8";
    case 936:
        return "GBK";
    case 950:
        return "BIG5";
    case 437:
        return "IBM-US";
    case 1252:
        return "Latin-1";
    default:
        return "CP-" + std::to_string(codePage);
    }
#else
    // Unix/Linux/macOS
    const char *lang = std::getenv("LC_ALL");
    if (!lang || !*lang)
        lang = std::getenv("LC_CTYPE");
    if (!lang || !*lang)
        lang = std::getenv("LANG");

    if (lang) {
        std::string slang(lang);
        // 解析类似 zh_CN.UTF-8 的格式
        size_t dotPos = slang.find('.');
        if (dotPos != std::string::npos) {
            std::string encoding = slang.substr(dotPos + 1);
            // 清除后续修饰符（如@euro）
            size_t modifierPos = encoding.find('@');
            if (modifierPos != std::string::npos) {
                encoding = encoding.substr(0, modifierPos);
            }
            return encoding;
        }
    }

    throw std::runtime_error("Unable to detect console encoding");
    // 下下策：使用当前C++环境的locale检测
    std::locale currentLocale("");
    int encValue =
        std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(currentLocale)
            .encoding();
    return std::to_string(encValue);
#endif
}

string detect_encoding(const string &str) {
    bool is_reliable;
    int bytes_consumed;
    Encoding encoding = CompactEncDet::DetectEncoding(
        str.c_str(), str.size(), nullptr, nullptr, nullptr, UNKNOWN_ENCODING,
        DEFAULT_LANG, CompactEncDet::WEB_CORPUS, false, &bytes_consumed,
        &is_reliable);

    string res = "UTF-8";
    if (encoding == CHINESE_GB) {
        res = "GB18030";
    } else if (encoding == UTF8) {
        res = "UTF-8";
    } else if (encoding == GB18030) {
        res = "GB18030";
    } else if (encoding == UTF16BE) {
        res = "UTF-16BE";
    } else if (encoding == UTF16LE) {
        res = "UTF-16LE";
    }
    return res;
}
string to_console_format(const u8string &str) {
    string u8s(str.begin(), str.end());
    return boost::locale::conv::between(u8s, console_encoding, "UTF-8");
}
u8string to_u8string(const string &str) {
    string u8s =
        boost::locale::conv::between(str, "UTF-8", detect_encoding(str));
    return u8string(u8s.begin(), u8s.end());
}

} // namespace str_encode