#include "worker.h"
#include "file_manager.h"
#include "utils.h"

#include <vector>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>

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

// XOR encryption: simple XOR with key
static std::vector<uint8_t> encrypt_xor(const std::vector<uint8_t> &data, const std::string &key) {
    std::vector<uint8_t> out;
    out.reserve(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t key_byte = static_cast<uint8_t>(key[i % key.length()]);
        out.push_back(data[i] ^ key_byte);
    }
    
    return out;
}

// XOR decryption: symmetric (XOR is its own inverse)
static std::vector<uint8_t> decrypt_xor(const std::vector<uint8_t> &data, const std::string &key) {
    return encrypt_xor(data, key); // XOR is symmetric
}

// Select encryption algorithm based on algorithm name
static std::vector<uint8_t> encrypt_data(const std::vector<uint8_t> &data, const std::string &key, const std::string &algorithm) {
    std::string alg = algorithm;
    // Convert to lowercase for case-insensitive comparison
    for (char &c : alg) {
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
    }
    
    if (alg.empty() || alg == "vigenere") {
        return encrypt_vigenere(data, key);
    } else if (alg == "xor") {
        return encrypt_xor(data, key);
    } else {
        // Unknown algorithm - return empty vector (error will be handled by caller)
        return std::vector<uint8_t>();
    }
}

// Select decryption algorithm based on algorithm name
static std::vector<uint8_t> decrypt_data(const std::vector<uint8_t> &data, const std::string &key, const std::string &algorithm) {
    std::string alg = algorithm;
    // Convert to lowercase for case-insensitive comparison
    for (char &c : alg) {
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
    }
    
    if (alg.empty() || alg == "vigenere") {
        return decrypt_vigenere(data, key);
    } else if (alg == "xor") {
        return decrypt_xor(data, key);
    } else {
        // Unknown algorithm - return empty vector (error will be handled by caller)
        return std::vector<uint8_t>();
    }
}

