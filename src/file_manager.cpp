#include "file_manager.h"
#include "utils.h"

#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <iostream>
#include <cstdint>

static const size_t CHUNK = 4096;

std::vector<std::string> list_input_files(const std::string &path) {
    std::vector<std::string> out;
    struct stat st;
    
    if (stat(path.c_str(), &st) != 0) {
        log_error("Failed to stat path '%s': %s", path.c_str(), strerror(errno));
        return out;
    }
    
    if (S_ISDIR(st.st_mode)) {
        // simple recursive BFS
        std::vector<std::string> q;
        q.push_back(path);
        for (size_t qi = 0; qi < q.size(); ++qi) {
            DIR *d = opendir(q[qi].c_str());
            if (!d) {
                log_error("Failed to open directory '%s': %s", q[qi].c_str(), strerror(errno));
                continue;
            }
            
            struct dirent *e;
            errno = 0; // Clear errno before readdir
            while ((e = readdir(d)) != nullptr) {
                if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
                
                std::string child = q[qi] + "/" + e->d_name;
                struct stat st2;
                if (stat(child.c_str(), &st2) != 0) {
                    log_error("Failed to stat '%s': %s", child.c_str(), strerror(errno));
                    continue;
                }
                
                if (S_ISDIR(st2.st_mode)) {
                    q.push_back(child);
                } else if (S_ISREG(st2.st_mode)) {
                    out.push_back(child);
                }
            }
            
            if (errno != 0) {
                log_error("Error reading directory '%s': %s", q[qi].c_str(), strerror(errno));
            }
            
            closedir(d);
        }
    } else if (S_ISREG(st.st_mode)) {
        out.push_back(path);
    } else {
        log_error("Path '%s' is neither a regular file nor a directory", path.c_str());
    }
    
    return out;
}

bool read_entire_file(const std::string &path, std::vector<uint8_t> &out) {
    out.clear();
    
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        log_error("Failed to open file '%s' for reading: %s", path.c_str(), strerror(errno));
        return false;
    }
    
    uint8_t buf[CHUNK];
    ssize_t r;
    while ((r = read(fd, buf, CHUNK)) > 0) {
        out.insert(out.end(), buf, buf + r);
    }
    
    if (r < 0) {
        log_error("Failed to read from file '%s': %s", path.c_str(), strerror(errno));
        close(fd);
        return false;
    }
    
    if (close(fd) != 0) {
        log_error("Failed to close file '%s' after reading: %s", path.c_str(), strerror(errno));
        // Don't fail the operation if close fails, but log it
    }
    
    return true;
}

bool write_entire_file(const std::string &path, const std::vector<uint8_t> &in) {
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        log_error("Failed to open file '%s' for writing: %s", path.c_str(), strerror(errno));
        return false;
    }
    
    ssize_t written = 0;
    size_t total = in.size();
    const uint8_t *ptr = in.data();
    
    while (written < (ssize_t)total) {
        ssize_t w = write(fd, ptr + written, total - written);
        if (w < 0) {
            log_error("Failed to write to file '%s': %s (wrote %zd of %zu bytes)", 
                     path.c_str(), strerror(errno), written, total);
            close(fd);
            return false;
        }
        written += w;
    }
    
    if (close(fd) != 0) {
        log_error("Failed to close file '%s' after writing: %s", path.c_str(), strerror(errno));
        // Don't fail the operation if close fails, but log it
    }
    
    return true;
}
