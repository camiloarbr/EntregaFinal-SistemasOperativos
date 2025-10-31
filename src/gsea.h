#pragma once

#include <string>
#include <vector>

// High-level operations
bool compress_path(const std::string &in_path, const std::string &out_path);
bool decompress_path(const std::string &in_path, const std::string &out_path);
bool encrypt_path(const std::string &in_path, const std::string &out_path, const std::string &key);
bool decrypt_path(const std::string &in_path, const std::string &out_path, const std::string &key);

// Low-level file helpers (POSIX)
bool is_directory(const std::string &path);
std::vector<std::string> list_files_recursive(const std::string &dirpath);

// Single-file operations (POSIX read/write)
bool compress_file_rle(const std::string &infile, const std::string &outfile);
bool decompress_file_rle(const std::string &infile, const std::string &outfile);
bool xor_encrypt_file(const std::string &infile, const std::string &outfile, const std::string &key);

