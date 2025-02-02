#include <boost/algorithm/hex.hpp>
#include <openssl/evp.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <string>

#include "file_info_md5.hpp"

namespace file_info {
void calculate_md5_value(FileInfo &file) {
    unsigned char digest[EVP_MAX_MD_SIZE]; // MD5 输出的缓冲区
    unsigned int digestLength;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        throw std::runtime_error("CalculateMD5: Failed to create MD5 context");
    }

    if (EVP_DigestInit_ex(mdctx, EVP_md5(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("CalculateMD5: Failed to initialize MD5");
    }

    std::ifstream fs(fs::path(file.path), std::ifstream::binary);
    if (!fs) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("CalculateMD5: Failed to open file");
    }

    // Read and update hash
    char buffer[1 << 15];
    while (fs.read(buffer, sizeof(buffer))) {
        EVP_DigestUpdate(mdctx, buffer, fs.gcount());
    }
    // Update for remaining bytes
    EVP_DigestUpdate(mdctx, buffer, fs.gcount());

    if (EVP_DigestFinal_ex(mdctx, digest, &digestLength) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("CalculateMD5: Failed to finalize MD5");
    }

    EVP_MD_CTX_free(mdctx);

    // Convert digest to hex string
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (unsigned int i = 0; i < digestLength; ++i) {
        ss << static_cast<int>(digest[i]);
    }
    file.md5_value = ss.str();
}
}