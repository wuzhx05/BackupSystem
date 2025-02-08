/// @file file_info_md5.cpp
/// @brief file_info_md5.hpp的实现文件
//
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

namespace fileinfo {
constexpr int MD5_DIGEST_BYTE_LEN = 16;
fs::path PATH_MD5_CACHE;
std::unordered_map<ull, std::array<unsigned char, MD5_DIGEST_BYTE_LEN>>
    cached_md5;
std::mutex cached_md5_mutex;

/// @brief Converts a binary digest to a hexadecimal string.
/// @param[in] digest The binary digest array.
/// @param[in] len The length of the digest.
/// @return A hexadecimal encoded string representation of the digest.
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

/// @brief Combines a value into the hash seed.
/// @details This function is used to combine different parts of an object's
/// data (e.g., path, modified time, size) into a single hash value for use in
/// unordered_map hashing.
/// @param[in,out] seed The seed value that will be combined with the hash of
/// the given value.
/// @param[in] value The value to be combined into the hash seed.
template <typename T> inline void hash_combine(size_t &seed, const T &value) {
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/// @brief Hashing function for FileInfo objects.
/// @details This function is used as a hasher in unordered_map for FileInfo
/// objects based on their path, modified time, and size.
/// @param[in] f The FileInfo object to be hashed.
/// @return A hash value calculated from the file's path, modified time, and
/// size(not related to file content).
ull FileInfo_hash(const FileInfo &f) {
    size_t seed = 0;
    hash_combine(seed, f.get_path());
    hash_combine(seed, f.get_modified_time());
    hash_combine(seed, f.get_file_size());
    return seed;
}

void init() {
    PATH_MD5_CACHE = config::PATH_MD5_CACHE / (env::UUID + ".bin");
    if (!fs::exists(config::PATH_MD5_CACHE))
        fs::create_directories(config::PATH_MD5_CACHE);
    std::ifstream ifs(PATH_MD5_CACHE, std::ios::binary);
    if (ifs) {
        ull hash;
        unsigned char md5[MD5_DIGEST_BYTE_LEN];
        while (ifs.read(reinterpret_cast<char *>(&hash), sizeof(hash))) {
            ifs.read(reinterpret_cast<char *>(md5), MD5_DIGEST_BYTE_LEN);
            cached_md5[hash] = std::array<unsigned char, MD5_DIGEST_BYTE_LEN>();
            std::copy(md5, md5 + MD5_DIGEST_BYTE_LEN, cached_md5[hash].begin());
        }
    }
}
void update_cached_md5() {
    std::ofstream cache_ouput_stream(PATH_MD5_CACHE, std::ios::binary);

    if (cache_ouput_stream) {
        for (const auto &[hash, md5] : cached_md5) {
            cache_ouput_stream.write(reinterpret_cast<const char *>(&hash),
                                     sizeof(hash));
            cache_ouput_stream.write(reinterpret_cast<const char *>(md5.data()),
                                     md5.size());
        }
    }
}

void calculate_md5_value(FileInfo &file) {
    unsigned char digest[EVP_MAX_MD_SIZE]; // MD5 输出的缓冲区
    unsigned int digest_length;

    ull hash = FileInfo_hash(file);
    if (config::SHOULD_CHECK_CACHED_MD5) {
        std::lock_guard lock(cached_md5_mutex);

        if (cached_md5.contains(hash)) {
            file.md5_value =
                hex_encode(cached_md5[hash].data(), MD5_DIGEST_BYTE_LEN);
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
        throw std::runtime_error("CalculateMD5: Failed to initialize MD5");
    }

    std::ifstream fs(fs::path(file.path), std::ifstream::binary);
    if (!fs) {
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

    if (EVP_DigestFinal_ex(mdctx, digest, &digest_length) != 1) {
        throw std::runtime_error("CalculateMD5: Failed to finalize MD5");
    }

    // Convert digest to hex string
    assert(digest_length == MD5_DIGEST_BYTE_LEN);
    string calc_res = hex_encode(digest, digest_length);
#ifdef DEBUG_MODE
    {
        std::lock_guard lock(cached_md5_mutex);
        if (config::SHOULD_CHECK_CACHED_MD5 && file.md5_value != "") {
            if (hex_encode(cached_md5[hash].data(), MD5_DIGEST_BYTE_LEN) !=
                calc_res) {
                throw std::runtime_error("CalculateMD5: MD5 value mismatch");
            }
        }
    }
#endif
    file.md5_value = calc_res;
    std::lock_guard lock(cached_md5_mutex);
    cached_md5[hash] = std::array<unsigned char, MD5_DIGEST_BYTE_LEN>();
    std::copy(digest, digest + MD5_DIGEST_BYTE_LEN, cached_md5[hash].begin());
}
} // namespace fileinfo