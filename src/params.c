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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// @brief Copies the argument strings
///
/// The argument strings might have been allocated on a stack; this will make
/// sure that spawn_argv is a single heap allocated block.
///
/// @return the allocated array
static char **copy_args (
    int argc, ///<the number of arguments>
    char **argv ///<the argument strings>
    ) {
    int i;
    size_t size = (argc + 1) * sizeof (char*);
    char **result, *ptr;
    for (i = 0; i < argc; i++) {
        size += strlen (argv[i]) + 1;
    }
    result = (char**)malloc (size);
    if (!result) abort ();
    ptr = (char*)(result + argc + 1);
    for (i = 0; i < argc; i++) {
        char *arg = argv[i];
        size_t len = strlen (arg);
        memcpy (ptr, arg, len + 1);
        result[i] = ptr;
        ptr += len + 1;
    }
    result[argc] = NULL;
    return result;
}

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
/// Note that this is implemented with `getopt` which can modify the array
/// passed. If used once in a program this is okay - the parameters to
/// main(int,char**) can be used. If needed multiple times, for example as
/// part of the unit tests, use the params_v(int,...) wrapper instead.
///
/// @return zero if the arguments are okay, otherwise a non-zero error code
int params (
    int argc, ///<the number of arguments, as passed to main(int,char**)
    char **argv ///<the argument values, as passed to main(int,char**)
    ) {
    data_dir = "~/.procctrl";
    global_identifier = 0;
    process_identifier = NULL;
    parent_process = getppid ();
    watch_parent = 0;
    verbose = 0;
    housekeep_mode = HOUSEKEEP_FULL;
    if (argc > 1) {
        int arg;
        int optind_save = optind;
        int opterr_save = opterr;
        optind = 1;
        opterr = 0;
        while ((arg = getopt (argc, argv, "d:H:Kk:P:pv")) != -1) {
            switch (arg) {
                case 'd' :
                    data_dir = strdup (optarg);
                    if (!data_dir) abort ();
                    break;
                case 'H' :
                    housekeep_mode = atoi (optarg);
                    break;
                case 'K' :
                    global_identifier = 1;
                    break;
                case 'k' :
                    process_identifier = strdup (optarg);
                    if (!process_identifier) abort ();
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
                        case 'H' :
                            fprintf (stderr, "-H requires a mode flag\n");
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
                                fprintf (stderr, "Unknown option 0x%02X\n", optopt & 0xFF);
                            }
                            break;
                    }
                    optind = optind_save;
                    opterr = opterr_save;
                    return EINVAL;
                default :
                    abort ();
            }
        }
        arg = optind;
        operation = (arg < argc) ? argv[arg++] : NULL;
        spawn_argc = argc - arg;
        argv += arg;
        optind = optind_save;
        opterr = opterr_save;
    } else {
        operation = NULL;
        spawn_argc = 0;
    }
    spawn_argv = copy_args (spawn_argc, argv);
    if (process_identifier == NULL) auto_process_identifier ();
    if ((data_dir[0] == '~') && (data_dir[1] == '/')) expand_home_dir ();
    if (verbose) {
        int arg;
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

/// @brief Vararg wrapper for params(int,char**)
///
/// This will allocate a block of memory with a copy of the parameter strings
/// supplied here. This will be passed to params(int,char**) which may then
/// modify that copy without affecting the original arguments.
///
/// Each of the variant arguments is expected to be `const char*`. A first
/// argument of "main" (for the program name) is prepended to the supplied
/// parameters automatically.
///
/// This is supplied for use by the unit tests.
///
/// @return the return code from params(int,char**) or ENOMEM
int params_v (
    int num, ///<the number of arguments following this parameter>
    ... ///<the `const char*` parameters>
    ) {
    char **argv;
    char *ptr;
    int i;
    va_list args;
    size_t size;
    // Calculate the amount of memory needed
    size = (num + 2) * sizeof (char*) + 5;
    va_start (args, num);
    for (i = 0; i < num; i++) {
        char *arg = va_arg (args, char*);
        size += strlen (arg) + 1;
    }
    va_end (args);
    // Allocate the block
    argv = (char**)malloc (size);
    if (!argv) return ENOMEM;
    // Copy the strings into the second part of the block and put pointers
    // to there in the first part of the block.
    ptr = (char*)(argv + num + 2);
    argv[0] = ptr;
    memcpy (ptr, "main", 5);
    ptr += 5;
    va_start (args, num);
    for (i = 0; i < num; i++) {
        char *arg = va_arg (args, char*);
        size = strlen (arg);
        argv[i + 1] = ptr;
        memcpy (ptr, arg, size + 1);
        ptr += size + 1;
    }
    va_end (args);
    argv[num + 1] = NULL;
    // Process the parameters
    i = params (num + 1, argv);
    // Release the memory
    free (argv);
    return i;
}

/// @brief Sets verbose mode for unit tests
void _verbose_test () {
    verbose = 1;
}
