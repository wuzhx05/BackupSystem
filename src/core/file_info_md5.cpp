// This file is part of BackupSystem - a C++ project.
//
// Licensed under the MIT License. See LICENSE file in the root directory for
// details.

#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#include <openssl/evp.h>

#include "file_info_md5.hpp"

namespace file_info {
constexpr int MD5_SIZE = 16;
fs::path PATH_MD5_DATA;
std::unordered_map<ull, std::array<unsigned char, MD5_SIZE>> cached_md5;
std::mutex cached_md5_mutex;

string hex_encode(const unsigned char digest[EVP_MAX_MD_SIZE],
                  unsigned int len) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (unsigned int i = 0; i < len; ++i) {
        ss << std::setw(2) << std::setfill('0')
           << (static_cast<unsigned int>(digest[i]) & 0xFF);
    }
    return ss.str();
}

template <typename T> inline void hash_combine(size_t &seed, const T &value) {
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
/**
 * @brief FileInfo的哈希函数
 * @details 用于unordered_map的哈希函数，根据FileInfo的path、modified_time、size
 * 计算哈希值
 */
ull FileInfo_hash(const FileInfo &f) {
    size_t seed = 0;
    hash_combine(seed, f.get_path());
    hash_combine(seed, f.get_modified_time());
    hash_combine(seed, f.get_file_size());
    return seed;
}

void init() {
    PATH_MD5_DATA = config::PATH_MD5_DATA / (env::UUID + ".bin");
    if (!fs::exists(config::PATH_MD5_DATA))
        fs::create_directories(config::PATH_MD5_DATA);
    std::ifstream ifs(PATH_MD5_DATA, std::ios::binary);
    if (ifs) {
        ull hash;
        unsigned char md5[MD5_SIZE];
        while (ifs.read(reinterpret_cast<char *>(&hash), sizeof(hash))) {
            ifs.read(reinterpret_cast<char *>(md5), MD5_SIZE);
            cached_md5[hash] = std::array<unsigned char, MD5_SIZE>();
            std::copy(md5, md5 + MD5_SIZE, cached_md5[hash].begin());
        }
    }
}
void update_cached_md5() {
    std::ofstream ofs(PATH_MD5_DATA, std::ios::binary);

    if (ofs) {
        for (const auto &[hash, md5] : cached_md5) {
            ofs.write(reinterpret_cast<const char *>(&hash), sizeof(hash));
            ofs.write(reinterpret_cast<const char *>(md5.data()),
                      md5.size()); //
        }
    }
}

void calculate_md5_value(FileInfo &file) {
    unsigned char digest[EVP_MAX_MD_SIZE]; // MD5 输出的缓冲区
    unsigned int digestLength;

    ull hash = FileInfo_hash(file);
    if (config::SHOULD_CHECK_CACHED_MD5) {
        std::lock_guard lock(cached_md5_mutex);

        if (cached_md5.contains(hash)) {
            file.md5_value = hex_encode(cached_md5[hash].data(), MD5_SIZE);
#ifdef RELEASE_MODE
            return;
#endif
        }
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        throw std::runtime_error("CalculateMD5: Failed to create MD5 context");
    }

    auto ctx_deleter = [](EVP_MD_CTX *ctx) { EVP_MD_CTX_free(ctx); };
    std::unique_ptr<EVP_MD_CTX, decltype(ctx_deleter)> ctx_guard(mdctx,
                                                                 ctx_deleter);

    if (EVP_DigestInit_ex(mdctx, EVP_md5(), NULL) != 1) {
        // EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("CalculateMD5: Failed to initialize MD5");
    }

    std::ifstream fs(fs::path(file.path), std::ifstream::binary);
    if (!fs) {
        // EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("CalculateMD5: Failed to open file");
    }

    // Read and update hash
    char buffer[READ_FILE_BUFFER_SIZE];
    while (fs.read(buffer, sizeof(buffer))) {
        EVP_DigestUpdate(mdctx, buffer, fs.gcount());
    }
    if (fs.gcount() > 0) {
        EVP_DigestUpdate(mdctx, buffer, fs.gcount());
    }

    if (EVP_DigestFinal_ex(mdctx, digest, &digestLength) != 1) {
        // EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("CalculateMD5: Failed to finalize MD5");
    }

    // EVP_MD_CTX_free(mdctx);

    // Convert digest to hex string
    assert(digestLength == MD5_SIZE);
    string calc_res = hex_encode(digest, digestLength);
#ifdef DEBUG_MODE
    {
        std::lock_guard lock(cached_md5_mutex);
        if (config::SHOULD_CHECK_CACHED_MD5 && file.md5_value != "") {
            if (hex_encode(cached_md5[hash].data(), MD5_SIZE) != calc_res) {
                throw std::runtime_error("CalculateMD5: MD5 value mismatch");
            }
        }
    }
#endif
    file.md5_value = calc_res;
    std::lock_guard lock(cached_md5_mutex);
    cached_md5[hash] = std::array<unsigned char, MD5_SIZE>();
    std::copy(digest, digest + MD5_SIZE, cached_md5[hash].begin());
}
} // namespace file_info