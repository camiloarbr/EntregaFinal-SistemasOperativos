#include "file_manager.h"

#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <iostream>

static const size_t CHUNK = 4096;

std::vector<std::string> list_input_files(const std::string &path) {
    std::vector<std::string> out;
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        // TODO: better error handling/reporting
        return out;
    }
    if (S_ISDIR(st.st_mode)) {
        // simple recursive BFS
        std::vector<std::string> q;
        q.push_back(path);
        for (size_t qi = 0; qi < q.size(); ++qi) {
            DIR *d = opendir(q[qi].c_str());
            if (!d) continue; // TODO: log error
            struct dirent *e;
            while ((e = readdir(d)) != nullptr) {
                if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
                std::string child = q[qi] + "/" + e->d_name;
                struct stat st2;
                if (stat(child.c_str(), &st2) != 0) continue;
                if (S_ISDIR(st2.st_mode)) q.push_back(child);
                else if (S_ISREG(st2.st_mode)) out.push_back(child);
            }
            closedir(d);
        }
    } else if (S_ISREG(st.st_mode)) {
        out.push_back(path);
    }
    return out;
}

bool read_entire_file(const std::string &path, std::vector<uint8_t> &out) {
    out.clear();
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        // TODO: propagate/handle errno
        return false;
    }
    uint8_t buf[CHUNK];
    ssize_t r;
    while ((r = read(fd, buf, CHUNK)) > 0) {
        out.insert(out.end(), buf, buf + r);
    }
    if (r < 0) {
        // TODO: handle read error
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

bool write_entire_file(const std::string &path, const std::vector<uint8_t> &in) {
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        // TODO: handle error
        return false;
    }
    ssize_t written = 0;
    size_t total = in.size();
    const uint8_t *ptr = in.data();
    while (written < (ssize_t)total) {
        ssize_t w = write(fd, ptr + written, total - written);
        if (w < 0) {
            // TODO: handle write error
            close(fd);
            return false;
        }
        written += w;
    }
    close(fd);
    return true;
}
