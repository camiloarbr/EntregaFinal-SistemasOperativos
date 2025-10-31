#include "gsea.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include <string.h>
#include <iostream>
#include <vector>
#include <queue>
#include <thread>

static const size_t BUFSZ = 4096;

bool is_directory(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

std::vector<std::string> list_files_recursive(const std::string &dirpath) {
    std::vector<std::string> out;
    std::queue<std::string> q;
    q.push(dirpath);

    while (!q.empty()) {
        std::string cur = q.front(); q.pop();
        DIR *d = opendir(cur.c_str());
        if (!d) continue;
        struct dirent *ent;
        while ((ent = readdir(d)) != nullptr) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            std::string child = cur + "/" + ent->d_name;
            struct stat st;
            if (stat(child.c_str(), &st) != 0) continue;
            if (S_ISDIR(st.st_mode)) {
                q.push(child);
            } else if (S_ISREG(st.st_mode)) {
                out.push_back(child);
            }
        }
        closedir(d);
    }
    return out;
}

// Simple RLE compression: encode as (count, byte) pairs. count is 1 byte; runs >255 split.
bool compress_file_rle(const std::string &infile, const std::string &outfile) {
    int infd = open(infile.c_str(), O_RDONLY);
    if (infd < 0) {
        perror("open in");
        return false;
    }
    int outfd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outfd < 0) {
        perror("open out");
        close(infd);
        return false;
    }

    unsigned char buf[BUFSZ];
    ssize_t n;
    unsigned char prev = 0;
    bool has_prev = false;
    unsigned int run = 0;

    while ((n = read(infd, buf, BUFSZ)) > 0) {
        for (ssize_t i = 0; i < n; ++i) {
            unsigned char b = buf[i];
            if (!has_prev) {
                prev = b; has_prev = true; run = 1;
            } else if (b == prev && run < 255) {
                ++run;
            } else {
                unsigned char outbuf[2] = { (unsigned char)run, prev };
                if (write(outfd, outbuf, 2) != 2) { perror("write"); close(infd); close(outfd); return false; }
                prev = b; run = 1;
            }
        }
    }
    if (n < 0) { perror("read"); close(infd); close(outfd); return false; }
    if (has_prev) {
        unsigned char outbuf[2] = { (unsigned char)run, prev };
        if (write(outfd, outbuf, 2) != 2) { perror("write"); close(infd); close(outfd); return false; }
    }

    close(infd);
    close(outfd);
    return true;
}

bool decompress_file_rle(const std::string &infile, const std::string &outfile) {
    int infd = open(infile.c_str(), O_RDONLY);
    if (infd < 0) { perror("open in"); return false; }
    int outfd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outfd < 0) { perror("open out"); close(infd); return false; }

    unsigned char hdr[2];
    ssize_t n;
    while ((n = read(infd, hdr, 2)) == 2) {
        unsigned char cnt = hdr[0];
        unsigned char val = hdr[1];
        // write cnt copies
        std::vector<unsigned char> block(cnt, val);
        if (write(outfd, block.data(), cnt) != cnt) { perror("write"); close(infd); close(outfd); return false; }
    }
    if (n < 0) { perror("read"); close(infd); close(outfd); return false; }

    close(infd);
    close(outfd);
    return true;
}

bool xor_encrypt_file(const std::string &infile, const std::string &outfile, const std::string &key) {
    if (key.empty()) {
        std::cerr << "empty key\n";
        return false;
    }
    int infd = open(infile.c_str(), O_RDONLY);
    if (infd < 0) { perror("open in"); return false; }
    int outfd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outfd < 0) { perror("open out"); close(infd); return false; }

    unsigned char buf[BUFSZ];
    ssize_t n;
    size_t ki = 0;
    while ((n = read(infd, buf, BUFSZ)) > 0) {
        for (ssize_t i = 0; i < n; ++i) {
            buf[i] ^= key[ki % key.size()];
            ++ki;
        }
        if (write(outfd, buf, n) != n) { perror("write"); close(infd); close(outfd); return false; }
    }
    if (n < 0) { perror("read"); close(infd); close(outfd); return false; }

    close(infd);
    close(outfd);
    return true;
}

// wrapper helpers for paths

static void process_file_thread_compress(const std::string &infile, const std::string &outfile) {
    if (!compress_file_rle(infile, outfile)) {
        std::cerr << "Failed compress: " << infile << " -> " << outfile << "\n";
    }
}

bool compress_path(const std::string &in_path, const std::string &out_path) {
    if (is_directory(in_path)) {
        auto files = list_files_recursive(in_path);
        std::vector<std::thread> ths;
        for (const auto &f : files) {
            // For simplicity, create output filename by appending .rle
            std::string out = f + ".rle";
            ths.emplace_back(process_file_thread_compress, f, out);
        }
        for (auto &t : ths) t.join();
        return true;
    } else {
        return compress_file_rle(in_path, out_path);
    }
}

bool decompress_path(const std::string &in_path, const std::string &out_path) {
    // symmetric: if directory provided, decompress every file with .rle -> .dec (simple)
    if (is_directory(in_path)) {
        auto files = list_files_recursive(in_path);
        std::vector<std::thread> ths;
        for (const auto &f : files) {
            std::string out = f + ".dec";
            ths.emplace_back([f, out](){ if (!decompress_file_rle(f, out)) std::cerr<<"Failed decompress "<<f<<"\n"; });
        }
        for (auto &t : ths) t.join();
        return true;
    } else {
        return decompress_file_rle(in_path, out_path);
    }
}

bool encrypt_path(const std::string &in_path, const std::string &out_path, const std::string &key) {
    if (is_directory(in_path)) {
        auto files = list_files_recursive(in_path);
        std::vector<std::thread> ths;
        for (const auto &f : files) {
            std::string out = f + ".enc";
            ths.emplace_back(xor_encrypt_file, f, out, key);
        }
        for (auto &t : ths) t.join();
        return true;
    } else {
        return xor_encrypt_file(in_path, out_path, key);
    }
}

bool decrypt_path(const std::string &in_path, const std::string &out_path, const std::string &key) {
    // XOR is symmetric
    return encrypt_path(in_path, out_path, key);
}
