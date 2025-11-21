#pragma once

#include <string>
#include <vector>
#include <cstdint>

// List files given an input path (file or directory). For directories, this should
// traverse recursively and return regular files only.
std::vector<std::string> list_input_files(const std::string &path);

// Read/write entire file into memory. Return true on success.
bool read_entire_file(const std::string &path, std::vector<uint8_t> &out);
bool write_entire_file(const std::string &path, const std::vector<uint8_t> &in);
