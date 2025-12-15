#ifndef HASHING_HPP
#define HASHING_HPP

#include <string>

/**
 * @brief Calculates the SHA256 checksum of a given file.
 *
 * This function reads the file at the specified path and computes its
 * SHA256 hash, returning the hash as a hexadecimal string.
 *
 * @param path The path to the file for which to calculate the checksum.
 * @return A string representing the SHA256 checksum in hexadecimal format.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::string sha256_file(const std::string& path);

#endif // HASHING_HPP
