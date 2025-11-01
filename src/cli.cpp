#include "../include/cli.h"

#include <getopt.h>
#include <iostream>

// Minimal parse implementation using getopt_long. This is a skeleton — extend as needed.
bool parse_cli(int argc, char **argv, Options &out) {
    static struct option long_options[] = {
        {"compress", no_argument, nullptr, 'c'},
        {"decompress", no_argument, nullptr, 'd'},
        {"encrypt", no_argument, nullptr, 'e'},
        {"decrypt", no_argument, nullptr, 'r'},
        {"input", required_argument, nullptr, 'i'},
        {"output", required_argument, nullptr, 'o'},
        {"key", required_argument, nullptr, 'k'},
        {"comp-alg", required_argument, nullptr, 'a'},
        {"enc-alg", required_argument, nullptr, 'b'},
        {0,0,0,0}
    };

    int opt;
    int opt_index = 0;
    while ((opt = getopt_long(argc, argv, "cderi:o:k:a:b:", long_options, &opt_index)) != -1) {
        switch (opt) {
            case 'c': out.do_compress = true; break;
            case 'd': out.do_decompress = true; break;
            case 'e': out.do_encrypt = true; break;
            case 'r': out.do_decrypt = true; break;
            case 'i': out.input_path = optarg; break;
            case 'o': out.output_path = optarg; break;
            case 'k': out.key = optarg; break;
            case 'a': out.comp_alg = optarg; break;
            case 'b': out.enc_alg = optarg; break;
            default:
                std::cerr << "Unknown option\n";
                return false;
        }
    }

    // Basic validation
    if (out.input_path.empty()) {
        std::cerr << "input path required\n";
        return false;
    }
    return true;
}
