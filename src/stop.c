/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Implements the `stop` operation

#include "operations.h"
#include "kill.h"
#include "params.h"
#include "process.h"
#ifndef _WIN32
# include <errno.h>
# include <signal.h>
#endif /* ifndef _WIN32 */
#include <stdio.h>
#include <stdlib.h>

/// @brief Stops the child process
///
/// If there is an active process with the symbolic identifier then it is
/// killed. If there is a watchdog process from the original spawn then that
/// will also terminate when it detects the child termination.
///
/// @return zero if successful, otherwise a non-zero error code
int operation_stop () {
	_WIN32_OR_POSIX (HANDLE, pid_t) process;
    if (verbose) fprintf (stdout, "Stopping spawned process\n");
    process = process_find ();
	if (process) {
        if (verbose) fprintf (stdout, "Killing process %u\n", _WIN32_OR_POSIX (GetProcessId (process), process));
        int result = kill_process (process);
#ifdef _WIN32
		CloseHandle (process);
#endif /* ifdef _WIN32 */
		return result;
    } else {
        if (verbose) fprintf (stdout, "No process to stop\n");
		return _WIN32_OR_POSIX (ERROR_NOT_FOUND, ESRCH);
    }
}