void *worker_entry(void *arg) {
    WorkerArgs *w = static_cast<WorkerArgs*>(arg);
    if (!w) {
        log_error("worker_entry: received null argument");
        return reinterpret_cast<void*>(1); // Return error code
    }

    log_info("Worker starting for file: %s", w->input_file.c_str());

    // Validate input file exists and is accessible
    struct stat st;
    if (stat(w->input_file.c_str(), &st) != 0) {
        log_error("Input file '%s' does not exist or is not accessible: %s", 
                 w->input_file.c_str(), strerror(errno));
        return reinterpret_cast<void*>(1);
    }
    
    if (!S_ISREG(st.st_mode)) {
        log_error("Input path '%s' is not a regular file", w->input_file.c_str());
        return reinterpret_cast<void*>(1);
    }

    // Validate key for encryption/decryption
    if (w->opts.do_encrypt || w->opts.do_decrypt) {
        if (w->key.empty() || w->key.length() == 0) {
            log_error("File '%s': Encryption/decryption requires a key (-k option)", 
                     w->input_file.c_str());
            return reinterpret_cast<void*>(1);
        }
        
        // Validate encryption algorithm
        std::string alg = w->opts.enc_alg;
        for (char &c : alg) {
            if (c >= 'A' && c <= 'Z') {
                c = c - 'A' + 'a';
            }
        }
        
        if (!alg.empty() && alg != "vigenere" && alg != "xor") {
            log_error("File '%s': Unknown encryption algorithm '%s'. Supported: vigenere, xor", 
                     w->input_file.c_str(), w->opts.enc_alg.c_str());
            return reinterpret_cast<void*>(1);
        }
    }

    std::vector<uint8_t> data;
    if (!read_entire_file(w->input_file, data)) {
        log_error("File '%s': Failed to read file", w->input_file.c_str());
        return reinterpret_cast<void*>(1);
    }

    if (data.empty()) {
        log_info("File '%s' is empty, skipping processing", w->input_file.c_str());
        // Still write empty file
        data.clear();
    }

    std::vector<uint8_t> out;
    
    try {
        // Handle operations in correct order:
        // For compression + encryption: compress first, then encrypt
        // For decryption + decompression: decrypt first, then decompress
        
        if (w->opts.do_compress) {
            out = compress_rle(data);
            if (out.empty() && !data.empty()) {
                log_error("File '%s': Compression failed (empty output)", w->input_file.c_str());
                return reinterpret_cast<void*>(1);
            }
            log_info("File '%s': Compressed %zu bytes to %zu bytes (RLE)", 
                    w->input_file.c_str(), data.size(), out.size());
            
            // If also encrypting, encrypt the compressed data
            if (w->opts.do_encrypt) {
                std::string alg = w->opts.enc_alg.empty() ? "vigenere" : w->opts.enc_alg;
                std::vector<uint8_t> encrypted = encrypt_data(out, w->key, alg);
                if (encrypted.empty() && !out.empty()) {
                    log_error("File '%s': Encryption failed", w->input_file.c_str());
                    return reinterpret_cast<void*>(1);
                }
                log_info("File '%s': Encrypted %zu bytes using %s cipher", 
                        w->input_file.c_str(), out.size(), alg.c_str());
                out = encrypted;
            }
        } else if (w->opts.do_decompress) {
            // If also decrypting, decrypt first, then decompress
            if (w->opts.do_decrypt) {
                std::string alg = w->opts.enc_alg.empty() ? "vigenere" : w->opts.enc_alg;
                std::vector<uint8_t> decrypted = decrypt_data(data, w->key, alg);
                if (decrypted.empty() && !data.empty()) {
                    log_error("File '%s': Decryption failed", w->input_file.c_str());
                    return reinterpret_cast<void*>(1);
                }
                log_info("File '%s': Decrypted %zu bytes using %s cipher", 
                        w->input_file.c_str(), data.size(), alg.c_str());
                
                out = decompress_rle(decrypted);
                if (out.empty() && !decrypted.empty()) {
                    log_error("File '%s': Decompression failed (invalid RLE data or empty output)", 
                             w->input_file.c_str());
                    return reinterpret_cast<void*>(1);
                }
                log_info("File '%s': Decompressed %zu bytes to %zu bytes (RLE)", 
                        w->input_file.c_str(), decrypted.size(), out.size());
            } else {
                out = decompress_rle(data);
                if (out.empty() && !data.empty()) {
                    log_error("File '%s': Decompression failed (invalid RLE data or empty output)", 
                             w->input_file.c_str());
                    return reinterpret_cast<void*>(1);
                }
                log_info("File '%s': Decompressed %zu bytes to %zu bytes (RLE)", 
                        w->input_file.c_str(), data.size(), out.size());
            }
        } else if (w->opts.do_encrypt) {
            std::string alg = w->opts.enc_alg.empty() ? "vigenere" : w->opts.enc_alg;
            out = encrypt_data(data, w->key, alg);
            if (out.empty() && !data.empty()) {
                log_error("File '%s': Encryption failed", w->input_file.c_str());
                return reinterpret_cast<void*>(1);
            }
            log_info("File '%s': Encrypted %zu bytes using %s cipher", 
                    w->input_file.c_str(), data.size(), alg.c_str());
        } else if (w->opts.do_decrypt) {
            std::string alg = w->opts.enc_alg.empty() ? "vigenere" : w->opts.enc_alg;
            out = decrypt_data(data, w->key, alg);
            if (out.empty() && !data.empty()) {
                log_error("File '%s': Decryption failed", w->input_file.c_str());
                return reinterpret_cast<void*>(1);
            }
            log_info("File '%s': Decrypted %zu bytes using %s cipher", 
                    w->input_file.c_str(), data.size(), alg.c_str());
        } else {
            // no-op: copy
            out = data;
        }
    } catch (const std::exception &e) {
        log_error("File '%s': Exception during processing: %s", w->input_file.c_str(), e.what());
        return reinterpret_cast<void*>(1);
    } catch (...) {
        log_error("File '%s': Unknown exception during processing", w->input_file.c_str());
        return reinterpret_cast<void*>(1);
    }

    if (!write_entire_file(w->output_file, out)) {
        log_error("File '%s': Failed to write output to '%s'", 
                 w->input_file.c_str(), w->output_file.c_str());
        return reinterpret_cast<void*>(1);
    }

    log_info("File '%s': Successfully processed and written to '%s'", 
            w->input_file.c_str(), w->output_file.c_str());
    return reinterpret_cast<void*>(0); // Return success code
}
