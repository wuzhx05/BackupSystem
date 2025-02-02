#include <boost/locale.hpp>
#include <compact_enc_det/compact_enc_det.h>

#include "str_encode.hpp"

namespace str_encode {
string console_encoding; // initialize to UTF-8
Language DEFAULT_LANG = Language::CHINESE;

void init() { console_encoding = "UTF-8"; }
string update_console_encoding(const string &str) {
    bool is_reliable;
    int bytes_consumed;
    Encoding encoding = CompactEncDet::DetectEncoding(
        str.c_str(), str.size(), nullptr, nullptr, nullptr, UNKNOWN_ENCODING,
        DEFAULT_LANG, CompactEncDet::WEB_CORPUS, false, &bytes_consumed,
        &is_reliable);

    if (encoding == CHINESE_GB) {
        console_encoding = "GB18030";
    } else if (encoding == UTF8) {
        console_encoding = "UTF-8";
    } else if (encoding == GB18030) {
        console_encoding = "GB18030";
    } else if (encoding == UTF16BE) {
        console_encoding = "UTF-16BE";
    } else if (encoding == UTF16LE) {
        console_encoding = "UTF-16LE";
    }
    return console_encoding;
}
string update_console_encoding(const vector<string> &strs) {
    string cat;
    for (const auto &s : strs)
        cat += s;
    return update_console_encoding(cat);
}
string to_console_format(const u8string &str) {
    string u8s(str.begin(), str.end());
    return boost::locale::conv::between(u8s, console_encoding, "UTF-8");
}
u8string to_u8string(const string &str) {
    string u8s =
        boost::locale::conv::between(str, "UTF-8", console_encoding);
    return u8string(u8s.begin(), u8s.end());
}

} // namespace str_encode