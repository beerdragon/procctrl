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
#include "process.h"
#ifndef _WIN32
# include <errno.h>
#endif /* ifndef _WIN32 */
#include <stdio.h>
#include <stdlib.h>

/// @brief Queries a child process
///
/// Tests if the child process, created by a previous call to the `start`
/// operation, is still active.
///
/// @return zero if the process is running, ESRCH/ERROR_NOT_FOUND or another
///         non-zero error code otherwise
int operation_query () {
	_WIN32_OR_POSIX (HANDLE, pid_t) process;
    if (verbose) fprintf (stdout, "Querying spawned process\n");
    process = process_find ();
    if (process) {
        if (verbose) fprintf (stdout, "Process %u is running\n", _WIN32_OR_POSIX (GetProcessId (process), process));
#ifdef _WIN32
		CloseHandle (process);
#endif /* ifdef _WIN32 */
        return 0;
    } else {
        if (verbose) fprintf (stdout, "No child process is running\n");
		return _WIN32_OR_POSIX (ERROR_NOT_FOUND, ESRCH);
    }
}
