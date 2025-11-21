#include "worker.h"
#include "file_manager.h"
#include "utils.h"

#include <vector>
#include <iostream>

// RLE Compression: encode sequences as [count][byte] pairs
// Handles runs > 255 by splitting into multiple runs
static std::vector<uint8_t> compress_rle(const std::vector<uint8_t> &in) {
    if (in.empty()) {
        return in;
    }
    
    std::vector<uint8_t> out;
    out.reserve(in.size()); // Reserve space, may grow if compression is effective
    
    uint8_t prev = in[0];
    unsigned int run = 1;
    
    for (size_t i = 1; i < in.size(); ++i) {
        uint8_t current = in[i];
        
        if (current == prev && run < 255) {
            // Continue the run
            ++run;
        } else {
            // Write the current run and start a new one
            // Split runs > 255 into multiple runs of 255
            while (run > 255) {
                out.push_back(255);
                out.push_back(prev);
                run -= 255;
            }
            if (run > 0) {
                out.push_back(static_cast<uint8_t>(run));
                out.push_back(prev);
            }
            prev = current;
            run = 1;
        }
    }
    
    // Write the final run
    while (run > 255) {
        out.push_back(255);
        out.push_back(prev);
        run -= 255;
    }
    if (run > 0) {
        out.push_back(static_cast<uint8_t>(run));
        out.push_back(prev);
    }
    
    return out;
}

// RLE Decompression: read [count][byte] pairs and expand
static std::vector<uint8_t> decompress_rle(const std::vector<uint8_t> &in) {
    if (in.empty()) {
        return in;
    }
    
    // RLE format requires pairs, so input must be even length
    if (in.size() % 2 != 0) {
        log_error("Invalid RLE data: odd number of bytes");
        return std::vector<uint8_t>();
    }
    
    std::vector<uint8_t> out;
    // Estimate output size (may be larger if compression was effective)
    out.reserve(in.size() * 2);
    
    for (size_t i = 0; i < in.size(); i += 2) {
        uint8_t count = in[i];
        uint8_t value = in[i + 1];
        
        // Expand: write 'count' copies of 'value'
        out.insert(out.end(), count, value);
    }
    
    return out;
}

// Vigenère encryption: add key byte to data byte modulo 256
static std::vector<uint8_t> encrypt_vigenere(const std::vector<uint8_t> &data, const std::string &key) {
    std::vector<uint8_t> out;
    out.reserve(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t key_byte = static_cast<uint8_t>(key[i % key.length()]);
        uint8_t encrypted = (data[i] + key_byte) % 256;
        out.push_back(encrypted);
    }
    
    return out;
}

// Vigenère decryption: subtract key byte from data byte modulo 256
static std::vector<uint8_t> decrypt_vigenere(const std::vector<uint8_t> &data, const std::string &key) {
    std::vector<uint8_t> out;
    out.reserve(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t key_byte = static_cast<uint8_t>(key[i % key.length()]);
        // Add 256 before modulo to handle negative values correctly
        uint8_t decrypted = (data[i] - key_byte + 256) % 256;
        out.push_back(decrypted);
    }
    
    return out;
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

    // Validate key for encryption/decryption
    if (w->opts.do_encrypt || w->opts.do_decrypt) {
        if (w->key.empty() || w->key.length() == 0) {
            log_error("Encryption/decryption requires a key (-k option)");
            return nullptr;
        }
    }

    std::vector<uint8_t> out;
    
    // Handle operations in correct order:
    // For compression + encryption: compress first, then encrypt
    // For decryption + decompression: decrypt first, then decompress
    
    if (w->opts.do_compress) {
        out = compress_rle(data);
        log_info("Compressed %zu bytes to %zu bytes (RLE)", data.size(), out.size());
        
        // If also encrypting, encrypt the compressed data
        if (w->opts.do_encrypt) {
            std::vector<uint8_t> encrypted = encrypt_vigenere(out, w->key);
            log_info("Encrypted %zu bytes using Vigenère cipher", out.size());
            out = encrypted;
        }
    } else if (w->opts.do_decompress) {
        // If also decrypting, decrypt first, then decompress
        if (w->opts.do_decrypt) {
            std::vector<uint8_t> decrypted = decrypt_vigenere(data, w->key);
            log_info("Decrypted %zu bytes using Vigenère cipher", data.size());
            out = decompress_rle(decrypted);
            log_info("Decompressed %zu bytes to %zu bytes (RLE)", decrypted.size(), out.size());
        } else {
            out = decompress_rle(data);
            log_info("Decompressed %zu bytes to %zu bytes (RLE)", data.size(), out.size());
        }
    } else if (w->opts.do_encrypt) {
        out = encrypt_vigenere(data, w->key);
        log_info("Encrypted %zu bytes using Vigenère cipher", data.size());
    } else if (w->opts.do_decrypt) {
        out = decrypt_vigenere(data, w->key);
        log_info("Decrypted %zu bytes using Vigenère cipher", data.size());
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
