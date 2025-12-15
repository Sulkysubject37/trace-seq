#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

std::string sha256_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for hashing.");
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    const int bufSize = 32768;
    char* buffer = new char[bufSize];

    while (file.good()) {
        file.read(buffer, bufSize);
        SHA256_Update(&sha256, buffer, file.gcount());
    }

    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    delete[] buffer;
    return ss.str();
}
