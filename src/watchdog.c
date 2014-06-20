/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Process termination watchdog

#include "watchdog.h"
#include "params.h"
#ifdef _WIN32
# include <Windows.h>
# define _WIN32_OR_POSIX(a,b) a
#else /* ifdef _WIN32 */
# include <wait.h>
# include <unistd.h>
# include <sys/stat.h>
# define _WIN32_OR_POSIX(a,b) b
#endif /* ifdef _WIN32 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Tests if a process is valid (still running)
///
/// On Linux this is used in favour of kill(pid_t,int) because invalid PID
/// values like 0 and -1 have special meaning to kill and do not get reported
/// as invalid.
///
/// On Windows this checks the status of the process handle.
///
/// @return zero if the process is not running, non-zero if it is
int _is_running (
	_WIN32_OR_POSIX (HANDLE, pid_t) process ///<the process to the check>
    ) {
#ifdef _WIN32
	if ((process == NULL) || (process == INVALID_HANDLE_VALUE)) return 0;
	return WaitForSingleObject (process, 0) == WAIT_TIMEOUT;
#else /* ifdef _WIN32 */
    char tmp[32];
    if ((kill (process, 0) == 0) && (snprintf (tmp, sizeof (tmp), "/proc/%u/cwd", process) < 32)) {
        struct stat info;
        if (stat (tmp, &info) == 0) {
            if (S_ISDIR (info.st_mode)) {
				// The process exists in the table, but might be a child that has
				// terminated and not yet signalled us.
				int status;
				pid_t terminated;
				terminated = waitpid (process, &status, WNOHANG);
				return terminated != process;
			}
        }
    }
    return 0;
#endif /* ifdef _WIN32 */
}

/// @brief Implementation of watchdog(int,...)
///
/// This is separated out for use by unit tests. It performs one scan of the
/// processes.
///
/// @return the index of the terminated process, or -1 if all are valid
int _watchdog0_v (
    int count, ///<the number of pid_t/HANDLE parameters in pids>
    va_list processes ///<the pid_t/HANDLE values to monitor>
    ) {
    int i;
    for (i = 0; i < count; i++) {
		_WIN32_OR_POSIX (HANDLE, pid_t) process = va_arg (processes, _WIN32_OR_POSIX (HANDLE, pid_t));
        if (_is_running (process)) continue;
        if (verbose) fprintf (stdout, "Process %u is no longer valid\n", _WIN32_OR_POSIX (GetProcessId (process), process));
        return i;
    }
    return -1;
}

/// @brief Watches one or more processes for termination
///
/// The supplied PID/HANDLEs are monitored for any that have terminated. These
/// may be children spawned by this process, or other arbitrary processes
/// corresponding to logical parents.
///
/// Note that a process which cannot be queried, perhaps because of security
/// reasons, will be reported as terminated.
///
/// If the terminated proces was a spawned child then the signal from that
/// child will be consumed (see POSIX `waitpid`).
///
/// @return the index of the process which is no longer valid.
int watchdog (
    int count, ///<the number of pid_t/HANDLE parameters to follow>
    ... ///<the pid_t/HANDLE values to monitor>
    ) {
    int i;
    va_list processes;
    if (verbose) {
        va_start (processes, count);
        for (i = 0; i < count; i++) {
            fprintf (stdout, "Watching process %u for termination\n", _WIN32_OR_POSIX (GetProcessId (va_arg (processes, HANDLE)), va_arg (processes, pid_t)));
        }
        va_end (processes);
    }
    do {
        va_start (processes, count);
        i = _watchdog0_v (count, processes);
        va_end (processes);
        if (i >= 0) return i;
		_WIN32_OR_POSIX (Sleep (1000), sleep (1));
    } while (1);
}
