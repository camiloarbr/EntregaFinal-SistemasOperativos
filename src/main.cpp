// New main using the CLI/file manager/worker skeleton with pthreads
#include <iostream>
#include <vector>
#include <pthread.h>
#include "cli.h"
#include "file_manager.h"
#include "worker.h"
#include "utils.h"

void usage() {
    std::cout << "gsea [--compress|--decompress|--encrypt|--decrypt] --input <path> --output <path> [-k key]\n";
}

int main(int argc, char **argv) {
    Options opts;
    if (!parse_cli(argc, argv, opts)) {
        usage();
        return 1;
    }

    init_logging();

    auto files = list_input_files(opts.input_path);
    if (files.empty()) {
        log_error("No input files found for path: %s", opts.input_path.c_str());
        return 2;
    }

    // Create one pthread per file
    std::vector<pthread_t> threads(files.size());
    std::vector<WorkerArgs*> args(files.size(), nullptr);

    for (size_t i = 0; i < files.size(); ++i) {
        args[i] = new WorkerArgs();
        args[i]->opts = opts;
        args[i]->input_file = files[i];
        // build a simple output filename: join output_path + basename(file)
        std::string base = basename_from_path(files[i]);
        args[i]->output_file = path_join(opts.output_path.empty() ? "." : opts.output_path, base);
        args[i]->key = opts.key;

        int rc = pthread_create(&threads[i], nullptr, worker_entry, args[i]);
        if (rc != 0) {
            log_error("Failed to create thread for %s", files[i].c_str());
        }
    }

    // join
    for (size_t i = 0; i < threads.size(); ++i) {
        pthread_join(threads[i], nullptr);
        delete args[i];
    }

    log_info("All workers finished");
    return 0;
}
