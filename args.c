#include <args.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage(FILE* stream)
{
    fprintf(stream, "Usage: xkeepbloom\n");
}

int parse_time(char* str, time_t* value)
{
    size_t hours;
    size_t minutes;
    char* pos = NULL;
    char* endptr;

    assert(NULL != str);
    pos = strstr(str, ":");
    if (NULL == pos) {
        return 1;
    }

    *pos = '\0';
    hours = strtol(str, &endptr, 10);
//    fprintf(stderr, "hours  = %ld\n", hours);
    if (endptr != pos || hours < 0 || hours > 23) {
        return 1;
    }

    pos++;

    minutes = strtol(pos, &endptr, 10);
//    fprintf(stderr, "minutes = %ld\n", minutes);
    if (endptr - pos != strlen(pos) || minutes < 0 || minutes > 59) {
        return 1;
    }

    assert(NULL != value);    
    *value = (hours * 60 + minutes) * 60;
//    fprintf(stderr, "value = %ld\n", *value);

    return 0;
}

int parse_arguments(int argc, char* argv[], struct arguments* args)
{
    char* str_min_time = NULL;
    char* str_max_time = NULL;
    int flag_help = 0;
    int index;
    int c;
    int exit_code = 0;

    assert(NULL != args);

    opterr = 0;
    while (0 == exit_code && (c = getopt (argc, argv, "t:T:h")) != -1)
    switch (c) {
       case 't':
            str_min_time = optarg;
            break;
        case 'T':
            str_max_time = optarg;
            break;
        case 'h':
            flag_help = 1;
            break;
        case '?':
            if ('t' == optopt || 'T' == optopt) {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            } else if (isprint (optopt)) {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            } else {
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            }
            exit_code = 1;
            break;
      default:
          exit_code = 1;
    }

    for (index = optind; index < argc; index++) {
        exit_code = 1;
        fprintf(stderr, "Non-option argument %s\n", argv[index]);
    }

    if (0 != exit_code) {
        usage(stderr);
        return exit_code;
    }

    if (flag_help) {
        args->help = 1;
        usage(stdout);
        return 0;
    }

    if (NULL != str_min_time) {
        if (!parse_time(str_min_time, &args->min_time)) {
            args->min_time_present = 1;
        } else {
            fprintf(stderr, "Can't parse min time value\n");
            return 1;
        }
    }
    if (NULL != str_max_time) {
        if (!parse_time(str_max_time, &args->max_time)) {
            args->max_time_present = 1;
        } else {
            fprintf(stderr, "Can't parse max time value\n");
            return 1;
        }
    }

    if (NULL != str_min_time && NULL != str_max_time &&
            args->min_time >= args->max_time) {
        fprintf(stderr, "Min time should be earlier than max time\n");
        return 1;
    }

    return 0;
}

