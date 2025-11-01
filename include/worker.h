#pragma once

#include <string>
#include <vector>
#include "cli.h"

struct WorkerArgs {
    Options opts;
    std::string input_file;
    std::string output_file;
    std::string key;
};

// Entry point for pthread
void *worker_entry(void *arg);
