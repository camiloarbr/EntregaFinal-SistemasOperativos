#include "../include/worker.h"
#include "../include/file_manager.h"
#include "../include/utils.h"

#include <vector>
#include <iostream>

// Placeholder compression/encryption functions. TODO: replace with real algorithms (LZW/AES/etc.)
static std::vector<uint8_t> placeholder_compress(const std::vector<uint8_t> &in) {
    // TODO: implement RLE/Huffman/LZW
    return in; // stub: no-op
}

static std::vector<uint8_t> placeholder_decompress(const std::vector<uint8_t> &in) {
    // TODO: implement
    return in; // stub
}

static std::vector<uint8_t> placeholder_encrypt(const std::vector<uint8_t> &in, const std::string &key) {
    // TODO: implement AES/Vigenere-like algorithm. For now XOR stub.
    std::vector<uint8_t> out = in;
    if (key.empty()) return out;
    for (size_t i = 0; i < out.size(); ++i) out[i] ^= key[i % key.size()];
    return out;
}

static std::vector<uint8_t> placeholder_decrypt(const std::vector<uint8_t> &in, const std::string &key) {
    // symmetric with XOR stub
    return placeholder_encrypt(in, key);
}

void *worker_entry(void *arg) {
    WorkerArgs *w = static_cast<WorkerArgs*>(arg);
    if (!w) return nullptr;

    log_info("Worker starting for file: %s", w->input_file.c_str());

    std::vector<uint8_t> data;
    if (!read_entire_file(w->input_file, data)) {
        log_error("Failed to read file: %s", w->input_file.c_str());
        return nullptr;
    }

    std::vector<uint8_t> out;
    if (w->opts.do_compress) {
        out = placeholder_compress(data);
    } else if (w->opts.do_decompress) {
        out = placeholder_decompress(data);
    } else if (w->opts.do_encrypt) {
        out = placeholder_encrypt(data, w->key);
    } else if (w->opts.do_decrypt) {
        out = placeholder_decrypt(data, w->key);
    } else {
        // no-op: copy
        out = data;
    }

    if (!write_entire_file(w->output_file, out)) {
        log_error("Failed to write output: %s", w->output_file.c_str());
    } else {
        log_info("Worker finished writing: %s", w->output_file.c_str());
    }

    return nullptr;
}
