#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "gsea.h"

void usage() {
    std::cout << "gsea [compress|decompress|encrypt|decrypt] -i <input> -o <output> [-k key]\n";
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(); return 1; }
    std::string cmd = argv[1];
    std::string in, out, key;

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "-i") == 0 && i+1 < argc) { in = argv[++i]; }
        else if (strcmp(argv[i], "-o") == 0 && i+1 < argc) { out = argv[++i]; }
        else if (strcmp(argv[i], "-k") == 0 && i+1 < argc) { key = argv[++i]; }
        else { std::cerr << "Unknown or incomplete arg: " << argv[i] << "\n"; usage(); return 1; }
    }

    if (in.empty()) { std::cerr << "missing -i\n"; usage(); return 1; }
    if (out.empty() && (cmd == "compress" || cmd == "decompress" || cmd == "encrypt" || cmd == "decrypt")) { std::cerr << "missing -o\n"; usage(); return 1; }

    bool ok = false;
    if (cmd == "compress") ok = compress_path(in, out);
    else if (cmd == "decompress") ok = decompress_path(in, out);
    else if (cmd == "encrypt") ok = encrypt_path(in, out, key);
    else if (cmd == "decrypt") ok = decrypt_path(in, out, key);
    else { std::cerr << "unknown command\n"; usage(); return 1; }

    return ok ? 0 : 2;
}
