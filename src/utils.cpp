#include "utils.h"

#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_logging() {
    // nothing for now; mutex already initialized
}

void log_info(const char *fmt, ...) {
    pthread_mutex_lock(&log_mutex);
    va_list ap;
    va_start(ap, fmt);
    fprintf(stdout, "[INFO] ");
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    va_end(ap);
    pthread_mutex_unlock(&log_mutex);
}

void log_error(const char *fmt, ...) {
    pthread_mutex_lock(&log_mutex);
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[ERROR] ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    pthread_mutex_unlock(&log_mutex);
}

std::string path_join(const std::string &a, const std::string &b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

std::string basename_from_path(const std::string &path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return path;
    return path.substr(pos+1);
}

std::string dirname_from_path(const std::string &path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return ".";
    if (pos == 0) return "/";
    return path.substr(0, pos);
}

bool create_directory_recursive(const std::string &path) {
    if (path.empty() || path == "." || path == "/") {
        return true;
    }
    
    // Check if directory already exists
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true; // Already exists
        } else {
            return false; // Path exists but is not a directory
        }
    }
    
    // Create parent directory first
    std::string parent = dirname_from_path(path);
    if (parent != path && !create_directory_recursive(parent)) {
        return false;
    }
    
    // Create this directory
    if (mkdir(path.c_str(), 0755) != 0) {
        if (errno != EEXIST) {
            return false;
        }
    }
    
    return true;
}

ssize_t safe_read_loop(int fd, void *buf, size_t count) {
    ssize_t total = 0;
    char *p = static_cast<char*>(buf);
    while ((size_t)total < count) {
        ssize_t r = read(fd, p + total, count - total);
        if (r < 0) return r;
        if (r == 0) break;
        total += r;
    }
    return total;
}

ssize_t safe_write_loop(int fd, const void *buf, size_t count) {
    ssize_t total = 0;
    const char *p = static_cast<const char*>(buf);
    while ((size_t)total < count) {
        ssize_t w = write(fd, p + total, count - total);
        if (w < 0) return w;
        total += w;
    }
    return total;
}
