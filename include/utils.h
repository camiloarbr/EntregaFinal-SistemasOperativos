#pragma once

#include <string>
#include <cstdarg>
#include <vector>

// Basic path utilities
std::string path_join(const std::string &a, const std::string &b);
std::string basename_from_path(const std::string &path);
std::string dirname_from_path(const std::string &path);
bool create_directory_recursive(const std::string &path);

// Safe read/write loops
ssize_t safe_read_loop(int fd, void *buf, size_t count);
ssize_t safe_write_loop(int fd, const void *buf, size_t count);

// Simple logging (thread-safe)
void init_logging();
void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
