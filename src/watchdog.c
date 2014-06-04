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
#include <wait.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

/// @brief Tests if a process is valid
///
/// This is used in favour of kill(pid_t,int) because invalid PID values like
/// 0 and -1 have special meaning to kill and do not get reported as invalid.
///
/// @return zero if the process is not running, non-zero if it is
int _is_running (
    pid_t process ///<the process identifier to check>
    ) {
    char tmp[32];
    if ((kill (process, 0) == 0) && (snprintf (tmp, sizeof (tmp), "/proc/%u/cwd", process) < 32)) {
        struct stat info;
        if (stat (tmp, &info) == 0) {
            return S_ISDIR (info.st_mode);
        }
    }
    return 0;
}

/// @brief Implementation of watchdog(int,...)
///
/// This is separated out for use by unit tests. It performs one scan of the
/// processes.
///
/// @return the index of the terminated process, or -1 if all are valid
int _watchdog0_v (
    int count, ///<the number of pid_t parameters in pids>
    va_list pids ///<the pid_t values to monitor>
    ) {
    int i;
    for (i = 0; i < count; i++) {
        pid_t process = va_arg (pids, pid_t);
        if (_is_running (process)) {
            // The process exists in the table, but might be a child that has
            // terminated and not yet signalled us.
            int status;
            pid_t terminated;
            terminated = waitpid (process, &status, WNOHANG);
            if (terminated != process) continue;
        }
        if (verbose) fprintf (stdout, "Process %u is no longer valid\n", process);
        return i;
    }
    return -1;
}

/// @brief Watches one or more PIDs for termination
///
/// The supplied PIDs are monitored for any that have terminated. These may be
/// child PIDs spawned by this process, or other arbitrary PIDs corresponding
/// to logical parent processes.
///
/// Note that a process which cannot be queried, perhaps because of security
/// reasons, will be reported as terminated.
///
/// If the terminated proces was a spawned child then the signal from that
/// child will be consumed (see POSIX `waitpid`).
///
/// @return the index of the process whose PID is no longer valid.
int watchdog (
    int count, ///<the number of pid_t parameters to follow>
    ... ///<the pid_t values to monitor>
    ) {
    int i;
    va_list pids;
    if (verbose) {
        va_start (pids, count);
        for (i = 0; i < count; i++) {
            fprintf (stdout, "Watching process %u for termination\n", va_arg (pids, pid_t));
        }
        va_end (pids);
    }
    do {
        va_start (pids, count);
        i = _watchdog0_v (count, pids);
        va_end (pids);
        if (i >= 0) return i;
        sleep (1);
    } while (1);
}
