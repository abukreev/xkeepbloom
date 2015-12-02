#ifndef ARGS_H
#define ARGS_H

#include <time.h>

struct arguments {
    int help;
    int min_time_present;
    time_t min_time;
    int max_time_present;
    time_t max_time;
};

int parse_arguments(int argc, char* argv[], struct arguments* args);

#endif // ARGS_H
