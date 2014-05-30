/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Program parameters
///
/// Processes the command line to populate the global variables used to hold
/// the parameters.

/// @brief Defines a global variable exported through params.h
#define MODULE_VAR_EXTERN
/// @brief Defines a global variable exported through params.h
#define MODULE_VAR_CONST
#include "params.h"
#undef MODULE_VAR_EXTERN
#undef MODULE_VAR_CONST
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// @brief Creates an automatic process identifier from the command line
///
/// The command line used for the symbolic process identifier if none is
/// explicitly set with the `k` parameter.
static void auto_process_identifier () {
    size_t size = spawn_argc;
    int arg;
    char *ptr;
    for (arg = 0; arg < spawn_argc; arg++) {
        size += strlen (spawn_argv[arg]);
    }
    process_identifier = ptr = (char*)malloc (size);
    if (!process_identifier) abort ();
    for (arg = 0; arg < spawn_argc; arg++) {
        if (arg) *(ptr++) = ' ';
        size_t len = strlen (spawn_argv[arg]);
        memcpy (ptr, spawn_argv[arg], len);
        ptr += len;
    }
    *ptr = 0;
}

/// @brief Expands the `~` character in the data directory path
///
/// The `~` is replaced by the content of the `HOME` environment variable.
static void expand_home_dir () {
    const char *home = getenv ("HOME");
    char *path;
    size_t size;
    if (!home) home = ".";
    size = strlen (home) + strlen (data_dir);
    path = (char*)malloc (size);
    if (!path) abort ();
    snprintf (path, size, "%s%s", home, data_dir + 1);
    data_dir = path;
}

/// @brief Process the command line arguments
///
/// The arguments are processed and values set into the global variables
/// published by params.h that other components can then access. Default
/// values are set where required for anything. The meaning of each
/// argument is documented in the `man` page.
///
/// @return zero if the arguments are okay, otherwise a non-zero error code
int params (
    int argc, ///<the number of arguments, as passed to main(int,char**)
    char **argv ///<the argument values, as passed to main(int,char**)
    ) {
    opterr = 0;
    int arg;
    data_dir = "~/.procctrl";
    global_identifier = 0;
    process_identifier = NULL;
    parent_process = getppid ();
    watch_parent = 0;
    verbose = 0;
    housekeep_mode = HOUSEKEEP_FULL;
    while ((arg = getopt (argc, argv, "d:h:Kk:P:pv")) != -1) {
        switch (arg) {
            case 'd' :
                data_dir = optarg;
                break;
            case 'h' :
                housekeep_mode = atoi (optarg);
                break;
            case 'K' :
                global_identifier = 1;
                break;
            case 'k' :
                process_identifier = optarg;
                break;
            case 'P' :
                parent_process = atoi (optarg);
                break;
            case 'p' :
                watch_parent = 1;
                break;
            case 'v' :
                verbose = 1;
                break;
            case '?' :
                switch (optopt) {
                    case 'd' :
                        fprintf (stderr, "-d requires a directory\n");
                        break;
                    case 'h' :
                        fprintf (stderr, "-h requires a mode flag\n");
                        break;
                    case 'k' :
                        fprintf (stderr, "-k requires a process identifier key\n");
                        break;
                    case 'P' :
                        fprintf (stderr, "-P requires a process ID\n");
                        break;
                    default :
                        if (isprint (optopt)) {
                            fprintf (stderr, "Unknown option '%c'\n", optopt);
                        } else {
                            fprintf (stderr, "Unknown option '\\x%x'\n", optopt);
                        }
                        break;
                }
                return EINVAL;
            default :
                abort ();
        }
    }
    arg = optind;
    operation = (arg < argc) ? argv[arg++] : NULL;
    spawn_argc = argc - arg;
    spawn_argv = (char**)malloc ((spawn_argc + 1) * sizeof (char*));
    if (!spawn_argv) abort ();
    memcpy (spawn_argv, argv + arg, spawn_argc * sizeof (char*));
    spawn_argv[spawn_argc] = NULL;
    if (process_identifier == NULL) auto_process_identifier ();
    if ((data_dir[0] == '~') && (data_dir[1] == '/')) expand_home_dir ();
    if (verbose) {
        fprintf (stdout, "Data directory     : %s\n", data_dir);
        fprintf (stdout, "Identifier scope   : %s\n", global_identifier ? "Global" : "Local to parent");
        fprintf (stdout, "Process identifier : %s\n", process_identifier ? process_identifier : "");
        fprintf (stdout, "Parent PID         : %u\n", parent_process);
        fprintf (stdout, "Watch parent       : %s\n", watch_parent ? "Yes" : "No");
        fprintf (stdout, "Housekeeping mode  : %d\n", housekeep_mode);
        fprintf (stdout, "Operation          : %s\n", operation);
        fprintf (stdout, "Command line       :");
        for (arg = 0; arg < spawn_argc; arg++) {
            fprintf (stdout, " %s", spawn_argv[arg]);
        }
        fprintf (stdout, "\n");
    }
    return 0;
}
