#include "../include/utils.h"

#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

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
