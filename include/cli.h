#pragma once

#include <string>

struct Options {
    bool do_compress = false;
    bool do_decompress = false;
    bool do_encrypt = false;
    bool do_decrypt = false;
    std::string comp_alg;
    std::string enc_alg;
    std::string input_path;
    std::string output_path;
    std::string key;
};

// Parse command line into options. Returns true on success.
bool parse_cli(int argc, char **argv, Options &out);
