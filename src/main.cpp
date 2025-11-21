// New main using the CLI/file manager/worker skeleton with pthreads
#include <iostream>
#include <vector>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
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

    // Validate input path exists and is accessible
    struct stat st;
    if (stat(opts.input_path.c_str(), &st) != 0) {
        log_error("Input path '%s' does not exist or is not accessible: %s", 
                 opts.input_path.c_str(), strerror(errno));
        return 2;
    }

    if (!S_ISDIR(st.st_mode) && !S_ISREG(st.st_mode)) {
        log_error("Input path '%s' is neither a file nor a directory", opts.input_path.c_str());
        return 2;
    }

    auto files = list_input_files(opts.input_path);
    if (files.empty()) {
        log_error("No input files found for path: %s", opts.input_path.c_str());
        return 2;
    }

    log_info("Found %zu file(s) to process", files.size());

    // Create one pthread per file
    std::vector<pthread_t> threads(files.size());
    std::vector<WorkerArgs*> args(files.size(), nullptr);
    size_t threads_created = 0;

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
            log_error("Failed to create thread for '%s': %s", files[i].c_str(), strerror(rc));
            delete args[i];
            args[i] = nullptr;
        } else {
            threads_created++;
        }
    }

    if (threads_created == 0) {
        log_error("Failed to create any worker threads");
        return 3;
    }

    // Join threads and count successes/failures
    size_t success_count = 0;
    size_t failure_count = 0;

    for (size_t i = 0; i < threads.size(); ++i) {
        if (args[i] == nullptr) {
            failure_count++;
            continue;
        }

        void *result = nullptr;
        int rc = pthread_join(threads[i], &result);
        if (rc != 0) {
            log_error("Failed to join thread for '%s': %s", args[i]->input_file.c_str(), strerror(rc));
            failure_count++;
        } else {
            // Check return value: 0 = success, non-zero = failure
            uintptr_t status = reinterpret_cast<uintptr_t>(result);
            if (status == 0) {
                success_count++;
            } else {
                failure_count++;
            }
        }
        delete args[i];
    }

    // Print summary
    log_info("Processing complete: %zu file(s) processed successfully, %zu file(s) failed", 
            success_count, failure_count);

    if (failure_count > 0) {
        return 4; // Return error code if any files failed
    }

    return 0;
}

