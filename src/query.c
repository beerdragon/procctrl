/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Implements the `query` operation

#include "operations.h"
#include "params.h"
#include "errno.h"
#include <stdio.h>
#include <stdlib.h>

/// @brief Queries a child process
///
/// Tests if the child process, created by a previous call to the `start`
/// operation, is still active.
///
/// @return zero if the process is running, ESRCH or another non-zero error
///         code otherwise
int operation_query () {
    pid_t process;
    if (verbose) fprintf (stdout, "Querying spawned process\n");
    process = process_find ();
    if (process) {
        fprintf (stdout, "Process %u is running\n", process);
        return 0;
    } else {
        fprintf (stdout, "No child process is running\n");
        return ESRCH;
    }
}
