#pragma once

#include <string>
#include <vector>

namespace filesystem {

std::vector<uint8_t> read(const std::string &filename);

/**
 * @brief Helper to read an file into a byte-array
 *
 * @param filename The path to the file
 * @param count (optional) How many bytes to read. If 0 or not specified, the
 * size of the file will be used.
 * @return A vector filled with data read from the file
 */
std::vector<uint8_t> read(const std::string &filename, uint32_t count);

} // namespace filesystem
