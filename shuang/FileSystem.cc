#include "FileSystem.h"

#include <fstream>

namespace filesystem {

std::vector<uint8_t> read(const std::string &filename) { return read(filename, 0); }

std::vector<uint8_t> read(const std::string &filename, const uint32_t count) {
  std::vector<uint8_t> data;

  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  uint64_t read_count = count;
  if (count == 0) {
    file.seekg(0, std::ios::end);
    read_count = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);
  }

  data.resize(static_cast<size_t>(read_count));
  file.read(reinterpret_cast<char *>(data.data()), read_count);
  file.close();

  return data;
}

} // namespace filesystem
